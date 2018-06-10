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

#ifndef NOISE_XX_FALLBACK_25519_AESGCM_SHA256_h
#define NOISE_XX_FALLBACK_25519_AESGCM_SHA256_h

#include "Noise_XXfallback.h"
#include "NoiseProtocolDescriptor.h"
#include "NoiseSymmetricState_AESGCM_SHA256.h"
#include "NoiseDHState_Curve25519.h"

class NoiseHandshakeState_XXfallback_25519_AESGCM_SHA256 : public NoiseHandshakeState_XXfallback
{
public:
    NoiseHandshakeState_XXfallback_25519_AESGCM_SHA256();
    virtual ~NoiseHandshakeState_XXfallback_25519_AESGCM_SHA256();

private:
    NoiseSymmetricState_AESGCM_SHA256 sym;
    NoiseDHState_Curve25519 dh;
};

class Noise_XXfallback_25519_AESGCM_SHA256 : public NoiseProtocolDescriptor
{
public:
    Noise_XXfallback_25519_AESGCM_SHA256();
    virtual ~Noise_XXfallback_25519_AESGCM_SHA256();

    NoiseHandshakeState *createHandshake() const;
};

#endif
