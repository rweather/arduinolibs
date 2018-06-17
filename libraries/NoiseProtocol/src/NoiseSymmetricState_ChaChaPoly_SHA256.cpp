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

#include "NoiseSymmetricState_ChaChaPoly_SHA256.h"
#include "NoiseCipherState_ChaChaPoly.h"
#include "ChaChaPoly.h"
#include "SHA256.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class NoiseSymmetricState_ChaChaPoly_SHA256 NoiseSymmetricState_ChaChaPoly_SHA256.h <NoiseSymmetricState_ChaChaPoly_SHA256.h>
 * \brief Noise symmetric state implementation using ChaChaPoly and SHA256.
 */

/**
 * \brief Constructs a new symmetric state using ChaChaPoly and SHA256.
 */
NoiseSymmetricState_ChaChaPoly_SHA256::NoiseSymmetricState_ChaChaPoly_SHA256()
{
    st.n = 0;
    st.hasKey = false;
}

/**
 * \brief Destroys this symmetric state object.
 */
NoiseSymmetricState_ChaChaPoly_SHA256::~NoiseSymmetricState_ChaChaPoly_SHA256()
{
    clean(st);
}

void NoiseSymmetricState_ChaChaPoly_SHA256::initialize
    (const char *protocolName)
{
    size_t len = strlen(protocolName);
    if (len <= 32) {
        memcpy(st.h, protocolName, len);
        memset(st.h + len, 0, 32 - len);
    } else {
        SHA256 hash;
        hash.update(protocolName, len);
        hash.finalize(st.h, 32);
    }
    memcpy(st.ck, st.h, 32);
    st.hasKey = false;
}

bool NoiseSymmetricState_ChaChaPoly_SHA256::hasKey() const
{
    return st.hasKey;
}

void NoiseSymmetricState_ChaChaPoly_SHA256::mixKey
    (const void *data, size_t size)
{
    hmac(st.key, st.ck, data, size, 0);
    hmac(st.ck, st.key, 0, 0, 1);
    hmac(st.key, st.key, st.ck, 32, 2);
    st.hasKey = true;
    st.n = 0;
}

void NoiseSymmetricState_ChaChaPoly_SHA256::mixHash
    (const void *data, size_t size)
{
    SHA256 hash;
    hash.update(st.h, sizeof(st.h));
    hash.update(data, size);
    hash.finalize(st.h, sizeof(st.h));
}

void NoiseSymmetricState_ChaChaPoly_SHA256::mixKeyAndHash
    (const void *data, size_t size)
{
    uint8_t temph[32];
    hmac(st.key, st.ck, data, size, 0);
    hmac(st.ck, st.key, 0, 0, 1);
    hmac(temph, st.key, st.ck, 32, 2);
    hmac(st.key, st.key, temph, 32, 3);
    st.hasKey = true;
    st.n = 0;
    mixHash(temph, 32);
    clean(temph);
}

void NoiseSymmetricState_ChaChaPoly_SHA256::getHandshakeHash
    (void *data, size_t size)
{
    if (size <= 32) {
        memcpy(data, st.h, size);
    } else {
        memcpy(data, st.h, 32);
        memset(((uint8_t *)data) + 32, 0, size - 32);
    }
}

int NoiseSymmetricState_ChaChaPoly_SHA256::encryptAndHash
    (uint8_t *output, size_t outputSize,
     const uint8_t *input, size_t inputSize)
{
    if (st.hasKey) {
        if (outputSize < 16 || (outputSize - 16) < inputSize)
            return -1;
        ChaChaPoly cipher;
        uint64_t iv = htole64(st.n);
        cipher.setKey(st.key, 32);
        cipher.setIV((const uint8_t *)&iv, sizeof(iv));
        cipher.addAuthData(st.h, sizeof(st.h));
        cipher.encrypt(output, input, inputSize);
        cipher.computeTag(output + inputSize, 16);
        mixHash(output, inputSize + 16);
        ++st.n;
        return inputSize + 16;
    } else {
        if (outputSize < inputSize)
            return -1;
        memcpy(output, input, inputSize);
        mixHash(output, inputSize);
        return inputSize;
    }
}

int NoiseSymmetricState_ChaChaPoly_SHA256::decryptAndHash
    (uint8_t *output, size_t outputSize,
     const uint8_t *input, size_t inputSize)
{
    if (st.hasKey) {
        if (inputSize < 16 || outputSize < (inputSize - 16))
            return -1;
        outputSize = inputSize - 16;
        ChaChaPoly cipher;
        uint64_t iv = htole64(st.n);
        cipher.setKey(st.key, 32);
        cipher.setIV((const uint8_t *)&iv, sizeof(iv));
        cipher.addAuthData(st.h, sizeof(st.h));
        mixHash(input, inputSize);
        cipher.decrypt(output, input, outputSize);
        if (cipher.checkTag(input + outputSize, 16)) {
            ++st.n;
            return outputSize;
        }
        memset(output, 0, outputSize); // Destroy output if tag is incorrect.
        return -1;
    } else {
        if (outputSize < inputSize)
            return -1;
        mixHash(input, inputSize);
        memcpy(output, input, inputSize);
        return inputSize;
    }
}

void NoiseSymmetricState_ChaChaPoly_SHA256::split
    (NoiseCipherState **c1, NoiseCipherState **c2)
{
    uint8_t k1[32];
    uint8_t k2[32];
    hmac(k2, st.ck, 0, 0, 0);
    hmac(k1, k2, 0, 0, 1);
    hmac(k2, k2, k1, 32, 2);
    if (c1)
        *c1 = new NoiseCipherState_ChaChaPoly(k1);
    if (c2)
        *c2 = new NoiseCipherState_ChaChaPoly(k2);
    clean(k1);
    clean(k2);
}

void NoiseSymmetricState_ChaChaPoly_SHA256::clear()
{
    clean(st);
    st.n = 0;
    st.hasKey = false;
}

void NoiseSymmetricState_ChaChaPoly_SHA256::hmac
    (uint8_t *output, const uint8_t *key,
     const void *data, size_t size, uint8_t tag)
{
    SHA256 hash;
    hash.resetHMAC(key, 32);
    hash.update(data, size);
    if (tag != 0)
        hash.update(&tag, 1);
    hash.finalizeHMAC(key, 32, output, 32);
}
