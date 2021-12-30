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

#include "HKDF.h"
#include <string.h>

/**
 * \class HKDFCommon HKDF.h <HKDF.h>
 * \brief Concrete base class to assist with implementing HKDF mode for
 * hash algorithms.
 *
 * Reference: https://datatracker.ietf.org/doc/html/rfc5869
 *
 * \sa HKDF
 */

/**
 * \brief Constructs a new HKDF instance.
 *
 * This constructor must be followed by a call to setHashAlgorithm().
 */
HKDFCommon::HKDFCommon()
    : hash(0)
    , buf(0)
    , counter(1)
    , posn(255)
{
}

/**
 * \brief Destroys this HKDF instance.
 */
HKDFCommon::~HKDFCommon()
{
}

/**
 * \brief Sets the key and salt for a HKDF session.
 *
 * \param key Points to the key.
 * \param keyLen Length of the \a key in bytes.
 * \param salt Points to the salt.
 * \param saltLen Length of the \a salt in bytes.
 */
void HKDFCommon::setKey(const void *key, size_t keyLen, const void *salt, size_t saltLen)
{
    // Initialise the HKDF context with the key and salt to generate the PRK.
    size_t hashSize = hash->hashSize();
    if (salt && saltLen) {
        hash->resetHMAC(salt, saltLen);
        hash->update(key, keyLen);
        hash->finalizeHMAC(salt, saltLen, buf + hashSize, hashSize);
    } else {
        // If no salt is provided, RFC 5869 says that a string of
        // hashSize zeroes should be used instead.
        memset(buf, 0, hashSize);
        hash->resetHMAC(buf, hashSize);
        hash->update(key, keyLen);
        hash->finalizeHMAC(buf, hashSize, buf + hashSize, hashSize);
    }
    counter = 1;
    posn = hashSize;
}

/**
 * \brief Extracts data from a HKDF session.
 *
 * \param out Points to the buffer to fill with extracted data.
 * \param outLen Number of bytes to extract into the \a out buffer.
 * \param info Points to the application-specific information string.
 * \param infoLen Length of the \a info string in bytes.
 *
 * \note RFC 5869 specifies that a maximum of 255 * HashLen bytes
 * should be extracted from a HKDF session.  This maximum is not
 * enforced by this function.
 */
void HKDFCommon::extract(void *out, size_t outLen, const void *info, size_t infoLen)
{
    size_t hashSize = hash->hashSize();
    uint8_t *outPtr = (uint8_t *)out;
    while (outLen > 0) {
        // Generate a new output block if necessary.
        if (posn >= hashSize) {
            hash->resetHMAC(buf + hashSize, hashSize);
            if (counter != 1)
                hash->update(buf, hashSize);
            if (info && infoLen)
                hash->update(info, infoLen);
            hash->update(&counter, 1);
            hash->finalizeHMAC(buf + hashSize, hashSize, buf, hashSize);
            ++counter;
            posn = 0;
        }

        // Copy as much output data as we can for this block.
        size_t len = hashSize - posn;
        if (len > outLen)
            len = outLen;
        memcpy(outPtr, buf + posn, len);
        posn += len;
        outPtr += len;
        outLen -= len;
    }
}

/**
 * \brief Clears sensitive information from this HKDF instance.
 */
void HKDFCommon::clear()
{
    size_t hashSize = hash->hashSize();
    hash->clear();
    clean(buf, hashSize * 2);
    counter = 1;
    posn = hashSize;
}

/**
 * \fn void HKDFCommon::setHashAlgorithm(Hash *hashAlg, uint8_t *buffer)
 * \brief Sets the hash algorithm to use for HKDF operations.
 *
 * \param hashAlg Points to the hash algorithm instance to use.
 * \param buffer Points to a buffer that must be at least twice the
 * size of the hash output from \a hashAlg.
 */

/**
 * \class HKDF HKDF.h <HKDF.h>
 * \brief Implementation of the HKDF mode for hash algorithms.
 *
 * HKDF expands a key to a larger amount of material that can be used
 * for cryptographic operations.  It is based around a hash algorithm.
 *
 * The template parameter T must be a concrete subclass of Hash
 * indicating the specific hash algorithm to use.
 *
 * The following example expands a 32-byte / 256-bit key into
 * 128 bytes / 1024 bits of key material for use in a cryptographic session:
 *
 * \code
 * uint8_t key[32] = {...};
 * uint8_t output[128];
 * HKDF<SHA256> hkdf;
 * hkdf.setKey(key, sizeof(key));
 * hkdf.extract(output, sizeof(output));
 * \endcode
 *
 * Usually the key will be salted, which can be passed to the setKey()
 * function:
 *
 * \code
 * hkdf.setKey(key, sizeof(key), salt, sizeof(salt));
 * \endcode
 *
 * It is also possible to acheive the same effect with a single function call
 * using the hkdf() templated function:
 *
 * \code
 * hkdf<SHA256>(output, sizeof(output), key, sizeof(key),
 *              salt, sizeof(salt), info, sizeof(info));
 * \endcode
 *
 * Reference: https://datatracker.ietf.org/doc/html/rfc5869
 */

/**
 * \fn HKDF::HKDF()
 * \brief Constructs a new HKDF object for the hash algorithm T.
 */

/**
 * \fn HKDF::~HKDF()
 * \brief Destroys a HKDF instance and all sensitive data within it.
 */

/**
 * \fn void hkdf<T>(void *out, size_t outLen, const void *key, size_t keyLen, const void *salt, size_t saltLen, const void *info, size_t infoLen)
 * \brief All-in-one implementation of HKDF using a hash algorithm.
 *
 * \param out Points to the buffer to fill with extracted data.
 * \param outLen Number of bytes to extract into the \a out buffer.
 * \param key Points to the key.
 * \param keyLen Length of the \a key in bytes.
 * \param salt Points to the salt.
 * \param saltLen Length of the \a salt in bytes.
 * \param info Points to the application-specific information string.
 * \param infoLen Length of the \a info string in bytes.
 *
 * The template parameter T must be a subclass of Hash.
 */
