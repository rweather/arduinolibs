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

#ifndef NOISE_PROTOCOL_DESCRIPTOR_h
#define NOISE_PROTOCOL_DESCRIPTOR_h

class NoiseHandshakeState;

/** Noise Protocol needs a local static key pair to connect */
#define NOISE_PROTOCOL_NEEDS_LOCAL_STATIC   0x0001
/** Noise protocol needs a remote static public key to connect */
#define NOISE_PROTOCOL_NEEDS_REMOTE_STATIC  0x0002
/** Noise protocol needs a pre-shared symmetric key to connect */
#define NOISE_PROTOCOL_NEEDS_PSK            0x0004

/**
 * \brief Structure that provides metadata for Noise protocols.
 */
struct NoiseProtocolDescriptor
{
    /** Flags that define the properties and required keys for the protocol
     *  when the handshake is created from the initiator side. */
    unsigned short initiatorFlags;

    /** Flags that define the properties and required keys for the protocol
     *  when the handshake is created from the responder side. */
    unsigned short responderFlags;

    /** Full Noise protocol name; e.g. "Noise_XX_25519_ChaChaPoly_BLAKE2s" */
    const char *protocolName;

    /** NoiseTinyLink alias for the protocol, or NULL if no alias */
    const char *protocolAlias;

    /** Function that creates a handshake instance for the protocol */
    NoiseHandshakeState *(*createHandshake)();
};

#endif
