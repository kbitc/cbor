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

#include <stdio.h>

const char *cbor_error_text[] = {
	"CBOR_NO_ERROR",
	"CBOR_ERR_OUT_OF_DATA",
	"CBOR_ERR_NOT_ALL_DATA_CONSUMED",
	"CBOR_ERR_ODD_SIZE_INDEF_MAP",
	"CBOR_ERR_BREAK_OUTSIDE_INDEF",
	"CBOR_ERR_MT_UNDEF_FOR_INDEF",
	"CBOR_ERR_RESERVED_AI",
	"CBOR_ERR_BYTES_TEXT_MISMATCH",	// bytes/text mismatch (UTF-8 != ASCII-8BIT) in streaming string
	"CBOR_ERR_OUT_OF_MEMORY",
	"CBOR_ERR_SIMPLE_OUT_OF_SCOPE"
};

const char *cbor_get_error(int errno)
{
	int len = sizeof(cbor_error_text) / sizeof(const char *);
	if (errno >= 0 && errno < len)
	{
		return cbor_error_text[errno];
	}
	return NULL;
}
