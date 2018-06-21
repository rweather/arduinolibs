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

#include "NoiseHandshakeState.h"
#include "Crypto.h"
#include <string.h>

/**
 * \class NoiseHandshakeState NoiseHandshakeState.h <NoiseHandshakeState.h>
 * \brief Abstract base class for the handshake phase of Noise protocols.
 *
 * This class is abstract.  Subclasses provide implementations of specific
 * Noise patterns like "XX".
 */

/**
 * \class NoiseHandshakeStatePSK NoiseHandshakeState.h <NoiseHandshakeState.h>
 * \brief Abstract base class for the handshake phase of Noise protocols,
 * when that handshake involves a pre-shared key.
 *
 * This class is abstract.  Subclasses provide implementations of specific
 * Noise patterns like "NNpsk0".
 *
 * When a PSK-using handshake is initialized, the Noise::PreSharedKey
 * parameter must be set on the handshake using setParameter().  Otherwise
 * the handshake will fail when the PSK is needed later.  For example:
 *
 * \code
 * uint8_t psk[32] = {...};
 * Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s handshake;
 * handshake.setParameter(Noise::PreSharedKey, psk, sizeof(psk));
 * handshake.start(Noise::Initiator);
 * \endcode
 */

/**
 * \brief Constructs a new Noise handshake.
 */
NoiseHandshakeState::NoiseHandshakeState()
    : sym(0)
    , dh(0)
    , protoName(0)
    , pty(Noise::Initiator)
    , st(Noise::Failed)
    , msgnum(0)
{
}

/**
 * \brief Destroys this Noise handshake.
 */
NoiseHandshakeState::~NoiseHandshakeState()
{
}

/**
 * \brief Starts the handshake for the local party.
 *
 * \param party Noise::Initiator or Noise::Responder.
 * to identity the local party.
 * \param prologue Points to the prologue string, or NULL if no prologue.
 * \param prologueLen Length of the prologue string, or 0 if no prologue.
 *
 * This function should be followed by calls to write() and read() to
 * process the packets in the handshake.  Once the handshake has completed,
 * the application should call split() to obtain the cipher objects to use
 * to encrypt and decrypt transport messages.
 *
 * If the prologue is split into multiple sections, then start() can be
 * followed by calls to addPrologue() to add the additional sections
 * prior to calling write() or read() for the first time.
 *
 * \sa state(), write(), read(), split(), addPrologue()
 */
void NoiseHandshakeState::start
    (Noise::Party party, const void *prologue, size_t prologueLen)
{
    pty = party;
    st = (party == Noise::Initiator) ? Noise::Write : Noise::Read;
    msgnum = 0;
    removeKeys();
    symmetricState()->initialize(protoName);
    symmetricState()->mixPrologue(prologue, prologueLen);
}

/**
 * \brief Adds additional prologue data to the handshake.
 *
 * \param prologue Points to the additional prologue data.
 * \param prologueLen Length of the prologue additional data in bytes.
 *
 * \return Returns true if the data was added, or false if the handshake
 * is not in the state just after start().  Once the first write() or read()
 * occurs, no further prologue data can be added.
 *
 * \sa start()
 */
bool NoiseHandshakeState::addPrologue(const void *prologue, size_t prologueLen)
{
    if (st != Noise::Write && st != Noise::Read)
        return false;
    return symmetricState()->mixPrologue(prologue, prologueLen);
}

/**
 * \fn Noise::Party NoiseHandshakeState::party() const
 * \brief Gets the identity of this party to the handshake.
 *
 * \return Noise::Initiator or Noise::Responder.
 */

/**
 * \fn Noise::HandshakeState NoiseHandshakeState::state() const
 * \brief Gets the state of the handshake.
 *
 * \return Noise::Write if the handshake is still in progress and the next
 * operation is write().
 * \return Noise::Read if the handshake is still in progress and the next
 * operation is read().
 * \return Noise::Split if the handshake has completed and the next operation
 * is split().
 * \return Noise::Finished if the handshake has completed and split() has
 * already been called.
 * \return Noise::Failed if the handshake has failed or it was never started.
 *
 * \sa write(), read()
 */

/**
 * \fn const char *NoiseHandshakeState::protocolName() const
 * \brief Gets the full name of the Noise protocol that is implemented
 * by this handshake object.
 *
 * \return The full name of the Noise protocol; for example
 * "Noise_XX_25519_ChaChaPoly_BLAKE2s".
 */

/**
 * \brief Sets a parameter on the handshake.
 *
 * \param id Identifies the parameter.
 * \param value Points to the value to set for the parameter.
 * \param size Size of the parameter value in bytes.
 *
 * \return Returns true if the parameter is set; false if the parameter
 * \a id is not supported by the handshake; or false if the \a size is
 * not valid for the parameter.
 *
 * If the \a id is for the private key component of a key pair, then the
 * public key component will be generated from \a value.  Returns false if
 * it is not possible to generate the public key component from the
 * private key component.
 *
 * \sa getParameter(), getParameterSize()
 */
bool NoiseHandshakeState::setParameter
    (Noise::Parameter id, const void *value, size_t size)
{
    if (dh)
        return dh->setParameter(id, value, size);
    return false;
}

/**
 * \brief Gets the value associated with a parameter on the handshake.
 *
 * \param id Identifies the parameter.
 * \param value Points to the buffer to write the parameter value to.
 * \param maxSize Maximum size of the \a value buffer in bytes.
 *
 * \return The number of bytes that were written to \a value.
 * \return 0 if \a id is not supported by the handshake.
 * \return 0 if \a maxSize is not large enough to hold the parameter value.
 * \return 0 if the parameter \a id has not been set yet.
 *
 * The getParameterSize() function can be used to determine the actual
 * size of the parameter so that the caller can allocate a buffer large
 * enough to contain it ahead of time.
 *
 * \sa setParameter(), getParameterSize()
 */
size_t NoiseHandshakeState::getParameter
    (Noise::Parameter id, void *value, size_t maxSize) const
{
    if (dh)
        return dh->getParameter(id, value, maxSize);
    return 0;
}

/**
 * \brief Gets the size of a parameter on the handshake.
 *
 * \param id Identifies the parameter.
 *
 * \return The parameter's size, or zero if \a id is not valid for
 * the handshake.
 *
 * If the parameter is variable-length, then this function will return
 * the actual length of the value, which may be zero if the value has
 * not been set yet.
 *
 * \sa getParameter(), setParameter(), hasParameter()
 */
size_t NoiseHandshakeState::getParameterSize(Noise::Parameter id) const
{
    if (dh)
        return dh->getParameterSize(id);
    return 0;
}

/**
 * \brief Determine if a parameter on this handshake has a value set.
 *
 * \param id Identifies the parameter.
 *
 * \return Returns true if \a id has a value; false if \a id does not
 * have a value; false if \a id is not valid for the handshake.
 *
 * \sa getParameterSize(), getParameter(), removeParameter()
 */
bool NoiseHandshakeState::hasParameter(Noise::Parameter id) const
{
    if (dh)
        return dh->hasParameter(id);
    return false;
}

/**
 * \brief Removes a parameter from this handshake.
 *
 * \param id Identifies the parameter.  If the parameter is unknown
 * to the handshake, the request will be ignored.
 *
 * \sa hasParameter()
 */
void NoiseHandshakeState::removeParameter(Noise::Parameter id)
{
    if (dh)
        dh->removeParameter(id);
}

/**
 * \brief Formats the next handshake packet to be written to the transport.
 *
 * \param output Points to a buffer to receive the formatted handshake packet.
 * \param maxOutputSize Maximum size of the \a output buffer in bytes.
 * \param payload Points to the application-supplied payload to be encrypted
 * and sent with the handshake packet.  May be NULL for no payload.
 * \param payloadSize Number of bytes of payload; zero if no payload.
 *
 * \return The number of bytes that were written to \a output if the
 * return value is greater than or equal to zero.
 * \return -1 if there is insufficient space in the \a output buffer for the
 * full handshake packet, or the state() is not NoiseHandshakeState::Write.
 *
 * After this function returns successfully, the application should call
 * state() to determine what operation to perform next: read() or split().
 *
 * \sa read(), split(), state()
 */
int NoiseHandshakeState::write
    (void *output, size_t maxOutputSize,
     const void *payload, size_t payloadSize)
{
    // Cannot write if the handshake is not in the correct state.
    if (st != Noise::Write)
        return -1;

    // Form a packet structure for the output buffer.
    Packet packet;
    packet.data = (uint8_t *)output;
    packet.posn = 0;
    packet.size = maxOutputSize;
    packet.error = false;
    packet.done = false;

    // Run the tokens for the handshake pattern and format the payload.
    writeTokens(packet, msgnum);
    writePayload(packet, payload, payloadSize);

    // Move onto the next state.
    if (packet.error) {
        st = Noise::Failed;
        return -1;
    } else if (packet.done) {
        st = Noise::Split;
        return (int)packet.posn;
    } else {
        ++msgnum;
        st = Noise::Read;
        return (int)packet.posn;
    }
}

/**
 * \brief Reads the contents of the next handshake packet from the transport.
 *
 * \param payload Points to a buffer to receive the decrypted payload data.
 * \param maxPayloadSize Maximum size of the \a payload buffer in bytes.
 * \param input Points to the incoming handshake packet from the transport.
 * \param inputSize Number of bytes in the incoming handshake packet.
 *
 * \return The number of bytes that were written to \a payload if the
 * return value is greater than or equal to zero.
 * \return -1 if there is insufficient space in the \a payload buffer for the
 * full payload, the state() is not NoiseHandshakeState::Read, or an
 * error occurred while decrypting the packet.
 *
 * After this function returns successfully, the application should call
 * state() to determine what operation to perform next: write() or split().
 *
 * \sa write(), split(), state()
 */
int NoiseHandshakeState::read
    (void *payload, size_t maxPayloadSize,
     const void *input, size_t inputSize)
{
    // Cannot read if the handshake is not in the correct state.
    if (st != Noise::Read)
        return -1;

    // Form a packet structure for the input buffer.
    Packet packet;
    packet.data = (uint8_t *)(const_cast<void *>(input));
    packet.posn = 0;
    packet.size = inputSize;
    packet.error = false;
    packet.done = false;

    // Run the tokens for the handshake pattern and extract the payload.
    readTokens(packet, msgnum);
    size_t result = readPayload(packet, payload, maxPayloadSize);

    // Move onto the next state.
    if (packet.error) {
        st = Noise::Failed;
        return -1;
    } else if (packet.done) {
        st = Noise::Split;
        return (int)result;
    } else {
        ++msgnum;
        st = Noise::Write;
        return (int)result;
    }
}

/**
 * \brief Splits cipher objects for the transport phase of the session out
 * of this handshake state once the handshake ends.
 *
 * \param tx Returns a pointer to the cipher object to use to encrypt messages
 * for transmission to the remote party.
 * \param rx Returns a pointer to the cipher object to use to decrypt messages
 * that were received from the remote party.
 *
 * \return Returns true if the cipher objects were split out, or false if
 * state() is not NoiseHandshakeState::Split.
 *
 * If \a tx or \a rx are NULL, then the respective cipher object will not
 * be created.  This is useful for one-way patterns.
 *
 * The application is responsible for destroying the \a tx and \a rx
 * objects when it no longer requires them.
 *
 * \sa write(), read(), getHandshakeHash()
 */
bool NoiseHandshakeState::split
    (NoiseCipherState **tx, NoiseCipherState **rx)
{
    if (tx)
        *tx = 0;
    if (rx)
        *rx = 0;
    if (st != Noise::Split)
        return false;
    if (pty == Noise::Initiator)
        symmetricState()->split(tx, rx);
    else
        symmetricState()->split(rx, tx);
    st = Noise::Finished;
    return true;
}

/**
 * \brief Gets the handshake hash after the handshake completes.
 *
 * \param data Data buffer to fill with the handshake hash.
 * \param size Size of the \a data buffer in bytes.
 *
 * \return Returns true if the handshake hash was copied to \a data,
 * or false if the handshake has not completed yet.
 *
 * If \a size is less than the size of the symmetric state's hash,
 * then the value will be truncated to the first \a size bytes.
 * If \a size is greater, then the extra bytes will be filled with zeroes.
 *
 * This function can be called just before or after split().  If the
 * handshake has not completed yet or it is has failed, then the hash
 * value will be unavailable.
 *
 * \sa split()
 */
bool NoiseHandshakeState::getHandshakeHash(void *data, size_t size)
{
    if (st != Noise::Split && st != Noise::Finished) {
        memset(data, 0, size);
        return false;
    }
    symmetricState()->getHandshakeHash(data, size);
    return true;
}

/**
 * \brief Clears all sensitive data from this handshake object.
 *
 * The application will need to call start() again to begin a new handshake.
 */
void NoiseHandshakeState::clear()
{
    sym->clear();
    dh->clear();
    pty = Noise::Initiator;
    st = Noise::Failed;
    msgnum = 0;
}

/**
 * \fn NoiseSymmetricState *NoiseHandshakeState::symmetricState() const
 * \brief Gets the symmetric state associated with this handshake.
 *
 * \return A pointer to the symmetric state.
 *
 * \sa setSymmetricState()
 */

/**
 * \fn void NoiseHandshakeState::setSymmetricState(NoiseSymmetricState *symState)
 * \brief Sets the symmetric state for this handshake.
 *
 * \param symState A pointer to the symmetric state.
 *
 * This function must be called from a subclass constructor to initialize
 * the handshake object properly before start() is called.
 *
 * \sa symmetricState()
 */

/**
 * \fn NoiseDHState *NoiseHandshakeState::dhState() const
 * \brief Gets the DH object for this handshake.
 *
 * \return A pointer to the DH object.
 *
 * \sa setDHState()
 */

/**
 * \fn void NoiseHandshakeState::setDHState(NoiseDHState *dhState)
 * \brief Sets the DH object for this handshake.
 *
 * \param dhState A pointer to the DH object.
 *
 * This function must be called from a subclass constructor to initialize
 * the handshake object properly before start() is called.
 *
 * \sa dhState()
 */

/**
 * \fn const NoiseDHState *NoiseHandshakeState::otherDHState(const NoiseHandshakeState *handshake)
 * \brief Gets the DH object within another handshake.
 *
 * \param handshake The other handshake.
 *
 * \return A pointer to the DH object for \a handshake.
 */

/**
 * \fn void NoiseHandshakeState::setProtocolName(const char *name)
 * \brief Sets the protocol name for this handshake.
 *
 * \param name The full Noise protocol name.
 *
 * This function must be called from a subclass constructor to initialize
 * the handshake object properly before start() is called.
 *
 * \sa protocolName()
 */

/**
 * \brief Removes keys from the parameter list that need to be
 * generated or discovered during the course of the handshake.
 *
 * The default implementation removes the local and remote ephemeral keys
 * but leaves local and remote static keys alone.  Subclasses may also
 * remove the remote static key if it is expected to be discovered during
 * the handshake.
 */
void NoiseHandshakeState::removeKeys()
{
    removeParameter(Noise::LocalEphemKeyPair);
    removeParameter(Noise::RemoteEphemPublicKey);
}

/**
 * \fn void NoiseHandshakeState::writeTokens(NoiseHandshakeState::Packet &packet, uint8_t msgnum)
 * \brief Runs the tokens for a handshake message in write mode.
 *
 * \param packet The packet to format based on the tokens.
 * \param msgnum The message number within the handshake, 0 for the first
 * message, 1 for the second, and so on.
 */

/**
 * \fn void NoiseHandshakeState::readTokens(NoiseHandshakeState::Packet &packet, uint8_t msgnum)
 * \brief Runs the tokens for a handshake message in read mode.
 *
 * \param packet The packet to read based on the tokens.
 * \param msgnum The message number within the handshake, 0 for the first
 * message, 1 for the second, and so on.
 */

/**
 * \brief Processes an "e" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * If there is not enough room in \a packet to hold the ephemeral public key,
 * then the error flag will be set in \a packet.
 *
 * \sa read_e()
 */
void NoiseHandshakeState::write_e(NoiseHandshakeState::Packet &packet)
{
    size_t len = dhState()->getParameterSize(Noise::LocalEphemPublicKey);
    if ((packet.size - packet.posn) >= len) {
        dhState()->generateLocalEphemeralKeyPair();
        dhState()->getParameter
            (Noise::LocalEphemPublicKey, packet.data + packet.posn, len);
        symmetricState()->mixHash(packet.data + packet.posn, len);
        packet.posn += len;
    } else {
        packet.error = true;
    }
}

/**
 * \brief Processes a "s" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * If there is not enough room in \a packet to hold the static public key,
 * then the error flag will be set in \a packet.
 *
 * \sa read_s()
 */
void NoiseHandshakeState::write_s(NoiseHandshakeState::Packet &packet)
{
    // Check that the application has supplied us with a local static key.
    if (!dhState()->hasParameter(Noise::LocalStaticPublicKey)) {
        packet.error = true;
        return;
    }

    // Write the local static public key to the outgoing message.
    size_t len = dhState()->getParameterSize(Noise::LocalStaticPublicKey);
    if ((packet.size - packet.posn) >= len) {
        dhState()->getParameter
            (Noise::LocalStaticPublicKey, packet.data + packet.posn, len);
        int size = symmetricState()->encryptAndHash
            (packet.data + packet.posn, packet.size - packet.posn,
             packet.data + packet.posn, len);
        if (size > 0)
            packet.posn += size;
        else
            packet.error = true;
    } else {
        packet.error = true;
    }
}

/**
 * \brief Processes an "ee" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * \sa read_ee()
 */
void NoiseHandshakeState::write_ee(NoiseHandshakeState::Packet &packet)
{
    size_t size = dhState()->sharedKeySize();
    uint8_t shared[size];
    dhState()->ee(shared);
    symmetricState()->mixKey(shared, size);
    clean(shared, size);
}

/**
 * \brief Processes an "es" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * \sa read_es()
 */
void NoiseHandshakeState::write_es(NoiseHandshakeState::Packet &packet)
{
    if (!dhState()->hasParameter
            (pty == Noise::Initiator ? Noise::RemoteStaticPublicKey
                                     : Noise::LocalStaticPublicKey)) {
        // We don't have the relevent static key.  It probably should have
        // been provided ahead of time when the handshake started.
        packet.error = true;
        return;
    }
    size_t size = dhState()->sharedKeySize();
    uint8_t shared[size];
    if (pty == Noise::Initiator)
        dhState()->es(shared);
    else
        dhState()->se(shared);
    symmetricState()->mixKey(shared, size);
    clean(shared, size);
}

/**
 * \brief Processes a "se" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * \sa read_se()
 */
void NoiseHandshakeState::write_se(NoiseHandshakeState::Packet &packet)
{
    if (!dhState()->hasParameter
            (pty == Noise::Initiator ? Noise::LocalStaticPublicKey
                                     : Noise::RemoteStaticPublicKey)) {
        // We don't have a relevant static key.  It probably should have
        // been provided ahead of time when the handshake started.
        packet.error = true;
        return;
    }
    size_t size = dhState()->sharedKeySize();
    uint8_t shared[size];
    if (pty == Noise::Initiator)
        dhState()->se(shared);
    else
        dhState()->es(shared);
    symmetricState()->mixKey(shared, size);
    clean(shared, size);
}

/**
 * \brief Processes a "ss" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * \sa read_ss()
 */
void NoiseHandshakeState::write_ss(NoiseHandshakeState::Packet &packet)
{
    if (!dhState()->hasParameter(Noise::LocalStaticPrivateKey) ||
        !dhState()->hasParameter(Noise::RemoteStaticPublicKey)) {
        // We don't have all required static keys.  They probably should
        // have been provided ahead of time when the handshake started.
        packet.error = true;
        return;
    }
    size_t size = dhState()->sharedKeySize();
    uint8_t shared[size];
    dhState()->ss(shared);
    symmetricState()->mixKey(shared, size);
    clean(shared, size);
}

/**
 * \brief Processes an "e" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * If there is not enough data in \a packet to for a valid ephemeral public
 * key, then the error flag will be set in \a packet.
 *
 * \sa write_e()
 */
void NoiseHandshakeState::read_e(NoiseHandshakeState::Packet &packet)
{
    size_t len = dhState()->getParameterSize(Noise::RemoteEphemPublicKey);
    if ((packet.size - packet.posn) >= len) {
        if (dhState()->setParameter
                (Noise::RemoteEphemPublicKey, packet.data + packet.posn, len)) {
            symmetricState()->mixHash(packet.data + packet.posn, len);
        } else {
            packet.error = true;
        }
        packet.posn += len;
    } else {
        packet.error = true;
    }
}

/**
 * \brief Processes a "s" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * If there is not enough data in \a packet to for a valid static public
 * key, or the value failed to decrypt, then the error flag will be set
 * in \a packet.
 *
 * \sa write_s()
 */
void NoiseHandshakeState::read_s(NoiseHandshakeState::Packet &packet)
{
    size_t len = dhState()->getParameterSize(Noise::RemoteStaticPublicKey);
    size_t fullLen = len;
    if (symmetricState()->hasKey())
        fullLen += symmetricState()->macLen();
    if ((packet.size - packet.posn) >= fullLen) {
        uint8_t s[len];
        int size = symmetricState()->decryptAndHash
            (s, len, packet.data + packet.posn, fullLen);
        if (size > 0) {
            packet.posn += fullLen;
            if (!dhState()->setParameter(Noise::RemoteStaticPublicKey, s, len))
                packet.error = true;
        } else {
            packet.error = true;
        }
        clean(s, len);
    } else {
        packet.error = true;
    }
}

/**
 * \fn void NoiseHandshakeState::read_ee(NoiseHandshakeState::Packet &packet)
 * \brief Processes an "ee" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This is a convenience function.  The "ee" token has identical behaviour
 * when both reading and writing.
 *
 * \sa write_ee()
 */

/**
 * \fn void NoiseHandshakeState::read_es(NoiseHandshakeState::Packet &packet)
 * \brief Processes an "es" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This is a convenience function.  The "es" token has identical behaviour
 * when both reading and writing.
 *
 * \sa write_es()
 */

/**
 * \fn void NoiseHandshakeState::read_se(NoiseHandshakeState::Packet &packet)
 * \brief Processes a "se" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This is a convenience function.  The "se" token has identical behaviour
 * when both reading and writing.
 *
 * \sa write_se()
 */

/**
 * \fn void NoiseHandshakeState::read_ss(NoiseHandshakeState::Packet &packet)
 * \brief Processes a "ss" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This is a convenience function.  The "ss" token has identical behaviour
 * when both reading and writing.
 *
 * \sa write_ss()
 */

/**
 * \brief Hashes a public key from the pre-message.
 *
 * \param packet The handshake packet that is being processed.
 * \param initiator The public key to hash if party() is Noise::Initiator.
 * \param responder The public key to hash if party() is Noise::Responder.
 *
 * The handshake will fail if the required key is not present.
 *
 * This function should be called within the subclass's writeTokens() and
 * readTokens() implementations when message number 0 is processed.
 */
void NoiseHandshakeState::premessage
    (NoiseHandshakeState::Packet &packet,
     Noise::Parameter initiator, Noise::Parameter responder)
{
    if (pty == Noise::Responder)
        initiator = responder;
    if (!dhState()->hashPublicKey(symmetricState(), initiator))
        packet.error = true;
}

/**
 * \fn void NoiseHandshakeState::setState(Noise::HandshakeState state)
 * \brief Sets the state of this handshake.
 *
 * \param state The new state.
 *
 * This is intended for use in subclass implementations of start() when
 * initializing the handshake.
 *
 * \sa state()
 */

/**
 * \brief Writes a payload to a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 * \param data Points to the plaintext payload data to be written.
 * \param size Number of bytes of plaintext payload to write.
 *
 * If there is not enough room in \a packet to hold the encrypted payload,
 * then the error flag will be set in \a packet.
 *
 * \sa readPayload()
 */
void NoiseHandshakeState::writePayload(NoiseHandshakeState::Packet &packet,
                                       const void *data, size_t size)
{
    int result = symmetricState()->encryptAndHash
        (packet.data + packet.posn, packet.size - packet.posn,
         (const uint8_t *)data, size);
    if (result >= 0)
        packet.posn += result;
    else
        packet.error = true;
}

/**
 * \brief Reads a payload from a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 * \param data Points to the buffer to place the plaintext payload data into.
 * \param maxSize Maximum number of bytes that can be written to \a data.
 *
 * \return Number of plaintext bytes that were written to the \a data buffer.
 *
 * If there is a problem decrypting the payload, or \a maxSize is not large
 * enough, then 0 will be returned and the error flag will be set in \a packet.
 *
 * \sa writePayload()
 */
size_t NoiseHandshakeState::readPayload(NoiseHandshakeState::Packet &packet,
                                        void *data, size_t maxSize)
{
    int result = symmetricState()->decryptAndHash
        ((uint8_t *)data, maxSize,
         packet.data + packet.posn, packet.size - packet.posn);
    packet.posn = packet.size;
    if (result >= 0)
        return result;
    packet.error = true;
    return 0;
}

/**
 * \brief Constructs a new PSK-using handshake.
 */
NoiseHandshakeStatePSK::NoiseHandshakeStatePSK()
    : havePSK(false)
{
}

/**
 * \brief Destroys this PSK-using handshake.
 */
NoiseHandshakeStatePSK::~NoiseHandshakeStatePSK()
{
    clean(psk);
}

bool NoiseHandshakeStatePSK::setParameter
    (Noise::Parameter id, const void *value, size_t size)
{
    if (id == Noise::PreSharedKey) {
        if (!value || size != 32)
            return false;
        memcpy(psk, value, size);
        havePSK = true;
        return true;
    } else {
        return NoiseHandshakeState::setParameter(id, value, size);
    }
}

size_t NoiseHandshakeStatePSK::getParameter
    (Noise::Parameter id, void *value, size_t maxSize) const
{
    if (id == Noise::PreSharedKey) {
        if (!havePSK || !value || maxSize < 32)
            return 0;
        memcpy(value, psk, 32);
        return 32;
    } else {
        return NoiseHandshakeState::getParameter(id, value, maxSize);
    }
}

size_t NoiseHandshakeStatePSK::getParameterSize(Noise::Parameter id) const
{
    if (id == Noise::PreSharedKey)
        return 32;
    else
        return NoiseHandshakeState::getParameterSize(id);
}

bool NoiseHandshakeStatePSK::hasParameter(Noise::Parameter id) const
{
    if (id == Noise::PreSharedKey)
        return havePSK;
    else
        return NoiseHandshakeState::hasParameter(id);
}

void NoiseHandshakeStatePSK::removeParameter(Noise::Parameter id)
{
    if (id == Noise::PreSharedKey)
        havePSK = false;
    else
        NoiseHandshakeState::removeParameter(id);
}

void NoiseHandshakeStatePSK::clear()
{
    clean(psk);
    havePSK = false;
    NoiseHandshakeState::clear();
}

/**
 * \brief Processes an "e" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * This overrides the base class implementation of write_e() to provide
 * additional behaviour for handshakes that involve PSK values.
 */
void NoiseHandshakeStatePSK::write_e(NoiseHandshakeState::Packet &packet)
{
    NoiseHandshakeState::write_e(packet);
    if (!packet.error) {
        size_t len = dhState()->getParameterSize(Noise::LocalEphemPublicKey);
        symmetricState()->mixKey(packet.data + packet.posn - len, len);
    }
}

/**
 * \brief Processes an "e" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This overrides the base class implementation of read_e() to provide
 * additional behaviour for handshakes that involve PSK values.
 */
void NoiseHandshakeStatePSK::read_e(NoiseHandshakeState::Packet &packet)
{
    NoiseHandshakeState::read_e(packet);
    if (!packet.error) {
        size_t len = dhState()->getParameterSize(Noise::RemoteEphemPublicKey);
        symmetricState()->mixKey(packet.data + packet.posn - len, len);
    }
}

/**
 * \brief Processes a "psk" token when writing a handshake packet.
 *
 * \param packet The handshake packet that is being written.
 *
 * \sa read_psk()
 */
void NoiseHandshakeStatePSK::write_psk(NoiseHandshakeState::Packet &packet)
{
    if (havePSK)
        symmetricState()->mixKeyAndHash(psk, 32);
    else
        packet.error = true;
}

/**
 * \fn void NoiseHandshakeStatePSK::read_psk(NoiseHandshakeState::Packet &packet)
 * \brief Processes a "psk" token when reading a handshake packet.
 *
 * \param packet The handshake packet that is being read.
 *
 * This is a convenience function.  The "psk" token has identical behaviour
 * when both reading and writing.
 *
 * \sa write_psk()
 */
