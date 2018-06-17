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

#ifndef NOISE_DH_STATE_CURVE25519_h
#define NOISE_DH_STATE_CURVE25519_h

#include "NoiseDHState.h"

class NoiseDHState_Curve25519;

class NoiseDHState_Curve25519_EphemOnly : public NoiseDHState
{
public:
    NoiseDHState_Curve25519_EphemOnly();
    virtual ~NoiseDHState_Curve25519_EphemOnly();

    bool setParameter(Noise::Parameter id, const void *value, size_t size);
    size_t getParameter(Noise::Parameter id, void *value, size_t maxSize) const;
    size_t getParameterSize(Noise::Parameter id) const;
    bool hasParameter(Noise::Parameter id) const;
    void removeParameter(Noise::Parameter id);

    bool hashPublicKey(NoiseSymmetricState *sym, Noise::Parameter id);

    size_t sharedKeySize() const;

    void generateLocalEphemeralKeyPair();

    void ee(uint8_t *sharedKey);
    void es(uint8_t *sharedKey);
    void se(uint8_t *sharedKey);
    void ss(uint8_t *sharedKey);

    void clear();

private:
    struct {
        uint8_t le[32];     // Local ephemeral private key.
        uint8_t lf[32];     // Local ephemeral public key.
        uint8_t re[32];     // Remote ephemeral public key.
        uint8_t flags;      // Indicates which keys are present.
    } st;

    friend class NoiseDHState_Curve25519;
};

class NoiseDHState_Curve25519 : public NoiseDHState_Curve25519_EphemOnly
{
public:
    NoiseDHState_Curve25519();
    virtual ~NoiseDHState_Curve25519();

    bool setParameter(Noise::Parameter id, const void *value, size_t size);
    size_t getParameter(Noise::Parameter id, void *value, size_t maxSize) const;
    size_t getParameterSize(Noise::Parameter id) const;
    bool hasParameter(Noise::Parameter id) const;
    void removeParameter(Noise::Parameter id);

    bool hashPublicKey(NoiseSymmetricState *sym, Noise::Parameter id);

    void es(uint8_t *sharedKey);
    void se(uint8_t *sharedKey);
    void ss(uint8_t *sharedKey);

    void clear();

private:
    struct {
        uint8_t ls[32];     // Local static private key.
        uint8_t lp[32];     // Local static public key.
        uint8_t rs[32];     // Remote static public key.
    } st2;
};

#endif
