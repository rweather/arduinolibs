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

class NoiseProtocolDescriptor
{
public:
    virtual ~NoiseProtocolDescriptor();

    const char *protocolName() const { return protoName; }
    const char *protocolAlias() const { return protoAlias; }

    virtual NoiseHandshakeState *createHandshake() const = 0;

    virtual const NoiseProtocolDescriptor *abbreviatedDescriptor() const;
    virtual const NoiseProtocolDescriptor *fallbackDescriptor() const;

protected:
    explicit NoiseProtocolDescriptor(const char *name, const char *alias = 0)
        : protoName(name), protoAlias(alias) {}

private:
    const char *protoName;
    const char *protoAlias;
};

#endif
