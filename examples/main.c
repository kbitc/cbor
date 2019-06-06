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
#include <stdio.h>
#include <stdlib.h>

void print_cbor(cbor_t *val)
{
	switch (val->ct)
	{
		case CBOR_FALSE:
			printf("false");
			break;
		case CBOR_TRUE:
			printf("true");
			break;
		case CBOR_NULL:
			printf("null");
			break;
		case CBOR_UNDEFINED:
			printf("undefined");
			break;
		case CBOR_SIMPLE:
		case CBOR_UINT:
			printf("%u", val->v.uint);
			break;
		case CBOR_NEGINT:
			printf("%d", val->v.sint);
			break;
		case CBOR_FLOAT:
			printf("%f", val->v.flt);
			break;
		case CBOR_DOUBLE:
			printf("%f", val->v.dbl);
			break;
		case CBOR_TAG:
			printf("%u(", val->v.uint);
			print_cbor(val->next);
			printf(")");
			break;
		case CBOR_BYTES:
		case CBOR_BYTES_INDEF:
		{
			size_t len;
			cbor_bytes_len(val, &len);
			uint8_t *bytes = (uint8_t *)malloc(sizeof(uint8_t) * len);
			if (bytes == NULL)
			{
				printf("[Fatal] Out of memory!\n");
			}

			size_t copied_len;
			cbor_bytes_copy(bytes, val, len, &copied_len);
			printf("Buffer<");
			for (size_t i = 0; i < copied_len; i++)
			{
				if (i == 0)
				{
					printf("%02x", bytes[i]);
				}
				else
				{
					printf(" %02x", bytes[i]);
				}
			}
			printf(">");

			free(bytes);
			break;
		}
		case CBOR_STRING:
		case CBOR_STRING_INDEF:
		{
			size_t len;
			cbor_bytes_len(val, &len);
			char *str = (char *)malloc(sizeof(char) * (len + 1));
			if (str == NULL)
			{
				printf("[Fatal] Out of memory!\n");
			}

			size_t copied_len;
			cbor_bytes_copy(str, val, len, &copied_len);
			str[copied_len - 1] = '\0';
			printf("\"%s\"", str);

			free(str);
			break;
		}
		default:
			break;
	}
}

int main()
{
	uint8_t buf[256];
	size_t pos = 0, size = sizeof(buf);

	// 1. encoding
	cbor_encode_map_indef(buf, size, &pos);				// begin indefinite-length map

	cbor_encode_string(buf, size, &pos, "uint");
	cbor_encode_uint(buf, size, &pos, 100);

	cbor_encode_string(buf, size, &pos, "int");	
	cbor_encode_int(buf, size, &pos, -100);

	cbor_encode_string(buf, size, &pos, "float");
	cbor_encode_float(buf, size, &pos, 1.2356);

	cbor_encode_string(buf, size, &pos, "simple");
	cbor_encode_simple(buf, size, &pos, AI_FALSE);

	uint8_t bytes[] = { 0x01, 0x02, 0x03, 0x04 };
	cbor_encode_string(buf, size, &pos, "bytes");
	cbor_encode_bytes(buf, size, &pos, bytes, sizeof(bytes));

	cbor_encode_string(buf, size, &pos, "string");
	cbor_encode_string(buf, size, &pos, "hello world!");

	cbor_encode_string(buf, size, &pos, "tag");
	cbor_encode_tag(buf, size, &pos, 1);
	cbor_encode_string(buf, size, &pos, "1970-01-01T00:00Z");

	cbor_encode_string(buf, size, &pos, "array");
	cbor_encode_array(buf, size, &pos, 2);
	cbor_encode_uint(buf, size, &pos, 1000);
	cbor_encode_uint(buf, size, &pos, 2000);

	cbor_encode_string(buf, size, &pos, "map");
	cbor_encode_map(buf, size, &pos, 2);
	cbor_encode_string(buf, size, &pos, "uint1");
	cbor_encode_uint(buf, size, &pos, 1000);
	cbor_encode_string(buf, size, &pos, "uint2");
	cbor_encode_uint(buf, size, &pos, 2000);

	cbor_encode_string(buf, size, &pos, "array_indef");
	cbor_encode_array_indef(buf, size, &pos);			// begin indefinite-length array
	cbor_encode_uint(buf, size, &pos, 3000);
	cbor_encode_uint(buf, size, &pos, 4000);
	cbor_encode_uint(buf, size, &pos, 5000);
	cbor_encode_break(buf, size, &pos);					// end indefinite-length array

	cbor_encode_string(buf, size, &pos, "string_indef");
	cbor_encode_string_indef(buf, size, &pos);			// begin indefinite-length string
	cbor_encode_string(buf, size, &pos, "this ");
	cbor_encode_string(buf, size, &pos, "is ");
	cbor_encode_string(buf, size, &pos, "indefinite-length ");
	cbor_encode_string(buf, size, &pos, "string.");
	cbor_encode_break(buf, size, &pos);					// end indefinite-length string

	cbor_encode_break(buf, size, &pos);					// end indefinite-length map
	
	printf("[CBOR]: \n");
	for (int i = 0; i < pos; i++)
	{
		printf("%02x", buf[i]);
	}
	printf("\n\n");

	
	// 2. decoding
	printf("[JSON]: \n");

	size_t err_pos = 0;
	cbor_t *cbor = cbor_create();
	int ret = cbor_decode(buf, size, &err_pos, cbor);
	if (ret != CBOR_NO_ERROR)
	{
		printf("pos = %d, errno = %d, errText = %s\n", err_pos, ret, cbor_get_error(ret));
		return 1;
	}

	cbor_t *val = cbor_create();
	printf("{");

	cbor_map_get(cbor, "uint", val);
	printf("\"uint\": ");
	print_cbor(val);

	cbor_map_get(cbor, "int", val);
	printf(", \"int\": ");
	print_cbor(val);

	cbor_map_get(cbor, "float", val);
	printf(", \"float\": ");
	print_cbor(val);

	cbor_map_get(cbor, "simple", val);
	printf(", \"simple\": ");
	print_cbor(val);

	cbor_map_get(cbor, "bytes", val);
	printf(", \"bytes\": ");
	print_cbor(val);

	cbor_map_get(cbor, "string", val);
	printf(", \"string\": ");
	print_cbor(val);

	cbor_map_get(cbor, "tag", val);
	printf(", \"tag\": ");
	print_cbor(val);

	cbor_map_get(cbor, "string_indef", val);
	printf(", \"string_indef\": ");
	print_cbor(val);

	cbor_map_get(cbor, "map", val);
	cbor_t *subval = cbor_create();
	printf(", \"map\": {");

	cbor_map_get(val, "uint1", subval);
	printf("\"uint1\": ");
	print_cbor(subval);

	cbor_map_get(val, "uint2", subval);
	printf(", \"uint2\": ");
	print_cbor(subval);

	printf("}");

	cbor_map_get(cbor, "array", val);
	printf(", \"array\": [");

	for (int i = 0; i < val->count; i++)
	{
		cbor_array_get(val, i, subval);
		if (i > 0)
		{
			printf(", ");
		}
		print_cbor(subval);
	}

	printf("]");

	printf("}\n");

	cbor_free(subval);

	cbor_free(val);

	cbor_free(cbor);
}
