/*
 * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef CRYPTO_ENDIANUTIL_H
#define CRYPTO_ENDIANUTIL_H

#include <inttypes.h>

// CPU is assumed to be little endian.   Edit this file if you
// need to port this library to a big endian CPU.

#define htole32(x)  (x)
#define le32toh(x)  (x)
#define htobe32(x)  \
    (__extension__ ({ \
        uint32_t _temp = (x); \
        ((_temp >> 24) & 0x000000FF) | \
        ((_temp >>  8) & 0x0000FF00) | \
        ((_temp <<  8) & 0x00FF0000) | \
        ((_temp << 24) & 0xFF000000); \
    }))
#define be32toh(x)  (htobe32((x)))

#endif
