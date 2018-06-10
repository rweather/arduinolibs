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

class KeyRing
{
private:
    KeyRing() {}
    ~KeyRing() {}

public:
    static bool setLocalKeyPair
        (uint16_t id, const void *pair, size_t size);
    static size_t getLocalKeyPair
        (uint16_t id, void *pair, size_t maxSize);
    static size_t getLocalKeyPairSize(uint16_t id);
    static void removeLocalKeyPair(uint16_t id);

    static bool setRemotePublicKey
        (uint16_t id, const void *key, size_t size);
    static size_t getRemotePublicKey
        (uint16_t id, void *key, size_t maxSize);
    static size_t getRemotePublicKeySize(uint16_t id);
    static void removeRemotePublicKey(uint16_t id);

    static bool setSharedSymmetricKey
        (uint16_t id, const void *key, size_t size);
    static size_t getSharedSymmetricKey
        (uint16_t id, void *key, size_t maxSize);
    static size_t getSharedSymmetricKeySize(uint16_t id);
    static void removeSharedSymmetricKey(uint16_t id);

    static bool setOtherData
        (uint16_t id, const void *data, size_t size);
    static size_t getOtherData
        (uint16_t id, void *data, size_t maxSize);
    static size_t getOtherDataSize(uint16_t id);
    static void removeOtherData(uint16_t id);

    static const uint16_t LocalCurve25519Default = 0x4301;  // 'C', 0x01
    static const uint16_t RemoteCurve25519Default = 0x6301; // 'c', 0x01

    static const uint16_t LocalEd25519Default = 0x4501;     // 'E', 0x01
    static const uint16_t RemoteEd25519Default = 0x6501;    // 'e', 0x01

private:
    static bool set(uint16_t id, uint8_t type, const void *data, size_t size);
    static size_t get(uint16_t id, uint8_t type, void *data, size_t maxSize);
    static size_t getSize(uint16_t id, uint8_t type);
    static void remove(uint16_t id, uint8_t type);
};

inline bool KeyRing::setLocalKeyPair
    (uint16_t id, const void *pair, size_t size)
{
    return set(id, 0, pair, size);
}

inline size_t KeyRing::getLocalKeyPair
    (uint16_t id, void *pair, size_t maxSize)
{
    return get(id, 0, pair, maxSize);
}

inline size_t KeyRing::getLocalKeyPairSize(uint16_t id)
{
    return getSize(id, 0);
}

inline void KeyRing::removeLocalKeyPair(uint16_t id)
{
    remove(id, 0);
}

inline bool KeyRing::setRemotePublicKey
    (uint16_t id, const void *key, size_t size)
{
    return set(id, 1, key, size);
}

inline size_t KeyRing::getRemotePublicKey
    (uint16_t id, void *key, size_t maxSize)
{
    return get(id, 1, key, maxSize);
}

inline size_t KeyRing::getRemotePublicKeySize(uint16_t id)
{
    return getSize(id, 1);
}

inline void KeyRing::removeRemotePublicKey(uint16_t id)
{
    remove(id, 1);
}

inline bool KeyRing::setSharedSymmetricKey
    (uint16_t id, const void *key, size_t size)
{
    return set(id, 2, key, size);
}

inline size_t KeyRing::getSharedSymmetricKey
    (uint16_t id, void *key, size_t maxSize)
{
    return get(id, 2, key, maxSize);
}

inline size_t KeyRing::getSharedSymmetricKeySize(uint16_t id)
{
    return getSize(id, 2);
}

inline void KeyRing::removeSharedSymmetricKey(uint16_t id)
{
    remove(id, 2);
}

inline bool KeyRing::setOtherData
    (uint16_t id, const void *data, size_t size)
{
    return set(id, 3, data, size);
}

inline size_t KeyRing::getOtherData
    (uint16_t id, void *data, size_t maxSize)
{
    return get(id, 3, data, maxSize);
}

inline size_t KeyRing::getOtherDataSize(uint16_t id)
{
    return getSize(id, 3);
}

inline void KeyRing::removeOtherData(uint16_t id)
{
    remove(id, 3);
}

#endif
