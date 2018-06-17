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

#include "NoiseDHState_Curve25519.h"
#include "NoiseSymmetricState.h"
#include "Curve25519.h"
#include "Crypto.h"
#include "RNG.h"
#include <string.h>

/**
 * \class NoiseDHState_Curve25519 NoiseDHState_Curve25519.h <NoiseDHState_Curve25519.h>
 * \brief Noise protocol interface to the Curve25519 algorithm.
 */

/**
 * \class NoiseDHState_Curve25519_EphemOnly NoiseDHState_Curve25519.h <NoiseDHState_Curve25519.h>
 * \brief Noise protocol interface to the ephemeral-only Curve25519 algorithm.
 *
 * This class is intended for use with the "NN" pattern which does not
 * require static keys.  It saves 96 bytes of memory compared with
 * NoiseDHState_Curve25519.
 */

#define HAVE_25519_LOCAL_EPHEM_PUBLIC   0x01
#define HAVE_25519_LOCAL_EPHEM_PRIVATE  0x02
#define HAVE_25519_LOCAL_STATIC_PUBLIC  0x04
#define HAVE_25519_LOCAL_STATIC_PRIVATE 0x08
#define HAVE_25519_REMOTE_EPHEM_PUBLIC  0x10
#define HAVE_25519_REMOTE_STATIC_PUBLIC 0x20

/**
 * \brief Constructs a new ephemeral-only Curve25519 DH object.
 */
NoiseDHState_Curve25519_EphemOnly::NoiseDHState_Curve25519_EphemOnly()
{
    memset(&st, 0, sizeof(st));
}

/**
 * \brief Destroys this ephemeral-only Curve25519 DH object.
 */
NoiseDHState_Curve25519_EphemOnly::~NoiseDHState_Curve25519_EphemOnly()
{
    clean(st);
}

bool NoiseDHState_Curve25519_EphemOnly::setParameter
    (Noise::Parameter id, const void *value, size_t size)
{
    switch (id) {
    case Noise::LocalEphemPrivateKey:
    case Noise::LocalEphem25519PrivateKey:
        if (!copyParameterIn(st.le, sizeof(st.le), value, size))
            break;
        st.le[0] &= 0xF8;
        st.le[31] = (st.le[31] & 0x7F) | 0x40;
        Curve25519::eval(st.lf, st.le, 0);
        st.flags |= HAVE_25519_LOCAL_EPHEM_PRIVATE |
                    HAVE_25519_LOCAL_EPHEM_PUBLIC;
        return true;
    case Noise::LocalEphemKeyPair:
    case Noise::LocalEphem25519KeyPair:
        if (size != 64)
            break;
        if (!copyParameterIn(st.le, sizeof(st.le), value, 32))
            break;
        if (!copyParameterIn
                (st.lf, sizeof(st.lf), ((const uint8_t *)value) + 32, 32))
            break;
        st.flags |= HAVE_25519_LOCAL_EPHEM_PRIVATE |
                    HAVE_25519_LOCAL_EPHEM_PUBLIC;
        return true;
    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        if (!copyParameterIn(st.re, sizeof(st.re), value, size))
            break;
        st.flags |= HAVE_25519_REMOTE_EPHEM_PUBLIC;
        return true;
    default: break;
    }
    return false;
}

size_t NoiseDHState_Curve25519_EphemOnly::getParameter
    (Noise::Parameter id, void *value, size_t maxSize) const
{
    switch (id) {
    case Noise::LocalEphemPrivateKey:
    case Noise::LocalEphem25519PrivateKey:
        if (!(st.flags & HAVE_25519_LOCAL_EPHEM_PRIVATE))
            break;
        return copyParameterOut(value, maxSize, st.le, sizeof(st.le));
    case Noise::LocalEphemPublicKey:
    case Noise::LocalEphem25519PublicKey:
        if (!(st.flags & HAVE_25519_LOCAL_EPHEM_PUBLIC))
            break;
        return copyParameterOut(value, maxSize, st.lf, sizeof(st.lf));
    case Noise::LocalEphemKeyPair:
    case Noise::LocalEphem25519KeyPair:
        if (!(st.flags & HAVE_25519_LOCAL_EPHEM_PRIVATE))
            break;
        if (maxSize < 64)
            break;
        if (copyParameterOut(value, 32, st.le, sizeof(st.le)) != 32)
            break;
        if (copyParameterOut
                (((uint8_t *)value) + 32, 32, st.lf, sizeof(st.lf)) != 32)
            break;
        return 64;
    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        if (!(st.flags & HAVE_25519_REMOTE_EPHEM_PUBLIC))
            break;
        return copyParameterOut(value, maxSize, st.re, sizeof(st.re));
    default: break;
    }
    return 0;
}

size_t NoiseDHState_Curve25519_EphemOnly::getParameterSize(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalEphemKeyPair:
    case Noise::LocalEphem25519KeyPair:
        return 64;

    case Noise::LocalEphemPrivateKey:
    case Noise::LocalEphem25519PrivateKey:
    case Noise::LocalEphemPublicKey:
    case Noise::LocalEphem25519PublicKey:
    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        return 32;

    default:
        return 0;
    }
}

bool NoiseDHState_Curve25519_EphemOnly::hasParameter(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalEphemKeyPair:
    case Noise::LocalEphemPrivateKey:
    case Noise::LocalEphem25519KeyPair:
    case Noise::LocalEphem25519PrivateKey:
        return (st.flags & HAVE_25519_LOCAL_EPHEM_PRIVATE) != 0;
    case Noise::LocalEphemPublicKey:
    case Noise::LocalEphem25519PublicKey:
        return (st.flags & HAVE_25519_LOCAL_EPHEM_PUBLIC) != 0;
    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        return (st.flags & HAVE_25519_REMOTE_EPHEM_PUBLIC) != 0;
    default: break;
    }
    return false;
}

void NoiseDHState_Curve25519_EphemOnly::removeParameter(Noise::Parameter id)
{
    switch (id) {
    case Noise::LocalEphemKeyPair:
    case Noise::LocalEphemPrivateKey:
    case Noise::LocalEphemPublicKey:
    case Noise::LocalEphem25519KeyPair:
    case Noise::LocalEphem25519PrivateKey:
    case Noise::LocalEphem25519PublicKey:
        st.flags &= ~(HAVE_25519_LOCAL_EPHEM_PRIVATE |
                      HAVE_25519_LOCAL_EPHEM_PUBLIC);
        break;

    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        st.flags &= ~HAVE_25519_REMOTE_EPHEM_PUBLIC;
        break;

    default: break;
    }
}

bool NoiseDHState_Curve25519_EphemOnly::hashPublicKey
    (NoiseSymmetricState *sym, Noise::Parameter id)
{
    switch (id) {
    case Noise::LocalEphemPublicKey:
    case Noise::LocalEphem25519PublicKey:
        if (!(st.flags & HAVE_25519_LOCAL_EPHEM_PUBLIC))
            break;
        sym->mixHash(st.lf, sizeof(st.lf));
        return true;
    case Noise::RemoteEphemPublicKey:
    case Noise::RemoteEphem25519PublicKey:
        if (!(st.flags & HAVE_25519_REMOTE_EPHEM_PUBLIC))
            break;
        sym->mixHash(st.re, sizeof(st.re));
        return true;
    default: break;
    }
    return false;
}

size_t NoiseDHState_Curve25519_EphemOnly::sharedKeySize() const
{
    return 32;
}

void NoiseDHState_Curve25519_EphemOnly::generateLocalEphemeralKeyPair()
{
    // For testing, the ephemeral key pair can be provided ahead of
    // time by the test harness.  Only create a new pair if we need it.
    if (!(st.flags & HAVE_25519_LOCAL_EPHEM_PRIVATE)) {
        RNG.rand(st.le, 32);
        st.le[0] &= 0xF8;
        st.le[31] = (st.le[31] & 0x7F) | 0x40;
        Curve25519::eval(st.lf, st.le, 0);
    }
}

void NoiseDHState_Curve25519_EphemOnly::ee(uint8_t *sharedKey)
{
    Curve25519::eval(sharedKey, st.le, st.re);
}

// es(), se(), and ss() shouldn't be called for ephemeral-only
// handshakes so we simply stub them out.  If they are called
// by accident then zeroing the sharedKey ensures that we don't
// accidentally leak information about previous memory contents.

void NoiseDHState_Curve25519_EphemOnly::es(uint8_t *sharedKey)
{
    memset(sharedKey, 0, 32);
}

void NoiseDHState_Curve25519_EphemOnly::se(uint8_t *sharedKey)
{
    memset(sharedKey, 0, 32);
}

void NoiseDHState_Curve25519_EphemOnly::ss(uint8_t *sharedKey)
{
    memset(sharedKey, 0, 32);
}

void NoiseDHState_Curve25519_EphemOnly::clear()
{
    clean(st);
}

/**
 * \brief Constructs a new Curve25519 DH object.
 */
NoiseDHState_Curve25519::NoiseDHState_Curve25519()
{
    memset(&st2, 0, sizeof(st2));
}

/**
 * \brief Destroys this Curve25519 DH object.
 */
NoiseDHState_Curve25519::~NoiseDHState_Curve25519()
{
    clean(st2);
}

bool NoiseDHState_Curve25519::setParameter
    (Noise::Parameter id, const void *value, size_t size)
{
    switch (id) {
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
        if (!copyParameterIn(st2.ls, sizeof(st2.ls), value, size))
            break;
        st2.ls[0] &= 0xF8;
        st2.ls[31] = (st2.ls[31] & 0x7F) | 0x40;
        Curve25519::eval(st2.lp, st2.ls, 0);
        st.flags |= HAVE_25519_LOCAL_STATIC_PRIVATE |
                    HAVE_25519_LOCAL_STATIC_PUBLIC;
        return true;
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        if (size != 64)
            break;
        if (!copyParameterIn(st2.ls, sizeof(st2.ls), value, 32))
            break;
        if (!copyParameterIn
                (st2.lp, sizeof(st2.lp), ((const uint8_t *)value) + 32, 32))
            break;
        st.flags |= HAVE_25519_LOCAL_STATIC_PRIVATE |
                    HAVE_25519_LOCAL_STATIC_PUBLIC;
        return true;
    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        if (!copyParameterIn(st2.rs, sizeof(st2.rs), value, size))
            break;
        st.flags |= HAVE_25519_REMOTE_STATIC_PUBLIC;
        return true;
    default:
        return NoiseDHState_Curve25519_EphemOnly::setParameter(id, value, size);
    }
    return false;
}

size_t NoiseDHState_Curve25519::getParameter
    (Noise::Parameter id, void *value, size_t maxSize) const
{
    switch (id) {
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
        if (!(st.flags & HAVE_25519_LOCAL_STATIC_PRIVATE))
            break;
        return copyParameterOut(value, maxSize, st2.ls, sizeof(st2.ls));
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        if (!(st.flags & HAVE_25519_LOCAL_STATIC_PUBLIC))
            break;
        return copyParameterOut(value, maxSize, st2.lp, sizeof(st2.lp));
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        if (!(st.flags & HAVE_25519_LOCAL_STATIC_PRIVATE))
            break;
        if (maxSize < 64)
            break;
        if (copyParameterOut(value, 32, st2.ls, sizeof(st2.ls)) != 32)
            break;
        if (copyParameterOut
                (((uint8_t *)value) + 32, 32, st2.lp, sizeof(st2.lp)) != 32)
            break;
        return 64;
    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        if (!(st.flags & HAVE_25519_REMOTE_STATIC_PUBLIC))
            break;
        return copyParameterOut(value, maxSize, st2.rs, sizeof(st2.rs));
    default:
        return NoiseDHState_Curve25519_EphemOnly::getParameter
            (id, value, maxSize);
    }
    return 0;
}

size_t NoiseDHState_Curve25519::getParameterSize(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        return 64;

    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        return 32;

    default:
        return NoiseDHState_Curve25519_EphemOnly::getParameterSize(id);
    }
}

bool NoiseDHState_Curve25519::hasParameter(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStatic25519PrivateKey:
        return (st.flags & HAVE_25519_LOCAL_STATIC_PRIVATE) != 0;
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        return (st.flags & HAVE_25519_LOCAL_STATIC_PUBLIC) != 0;
    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        return (st.flags & HAVE_25519_REMOTE_STATIC_PUBLIC) != 0;
    default:
        return NoiseDHState_Curve25519_EphemOnly::getParameterSize(id);
    }
    return false;
}

void NoiseDHState_Curve25519::removeParameter(Noise::Parameter id)
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStatic25519PublicKey:
        st.flags &= ~(HAVE_25519_LOCAL_STATIC_PRIVATE |
                      HAVE_25519_LOCAL_STATIC_PUBLIC);
        break;

    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        st.flags &= ~HAVE_25519_REMOTE_STATIC_PUBLIC;
        break;

    default:
        NoiseDHState_Curve25519_EphemOnly::removeParameter(id);
        break;
    }
}

bool NoiseDHState_Curve25519::hashPublicKey
    (NoiseSymmetricState *sym, Noise::Parameter id)
{
    switch (id) {
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        if (!(st.flags & HAVE_25519_LOCAL_STATIC_PUBLIC))
            break;
        sym->mixHash(st2.lp, sizeof(st2.lp));
        return true;
    case Noise::RemoteStaticPublicKey:
    case Noise::RemoteStatic25519PublicKey:
        if (!(st.flags & HAVE_25519_REMOTE_STATIC_PUBLIC))
            break;
        sym->mixHash(st2.rs, sizeof(st2.rs));
        return true;
    default:
        return NoiseDHState_Curve25519_EphemOnly::hashPublicKey(sym, id);
    }
    return false;
}

void NoiseDHState_Curve25519::es(uint8_t *sharedKey)
{
    Curve25519::eval(sharedKey, st.le, st2.rs);
}

void NoiseDHState_Curve25519::se(uint8_t *sharedKey)
{
    Curve25519::eval(sharedKey, st2.ls, st.re);
}

void NoiseDHState_Curve25519::ss(uint8_t *sharedKey)
{
    Curve25519::eval(sharedKey, st2.ls, st2.rs);
}

void NoiseDHState_Curve25519::clear()
{
    NoiseDHState_Curve25519_EphemOnly::clear();
    clean(st2);
}
