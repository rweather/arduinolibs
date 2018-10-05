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
This example runs tests on the SHA384 implementation to verify correct behaviour.
*/

#include <Crypto.h>
#include <SHA384.h>
#include <string.h>

#define HASH_SIZE 48
#define BLOCK_SIZE 128

struct TestHashVector
{
    const char *name;
    const char *data;
    uint8_t hash[HASH_SIZE];
};

static TestHashVector const testVectorSHA384_1 = {
    "SHA-384 #1",
    "",
    {0x38, 0xb0, 0x60, 0xa7, 0x51, 0xac, 0x96, 0x38, 
     0x4c, 0xd9, 0x32, 0x7e, 0xb1, 0xb1, 0xe3, 0x6a, 
     0x21, 0xfd, 0xb7, 0x11, 0x14, 0xbe, 0x07, 0x43, 
     0x4c, 0x0c, 0xc7, 0xbf, 0x63, 0xf6, 0xe1, 0xda, 
     0x27, 0x4e, 0xde, 0xbf, 0xe7, 0x6f, 0x65, 0xfb, 
     0xd5, 0x1a, 0xd2, 0xf1, 0x48, 0x98, 0xb9, 0x5b}
};
static TestHashVector const testVectorSHA384_2 = {
    "SHA-384 #2",
    "abc",
    {0xcb, 0x00, 0x75, 0x3f, 0x45, 0xa3, 0x5e, 0x8b, 
     0xb5, 0xa0, 0x3d, 0x69, 0x9a, 0xc6, 0x50, 0x07, 
     0x27, 0x2c, 0x32, 0xab, 0x0e, 0xde, 0xd1, 0x63, 
     0x1a, 0x8b, 0x60, 0x5a, 0x43, 0xff, 0x5b, 0xed, 
     0x80, 0x86, 0x07, 0x2b, 0xa1, 0xe7, 0xcc, 0x23, 
     0x58, 0xba, 0xec, 0xa1, 0x34, 0xc8, 0x25, 0xa7}
};
static TestHashVector const testVectorSHA384_3 = {
    "SHA-384 #3",
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
    "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
    {0x09, 0x33, 0x0c, 0x33, 0xf7, 0x11, 0x47, 0xe8, 
     0x3d, 0x19, 0x2f, 0xc7, 0x82, 0xcd, 0x1b, 0x47, 
     0x53, 0x11, 0x1b, 0x17, 0x3b, 0x3b, 0x05, 0xd2, 
     0x2f, 0xa0, 0x80, 0x86, 0xe3, 0xb0, 0xf7, 0x12, 
     0xfc, 0xc7, 0xc7, 0x1a, 0x55, 0x7e, 0x2d, 0xb9, 
     0x66, 0xc3, 0xe9, 0xfa, 0x91, 0x74, 0x60, 0x39}
};

SHA384 sha384;

byte buffer[BLOCK_SIZE + 2];

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
    for (count = 0; count < 250; ++count) {
        hash->update(buffer, sizeof(buffer));
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (sizeof(buffer) * 250.0));
    Serial.print("us per byte, ");
    Serial.print((sizeof(buffer) * 250.0 * 1000000.0) / elapsed);
    Serial.println(" bytes per second");
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

    Serial.print("HMAC-SHA-384 keysize=");
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

void setup()
{
    Serial.begin(9600);

    Serial.println();

    Serial.print("State Size ...");
    Serial.println(sizeof(SHA384));
    Serial.println();

    Serial.println("Test Vectors:");
    testHash(&sha384, &testVectorSHA384_1);
    testHash(&sha384, &testVectorSHA384_2);
    testHash(&sha384, &testVectorSHA384_3);
    testHMAC(&sha384, (size_t)0);
    testHMAC(&sha384, 1);
    testHMAC(&sha384, HASH_SIZE);
    testHMAC(&sha384, BLOCK_SIZE);
    testHMAC(&sha384, BLOCK_SIZE + 1);
    testHMAC(&sha384, BLOCK_SIZE + 2);

    Serial.println();

    Serial.println("Performance Tests:");
    perfHash(&sha384);
    perfFinalize(&sha384);
}

void loop()
{
}
