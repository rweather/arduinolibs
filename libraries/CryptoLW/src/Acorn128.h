/*
 * Copyright (C) 2018 Southern Storm Software, Pty Ltd.
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

#ifndef CRYPTO_ACORN128_H
#define CRYPTO_ACORN128_H

#include "AuthenticatedCipher.h"

/** @cond acorn128_state */

// The ACORN-128 state consists of 293 bits split across six
// Linear Feedback Shift Registers (LFSR's) and 4 bits spare.
// In this implementation, each LFSR is represented by a
// 48-bit or 64-bit register split into 32/16-bit words.
// The optimized reference implementation from the algorithm's
// authors uses 7 uint64_t registers, for a total state size
// of 448 bits.  This version uses 328 bits for same data and
// should be efficient on 8-bit and 32-bit microcontrollers.
typedef struct
{
    uint32_t k[4];      // Cached copy of the key for multiple requests.
    uint32_t s1_l;      // LFSR1, 61 bits, 0..60, low word
    uint32_t s1_h;      // LFSR1, high word
    uint32_t s2_l;      // LFSR2, 46 bits, 61..106, low word
    uint16_t s2_h;      // LFSR2, high word
    uint16_t s3_h;      // LFSR3, 47 bits, 107..153, high word
    uint32_t s3_l;      // LFSR3, low word
    uint32_t s4_l;      // LFSR4, 39 bits, 154..192, low word
    uint16_t s4_h;      // LFSR4, high word
    uint16_t s5_h;      // LFSR5, 37 bits, 193..229, high word
    uint32_t s5_l;      // LFSR5, low word
    uint32_t s6_l;      // LFSR6, 59 bits, 230..288, low word
    uint32_t s6_h;      // LFSR6, high word
    uint8_t s7;         // Top most 4 bits, 289..292
    uint8_t authDone;   // Non-zero once authentication is done.

} Acorn128State;

// Determine which Acorn128 implementation to export to applications.
#if defined(__AVR__)
#define CRYPTO_ACORN128_AVR 1
#else
#define CRYPTO_ACORN128_DEFAULT 1
#endif

/** @endcond */

class Acorn128 : public AuthenticatedCipher
{
public:
    Acorn128();
    virtual ~Acorn128();

    size_t keySize() const;
    size_t ivSize() const;
    size_t tagSize() const;

    bool setKey(const uint8_t *key, size_t len);
    bool setIV(const uint8_t *iv, size_t len);

    void encrypt(uint8_t *output, const uint8_t *input, size_t len);
    void decrypt(uint8_t *output, const uint8_t *input, size_t len);

    void addAuthData(const void *data, size_t len);

    void computeTag(void *tag, size_t len);
    bool checkTag(const void *tag, size_t len);

    void clear();

private:
    Acorn128State state;
};

#endif
