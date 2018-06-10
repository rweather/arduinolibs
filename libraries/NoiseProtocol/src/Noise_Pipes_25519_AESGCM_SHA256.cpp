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

#include "Noise_Pipes_25519_AESGCM_SHA256.h"

/**
 * \class Noise_Pipes_25519_AESGCM_SHA256 Noise_Pipes_25519_AESGCM_SHA256.h <Noise_Pipes_25519_AESGCM_SHA256.h>
 * \brief Noise Pipes descriptor, using Curve25519, AES256, GCM, and SHA256.
 *
 * Noise Pipes combines the effect of XX, IK, and XXfallback to produce a
 * protocol that requires only two handshake messages if the responder's
 * static public key is known, or three handshake messages if the key
 * is unknown or incorrect.
 */

Noise_Pipes_25519_AESGCM_SHA256::Noise_Pipes_25519_AESGCM_SHA256()
{
}

Noise_Pipes_25519_AESGCM_SHA256::~Noise_Pipes_25519_AESGCM_SHA256()
{
}

const NoiseProtocolDescriptor *Noise_Pipes_25519_AESGCM_SHA256::abbreviatedDescriptor() const
{
    return &ik;
}

const NoiseProtocolDescriptor *Noise_Pipes_25519_AESGCM_SHA256::fallbackDescriptor() const
{
    return &fallback;
}
