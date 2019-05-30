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

#ifndef CBOR_CONST_H
#define CBOR_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

#define IB_UINT						0
#define IB_NEGINT					(1 << 5)
#define IB_BYTES					(2 << 5)
#define IB_STRING					(3 << 5)
#define IB_ARRAY					(4 << 5)
#define IB_MAP						(5 << 5)
#define IB_TAG						(6 << 5)
#define IB_PRIM						(7 << 5)

#define AI_1						24
#define AI_2						25
#define AI_4						26
#define AI_8						27
#define AI_INDEF					31
#define AI_BRKCD					0xFF

#ifdef __cplusplus
}
#endif

#endif  /* CBOR_CONST_H */
