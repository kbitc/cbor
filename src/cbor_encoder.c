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
#include <stdlib.h>

cbor_t *cbor_create(size_t size)
{
	cbor_t *cbor = (cbor_t *)malloc(sizeof(cbor_t));
	if (cbor != NULL)
	{
		cbor->buf = (uint8_t *)malloc(sizeof(uint8_t) * size);
		if (cbor->buf == NULL)
		{
			free(cbor);
			cbor = NULL;
		}
		else
		{
			cbor->offset = 0;
			cbor->size = size;
		}
	}
	return cbor;
}

void cbor_free(cbor_t *cbor)
{
	if (cbor != NULL)
	{
		if (cbor->buf != NULL)
		{
			free(cbor->buf);
		}
		free(cbor);
	}
}

__attribute__((weak)) bool ensure_capacity(cbor_t *cbor, size_t size)
{
	return cbor->offset + size <= cbor->size;
}

static bool __cbor_create_int(cbor_t *cbor, int64_t val, bool offsetitive)
{
	uint64_t uv = (uint64_t)val;
	uint8_t ib_mt = IB_UINT;
	if (!offsetitive && (val & 0x8000000000000000))
	{
		uv = ~uv;
		ib_mt = IB_NEGINT;
	}

	if (uv <= 23)
	{
		if (ensure_capacity(cbor, 1))
		{
			cbor->buf[cbor->offset++] = ib_mt | uv;
			return true;
		}
	}
	else if (uv <= 255)
	{
		if (ensure_capacity(cbor, 2))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_1;
			cbor->buf[cbor->offset++] = uv;
			return true;
		}
	}
	else if (uv <= 65535)
	{
		if (ensure_capacity(cbor, 3))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_2;
			stonb((uint16_t)uv, cbor->buf + cbor->offset);
			cbor->offset += 2;
			return true;
		}
	}
	else if (uv <= 4294967295)
	{
		if (ensure_capacity(cbor, 5))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_4;
			ltonb((uint32_t)uv, cbor->buf + cbor->offset);
			cbor->offset += 4;
			return true;
		}
	}
	else
	{
		if (ensure_capacity(cbor, 9))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_8;
			lltonb(uv, cbor->buf + cbor->offset);
			cbor->offset += 8;
			return true;
		}
	}
	return false;
}

bool cbor_set_int(cbor_t *cbor, int64_t val)
{
	return __cbor_create_int(cbor, val, false);
}

bool cbor_set_uint(cbor_t *cbor, uint64_t val)
{
	return __cbor_create_int(cbor, val, true);
}


static bool __cbor_create_bytes(cbor_t *cbor, const uint8_t *bytes, size_t len, uint8_t ib_mt)
{
	if (len <= 23)
	{
		if (ensure_capacity(cbor, 1 + len))
		{
			cbor->buf[cbor->offset++] = ib_mt | len;
			memcpy(cbor->buf + cbor->offset, bytes, len);
			cbor->offset += len;
			return true;
		}
	}
	else if (len <= 255)
	{
		if (ensure_capacity(cbor, 2 + len))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_1;
			cbor->buf[cbor->offset++] = len;
			memcpy(cbor->buf + cbor->offset, bytes, len);
			cbor->offset += len;
			return true;
		}
	}
	else if (len <= 65535)
	{
		if (ensure_capacity(cbor, 3 + len))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_2;
			stonb(len, cbor->buf + cbor->offset);
			cbor->offset += 2;
			memcpy(cbor->buf + cbor->offset, bytes, len);
			cbor->offset += len;
			return true;
		}
	}
	else if (len <= 4294967295)
	{
		if (ensure_capacity(cbor, 5 + len))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_4;
			ltonb(len, cbor->buf + cbor->offset);
			cbor->offset += 4;
			memcpy(cbor->buf + cbor->offset, bytes, len);
			cbor->offset += len;
			return true;
		}
	}
	else
	{
		if (ensure_capacity(cbor, 9 + len))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_8;
			lltonb(len, cbor->buf + cbor->offset);
			cbor->offset += 8;
			memcpy(cbor->buf + cbor->offset, bytes, len);
			cbor->offset += len;
			return true;
		}
	}
	return false;
}

bool cbor_set_bytes(cbor_t *cbor, const uint8_t *bytes, size_t len)
{
	return __cbor_create_bytes(cbor, bytes, len, IB_BYTES);
}

bool cbor_set_string(cbor_t *cbor, const char *str)
{
	return __cbor_create_bytes(cbor, (const uint8_t *)str, strlen(str), IB_STRING);
}


bool cbor_begin_bytes(cbor_t *cbor)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = IB_BYTES | AI_INDEF;
		return true;
	}
	return false;
}

bool cbor_end_bytes(cbor_t *cbor)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = AI_BRKCD;
		return true;
	}
	return false;
}

bool cbor_begin_string(cbor_t *cbor)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = IB_STRING | AI_INDEF;
		return true;
	}
	return false;
}

bool cbor_end_string(cbor_t *cbor)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = AI_BRKCD;
		return true;
	}
	return false;
}


static bool __cbor_create_set(cbor_t *cbor, size_t len, uint8_t ib_mt)
{
	if (len <= 23)
	{
		if (ensure_capacity(cbor, 1))
		{
			cbor->buf[cbor->offset++] = ib_mt | len;
			return true;
		}
	}
	else if (len <= 255)
	{
		if (ensure_capacity(cbor, 2))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_1;
			cbor->buf[cbor->offset++] = len;
			return true;
		}
	}
	else if (len <= 65535)
	{
		if (ensure_capacity(cbor, 3))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_2;
			stonb(len, cbor->buf + cbor->offset);
			cbor->offset += 2;
			return true;
		}
	}
	else if (len <= 4294967295)
	{
		if (ensure_capacity(cbor, 5))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_4;
			ltonb(len, cbor->buf + cbor->offset);
			cbor->offset += 4;
			return true;
		}
	}
	else
	{
		if (ensure_capacity(cbor, 9))
		{
			cbor->buf[cbor->offset++] = ib_mt | AI_8;
			lltonb(len, cbor->buf + cbor->offset);
			cbor->offset += 8;
			return true;
		}
	}
	return false;
}

bool cbor_set_array(cbor_t *cbor, size_t len)
{
	return __cbor_create_set(cbor, len, IB_ARRAY);
}

bool cbor_set_map(cbor_t *cbor, size_t len)
{
	return __cbor_create_set(cbor, len, IB_MAP);
}

static bool __cbor_begin_set(cbor_t *cbor, uint8_t ib_mt)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = ib_mt | AI_INDEF;
		return true;
	}
	return false;
}

static bool __cbor_end_set(cbor_t *cbor)
{
	if (ensure_capacity(cbor, 1))
	{
		cbor->buf[cbor->offset++] = AI_BRKCD;
		return true;
	}
	return false;
}

bool cbor_begin_array(cbor_t *cbor)
{
	return __cbor_begin_set(cbor, IB_ARRAY);
}

bool cbor_end_array(cbor_t *cbor)
{
	return __cbor_end_set(cbor);
}

bool cbor_begin_map(cbor_t *cbor)
{
	return __cbor_begin_set(cbor, IB_MAP);
}

bool cbor_end_map(cbor_t *cbor)
{
	return __cbor_end_set(cbor);
}


bool cbor_set_float(cbor_t *cbor, double val)
{
	if (val == 0.0)									// 0.0, -0.0
	{
		if (ensure_capacity(cbor, 3))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_2;
			cbor->buf[cbor->offset++] = 0x00;
			cbor->buf[cbor->offset++] = 0x00;
			return true;
		}
	}
	else if (!isfinite(val))						// Infinity
	{
		if (ensure_capacity(cbor, 3))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_2;
			cbor->buf[cbor->offset++] = 0x7c;
			cbor->buf[cbor->offset++] = 0x00;
			return true;
		}
	}
	else if (isnan(val))							// NaN
	{
		if (ensure_capacity(cbor, 3))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_2;
			cbor->buf[cbor->offset++] = 0x7e;
			cbor->buf[cbor->offset++] = 0x00;
			return true;
		}
	}
	
	float fval = (float)val;
	if (fval != val)								// double
	{
		if (ensure_capacity(cbor, 9))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_8;
			dtonb(val, cbor->buf + cbor->offset);
			cbor->offset += 8;
			return true;
		}
	}
	else if (is_ftoh_loss(fval))					// float
	{
		if (ensure_capacity(cbor, 5))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_4;
			ftonb(fval, cbor->buf + cbor->offset);
			cbor->offset += 4;
			return true;
		}
	}
	else											// float16
	{
		if (ensure_capacity(cbor, 3))
		{
			half hval = ftoh(fval);
			cbor->buf[cbor->offset++] = IB_PRIM | AI_2;
			htonb(hval, cbor->buf + cbor->offset);
			cbor->offset += 2;
			return true;
		}
	}

	return false;
}


bool cbor_set_simple(cbor_t *cbor, uint8_t val)
{
	if (val <= 23 || val == AI_INDEF)
	{
		if (ensure_capacity(cbor, 1))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | val;
			return true;
		}
	}
	else if (val >=32)
	{
		if (ensure_capacity(cbor, 2))
		{
			cbor->buf[cbor->offset++] = IB_PRIM | AI_1;
			cbor->buf[cbor->offset++] = val;
			return true;
		}
	}
	return false;
}
