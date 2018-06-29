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

#include "NoiseLinkOptions.h"
#include "NoiseHandshakeState.h"
#include "Crypto.h"
#include "RNG.h"
#include "Curve25519.h"
#include <string.h>

/**
 * \class NoiseLinkOptions NoiseLinkOptions.h <NoiseLinkOptions.h>
 * \brief Collection of options for NoiseLink clients and servers.
 *
 * The global objects NoiseClientOptions and NoiseServerOptions are
 * used to set the initial options for Noise sessions created by
 * NoiseClient and NoiseServer respectively.  They are usually populated
 * at system startup.
 *
 * \code
 * void setup()
 * {
 *     ...
 *
 *     // Load the device's local key pair from the key ring.  Otherwise,
 *     // generate a new key pair and store it in the key ring.
 *     if (!NoiseClientOptions.load(Noise::LocalStaticKeyPair)) {
 *         Serial.println("Generating a new key pair, please wait ...");
 *         NoiseClientOptions.generate(Noise::LocalStaticKeyPair);
 *     }
 *
 *     ...
 * }
 * \endcode
 *
 * If you are using the "IK" pattern to connect to a remote device,
 * then you also need to load the public key of the remote device:
 *
 * \code
 * void setup()
 * {
 *     ...
 *
 *     // Load the public key of the remote device we are communicating with.
 *     if (!NoiseClientOptions.load(Noise::RemoteStaticPublicKey)) {
 *         Serial.println("Do not know the identity of the remote device");
 *         ...
 *     }
 *
 *     ...
 * }
 * \endcode
 *
 * If you want to be more clever, you can try connecting with the "XX"
 * pattern if the identity of the remote device is unknown and then
 * record the public key for use with "IK" in subsequent sessions.
 *
 * Keys do not have to be stored in the key ring.  If you have obtained a
 * remote public key through other means, you can pass it to the options
 * block with setParameter():
 *
 * \code
 * uint8_t remoteKey[32] = {...};
 *
 * NoiseClientOptions.setParameter(Noise::RemoteStaticPublicKey, remoteKey, 32);
 * \endcode
 *
 * At present, NoiseLinkOptions only supports Curve25519 keys and PSK's.
 * Support for other Noise DH algorithms is not implemented yet.
 */

/**
 * \brief Default options to use when establishing an outgoing session
 * from a client device using NoiseClient.
 */
NoiseLinkOptions NoiseClientOptions;

/**
 * \brief Default options to use when establishing an incoming session
 * on a server device using NoiseServer.
 */
NoiseLinkOptions NoiseServerOptions;

/** @cond noise_link_options_private */

#define HAVE_KEY_PAIR_25519     0x01
#define HAVE_REMOTE_KEY_25519   0x02
#define HAVE_PSK                0x04

#define MAX_PROTOCOLS           4

class NoiseLinkOptionsPrivate
{
public:
    NoiseLinkOptionsPrivate()
        : allowAliases(true)
        , padding(Noise::NoPadding)
        , sendSize(NOISE_DEFAULT_BUFSIZ)
        , recvSize(NOISE_DEFAULT_BUFSIZ)
        , numProtocols(0)
    {
        st.flags = 0;
    }
    ~NoiseLinkOptionsPrivate() { clean(&st, sizeof(st)); }

    void clear() { clean(&st, sizeof(st)); }

    void copyFrom(const NoiseLinkOptionsPrivate *other)
    {
        memcpy(&st, &(other->st), sizeof(st));
        allowAliases = other->allowAliases;
        padding = other->padding;
        sendSize = other->sendSize;
        recvSize = other->recvSize;
        numProtocols = other->numProtocols;
        memcpy(protocols, other->protocols,
               numProtocols * sizeof(protocols[0]));
    }

    struct {
        uint8_t flags;
        uint8_t keyPair25519[64];
        uint8_t remoteKey25519[32];
        uint8_t psk[32];
    } st;
    bool allowAliases;
    Noise::Padding padding;
    uint16_t sendSize;
    uint16_t recvSize;
    size_t numProtocols;
    const NoiseProtocolDescriptor *protocols[MAX_PROTOCOLS];
};

/** @endcond */

/**
 * \brief Constructs a new options block.
 */
NoiseLinkOptions::NoiseLinkOptions()
    : d(new NoiseLinkOptionsPrivate())
{
}

/**
 * \brief Destroys this options block.
 */
NoiseLinkOptions::~NoiseLinkOptions()
{
    delete d;
}

/**
 * \brief Sets a parameter within this options block.
 *
 * \param id Identifies the parameter.
 * \param value Points to the value to set for the parameter.
 * \param size Size of the parameter value in bytes.
 *
 * \return Returns true if the parameter is set; false if the parameter
 * \a id is not supported; or false if the \a size is not valid for the
 * parameter.
 *
 * If the \a id is for the private key component of a key pair, then the
 * public key component will be generated from \a value.  Returns false if
 * it is not possible to generate the public key component from the
 * private key component.
 *
 * \sa getParameter(), getParameterSize()
 */
bool NoiseLinkOptions::setParameter(Noise::Parameter id, const void *value, size_t size)
{
    if (!value)
        return false;
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        if (size != 64)
            break;
        memcpy(d->st.keyPair25519, value, 64);
        d->st.flags |= HAVE_KEY_PAIR_25519;
        return true;
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
        if (size != 32)
            break;
        memcpy(d->st.keyPair25519, value, 32);
        d->st.keyPair25519[0] &= 0xF8;
        d->st.keyPair25519[31] = (d->st.keyPair25519[31] & 0x7F) | 0x40;
        Curve25519::eval(d->st.keyPair25519 + 32, d->st.keyPair25519, 0);
        d->st.flags |= HAVE_KEY_PAIR_25519;
        return true;
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        if (size != 32)
            break;
        memcpy(d->st.remoteKey25519, value, 32);
        d->st.flags |= HAVE_REMOTE_KEY_25519;
        return true;
    case Noise::PreSharedKey:
        if (size != 32)
            break;
        memcpy(d->st.psk, value, 32);
        d->st.flags |= HAVE_PSK;
        return true;
    default: break;
    }
    return false;
}

/**
 * \brief Gets the value associated from within this options block.
 *
 * \param id Identifies the parameter.
 * \param value Points to the buffer to write the parameter value to.
 * \param maxSize Maximum size of the \a value buffer in bytes.
 *
 * \return The number of bytes that were written to \a value.
 * \return 0 if \a id is not valid.
 * \return 0 if \a maxSize is not large enough to hold the parameter value.
 * \return 0 if the parameter \a id has not been set yet.
 *
 * The getParameterSize() function can be used to determine the actual
 * size of the parameter so that the caller can allocate a buffer large
 * enough to contain it ahead of time.
 *
 * \sa setParameter(), getParameterSize()
 */
size_t NoiseLinkOptions::getParameter(Noise::Parameter id, void *value, size_t maxSize) const
{
    if (!value)
        return 0;
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        if (maxSize < 64 || !(d->st.flags & HAVE_KEY_PAIR_25519))
            break;
        memcpy(value, d->st.keyPair25519, 64);
        return 64;
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
        if (maxSize < 32 || !(d->st.flags & HAVE_KEY_PAIR_25519))
            break;
        memcpy(value, d->st.keyPair25519, 32);
        return 32;
    case Noise::LocalStatic25519PublicKey:
    case Noise::LocalStaticPublicKey:
        if (maxSize < 32 || !(d->st.flags & HAVE_KEY_PAIR_25519))
            break;
        memcpy(value, d->st.keyPair25519 + 32, 32);
        return 32;
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        if (maxSize < 32 || !(d->st.flags & HAVE_KEY_PAIR_25519))
            break;
        memcpy(value, d->st.remoteKey25519, 32);
        return 32;
    case Noise::PreSharedKey:
        if (maxSize < 32 || !(d->st.flags & HAVE_PSK))
            break;
        memcpy(value, d->st.psk, 32);
        return 32;
    default: break;
    }
    return 0;
}

/**
 * \brief Gets the size of a parameter within this options block.
 *
 * \param id Identifies the parameter.
 *
 * \return The parameter's size, or zero if \a id is not recognised.
 *
 * If the parameter is variable-length, then this function will return
 * the actual length of the value, which may be zero if the value has
 * not been set yet.
 *
 * \sa getParameter(), setParameter(), hasParameter()
 */
size_t NoiseLinkOptions::getParameterSize(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
        return 64;
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
    case Noise::PreSharedKey:
        return 32;
    default:
        return 0;
    }
}

/**
 * \brief Determine if a parameter has a value set within this options block.
 *
 * \param id Identifies the parameter.
 *
 * \return Returns true if \a id has a value; false if \a id does not
 * have a value; false if \a id is not valid.
 *
 * \sa getParameterSize(), getParameter(), removeParameter()
 */
bool NoiseLinkOptions::hasParameter(Noise::Parameter id) const
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        return (d->st.flags & HAVE_KEY_PAIR_25519) != 0;
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        return (d->st.flags & HAVE_REMOTE_KEY_25519) != 0;
    case Noise::PreSharedKey:
        return (d->st.flags & HAVE_PSK) != 0;
    default:
        return false;
    }
}

/**
 * \brief Removes a parameter from this options block.
 *
 * \param id Identifies the parameter.  If the parameter is unknown,
 * the request will be ignored.
 *
 * The clear() function will remove all parameters in a single call.
 *
 * \sa clear(), hasParameter()
 */
void NoiseLinkOptions::removeParameter(Noise::Parameter id)
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        clean(d->st.keyPair25519, 64);
        d->st.flags &= ~HAVE_KEY_PAIR_25519;
        break;
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        clean(d->st.remoteKey25519, 32);
        d->st.flags &= ~HAVE_REMOTE_KEY_25519;
        break;
    case Noise::PreSharedKey:
        clean(d->st.psk, 32);
        d->st.flags &= ~HAVE_PSK;
        break;
    default: break;
    }
}

/**
 * \brief Loads a parameter from the device's key ring.
 *
 * \param id Identifies the parameter to load.
 * \param keyRingId Identifier of the stored key within the key ring,
 * or zero to use the default key ring identifier for \a id.
 *
 * \return Returns true if the parameter was loaded.
 * \return Returns false if \a id is not a valid parameter.
 * \return Returns false if \a keyRingId does not have a key stored for it.
 *
 * The following example loads the Curve25519 key pair for the local
 * device and the public key for the remote device that we are expecting
 * to communicate with.  If the local device does not have a key pair,
 * then this example will generate a new one:
 *
 * \code
 * if (!NoiseClientOptions.load(Noise::LocalStaticKeyPair)) {
 *     Serial.println("Generating a new key pair, please wait ...");
 *     NoiseClientOptions.generate(Noise::LocalStaticKeyPair);
 * }
 * NoiseClientOptions.load(Noise::RemoteStaticPublicKey);
 * \endcode
 *
 * If the key ring is protected by an encryption key or passphrase,
 * then it is assumed that the key ring was already unlocked by a
 * previous call to KeyRing.begin().
 *
 * \sa generate(), save()
 */
bool NoiseLinkOptions::load(Noise::Parameter id, uint16_t keyRingId)
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::LocalCurve25519Default;
        if (KeyRing.getLocalKeyPair(keyRingId, d->st.keyPair25519, 64) == 64) {
            d->st.flags |= HAVE_KEY_PAIR_25519;
            return true;
        }
        clean(d->st.keyPair25519, 64);
        d->st.flags &= ~HAVE_KEY_PAIR_25519;
        break;
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::RemoteCurve25519Default;
        if (KeyRing.getRemotePublicKey
                (keyRingId, d->st.remoteKey25519, 32) == 32) {
            d->st.flags |= HAVE_REMOTE_KEY_25519;
            return true;
        }
        clean(d->st.remoteKey25519, 32);
        d->st.flags &= ~HAVE_REMOTE_KEY_25519;
        break;
    case Noise::PreSharedKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::PreSharedKeyDefault;
        if (KeyRing.getSharedSymmetricKey(keyRingId, d->st.psk, 32) == 32) {
            d->st.flags |= HAVE_PSK;
            return true;
        }
        clean(d->st.psk, 32);
        d->st.flags &= ~HAVE_PSK;
        break;
    default: break;
    }
    return false;
}

/**
 * \brief Saves a parameter into the device's key ring.
 *
 * \param id Identifies the parameter to save.
 * \param keyRingId Identifier of the stored key within the key ring,
 * or zero to use the default key ring identifier for \a id.
 *
 * \return Returns true if the parameter was saved.
 * \return Returns false if \a id is not a valid parameter or it does
 * not currently have a value in this options block.
 * \return Returns false if the key ring does not have enough space
 * to store the parameter.
 *
 * If the key ring is protected by an encryption key or passphrase,
 * then it is assumed that the key ring was already unlocked by a
 * previous call to KeyRing.begin().
 *
 * \sa load(), generate()
 */
bool NoiseLinkOptions::save(Noise::Parameter id, uint16_t keyRingId)
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::LocalCurve25519Default;
        if ((d->st.flags & HAVE_KEY_PAIR_25519) == 0)
            break;
        return KeyRing.setLocalKeyPair(keyRingId, d->st.keyPair25519, 64);
    case Noise::RemoteStatic25519PublicKey:
    case Noise::RemoteStaticPublicKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::RemoteCurve25519Default;
        if ((d->st.flags & HAVE_REMOTE_KEY_25519) == 0)
            break;
        return KeyRing.setRemotePublicKey(keyRingId, d->st.remoteKey25519, 32);
    case Noise::PreSharedKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::PreSharedKeyDefault;
        if ((d->st.flags & HAVE_PSK) == 0)
            break;
        return KeyRing.setSharedSymmetricKey(keyRingId, d->st.psk, 32);
    default: break;
    }
    return false;
}

/**
 * \brief Generates a new local key pair for the device and saves it
 * into the device's key ring.
 *
 * \param id Identifies the key pair to be generated.
 * \param keyRingId Identifier for the saved key within the key ring,
 * or zero to use the default key ring identifier for \a id.
 * \param wait Set to true to wait for the system random number generator
 * to have enough entropy to safely generate the key; false to immediately
 * generate the key with whatever entropy is available at the moment.
 * The default value for this parameter is true.
 *
 * \return Returns true if the new key pair was generated and saved.
 * \return Returns false if \a id does not specify a valid key pair name.
 * \return Returns false if there is not enough space left in the
 * key ring to save the new key.
 *
 * The new key pair will also be loaded into this options block, ready for use.
 *
 * If the key ring is protected by an encryption key or passphrase,
 * then it is assumed that the key ring was already unlocked by a
 * previous call to KeyRing.begin().
 *
 * \note If \a wait is true then this function may block for a long
 * time on some devices to accumulate enough entropy to safely generate
 * the key pair.  Alternatively, the application can poll
 * \link RNGClass::available() RNG.available()\endlink itself and call
 * this function with \a wait set to false when ready; allowing the
 * application to perform other setup tasks while waiting.
 *
 * \sa load(), save()
 */
bool NoiseLinkOptions::generate
    (Noise::Parameter id, uint16_t keyRingId, bool wait)
{
    switch (id) {
    case Noise::LocalStaticKeyPair:
    case Noise::LocalStatic25519KeyPair:
    case Noise::LocalStaticPrivateKey:
    case Noise::LocalStatic25519PrivateKey:
    case Noise::LocalStaticPublicKey:
    case Noise::LocalStatic25519PublicKey:
        if (!keyRingId)
            keyRingId = KeyRingClass::LocalCurve25519Default;
        while (wait && !RNG.available(32)) {
            RNG.loop();
            crypto_feed_watchdog();
        }
        Curve25519::generateKeyPair(d->st.keyPair25519);
        if (KeyRing.setLocalKeyPair(keyRingId, d->st.keyPair25519, 64)) {
            d->st.flags |= HAVE_KEY_PAIR_25519;
            return true;
        }
        clean(d->st.keyPair25519, 64);
        d->st.flags &= ~HAVE_KEY_PAIR_25519;
        break;
    default: break;
    }
    return false;
}

/**
 * \brief Gets the number of protocols that have been added to this
 * options block.
 *
 * \return The number of protocols.
 *
 * \sa protocolAt(), addProtocol(), removeProtocol()
 */
size_t NoiseLinkOptions::protocolCount() const
{
    return d->numProtocols;
}

/**
 * \brief Gets a specific protocol from this options block.
 *
 * \param index The index of the protocol between 0 and protocolCount() - 1.
 *
 * \return A pointer to the protocol descriptor or NULL if \a index
 * is out of range.
 *
 * \sa protocolCount(), addProtocol(), removeProtocol()
 */
const NoiseProtocolDescriptor *NoiseLinkOptions::protocolAt(size_t index) const
{
    if (index < d->numProtocols)
        return d->protocols[index];
    else
        return 0;
}

/**
 * \brief Adds a protocol to this options block.
 *
 * \param protocol The new protocol to add.
 *
 * The caller can add a maximum of 4 protocols to an options block.
 * The first protocol added is the one used for outgoing connections
 * using NoiseClient.  All protocols can be accepted for incoming
 * connections using NoiseServer.
 *
 * \sa removeProtocol(), protocolCount(), protocolAt()
 */
void NoiseLinkOptions::addProtocol(const NoiseProtocolDescriptor &protocol)
{
    if (d->numProtocols < MAX_PROTOCOLS)
        d->protocols[(d->numProtocols)++] = &protocol;
}

/**
 * \brief Removes a protocol from this options block.
 *
 * \param protocol The protocol to remove.
 *
 * \sa addProtocol(), protocolCount(), protocolAt()
 */
void NoiseLinkOptions::removeProtocol(const NoiseProtocolDescriptor &protocol)
{
    for (size_t index = 0; index < d->numProtocols; ++index) {
        if (d->protocols[index] == &protocol) {
            memmove(d->protocols + index, d->protocols + index + 1,
                    (d->numProtocols - index - 1) * sizeof(d->protocols[0]));
            --(d->numProtocols);
            return;
        }
    }
}

/**
 * \brief Gets the size of the send buffer when a session is in progress.
 *
 * \return The size of the send buffer between 128 and 2048; default is 512.
 *
 * \sa setSendBufferSize(), receiveBufferSize()
 */
size_t NoiseLinkOptions::sendBufferSize() const
{
    return d->sendSize;
}

/**
 * \brief Sets the size of the send buffer when a session is in progress.
 *
 * \param size The size of the send buffer between 128 and 2048.
 *
 * \sa sendBufferSize(), setReceiveBufferSize(), setBufferSize()
 */
void NoiseLinkOptions::setSendBufferSize(size_t size)
{
    if (size < NOISE_MIN_BUFSIZ)
        size = NOISE_MIN_BUFSIZ;
    else if (size > NOISE_MAX_BUFSIZ)
        size = NOISE_MAX_BUFSIZ;
    d->sendSize = size;
}

/**
 * \brief Gets the size of the receive buffer when a session is in progress.
 *
 * \return The size of the receive buffer between 128 and 2048; default is 512.
 *
 * \sa setReceiveBufferSize(), sendBufferSize()
 */
size_t NoiseLinkOptions::receiveBufferSize() const
{
    return d->recvSize;
}

/**
 * \brief Sets the size of the receive buffer when a session is in progress.
 *
 * \param size The size of the receive buffer between 128 and 2048.
 *
 * \sa receiveBufferSize(), setSendBufferSize(), setBufferSize()
 */
void NoiseLinkOptions::setReceiveBufferSize(size_t size)
{
    if (size < NOISE_MIN_BUFSIZ)
        size = NOISE_MIN_BUFSIZ;
    else if (size > NOISE_MAX_BUFSIZ)
        size = NOISE_MAX_BUFSIZ;
    d->recvSize = size;
}

/**
 * \fn void NoiseLinkOptions::setBufferSize(size_t size)
 * \brief Sets the size of the send and receive buffers to the same value.
 *
 * \param size The size of the send and receive buffers between 128 and 2048.
 *
 * \sa setSendBufferSize(), setReceiveBufferSize()
 */

/**
 * \brief Determine if NoiseTinyLink protocol name aliases are allowed.
 *
 * \return Returns true if protocol name aliases are allowed or false if not.
 * The default is true.
 *
 * Protocol name aliases are strings like "2" instead of a full protocol
 * name like "Noise_XX_25519_ChaChaPoly_SHA256".  This can save bytes on
 * the network when making an outgoing connection using NoiseClient.
 *
 * Incoming connections via NoiseServer can use either full names or aliases.
 * This option has no effect on NoiseServer.
 *
 * If a protocol does not have a name alias, the full name will be used
 * regardless of this option's setting.
 *
 * If your device is communicating with a server that is running the full
 * version of NoiseLink, then it may not have support for protocol name
 * aliases.  In that case, use setAllowAliases(false) to force the use
 * of the full protocol name when communicating with that server.
 *
 * \sa setAllowAliases()
 */
bool NoiseLinkOptions::allowAliases() const
{
    return d->allowAliases;
}

/**
 * \brief Enables or disables the use of NoiseTinyLink protocol name aliases.
 *
 * \param allow Set to true if protocol name aliases are allowed; false if not.
 *
 * \sa allowAliases()
 */
void NoiseLinkOptions::setAllowAliases(bool allow)
{
    d->allowAliases = allow;
}

/**
 * \brief Gets the type of padding to use on plaintext before encryption.
 *
 * \return The type of padding; default is Noise::NoPadding.
 *
 * \sa setPadding()
 */
Noise::Padding NoiseLinkOptions::padding() const
{
    return d->padding;
}

/**
 * \brief Sets the type of padding to use on plaintext before encryption.
 *
 * \param padding The type of padding; Noise::NoPadding, Noise::ZeroPadding,
 * or Noise::RandomPadding.
 *
 * If \a padding is Noise::ZeroPadding, then the plaintext on outgoing
 * packets will be padded with zeroes to sendBufferSize() before encryption.
 *
 * If \a padding is Noise::RandomPadding, then the plaintext on outgoing
 * packets will be padded with random data to sendBufferSize() before
 * encryption.
 *
 * \sa setPadding()
 */
void NoiseLinkOptions::setPadding(Noise::Padding padding)
{
    d->padding = padding;
}

/**
 * \brief Copies all values from another options block into this one.
 *
 * \param options The other options block to copy from.
 */
void NoiseLinkOptions::copyFrom(const NoiseLinkOptions &options)
{
    if (d != options.d)
        d->copyFrom(options.d);
}

/**
 * \brief Destroys all security-sensitive data within this options block.
 *
 * If you only need to destroy a specific parameter (e.g. the local key pair),
 * then you can use removeParameter() instead.
 *
 * \sa removeParameter()
 */
void NoiseLinkOptions::clear()
{
    d->clear();
}

/**
 * \brief Creates a Noise handshake object for a specific protocol.
 *
 * \param party The party to create the handshake for; one of
 * Noise::Initiator or Noise::Responder.
 * \param protocolIndex The index of the protocol in this options block.
 *
 * \return A pointer to a new handshake object, populated with the keys
 * that are needed to initialize the handshake.
 * \return Returns NULL if \a protocolIndex is invalid or there are
 * insufficient keys in this options block to initialize the handshake.
 *
 * \sa protocolAt()
 */
NoiseHandshakeState *NoiseLinkOptions::createHandshake
    (Noise::Party party, size_t protocolIndex) const
{
    // Find the protocol to be created.
    const NoiseProtocolDescriptor *desc = protocolAt(protocolIndex);
    if (!desc)
        return 0;

    // Verify that we have sufficient parameters to start the protocol.
    unsigned short flags;
    if (party == Noise::Initiator)
        flags = desc->initiatorFlags;
    else
        flags = desc->responderFlags;
    if ((flags & NOISE_PROTOCOL_NEEDS_LOCAL_STATIC) != 0 &&
            (d->st.flags & HAVE_KEY_PAIR_25519) == 0)
        return 0;
    if ((flags & NOISE_PROTOCOL_NEEDS_REMOTE_STATIC) != 0 &&
            (d->st.flags & HAVE_REMOTE_KEY_25519) == 0)
        return 0;
    if ((flags & NOISE_PROTOCOL_NEEDS_PSK) != 0 &&
            (d->st.flags & HAVE_PSK) == 0)
        return 0;

    // Create the handshake object.
    NoiseHandshakeState *handshake = desc->createHandshake();
    if (!handshake)
        return 0;

    // Add all required keys.  It is possible that this may fail if the
    // protocol uses a different DH algorithm than Curve25519 or the
    // flags in the protocol descriptor were incorrect for the handshake.
    bool ok = true;
    if ((flags & NOISE_PROTOCOL_NEEDS_LOCAL_STATIC) != 0) {
        if (!handshake->setParameter
                (Noise::LocalStatic25519KeyPair, d->st.keyPair25519, 64))
            ok = false;
    }
    if (ok && (flags & NOISE_PROTOCOL_NEEDS_REMOTE_STATIC) != 0) {
        if (!handshake->setParameter
                (Noise::RemoteStatic25519PublicKey, d->st.remoteKey25519, 32))
            ok = false;
    }
    if (ok && (flags & NOISE_PROTOCOL_NEEDS_PSK) != 0) {
        if (!handshake->setParameter(Noise::PreSharedKey, d->st.psk, 32))
            ok = false;
    }
    if (!ok) {
        delete handshake;
        return 0;
    }

    // Ready to go.
    return handshake;
}
