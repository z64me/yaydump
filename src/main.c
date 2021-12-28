/*
 * yaydump.c <z64.me>
 *
 * finds, decompresses, and dumps all compressed
 * yay files from a Nintendo 64 rom image
 *
 */

#include "yay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

void die(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	if (errno)
	{
		fprintf(stderr, ": ");
		perror("");
	}
	else
		fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/* minimal file loader
 * returns 0 on failure
 * returns pointer to loaded file on success
 */
void *loadfile(const char *fn, size_t *sz)
{
	FILE *fp;
	void *dat;
	
	/* rudimentary error checking returns 0 on any error */
	if (
		!fn
		|| !sz
		|| !(fp = fopen(fn, "rb"))
		|| fseek(fp, 0, SEEK_END)
		|| !(*sz = ftell(fp))
		|| fseek(fp, 0, SEEK_SET)
		|| !(dat = malloc(*sz))
		|| fread(dat, 1, *sz, fp) != *sz
		|| fclose(fp)
	)
		return 0;
	
	return dat;
}

/* minimal file writer
 * returns 0 on failure
 * returns non-zero on success
 */
int savefile(const char *fn, const void *dat, const size_t sz)
{
	FILE *fp;
	
	/* rudimentary error checking returns 0 on any error */
	if (
		!fn
		|| !sz
		|| !dat
		|| !(fp = fopen(fn, "wb"))
		|| fwrite(dat, 1, sz, fp) != sz
		|| fclose(fp)
	)
		return 0;
	
	return 1;
}

void yaydump(const void *in, size_t inSz, const char *outDir)
{
	const uint8_t *datStart = in;
	const uint8_t *datEnd = datStart + (inSz - 16);
	const uint8_t *dat;
	void *out;
	
	if (!(out = malloc(inSz)))
		die("memory error");
	
	for (dat = datStart; dat < datEnd; dat += 4)
	{
		char outFn[1024];
		uint32_t decSz;
		uint32_t ret;
		
		/* skip non-matches */
		if (memcmp(dat, "Yay0", 4) && memcmp(dat, "Yay1", 4))
			continue;
		
		decSz = (dat[4] << 24) | (dat[5] << 16) | (dat[6] << 8) | dat[7];
		
		/* skip files with decompressed sizes not multiples of 4 */
		/*if (decSz & 0x3)
		{
			fprintf(stderr
				, "skipping file at %08lx, (decSz & 0x3) != 0 (%08x)\n"
				, dat - datStart, decSz
			);
			continue;
		}*/
		
		ret = Yay_decodeAll(dat, -1, out);
		if (ret > decSz)
		{
			/*fprintf(stderr
				, "skipping file at %08lx, ret > decSz (%08x > %08x)\n"
				, dat - datStart, ret, decSz
			);
			continue;*/
		}
		
		snprintf(outFn, sizeof(outFn), "%s/%08lx.bin", outDir, dat - datStart);
		if (!savefile(outFn, out, ret))
			die("error writing file '%s'", outFn);
	}
	
	free(out);
}

int main(int argc, char *argv[])
{
	const char *inFn = argv[1];
	const char *outDir = argv[2];
	void *in;
	size_t inSz;
	
	if (argc != 3)
		die("arguments: yaydump \"in-rom.ndd\" \"outdir\"");
	
	if (!(in = loadfile(inFn, &inSz)))
		die("failed to read file '%s'", inFn);
	
	if (mkdir(outDir, 0777) && errno != EEXIST)
		die("something went wrong creating directory '%s'", outDir);
	
	yaydump(in, inSz, outDir);
	
	free(in);
	return 0;
}
