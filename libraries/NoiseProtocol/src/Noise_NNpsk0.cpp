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

#include "Noise_NNpsk0.h"

/**
 * \class NoiseHandshakeState_NNpsk0 Noise_NNpsk0.h <Noise_NNpsk0.h>
 * \brief Handshake implementation of the Noise "NNpsk0" pattern.
 *
 * The "NNpsk0" pattern is intended for use in place of "XX" when
 * the parties have a pre-shared symmetric key (or "PSK").
 *
 * This class provides the core "NNpsk0" functionality.  Subclasses provide
 * implementations of "NNpsk0" that use specific algorithms, such as
 * Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s.
 */

/**
 * \fn NoiseHandshakeState_NNpsk0::NoiseHandshakeState_NNpsk0()
 * \brief Constructs a new Noise handshake that uses the NNpsk0 pattern.
 */

/**
 * \brief Destroys this Noise handshake.
 */
NoiseHandshakeState_NNpsk0::~NoiseHandshakeState_NNpsk0()
{
}

void NoiseHandshakeState_NNpsk0::writeTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        write_psk(packet);
        write_e(packet);
    } else if (msgnum == 1) {
        write_e(packet);
        write_ee(packet);
        packet.done = true;
    }
}

void NoiseHandshakeState_NNpsk0::readTokens
    (NoiseHandshakeState::Packet &packet, uint8_t msgnum)
{
    if (msgnum == 0) {
        read_psk(packet);
        read_e(packet);
    } else if (msgnum == 1) {
        read_e(packet);
        read_ee(packet);
        packet.done = true;
    }
}
