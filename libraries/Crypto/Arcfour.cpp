/*
 * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
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

#include "Arcfour.h"
#include "Crypto.h"

/**
 * \class Arcfour Arcfour.h <Arcfour.h>
 * \brief Implementation of the Arcfour stream cipher.
 *
 * \note While fast and small on 8-bit platforms, Arcfour is a very weak
 * algorithm when used incorrectly.  Security can be improved slightly using
 * drop() and good key generation.  Never reuse the same key with Arcfour.
 *
 * The default key size is 128 bits, but any key size between 40 and
 * 256 bits (5 to 32 bytes) can be used with setKey().
 *
 * This implementation supports the "Arcfour-drop[N]" variant of Arcfour via
 * the drop() function.  This variant is used in many Internet standards
 * because the prefix of the Arcfour keystream can reveal information
 * about the key.
 *
 * Reference: http://en.wikipedia.org/wiki/RC4
 *
 * \sa Cipher
 */

/**
 * \brief Constructs an Arcfour cipher with no initial key.
 *
 * This constructor must be followed by a call to setKey() before the
 * cipher can be used for encryption or decryption.
 */
Arcfour::Arcfour()
{
}

/**
 * \brief Destroys this cipher object after clearing sensitive information.
 */
Arcfour::~Arcfour()
{
    clean(state);
}

/**
 * \brief Default key size for Arcfour in bytes.
 *
 * \return Always returns 16, indicating the default key length of 128 bits.
 *
 * Arcfour can use any key size between 40 and 256 bits (5 to 32 bytes)
 * with the setKey() function.
 *
 * \sa setKey()
 */
size_t Arcfour::keySize() const
{
    return 16;
}

/**
 * \brief Size of the initialization vector for Arcfour.
 *
 * \return Always returns 0 because Arcfour does not use initialization vectors.
 */
size_t Arcfour::ivSize() const
{
    return 0;
}

/**
 * \brief Sets the Arcfour key to use for future encryption and decryption
 * operations.
 *
 * \param key The key which must contain between 5 and 32 bytes,
 * with at least 16 recommended.
 * \param len The length of the key in bytes.
 * \return Returns false if the key length is not between 5 and 32;
 * or true if the key was set successfully.
 *
 * It is a good idea to drop() the prefix from the keystream before using
 * it to encrypt() or decrypt() because the prefix can reveal information
 * about the key.
 *
 * Reference: http://en.wikipedia.org/wiki/RC4#Key-scheduling_algorithm_.28KSA.29
 *
 * \sa drop()
 */
bool Arcfour::setKey(const uint8_t *key, size_t len)
{
    // Check the key length.
    if (len < 5 || len > 32)
        return false;

    // Set up the key schedule.
    size_t i, k;
    uint8_t j, t;
    uint8_t *s = state.s;
    for (i = 0; i < 256; ++i)
        s[i] = i;
    j = 0;
    k = 0;
    for (i = 0; i < 256; ++i) {
        t = s[i];
        j += t + key[k];
        s[i] = s[j];
        s[j] = t;
        if (++k >= len)
            k = 0;
    }
    state.i = 0;
    state.j = 0;
    return true;
}

bool Arcfour::setIV(const uint8_t *, size_t len)
{
    // Initialization vectors are not supported by Arcfour.
    return len == 0;
}

void Arcfour::encrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    while (len > 0) {
        ++state.i;
        state.j += state.s[state.i];
        uint8_t t = state.s[state.i];
        uint8_t u = state.s[state.j];
        state.s[state.i] = u;
        state.s[state.j] = t;
        *output++ = *input++ ^ state.s[(u + t) & 0xFF];
        --len;
    }
}

void Arcfour::decrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    return encrypt(output, input, len);
}

void Arcfour::clear()
{
    clean(state);
}

/**
 * \brief Drops the next \a count bytes of keystream data.
 *
 * \param count The number of bytes to drop.
 *
 * The initial keystream data that emerges from Arcfour after the key is set
 * can reveal information about the key.  This function can be used to
 * drop bytes from the prefix of the keystream until the predictable part
 * has been exhausted.  Encryption can then safely use the keystream that
 * follows the dropped bytes.
 *
 * Reference: http://en.wikipedia.org/wiki/RC4#Fluhrer.2C_Mantin_and_Shamir_attack
 *
 * \sa setKey()
 */
void Arcfour::drop(size_t count)
{
    while (count > 0) {
        ++state.i;
        state.j = state.j + state.s[state.i];
        uint8_t t = state.s[state.i];
        uint8_t u = state.s[state.j];
        state.s[state.i] = u;
        state.s[state.j] = t;
        --count;
    }
}
