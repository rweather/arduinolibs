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

#include "NoiseCipherState_AESGCM.h"
#include "Crypto.h"
#include <string.h>

/**
 * \class NoiseCipherState_AESGCM NoiseCipherState_AESGCM.h <NoiseCipherState_AESGCM.h>
 * \brief Implementation of the AESGCM cipher
 */

/**
 * \brief Constructs a new AESGCM cipher object.
 *
 * \param key The 256-bit key for the cipher object.
 */
NoiseCipherState_AESGCM::NoiseCipherState_AESGCM(const uint8_t key[32])
{
    cipher.setKey(key, 32);
    n = 0;
}

/**
 * \brief Destroys this AESGCM cipher object.
 */
NoiseCipherState_AESGCM::~NoiseCipherState_AESGCM()
{
    clean(&n, sizeof(n));
}

void NoiseCipherState_AESGCM::setNonce(uint64_t nonce)
{
    n = nonce;
}

// Import from NoiseSymmetricState_AESGCM_SHA256.cpp.
void noiseAESGCMFormatIV(uint8_t iv[12], uint64_t n);

int NoiseCipherState_AESGCM::encryptPacket
    (void *output, size_t outputSize, const void *input, size_t inputSize)
{
    if (outputSize < 16 || (outputSize - 16) < inputSize)
        return -1;
    uint8_t iv[12];
    noiseAESGCMFormatIV(iv, n);
    cipher.setIV(iv, sizeof(iv));
    cipher.encrypt((uint8_t *)output, (const uint8_t *)input, inputSize);
    cipher.computeTag(((uint8_t *)output) + inputSize, 16);
    ++n;
    return inputSize + 16;
}

int NoiseCipherState_AESGCM::decryptPacket
    (void *output, size_t outputSize, const void *input, size_t inputSize)
{
    if (inputSize < 16 || outputSize < (inputSize - 16))
        return -1;
    uint8_t iv[12];
    noiseAESGCMFormatIV(iv, n);
    cipher.setIV(iv, sizeof(iv));
    cipher.decrypt((uint8_t *)output, (const uint8_t *)input, inputSize - 16);
    if (cipher.checkTag(((const uint8_t *)input) + inputSize - 16, 16)) {
        ++n;
        return inputSize - 16;
    }
    memset(output, 0, outputSize); // Destroy the output if the tag is invalid.
    return -1;
}

void NoiseCipherState_AESGCM::rekey()
{
    uint8_t k[32];
    static uint8_t const iv[12] = {
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
    memset(k, 0, sizeof(k));
    cipher.setIV(iv, sizeof(iv));
    cipher.encrypt(k, k, sizeof(k));
    cipher.setKey(k, sizeof(k));
    clean(k);
}

void NoiseCipherState_AESGCM::clear()
{
    cipher.clear();
    n = 0;
}
