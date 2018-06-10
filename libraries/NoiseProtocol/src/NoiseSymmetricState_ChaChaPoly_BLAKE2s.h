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

#ifndef NOISE_SYMMETRIC_STATE_CHACHAPOLY_BLAKE2S_H
#define NOISE_SYMMETRIC_STATE_CHACHAPOLY_BLAKE2S_H

#include "NoiseSymmetricState.h"

class NoiseSymmetricState_ChaChaPoly_BLAKE2s : public NoiseSymmetricState
{
public:
    NoiseSymmetricState_ChaChaPoly_BLAKE2s();
    virtual ~NoiseSymmetricState_ChaChaPoly_BLAKE2s();

    void initialize(const char *protocolName);

    bool hasKey() const;

    void mixKey(const void *data, size_t size);
    void mixHash(const void *data, size_t size);
    void mixKeyAndHash(const void *data, size_t size);

    void getHandshakeHash(void *data, size_t size);

    int encryptAndHash
        (uint8_t *output, size_t outputSize,
         const uint8_t *input, size_t inputSize);
    int decryptAndHash
        (uint8_t *output, size_t outputSize,
         const uint8_t *input, size_t inputSize);

    void split(NoiseCipherState **c1, NoiseCipherState **c2);

    void clear();

private:
    struct {
        uint8_t ck[32];
        uint8_t h[32];
        uint8_t key[32];
        uint64_t n;
        bool hasKey;
    } st;

    static void hmac(uint8_t *output, const uint8_t *key,
                     const void *data, size_t size, uint8_t tag);
};

#endif
