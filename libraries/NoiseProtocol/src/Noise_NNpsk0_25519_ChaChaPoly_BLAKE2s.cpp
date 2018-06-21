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

#include "Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.h"

/**
 * \class NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.h <Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.h>
 * \brief "NNpsk0" Noise handshake, using Curve25519, ChaChaPoly, and BLAKE2s.
 */

/**
 * \class Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.h <Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.h>
 * \brief "NNpsk0" Noise descriptor, using Curve25519, ChaChaPoly, and BLAKE2s.
 */

static char const Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s_Name[] =
    "Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s";

NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s::NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s()
{
    setSymmetricState(&sym);
    setDHState(&dh);
    setProtocolName(Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s_Name);
}

NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s::~NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s()
{
}

static NoiseHandshakeState *Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s_createHandshake()
{
    return new NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s();
}

/**
 * \brief Protocol descriptor for "Noise_NNps0_25519_ChaChaPoly_BLAKE2s".
 */
const NoiseProtocolDescriptor Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s = {
    NOISE_PROTOCOL_NEEDS_PSK,
    Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s_Name,
    0,
    Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s_createHandshake
};
