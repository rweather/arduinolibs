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

#include "BigNumberUtil.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class BigNumberUtil BigNumberUtil.h <BigNumberUtil.h>
 * \brief Utilities to assist with implementing big number arithmetic.
 *
 * Big numbers are represented as arrays of limb_t words, which may be
 * 8 bits, 16 bits, or 32 bits in size depending upon how the library
 * was configured.  For AVR, 16 bit limbs usually give the best performance.
 *
 * Limb arrays are ordered from the least significant word to the most
 * significant.
 */

/**
 * \brief Unpacks the little-endian byte representation of a big number
 * into a limb array.
 *
 * \param limbs The limb array, starting with the least significant word.
 * \param count The number of elements in the \a limbs array.
 * \param bytes The bytes to unpack.
 * \param len The number of bytes to unpack.
 *
 * If \a len is shorter than the length of \a limbs, then the high bytes
 * will be filled with zeroes.  If \a len is longer than the length of
 * \a limbs, then the high bytes will be truncated and lost.
 *
 * \sa packLE(), unpackBE()
 */
void BigNumberUtil::unpackLE(limb_t *limbs, size_t count,
                             const uint8_t *bytes, size_t len)
{
#if BIGNUMBER_LIMB_8BIT
    if (len < count) {
        memcpy(limbs, bytes, len);
        memset(limbs + len, 0, count - len);
    } else {
        memcpy(limbs, bytes, count);
    }
#elif CRYPTO_LITTLE_ENDIAN
    count *= sizeof(limb_t);
    if (len < count) {
        memcpy(limbs, bytes, len);
        memset(((uint8_t *)limbs) + len, 0, count - len);
    } else {
        memcpy(limbs, bytes, count);
    }
#elif BIGNUMBER_LIMB_16BIT
    while (count > 0 && len >= 2) {
        *limbs++ = ((limb_t)(bytes[0])) |
                  (((limb_t)(bytes[1])) << 8);
        bytes += 2;
        --count;
        len -= 2;
    }
    if (count > 0 && len == 1) {
        *limbs++ = ((limb_t)(bytes[0]));
        --count;
    }
    while (count > 0) {
        *limbs++ = 0;
        --count;
    }
#elif BIGNUMBER_LIMB_32BIT
    while (count > 0 && len >= 4) {
        *limbs++ = ((limb_t)(bytes[0])) |
                  (((limb_t)(bytes[1])) <<  8) |
                  (((limb_t)(bytes[2])) << 16) |
                  (((limb_t)(bytes[3])) << 24);
        bytes += 4;
        --count;
        len -= 4;
    }
    if (count > 0) {
        if (len == 3) {
            *limbs++ = ((limb_t)(bytes[0])) |
                      (((limb_t)(bytes[1])) <<  8) |
                      (((limb_t)(bytes[2])) << 16);
        } else if (len == 2) {
            *limbs++ = ((limb_t)(bytes[0])) |
                      (((limb_t)(bytes[1])) <<  8);
        } else if (len == 1) {
            *limbs++ = ((limb_t)(bytes[0]));
        }
        --count;
    }
    while (count > 0) {
        *limbs++ = 0;
        --count;
    }
#endif
}

/**
 * \brief Unpacks the big-endian byte representation of a big number
 * into a limb array.
 *
 * \param limbs The limb array, starting with the least significant word.
 * \param count The number of elements in the \a limbs array.
 * \param bytes The bytes to unpack.
 * \param len The number of bytes to unpack.
 *
 * If \a len is shorter than the length of \a limbs, then the high bytes
 * will be filled with zeroes.  If \a len is longer than the length of
 * \a limbs, then the high bytes will be truncated and lost.
 *
 * \sa packBE(), unpackLE()
 */
void BigNumberUtil::unpackBE(limb_t *limbs, size_t count,
                             const uint8_t *bytes, size_t len)
{
#if BIGNUMBER_LIMB_8BIT
    while (count > 0 && len > 0) {
        --count;
        --len;
        *limbs++ = bytes[len];
    }
    memset(limbs, 0, count);
#elif BIGNUMBER_LIMB_16BIT
    bytes += len;
    while (count > 0 && len >= 2) {
        --count;
        bytes -= 2;
        len -= 2;
        *limbs++ = ((limb_t)(bytes[1])) |
                  (((limb_t)(bytes[0])) << 8);
    }
    if (count > 0 && len == 1) {
        --count;
        --bytes;
        *limbs++ = (limb_t)(bytes[0]);
    }
    memset(limbs, 0, count * sizeof(limb_t));
#elif BIGNUMBER_LIMB_32BIT
    bytes += len;
    while (count > 0 && len >= 4) {
        --count;
        bytes -= 4;
        len -= 4;
        *limbs++ = ((limb_t)(bytes[3])) |
                  (((limb_t)(bytes[2])) << 8) |
                  (((limb_t)(bytes[1])) << 16) |
                  (((limb_t)(bytes[0])) << 24);
    }
    if (count > 0) {
        if (len == 3) {
            --count;
            bytes -= 3;
            *limbs++ = ((limb_t)(bytes[2])) |
                      (((limb_t)(bytes[1])) << 8) |
                      (((limb_t)(bytes[0])) << 16);
        } else if (len == 2) {
            --count;
            bytes -= 2;
            *limbs++ = ((limb_t)(bytes[1])) |
                      (((limb_t)(bytes[0])) << 8);
        } else if (len == 1) {
            --count;
            --bytes;
            *limbs++ = (limb_t)(bytes[0]);
        }
    }
    memset(limbs, 0, count * sizeof(limb_t));
#endif
}

/**
 * \brief Packs the little-endian byte representation of a big number
 * into a byte array.
 *
 * \param bytes The byte array to pack into.
 * \param len The number of bytes in the destination \a bytes array.
 * \param limbs The limb array representing the big number, starting with
 * the least significant word.
 * \param count The number of elements in the \a limbs array.
 *
 * If \a len is shorter than the length of \a limbs, then the number will
 * be truncated to the least significant \a len bytes.  If \a len is longer
 * than the length of \a limbs, then the high bytes will be filled with zeroes.
 *
 * \sa unpackLE(), packBE()
 */
void BigNumberUtil::packLE(uint8_t *bytes, size_t len,
                           const limb_t *limbs, size_t count)
{
#if BIGNUMBER_LIMB_8BIT
    if (len <= count) {
        memcpy(bytes, limbs, len);
    } else {
        memcpy(bytes, limbs, count);
        memset(bytes + count, 0, len - count);
    }
#elif CRYPTO_LITTLE_ENDIAN
    count *= sizeof(limb_t);
    if (len <= count) {
        memcpy(bytes, limbs, len);
    } else {
        memcpy(bytes, limbs, count);
        memset(bytes + count, 0, len - count);
    }
#elif BIGNUMBER_LIMB_16BIT
    limb_t word;
    while (count > 0 && len >= 2) {
        word = *limbs++;
        bytes[0] = (uint8_t)word;
        bytes[1] = (uint8_t)(word >> 8);
        --count;
        len -= 2;
        bytes += 2;
    }
    if (count > 0 && len == 1) {
        bytes[0] = (uint8_t)(*limbs);
        --len;
        ++bytes;
    }
    memset(bytes, 0, len);
#elif BIGNUMBER_LIMB_32BIT
    limb_t word;
    while (count > 0 && len >= 4) {
        word = *limbs++;
        bytes[0] = (uint8_t)word;
        bytes[1] = (uint8_t)(word >> 8);
        bytes[2] = (uint8_t)(word >> 16);
        bytes[3] = (uint8_t)(word >> 24);
        --count;
        len -= 4;
        bytes += 4;
    }
    if (count > 0) {
        if (len == 3) {
            word = *limbs;
            bytes[0] = (uint8_t)word;
            bytes[1] = (uint8_t)(word >> 8);
            bytes[2] = (uint8_t)(word >> 16);
            len -= 3;
            bytes += 3;
        } else if (len == 2) {
            word = *limbs;
            bytes[0] = (uint8_t)word;
            bytes[1] = (uint8_t)(word >> 8);
            len -= 2;
            bytes += 2;
        } else if (len == 1) {
            bytes[0] = (uint8_t)(*limbs);
            --len;
            ++bytes;
        }
    }
    memset(bytes, 0, len);
#endif
}

/**
 * \brief Packs the big-endian byte representation of a big number
 * into a byte array.
 *
 * \param bytes The byte array to pack into.
 * \param len The number of bytes in the destination \a bytes array.
 * \param limbs The limb array representing the big number, starting with
 * the least significant word.
 * \param count The number of elements in the \a limbs array.
 *
 * If \a len is shorter than the length of \a limbs, then the number will
 * be truncated to the least significant \a len bytes.  If \a len is longer
 * than the length of \a limbs, then the high bytes will be filled with zeroes.
 *
 * \sa unpackLE(), packBE()
 */
void BigNumberUtil::packBE(uint8_t *bytes, size_t len,
                           const limb_t *limbs, size_t count)
{
#if BIGNUMBER_LIMB_8BIT
    if (len > count) {
        size_t size = len - count;
        memset(bytes, 0, size);
        len -= size;
        bytes += size;
    } else if (len < count) {
        count = len;
    }
    limbs += count;
    while (count > 0) {
        --count;
        *bytes++ = *(--limbs);
    }
#elif BIGNUMBER_LIMB_16BIT
    size_t countBytes = count * sizeof(limb_t);
    limb_t word;
    if (len >= countBytes) {
        size_t size = len - countBytes;
        memset(bytes, 0, size);
        len -= size;
        bytes += size;
        limbs += count;
    } else {
        count = len / sizeof(limb_t);
        limbs += count;
        if ((len & 1) != 0)
            *bytes++ = (uint8_t)(*limbs);
    }
    while (count > 0) {
        --count;
        word = *(--limbs);
        *bytes++ = (uint8_t)(word >> 8);
        *bytes++ = (uint8_t)word;
    }
#elif BIGNUMBER_LIMB_32BIT
    size_t countBytes = count * sizeof(limb_t);
    limb_t word;
    if (len >= countBytes) {
        size_t size = len - countBytes;
        memset(bytes, 0, size);
        len -= size;
        bytes += size;
        limbs += count;
    } else {
        count = len / sizeof(limb_t);
        limbs += count;
        if ((len & 3) == 3) {
            word = *limbs;
            *bytes++ = (uint8_t)(word >> 16);
            *bytes++ = (uint8_t)(word >> 8);
            *bytes++ = (uint8_t)word;
        } else if ((len & 3) == 2) {
            word = *limbs;
            *bytes++ = (uint8_t)(word >> 8);
            *bytes++ = (uint8_t)word;
        } else if ((len & 3) == 1) {
            *bytes++ = (uint8_t)(*limbs);
        }
    }
    while (count > 0) {
        --count;
        word = *(--limbs);
        *bytes++ = (uint8_t)(word >> 24);
        *bytes++ = (uint8_t)(word >> 16);
        *bytes++ = (uint8_t)(word >> 8);
        *bytes++ = (uint8_t)word;
    }
#endif
}
