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
This example runs tests on the SHA512 implementation to verify correct behaviour.
*/

#include <Crypto.h>
#include <SHA512.h>
#include <string.h>

#define HASH_SIZE 64

struct TestHashVector
{
    const char *name;
    const char *data;
    uint8_t hash[HASH_SIZE];
};

static TestHashVector const testVectorSHA512_1 = {
    "SHA-512 #1",
    "",
    {0xcf, 0x83, 0xe1, 0x35, 0x7e, 0xef, 0xb8, 0xbd,
     0xf1, 0x54, 0x28, 0x50, 0xd6, 0x6d, 0x80, 0x07,
     0xd6, 0x20, 0xe4, 0x05, 0x0b, 0x57, 0x15, 0xdc,
     0x83, 0xf4, 0xa9, 0x21, 0xd3, 0x6c, 0xe9, 0xce,
     0x47, 0xd0, 0xd1, 0x3c, 0x5d, 0x85, 0xf2, 0xb0,
     0xff, 0x83, 0x18, 0xd2, 0x87, 0x7e, 0xec, 0x2f,
     0x63, 0xb9, 0x31, 0xbd, 0x47, 0x41, 0x7a, 0x81,
     0xa5, 0x38, 0x32, 0x7a, 0xf9, 0x27, 0xda, 0x3e}
};
static TestHashVector const testVectorSHA512_2 = {
    "SHA-512 #2",
    "abc",
    {0xdd, 0xaf, 0x35, 0xa1, 0x93, 0x61, 0x7a, 0xba,
     0xcc, 0x41, 0x73, 0x49, 0xae, 0x20, 0x41, 0x31,
     0x12, 0xe6, 0xfa, 0x4e, 0x89, 0xa9, 0x7e, 0xa2,
     0x0a, 0x9e, 0xee, 0xe6, 0x4b, 0x55, 0xd3, 0x9a,
     0x21, 0x92, 0x99, 0x2a, 0x27, 0x4f, 0xc1, 0xa8,
     0x36, 0xba, 0x3c, 0x23, 0xa3, 0xfe, 0xeb, 0xbd,
     0x45, 0x4d, 0x44, 0x23, 0x64, 0x3c, 0xe8, 0x0e,
     0x2a, 0x9a, 0xc9, 0x4f, 0xa5, 0x4c, 0xa4, 0x9f}
};
static TestHashVector const testVectorSHA512_3 = {
    "SHA-512 #3",
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
    "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
    {0x8e, 0x95, 0x9b, 0x75, 0xda, 0xe3, 0x13, 0xda,
     0x8c, 0xf4, 0xf7, 0x28, 0x14, 0xfc, 0x14, 0x3f,
     0x8f, 0x77, 0x79, 0xc6, 0xeb, 0x9f, 0x7f, 0xa1,
     0x72, 0x99, 0xae, 0xad, 0xb6, 0x88, 0x90, 0x18,
     0x50, 0x1d, 0x28, 0x9e, 0x49, 0x00, 0xf7, 0xe4,
     0x33, 0x1b, 0x99, 0xde, 0xc4, 0xb5, 0x43, 0x3a,
     0xc7, 0xd3, 0x29, 0xee, 0xb6, 0xdd, 0x26, 0x54,
     0x5e, 0x96, 0xe5, 0x5b, 0x87, 0x4b, 0xe9, 0x09}
};

SHA512 sha512;

byte buffer[128];

bool testHash_N(Hash *hash, const struct TestHashVector *test, size_t inc)
{
    size_t size = strlen(test->data);
    size_t posn, len;
    uint8_t value[HASH_SIZE];

    for (posn = 0; posn < size; posn += inc) {
        len = size - posn;
        if (len > inc)
            len = inc;
        hash->update(test->data + posn, len);
    }
    hash->finalize(value, sizeof(value));
    if (memcmp(value, test->hash, sizeof(value)) != 0)
        return false;

    // Try again to make sure the hash resets.
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

void setup()
{
    Serial.begin(9600);

    Serial.println();

    Serial.println("Test Vectors:");
    testHash(&sha512, &testVectorSHA512_1);
    testHash(&sha512, &testVectorSHA512_2);
    testHash(&sha512, &testVectorSHA512_3);

    Serial.println();

    Serial.println("Performance Tests:");
    perfHash(&sha512);
}

void loop()
{
}
