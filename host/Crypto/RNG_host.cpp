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

#include "RNG.h"
#include "Crypto.h"
#include "ChaCha.h"
#include <string.h>
#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#endif

// Host emulation for the RNG class, using the native random number generator.

#define RNG_ROUNDS          20
#define RNG_REKEY_BLOCKS    16
#define RNG_MORE_ENTROPY    16384

static const char tagRNG[16] = {
    'e', 'x', 'p', 'a', 'n', 'd', ' ', '3',
    '2', '-', 'b', 'y', 't', 'e', ' ', 'k'
};

RNGClass::RNGClass()
    : timer(0)
{
    memcpy(block, tagRNG, sizeof(tagRNG));
    memset(block + 4, 0, 48);
}

RNGClass::~RNGClass()
{
    clean(block);
    clean(stream);
}

void RNGClass::begin(const char *tag)
{
}

void RNGClass::addNoiseSource(NoiseSource &source)
{
}

void RNGClass::setAutoSaveTime(uint16_t minutes)
{
}

// Get more entropy from the underlying operating system.
static void getMoreEntropy(uint8_t *data, size_t len)
{
#if defined(__linux__)
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("/dev/urandom");
        exit(1);
    }
    int ret = read(fd, data, len);
    if (ret != (int)len) {
        if (ret < 0)
            perror("/dev/urandom");
        close(fd);
        exit(1);
    }
    close(fd);
#else
    #warning "TODO: No random entropy on this system!"
    memset(data, 0xAA, len);
#endif
}

void RNGClass::rand(uint8_t *data, size_t len)
{
    // Generate the random data.
    uint8_t count = 0;
    while (len > 0) {
        // Get more entropy from the operating system if necessary.
        if (!timer) {
            getMoreEntropy((uint8_t *)(block + 4), 48);
            rekey();
            timer = RNG_MORE_ENTROPY;
        }

        // Force a rekey if we have generated too many blocks in this request.
        if (count >= RNG_REKEY_BLOCKS) {
            rekey();
            count = 1;
        } else {
            ++count;
        }

        // Increment the low counter word and generate a new keystream block.
        ++(block[12]);
        ChaCha::hashCore(stream, block, RNG_ROUNDS);

        // Copy the data to the return buffer.
        size_t size = (len < 64) ? len : 64;
        memcpy(data, stream, size);
        data += size;
        len -= size;

        // Keep track of the number of bytes we have generated so that
        // we can fetch more entropy from the operating system if necessary.
        if (timer >= size)
            timer -= size;
        else
            timer = 0;
    }

    // Force a rekey after every request.
    rekey();
}

bool RNGClass::available(size_t len) const
{
    return true;
}

void RNGClass::stir(const uint8_t *data, size_t len, unsigned int credit)
{
}

void RNGClass::save()
{
}

void RNGClass::loop()
{
}

void RNGClass::destroy()
{
}

void RNGClass::rekey()
{
    ++(block[12]);
    ChaCha::hashCore(stream, block, RNG_ROUNDS);
    memcpy(block + 4, stream, 48);
}

RNGClass RNG;
