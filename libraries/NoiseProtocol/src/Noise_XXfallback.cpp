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

#include "Noise_XXfallback.h"

/**
 * \class NoiseHandshakeState_XXfallback Noise_XXfallback.h <Noise_XXfallback.h>
 * \brief Handshake implementation of the Noise "XXfallback" pattern.
 *
 * The "XXfallback" pattern provides mutual authentication of the two
 * communicating parties after a previous "IK" handshake has failed.
 * The application should use startFallback() instead of start() to
 * start the new handshake.
 *
 * This class provides the core "XXfallback" functionality.  Subclasses provide
 * implementations of "XXfallback" that use specific algorithms, such as
 * Noise_XXfallback_25519_ChaChaPoly_BLAKE2s.
 */

/**
 * \fn NoiseHandshakeState_XXfallback::NoiseHandshakeState_XXfallback()
 * \brief Constructs a new Noise handshake that uses the XXfallback pattern.
 */

/**
 * \brief Destroys this Noise handshake.
 */
NoiseHandshakeState_XXfallback::~NoiseHandshakeState_XXfallback()
{
}

bool NoiseHandshakeState_XXfallback::startFallback
    (const NoiseHandshakeState *fallbackFrom, Noise::Party party,
     const void *prologue, size_t prologueLen)
{
    // Cannot fallback if no previous handshake, or fall back from ourselves.
    if (!fallbackFrom || fallbackFrom == this) {
        setState(Noise::Failed);
        return false;
    }
    NoiseDHState *thisDH = dhState();

    // Copy keys from the previous handshake into this one.
    const NoiseDHState *otherDH = otherDHState(fallbackFrom);
    if (!thisDH || !otherDH || !thisDH->fallback(party, otherDH)) {
        setState(Noise::Failed);
        return false;
    }

    // Start the new handshake.
    start(party, prologue, prologueLen);

    // The responder writes first in a XXfallback handshake.
    setState(party == Noise::Initiator ? Noise::Read : Noise::Write);
    return true;
}

void NoiseHandshakeState_XXfallback::removeKeys()
{
    // Remove the remote static public key only.  We need to keep the
    // ephemeral keys to perform the fallback correctly.
    removeParameter(Noise::RemoteStaticPublicKey);
}

void NoiseHandshakeState_XXfallback::writeTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        premessage(packet, Noise::LocalEphemPublicKey,
                   Noise::RemoteEphemPublicKey);
        write_e(packet);
        write_ee(packet);
        write_s(packet);
        write_es(packet);
    } else if (msgnum == 1) {
        write_s(packet);
        write_se(packet);
        packet.done = true;
    }
}

void NoiseHandshakeState_XXfallback::readTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        premessage(packet, Noise::LocalEphemPublicKey,
                   Noise::RemoteEphemPublicKey);
        read_e(packet);
        read_ee(packet);
        read_s(packet);
        read_es(packet);
    } else if (msgnum == 1) {
        read_s(packet);
        read_se(packet);
        packet.done = true;
    }
}
