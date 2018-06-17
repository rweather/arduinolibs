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

#include "NoiseProtocolDescriptor.h"

/**
 * \class NoiseProtocolDescriptor NoiseProtocolDescriptor.h <NoiseProtocolDescriptor.h>
 * \brief Description of a Noise protocol and a method to create a handshake
 * object that implements the protocol.
 *
 * This class is abstract.  The caller should instantiate a subclass like
 * Noise_XX_25519_ChaChaPoly_BLAKE2s or Noise_XX_25519_AESGCM_SHA256 to
 * get an actual descriptor.
 */

/**
 * \fn NoiseProtocolDescriptor::NoiseProtocolDescriptor(const char *name, const char *alias)
 * \brief Creates a new Noise protocol descriptor.
 *
 * \param name Name of the Noise protocol, e.g.  "Noise_XX_25519_AESGCM_SHA256".
 * \param alias NoiseTinyLink alias for the Noise protocol; e.g. "1".  Set this
 * to NULL if the protocol does not have a NoiseTinyLink alias defined.
 */

/**
 * \brief Destroys this Noise protocol descriptor.
 */
NoiseProtocolDescriptor::~NoiseProtocolDescriptor()
{
}

/**
 * \fn const char *NoiseProtocolDescriptor::protocolName() const
 * \brief Gets the name of the Noise protocol represented by this descriptor.
 *
 * \return The name of the Noise protocol.
 *
 * \sa protocolAlias()
 */

/**
 * \fn const char *NoiseProtocolDescriptor::protocolAlias() const
 * \brief Gets the NoiseTinyLink alias for the Noise protocol represented
 * by this descriptor.
 *
 * \return The alias or NULL if the protocol does not have a NoiseTinyLink
 * alias defined.
 *
 * \sa protocolName()
 */

/**
 * \fn NoiseHandshakeState *NoiseProtocolDescriptor::createHandshake() const
 * \brief Creates a handshake object for the Noise protocol represented
 * by this descriptor.
 *
 * \return A new handshake object for the protocol.
 */
