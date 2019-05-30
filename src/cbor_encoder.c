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
#include <math.h>

static size_t __cbor_create_integer(bool positive, int64_t val, uint8_t *buf)
{
	uint64_t uv = (uint64_t)val;
	if (!positive && (val & 0x8000000000000000))
	{
		uv = ~uv;
		buf[0] = IB_NEGINT;
	}
	else
	{
		buf[0] = IB_UINT;
	}

	if (uv <= 23)
	{
		buf[0] |= uv;
		return 1;
	}
	else if (uv <= 255)
	{
		buf[0] |= AI_1;
		buf[1] = uv;
		return 2;
	}
	else if (uv <= 65535)
	{
		buf[0] |= AI_2;
		stonb((uint16_t)uv, buf + 1);
		return 3;
	}
	else if (uv <= 4294967295)
	{
		buf[0] |= AI_4;
		ltonb((uint32_t)uv, buf + 1);
		return 5;
	}
	else
	{
		buf[0] |= AI_8;
		lltonb(uv, buf + 1);
		return 9;
	}
}

size_t cbor_create_int(int64_t val, uint8_t *buf)
{
	return __cbor_create_integer(false, val, buf);
}

size_t cbor_create_uint(uint64_t val, uint8_t *buf)
{
	return __cbor_create_integer(true, val, buf);
}

static size_t __cbor_create_bytes(uint8_t ib_mt, const uint8_t *bytes, uint8_t *buf, size_t len)
{
	if (len <= 23)
	{
		buf[0] = ib_mt | len;
		memcpy(buf + 1, bytes, len);
		return len + 1;
	}
	else if (len <= 255)
	{
		buf[0] = ib_mt | AI_1;
		buf[1] = len;
		memcpy(buf + 2, bytes, len);
		return len + 2;
	}
	else if (len <= 65535)
	{
		buf[0] = ib_mt | AI_2;
		stonb(len, buf + 1);
		memcpy(buf + 3, bytes, len);
		return len + 3;
	}
	else if (len <= 4294967295)
	{
		buf[0] = ib_mt | AI_4;
		ltonb(len, buf + 1);
		memcpy(buf + 5, bytes, len);
		return len + 5;
	}
	else
	{
		buf[0] = ib_mt | AI_8;
		lltonb(len, buf + 1);
		memcpy(buf + 9, bytes, len);
		return len + 9;
	}
}

size_t cbor_create_bytes(const uint8_t *bytes, uint8_t *buf, size_t len)
{
	return __cbor_create_bytes(IB_BYTES, bytes, buf, len);
}

size_t cbor_create_string(const char *str, uint8_t *buf)
{
	return __cbor_create_bytes(IB_STRING, (const uint8_t *)str, buf, strlen(str));
}

static size_t __cbor_create_set(uint8_t ib_mt, size_t len, uint8_t *buf)
{
	if (len <= 23)
	{
		buf[0] = ib_mt | len;
		return 1;
	}
	else if (len <= 255)
	{
		buf[0] = ib_mt | AI_1;
		buf[1] = len;
		return 2;
	}
	else if (len <= 65535)
	{
		buf[0] = ib_mt | AI_2;
		stonb(len, buf + 1);
		return 3;
	}
	else if (len <= 4294967295)
	{
		buf[0] = ib_mt | AI_4;
		stonb(len, buf + 1);
		return 5;
	}
	else
	{
		buf[0] = ib_mt | AI_8;
		stonb(len, buf + 1);
		return 9;
	}
}

size_t cbor_create_array(size_t len, uint8_t *buf)
{
	return __cbor_create_set(IB_ARRAY, len, buf);
}

size_t cbor_create_map(size_t len, uint8_t *buf)
{
	return __cbor_create_set(IB_MAP, len, buf);
}

static size_t __cbor_begin_set(uint8_t ib, uint8_t *buf)
{
	buf[0] = ib | AI_INDEF;
	return 1;
}

static size_t __cbor_end_set(uint8_t *buf)
{
	buf[0] = AI_BRKCD;
	return 1;
}

size_t cbor_begin_array(uint8_t *buf)
{
	return __cbor_begin_set(IB_ARRAY, buf);
}

size_t cbor_end_array(uint8_t *buf)
{
	return __cbor_end_set(buf);
}

size_t cbor_begin_map(uint8_t *buf)
{
	return __cbor_begin_set(IB_MAP, buf);
}

size_t cbor_end_map(uint8_t *buf)
{
	return __cbor_end_set(buf);
}

size_t cbor_create_float(double val, uint8_t *buf)
{
	if (val == 0.0)									// 0.0, -0.0
	{
		buf[0] = IB_PRIM | AI_2;
		buf[1] = 0x00;
		buf[2] = 0x00;
		return 3;
	}
	else if (!isfinite(val))						// Infinity
	{
		buf[0] = IB_PRIM | AI_2;
		buf[1] = 0x7c;
		buf[2] = 0x00;
		return 3;
	}
	else if (isnan(val))							// NaN
	{
		buf[0] = IB_PRIM | AI_2;
		buf[1] = 0x7e;
		buf[2] = 0x00;
		return 3;
	}
	
	float fval = (float)val;
	if (fval != val)								// double
	{
		buf[0] = IB_PRIM | AI_8;
		dtonb(val, buf + 1);
		return 9;
	}
	else if (is_ftoh_loss(fval))					// float
	{
		buf[0] = IB_PRIM | AI_4;
		ftonb(fval, buf + 1);
		return 5;
	}
	else											// float16
	{
		half hval = ftoh(fval);
		buf[0] = IB_PRIM | AI_2;
		htonb(hval, buf + 1);
		return 3;
	}
}

size_t cbor_create_simple(uint8_t val, uint8_t *buf)
{
	if (val <= 23 || val == AI_INDEF)
	{
		buf[0] = IB_PRIM | val;
		return 1;
	}
	else if (val >=32)
	{
		buf[0] = IB_PRIM | AI_1;
		buf[1] = val;
		return 2;
	}
	return 0;
}
