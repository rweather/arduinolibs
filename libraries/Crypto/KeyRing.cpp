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

#include "KeyRing.h"

/**
 * \class KeyRing KeyRing.h <KeyRing.h>
 * \brief Permanent storage for key material.
 *
 * This class provides permanent storage for local key pairs, remote public
 * keys, shared symmetric keys, and other data on a device.  Other data may
 * be certificates, configuration data, or any other information that
 * isn't strictly key material.
 *
 * As an example, the following code will generate a new Curve25519
 * key pair and save it into permanent storage as the default key pair
 * for the device:
 *
 * \code
 * uint8_t keyPair[64];
 * Curve25519::generateKeyPair(keyPair);
 * KeyRing::setLocalKeyPair(KeyRing::LocalCurve25519Default, keyPair, 64);
 * \endcode
 *
 * Later, when the application needs the key pair it does the following:
 *
 * \code
 * uint8_t keyPair[64];
 * if (!KeyRing::getLocalKeyPair(KeyRing::LocalCurve25519Default, keyPair, 64)) {
 *     Serial.println("Curve25519 key pair not found!");
 * } else {
 *     // Make use of the key pair.
 *     ...
 * }
 * \endcode
 *
 * The key ring data is not encrypted in the permanent storage, so the
 * data may be compromised if the device is captured.  The application can
 * arrange to encrypt the values separately before calling the set functions
 * in this class.
 *
 * The data is not encrypted by default because it creates a chicken-and-egg
 * problem: where is the encryption key itself stored if the device does
 * not have some way to enter a passphrase at runtime?  We therefore leave
 * the issue of key ring encryption up to the application.
 *
 * The amount of key data that can be stored is limited by the size of
 * the permanent storage mechanism on the device:
 *
 * \li AVR devices will use no more than 1/2 of the available EEPROM
 * space, starting at the end of EEPROM and working backwards.
 * \li Arduino Due and ESP8266 allocate up to 8K of flash memory for key data.
 * \li ESP32 storage is built on top of that chip's Non-Volatile Storage (NVS)
 * mechanism, and is limited only by the maximum NVS size.
 *
 * If a device is not currently supported, then requests to store values
 * in the key ring will fail.  Requests to retrieve values from the key
 * ring will return nothing.
 */

/**
 * \var KeyRing::LocalCurve25519Default
 * \brief Identifier for the default local Curve25519 key pair on the device.
 */

/**
 * \var KeyRing::RemoteCurve25519Default
 * \brief Identifier for the default remote Curve25519 public key for the
 * primary remote device that this device will be communicating with.
 */

/**
 * \var KeyRing::LocalEd25519Default
 * \brief Identifier for the default local Ed25519 key pair on the device.
 */

/**
 * \var KeyRing::RemoteEd25519Default
 * \brief Identifier for the default remote Ed25519 public key for the
 * primary remote device that this device will be communicating with.
 */

/**
 * \fn bool KeyRing::setLocalKeyPair(uint16_t id, const void *pair, size_t size)
 * \brief Sets a local key pair in permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.
 * \param pair Points to the key pair value, which is assumed to be the
 * private key followed by the public key.
 * \param size Size of the key pair, including both the private and public
 * components.
 *
 * \return Returns true if the key pair was stored, or false if there is
 * insufficient space to store the key pair, or this platform does not have
 * permanent storage available.
 *
 * \sa getLocalKeyPair()
 */

/**
 * \fn size_t KeyRing::getLocalKeyPair(uint16_t id, void *pair, size_t maxSize)
 * \brief Gets a local key pair from permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.
 * \param pair Points to the buffer to retrieve the key pair value, which
 * is assumed to consist of the private key followed by the public key.
 * \param maxSize Maximum size of the \a pair buffer which may be larger
 * than the actual key pair size, to support the retrieval of variable-length
 * key pairs.
 *
 * \return The actual number of bytes in the key pair, or zero if the key
 * pair was not found.
 *
 * The companion function getLocalKeyPairSize() can be used to query the
 * size of the key pair before it is retrieved.
 *
 * \sa getLocalKeyPairSize(), setLocalKeyPair()
 */

/**
 * \fn size_t KeyRing::getLocalKeyPairSize(uint16_t id)
 * \brief Gets the size of a local key pair in permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.
 *
 * \return The number of bytes of data that are stored for the key pair,
 * or zero if there is no key pair currently associated with \a id.
 *
 * \sa getLocalKeyPair()
 */

/**
 * \fn void KeyRing::removeLocalKeyPair(uint16_t id)
 * \brief Removes a local key pair from permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.
 *
 * The key pair value will be overwritten with 0xFF bytes to erase it.
 * However, this may not be sufficient to completely remove all trace of
 * the key pair from flash memory or EEPROM.  If the underlying storage
 * mechanism is performing wear-levelling, then it may leave older copies
 * of the value in unused pages when new values are written.
 */

/**
 * \fn bool KeyRing::setRemotePublicKey(uint16_t id, const void *key, size_t size)
 * \brief Sets a remote public key in permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.
 * \param key Points to the public key value.
 * \param size Size of the public key value in bytes.
 *
 * \return Returns true if the public key was stored, or false if there is
 * insufficient space to store the public key, or this platform does not have
 * permanent storage available.
 *
 * \sa getRemotePublicKey()
 */

/**
 * \fn size_t KeyRing::getRemotePublicKey(uint16_t id, void *key, size_t maxSize)
 * \brief Gets a remote public key from permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.
 * \param key Points to the buffer to retrieve the public key value.
 * \param maxSize Maximum size of the \a key buffer which may be larger
 * than the actual public key size, to support the retrieval of
 * variable-length public keys.
 *
 * \return The actual number of bytes in the public key, or zero if the
 * public key was not found.
 *
 * The companion function getRemotePublicKeySize() can be used to query the
 * size of the public key before it is retrieved.
 *
 * \sa getRemotePublicKeySize(), setRemotePublicKey()
 */

/**
 * \fn size_t KeyRing::getRemotePublicKeySize(uint16_t id)
 * \brief Gets the size of a remote public key in permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.
 *
 * \return The number of bytes of data that are stored for the public key,
 * or zero if there is no public key currently associated with \a id.
 *
 * \sa getRemotePublicKey()
 */

/**
 * \fn void KeyRing::removeRemotePublicKey(uint16_t id)
 * \brief Removes a remote public key from permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.
 */

/**
 * \fn bool KeyRing::setSharedSymmetricKey(uint16_t id, const void *key, size_t size)
 * \brief Sets a shared symmetric key in permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.
 * \param key Points to the shared symmetric key value.
 * \param size Size of the shared symmetric key value in bytes.
 *
 * \return Returns true if the shared symmetric key was stored, or false if
 * there is insufficient space to store the key, or this platform does not
 * have permanent storage available.
 *
 * \sa getSharedSymmetricKey()
 */

/**
 * \fn size_t KeyRing::getSharedSymmetricKey(uint16_t id, void *key, size_t maxSize)
 * \brief Gets a shared symmetric key from permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.
 * \param key Points to the buffer to retrieve the shared symmetric key value.
 * \param maxSize Maximum size of the \a key buffer which may be larger
 * than the actual shared symmetric key size, to support the retrieval of
 * variable-length keys.
 *
 * \return The actual number of bytes in the shared symmetric key,
 * or zero if the key was not found.
 *
 * The companion function getSharedSymmetricKeySize() can be used to
 * query the size of the shared symmetric key before it is retrieved.
 *
 * \sa getSharedSymmetricKeySize(), setSharedSymmetricKey()
 */

/**
 * \fn size_t KeyRing::getSharedSymmetricKeySize(uint16_t id)
 * \brief Gets the size of a shared symmetric key in permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.
 *
 * \return The number of bytes of data that are stored for the shared
 * symmetric key, or zero if there is no key currently associated with \a id.
 *
 * \sa getSharedSymmetricKey()
 */

/**
 * \fn void KeyRing::removeSharedSymmetricKey(uint16_t id)
 * \brief Removes a shared symmetric key from permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.
 *
 * The symmetric key value will be overwritten with 0xFF bytes to erase it.
 * However, this may not be sufficient to completely remove all trace of
 * the key from flash memory or EEPROM.  If the underlying storage mechanism
 * is performing wear-levelling, then it may leave older copies of the value
 * in unused pages when new values are written.
 */

/**
 * \fn bool KeyRing::setOtherData(uint16_t id, const void *data, size_t size);
 * \brief Sets an arbitrary data value in permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.
 * \param data Points to the data value to store.
 * \param size Size of the data value in bytes.
 *
 * \return Returns true if the data value was stored, or false if there is
 * insufficient space to store the value, or this platform does not have
 * permanent storage available.
 *
 * \sa getOtherData()
 */

/**
 * \fn size_t KeyRing::getOtherData(uint16_t id, void *data, size_t maxSize)
 * \brief Gets an arbitrary data value from permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.
 * \param data Points to the buffer to retrieve the data value.
 * \param maxSize Maximum size of the \a data buffer which may be larger
 * than the actual data value size, to support the retrieval of
 * variable-length values.
 *
 * \return The actual number of bytes in the data value, or zero if the value
 * was not found.
 *
 * The companion function getOtherDataSize() can be used to query the size of
 * the data value before it is retrieved.
 *
 * \sa getOtherDataSize(), setOtherData()
 */

/**
 * \fn size_t KeyRing::getOtherDataSize(uint16_t id)
 * \brief Gets the size of an arbitrary data value in permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.
 *
 * \return The number of bytes of data that are stored for the value,
 * or zero if there is no value currently associated with \a id.
 *
 * \sa getOtherData()
 */

/**
 * \fn void KeyRing::removeOtherData(uint16_t id)
 * \brief Removes an arbitrary data value from permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.
 */

bool KeyRing::set(uint16_t id, uint8_t type, const void *data, size_t size)
{
    // TODO
    return false;
}

size_t KeyRing::get(uint16_t id, uint8_t type, void *data, size_t maxSize)
{
    // TODO
    return 0;
}

size_t KeyRing::getSize(uint16_t id, uint8_t type)
{
    // TODO
    return 0;
}

void KeyRing::remove(uint16_t id, uint8_t type)
{
    // TODO
}
