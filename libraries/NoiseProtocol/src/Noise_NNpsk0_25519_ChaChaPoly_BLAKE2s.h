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

#ifndef NOISE_NN_PSK0_25519_CHACHAPOLY_BLAKE2S_h
#define NOISE_NN_PSK0_25519_CHACHAPOLY_BLAKE2S_h

#include "Noise_NNpsk0.h"
#include "NoiseProtocolDescriptor.h"
#include "NoiseSymmetricState_ChaChaPoly_BLAKE2s.h"
#include "NoiseDHState_Curve25519.h"

class NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s
    : public NoiseHandshakeState_NNpsk0
{
public:
    NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s();
    virtual ~NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s();

private:
    NoiseSymmetricState_ChaChaPoly_BLAKE2s sym;
    NoiseDHState_Curve25519_EphemOnly dh;
};

class Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s : public NoiseProtocolDescriptor
{
public:
    Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s();
    virtual ~Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s();

    NoiseHandshakeState *createHandshake() const;
};

#endif
