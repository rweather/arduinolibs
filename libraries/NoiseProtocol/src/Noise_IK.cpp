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

#include "Noise_IK.h"

/**
 * \class NoiseHandshakeState_IK Noise_IK.h <Noise_IK.h>
 * \brief Handshake implementation of the Noise "IK" pattern.
 *
 * The "IK" pattern provides mutual authentication of the two communicating
 * parties, with the identifying key for the initiating party sent during
 * the handshake.  The identifying key for the responding party is assumed
 * to already be known to the initiator.  If the responding party's key
 * changes, then the application will need to transition to "XXfallback"
 * to discover the new key.
 *
 * This class provides the core "IK" functionality.  Subclasses provide
 * implementations of "IK" that use specific algorithms, such as
 * Noise_IK_25519_ChaChaPoly_BLAKE2s.
 */

/**
 * \fn NoiseHandshakeState_IK::NoiseHandshakeState_IK()
 * \brief Constructs a new Noise handshake that uses the IK pattern.
 */

/**
 * \brief Destroys this Noise handshake.
 */
NoiseHandshakeState_IK::~NoiseHandshakeState_IK()
{
}

void NoiseHandshakeState_IK::writeTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        write_e(packet);
        write_es(packet);
        write_s(packet);
        write_ss(packet);
    } else if (msgnum == 1) {
        write_e(packet);
        write_ee(packet);
        write_se(packet);
        packet.done = true;
    }
}

void NoiseHandshakeState_IK::readTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        read_e(packet);
        read_es(packet);
        read_s(packet);
        read_ss(packet);
    } else if (msgnum == 1) {
        read_e(packet);
        read_ee(packet);
        read_se(packet);
        packet.done = true;
    }
}
