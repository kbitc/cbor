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
#include <stdlib.h>

cbor_t *cbor_create()
{
	cbor_t *cbor = (cbor_t *)malloc(sizeof(cbor_t));
	if (cbor != NULL)
	{
		memset(cbor, 0, sizeof(cbor_t));
	}
	return cbor;
}

void cbor_free(cbor_t *cbor)
{
	if (cbor != NULL)
	{
		if (cbor->next != NULL)
		{
			cbor_free(cbor->next);
			cbor->next = NULL;
		}
		free(cbor);
	}
}

int cbor_bytes_len(cbor_t *cbor, size_t *len)
{
	if (cbor->ct == CBOR_BYTES || cbor->ct == CBOR_STRING)
	{
		*len = cbor->size;
		return CBOR_NO_ERROR;
	}
	else if (cbor->ct == CBOR_BYTES_INDEF || cbor->ct == CBOR_STRING_INDEF)
	{
		*len = 0;
		for (size_t i = 0, pos = 0; i < cbor->count; i++)
		{
			cbor_t item;
			int ret = cbor_decode(cbor->v.bytes, cbor->size, &pos, &item);
			if (ret != CBOR_NO_ERROR)
			{
				return ret;
			}

			*len += item.size;
		}
		return CBOR_NO_ERROR;
	}
	else
	{
		return -1;
	}
}

int cbor_bytes_cmp(cbor_t *cbor, const void *buf, size_t size, int *res)
{
	if (cbor->ct == CBOR_BYTES || cbor->ct == CBOR_STRING)
	{
		size_t len = cbor->size > size ? size : cbor->size;
		*res = memcmp(cbor->v.bytes, buf, len);
		if (!*res)
		{
			*res = cbor->size - size;
		}
		return CBOR_NO_ERROR;
	}
	else if (cbor->ct == CBOR_BYTES_INDEF || cbor->ct == CBOR_STRING_INDEF)
	{
		for (size_t i = 0, pos1 = 0, pos2 = 0; i < cbor->count && pos2 < size; i++)
		{
			cbor_t item;
			int ret = cbor_decode(cbor->v.bytes, cbor->size, &pos1, &item);
			if (ret != CBOR_NO_ERROR)
			{
				return ret;
			}

			ret = cbor_bytes_cmp(&item, buf + pos2, item.size, res);
			if (ret != CBOR_NO_ERROR)
			{
				return ret;
			}

			if (*res)
			{
				break;
			}
			pos2 += item.size;
		}
		return CBOR_NO_ERROR;
	}
	else
	{
		return -1;
	}
}

int cbor_array_get(cbor_t *cbor, size_t index, cbor_t *val)
{
	if (cbor->ct != CBOR_ARRAY)
	{
		return -1;
	}

	if (index >= cbor->count)
	{
		return -2;
	}

	size_t pos = 0;
	for (size_t i = 0; i < index; i++)
	{
		int ret = cbor_verify(cbor->v.bytes, cbor->size, &pos);
		if (ret != CBOR_NO_ERROR)
		{
			return ret;
		}
	}
	return cbor_decode(cbor->v.bytes, cbor->size, &pos, val);
}

int cbor_map_get(cbor_t *cbor, const char *key, cbor_t *val)
{
	if (cbor->ct != CBOR_MAP)
	{
		return -1;
	}

	for (size_t i = 0, pos = 0; i < cbor->count; i += 2)
	{
		cbor_t _key;
		int ret = cbor_decode(cbor->v.bytes, cbor->size, &pos, &_key);
		if (ret != CBOR_NO_ERROR)
		{
			return ret;
		}

		if (_key.ct == CBOR_STRING)
		{
			if (_key.size == strlen(key) && !memcmp(_key.v.str, key, _key.size))
			{
				return cbor_decode(cbor->v.bytes, cbor->size, &pos, val);
			}
		}
		else if (_key.ct == CBOR_STRING_INDEF)
		{
			int res = 0;
			ret = cbor_bytes_cmp(&_key, key, strlen(key), &res);
			if (ret != CBOR_NO_ERROR)
			{
				return ret;
			}

			if (!res)
			{
				return cbor_decode(cbor->v.bytes, cbor->size, &pos, val);
			}
		}

		ret = cbor_verify(cbor->v.bytes, cbor->size, &pos);
		if (ret != CBOR_NO_ERROR)
		{
			return ret;
		}
	}
	return -2;
}
