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

#ifndef NOISE_LINK_OPTIONS_h
#define NOISE_LINK_OPTIONS_h

#include "NoiseNamespace.h"
#include "NoiseProtocolDescriptor.h"
#include "KeyRing.h"

/** Default send/receive buffer size for NoiseLink sessions */
#define NOISE_DEFAULT_BUFSIZ 512

/** Minimum send/receive buffer size that we allow for NoiseLink sessions */
#define NOISE_MIN_BUFSIZ 128

/** Maximum send/receive buffer size that we allow for NoiseLink sessions */
#define NOISE_MAX_BUFSIZ 2048

class NoiseLinkOptionsPrivate;

class NoiseLinkOptions
{
private:
    // Disable the copy operators.
    NoiseLinkOptions(const NoiseLinkOptions &other) {}
    NoiseLinkOptions &operator=(const NoiseLinkOptions &other) { return *this; }

public:
    NoiseLinkOptions();
    ~NoiseLinkOptions();

    bool setParameter(Noise::Parameter id, const void *value, size_t size);
    size_t getParameter(Noise::Parameter id, void *value, size_t maxSize) const;
    size_t getParameterSize(Noise::Parameter id) const;
    bool hasParameter(Noise::Parameter id) const;
    void removeParameter(Noise::Parameter id);

    bool load(Noise::Parameter id, uint16_t keyRingId = 0);
    bool save(Noise::Parameter id, uint16_t keyRingId = 0);
    bool generate(Noise::Parameter id, uint16_t keyRingId = 0, bool wait = true);

    size_t protocolCount() const;
    const NoiseProtocolDescriptor *protocolAt(size_t index) const;
    void addProtocol(const NoiseProtocolDescriptor &protocol);
    void removeProtocol(const NoiseProtocolDescriptor &protocol);

    size_t sendBufferSize() const;
    void setSendBufferSize(size_t size);
    size_t receiveBufferSize() const;
    void setReceiveBufferSize(size_t size);
    void setBufferSize(size_t size)
        { setSendBufferSize(size); setReceiveBufferSize(size); }

    bool allowAliases() const;
    void setAllowAliases(bool allow);

    Noise::Padding padding() const;
    void setPadding(Noise::Padding padding);

    void copyFrom(const NoiseLinkOptions &options);

    void clear();

    NoiseHandshakeState *createHandshake
        (Noise::Party party, size_t protocolIndex) const;

private:
    NoiseLinkOptionsPrivate *d;
};

extern NoiseLinkOptions NoiseClientOptions;
extern NoiseLinkOptions NoiseServerOptions;

#endif
