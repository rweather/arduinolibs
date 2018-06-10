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

#include "NoiseCipherState.h"

/**
 * \class NoiseCipherState NoiseCipherState.h <NoiseCipherState.h>
 * \brief Abstract base class for the Noise protocol's cipher state.
 *
 * This class is abstract.  The caller should instantiate a subclass like
 * NoiseCipherState_ChaChaPoly or NoiseCipherState_AESGCM to choose the
 * specific algorithm to use in the cipher state.
 *
 * Normally the application will not instantiate instances of this
 * class or its subclasses directly.  Instead, it will call
 * NoiseHandshakeState::split() at the end of the handshake phase
 * of a connection.
 */

/**
 * \fn NoiseCipherState::NoiseCipherState()
 * \brief Constructs a new cipher object.
 */

/**
 * \brief Destroys this cipher object.
 */
NoiseCipherState::~NoiseCipherState()
{
}

/**
 * \fn void NoiseCipherState::setNonce(uint64_t nonce)
 * \brief Sets the nonce value for the next packet to be encrypted or
 * decrypted.
 *
 * \param nonce The nonce value between 0 and 2^64 - 2.
 *
 * You won't need to use this function on reliable transports like TCP.
 * You may need to use this function on unreliable transports like UDP.
 * Care must be taken not to use the same nonce with multiple packets,
 * and to reject duplicate packets.
 */

/**
 * \fn int NoiseCipherState::encryptPacket(void *output, size_t outputSize, const void *input, size_t inputSize)
 * \brief Encrypts a packet in a Noise transport session.
 *
 * \param output Output buffer to write the ciphertext and MAC to.
 * \param outputSize Available space in the \a output buffer.
 * \param input Input buffer containing the plaintext to be encrypted.
 * \param inputSize Number of bytes of plaintext.
 *
 * \return The number of bytes that were written to \a output.
 * \return -1 if \a outputSize is not large enough to contain the
 * ciphertext and MAC.
 *
 * MAC values will add an extra 16 bytes to the size of the packet.
 *
 * \sa decryptPacket()
 */

/**
 * \fn int NoiseCipherState::decryptPacket(void *output, size_t outputSize, const void *input, size_t inputSize)
 * \brief Decrypts a packet in a Noise transport session.
 *
 * \param output Output buffer to write the plaintext to.
 * \param outputSize Available space in the \a output buffer.
 * \param input Input buffer containing the ciphertext and MAC.
 * \param inputSize Number of bytes of ciphertext plus the MAC.
 *
 * \return The number of bytes that were written to \a output.
 * \return -1 if \a outputSize is not large enough to contain the plaintext.
 * \return -1 if the MAC check failed; the plaintext output will be invalid.
 *
 * \sa encryptPacket()
 */

/**
 * \fn void NoiseCipherState::rekey()
 * \brief Rekey the cipher object.
 *
 * This function should be called periodically to refresh the key that is
 * used to encrypt data.  This avoids the same key being used over and over
 * as the more data that is encrypted with the same key, the more likely
 * statistical trends may reveal themselves to an attacker.
 */

/**
 * \fn void NoiseCipherState::clear()
 * \brief Destroys sensitive data in this cipher object.
 *
 * After this function is called, no more packets can be encrypted or
 * decrypted and a new cipher object must be created.
 */
