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

#ifndef CRYPTO_CURVE15519_h
#define CRYPTO_CURVE15519_h

#include <inttypes.h>
#include <stddef.h>

// Define exactly one of these to 1 to set the size of the basic limb type.
// 16-bit limbs seems to give the best performance on 8-bit AVR micros.
#define CURVE25519_LIMB_8BIT  0
#define CURVE25519_LIMB_16BIT 1
#define CURVE25519_LIMB_32BIT 0

class Curve25519
{
public:
    static bool eval(uint8_t result[32], const uint8_t s[32], const uint8_t x[32]);

    static void dh1(uint8_t k[32], uint8_t f[32]);
    static bool dh2(uint8_t k[32], uint8_t f[32]);

#if defined(TEST_CURVE25519_FIELD_OPS)
public:
#else
private:
#endif
    // Define the limb types to use on this platform.
    #if CURVE25519_LIMB_8BIT
    typedef uint8_t limb_t;
    typedef int8_t slimb_t;
    typedef uint16_t dlimb_t;
    #elif CURVE25519_LIMB_16BIT
    typedef uint16_t limb_t;
    typedef int16_t slimb_t;
    typedef uint32_t dlimb_t;
    #elif CURVE25519_LIMB_32BIT
    typedef uint32_t limb_t;
    typedef int32_t slimb_t;
    typedef uint64_t dlimb_t;
    #else
    #error "limb_t must be 8, 16, or 32 bits in size"
    #endif

    static uint8_t isWeakPoint(const uint8_t k[32]);

    static void reduce(limb_t *result, limb_t *x, uint8_t size);
    static limb_t reduceQuick(limb_t *x);

    static void mul(limb_t *result, const limb_t *x, const limb_t *y);
    static void square(limb_t *result, const limb_t *x)
    {
        mul(result, x, x);
    }

    static void mulA24(limb_t *result, const limb_t *x);

    static void add(limb_t *result, const limb_t *x, const limb_t *y);
    static void sub(limb_t *result, const limb_t *x, const limb_t *y);

    static void cswap(uint8_t select, limb_t *x, limb_t *y);

    static void recip(limb_t *result, const limb_t *x);

    static void unpack(limb_t *result, const uint8_t *x);
    static void pack(uint8_t *result, const limb_t *x);

    // Constructor and destructor are private - cannot instantiate this class.
    Curve25519() {}
    ~Curve25519() {}
};

#endif
