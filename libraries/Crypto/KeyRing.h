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

#ifndef KEY_RING_h
#define KEY_RING_h

#include <inttypes.h>
#include <stddef.h>

class KeyRingClass
{
public:
    KeyRingClass();
    ~KeyRingClass();

    void begin();
    void begin(const void *key, size_t size);
    void begin(const char *passphrase);
    void end();

    bool setLocalKeyPair(uint16_t id, const void *pair, size_t size);
    size_t getLocalKeyPair(uint16_t id, void *pair, size_t maxSize);
    size_t getLocalKeyPairSize(uint16_t id);
    void removeLocalKeyPair(uint16_t id);

    bool setRemotePublicKey(uint16_t id, const void *key, size_t size);
    size_t getRemotePublicKey(uint16_t id, void *key, size_t maxSize);
    size_t getRemotePublicKeySize(uint16_t id);
    void removeRemotePublicKey(uint16_t id);

    bool setSharedSymmetricKey(uint16_t id, const void *key, size_t size);
    size_t getSharedSymmetricKey(uint16_t id, void *key, size_t maxSize);
    size_t getSharedSymmetricKeySize(uint16_t id);
    void removeSharedSymmetricKey(uint16_t id);

    bool setOtherData(uint16_t id, const void *data, size_t size);
    size_t getOtherData(uint16_t id, void *data, size_t maxSize);
    size_t getOtherDataSize(uint16_t id);
    void removeOtherData(uint16_t id);

    void removeAll();

    static const uint16_t LocalCurve25519Default = 0x4301;  // 'C', 0x01
    static const uint16_t RemoteCurve25519Default = 0x6301; // 'c', 0x01

    static const uint16_t LocalEd25519Default = 0x4501;     // 'E', 0x01
    static const uint16_t RemoteEd25519Default = 0x6501;    // 'e', 0x01

    static const uint16_t EthernetMACAddress = 0x4D01;      // 'M', 0x01

private:
    static const size_t ChunkSize = 36;

    uint8_t k[16];

    void (*crypt)(uint8_t k[16], uint8_t chunk[ChunkSize],
                  unsigned posn, bool encrypt);

    bool set(uint16_t id, uint8_t type, const void *data, size_t size);
    size_t get(uint16_t id, uint8_t type, void *data, size_t maxSize);
    size_t getSize(uint16_t id, uint8_t type);
    void remove(uint16_t id, uint8_t type);

    void readStart(unsigned &posn);
    bool readChunk(unsigned &posn, uint8_t chunk[ChunkSize], unsigned &actual);
    bool readChunk
        (unsigned &posn, uint8_t chunk[ChunkSize], uint16_t id,
         uint8_t type, bool setMode, bool decrypt);

    bool writeChunk(unsigned posn, uint8_t chunk[ChunkSize]);
    bool writeExtraChunk(unsigned& posn, uint8_t chunk[ChunkSize]);
    bool eraseChunk(unsigned &posn);

    void readWriteEnd();
};

extern KeyRingClass KeyRing;

inline bool KeyRingClass::setLocalKeyPair
    (uint16_t id, const void *pair, size_t size)
{
    return set(id, 0, pair, size);
}

inline size_t KeyRingClass::getLocalKeyPair
    (uint16_t id, void *pair, size_t maxSize)
{
    return get(id, 0, pair, maxSize);
}

inline size_t KeyRingClass::getLocalKeyPairSize(uint16_t id)
{
    return getSize(id, 0);
}

inline void KeyRingClass::removeLocalKeyPair(uint16_t id)
{
    remove(id, 0);
}

inline bool KeyRingClass::setRemotePublicKey
    (uint16_t id, const void *key, size_t size)
{
    return set(id, 1, key, size);
}

inline size_t KeyRingClass::getRemotePublicKey
    (uint16_t id, void *key, size_t maxSize)
{
    return get(id, 1, key, maxSize);
}

inline size_t KeyRingClass::getRemotePublicKeySize(uint16_t id)
{
    return getSize(id, 1);
}

inline void KeyRingClass::removeRemotePublicKey(uint16_t id)
{
    remove(id, 1);
}

inline bool KeyRingClass::setSharedSymmetricKey
    (uint16_t id, const void *key, size_t size)
{
    return set(id, 2, key, size);
}

inline size_t KeyRingClass::getSharedSymmetricKey
    (uint16_t id, void *key, size_t maxSize)
{
    return get(id, 2, key, maxSize);
}

inline size_t KeyRingClass::getSharedSymmetricKeySize(uint16_t id)
{
    return getSize(id, 2);
}

inline void KeyRingClass::removeSharedSymmetricKey(uint16_t id)
{
    remove(id, 2);
}

inline bool KeyRingClass::setOtherData
    (uint16_t id, const void *data, size_t size)
{
    return set(id, 3, data, size);
}

inline size_t KeyRingClass::getOtherData
    (uint16_t id, void *data, size_t maxSize)
{
    return get(id, 3, data, maxSize);
}

inline size_t KeyRingClass::getOtherDataSize(uint16_t id)
{
    return getSize(id, 3);
}

inline void KeyRingClass::removeOtherData(uint16_t id)
{
    remove(id, 3);
}

#endif
