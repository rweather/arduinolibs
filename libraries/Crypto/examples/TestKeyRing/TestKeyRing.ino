/*
 * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
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

/*
This example runs tests on the KeyRing class.
*/

#include <Crypto.h>
#include <KeyRing.h>
#include <string.h>

uint8_t const testKey1[32] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20
};
uint8_t const testKey2[64] = {
    0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80
};

uint8_t buffer[128];

void verifyKey(const char *name, uint16_t id, const void *expected, size_t len,
               const void *actual, size_t size, size_t size2)
{
    if (expected) {
        if (size == len) {
            if (memcmp(actual, expected, len) != 0) {
                Serial.print(name);
                Serial.println(" failed: wrong value returned");
            }
        } else {
            Serial.print(name);
            Serial.println(" failed: wrong size returned");
        }
    } else {
        if (size != 0) {
            Serial.print(name);
            Serial.println(" failed: value returned when none expected");
        }
    }
    if (size2 != len) {
        Serial.print(name);
        Serial.println(" failed: incorrect size report");
    }
}

#define verifyLocalKeyPair(name,id,expected,len) \
    do { \
        memset(buffer, 0, sizeof(buffer)); \
        size_t size = KeyRing.getLocalKeyPair(id, buffer, sizeof(buffer)); \
        size_t size2 = KeyRing.getLocalKeyPairSize(id); \
        verifyKey((name), (id), (expected), (len), buffer, size, size2); \
    } while (0)

#define verifyRemotePublicKey(name,id,expected,len) \
    do { \
        memset(buffer, 0, sizeof(buffer)); \
        size_t size = KeyRing.getRemotePublicKey(id, buffer, sizeof(buffer)); \
        size_t size2 = KeyRing.getRemotePublicKeySize(id); \
        verifyKey((name), (id), (expected), (len), buffer, size, size2); \
    } while (0)

#define verifySharedSymmetricKey(name,id,expected,len) \
    do { \
        memset(buffer, 0, sizeof(buffer)); \
        size_t size = KeyRing.getSharedSymmetricKey(id, buffer, sizeof(buffer)); \
        size_t size2 = KeyRing.getSharedSymmetricKeySize(id); \
        verifyKey((name), (id), (expected), (len), buffer, size, size2); \
    } while (0)

#define verifyOtherData(name,id,expected,len) \
    do { \
        memset(buffer, 0, sizeof(buffer)); \
        size_t size = KeyRing.getOtherData(id, buffer, sizeof(buffer)); \
        size_t size2 = KeyRing.getOtherDataSize(id); \
        verifyKey((name), (id), (expected), (len), buffer, size, size2); \
    } while (0)

void runAllTests()
{
    Serial.println("Writing keys ...");

    KeyRing.setLocalKeyPair(1, testKey1, 32);
    KeyRing.setLocalKeyPair(2, testKey2, 64);

    KeyRing.setRemotePublicKey(2, testKey2, 48);
    KeyRing.setRemotePublicKey(0xABCD, testKey1, 27);

    KeyRing.setSharedSymmetricKey(0xFFFF, testKey1, sizeof(testKey1));
    KeyRing.setSharedSymmetricKey(3, testKey2, 1);

    KeyRing.setOtherData(1, testKey1, 15);
    KeyRing.setOtherData(2, testKey2, 33);

    Serial.println("Verifying keys ...");

    verifyLocalKeyPair("local #1", 1, testKey1, 32);
    verifyLocalKeyPair("local #2", 2, testKey2, 64);
    verifyLocalKeyPair("local #3", 0xABCD, 0, 0);

    verifyRemotePublicKey("remote #1", 2, testKey2, 48);
    verifyRemotePublicKey("remote #2", 0xABCD, testKey1, 27);
    verifyRemotePublicKey("remote #3", 1, 0, 0);

    verifySharedSymmetricKey("shared #1", 0xFFFF, testKey1, sizeof(testKey1));
    verifySharedSymmetricKey("shared #2", 3, testKey2, 1);
    verifySharedSymmetricKey("shared #3", 0xABCD, 0, 0);

    verifyOtherData("other #1", 1, testKey1, 15);
    verifyOtherData("other #2", 2, testKey2, 33);

    Serial.println("Removing some keys ...");

    KeyRing.removeLocalKeyPair(1);
    KeyRing.removeRemotePublicKey(2);
    KeyRing.removeSharedSymmetricKey(2); // Not present, so not removed.
    KeyRing.removeOtherData(1);

    Serial.println("Verifying remaining keys ...");

    verifyLocalKeyPair("local #1", 1, 0, 0);
    verifyLocalKeyPair("local #2", 2, testKey2, 64);
    verifyLocalKeyPair("local #3", 0xABCD, 0, 0);

    verifyRemotePublicKey("remote #1", 2, 0, 0);
    verifyRemotePublicKey("remote #2", 0xABCD, testKey1, 27);
    verifyRemotePublicKey("remote #3", 1, 0, 0);

    verifySharedSymmetricKey("shared #1", 0xFFFF, testKey1, sizeof(testKey1));
    verifySharedSymmetricKey("shared #2", 3, testKey2, 1);
    verifySharedSymmetricKey("shared #3", 0xABCD, 0, 0);

    verifyOtherData("other #1", 1, 0, 0);
    verifyOtherData("other #2", 2, testKey2, 33);

    Serial.println("Removing all keys ...");

    KeyRing.removeAll();

    Serial.println("Verifying that no keys remain ...");

    verifyLocalKeyPair("local #1", 1, 0, 0);
    verifyLocalKeyPair("local #2", 2, 0, 0);
    verifyLocalKeyPair("local #3", 0xABCD, 0, 0);

    verifyRemotePublicKey("remote #1", 2, 0, 0);
    verifyRemotePublicKey("remote #2", 0xABCD, 0, 0);
    verifyRemotePublicKey("remote #3", 1, 0, 0);

    verifySharedSymmetricKey("shared #1", 0xFFFF, 0, 0);
    verifySharedSymmetricKey("shared #2", 3, 0, 0);
    verifySharedSymmetricKey("shared #3", 0xABCD, 0, 0);

    verifyOtherData("other #1", 1, 0, 0);
    verifyOtherData("other #2", 2, 0, 0);
}

void setup()
{
    Serial.begin(9600);
    Serial.println();

    Serial.println("No passphrase:");
    KeyRing.begin();
    runAllTests();
    Serial.println();

    Serial.println("With passphrase:");
    KeyRing.begin("Hello World!");
    runAllTests();
    Serial.println();

    Serial.println("Done");
}

void loop()
{
}
