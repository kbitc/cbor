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

int cbor_decode(const uint8_t *buf, size_t size, size_t *pos, cbor_t *cbor)
{
	if (buf[*pos] == AI_BRKCD)
	{
		return CBOR_ERR_BREAK_OUTSIDE_INDEF;
	}

	uint8_t ib_mt = buf[*pos] & 0xe0;
	uint8_t ib_ai = buf[*pos] & 0x1f;
	if (ib_ai < 28)
	{
		size_t len = (ib_ai == AI_1) ? 1 \
			: (ib_ai == AI_2) ? 2 \
			: (ib_ai == AI_4) ? 4 \
			: (ib_ai == AI_8) ? 8 \
			: 0;
		if (!ensure_capacity(buf, size, *pos + len + 1))
		{
			return CBOR_ERR_OUT_OF_DATA;
		}
		
		++*pos;
		uint64_t val = (len == 1) ? buf[*pos] \
			: (len == 2) ? nbtos(buf + *pos) \
			: (len == 4) ? nbtol(buf + *pos) \
			: (len == 8) ? nbtoll(buf + *pos) \
			: ib_ai;
		*pos += len;

		if (ib_mt == IB_UINT)
		{
			cbor->ct = CBOR_UINT;
			cbor->v.uint = val;
		}
		else if (ib_mt == IB_NEGINT)
		{
			cbor->ct = CBOR_NEGINT;
			cbor->v.sint = (int64_t)~val;
		}
		if (ib_mt == IB_BYTES || ib_mt == IB_STRING)
		{
			if (!ensure_capacity(buf, size, *pos + val))
			{
				return CBOR_ERR_OUT_OF_DATA;
			}

			cbor->ct = ib_mt == IB_BYTES ? CBOR_BYTES : CBOR_STRING;
			cbor->v.bytes = buf + *pos;
			cbor->size = val;

			*pos += val;
		}
		else if (ib_mt == IB_ARRAY || ib_mt == IB_MAP)
		{
			if (ib_mt == IB_MAP && val % 2 == 1)
			{
				return CBOR_ERR_ODD_SIZE_INDEF_MAP;
			}

			size_t _pos = *pos;
			for (uint64_t i = 0; i < val; i++)
			{
				int ret = cbor_verify(buf, size, pos);
				if (ret != CBOR_NO_ERROR)
				{
					return ret;
				}
			}

			cbor->ct = ib_mt == IB_ARRAY ? CBOR_ARRAY : CBOR_MAP;
			cbor->v.bytes = buf + _pos;
			cbor->count = val;
			cbor->size = *pos - _pos;
		}
		else if (ib_mt == IB_TAG)
		{
			cbor->ct = CBOR_TAG;
			cbor->v.uint = val;

			cbor->next = cbor_create();
			if (cbor->next == NULL)
			{
				return CBOR_ERR_OUT_OF_MEMORY;
			}
			
			int ret = cbor_decode_tag(buf, size, pos, val, cbor->next);
			if (ret != CBOR_NO_ERROR)
			{
				cbor_free(cbor->next);
				cbor->next = NULL;
			}
			return ret;
		}
		else // if (ib_mt == IB_PRIM)
		{
			if (ib_ai == 20)
			{
				cbor->ct = CBOR_FALSE;
			}
			else if (ib_ai == 21)
			{
				cbor->ct = CBOR_TRUE;
			}
			else if (ib_ai == 22)
			{
				cbor->ct = CBOR_NULL;
			}
			else if (ib_ai == 23)
			{
				cbor->ct = CBOR_UNDEFINED;
			}
			else if (ib_ai == AI_2)
			{
				cbor->ct = CBOR_FLOAT;
				uint16_t s = (uint16_t)val;
				cbor->v.flt = htof(*((half *)&s));
			}
			else if (ib_ai == AI_4)
			{
				cbor->ct = CBOR_FLOAT;
				uint32_t l = (uint32_t)val;
				cbor->v.flt = *((float *)&l);
			}
			else if (ib_ai == AI_8)
			{
				cbor->ct = CBOR_DOUBLE;
				cbor->v.dbl = *((double *)&val);
			}
			else
			{
				cbor->ct = CBOR_SIMPLE;
				cbor->v.uint = (uint8_t)val;
			}
		}
		return CBOR_NO_ERROR;
	}
	else if (ib_ai < AI_INDEF)
	{
		return CBOR_ERR_RESERVED_AI;
	}
	else // if (ib_ai == AI_INDEF)
	{
		if (ib_mt != IB_BYTES && ib_mt != IB_STRING && ib_mt != IB_ARRAY && ib_mt != IB_MAP)
		{
			return CBOR_ERR_MT_UNDEF_FOR_INDEF;
		}

		if (!ensure_capacity(buf, size, *pos + 2))
		{
			return CBOR_ERR_OUT_OF_DATA;
		}

		size_t _pos = ++*pos;
		size_t count = 0;
		if (ib_mt == IB_BYTES || ib_mt == IB_STRING)
		{
			while (buf[*pos] != AI_BRKCD)
			{
				if ((buf[*pos] & 0xe0) != ib_mt /*|| (buf[*pos] & 0x1f) == AI_INDEF*/)
				{
					return CBOR_ERR_BYTES_TEXT_MISMATCH;
				}

				int ret = cbor_verify(buf, size, pos);
				if (ret != CBOR_NO_ERROR)
				{
					return ret;
				}

				count++;
			}

			cbor->ct = ib_mt == IB_BYTES ? CBOR_BYTES_INDEF : CBOR_STRING_INDEF;
		}
		else // if (ib_mt == IB_ARRAY || ib_mt == IB_MAP)
		{
			while (buf[*pos] != AI_BRKCD)
			{
				int ret = cbor_verify(buf, size, pos);
				if (ret != CBOR_NO_ERROR)
				{
					return ret;
				}

				count++;
			}

			if (ib_mt == IB_MAP && count % 2 == 1)
			{
				return CBOR_ERR_ODD_SIZE_INDEF_MAP;
			}

			cbor->ct = ib_mt == IB_ARRAY ? CBOR_ARRAY : CBOR_MAP;
		}
		cbor->v.bytes = buf + _pos;
		cbor->count = count;
		cbor->size = ++*pos - _pos;

		return CBOR_NO_ERROR;
	}
}
