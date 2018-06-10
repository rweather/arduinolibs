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

#include "Noise_XX.h"

/**
 * \class NoiseHandshakeState_XX Noise_XX.h <Noise_XX.h>
 * \brief Handshake implementation of the Noise "XX" pattern.
 *
 * The "XX" pattern provides mutual authentication of the two communicating
 * parties, with the identifying keys for the parties exchanged during
 * the handshake.
 *
 * This class provides the core "XX" functionality.  Subclasses provide
 * implementations of "XX" that use specific algorithms, such as
 * Noise_XX_25519_ChaChaPoly_BLAKE2s.
 */

/**
 * \fn NoiseHandshakeState_XX::NoiseHandshakeState_XX()
 * \brief Constructs a new Noise handshake that uses the XX pattern.
 */

/**
 * \brief Destroys this Noise handshake.
 */
NoiseHandshakeState_XX::~NoiseHandshakeState_XX()
{
}

void NoiseHandshakeState_XX::removeKeys()
{
    // Remote static key is expected to be discovered during the XX handshake.
    // Local static key is preserved because we will eventually need to
    // send it to the remote party during the XX handshake.
    removeParameter(Noise::RemoteStaticPublicKey);

    // Base class implementation removes the ephemeral keys.
    NoiseHandshakeState::removeKeys();
}

void NoiseHandshakeState_XX::writeTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        write_e(packet);
    } else if (msgnum == 1) {
        write_e(packet);
        write_ee(packet);
        write_s(packet);
        write_es(packet);
    } else if (msgnum == 2) {
        write_s(packet);
        write_se(packet);
        packet.done = true;
    }
}

void NoiseHandshakeState_XX::readTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        read_e(packet);
    } else if (msgnum == 1) {
        read_e(packet);
        read_ee(packet);
        read_s(packet);
        read_es(packet);
    } else if (msgnum == 2) {
        read_s(packet);
        read_se(packet);
        packet.done = true;
    }
}
