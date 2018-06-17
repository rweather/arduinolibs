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

#include "NoiseDHState.h"
#include <string.h>

/**
 * \class NoiseDHState NoiseDHState.h <NoiseDHState.h>
 * \brief Abstract base class for Diffie-Hellman (DH) algorithms for Noise.
 *
 * This API defines how NoiseHandshakeState interacts with the DH
 * algorithm to set local keys, query remote keys, generate ephemeral
 * key pairs, etc.  Subclasses implement specific DH algorithms, such as
 * NoiseDHState_Curve25519.
 */

/**
 * \fn NoiseDHState::NoiseDHState()
 * \brief Constructs a new DH state object.
 */

/**
 * \brief Destroys this DH state object.
 */
NoiseDHState::~NoiseDHState()
{
}

/**
 * \fn bool NoiseDHState::setParameter(Noise::Parameter id, const void *value, size_t size)
 * \brief Sets a parameter on the DH object.
 *
 * \param id Identifies the parameter.
 * \param value Points to the value to set for the parameter.
 * \param size Size of the parameter value in bytes.
 *
 * \return Returns true if the parameter is set; false if the parameter
 * \a id is not supported by the DH object; or false if the \a size is
 * not valid for the parameter.
 *
 * If the \a id is for the private key component of a key pair, then the
 * public key component will be generated from \a value.  Returns false if
 * it is not possible to generate the public key component from the
 * private key component.
 *
 * \sa getParameter(), getParameterSize()
 */

/**
 * \fn size_t NoiseDHState::getParameter(Noise::Parameter id, void *value, size_t maxSize) const
 * \brief Gets the value associated with a parameter on the DH object.
 *
 * \param id Identifies the parameter.
 * \param value Points to the buffer to write the parameter value to.
 * \param maxSize Maximum size of the \a value buffer in bytes.
 *
 * \return The number of bytes that were written to \a value.
 * \return 0 if \a id is not supported by the DH object.
 * \return 0 if \a maxSize is not large enough to hold the parameter value.
 * \return 0 if the parameter \a id has not been set yet.
 *
 * The getParameterSize() function can be used to determine the actual
 * size of the parameter so that the caller can allocate a buffer large
 * enough to contain it ahead of time.
 *
 * \sa setParameter(), getParameterSize()
 */

/**
 * \fn size_t NoiseDHState::getParameterSize(Noise::Parameter id) const
 * \brief Gets the size of a parameter on the DH object.
 *
 * \param id Identifies the parameter.
 *
 * \return The parameter's size, or zero if \a id is not valid for
 * the DH object.
 *
 * If the parameter is variable-length, then this function will return
 * the actual length of the value, which may be zero if the value has
 * not been set yet.
 *
 * \sa getParameter(), setParameter(), hasParameter()
 */

/**
 * \fn bool NoiseDHState::hasParameter(Noise::Parameter id) const
 * \brief Determine if a parameter on this DH object has a value set.
 *
 * \param id Identifies the parameter.
 *
 * \return Returns true if \a id has a value; false if \a id does not
 * have a value; false if \a id is not valid for the DH object.
 *
 * \sa getParameterSize(), getParameter(), removeParameter()
 */

/**
 * \fn void NoiseDHState::removeParameter(Noise::Parameter id)
 * \brief Removes a parameter from this DH object.
 *
 * \param id Identifies the parameter.  If the parameter is unknown
 * to the DH object, the request will be ignored.
 *
 * \sa hasParameter()
 */

/**
 * \fn bool NoiseDHState::hashPublicKey(NoiseSymmetricState *sym, Noise::Parameter id)
 * \brief Hashes a public key into the symmetric state.
 *
 * \param sym Symmetric state to hash the public key into.
 * \param id Identifies the public key to hash.
 *
 * \return Returns true if the key was hashed; false if \a id is not supported
 * by the DH object the \a id does not have a value at present.
 */

/**
 * \fn size_t NoiseDHState::sharedKeySize() const
 * \brief Gets the size of the shared keys that will be generated by
 * ee(), es(), se(), and ss().
 *
 * \return The size of the shared keys in bytes.
 *
 * \sa ee(), es(), se(), ss()
 */

/**
 * \fn void NoiseDHState::generateLocalEphemeralKeyPair()
 * \brief Generates the local ephemeral key pair during a handshake.
 *
 * For testing purposes, the application can use setParameterKeyPair()
 * to specify an ephemeral key after NoiseHandshakeState::start() and
 * before this function is called.  This permits testing with fixed
 * key values.  This functionality should not be used in real applications.
 */

/**
 * \fn void NoiseDHState::ee(uint8_t *sharedKey)
 * \brief Performs a DH operation using the local and remote ephemeral keys.
 *
 * \param sharedKey Returns the shared key.  The buffer must be at least
 * sharedKeySize() bytes in size.
 *
 * It is assumed that the caller has already verified that the DH object
 * has all of the required keys before invoking this function.
 *
 * \sa es(), se(), ss()
 */

/**
 * \fn void NoiseDHState::es(uint8_t *sharedKey)
 * \brief Performs a DH operation using the local ephemeral key and the
 * remote static key.
 *
 * \param sharedKey Returns the shared key.  The buffer must be at least
 * sharedKeySize() bytes in size.
 *
 * It is assumed that the caller has already verified that the DH object
 * has all of the required keys before invoking this function.
 *
 * \sa ee(), se(), ss()
 */

/**
 * \fn void NoiseDHState::se(uint8_t *sharedKey)
 * \brief Performs a DH operation using the local static key and the
 * remote ephemeral key.
 *
 * \param sharedKey Returns the shared key.  The buffer must be at least
 * sharedKeySize() bytes in size.
 *
 * It is assumed that the caller has already verified that the DH object
 * has all of the required keys before invoking this function.
 *
 * \sa ee(), es(), ss()
 */

/**
 * \fn void NoiseDHState::ss(uint8_t *sharedKey)
 * \brief Performs a DH operation using the local and remote static keys.
 *
 * \param sharedKey Returns the shared key.  The buffer must be at least
 * sharedKeySize() bytes in size.
 *
 * It is assumed that the caller has already verified that the DH object
 * has all of the required keys before invoking this function.
 *
 * \sa es(), se(), ss()
 */

/**
 * \fn void NoiseDHState::clear()
 * \brief Clears all sensitive data from this object.
 */

/**
 * \brief Helper function for copying a key value into the DH object.
 *
 * \param output Output buffer to copy the key into.
 * \param expectedSize Expected size of the key value in bytes.
 * \param value Points to the input value to be stored.
 * \param size Size of the input value in bytes.
 *
 * \return Returns true if the value is stored, or false if \a size is
 * not the same as \a expectedSize or \a value is NULL.
 *
 * \sa copyParameterOut()
 */
bool NoiseDHState::copyParameterIn
    (void *output, size_t expectedSize, const void *value, size_t size)
{
    if (size != expectedSize || !value)
        return false;
    memcpy(output, value, size);
    return true;
}

/**
 * \brief Helper function for copying a key value out of the DH object.
 *
 * \param output Output buffer to copy the key into.
 * \param maxOutput Maximum size of the output buffer in bytes.
 * \param value Points to input key value to be copied.
 * \param size Size of the input key value.
 *
 * \return The actual size of the copied key value if it fits within
 * \a maxOutputBytes.  Returns zero if \a size is greater than \a maxOutput
 * or \a output is NULL.
 *
 * \sa copyParameterIn()
 */
size_t NoiseDHState::copyParameterOut
    (void *output, size_t maxOutput, const void *value, size_t size)
{
    if (size > maxOutput || !output)
        return 0;
    memcpy(output, value, size);
    return size;
}
