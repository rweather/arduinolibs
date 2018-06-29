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

#include "NoiseClient.h"

/**
 * \class NoiseClient NoiseClient.h <NoiseClient.h>
 * \brief Network class for connecting to a peer using the Noise protocol.
 */

/**
 * \var NoiseEthernetClient
 * \brief Convenience type for communicating via Noise over an Ethernet link.
 */

/**
 * \var NoiseWiFiClient
 * \brief Convenience type for communicating via Noise over a Wi-Fi link.
 */

/** @cond noise_client_private */

// Standard Arduino network error codes for connect().
#define NOISE_CLIENT_SUCCESS            1
#define NOISE_CLIENT_FAILED             0
#define NOISE_CLIENT_TIMED_OUT         -1
#define NOISE_CLIENT_INVALID_SERVER    -2
#define NOISE_CLIENT_TRUNCATED         -3
#define NOISE_CLIENT_INVALID_RESPONSE  -4

/** @endcond */

/**
 * \brief Constructs a new Noise network client.
 *
 * \param networkClient The underlying transport client; usually an instance
 * of EthernetClient or WiFiClient.
 *
 * The new object will take owership of \a networkClient and will delete it
 * when this object is destroyed.
 */
NoiseClient::NoiseClient(Client *networkClient)
    : refCount(1)
    , net(networkClient)
    , opts(0)
{
}

/**
 * \brief Destroys this Noise network client.
 */
NoiseClient::~NoiseClient()
{
    delete net;
    delete opts;
}

/**
 * \brief Gets the options block for this client so that the application
 * can modify the options for this client only.
 *
 * \return A pointer to the options block.
 *
 * Normally the options for clients are set on the global NoiseClientOptions
 * object, which then apply to all clients on the device.  If this function
 * is called, then the options in NoiseClientOptions are copied and the
 * application can then make local modifications specific to this session.
 *
 * The options can be modified up until the point that connect() is
 * called.  After that, whatever options were set on the client become
 * permanent for the session.  Any modifications that are made to the
 * options will be ignored until the next call to connect() on this object.
 *
 * For incoming connections, use NoiseServerOptions and NoiseServer::options()
 * instead.
 *
 * \sa NoiseServer::options()
 */
NoiseLinkOptions *NoiseClient::options()
{
    // The caller may be modifying the options, so we need to clone
    // the global options block to avoid making modifications to it.
    if (opts)
        return opts;
    opts = new NoiseLinkOptions();
    opts->copyFrom(NoiseClientOptions);
    return opts;
}

/**
 * \brief Gets a constant reference to the options block for this client.
 *
 * \return A constant pointer to the options block.
 */
const NoiseLinkOptions *NoiseClient::constOptions() const
{
    // Avoid cloning the global options block if we don't need to.
    if (opts)
        return opts;
    return &NoiseClientOptions;
}

/**
 * \fn Client *NoiseClient::network() const
 * \brief Gets the underlying network client.
 *
 * \return The underlying network client for this Noise client;
 * usually an instance of EthernetClient or WiFiClient.
 *
 * Applications normally won't need to use this unless they need to
 * adjust the raw socket options on the transport.
 */

/**
 * \brief Gets the status of the connection.
 *
 * \return Noise::Closed if the connection has closed or it hasn't started yet.
 * \return Noise::Connecting if the connection is in the process of connecting
 * to the remote host, but the secure handshake has not completed yet.
 * \return Noise::Connected if the connection is active and ready to
 * send and receive data.
 * \return Noise::Closing if the remote end of the connection has been
 * closed but there are still bytes left in the receive buffer.  Any data
 * that is transmitted in this state will be discarded.
 * \return HandshakeFailed if the client failed to establish a secure
 * connection with the remote party.
 *
 * \sa connected()
 */
Noise::ConnectionStatus NoiseClient::connectionStatus() const
{
    // TODO
    if (!net)
        return Noise::Closed;
    return net->connected() ? Noise::Closed : Noise::Connected;
}

/**
 * \brief Clears security-sensitive data from this object.
 *
 * If the connection is active, then calling this function will stop() it.
 */
void NoiseClient::clear()
{
    stop();
    if (opts)
        opts->clear();
    // TODO
}

/**
 * \brief Gets the public key of the remote party that this client
 * is communicating with.
 *
 * \param value Buffer to receive the remote public key.
 * \param maxSize Maximum size of the \a value buffer.
 *
 * \return The number of bytes that were written to the \a value buffer if
 * the remote public key is available.
 * \return 0 if the remote public key is not available yet.
 * \return 0 if \a maxSize is too small to contain the key.
 * \return 0 if the protocol does not make use of remote public keys,
 * such as the "NNpsk0" pattern.
 *
 * The remote public key is available once the handshake completes
 * and when connected() becomes true.  Typically an application will use
 * this to verify the identity of the party they are communicating with.
 *
 * \sa getHandshakeHash()
 */
size_t NoiseClient::getRemotePublicKey(void *value, size_t maxSize) const
{
    // TODO
    return 0;
}

/**
 * \brief Gets the handshake hash at the end of the handshake.
 *
 * \param value Buffer to receive the handshake hash.
 * \param maxSize Maximum size of the \a value buffer.
 *
 * \return The number of bytes that were written to the \a value buffer if
 * the handshake hash is available.
 * \return 0 if the handshake hash is not available yet.
 * \return 0 if \a maxSize is too small to contain the handshake hash.
 *
 * The handshake hash is a value defined by the Noise specification that
 * is unique for every connection, and which can be used to secure higher
 * level authentication protocols.  See the Noise specification for details.
 *
 * \sa getRemotePublicKey()
 */
size_t NoiseClient::getHandshakeHash(void *value, size_t maxSize) const
{
    // TODO: modify NoiseHandshakeState API to use the maxSize idiom.
    return 0;
}

/**
 * \brief Connects to a remote host using an IP address and port number.
 *
 * \param ip The IP address of the remote host.
 * \param port The port number on the remote host.
 *
 * \return 1 if the connection was initiated.
 * \return 0 if the connection attempt failed.
 * \return A negative code on error.
 *
 * This function establishes the basic connection but the security handshake
 * will not be complete when this function returns.  The connection will not
 * be ready send or receive until conected() returns true.  Data that is
 * sent before then will be discarded.
 *
 * \sa connected()
 */
int NoiseClient::connect(IPAddress ip, uint16_t port)
{
    // Stop the previous connection first.
    stop();

    // Fail if the underlying network client is not ready.
    if (!net || !(net->operator bool()))
        return NOISE_CLIENT_FAILED;

    // Attempt to connect to the remote host using the underlying network.
    int ret = net->connect(ip, port);
    if (ret <= 0)
        return ret;

    // Start the handshake running.
    return connect();
}

/**
 * \brief Connects to a remote host using a host name and port number.
 *
 * \param host The name of the remote host.
 * \param port The port number on the remote host.
 *
 * \return 1 if the connection was initiated.
 * \return 0 if the connection attempt failed.
 * \return A negative code on error.
 *
 * This function establishes the basic connection but the security handshake
 * will not be complete when this function returns.  The connection will not
 * be ready send or receive until conected() returns true.  Data that is
 * sent before then will be discarded.
 *
 * \sa connected()
 */
int NoiseClient::connect(const char *host, uint16_t port)
{
    // Stop the previous connection first.
    stop();

    // Fail if the underlying network client is not ready.
    if (!net || !(net->operator bool()))
        return NOISE_CLIENT_FAILED;

    // Attempt to connect to the remote host using the underlying network.
    int ret = net->connect(host, port);
    if (ret <= 0)
        return ret;

    // Start the handshake running.
    return connect();
}

/**
 * \brief Internal implementation of the public connect() methods that
 * is called after the basic network link is established.
 *
 * \return 1 if the connection was initiated.
 * \return 0 if the connection attempt failed.
 * \return A negative code on error.
 */
int NoiseClient::connect()
{
    // TODO
    return NOISE_CLIENT_SUCCESS;
}

/**
 * \brief Writes a single byte to the connection.
 *
 * \param data The byte to write.
 *
 * \return 1 if the byte was written successfully, 0 if the byte was
 * not written.
 *
 * If the security handshake has not completed or the connection has been
 * lost, then the write will be rejected with a 0 return value.
 *
 * This function may not send the byte to the other party immediately.
 * It may be queued up until the next call to flush(), read(), or
 * available().
 *
 * \sa flush(), read(), available()
 */
size_t NoiseClient::write(uint8_t data)
{
    // TODO
    return 0;
}

/**
 * \brief Writes a buffer of bytes to the connection.
 *
 * \param buf Points to the start of the buffer.
 * \param size The number of bytes in the buffer.
 *
 * \return The number of bytes that were written, which may be 0 if
 * the connection is not ready to accept data.
 *
 * If the security handshake has not completed or the connection has been
 * lost, then the write will be rejected with a 0 return value.
 *
 * This function may not send the data to the other party immediately.
 * It may be queued up until the next call to flush(), read(), or
 * available().
 *
 * \sa flush(), read()
 */
size_t NoiseClient::write(const uint8_t *buf, size_t size)
{
    // TODO
    return size;
}

/**
 * \brief Gets the number of bytes that are available to read().
 *
 * \return The number of bytes that are available.
 *
 * This function will implicitly perform a flush() to send any outstanding
 * data in the internal write buffer to the other party.
 *
 * \sa read(), flush()
 */
int NoiseClient::available()
{
    flush();
    // TODO
    return 0;
}

/**
 * \brief Reads a single byte from the connection if one is ready.
 *
 * \return The next byte or -1 if no bytes are ready to read.
 *
 * This function will implicitly perform a flush() to send any outstanding
 * data in the internal write buffer to the other party.
 *
 * \sa write(), available(), flush(), peek()
 */
int NoiseClient::read()
{
    flush();
    // TODO
    return -1;
}

/**
 * \brief Reads a buffer of bytes from the connection if data is ready.
 *
 * \param buf Points to the buffer to read into.
 * \param size Maximum number of bytes that can be read into the buffer.
 *
 * This function will implicitly perform a flush() to send any outstanding
 * data in the internal write buffer to the other party.
 *
 * \sa write(), available(), flush()
 */
int NoiseClient::read(uint8_t *buf, size_t size)
{
    flush();
    // TODO
    return 0;
}

/**
 * \brief Peeks at the next byte without removing it from the receive buffer.
 *
 * \return The next byte or -1 if no bytes are ready to read.
 *
 * This function will implicitly perform a flush() to send any outstanding
 * data in the internal write buffer to the other party.
 *
 * \sa read()
 */
int NoiseClient::peek()
{
    flush();
    // TODO
    return -1;
}

/**
 * \brief Flushes pending data in internal buffers to the other party.
 */
void NoiseClient::flush()
{
    // TODO
}

/**
 * \brief Closes the connection and stops it.
 */
void NoiseClient::stop()
{
    // TODO
}

/**
 * \brief Determine if this client is connected to the remote peer and
 * ready to send or receive application data.
 *
 * \return Returns non-zero if the client is connected and the security
 * handshake has completed successfully; zero if not.
 *
 * This function can be used to determine if the connection is ready
 * to send and receive application data.  Requests to read() or write()
 * before this point will fail.
 *
 * The connectionStatus() function can be used to get more detailed
 * information on the state of the connection.
 *
 * \sa connectionStatus()
 */
uint8_t NoiseClient::connected()
{
    Noise::ConnectionStatus status = connectionStatus();
    return status == Noise::Connected || status == Noise::Closing;
}

/**
 * \brief Determine if this network client is ready to make connections.
 *
 * \return Returns true if the client is ready, false otherwise.
 */
NoiseClient::operator bool()
{
    if (!net)
        return false;
    return net->operator bool();
}

void NoiseClient::ref(NoiseClient *client)
{
    if (client)
        ++(client->refCount);
}

void NoiseClient::deref(NoiseClient *client)
{
    if (client && --(client->refCount) <= 0)
        delete client;
}

bool NoiseClient::accept(const NoiseLinkOptions &options)
{
    // Make a copy of the incoming server options.
    if (!opts)
        opts = new NoiseLinkOptions();
    opts->copyFrom(options);
    // TODO
    return false;
}
