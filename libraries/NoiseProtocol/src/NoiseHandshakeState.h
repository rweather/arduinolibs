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

#ifndef NOISE_HANDSHAKE_STATE_h
#define NOISE_HANDSHAKE_STATE_h

#include "NoiseNamespace.h"
#include "NoiseSymmetricState.h"
#include "NoiseDHState.h"
#include "NoiseCipherState.h"

class NoiseHandshakeState
{
public:
    virtual ~NoiseHandshakeState();

    void start(Noise::Party party, const void *prologue = 0, size_t prologueLen = 0);
    bool addPrologue(const void *prologue, size_t prologueLen);

    Noise::Party party() const { return pty; }
    Noise::HandshakeState state() const { return st; }
    const char *protocolName() const { return protoName; }

    virtual bool setParameter
        (Noise::Parameter id, const void *value, size_t size);
    virtual size_t getParameter
        (Noise::Parameter id, void *value, size_t maxSize) const;
    virtual size_t getParameterSize(Noise::Parameter id) const;
    virtual bool hasParameter(Noise::Parameter id) const;
    virtual void removeParameter(Noise::Parameter id);

    int write(void *output, size_t maxOutputSize,
              const void *payload, size_t payloadSize);
    int read(void *payload, size_t maxPayloadSize,
             const void *input, size_t inputSize);
    bool split(NoiseCipherState **tx, NoiseCipherState **rx);
    bool getHandshakeHash(void *data, size_t size);

    virtual void clear();

protected:
    NoiseHandshakeState();

    NoiseSymmetricState *symmetricState() const { return sym; }
    void setSymmetricState(NoiseSymmetricState *symState) { sym = symState; }

    NoiseDHState *dhState() const { return dh; }
    void setDHState(NoiseDHState *dhState) { dh = dhState; }

    static const NoiseDHState *otherDHState
        (const NoiseHandshakeState *handshake) { return handshake->dh; }

    void setProtocolName(const char *name) { protoName = name; }

    virtual void removeKeys();

    /**
     * \brief Information about a handshake packet that is being processed.
     */
    struct Packet
    {
        uint8_t *data;  /**< Points to the start of the packet's data */
        size_t posn;    /**< Current processing position in the data array */
        size_t size;    /**< Maximum amount of available packet data */
        bool error;     /**< Set to true if an error has occurred */
        bool done;      /**< Set to true for the last handshake message */
    };

    virtual void writeTokens
        (NoiseHandshakeState::Packet &packet, uint8_t msgnum) = 0;
    virtual void readTokens
        (NoiseHandshakeState::Packet &packet, uint8_t msgnum) = 0;

    virtual void write_e(NoiseHandshakeState::Packet &packet);
    void write_s(NoiseHandshakeState::Packet &packet);
    void write_ee(NoiseHandshakeState::Packet &packet);
    void write_es(NoiseHandshakeState::Packet &packet);
    void write_se(NoiseHandshakeState::Packet &packet);
    void write_ss(NoiseHandshakeState::Packet &packet);

    virtual void read_e(NoiseHandshakeState::Packet &packet);
    void read_s(NoiseHandshakeState::Packet &packet);
    void read_ee(NoiseHandshakeState::Packet &packet) { write_ee(packet); }
    void read_es(NoiseHandshakeState::Packet &packet) { write_es(packet); }
    void read_se(NoiseHandshakeState::Packet &packet) { write_se(packet); }
    void read_ss(NoiseHandshakeState::Packet &packet) { write_ss(packet); }

    void premessage
        (NoiseHandshakeState::Packet &packet,
         Noise::Parameter initiator, Noise::Parameter responder);

    void setState(Noise::HandshakeState state) { st = state; }

private:
    NoiseSymmetricState *sym;
    NoiseDHState *dh;
    const char *protoName;
    Noise::Party pty;
    Noise::HandshakeState st;
    uint8_t msgnum;

    void writePayload(NoiseHandshakeState::Packet &packet,
                      const void *data, size_t size);
    size_t readPayload(NoiseHandshakeState::Packet &packet,
                       void *data, size_t maxSize);
};

class NoiseHandshakeStatePSK : public NoiseHandshakeState
{
public:
    virtual ~NoiseHandshakeStatePSK();

    bool setParameter(Noise::Parameter id, const void *value, size_t size);
    size_t getParameter(Noise::Parameter id, void *value, size_t maxSize) const;
    size_t getParameterSize(Noise::Parameter id) const;
    bool hasParameter(Noise::Parameter id) const;
    void removeParameter(Noise::Parameter id);

    void clear();

protected:
    NoiseHandshakeStatePSK();

    void write_e(NoiseHandshakeState::Packet &packet);
    void read_e(NoiseHandshakeState::Packet &packet);
    void write_psk(NoiseHandshakeState::Packet &packet);
    void read_psk(NoiseHandshakeState::Packet &packet) { write_psk(packet); }

private:
    uint8_t psk[32];
    bool havePSK;
};

#endif
