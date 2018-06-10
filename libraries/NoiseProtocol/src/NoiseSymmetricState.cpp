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

#include "NoiseSymmetricState.h"

/**
 * \class NoiseSymmetricState NoiseSymmetricState.h <NoiseSymmetricState.h>
 * \brief Abstract base class for the Noise protocol's symmetric state.
 *
 * This class is abstract.  The caller should instantiate a subclass like
 * NoiseSymmetricState_ChaChaPoly_BLAKE2s or NoiseSymmetricState_AESGCM_SHA256
 * to choose the specific algorithms to use in the symmetric state.
 */

/**
 * \fn NoiseSymmetricState::NoiseSymmetricState()
 * \brief Constructs a new symmetric state object.
 */

/**
 * \brief Destroys this symmetric state object.
 */
NoiseSymmetricState::~NoiseSymmetricState()
{
}

/**
 * \fn void NoiseSymmetricState::initialize(const char *protocolName)
 * \brief Initializes a Noise protocol using this symmetric state.
 *
 * \param protocolName The full name of the Noise protocol.
 */

/**
 * \fn bool NoiseSymmetricState::hasKey() const
 * \brief Determine if this symmetric state has an encryption key set.
 *
 * \return Returns true if an encryption key has been set, false if not.
 */

/**
 * \brief Returns the length of MAC values for this symmetric state.
 *
 * \return The base class returns 16 which is the recommended size for
 * MAC values in all Noise protocols.
 *
 * In theory, subclasses could return a smaller value than 16 if MAC values
 * were being truncated to save packet space.  The Noise specification
 * recommends against MAC truncation as it reduces security.
 */
size_t NoiseSymmetricState::macLen() const
{
    return 16;
}

/**
 * \fn void NoiseSymmetricState::mixKey(const void *data, size_t size)
 * \brief Mixes input data into the chaining key.
 *
 * \param data Points to the data to be mixed in.
 * \param size Number of bytes to mix in.
 *
 * \sa mixHash(), mixKeyAndHash()
 */

/**
 * \fn void NoiseSymmetricState::mixHash(const void *data, size_t size)
 * \brief Mixes input data into the handshake hash.
 *
 * \param data Points to the data to be mixed in.
 * \param size Number of bytes to mix in.
 *
 * \sa mixKey(), mixKeyAndHash()
 */

/**
 * \fn void NoiseSymmetricState::mixKeyAndHash(const void *data, size_t size)
 * \brief Mixes input data into the chaining key and handshake hash.
 *
 * \param data Points to the data to be mixed in.
 * \param size Number of bytes to mix in.
 *
 * \sa mixKey(), mixHash()
 */

/**
 * \fn void NoiseSymmetricState::getHandshakeHash(void *data, size_t size)
 * \brief Gets the handshake hash from the symmetric state.
 *
 * \param data Data buffer to fill with the handshake hash.
 * \param size Size of the \a data buffer in bytes.
 *
 * If \a size is less than the size of the symmetric state's hash,
 * then the value will be truncated to the first \a size bytes.
 * If \a size is greater, then the extra bytes will be filled with zeroes.
 */

/**
 * \fn int NoiseSymmetricState::encryptAndHash(uint8_t *output, size_t outputSize, const uint8_t *input, size_t inputSize)
 * \brief Encrypts data and hashes it into the handshake hash.
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
 * The plaintext will be copied directly to the output buffer without a MAC
 * if mixKey() has not been called yet.
 *
 * \sa decryptAndHash()
 */

/**
 * \fn int NoiseSymmetricState::decryptAndHash(uint8_t *output, size_t outputSize, const uint8_t *input, size_t inputSize)
 * \brief Decrypts data and hashes it into the handshake hash.
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
 * The ciphertext will be copied directly to the output buffer without
 * removing a MAC if mixKey() has not been called yet.
 *
 * \sa encryptAndHash()
 */

/**
 * \fn void NoiseSymmetricState::split(NoiseCipherState **c1, NoiseCipherState **c2)
 * \brief Splits the symmetric state into two cipher objects for the
 * transport phase of the Noise session.
 *
 * \param c1 Returns a new cipher object for encrypting traffic from the
 * initiator to the responder.  May be NULL if the object is not required.
 * \param c2 Returns a new cipher object for encrypting traffic from the
 * responder to the initiator.  May be NULL if the object is not required.
 */

/**
 * \fn void NoiseSymmetricState::clear()
 * \brief Clears all sensitive data from this symmetric state object.
 */
