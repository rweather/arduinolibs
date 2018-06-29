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

#ifndef NOISE_CLIENT_h
#define NOISE_CLIENT_h

#include "NoiseLinkOptions.h"
#include <Client.h>

class NoiseClient : public Client
{
private:
    // Disable the copy operators.  NoiseEthernetClient and NoiseWiFiClient
    // should be used instead for safe copiable network client objects.
    NoiseClient(const NoiseClient &other) {}
    NoiseClient &operator=(const NoiseClient &other) { return *this; }

public:
    explicit NoiseClient(Client *networkClient);
    virtual ~NoiseClient();

    NoiseLinkOptions *options();

    Client *network() const { return net; }

    Noise::ConnectionStatus connectionStatus() const;

    void clear();

    // TODO: How to check for known remote public keys in a simple way?

    size_t getRemotePublicKey(void *value, size_t maxSize) const;
    size_t getHandshakeHash(void *value, size_t maxSize) const;

    // Standard API for Arduino network clients.
    int connect(IPAddress ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    size_t write(uint8_t data);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    void flush();
    void stop();
    uint8_t connected();
    operator bool();

    /** @cond noise_client_internal */
    // Internal helper functions for NoiseClientWrapper and NoiseServer.
    // Not part of the public API that applications should be using.
    static void ref(NoiseClient *client);
    static void deref(NoiseClient *client);
    bool accept(const NoiseLinkOptions &options);
    /** @endcond */

private:
    int refCount;
    Client *net;
    NoiseLinkOptions *opts;

    const NoiseLinkOptions *constOptions() const;

    int connect();
};

/** @cond noise_client_wrapper */

// Nelper template for safely creating and copying instances of NoiseClient for
// specific underlying network stacks such as EthernetClient or WiFiClient.
template <typename T>
class NoiseClientWrapper : public Client
{
public:
    inline NoiseClientWrapper() : d(0) {}
    inline NoiseClientWrapper(const NoiseClientWrapper<T> &other)
        : d(other.d) { NoiseClient::ref(d); }
    inline NoiseClientWrapper(NoiseClient *client) : d(client) {}
    inline ~NoiseClientWrapper() { NoiseClient::deref(d); }

    inline NoiseClientWrapper<T> &operator=(const NoiseClientWrapper<T> &other)
    {
        NoiseClient::ref(other.d);
        NoiseClient::deref(d);
        d = other.d;
        return *this;
    }

    inline NoiseLinkOptions *options()
    {
        return d ? d->options() : &NoiseClientOptions;
    }

    inline T *network() const
    {
        return d ? (T *)(d->network()) : 0;
    }

    int connect(IPAddress ip, uint16_t port)
    {
        if (!d)
            d = new NoiseClient(new T());
        return d->connect(ip, port);
    }

    int connect(const char *host, uint16_t port)
    {
        if (!d)
            d = new NoiseClient(new T());
        return d->connect(host, port);
    }

    size_t write(uint8_t data)
    {
        return d ? d->write(data) : 0;
    }

    size_t write(const uint8_t *buf, size_t size)
    {
        return d ? d->write(buf, size) : 0;
    }

    int available()
    {
        return d ? d->available() : 0;
    }

    int read()
    {
        return d ? d->read() : -1;
    }

    int read(uint8_t *buf, size_t size)
    {
        return d ? d->read(buf, size) : -1;
    }

    int peek()
    {
        return d ? d->peek() : -1;
    }

    void flush()
    {
        if (d)
            d->flush();
    }

    void stop()
    {
        if (d)
            d->stop();
    }

    uint8_t connected()
    {
        return d ? d->connected() : false;
    }

    operator bool()
    {
        return d ? (d->operator bool()) : false;
    }

private:
    NoiseClient *d;
};

/** @endcond */

#define NoiseEthernetClient NoiseClientWrapper<EthernetClient>
#define NoiseWiFiClient NoiseClientWrapper<WiFiClient>

#endif
