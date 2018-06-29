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

#ifndef NOISE_NAMESPACE_h
#define NOISE_NAMESPACE_h

/**
 * \namespace Noise
 * \brief Common definitions for the Noise protocol.
 */
namespace Noise
{
    /**
     * \brief Indicates which party is engaging in a handshake.
     */
    enum Party
    {
        Initiator,      /**< Local party is the initiator of the handshake */
        Responder       /**< Local party is the responder of the handshake */
    };

    /**
     * \brief Current state of a handshake.
     */
    enum HandshakeState
    {
        Write,          /**< Next operation is a handshake packet write() */
        Read,           /**< Next operation is a handshake packet read() */
        Split,          /**< Handshake complete, next operation is split() */
        Finished,       /**< Handshake finished and split() already done */
        Failed          /**< Handshake has failed or was never started */
    };

    /**
     * \brief Identifiers for handshake parameters.
     */
    enum Parameter
    {
        // Standard handshake parameters.
        LocalStaticKeyPair         = 1,   /**< Local DH static key pair */
        LocalStaticPrivateKey      = 2,   /**< Local DH static private key */
        LocalStaticPublicKey       = 3,   /**< Local DH static public key */
        RemoteStaticPublicKey      = 4,   /**< Remote DH static public key */
        PreSharedKey               = 5,   /**< Pre-shared symmetric key */

        // Specific algorithm names for meta-handshakes.
        LocalStatic25519KeyPair    = 100, /**< Local Curve25519 key pair */
        LocalStatic25519PrivateKey = 101, /**< Local Curve25519 private key */
        LocalStatic25519PublicKey  = 102, /**< Local Curve25519 public key */
        RemoteStatic25519PublicKey = 103, /**< Remote Curve25519 public key */

        // Internal parameters (for application testing use only).
        LocalEphemKeyPair          = 200, /**< Local DH ephemeral key pair */
        LocalEphemPrivateKey       = 201, /**< Local DH ephemeral private key */
        LocalEphemPublicKey        = 202, /**< Local DH ephemeral public key */
        RemoteEphemPublicKey       = 203, /**< Remote DH ephemeral public key */
        LocalEphem25519KeyPair     = 204, /**< Local ephemeral Curve25519 key pair */
        LocalEphem25519PrivateKey  = 205, /**< Local ephemeral Curve25519 private key */
        LocalEphem25519PublicKey   = 206, /**< Local ephemeral Curve25519 public key */
        RemoteEphem25519PublicKey  = 207  /**< Remote ephemeral Curve25519 public key */
    };

    /**
     * \brief Type of padding to apply to plaintext before encryption.
     */
    enum Padding
    {
        NoPadding,      /**< No padding */
        ZeroPadding,    /**< Pad with zeroes to the maximum length */
        RandomPadding   /**< Pad with random data to the maximum length */
    };

    /**
     * \brief Status of a NoiseClient connection.
     */
    enum ConnectionStatus
    {
        Closed,         /**< Connection is closed or session hasn't started */
        Connecting,     /**< Connecting, handshake is not complete yet */
        Connected,      /**< Connected, ready to transmit/receive data */
        Closing,        /**< Connection is closing, but still unread data */
        HandshakeFailed /**< Failed to establish a secure connection */
    };

}; // namespace Noise

#endif
