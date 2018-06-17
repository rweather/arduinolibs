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

#ifndef NOISE_DH_STATE_h
#define NOISE_DH_STATE_h

#include "NoiseNamespace.h"
#include <stddef.h>
#include <inttypes.h>

class NoiseSymmetricState;

class NoiseDHState
{
public:
    NoiseDHState() {}
    virtual ~NoiseDHState();

    virtual bool setParameter
        (Noise::Parameter id, const void *value, size_t size) = 0;
    virtual size_t getParameter
        (Noise::Parameter id, void *value, size_t maxSize) const = 0;
    virtual size_t getParameterSize(Noise::Parameter id) const = 0;
    virtual bool hasParameter(Noise::Parameter id) const = 0;
    virtual void removeParameter(Noise::Parameter id) = 0;

    virtual bool hashPublicKey
        (NoiseSymmetricState *sym, Noise::Parameter id) = 0;

    virtual size_t sharedKeySize() const = 0;

    virtual void generateLocalEphemeralKeyPair() = 0;

    virtual void ee(uint8_t *sharedKey) = 0;
    virtual void es(uint8_t *sharedKey) = 0;
    virtual void se(uint8_t *sharedKey) = 0;
    virtual void ss(uint8_t *sharedKey) = 0;

    virtual void clear() = 0;

protected:
    static bool copyParameterIn
        (void *output, size_t expectedSize, const void *value, size_t size);
    static size_t copyParameterOut
        (void *output, size_t maxOutput, const void *value, size_t size);
};

#endif
