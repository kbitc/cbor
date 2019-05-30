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

#ifndef CBOR_H
#define CBOR_H

#include "fp16.h"
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t cbor_create_int(int64_t val, uint8_t *buf);
size_t cbor_create_uint(uint64_t val, uint8_t *buf);

size_t cbor_create_bytes(const uint8_t *bytes, uint8_t *buf, size_t len);
size_t cbor_create_string(const char *str, uint8_t *buf);

size_t cbor_create_array(size_t len, uint8_t *buf);
size_t cbor_create_map(size_t len, uint8_t *buf);

size_t cbor_begin_array(uint8_t *buf);
size_t cbor_end_array(uint8_t *buf);

size_t cbor_begin_map(uint8_t *buf);
size_t cbor_end_map(uint8_t *buf);

size_t cbor_create_float(double val, uint8_t *buf);
size_t cbor_create_simple(uint8_t val, uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif  /* CBOR_H */
