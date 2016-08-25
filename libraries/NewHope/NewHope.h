/*
 * Copyright (C) 2016 Southern Storm Software, Pty Ltd.
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

#ifndef CRYPTO_NEWHOPE_h
#define CRYPTO_NEWHOPE_h

#include <inttypes.h>

#define NEWHOPE_SENDABYTES  1824
#define NEWHOPE_SENDBBYTES  2048
#define NEWHOPE_SHAREDBYTES 32

#if defined(__AVR__)
#define NEWHOPE_SMALL_FOOTPRINT 1
#else
#define NEWHOPE_SMALL_FOOTPRINT 0
#endif

typedef struct
{
    /** @cond */
#if NEWHOPE_SMALL_FOOTPRINT
    uint8_t seed[32];
#else
    uint16_t coeffs[1024];
#endif
    /** @endcond */

} NewHopePrivateKey;

class NewHope
{
private:
    NewHope() {}
    ~NewHope() {}

public:
    enum Variant
    {
        Ref,
        Torref
    };

    static void keygen(uint8_t send[NEWHOPE_SENDABYTES], NewHopePrivateKey &sk,
                       Variant variant = Ref, const uint8_t *random_seed = 0);
    static void sharedb(uint8_t shared_key[NEWHOPE_SHAREDBYTES],
                        uint8_t send[NEWHOPE_SENDBBYTES],
                        uint8_t received[NEWHOPE_SENDABYTES],
                        Variant variant = Ref, const uint8_t *random_seed = 0);
    static void shareda(uint8_t shared_key[NEWHOPE_SHAREDBYTES],
                        const NewHopePrivateKey &sk,
                        uint8_t received[NEWHOPE_SENDBBYTES]);
};

#endif
