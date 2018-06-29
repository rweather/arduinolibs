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

#ifndef NOISE_SERVER_h
#define NOISE_SERVER_h

#include "NoiseClient.h"
#include <Server.h>

template <typename ServerT, typename ClientT>
class NoiseServer : public Server
{
private:
    // Disable the copy operators.
    NoiseServer(const NoiseServer<ServerT, ClientT> &other) {}
    NoiseServer &operator=(const NoiseServer<ServerT, ClientT> &other) { return *this; }

public:
    explicit NoiseServer(uint16_t port) : net(port) {}
    virtual ~NoiseServer() {}

    inline ServerT *network() { return &net; }

    // Standard Arduino Server interface.
    inline void begin() { net.begin(); }
    NoiseClientWrapper<ClientT> available();

    /** @cond noise_server_write */
    // Disable the write() functions from the base Print class.
    // "Write to all clients" behaviour is not supported.
    size_t write(uint8_t) { return 0; }
    size_t write(const uint8_t *buffer, size_t size) { return 0; }
    /** @endcond */

private:
    ServerT net;
};

template <typename ServerT, typename ClientT>
NoiseClientWrapper<ClientT> NoiseServer<ServerT, ClientT>::available()
{
    ClientT client = net.available();
    if (client) {
        ClientT *networkClient = new ClientT();
        *networkClient = client;
        NoiseClient *noiseClient = new NoiseClient(networkClient);
        if (noiseClient->accept(NoiseServerOptions))
            return NoiseClientWrapper<ClientT>(noiseClient);
        delete networkClient;
    }
    return NoiseClientWrapper<ClientT>();
}

#define NoiseEthernetServer NoiseServer<EthernetServer, EthernetClient>
#define NoiseWiFiServer NoiseServer<WiFiServer, WiFiClient>

#endif
