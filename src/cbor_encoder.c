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

#include "cbor.h"
#include "fp16.h"
#include "endian.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

bool ensure_capacity(const uint8_t *buf, size_t size, size_t offset)
{
	return size >= offset;
}

static int __cbor_encode_uint(uint8_t *buf, size_t size, size_t *pos, uint8_t ib_mt, uint64_t val)
{
	size_t len = (val <= 23) ? 0 \
		: (val <= 255) ? 1 \
		: (val <= 65535) ? 2 \
		: (val <= 4294967295) ? 4 \
		: 8;
	if (!ensure_capacity(buf, size, *pos + len + 1))
	{
		return CBOR_ERR_OUT_OF_MEMORY;
	}

	if (len == 0)
	{
		buf[(*pos)++] = ib_mt | val;
	}
	else if (len == 1)
	{
		buf[(*pos)++] = ib_mt | AI_1;
		buf[*pos] = val;
	}
	else if (len == 2)
	{
		buf[(*pos)++] = ib_mt | AI_2;
		stonb((uint16_t)val, buf + *pos);
	}
	else if (len == 4)
	{
		buf[(*pos)++] = ib_mt | AI_4;
		ltonb((uint32_t)val, buf + *pos);
	}
	else // if (len == 8)
	{
		buf[(*pos)++] = ib_mt | AI_8;
		lltonb(val, buf + *pos);
	}

	*pos += len;
	return CBOR_NO_ERROR;
}

static int __cbor_encode_bytes(uint8_t *buf, size_t size, size_t *pos, uint8_t ib_mt, const void *bytes, size_t len)
{
	int ret = __cbor_encode_uint(buf, size, pos, ib_mt, len);
	if (ret != CBOR_NO_ERROR)
	{
		return ret;
	}

	if (ensure_capacity(buf, size, *pos + len))
	{
		memcpy(buf + *pos, bytes, len);
		*pos += len;
	}
	return CBOR_ERR_OUT_OF_MEMORY;
}

static int __cbor_encode_indef(uint8_t *buf, size_t size, size_t *pos, uint8_t ib_mt)
{
	if (ensure_capacity(buf, size, *pos + 2))
	{
		buf[(*pos)++] = ib_mt | AI_INDEF;
		return CBOR_NO_ERROR;
	}
	return CBOR_ERR_OUT_OF_MEMORY;
}

// 1. encode signed integer
int cbor_encode_int(uint8_t *buf, size_t size, size_t *pos, int64_t val)
{
	return (val & 0x8000000000000000) \
		? __cbor_encode_uint(buf, size, pos, IB_NEGINT, ~val) \
		: __cbor_encode_uint(buf, size, pos, IB_UINT, val);
}

// 2. encode unsigned integer
int cbor_encode_uint(uint8_t *buf, size_t size, size_t *pos, uint64_t val)
{
	return __cbor_encode_uint(buf, size, pos, IB_UINT, val);
}

// 3. encode tag
int cbor_encode_tag(uint8_t *buf, size_t size, size_t *pos, uint64_t val)
{
	return __cbor_encode_uint(buf, size, pos, IB_TAG, val);
}

// 4. encode simple
int cbor_encode_simple(uint8_t *buf, size_t size, size_t *pos, uint8_t val)
{
	if (val > 23 && val < 32)
	{
		return CBOR_ERR_SIMPLE_OUT_OF_SCOPE;
	}
	return __cbor_encode_uint(buf, size, pos, IB_PRIM, val);
}

// 5. encode double, float and half
int cbor_encode_float(uint8_t *buf, size_t size, size_t *pos, double val)
{
	if (!ensure_capacity(buf, size, *pos + 3))
	{
		return CBOR_ERR_OUT_OF_MEMORY;
	}

	if (val == 0.0)									// 0.0, -0.0
	{
		buf[(*pos)++] = IB_PRIM | AI_2;
		stonb((*((uint64_t *)&val) & 0x8000000000000000) ? 0x8000 : 0x0000, buf + *pos);
		*pos += 2;
	}
	else if (isnan(val))							// NaN
	{
		buf[(*pos)++] = IB_PRIM | AI_2;
		stonb(0x7e00, buf + *pos);
		*pos += 2;
	}
	else if (!isfinite(val))						// Infinity
	{
		buf[(*pos)++] = IB_PRIM | AI_2;
		stonb((*((uint64_t *)&val) & 0x8000000000000000) ? 0xfc00 : 0x7c00, buf + *pos);
		*pos += 2;
	}
	else
	{
		float fval = (float)val;
		if (fval != val)								// double
		{
			if (!ensure_capacity(buf, size, *pos + 9))
			{
				return CBOR_ERR_OUT_OF_MEMORY;
			}

			buf[(*pos)++] = IB_PRIM | AI_8;
			dtonb(val, buf + *pos);
			*pos += 8;
		}
		else if (is_ftoh_loss(fval))					// float
		{
			if (!ensure_capacity(buf, size, *pos + 5))
			{
				return CBOR_ERR_OUT_OF_MEMORY;
			}

			buf[(*pos)++] = IB_PRIM | AI_4;
			ftonb(fval, buf + *pos);
			*pos += 4;
		}
		else											// float16
		{
			half hval = ftoh(fval);
			buf[(*pos)++] = IB_PRIM | AI_2;
			htonb(hval, buf + *pos);
			*pos += 2;
		}
	}
	return CBOR_NO_ERROR;
}

// 6. encode bytes
int cbor_encode_bytes(uint8_t *buf, size_t size, size_t *pos, const uint8_t *bytes, size_t len)
{
	return __cbor_encode_bytes(buf, size, pos, IB_BYTES, bytes, len);
}

// 7. encode indefinite-length bytes
int cbor_encode_bytes_indef(uint8_t *buf, size_t size, size_t *pos)
{
	return __cbor_encode_indef(buf, size, pos, IB_BYTES);
}

// 8. encode string
int cbor_encode_string(uint8_t *buf, size_t size, size_t *pos, const char *str)
{
	return __cbor_encode_bytes(buf, size, pos, IB_STRING, str, strlen(str));
}

// 9. encode indefinite-length string
int cbor_encode_string_indef(uint8_t *buf, size_t size, size_t *pos)
{
	return __cbor_encode_indef(buf, size, pos, IB_STRING);
}

// 10. encode array
int cbor_encode_array(uint8_t *buf, size_t size, size_t *pos, size_t len)
{
	return __cbor_encode_uint(buf, size, pos, IB_ARRAY, len);
}

// 11. encode indefinite-length array
int cbor_encode_array_indef(uint8_t *buf, size_t size, size_t *pos)
{
	return __cbor_encode_indef(buf, size, pos, IB_ARRAY);
}

// 12. encode map
int cbor_encode_map(uint8_t *buf, size_t size, size_t *pos, size_t len)
{
	return __cbor_encode_uint(buf, size, pos, IB_MAP, len << 1);
}

// 13. encode indefinite-length map
int cbor_encode_map_indef(uint8_t *buf, size_t size, size_t *pos)
{
	return __cbor_encode_indef(buf, size, pos, IB_MAP);
}

// 14. encode break code
int cbor_encode_break(uint8_t *buf, size_t size, size_t *pos)
{
	if (ensure_capacity(buf, size, *pos + 1))
	{
		buf[(*pos)++] = AI_BRKCD;
		return CBOR_NO_ERROR;
	}
	return CBOR_ERR_OUT_OF_MEMORY;
}
