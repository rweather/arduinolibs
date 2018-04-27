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

#include "Ascon128.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include "utility/RotateUtil.h"
#include "utility/ProgMemUtil.h"
#include <string.h>

/**
 * \class Ascon128 Ascon128.h <Ascon128.h>
 * \brief ASCON-128 authenticated cipher.
 *
 * Ascon128 is an authenticated cipher designed for memory-limited
 * environments with a 128-bit key, a 128-bit initialization vector,
 * and a 128-bit authentication tag.  It was one of the finalists
 * in the CAESAR AEAD competition.
 *
 * References: http://competitions.cr.yp.to/round3/asconv12.pdf,
 * http://ascon.iaik.tugraz.at/
 *
 * \sa AuthenticatedCipher
 */

/**
 * \brief Constructs a new Ascon128 authenticated cipher.
 */
Ascon128::Ascon128()
#if defined(CRYPTO_LITTLE_ENDIAN)
    : posn(7)
    , authMode(1)
#else
    : posn(0)
    , authMode(1)
#endif
{
}

/**
 * \brief Destroys this Ascon128 authenticated cipher.
 */
Ascon128::~Ascon128()
{
    clean(state);
}

/**
 * \brief Gets the size of the Ascon128 key in bytes.
 *
 * \return Always returns 16, indicating a 128-bit key.
 */
size_t Ascon128::keySize() const
{
    return 16;
}

/**
 * \brief Gets the size of the Ascon128 initialization vector in bytes.
 *
 * \return Always returns 16, indicating a 128-bit IV.
 *
 * Authentication tags may be truncated to 8 bytes, but the algorithm authors
 * recommend using a full 16-byte tag.
 */
size_t Ascon128::ivSize() const
{
    return 16;
}

/**
 * \brief Gets the size of the Ascon128 authentication tag in bytes.
 *
 * \return Always returns 16, indicating a 128-bit authentication tag.
 */
size_t Ascon128::tagSize() const
{
    return 16;
}

bool Ascon128::setKey(const uint8_t *key, size_t len)
{
    if (len != 16)
        return false;
    memcpy(state.K, key, 16);
#if defined(CRYPTO_LITTLE_ENDIAN)
    state.K[0] = be64toh(state.K[0]);
    state.K[1] = be64toh(state.K[1]);
#endif
    return true;
}

bool Ascon128::setIV(const uint8_t *iv, size_t len)
{
    // Validate the length of the IV.
    if (len != 16)
        return false;

    // Set up the initial state.
    state.S[0] = 0x80400C0600000000ULL;
    state.S[1] = state.K[0];
    state.S[2] = state.K[1];
    memcpy(state.S + 3, iv, 16);
#if defined(CRYPTO_LITTLE_ENDIAN)
    state.S[3] = be64toh(state.S[3]);
    state.S[4] = be64toh(state.S[4]);
    posn = 7;
    authMode = 1;
#else
    posn = 0;
    authMode = 1;
#endif

    // Permute the state with 12 rounds starting at round 0.
    permute(0);

    // XOR the end of the state with the original key.
    state.S[3] ^= state.K[0];
    state.S[4] ^= state.K[1];
    return true;
}

void Ascon128::encrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    if (authMode)
        endAuth();
    const uint8_t *in = (const uint8_t *)input;
    uint8_t *out = (uint8_t *)output;
    while (len > 0) {
        // Encrypt the next byte using the first 64-bit word in the state.
        ((uint8_t *)(state.S))[posn] ^= *in++;
        *out++ = ((const uint8_t *)(state.S))[posn];
        --len;

        // Permute the state for b = 6 rounds at the end of each block.
#if defined(CRYPTO_LITTLE_ENDIAN)
        if (posn > 0) {
            --posn;
        } else {
            permute(6);
            posn = 7;
        }
#else
        if ((++posn) == 8) {
            permute(6);
            posn = 0;
        }
#endif
    }
}

void Ascon128::decrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    if (authMode)
        endAuth();
    const uint8_t *in = (const uint8_t *)input;
    uint8_t *out = (uint8_t *)output;
    while (len > 0) {
        // Decrypt the next byte using the first 64-bit word in the state.
        *out++ = ((const uint8_t *)(state.S))[posn] ^ *in;
        ((uint8_t *)(state.S))[posn] = *in++;
        --len;

        // Permute the state for b = 6 rounds at the end of each block.
#if defined(CRYPTO_LITTLE_ENDIAN)
        if (posn > 0) {
            --posn;
        } else {
            permute(6);
            posn = 7;
        }
#else
        if ((++posn) == 8) {
            permute(6);
            posn = 0;
        }
#endif
    }
}

void Ascon128::addAuthData(const void *data, size_t len)
{
    if (!authMode)
        return;
    const uint8_t *in = (const uint8_t *)data;
    while (len > 0) {
        // Incorporate the next byte of auth data into the internal state.
        ((uint8_t *)(state.S))[posn] ^= *in++;
        --len;

        // Permute the state for b = 6 rounds at the end of each block.
#if defined(CRYPTO_LITTLE_ENDIAN)
        if (posn > 0) {
            --posn;
        } else {
            permute(6);
            posn = 7;
        }
#else
        if ((++posn) == 8) {
            permute(6);
            posn = 0;
        }
#endif
    }
    authMode = 2; // We have some auth data now.
}

void Ascon128::computeTag(void *tag, size_t len)
{
    // End authentication mode if there was no plaintext/ciphertext.
    if (authMode)
        endAuth();

    // Pad the last block, add the original key, and permute the state.
    ((uint8_t *)(state.S))[posn] ^= 0x80;
    state.S[1] ^= state.K[0];
    state.S[2] ^= state.K[1];
    permute(0);

    // Compute the tag and convert it into big-endian in the return buffer.
    uint64_t T[2];
    T[0] = htobe64(state.S[3] ^ state.K[0]);
    T[1] = htobe64(state.S[4] ^ state.K[1]);
    if (len > 16)
        len = 16;
    memcpy(tag, T, len);
    clean(T);
}

bool Ascon128::checkTag(const void *tag, size_t len)
{
    // The tag can never match if it is larger than the maximum allowed size.
    if (len > 16)
        return false;

    // End authentication mode if there was no plaintext/ciphertext.
    if (authMode)
        endAuth();

    // Pad the last block, add the original key, and permute the state.
    ((uint8_t *)(state.S))[posn] ^= 0x80;
    state.S[1] ^= state.K[0];
    state.S[2] ^= state.K[1];
    permute(0);

    // Compute the tag and convert it into big-endian.
    uint64_t T[2];
    T[0] = htobe64(state.S[3] ^ state.K[0]);
    T[1] = htobe64(state.S[4] ^ state.K[1]);
    if (len > 16)
        len = 16;
    bool ok = secure_compare(T, tag, len);
    clean(T);
    return ok;
}

/**
 * \brief Clears all security-sensitive state from this cipher object.
 */
void Ascon128::clear()
{
    clean(state);
#if defined(CRYPTO_LITTLE_ENDIAN)
    posn = 7;
    authMode = 1;
#else
    posn = 0;
    authMode = 1;
#endif
}

#if !defined(__AVR__) || defined(CRYPTO_DOC)

/**
 * \brief Permutes the Ascon128 state.
 *
 * \param first The first round start permuting at, between 0 and 11.
 */
void Ascon128::permute(uint8_t first)
{
    uint64_t t0, t1, t2, t3, t4;
    #define x0 state.S[0]
    #define x1 state.S[1]
    #define x2 state.S[2]
    #define x3 state.S[3]
    #define x4 state.S[4]
    while (first < 12) {
        // Add the round constant to the state.
        x2 ^= ((0x0F - first) << 4) | first;

        // Substitution layer - apply the s-box using bit-slicing
        // according to the algorithm recommended in the specification.
        x0 ^= x4;   x4 ^= x3;   x2 ^= x1;
        t0 = ~x0;   t1 = ~x1;   t2 = ~x2;   t3 = ~x3;   t4 = ~x4;
        t0 &= x1;   t1 &= x2;   t2 &= x3;   t3 &= x4;   t4 &= x0;
        x0 ^= t1;   x1 ^= t2;   x2 ^= t3;   x3 ^= t4;   x4 ^= t0;
        x1 ^= x0;   x0 ^= x4;   x3 ^= x2;   x2 = ~x2;

        // Linear diffusion layer.
        x0 ^= rightRotate19_64(x0) ^ rightRotate28_64(x0);
        x1 ^= rightRotate61_64(x1) ^ rightRotate39_64(x1);
        x2 ^= rightRotate1_64(x2)  ^ rightRotate6_64(x2);
        x3 ^= rightRotate10_64(x3) ^ rightRotate17_64(x3);
        x4 ^= rightRotate7_64(x4)  ^ rightRotate41_64(x4);

        // Move onto the next round.
        ++first;
    }
    #undef x0
    #undef x1
    #undef x2
    #undef x3
    #undef x4
}

#endif // !__AVR__

/**
 * \brief Ends authenticating the associated data and moves onto encryption.
 */
void Ascon128::endAuth()
{
    if (authMode == 2) {
        // We had some auth data, so we need to pad and permute the last block.
        // There is no need to do this if there were zero bytes of auth data.
        ((uint8_t *)(state.S))[posn] ^= 0x80;
        permute(6);
    }
    state.S[4] ^= 1; // Domain separation between auth data and payload data.
    authMode = 0;
#if defined(CRYPTO_LITTLE_ENDIAN)
    posn = 7;
#else
    posn = 0;
#endif
}
