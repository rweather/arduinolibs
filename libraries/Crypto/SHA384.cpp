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

#include "SHA384.h"
#include "Crypto.h"
#include "utility/ProgMemUtil.h"
#include <string.h>

/**
 * \class SHA384 SHA384.h <SHA384.h>
 * \brief SHA-384 hash algorithm.
 *
 * Reference: http://en.wikipedia.org/wiki/SHA-2
 *
 * \sa SHA256, SHA512, SHA3_256, BLAKE2s
 */

/**
 * \var SHA384::HASH_SIZE
 * \brief Constant for the size of the hash output of SHA384.
 */

/**
 * \var SHA384::BLOCK_SIZE
 * \brief Constant for the block size of SHA384.
 */

/**
 * \brief Constructs a SHA-384 hash object.
 */
SHA384::SHA384()
{
    reset();
}

size_t SHA384::hashSize() const
{
    return 48;
}

void SHA384::reset()
{
    static uint64_t const hashStart[8] PROGMEM = {
        0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL,
        0x152fecd8f70e5939ULL, 0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL,
        0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL
    };
    memcpy_P(state.h, hashStart, sizeof(hashStart));
    state.chunkSize = 0;
    state.lengthLow = 0;
    state.lengthHigh = 0;
}
