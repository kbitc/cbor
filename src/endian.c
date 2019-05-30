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

#include "endian.h"
#include <string.h>

#define LITTLE_ENDIAN		1
#define BIG_ENDIAN			2

static uint8_t check_endian()
{
	static uint8_t endian = 0;
	if (endian == 0)
	{
		union
		{
			uint16_t s;
			uint8_t c;
		} v;

		v.s = 1;
		endian = v.c ? LITTLE_ENDIAN : BIG_ENDIAN;
	}
	return endian;
}

static void reverse(void *buf, int len)
{
	uint8_t *p = (uint8_t *)buf;
	for (int i = --len >> 1; i >= 0; i--)
	{
		int j = len - i;
		uint8_t tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
	}
}

uint16_t endian_swap16(uint16_t n)
{
	if (LITTLE_ENDIAN == check_endian())
	{
		reverse((void *)&n, sizeof(uint16_t));
	}
	return n;
}

uint32_t endian_swap32(uint32_t n)
{
	if (LITTLE_ENDIAN == check_endian())
	{
		reverse((void *)&n, sizeof(uint32_t));
	}
	return n;
}

uint64_t endian_swap64(uint64_t n)
{
	if (LITTLE_ENDIAN == check_endian())
	{
		reverse((void *)&n, sizeof(uint64_t));
	}
	return n;
}


uint16_t nbtos(uint8_t *p)
{
	uint16_t u = 0;
	memcpy(&u, p, sizeof(uint16_t));
	return ntohs(u);
}

uint32_t nbtol(uint8_t *p)
{
	uint32_t u = 0;
	memcpy(&u, p, sizeof(uint32_t));
	return ntohl(u);
}

uint64_t nbtoll(uint8_t *p)
{
	uint64_t u = 0;
	memcpy(&u, p, sizeof(uint64_t));
	return ntohll(u);
}

half nbtoh(uint8_t *p)
{
	uint16_t u = nbtos(p);
	return *((half *)&u);
}

float nbtof(uint8_t *p)
{
	uint32_t u = nbtol(p);
	return *((float *)&u);
}

double nbtod(uint8_t *p)
{
	uint64_t u = nbtoll(p);
	return *((double *)&u);
}

void stonb(uint16_t s, uint8_t *p)
{
	uint16_t u = htons(s);
	memcpy(p, &u, sizeof(uint16_t));
}

void ltonb(uint32_t l, uint8_t *p)
{
	uint32_t u = htonl(l);
	memcpy(p, &u, sizeof(uint32_t));
}

void lltonb(uint64_t ll, uint8_t *p)
{
	uint64_t u = htonll(ll);
	memcpy(p, &u, sizeof(uint64_t));
}

void htonb(half h, uint8_t *p)
{
	uint16_t u = htons(*((uint16_t *)(&h)));
	memcpy(p, &u, sizeof(uint16_t));
}

void ftonb(float f, uint8_t *p)
{
	uint32_t u = htonl(*((uint32_t *)(&f)));
	memcpy(p, &u, sizeof(uint32_t));
}

void dtonb(double d, uint8_t *p)
{
	uint64_t u = htonll(*((uint64_t *)(&d)));
	memcpy(p, &u, sizeof(uint64_t));
}
