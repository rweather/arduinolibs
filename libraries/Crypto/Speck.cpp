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

#include "Speck.h"
#include "Crypto.h"
#include "utility/RotateUtil.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class Speck Speck.h <Speck.h>
 * \brief Speck block cipher with a 128-bit block size.
 *
 * Speck is a family of lightweight block ciphers designed by the
 * National Security Agency (NSA).  The ciphers are highly optimized
 * for software implementation on microcontrollers.
 *
 * This class implements the Speck family that uses 128-bit block sizes
 * with 128-bit, 192-bit, or 256-bit key sizes.  Other Speck families support
 * smaller block sizes of 32, 48, 64, or 96 bits but such block sizes are
 * really too small for use in modern cryptosystems.
 *
 * \note Current crytoanalysis (up until 2015) has not revealed any obvious
 * weaknesses in the full-round version of Speck.  But if you are wary of
 * ciphers designed by the NSA, then use ChaCha or AES instead.
 *
 * References: https://en.wikipedia.org/wiki/Speck_%28cipher%29,
 * http://eprint.iacr.org/2013/404
 */

/**
 * \brief Constructs a Speck block cipher with no initial key.
 *
 * This constructor must be followed by a call to setKey() before the
 * block cipher can be used for encryption or decryption.
 */
Speck::Speck()
    : rounds(32)
{
}

Speck::~Speck()
{
    clean(k);
}

size_t Speck::blockSize() const
{
    return 16;
}

size_t Speck::keySize() const
{
    // Also supports 128-bit and 192-bit, but we only report 256-bit.
    return 32;
}

// Pack/unpack big-endian 64-bit quantities.
#if defined(__AVR__)
#define pack64(data, value) \
    do { \
        const uint8_t *src = (const uint8_t *)&(value); \
        (data)[0] = src[7]; \
        (data)[1] = src[6]; \
        (data)[2] = src[5]; \
        (data)[3] = src[4]; \
        (data)[4] = src[3]; \
        (data)[5] = src[2]; \
        (data)[6] = src[1]; \
        (data)[7] = src[0]; \
    } while (0)
#define unpack64(value, data) \
    do { \
        uint8_t *dest = (uint8_t *)&(value); \
        dest[0] = (data)[7]; \
        dest[1] = (data)[6]; \
        dest[2] = (data)[5]; \
        dest[3] = (data)[4]; \
        dest[4] = (data)[3]; \
        dest[5] = (data)[2]; \
        dest[6] = (data)[1]; \
        dest[7] = (data)[0]; \
    } while (0)
#else
#define pack64(data, value) \
    do { \
        uint64_t v = htobe64((value)); \
        memcpy((data), &v, sizeof(uint64_t)); \
    } while (0)
#define unpack64(value, data) \
    do { \
        memcpy(&(value), (data), sizeof(uint64_t)); \
        (value) = be64toh((value)); \
    } while (0)
#endif

bool Speck::setKey(const uint8_t *key, size_t len)
{
    uint64_t l[4];
    uint8_t m;
    if (len == 32) {
        m = 4;
        unpack64(l[2], key);
        unpack64(l[1], key + 8);
        unpack64(l[0], key + 16);
        unpack64(k[0], key + 24);
    } else if (len == 24) {
        m = 3;
        unpack64(l[1], key);
        unpack64(l[0], key + 8);
        unpack64(k[0], key + 16);
    } else if (len == 16) {
        m = 2;
        unpack64(l[0], key);
        unpack64(k[0], key + 8);
    } else {
        return false;
    }
    rounds = 30 + m;
    uint8_t li_in = 0;
    uint8_t li_out = m - 1;
    for (uint8_t i = 0; i < (rounds - 1); ++i) {
        l[li_out] = (k[i] + rightRotate8_64(l[li_in])) ^ i;
        k[i + 1] = leftRotate3_64(k[i]) ^ l[li_out];
        if ((++li_in) >= m)
            li_in = 0;
        if ((++li_out) >= m)
            li_out = 0;
    }
    clean(l);
    return true;
}

void Speck::encryptBlock(uint8_t *output, const uint8_t *input)
{
    uint64_t x, y;
    const uint64_t *s = k;
    unpack64(x, input);
    unpack64(y, input + 8);
    for (uint8_t round = rounds; round > 0; --round, ++s) {
        x = (rightRotate8_64(x) + y) ^ s[0];
        y = leftRotate3_64(y) ^ x;
    }
    pack64(output, x);
    pack64(output + 8, y);
}

void Speck::decryptBlock(uint8_t *output, const uint8_t *input)
{
    uint64_t x, y;
    const uint64_t *s = k + rounds - 1;
    unpack64(x, input);
    unpack64(y, input + 8);
    for (uint8_t round = rounds; round > 0; --round, --s) {
        y = rightRotate3_64(x ^ y);
        x = leftRotate8_64((x ^ s[0]) - y);
    }
    pack64(output, x);
    pack64(output + 8, y);
}

void Speck::clear()
{
    clean(k);
}
