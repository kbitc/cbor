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
#define CBOR_ERR_MT_UNDEF_FOR_INDEF							5
#define CBOR_ERR_RESERVED_AI								6
#define CBOR_ERR_BYTES_TEXT_MISMATCH						7
#define CBOR_ERR_OUT_OF_MEMORY								8
#define CBOR_ERR_SIMPLE_OUT_OF_SCOPE						9

typedef enum
{
	/** false */
	CBOR_FALSE,
	/** true */
	CBOR_TRUE,
	/** null */
	CBOR_NULL,
	/** undefined */
	CBOR_UNDEFINED,
	/** Positive integers */
	CBOR_UINT,
	/** Negative integers */
	CBOR_NEGINT,
	/** Byte string */
	CBOR_BYTES,
	/** UTF-8 string */
	CBOR_STRING,
	/** Byte string, in chunks */
	CBOR_BYTES_INDEF,
	/** UTF-8 string, in chunks */
	CBOR_STRING_INDEF,
	/** Array of CBOR values */
	CBOR_ARRAY,
	/** Map of key/value pairs */
	CBOR_MAP,
	/** Tag describing the next value.  The next value is the single child. */
	CBOR_TAG,
	/** Simple value, other than the defined ones */
	CBOR_SIMPLE,
	/** Doubles, floats, and half-floats */
	CBOR_DOUBLE,
	/** Floats, and half-floats */
	CBOR_FLOAT
} cbor_type;

typedef struct _cbor_t
{
	cbor_type ct;
	union
	{
		const uint8_t *bytes;
		const char *str;
		int64_t sint;
		uint64_t uint;
		double dbl;
		float flt;
	} v;
	size_t size;
	struct _cbor_t *next;
} cbor_t;


const char *cbor_get_error(int errno);

int cbor_verify(uint8_t *buf, size_t size, size_t *pos);
int cbor_verify_tag(uint8_t *buf, size_t size, size_t *pos, uint64_t tag);
int cbor_well_formed(uint8_t *buf, size_t size, size_t *err_pos);

int cbor_decode(uint8_t *buf, size_t size, size_t *pos, cbor_t *cbor);
int cbor_decode_tag(uint8_t *buf, size_t size, size_t *pos, uint64_t tag, cbor_t *cbor);

// 0. ensure buffer capacity
bool ensure_capacity(uint8_t *buf, size_t size, size_t offset);
// 1. encode signed integer
int cbor_encode_int(uint8_t *buf, size_t size, size_t *pos, int64_t val);
// 2. encode unsigned integer
int cbor_encode_uint(uint8_t *buf, size_t size, size_t *pos, uint64_t val);
// 3. encode tag
int cbor_encode_tag(uint8_t *buf, size_t size, size_t *pos, uint64_t val);
// 4. encode simple
int cbor_encode_simple(uint8_t *buf, size_t size, size_t *pos, uint8_t val);
// 5. encode double, float and half
int cbor_encode_float(uint8_t *buf, size_t size, size_t *pos, double val);
// 6. encode bytes
int cbor_encode_bytes(uint8_t *buf, size_t size, size_t *pos, const uint8_t *bytes, size_t len);
// 7. encode indefinite-length bytes
int cbor_encode_bytes_indef(uint8_t *buf, size_t size, size_t *pos);
// 8. encode string
int cbor_encode_string(uint8_t *buf, size_t size, size_t *pos, const char *str);
// 9. encode indefinite-length string
int cbor_encode_string_indef(uint8_t *buf, size_t size, size_t *pos);
// 10. encode array
int cbor_encode_array(uint8_t *buf, size_t size, size_t *pos, size_t len);
// 11. encode indefinite-length array
int cbor_encode_array_indef(uint8_t *buf, size_t size, size_t *pos, size_t len);
// 12. encode map
int cbor_encode_map(uint8_t *buf, size_t size, size_t *pos, size_t len);
// 13. encode indefinite-length map
int cbor_encode_map_indef(uint8_t *buf, size_t size, size_t *pos, size_t len);
// 14. encode break code
int cbor_encode_break(uint8_t *buf, size_t size, size_t *pos, const uint8_t *bytes, size_t len);

#ifdef __cplusplus
}
#endif

#endif  /* CBOR_H */
