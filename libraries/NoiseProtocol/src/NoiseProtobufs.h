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

#ifndef NOISE_PROTOBUFS_h
#define NOISE_PROTOBUFS_h

#include <inttypes.h>
#include <stddef.h>

typedef uint8_t NoiseProtobufTag_t;

namespace Noise
{
    // Tags for all NoiseLink negotiation options, for convenience.
    // Not all of these are used for the NoiseTinyLink profile but we
    // provide them in case we need more of full NoiseLink in the future.

    /**
     * \brief Protobuf tags for NoiseLink session negotiation requests
     * from the client.
     */
    enum NegotiationRequest
    {
        ReqServerName       = 1,    /**< Server name */
        ReqInitialProtocol  = 2,    /**< Initial Noise protocol to try */
        ReqSwitchProtocol   = 3,    /**< List of supported fallback protocols */
        ReqRetryProtocol    = 4,    /**< List of other non-fallback protocols */
        ReqRejectedProtocol = 5,    /**< Previous protocol rejected by server */
        ReqPSKId            = 6     /**< Identifier for the client's PSK */
    };

    /**
     * \brief Protobuf tags for NoiseLink session negotiation responses
     * from the server.
     */
    enum NegotiationResponse
    {
        RespSwitchProtocol  = 3,    /**< Switch to a fallback protocol */
        RespRetryProtocol   = 4,    /**< Retry with a non-fallback protocol */
        RespRejected        = 5     /**< Server totally rejected the client */
    };

    /**
     * \brief Protobuf tags for NoiseLink options that may appear in
     * handshake message payloads.
     */
    enum PayloadOptions
    {
        OptEvidenceReqType  = 1,    /**< Evidence request type */
        OptEvidenceBlobType = 2,    /**< Evidence blob type */
        OptEvidenceBlob     = 3,    /**< Evidence blob */
        OptPSKId            = 4,    /**< Identifier for the client's PSK */
        OptTransport        = 5     /**< Embedded transport options */
    };

    /**
     * \brief Protobuf tags for NoiseLink transport options that may
     * appear in handshake message payloads within an embedded message
     * with the tag number OptTransport.
     */
    enum TransportOptions
    {
        TrMaxSendLength     = 1,    /**< Maximum send buffer length */
        TrMaxRecvLength     = 2,    /**< Maximum receive buffer length */
        TrContinuousRekey   = 3,    /**< Rekey after every message */
        TrShortTerminated   = 4     /**< Terminate session on short message */
    };
};

class NoiseProtobufFormatter
{
public:
    NoiseProtobufFormatter(void *data, size_t maxSize);
    ~NoiseProtobufFormatter();

    size_t size() const { return pos; }
    bool error() const { return err; }

    void addBool(NoiseProtobufTag_t tag, bool value);

    void addUInt32(NoiseProtobufTag_t tag, uint32_t value);
    void addUInt64(NoiseProtobufTag_t tag, uint64_t value);
    void addInt32(NoiseProtobufTag_t tag, int32_t value);
    void addInt64(NoiseProtobufTag_t tag, int64_t value);

    void addBytes(NoiseProtobufTag_t tag, const void *data, size_t len);

    void addString(NoiseProtobufTag_t tag, const char *str);
    void addString(NoiseProtobufTag_t tag, const char *str, size_t len);

    size_t startEmbedded(NoiseProtobufTag_t tag);
    void endEmbedded(size_t start);

private:
    uint8_t *buf;
    size_t maxsz;
    size_t pos;
    bool err;

    void addHeader(NoiseProtobufTag_t tag, uint8_t wireType);
    void addVarUInt14(uint16_t value);
    void addVarUInt32(uint32_t value);
    void addVarUInt64(uint64_t value);
};

class NoiseProtobufParser
{
public:
    NoiseProtobufParser(const void *data, size_t size);
    ~NoiseProtobufParser();

    bool error() const { return err; }

    bool readNext(NoiseProtobufTag_t &tag);

    bool readBool();
    uint32_t readUInt32();
    uint64_t readUInt64();
    int32_t readInt32();
    int64_t readInt64();

    size_t readString(const char * &str);
    size_t readBytes(const void * &data);

    size_t startEmbedded();
    void endEmbedded(size_t posn);

private:
    const uint8_t *buf;
    size_t pos;
    size_t end;
    size_t ptr;
    uint64_t value;
    uint8_t wire;
    bool err;

    uint64_t readVarint();
};

inline void NoiseProtobufFormatter::addHeader
    (NoiseProtobufTag_t tag, uint8_t wireType)
{
    addVarUInt14((((uint16_t)tag) << 3) | wireType);
}

inline void NoiseProtobufFormatter::addBool(NoiseProtobufTag_t tag, bool value)
{
    addHeader(tag, 0);
    addVarUInt14(value ? 1 : 0);
}

inline void NoiseProtobufFormatter::addUInt32
    (NoiseProtobufTag_t tag, uint32_t value)
{
    addHeader(tag, 0);
    addVarUInt32(value);
}

inline void NoiseProtobufFormatter::addUInt64
    (NoiseProtobufTag_t tag, uint64_t value)
{
    addHeader(tag, 0);
    addVarUInt64(value);
}

inline void NoiseProtobufFormatter::addInt32
    (NoiseProtobufTag_t tag, int32_t value)
{
    addHeader(tag, 0);
    addVarUInt32((uint32_t)((value << 1) ^ (value >> 31)));
}

inline void NoiseProtobufFormatter::addInt64
    (NoiseProtobufTag_t tag, int64_t value)
{
    addHeader(tag, 0);
    addVarUInt64((uint64_t)((value << 1) ^ (value >> 63)));
}

inline void NoiseProtobufFormatter::addString
    (NoiseProtobufTag_t tag, const char *str, size_t len)
{
    addBytes(tag, str, len);
}

#endif
