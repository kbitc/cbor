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
#include "endian.h"
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IB_UINT												0
#define IB_NEGINT											(1 << 5)
#define IB_BYTES											(2 << 5)
#define IB_STRING											(3 << 5)
#define IB_ARRAY											(4 << 5)
#define IB_MAP												(5 << 5)
#define IB_TAG												(6 << 5)
#define IB_PRIM												(7 << 5)

#define AI_1												24
#define AI_2												25
#define AI_4												26
#define AI_8												27
#define AI_INDEF											31
#define AI_BRKCD											0xFF

#define CBOR_NO_ERROR										0
#define CBOR_ERR_OUT_OF_DATA								1
#define CBOR_ERR_NOT_ALL_DATA_CONSUMED						2
#define CBOR_ERR_ODD_SIZE_INDEF_MAP							3
#define CBOR_ERR_BREAK_OUTSIDE_INDEF						4
#define CBOR_ERR_RESERVED_AI								5
#define CBOR_ERR_BYTES_TEXT_MISMATCH						6
#define CBOR_ERR_OUT_OF_MEMORY								7

typedef struct 
{
	uint8_t *buf;
	size_t offset;
	size_t size;
} cbor_t;


const char *cbor_get_error(int errno);

int cbor_verify(cbor_t *cbor);


cbor_t *cbor_create(size_t size);
void cbor_free(cbor_t *cbor);

bool cbor_set_int(cbor_t *cbor, int64_t val);
bool cbor_set_uint(cbor_t *cbor, uint64_t val);

bool cbor_set_bytes(cbor_t *cbor, const uint8_t *bytes, size_t len);
bool cbor_begin_bytes(cbor_t *cbor);
bool cbor_end_bytes(cbor_t *cbor);

bool cbor_set_string(cbor_t *cbor, const char *str);
bool cbor_begin_string(cbor_t *cbor);
bool cbor_end_string(cbor_t *cbor);

bool cbor_set_array(cbor_t *cbor, size_t len);
bool cbor_begin_array(cbor_t *cbor);
bool cbor_end_array(cbor_t *cbor);

bool cbor_set_map(cbor_t *cbor, size_t len);
bool cbor_begin_map(cbor_t *cbor);
bool cbor_end_map(cbor_t *cbor);

bool cbor_set_float(cbor_t *cbor, double val);
bool cbor_set_simple(cbor_t *cbor, uint8_t val);

/*
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
*/

#ifdef __cplusplus
}
#endif

#endif  /* CBOR_H */
