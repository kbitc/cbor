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

static int __cbor_verify(uint8_t *buf, size_t *pos, size_t end)
{
	uint8_t ib_mt = buf[*pos] & 0xe0;
	uint8_t ib_ai = buf[*pos] & 0x1f;
	if (ib_mt == IB_ARRAY || ib_mt == IB_MAP)
	{
		if (ib_ai == AI_INDEF)
		{
			++*pos;

			size_t count = 0;
			while (*pos <= end)
			{
				if (buf[*pos] == AI_BRKCD)
				{
					++*pos;
					if (ib_mt == IB_MAP && count % 2 == 1)
					{
						return CBOR_ERR_ODD_SIZE_INDEF_MAP;
					}
					return CBOR_NO_ERROR;
				}
				
				int ret = __cbor_verify(buf, pos, end);
				if (ret)
				{
					return ret;
				}

				count++;
			}

			return CBOR_ERR_OUT_OF_DATA;
		}
		else
		{
			size_t count = 0;
			if (ib_ai <= 23)
			{
				count = ib_ai;
				++*pos;
			}
			else if (ib_ai == AI_1)
			{
				count = buf[++*pos];
				++*pos;
			}
			else if (ib_ai == AI_2)
			{
				count = nbtos(buf + ++*pos);
				*pos += 2;
			}
			else if (ib_ai == AI_4)
			{
				count = nbtol(buf + ++*pos);
				*pos += 4;
			}
			else if (ib_ai == AI_8)
			{
				count = nbtoll(buf + ++*pos);
				*pos += 8;
			}
			else
			{
				return CBOR_ERR_RESERVED_AI;
			}

			if (ib_mt == IB_MAP)
			{
				count <<= 1;
			}

			for (size_t i = 0; i < count; i++)
			{
				if (buf[*pos] == AI_BRKCD)
				{
					return CBOR_ERR_BREAK_OUTSIDE_INDEF;
				}

				int ret = __cbor_verify(buf, pos, end);
				if (ret)
				{
					return ret;
				}
			}

			return CBOR_NO_ERROR;
		}
	}
	else if (ib_mt == IB_BYTES || ib_mt == IB_STRING)
	{
		if (ib_ai == AI_INDEF)
		{
			++*pos;

			while (*pos <= end)
			{
				if (buf[*pos] == AI_BRKCD)
				{
					++*pos;
					return CBOR_NO_ERROR;
				}

				uint8_t sub_ib_mt = buf[*pos] & 0xe0;
				if (sub_ib_mt != ib_mt)
				{
					return CBOR_ERR_BYTES_TEXT_MISMATCH;
				}
				
				int ret = __cbor_verify(buf, pos, end);
				if (ret)
				{
					return ret;
				}
			}

			return CBOR_ERR_OUT_OF_DATA;
		}
		else
		{
			if (ib_ai <= 23)
			{
				*pos += ib_ai + 1;
			}
			else if (ib_ai == AI_1)
			{
				size_t len = buf[++*pos];
				*pos += len + 1;
			}
			else if (ib_ai == AI_2)
			{
				size_t len = nbtos(buf + ++*pos);
				*pos += len + 2;
			}
			else if (ib_ai == AI_4)
			{
				size_t len = nbtol(buf + ++*pos);
				*pos += len + 4;
			}
			else if (ib_ai == AI_8)
			{
				size_t len = nbtoll(buf + ++*pos);
				*pos += len + 8;
			}
			else
			{
				return CBOR_ERR_RESERVED_AI;
			}
			return *pos > end + 1 ? CBOR_ERR_OUT_OF_DATA : 0;
		}
	}
	else if (ib_mt == IB_UINT || ib_mt == IB_NEGINT)
	{
		if (ib_ai <= 23)
		{
			++*pos;
		}
		else if (ib_ai == AI_1)
		{
			*pos += 2;
		}
		else if (ib_ai == AI_2)
		{
			*pos += 3;
		}
		else if (ib_ai == AI_4)
		{
			*pos += 5;
		}
		else if (ib_ai == AI_8)
		{
			*pos += 9;
		}
		else
		{
			return CBOR_ERR_RESERVED_AI;
		}
		return *pos > end + 1 ? CBOR_ERR_OUT_OF_DATA : 0;
	}
	else if (ib_mt == IB_PRIM)
	{
		if (ib_ai <= 23)
		{
			++*pos;
		}
		else if (ib_ai == AI_1)
		{
			*pos += 2;
		}
		else if (ib_ai == AI_2)
		{
			*pos += 3;
		}
		else if (ib_ai == AI_4)
		{
			*pos += 5;
		}
		else if (ib_ai == AI_8)
		{
			*pos += 9;
		}
		else if (ib_ai == AI_INDEF)
		{
			return CBOR_ERR_BREAK_OUTSIDE_INDEF;
		}
		else
		{
			return CBOR_ERR_RESERVED_AI;
		}
		return *pos > end + 1 ? CBOR_ERR_OUT_OF_DATA : 0;
	}
	else	// TAG: not supported yet
	{
		++*pos;
		return 0;
	}
}

int cbor_verify(cbor_t *cbor)
{
	size_t pos = 0;
	int ret = __cbor_verify(cbor->buf, &pos, cbor->offset - 1);
	if (ret == CBOR_NO_ERROR)
	{
		if (pos < cbor->offset)
		{
			return CBOR_ERR_NOT_ALL_DATA_CONSUMED;
		}
	}
	return ret;
}
