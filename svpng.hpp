/*
Copyright (C) 2017 Milo Yip. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of pngout nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*! \file
    \brief      svpng() is a minimalistic C function for saving RGB/RGBA image
    into uncompressed PNG.
    \author     Milo Yip
    \version    0.1.1
    \copyright  MIT license
    \sa         http://github.com/miloyip/svpng
*/

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*!
    \brief Save a RGB/RGBA image in PNG format.
    \param filename Output file name.
    \param w Width of the image. (<16383)
    \param h Height of the image.
    \param img Image pixel data in 24-bit RGB or 32-bit RGBA format.
    \param alpha Whether the image contains alpha channel.
*/
inline void svpng(char const* filename, unsigned w, unsigned h,
                  void const* img_, bool alpha) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "svpng: cannot open file %s\n", filename);
    return;
  }
  const unsigned char* img = (const unsigned char*)img_;
  /* CRC32 Table */
  static const uint_fast32_t t[] = {
      0,          0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
      0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
      0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};
  /* ADLER-a, ADLER-b, CRC, pitch */
  uint_fast32_t a = 1, b = 0, c, p = w * (alpha ? 4 : 3) + 1, x, y, i;
#define SVPNG_U8A(ua, l) \
  for (i = 0; i < l; i++) SVPNG_PUT((ua)[i]);
#define SVPNG_U32(u)              \
  do {                            \
    SVPNG_PUT((u) >> 24);         \
    SVPNG_PUT(((u) >> 16) & 255); \
    SVPNG_PUT(((u) >> 8) & 255);  \
    SVPNG_PUT((u)&255);           \
  } while (0)
#define SVPNG_U8C(u)          \
  do {                        \
    SVPNG_PUT(u);             \
    c ^= (u);                 \
    c = (c >> 4) ^ t[c & 15]; \
    c = (c >> 4) ^ t[c & 15]; \
  } while (0)
#define SVPNG_U8AC(ua, l) \
  for (i = 0; i < l; i++) SVPNG_U8C((ua)[i])
#define SVPNG_U16LC(u)           \
  do {                           \
    SVPNG_U8C((u)&255);          \
    SVPNG_U8C(((u) >> 8) & 255); \
  } while (0)
#define SVPNG_U32C(u)             \
  do {                            \
    SVPNG_U8C((u) >> 24);         \
    SVPNG_U8C(((u) >> 16) & 255); \
    SVPNG_U8C(((u) >> 8) & 255);  \
    SVPNG_U8C((u)&255);           \
  } while (0)
#define SVPNG_U8ADLER(u)   \
  do {                     \
    SVPNG_U8C(u);          \
    a = (a + (u)) % 65521; \
    b = (b + a) % 65521;   \
  } while (0)
#define SVPNG_BEGIN(s, l) \
  do {                    \
    SVPNG_U32(l);         \
    c = ~0U;              \
    SVPNG_U8AC(s, 4);     \
  } while (0)
#define SVPNG_PUT(u) fputc(u, fp)
#define SVPNG_END() SVPNG_U32(~c)
  SVPNG_U8A("\x89PNG\r\n\32\n", 8); /* Magic */
  SVPNG_BEGIN("IHDR", 13);          /* IHDR chunk { */
  SVPNG_U32C(w);
  SVPNG_U32C(h); /*   Width & Height (8 bytes) */
  SVPNG_U8C(8);
  /*   Depth=8, Color=True color with/without alpha (2 bytes) */
  SVPNG_U8C(alpha ? 6 : 2);
  /*   Compression=Deflate, Filter=No, Interlace=No (3 bytes) */
  SVPNG_U8AC("\0\0\0", 3);
  SVPNG_END();                              /* } */
  SVPNG_BEGIN("IDAT", 2 + h * (5 + p) + 4); /* IDAT chunk { */
  SVPNG_U8AC("\x78\1", 2); /*   Deflate block begin (2 bytes) */
  for (y = 0; y < h; y++) {
    /*   Each horizontal line makes a block for simplicity */
    SVPNG_U8C(y == h - 1); /*   1 for the last block, 0 for others (1 byte) */
    SVPNG_U16LC(p);
    SVPNG_U16LC(~p);
    /*   Size of block in little endian and its 1's complement (4 bytes) */
    SVPNG_U8ADLER(0); /*   No filter prefix (1 byte) */
    for (x = 0; x < p - 1; x++, img++)
      SVPNG_U8ADLER(*img); /*   Image pixel data */
  }
  SVPNG_U32C((b << 16) | a); /*   Deflate block end with adler (4 bytes) */
  SVPNG_END();               /* } */
  SVPNG_BEGIN("IEND", 0);
  SVPNG_END(); /* IEND chunk {} */
  fclose(fp);
  printf("svpng: image saved in file \"%s\"\n", filename);
}
