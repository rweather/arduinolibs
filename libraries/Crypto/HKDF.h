/*
 * Copyright (C) 2022 Southern Storm Software, Pty Ltd.
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

#ifndef CRYPTO_HKDF_h
#define CRYPTO_HKDF_h

#include "Hash.h"
#include "Crypto.h"

class HKDFCommon
{
public:
    virtual ~HKDFCommon();

    void setKey(const void *key, size_t keyLen, const void *salt = 0, size_t saltLen = 0);

    void extract(void *out, size_t outLen, const void *info = 0, size_t infoLen = 0);

    void clear();

protected:
    HKDFCommon();
    void setHashAlgorithm(Hash *hashAlg, uint8_t *buffer)
    {
        hash = hashAlg;
        buf = buffer;
    }

private:
    Hash *hash;
    uint8_t *buf;
    uint8_t counter;
    uint8_t posn;
};

template <typename T>
class HKDF : public HKDFCommon
{
public:
    HKDF() { setHashAlgorithm(&hashAlg, buffer); }
    ~HKDF() { ::clean(buffer, sizeof(buffer)); }

private:
    T hashAlg;
    uint8_t buffer[T::HASH_SIZE * 2];
};

template <typename T> void hkdf
    (void *out, size_t outLen, const void *key, size_t keyLen,
     const void *salt, size_t saltLen, const void *info, size_t infoLen)
{
    HKDF<T> context;
    context.setKey(key, keyLen, salt, saltLen);
    context.extract(out, outLen, info, infoLen);
}

#endif
