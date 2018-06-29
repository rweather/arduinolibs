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

#include "NoiseServer.h"

/**
 * \class NoiseServer NoiseServer.h <NoiseServer.h>
 * \brief Network server that uses Noise to secure incoming connections.
 *
 * This class is a template that wraps an underlying network server
 * interface to layer the Noise protocol on top to provide security.
 * Normally an application will use the NoiseEthernetServer or
 * NoiseWiFiServer template instances to create a network server.
 *
 * Incoming Noise connections are secured using the options within
 * the global NoiseServerOptions instance.
 *
 * \code
 * #include <NoiseProtocol.h>
 * #include <Ethernet.h>
 * #include <RNG.h>
 *
 * uint8_t mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
 * NoiseEthernetServer server(4433);
 *
 * void setup() {
 *     // Initialise the system random number generator.
 *     RNG.begin("MyApp 1.0");
 *
 *     // Start the Ethernet interface.
 *     Ethernet.begin(mac);
 *
 *     // Load or generate the Curve25519 key pair for this device.
 *     if (!NoiseServerOptions.load(Noise::LocalStaticKeyPair))
 *         NoiseServerOptions.generate(Noise::LocalStaticKeyPair);
 *
 *     // Specify the protocols that we permit incoming connections to use.
 *     NoiseServerOptions.addProtocol(Noise_XX_25519_ChaChaPoly_BLAKE2s);
 *     NoiseServerOptions.addProtocol(Noise_XX_25519_AESGCM_SHA256);
 *
 *     // Limit packet payloads to 128 bytes and pad to full length.
 *     NoiseServerOptions.setBufferSize(128);
 *     NoiseServerOptions.setPadding(Noise::RandomPadding);
 *
 *     // Start Ethernet server operations on the port.
 *     server.begin();
 * }
 *
 * void loop() {
 *     NoiseEthernetClient client = server.available();
 *     if (client) {
 *         // Handle the incoming connection.
 *         ...
 *     }
 * }
 * \endcode
 */

/**
 * \fn NoiseServer::NoiseServer(uint16_t port)
 * \brief Constructs a new network server that uses Noise.
 *
 * \param port The network port number to bind to on the underlying
 * network interface when begin() is called.
 *
 * This will also construct the underlying network interface, which is
 * usually an instance of EthernetServer or WiFiServer.
 */

/**
 * \fn NoiseServer::~NoiseServer()
 * \brief Destroys this Noise-based network server.
 *
 * This will also destroy the underlying network interface.
 */

/**
 * \fn ServerT *NoiseServer::network()
 * \brief Returns a reference to the underlying network interface.
 *
 * \return The underlying network server object; usually something like
 * EthernetServer or WiFiServer.
 *
 * This function is intended to let the application adjust custom settings
 * on the underlying network interface prior to calling begin():
 *
 * \code
 * NoiseWiFiServer server;
 * server.network()->setNoDelay(true);
 * server.begin();
 * \endcode
 *
 * \sa begin()
 */

/**
 * \fn void NoiseServer::begin()
 * \brief Begins network server operations.
 *
 * \sa available()
 */

/**
 * \fn NoiseClientWrapper<ClientT> NoiseServer::available()
 * \brief Checks for the next available incoming client.
 *
 * \return The next available incoming client, which may be false if there
 * is no client available yet.  Typically an instance of NoiseEthernetClient
 * or NoiseWiFiClient depending upon the underlying network interface type.
 *
 * \code
 * NoiseEthernetServer server;
 * server.begin();
 * ...
 * void loop() {
 *     NoiseEthernetClient client = server.available();
 *     if (client) {
 *         // Handle the new client.
 *         ...
 *     }
 * }
 * \endcode
 *
 * \sa begin()
 */
