#ifndef N64_YAY_H_INCLUDED
#define N64_YAY_H_INCLUDED

#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;

u32 Yay_decodeAll(const u8 *src, int srcSize, u8 *Dst);

#endif /* N64_YAY_H_INCLUDED */
