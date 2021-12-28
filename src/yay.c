#include "yay.h"

// modified by Luigi Auriemma
// version 1.0 (20050701)
// by thakis

static u32 Yay_doDWORD(u32 d)
{
	u8 w1 = d & 0xFF;
	u8 w2 = (d >> 8) & 0xFF;
	u8 w3 = (d >> 16) & 0xFF;
	u8 w4 = d >> 24;
	return (w1 << 24) | (w2 << 16) | (w3 << 8) | w4;
}

static u32 decodeYay(const u8 *codes, const u8 *counts, const u8 *srcData, u8 *dst, u32 uncompressedSize)
{
	u32 r_srcPos = 0;
	u32 r_dstPos = 0;
	//Ret r = { 0, 0 };
	//int srcPlace = 0, dstPlace = 0; //current read/write positions
	
	u32 codePos = 0;
	u32 countPos = 0;

	u32 validBitCount = 0; //number of valid bits left in "code" byte
	u8 currCodeByte;
	while(r_dstPos < uncompressedSize)
	{
		//read new "code" byte if the current one is used up
		if(validBitCount == 0)
		{
			currCodeByte = codes[codePos];
			++codePos;
			validBitCount = 8;
		}

		if((currCodeByte & 0x80) != 0)
		{
			//straight copy
			dst[r_dstPos] = srcData[r_srcPos];
			r_dstPos++;
			r_srcPos++;
			//if(r_srcPos >= srcSize)
			//	return r;
		}
		else
		{
			u32 i;
			//RLE part
			u8 byte1 = counts[countPos];
			u8 byte2 = counts[countPos + 1];
			countPos += 2;
			//if(r_srcPos >= srcSize)
			//	return r;
			
			u32 dist = ((byte1 & 0xF) << 8) | byte2;
			u32 copySource = r_dstPos - (dist + 1);

			u32 numBytes = byte1 >> 4;
			if(numBytes == 0)
			{
				numBytes = srcData[r_srcPos] + 0x12;
				r_srcPos++;
				//if(r_srcPos >= srcSize)
				//	return r;
			}
			else
				numBytes += 2;

			//copy run
			for( i = 0; i < numBytes; ++i)
			{
				dst[r_dstPos] = dst[copySource];
				copySource++;
				r_dstPos++;
			}
		}
		
		//use next bit from "code" byte
		currCodeByte <<= 1;
		validBitCount-=1;		
	}

	return r_dstPos;
}

u32 Yay_decodeAll(const u8 *src, int srcSize, u8 *Dst)
{
	u32 decodedSize = Yay_doDWORD(*(u32*)(src + 4));
	u32 countOffset = Yay_doDWORD(*(u32*)(src + 8));
	u32 dataOffset = Yay_doDWORD(*(u32*)(src + 12));

	return decodeYay(src + 16, src + countOffset, 
		src + dataOffset, Dst, decodedSize);
	
	(void)srcSize; /* unused */
}
