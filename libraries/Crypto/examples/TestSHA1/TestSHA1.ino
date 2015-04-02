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
This example runs tests on the SHA1 implementation to verify correct behaviour.
*/

#include <Crypto.h>
#include <SHA1.h>
#include <string.h>

#define HASH_SIZE 20
#define BLOCK_SIZE 64

struct TestHashVector
{
    const char *name;
    const char *key;
    const char *data;
    uint8_t hash[HASH_SIZE];
};

static TestHashVector const testVectorSHA1_1 = {
    "SHA-1 #1",
    0,
    "abc",
    {0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A,
     0xBA, 0x3E, 0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C,
     0x9C, 0xD0, 0xD8, 0x9D}
};
static TestHashVector const testVectorSHA1_2 = {
    "SHA-1 #2",
    0,
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    {0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E,
     0xBA, 0xAE, 0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5,
     0xE5, 0x46, 0x70, 0xF1}
};
static TestHashVector const testVectorHMAC_SHA1_1 = {
    "HMAC-SHA-1 #1",
    "",
    "",
    {0xfb, 0xdb, 0x1d, 0x1b, 0x18, 0xaa, 0x6c, 0x08,
     0x32, 0x4b, 0x7d, 0x64, 0xb7, 0x1f, 0xb7, 0x63,
     0x70, 0x69, 0x0e, 0x1d}
};
static TestHashVector const testVectorHMAC_SHA1_2 = {
    "HMAC-SHA-1 #2",
    "key",
    "The quick brown fox jumps over the lazy dog",
    {0xde, 0x7c, 0x9b, 0x85, 0xb8, 0xb7, 0x8a, 0xa6,
     0xbc, 0x8a, 0x7a, 0x36, 0xf7, 0x0a, 0x90, 0x70,
     0x1c, 0x9d, 0xb4, 0xd9}
};

SHA1 sha1;

byte buffer[128];

bool testHash_N(Hash *hash, const struct TestHashVector *test, size_t inc)
{
    size_t size = strlen(test->data);
    size_t posn, len;
    uint8_t value[HASH_SIZE];

    hash->reset();
    for (posn = 0; posn < size; posn += inc) {
        len = size - posn;
        if (len > inc)
            len = inc;
        hash->update(test->data + posn, len);
    }
    hash->finalize(value, sizeof(value));
    if (memcmp(value, test->hash, sizeof(value)) != 0)
        return false;

    return true;
}

void testHash(Hash *hash, const struct TestHashVector *test)
{
    bool ok;

    Serial.print(test->name);
    Serial.print(" ... ");

    ok  = testHash_N(hash, test, strlen(test->data));
    ok &= testHash_N(hash, test, 1);
    ok &= testHash_N(hash, test, 2);
    ok &= testHash_N(hash, test, 5);
    ok &= testHash_N(hash, test, 8);
    ok &= testHash_N(hash, test, 13);
    ok &= testHash_N(hash, test, 16);
    ok &= testHash_N(hash, test, 24);
    ok &= testHash_N(hash, test, 63);
    ok &= testHash_N(hash, test, 64);

    if (ok)
        Serial.println("Passed");
    else
        Serial.println("Failed");
}

// Very simple method for hashing a HMAC inner or outer key.
void hashKey(Hash *hash, const uint8_t *key, size_t keyLen, uint8_t pad)
{
    size_t posn;
    uint8_t buf;
    uint8_t result[HASH_SIZE];
    if (keyLen <= BLOCK_SIZE) {
        hash->reset();
        for (posn = 0; posn < BLOCK_SIZE; ++posn) {
            if (posn < keyLen)
                buf = key[posn] ^ pad;
            else
                buf = pad;
            hash->update(&buf, 1);
        }
    } else {
        hash->reset();
        hash->update(key, keyLen);
        hash->finalize(result, HASH_SIZE);
        hash->reset();
        for (posn = 0; posn < BLOCK_SIZE; ++posn) {
            if (posn < HASH_SIZE)
                buf = result[posn] ^ pad;
            else
                buf = pad;
            hash->update(&buf, 1);
        }
    }
}

void testHMAC(Hash *hash, size_t keyLen)
{
    uint8_t result[HASH_SIZE];

    Serial.print("HMAC-SHA-1 keysize=");
    Serial.print(keyLen);
    Serial.print(" ... ");

    // Construct the expected result with a simple HMAC implementation.
    memset(buffer, (uint8_t)keyLen, keyLen);
    hashKey(hash, buffer, keyLen, 0x36);
    memset(buffer, 0xBA, sizeof(buffer));
    hash->update(buffer, sizeof(buffer));
    hash->finalize(result, HASH_SIZE);
    memset(buffer, (uint8_t)keyLen, keyLen);
    hashKey(hash, buffer, keyLen, 0x5C);
    hash->update(result, HASH_SIZE);
    hash->finalize(result, HASH_SIZE);

    // Now use the library to compute the HMAC.
    hash->resetHMAC(buffer, keyLen);
    memset(buffer, 0xBA, sizeof(buffer));
    hash->update(buffer, sizeof(buffer));
    memset(buffer, (uint8_t)keyLen, keyLen);
    hash->finalizeHMAC(buffer, keyLen, buffer, HASH_SIZE);

    // Check the result.
    if (!memcmp(result, buffer, HASH_SIZE))
        Serial.println("Passed");
    else
        Serial.println("Failed");
}

void testHMAC(Hash *hash, const struct TestHashVector *test)
{
    uint8_t result[HASH_SIZE];

    Serial.print(test->name);
    Serial.print(" ... ");

    hash->resetHMAC(test->key, strlen(test->key));
    hash->update(test->data, strlen(test->data));
    hash->finalizeHMAC(test->key, strlen(test->key), result, sizeof(result));

    if (!memcmp(result, test->hash, HASH_SIZE))
        Serial.println("Passed");
    else
        Serial.println("Failed");
}

void perfHash(Hash *hash)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    Serial.print("Hashing ... ");

    for (size_t posn = 0; posn < sizeof(buffer); ++posn)
        buffer[posn] = (uint8_t)posn;

    hash->reset();
    start = micros();
    for (count = 0; count < 1000; ++count) {
        hash->update(buffer, sizeof(buffer));
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (sizeof(buffer) * 1000.0));
    Serial.print("us per byte, ");
    Serial.print((sizeof(buffer) * 1000.0 * 1000000.0) / elapsed);
    Serial.println(" bytes per second");
}

void perfFinalize(Hash *hash)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    Serial.print("Finalizing ... ");

    hash->reset();
    hash->update("abc", 3);
    start = micros();
    for (count = 0; count < 1000; ++count) {
        hash->finalize(buffer, hash->hashSize());
    }
    elapsed = micros() - start;

    Serial.print(elapsed / 1000.0);
    Serial.print("us per op, ");
    Serial.print((1000.0 * 1000000.0) / elapsed);
    Serial.println(" ops per second");
}

void perfHMAC(Hash *hash)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    Serial.print("HMAC Reset ... ");

    for (size_t posn = 0; posn < sizeof(buffer); ++posn)
        buffer[posn] = (uint8_t)posn;

    start = micros();
    for (count = 0; count < 1000; ++count) {
        hash->resetHMAC(buffer, hash->hashSize());
    }
    elapsed = micros() - start;

    Serial.print(elapsed / 1000.0);
    Serial.print("us per op, ");
    Serial.print((1000.0 * 1000000.0) / elapsed);
    Serial.println(" ops per second");

    Serial.print("HMAC Finalize ... ");

    hash->resetHMAC(buffer, hash->hashSize());
    hash->update("abc", 3);
    start = micros();
    for (count = 0; count < 1000; ++count) {
        hash->finalizeHMAC(buffer, hash->hashSize(), buffer, hash->hashSize());
    }
    elapsed = micros() - start;

    Serial.print(elapsed / 1000.0);
    Serial.print("us per op, ");
    Serial.print((1000.0 * 1000000.0) / elapsed);
    Serial.println(" ops per second");
}

void setup()
{
    Serial.begin(9600);

    Serial.println();

    Serial.print("State Size ...");
    Serial.println(sizeof(SHA1));
    Serial.println();

    Serial.println("Test Vectors:");
    testHash(&sha1, &testVectorSHA1_1);
    testHash(&sha1, &testVectorSHA1_2);
    testHMAC(&sha1, &testVectorHMAC_SHA1_1);
    testHMAC(&sha1, &testVectorHMAC_SHA1_2);
    testHMAC(&sha1, (size_t)0);
    testHMAC(&sha1, 1);
    testHMAC(&sha1, HASH_SIZE);
    testHMAC(&sha1, BLOCK_SIZE);
    testHMAC(&sha1, BLOCK_SIZE + 1);
    testHMAC(&sha1, sizeof(buffer));

    Serial.println();

    Serial.println("Performance Tests:");
    perfHash(&sha1);
    perfFinalize(&sha1);
    perfHMAC(&sha1);
}

void loop()
{
}
