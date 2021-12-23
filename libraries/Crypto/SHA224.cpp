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

#include "SHA224.h"
#include "Crypto.h"

/**
 * \class SHA224 SHA224.h <SHA224.h>
 * \brief SHA-224 hash algorithm.
 *
 * Reference: http://en.wikipedia.org/wiki/SHA-2
 *
 * \sa SHA256, SHA512, SHA3_256, BLAKE2s
 */

/**
 * \var SHA224::HASH_SIZE
 * \brief Constant for the size of the hash output of SHA224.
 */

/**
 * \var SHA224::BLOCK_SIZE
 * \brief Constant for the block size of SHA224.
 */

/**
 * \brief Constructs a SHA-224 hash object.
 */
SHA224::SHA224()
{
    reset();
}

size_t SHA224::hashSize() const
{
    return 28;
}

void SHA224::reset()
{
    state.h[0] = 0xc1059ed8;
    state.h[1] = 0x367cd507;
    state.h[2] = 0x3070dd17;
    state.h[3] = 0xf70e5939;
    state.h[4] = 0xffc00b31;
    state.h[5] = 0x68581511;
    state.h[6] = 0x64f98fa7;
    state.h[7] = 0xbefa4fa4;
    state.chunkSize = 0;
    state.length = 0;
}
