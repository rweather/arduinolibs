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

#include "KeccakCore.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include "utility/RotateUtil.h"
#include "utility/ProgMemUtil.h"
#include <string.h>

/**
 * \class KeccakCore KeccakCore.h <KeccakCore.h>
 * \brief Keccak core sponge function.
 *
 * KeccakCore provides the core sponge function for different capacities.
 * It is used to implement Hash algorithms such as SHA3.
 *
 * References: http://en.wikipedia.org/wiki/SHA-3
 *
 * \sa SHA3
 */

/**
 * \brief Constructs a new Keccak sponge function.
 *
 * The capacity() will initially be set to 1536, which normally won't be
 * of much use to the caller.  The constructor should be followed by a
 * call to setCapacity() to select the capacity of interest.
 */
KeccakCore::KeccakCore()
    : _blockSize(8)
{
    memset(state.A, 0, sizeof(state.A));
    state.inputSize = 0;
    state.outputSize = 0;
}

/**
 * \brief Destroys this Keccak sponge function after clearing all
 * sensitive information.
 */
KeccakCore::~KeccakCore()
{
    clean(state);
}

/**
 * \brief Returns the capacity of the sponge function in bits.
 *
 * \sa setCapacity(), blockSize()
 */
size_t KeccakCore::capacity() const
{
    return 1600 - ((size_t)_blockSize) * 8;
}

/**
 * \brief Sets the capacity of the Keccak sponge function in bits.
 *
 * \param capacity The capacity of the Keccak sponge function in bits which
 * should be a multiple of 64 and between 64 and 1536.
 *
 * \note It is possible to create a sponge function with this constructor that
 * doesn't strictly conform with the capacity and hash size constraints
 * defined in the relevant standards.  It is the responsibility of callers
 * to only use standard parameter combinations.
 *
 * \sa capacity(), blockSize()
 */
void KeccakCore::setCapacity(size_t capacity)
{
    _blockSize = (1600 - capacity) / 8;
    reset();
}

/**
 * \fn size_t KeccakCore::blockSize() const
 * \brief Returns the input block size for the sponge function in bytes.
 *
 * The block size is (1600 - capacity()) / 8.
 *
 * \sa capacity()
 */

/**
 * \brief Resets the Keccak sponge function ready for a new session.
 *
 * \sa update(), extract()
 */
void KeccakCore::reset()
{
    memset(state.A, 0, sizeof(state.A));
    state.inputSize = 0;
    state.outputSize = 0;
}

/**
 * \brief Updates the Keccak sponge function with more input data.
 *
 * \param data The extra input data to incorporate.
 * \param size The size of the new data to incorporate.
 *
 * This function will invoke the sponge function whenever a full blockSize()
 * bytes of input data have been accumulated.  Call pad() after the last
 * block to finalize the input before calling extract().
 *
 * \sa pad(), extract(), reset()
 */
void KeccakCore::update(const void *data, size_t size)
{
    // Stop generating output while we incorporate the new data.
    state.outputSize = 0;

    // Break the input up into chunks and process each in turn.
    const uint8_t *d = (const uint8_t *)data;
#if !defined(CRYPTO_LITTLE_ENDIAN)
    uint64_t *Awords = &(state.A[0][0]);
    uint8_t index, index2;
#endif
    while (size > 0) {
        uint8_t len = _blockSize - state.inputSize;
        if (len > size)
            len = size;
#if defined(CRYPTO_LITTLE_ENDIAN)
        uint8_t *Abytes = ((uint8_t *)state.A) + state.inputSize;
        for (uint8_t posn = 0; posn < len; ++posn)
            Abytes[posn] ^= d[posn];
#else
        index2 = state.inputSize;
        for (index = 0; index < len; ++index) {
            Awords[index2 / 8] ^= (((uint64_t)d[index]) << ((index2 % 8) * 8));
            ++index2;
        }
#endif
        state.inputSize += len;
        size -= len;
        d += len;
        if (state.inputSize == _blockSize) {
            keccakp();
            state.inputSize = 0;
        }
    }
}

/**
 * \brief Pads the last block of input data to blockSize().
 *
 * \param tag The tag byte to add to the padding to identify SHA3 (0x06),
 * SHAKE (0x1F), or the plain pre-standardized version of Keccak (0x01).
 *
 * The sponge function will be invoked to process the completed padding block.
 *
 * \sa update(), extract()
 */
void KeccakCore::pad(uint8_t tag)
{
    // Padding for SHA3-NNN variants according to FIPS 202 appends "01",
    // then another "1", then many zero bits, followed by a final "1".
    // SHAKE appends "1111" first instead of "01".  Note that SHA-3 numbers
    // bits from the least significant, so appending "01" is equivalent
    // to 0x02 for byte-aligned data, not 0x40.
    uint8_t size = state.inputSize;
    uint64_t *Awords = &(state.A[0][0]);
    Awords[size / 8] ^= (((uint64_t)tag) << ((size % 8) * 8));
    Awords[(_blockSize - 1) / 8] ^= 0x8000000000000000ULL;
    keccakp();
    state.inputSize = 0;
    state.outputSize = 0;
}

/**
 * \brief Extracts data from the Keccak sponge function.
 *
 * \param data The data buffer to fill with extracted data.
 * \param size The number number of bytes of extracted data that are required.
 *
 * If more than blockSize() bytes are required, the sponge function will
 * be invoked to generate additional data.
 *
 * \sa update(), reset(), extractHash()
 */
void KeccakCore::extract(void *data, size_t size)
{
#if !defined(CRYPTO_LITTLE_ENDIAN)
    uint8_t index, index2;
    const uint64_t *Awords = &(state.A[0][0]);
#endif

    // Stop accepting input while we are generating output.
    state.inputSize = 0;

    // Copy the output data into the caller's return buffer.
    uint8_t *d = (uint8_t *)data;
    uint8_t tempSize;
    while (size > 0) {
        // Generate another output block if the current one has been exhausted.
        if (state.outputSize >= _blockSize) {
            keccakp();
            state.outputSize = 0;
        }

        // How many bytes can we copy this time around?
        tempSize = _blockSize - state.outputSize;
        if (tempSize > size)
            tempSize = size;

        // Copy the partial output data into the caller's return buffer.
#if defined(CRYPTO_LITTLE_ENDIAN)
        memcpy(d, ((uint8_t *)(state.A)) + state.outputSize, tempSize);
#else
        index2 = state.outputSize;
        for (index = 0; index < tempSize; ++index) {
            d[index] = (uint8_t)(Awords[index2 / 8] >> ((index2 % 8) * 8));
            ++index2;
        }
#endif
        state.outputSize += tempSize;
        size -= tempSize;
        d += tempSize;
    }
}

/**
 * \brief Clears all sensitive data from this object.
 */
void KeccakCore::clear()
{
    clean(state);
}

/**
 * \brief Sets a HMAC key for a Keccak-based hash algorithm.
 *
 * \param key Points to the HMAC key for the hashing process.
 * \param len Length of the HMAC \a key in bytes.
 * \param pad Inner (0x36) or outer (0x5C) padding value to XOR with
 * the formatted HMAC key.
 * \param hashSize The size of the output from the hash algorithm.
 *
 * This function is intended to help classes implement Hash::resetHMAC() and
 * Hash::finalizeHMAC() by directly formatting the HMAC key into the
 * internal block buffer and resetting the hash.
 */
void KeccakCore::setHMACKey(const void *key, size_t len, uint8_t pad, size_t hashSize)
{
    uint8_t *b = (uint8_t *)state.B;
    size_t size = blockSize();
    reset();
    if (len <= size) {
        memcpy(b, key, len);
    } else {
        update(key, len);
        this->pad(0x06);
        extract(b, hashSize);
        len = hashSize;
        reset();
    }
    memset(b + len, pad, size - len);
    while (len > 0) {
        *b++ ^= pad;
        --len;
    }
    update(state.B, size);
}

/**
 * \brief Transform the state with the KECCAK-p sponge function with b = 1600.
 */
void KeccakCore::keccakp()
{
    static const uint8_t addMod5Table[9] PROGMEM = {
        0, 1, 2, 3, 4, 0, 1, 2, 3
    };
    #define addMod5(x, y) (pgm_read_byte(&(addMod5Table[(x) + (y)])))
    uint64_t D;
    uint8_t index, index2;
    for (uint8_t round = 0; round < 24; ++round) {
        // Step mapping theta.  The specification mentions two temporary
        // arrays of size 5 called C and D.  To save a bit of memory,
        // we use the first row of B to store C and compute D on the fly.
        for (index = 0; index < 5; ++index) {
            state.B[0][index] = state.A[0][index] ^ state.A[1][index] ^
                                state.A[2][index] ^ state.A[3][index] ^
                                state.A[4][index];
        }
        for (index = 0; index < 5; ++index) {
            D = state.B[0][addMod5(index, 4)] ^
                leftRotate1_64(state.B[0][addMod5(index, 1)]);
            for (index2 = 0; index2 < 5; ++index2)
                state.A[index2][index] ^= D;
        }

        // Step mapping rho and pi combined into a single step.
        // Rotate all lanes by a specific offset and rearrange.
        state.B[0][0] = state.A[0][0];
        state.B[1][0] = leftRotate28_64(state.A[0][3]);
        state.B[2][0] = leftRotate1_64 (state.A[0][1]);
        state.B[3][0] = leftRotate27_64(state.A[0][4]);
        state.B[4][0] = leftRotate62_64(state.A[0][2]);
        state.B[0][1] = leftRotate44_64(state.A[1][1]);
        state.B[1][1] = leftRotate20_64(state.A[1][4]);
        state.B[2][1] = leftRotate6_64 (state.A[1][2]);
        state.B[3][1] = leftRotate36_64(state.A[1][0]);
        state.B[4][1] = leftRotate55_64(state.A[1][3]);
        state.B[0][2] = leftRotate43_64(state.A[2][2]);
        state.B[1][2] = leftRotate3_64 (state.A[2][0]);
        state.B[2][2] = leftRotate25_64(state.A[2][3]);
        state.B[3][2] = leftRotate10_64(state.A[2][1]);
        state.B[4][2] = leftRotate39_64(state.A[2][4]);
        state.B[0][3] = leftRotate21_64(state.A[3][3]);
        state.B[1][3] = leftRotate45_64(state.A[3][1]);
        state.B[2][3] = leftRotate8_64 (state.A[3][4]);
        state.B[3][3] = leftRotate15_64(state.A[3][2]);
        state.B[4][3] = leftRotate41_64(state.A[3][0]);
        state.B[0][4] = leftRotate14_64(state.A[4][4]);
        state.B[1][4] = leftRotate61_64(state.A[4][2]);
        state.B[2][4] = leftRotate18_64(state.A[4][0]);
        state.B[3][4] = leftRotate56_64(state.A[4][3]);
        state.B[4][4] = leftRotate2_64 (state.A[4][1]);

        // Step mapping chi.  Combine each lane with two other lanes in its row.
        for (index = 0; index < 5; ++index) {
            for (index2 = 0; index2 < 5; ++index2) {
                state.A[index2][index] =
                    state.B[index2][index] ^
                    ((~state.B[index2][addMod5(index, 1)]) &
                     state.B[index2][addMod5(index, 2)]);
            }
        }

        // Step mapping iota.  XOR A[0][0] with the round constant.
        static uint64_t const RC[24] PROGMEM = {
            0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808AULL,
            0x8000000080008000ULL, 0x000000000000808BULL, 0x0000000080000001ULL,
            0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008AULL,
            0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000AULL,
            0x000000008000808BULL, 0x800000000000008BULL, 0x8000000000008089ULL,
            0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
            0x000000000000800AULL, 0x800000008000000AULL, 0x8000000080008081ULL,
            0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
        };
        state.A[0][0] ^= pgm_read_qword(RC + round);
    }
}
