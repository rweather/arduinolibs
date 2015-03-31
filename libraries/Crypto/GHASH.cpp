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

#include "GHASH.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class GHASH GHASH.h <GHASH.h>
 * \brief Implementation of the GHASH message authenticator.
 *
 * GHASH is the message authentication part of Galois Counter Mode (GCM).
 *
 * \note GHASH is not the same as GMAC.  GHASH implements the low level
 * hashing primitive that is used by both GCM and GMAC.  GMAC can be
 * simulated using GCM and an empty plaintext/ciphertext.
 *
 * References: <a href="http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf">NIST SP 800-38D</a>,
 * http://en.wikipedia.org/wiki/Galois/Counter_Mode
 *
 * \sa GCM
 */

/**
 * \brief Constructs a new GHASH message authenticator.
 */
GHASH::GHASH()
{
    state.posn = 0;
}

/**
 * \brief Destroys this GHASH message authenticator.
 */
GHASH::~GHASH()
{
    clean(state);
}

/**
 * \brief Resets the GHASH message authenticator for a new session.
 *
 * \param key Points to the 16 byte authentication key.
 *
 * \sa update(), finalize()
 */
void GHASH::reset(const void *key)
{
    // Copy the key into H and convert from big endian to host order.
    memcpy(state.H, key, 16);
#if defined(CRYPTO_LITTLE_ENDIAN)
    state.H[0] = be32toh(state.H[0]);
    state.H[1] = be32toh(state.H[1]);
    state.H[2] = be32toh(state.H[2]);
    state.H[3] = be32toh(state.H[3]);
#endif

    // Reset the hash.
    memset(state.Y, 0, sizeof(state.Y));
    state.posn = 0;
}

/**
 * \brief Updates the message authenticator with more data.
 *
 * \param data Data to be hashed.
 * \param len Number of bytes of data to be hashed.
 *
 * If finalize() has already been called, then the behavior of update() will
 * be undefined.  Call reset() first to start a new authentication process.
 *
 * \sa pad(), reset(), finalize()
 */
void GHASH::update(const void *data, size_t len)
{
    // XOR the input with state.Y in 128-bit chunks and process them.
    const uint8_t *d = (const uint8_t *)data;
    while (len > 0) {
        uint8_t size = 16 - state.posn;
        if (size > len)
            size = len;
        uint8_t *y = ((uint8_t *)state.Y) + state.posn;
        for (uint8_t i = 0; i < size; ++i)
            y[i] ^= d[i];
        state.posn += size;
        len -= size;
        d += size;
        if (state.posn == 16) {
            processChunk();
            state.posn = 0;
        }
    }
}

/**
 * \brief Finalizes the authentication process and returns the token.
 *
 * \param token The buffer to return the token value in.
 * \param len The length of the \a token buffer between 0 and 16.
 *
 * If \a len is less than 16, then the token value will be truncated to
 * the first \a len bytes.  If \a len is greater than 16, then the remaining
 * bytes will left unchanged.
 *
 * If finalize() is called again, then the returned \a token value is
 * undefined.  Call reset() first to start a new authentication process.
 *
 * \sa reset(), update()
 */
void GHASH::finalize(void *token, size_t len)
{
    // Pad with zeroes to a multiple of 16 bytes.
    pad();

    // The token is the current value of Y.
    if (len > 16)
        len = 16;
    memcpy(token, state.Y, len);
}

/**
 * \brief Pads the input stream with zero bytes to a multiple of 16.
 *
 * \sa update()
 */
void GHASH::pad()
{
    if (state.posn != 0) {
        // Padding involves XOR'ing the rest of state.Y with zeroes,
        // which does nothing.  Immediately process the next chunk.
        processChunk();
        state.posn = 0;
    }
}

/**
 * \brief Clears the authenticator's state, removing all sensitive data.
 */
void GHASH::clear()
{
    clean(state);
}

void GHASH::processChunk()
{
    uint32_t Z0 = 0;            // Z = 0
    uint32_t Z1 = 0;
    uint32_t Z2 = 0;
    uint32_t Z3 = 0;
    uint32_t V0 = state.H[0];   // V = H
    uint32_t V1 = state.H[1];
    uint32_t V2 = state.H[2];
    uint32_t V3 = state.H[3];

    // Multiply Z by V for the set bits in Y, starting at the top.
    // This is a very simple bit by bit version that may not be very
    // fast but it should be resistant to cache timing attacks.
    for (uint8_t posn = 0; posn < 16; ++posn) {
        uint8_t value = ((const uint8_t *)state.Y)[posn];
        for (uint8_t bit = 0; bit < 8; ++bit, value <<= 1) {
            // Extract the high bit of "value" and turn it into a mask.
            uint32_t mask = (~((uint32_t)(value >> 7))) + 1;

            // XOR V with Z if the bit is 1.
            Z0 ^= (V0 & mask);
            Z1 ^= (V1 & mask);
            Z2 ^= (V2 & mask);
            Z3 ^= (V3 & mask);

            // Rotate V right by 1 bit.
            mask = ((~(V3 & 0x01)) + 1) & 0xE1000000;
            V3 = (V3 >> 1) | (V2 << 31);
            V2 = (V2 >> 1) | (V1 << 31);
            V1 = (V1 >> 1) | (V0 << 31);
            V0 = (V0 >> 1) ^ mask;
        }
    }

    // We have finished the block so copy Z into Y and byte-swap.
    state.Y[0] = htobe32(Z0);
    state.Y[1] = htobe32(Z1);
    state.Y[2] = htobe32(Z2);
    state.Y[3] = htobe32(Z3);
}
