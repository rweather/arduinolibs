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

#include "BLAKE2b.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include "utility/RotateUtil.h"
#include "utility/ProgMemUtil.h"
#include <string.h>

/**
 * \class BLAKE2b BLAKE2b.h <BLAKE2b.h>
 * \brief BLAKE2b hash algorithm.
 *
 * BLAKE2b is a variation on the ChaCha stream cipher, designed for hashing,
 * with a 512-bit hash output.  It is intended as a high performance
 * replacement for SHA512 for when speed is critical but exact SHA512
 * compatibility is not.
 *
 * Reference: https://blake2.net/
 *
 * \sa BLAKE2s, SHA512
 */

/**
 * \brief Constructs a BLAKE2b hash object.
 */
BLAKE2b::BLAKE2b()
{
    reset();
}

/**
 * \brief Destroys this BLAKE2b hash object after clearing
 * sensitive information.
 */
BLAKE2b::~BLAKE2b()
{
    clean(state);
}

size_t BLAKE2b::hashSize() const
{
    return 64;
}

size_t BLAKE2b::blockSize() const
{
    return 128;
}

// Initialization vectors for BLAKE2b.
#define BLAKE2b_IV0 0x6a09e667f3bcc908ULL
#define BLAKE2b_IV1 0xbb67ae8584caa73bULL
#define BLAKE2b_IV2 0x3c6ef372fe94f82bULL
#define BLAKE2b_IV3 0xa54ff53a5f1d36f1ULL
#define BLAKE2b_IV4 0x510e527fade682d1ULL
#define BLAKE2b_IV5 0x9b05688c2b3e6c1fULL
#define BLAKE2b_IV6 0x1f83d9abfb41bd6bULL
#define BLAKE2b_IV7 0x5be0cd19137e2179ULL

void BLAKE2b::reset()
{
    state.h[0] = BLAKE2b_IV0 ^ 0x01010040; // Default output length of 64.
    state.h[1] = BLAKE2b_IV1;
    state.h[2] = BLAKE2b_IV2;
    state.h[3] = BLAKE2b_IV3;
    state.h[4] = BLAKE2b_IV4;
    state.h[5] = BLAKE2b_IV5;
    state.h[6] = BLAKE2b_IV6;
    state.h[7] = BLAKE2b_IV7;
    state.chunkSize = 0;
    state.finalized = false;
    state.lengthLow = 0;
    state.lengthHigh = 0;
}

/**
 * \brief Resets the hash ready for a new hashing process with a specified
 * output length.
 *
 * \param outputLength The output length to use for the final hash in bytes,
 * between 1 and 64.
 */
void BLAKE2b::reset(uint8_t outputLength)
{
    state.h[0] = BLAKE2b_IV0 ^ 0x01010000 ^ outputLength;
    state.h[1] = BLAKE2b_IV1;
    state.h[2] = BLAKE2b_IV2;
    state.h[3] = BLAKE2b_IV3;
    state.h[4] = BLAKE2b_IV4;
    state.h[5] = BLAKE2b_IV5;
    state.h[6] = BLAKE2b_IV6;
    state.h[7] = BLAKE2b_IV7;
    state.chunkSize = 0;
    state.finalized = false;
    state.lengthLow = 0;
    state.lengthHigh = 0;
}

void BLAKE2b::update(const void *data, size_t len)
{
    // Reset the hashing process if finalize() was called previously.
    if (state.finalized)
        reset();

    // Break the input up into 1024-bit chunks and process each in turn.
    const uint8_t *d = (const uint8_t *)data;
    while (len > 0) {
        if (state.chunkSize == 128) {
            // Previous chunk was full and we know that it wasn't the
            // last chunk, so we can process it now with f0 set to zero.
            processChunk(0);
            state.chunkSize = 0;
        }
        uint8_t size = 128 - state.chunkSize;
        if (size > len)
            size = len;
        memcpy(((uint8_t *)state.m) + state.chunkSize, d, size);
        state.chunkSize += size;
        uint64_t temp = state.lengthLow;
        state.lengthLow += size;
        if (state.lengthLow < temp)
            ++state.lengthHigh;
        len -= size;
        d += size;
    }
}

void BLAKE2b::finalize(void *hash, size_t len)
{
    // Finalize the hash if necessary.
    if (!state.finalized) {
        // Pad the last chunk and hash it with f0 set to all-ones.
        memset(((uint8_t *)state.m) + state.chunkSize, 0, 128 - state.chunkSize);
        processChunk(0xFFFFFFFFFFFFFFFFULL);

        // Convert the hash into little-endian in the message buffer.
        for (uint8_t posn = 0; posn < 8; ++posn)
            state.m[posn] = htole64(state.h[posn]);
        state.finalized = true;
    }

    // Copy the hash to the caller's return buffer.
    if (len > 64)
        len = 64;
    memcpy(hash, state.m, len);
}

void BLAKE2b::clear()
{
    clean(state);
    reset();
}

// Permutation on the message input state for BLAKE2b.
static const uint8_t sigma[12][16] PROGMEM = {
    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15},
    {14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3},
    {11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4},
    { 7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8},
    { 9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13},
    { 2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9},
    {12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11},
    {13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10},
    { 6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5},
    {10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0},
    { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15},
    {14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3},
};

// Perform a BLAKE2b quarter round operation.
#define quarterRound(a, b, c, d, i)    \
    do { \
        uint64_t _b = (b); \
        uint64_t _a = (a) + _b + state.m[pgm_read_byte(&(sigma[index][2 * (i)]))]; \
        uint64_t _d = rightRotate32_64((d) ^ _a); \
        uint64_t _c = (c) + _d; \
        _b = rightRotate24_64(_b ^ _c); \
        _a += _b + state.m[pgm_read_byte(&(sigma[index][2 * (i) + 1]))]; \
        (d) = _d = rightRotate16_64(_d ^ _a); \
        _c += _d; \
        (a) = _a; \
        (b) = rightRotate63_64(_b ^ _c); \
        (c) = _c; \
    } while (0)

void BLAKE2b::processChunk(uint64_t f0)
{
    uint8_t index;

    // Byte-swap the message buffer into little-endian if necessary.
#if !defined(CRYPTO_LITTLE_ENDIAN)
    for (index = 0; index < 16; ++index)
        state.m[index] = le64toh(state.m[index]);
#endif

    // Format the block to be hashed.
    memcpy(state.v, state.h, sizeof(state.h));
    state.v[8]  = BLAKE2b_IV0;
    state.v[9]  = BLAKE2b_IV1;
    state.v[10] = BLAKE2b_IV2;
    state.v[11] = BLAKE2b_IV3;
    state.v[12] = BLAKE2b_IV4 ^ state.lengthLow;
    state.v[13] = BLAKE2b_IV5 ^ state.lengthHigh;
    state.v[14] = BLAKE2b_IV6 ^ f0;
    state.v[15] = BLAKE2b_IV7;

    // Perform the 12 BLAKE2b rounds.
    for (index = 0; index < 12; ++index) {
        // Column round.
        quarterRound(state.v[0], state.v[4], state.v[8],  state.v[12], 0);
        quarterRound(state.v[1], state.v[5], state.v[9],  state.v[13], 1);
        quarterRound(state.v[2], state.v[6], state.v[10], state.v[14], 2);
        quarterRound(state.v[3], state.v[7], state.v[11], state.v[15], 3);

        // Diagonal round.
        quarterRound(state.v[0], state.v[5], state.v[10], state.v[15], 4);
        quarterRound(state.v[1], state.v[6], state.v[11], state.v[12], 5);
        quarterRound(state.v[2], state.v[7], state.v[8],  state.v[13], 6);
        quarterRound(state.v[3], state.v[4], state.v[9],  state.v[14], 7);
    }

    // Combine the new and old hash values.
    for (index = 0; index < 8; ++index)
        state.h[index] ^= (state.v[index] ^ state.v[index + 8]);
}
