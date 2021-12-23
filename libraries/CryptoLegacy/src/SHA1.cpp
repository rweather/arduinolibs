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

#include "SHA1.h"
#include "Crypto.h"
#include "utility/RotateUtil.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class SHA1 SHA1.h <SHA1.h>
 * \brief SHA-1 hash algorithm.
 *
 * Reference: http://en.wikipedia.org/wiki/SHA-1
 *
 * \sa SHA256, SHA384, SHA512
 */

/**
 * \var SHA1::HASH_SIZE
 * \brief Constant for the size of the hash output of SHA1.
 */

/**
 * \var SHA1::BLOCK_SIZE
 * \brief Constant for the block size of SHA1.
 */

/**
 * \brief Constructs a SHA-1 hash object.
 */
SHA1::SHA1()
{
    reset();
}

/**
 * \brief Destroys this SHA-1 hash object after clearing sensitive information.
 */
SHA1::~SHA1()
{
    clean(state);
}

size_t SHA1::hashSize() const
{
    return 20;
}

size_t SHA1::blockSize() const
{
    return 64;
}

void SHA1::reset()
{
    state.h[0] = 0x67452301;
    state.h[1] = 0xEFCDAB89;
    state.h[2] = 0x98BADCFE;
    state.h[3] = 0x10325476;
    state.h[4] = 0xC3D2E1F0;
    state.chunkSize = 0;
    state.length = 0;
}

void SHA1::update(const void *data, size_t len)
{
    // Update the total length (in bits, not bytes).
    state.length += ((uint64_t)len) << 3;

    // Break the input up into 512-bit chunks and process each in turn.
    const uint8_t *d = (const uint8_t *)data;
    while (len > 0) {
        uint8_t size = 64 - state.chunkSize;
        if (size > len)
            size = len;
        memcpy(((uint8_t *)state.w) + state.chunkSize, d, size);
        state.chunkSize += size;
        len -= size;
        d += size;
        if (state.chunkSize == 64) {
            processChunk();
            state.chunkSize = 0;
        }
    }
}

void SHA1::finalize(void *hash, size_t len)
{
    // Pad the last chunk.  We may need two padding chunks if there
    // isn't enough room in the first for the padding and length.
    uint8_t *wbytes = (uint8_t *)state.w;
    if (state.chunkSize <= (64 - 9)) {
        wbytes[state.chunkSize] = 0x80;
        memset(wbytes + state.chunkSize + 1, 0x00, 64 - 8 - (state.chunkSize + 1));
        state.w[14] = htobe32((uint32_t)(state.length >> 32));
        state.w[15] = htobe32((uint32_t)state.length);
        processChunk();
    } else {
        wbytes[state.chunkSize] = 0x80;
        memset(wbytes + state.chunkSize + 1, 0x00, 64 - (state.chunkSize + 1));
        processChunk();
        memset(wbytes, 0x00, 64 - 8);
        state.w[14] = htobe32((uint32_t)(state.length >> 32));
        state.w[15] = htobe32((uint32_t)state.length);
        processChunk();
    }

    // Convert the result into big endian and return it.
    for (uint8_t posn = 0; posn < 5; ++posn)
        state.w[posn] = htobe32(state.h[posn]);

    // Copy the hash to the caller's return buffer.
    if (len > 20)
        len = 20;
    memcpy(hash, state.w, len);
}

void SHA1::clear()
{
    clean(state);
    reset();
}

void SHA1::resetHMAC(const void *key, size_t keyLen)
{
    formatHMACKey(state.w, key, keyLen, 0x36);
    state.length += 64 * 8;
    processChunk();
}

void SHA1::finalizeHMAC(const void *key, size_t keyLen, void *hash, size_t hashLen)
{
    uint8_t temp[20];
    finalize(temp, sizeof(temp));
    formatHMACKey(state.w, key, keyLen, 0x5C);
    state.length += 64 * 8;
    processChunk();
    update(temp, sizeof(temp));
    finalize(hash, hashLen);
    clean(temp);
}

/**
 * \brief Processes a single 512-bit chunk with the core SHA-1 algorithm.
 *
 * Reference: http://en.wikipedia.org/wiki/SHA-1
 */
void SHA1::processChunk()
{
    uint8_t index;

    // Convert the first 16 words from big endian to host byte order.
    for (index = 0; index < 16; ++index)
        state.w[index] = be32toh(state.w[index]);

    // Initialize the hash value for this chunk.
    uint32_t a = state.h[0];
    uint32_t b = state.h[1];
    uint32_t c = state.h[2];
    uint32_t d = state.h[3];
    uint32_t e = state.h[4];

    // Perform the first 16 rounds of the compression function main loop.
    uint32_t temp;
    for (index = 0; index < 16; ++index) {
        temp = leftRotate5(a) + ((b & c) | ((~b) & d)) + e + 0x5A827999 + state.w[index];
        e = d;
        d = c;
        c = leftRotate30(b);
        b = a;
        a = temp;
    }

    // Perform the 64 remaining rounds.  We expand the first 16 words to
    // 80 in-place in the "w" array.  This saves 256 bytes of memory
    // that would have otherwise need to be allocated to the "w" array.
    for (; index < 20; ++index) {
        temp = state.w[index & 0x0F] = leftRotate1
            (state.w[(index - 3) & 0x0F] ^ state.w[(index - 8) & 0x0F] ^
             state.w[(index - 14) & 0x0F] ^ state.w[(index - 16) & 0x0F]);
        temp = leftRotate5(a) + ((b & c) | ((~b) & d)) + e + 0x5A827999 + temp;
        e = d;
        d = c;
        c = leftRotate30(b);
        b = a;
        a = temp;
    }
    for (; index < 40; ++index) {
        temp = state.w[index & 0x0F] = leftRotate1
            (state.w[(index - 3) & 0x0F] ^ state.w[(index - 8) & 0x0F] ^
             state.w[(index - 14) & 0x0F] ^ state.w[(index - 16) & 0x0F]);
        temp = leftRotate5(a) + (b ^ c ^ d) + e + 0x6ED9EBA1 + temp;
        e = d;
        d = c;
        c = leftRotate30(b);
        b = a;
        a = temp;
    }
    for (; index < 60; ++index) {
        temp = state.w[index & 0x0F] = leftRotate1
            (state.w[(index - 3) & 0x0F] ^ state.w[(index - 8) & 0x0F] ^
             state.w[(index - 14) & 0x0F] ^ state.w[(index - 16) & 0x0F]);
        temp = leftRotate5(a) + ((b & c) | (b & d) | (c & d)) + e + 0x8F1BBCDC + temp;
        e = d;
        d = c;
        c = leftRotate30(b);
        b = a;
        a = temp;
    }
    for (; index < 80; ++index) {
        temp = state.w[index & 0x0F] = leftRotate1
            (state.w[(index - 3) & 0x0F] ^ state.w[(index - 8) & 0x0F] ^
             state.w[(index - 14) & 0x0F] ^ state.w[(index - 16) & 0x0F]);
        temp = leftRotate5(a) + (b ^ c ^ d) + e + 0xCA62C1D6 + temp;
        e = d;
        d = c;
        c = leftRotate30(b);
        b = a;
        a = temp;
    }

    // Add this chunk's hash to the result so far.
    state.h[0] += a;
    state.h[1] += b;
    state.h[2] += c;
    state.h[3] += d;
    state.h[4] += e;

    // Attempt to clean up the stack.
    a = b = c = d = e = temp = 0;
}
