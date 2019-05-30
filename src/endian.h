/****************************************************************************
**
** Copyright (C) 2019 King Brain Infotech Co., Ltd.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#ifndef ENDIAN_H
#define ENDIAN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ntohs(n)				endian_swap16(n)
#define ntohl(n)				endian_swap32(n)
#define ntohll(n)				endian_swap64(n)

#define htons(n)				endian_swap16(n)
#define htonl(n)				endian_swap32(n)
#define htonll(n)				endian_swap64(n)


uint16_t endian_swap16(uint16_t n);
uint32_t endian_swap32(uint32_t n);
uint64_t endian_swap64(uint64_t n);

#ifdef __cplusplus
}
#endif

#endif  /* ENDIAN_H */
