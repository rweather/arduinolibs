/*
 * Copyright (C) 2016 Miguel Azevedo, <lobaoazevedo@ua.pt>.
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

/**
 * \class HKDF HKDF.h <HKDF.h>
 * \brief HKDF support for the hashing algorithms.
 *
 * Reference: RFC5869 - https://tools.ietf.org/html/rfc5869
 *
 * \sa Hash, SHA256
 */

template < typename T >
class HKDF
{
public:
    ~HKDF() { delete & hash; }

    /**
     * \brief First phase of HKDF.
     * \brief Extracts a pseudorandom key from a given key and salt.
     *  \param salt Optional (might be NULL) salt value (a non-secret random value);
     * 	\param saltLen The salt length in bytes. If salt is NUll this value is ignored.
     *  \param inputKey Input keying material.
     *	\param keyLen inputKey length in bytes.
     *	\param outputKey a previous allocated buffer of the same length of the
     *   hash function being used.
     *
     * Reference: RFC5869 - https://tools.ietf.org/html/rfc5869
     *
     * \sa expand()
     */
    void extract(void *salt, size_t saltLen, void *inputKey, size_t keyLen, void *outputKey)
    {
	uint8_t s[hash.hashSize()];
	if (salt == NULL) {
	    memset(s, 0, hash.hashSize());
	    salt = s;
	    saltLen = hash.hashSize();
	}

	hash.resetHMAC(salt, saltLen);
	hash.update(inputKey, keyLen);
	hash.finalizeHMAC(salt, saltLen, outputKey, hash.hashSize());
    }

    /**
     * \brief Second phase of HKDF.
     * 	\brief Expands a given pseudorandom key using a given info.
     *  \param inputKey The pseudorandom key extracted with extract().
     *	\param info The optional (can be NULL) context and application specific information.
     *	\param infoLen The length of info. If info is NULL this value is ignored.
     *	\param outputKey Output keying material. A buffer of L lenght bytes.
     *	\param L The length of outputKey in bytes.
     *
     * \sa extract()
     */
    void expand(void *inputKey, void *info, size_t infoLen,void *outputKey, size_t L)
    {
	int N = L / hash.hashSize();
	uint8_t i;

	infoLen = (info == NULL) ? 0 : infoLen;
	size_t saltLen = hash.hashSize() + infoLen + 1;
	uint8_t salt[saltLen];
	salt[saltLen - 1] = 1;

	if (info != NULL) {
	    memcpy(salt + hash.hashSize(), info, infoLen);
	}
	// Calculate T(1)
	hash.resetHMAC(inputKey, hash.hashSize());
	hash.update(salt + hash.hashSize(), infoLen + 1);
	hash.finalizeHMAC(inputKey, hash.hashSize(), outputKey, hash.hashSize());

	salt[saltLen - 1] += 1;
	memcpy(salt, outputKey, hash.hashSize());

	// Calculate T(2) ... T(N)
	for (i = 1; i < N; i++) {
	    hash.resetHMAC(inputKey, hash.hashSize());
	    hash.update(salt, saltLen);
	    hash.finalizeHMAC(inputKey,
			      hash.hashSize(),
			      ((uint8_t *) outputKey) + (i * hash.hashSize()),
			      hash.hashSize());

	    salt[saltLen - 1] += 1;
	    memcpy(salt,
		   ((uint8_t *) outputKey) + (i * hash.hashSize()),
		   hash.hashSize());
	}

	// Process remaining octets if there are any.
	if (L % hash.hashSize()) {
	    uint8_t rslt[hash.hashSize()];
	    int remain = L - N * hash.hashSize();
	    hash.resetHMAC(inputKey, hash.hashSize());
	    hash.update(salt, saltLen);
	    hash.finalizeHMAC(inputKey,
			      hash.hashSize(),
			      rslt,
			      hash.hashSize());

	    memcpy(((uint8_t *) outputKey) + (N * hash.hashSize()),
		   rslt,
		   remain);
	}
    }


  private:
      T hash;
};

#endif
