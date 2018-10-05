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
This example runs tests on the SHA224 implementation to verify correct behaviour.
*/

#include <Crypto.h>
#include <SHA224.h>
#include <string.h>

#define HASH_SIZE 28
#define BLOCK_SIZE 64

struct TestHashVector
{
    const char *name;
    const char *key;
    const char *data;
    uint8_t hash[HASH_SIZE];
};

static TestHashVector const testVectorSHA224_1 = {
    "SHA-224 #1",
    0,
    "abc",
    {0x23, 0x09, 0x7d, 0x22, 0x34, 0x05, 0xd8, 0x22,
     0x86, 0x42, 0xa4, 0x77, 0xbd, 0xa2, 0x55, 0xb3,
     0x2a, 0xad, 0xbc, 0xe4, 0xbd, 0xa0, 0xb3, 0xf7,
     0xe3, 0x6c, 0x9d, 0xa7}
};
static TestHashVector const testVectorSHA224_2 = {
    "SHA-224 #2",
    0,
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    {0x75, 0x38, 0x8b, 0x16, 0x51, 0x27, 0x76, 0xcc,
     0x5d, 0xba, 0x5d, 0xa1, 0xfd, 0x89, 0x01, 0x50,
     0xb0, 0xc6, 0x45, 0x5c, 0xb4, 0xf5, 0x8b, 0x19,
     0x52, 0x52, 0x25, 0x25}
};
static TestHashVector const testVectorHMAC_SHA224_1 = {
    "HMAC-SHA-224 #1",
    "",
    "",
    {0x5c, 0xe1, 0x4f, 0x72, 0x89, 0x46, 0x62, 0x21,
     0x3e, 0x27, 0x48, 0xd2, 0xa6, 0xba, 0x23, 0x4b,
     0x74, 0x26, 0x39, 0x10, 0xce, 0xdd, 0xe2, 0xf5,
     0xa9, 0x27, 0x15, 0x24}
};
static TestHashVector const testVectorHMAC_SHA224_2 = {
    "HMAC-SHA-224 #2",
    "key",
    "The quick brown fox jumps over the lazy dog",
    {0x88, 0xff, 0x8b, 0x54, 0x67, 0x5d, 0x39, 0xb8,
     0xf7, 0x23, 0x22, 0xe6, 0x5f, 0xf9, 0x45, 0xc5,
     0x2d, 0x96, 0x37, 0x99, 0x88, 0xad, 0xa2, 0x56,
     0x39, 0x74, 0x7e, 0x69}
};

SHA224 sha224;

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

    Serial.print("HMAC-SHA-224 keysize=");
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
    for (count = 0; count < 500; ++count) {
        hash->update(buffer, sizeof(buffer));
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (sizeof(buffer) * 500.0));
    Serial.print("us per byte, ");
    Serial.print((sizeof(buffer) * 500.0 * 1000000.0) / elapsed);
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
    Serial.println(sizeof(SHA224));
    Serial.println();

    Serial.println("Test Vectors:");
    testHash(&sha224, &testVectorSHA224_1);
    testHash(&sha224, &testVectorSHA224_2);
    testHMAC(&sha224, &testVectorHMAC_SHA224_1);
    testHMAC(&sha224, &testVectorHMAC_SHA224_2);
    testHMAC(&sha224, (size_t)0);
    testHMAC(&sha224, 1);
    testHMAC(&sha224, HASH_SIZE);
    testHMAC(&sha224, BLOCK_SIZE);
    testHMAC(&sha224, BLOCK_SIZE + 1);
    testHMAC(&sha224, sizeof(buffer));

    Serial.println();

    Serial.println("Performance Tests:");
    perfHash(&sha224);
    perfFinalize(&sha224);
    perfHMAC(&sha224);
}

void loop()
{
}
