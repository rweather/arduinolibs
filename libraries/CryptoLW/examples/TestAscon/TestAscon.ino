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

/*
This example runs tests on the Ascon128 implementation to verify
correct behaviour.
*/

#include <Crypto.h>
#include <CryptoLW.h>
#include <Ascon128.h>
#include "utility/ProgMemUtil.h"

#define MAX_PLAINTEXT_LEN 43
#define MAX_AUTHDATA_LEN 17

struct TestVector
{
    const char *name;
    uint8_t key[16];
    uint8_t plaintext[MAX_PLAINTEXT_LEN];
    uint8_t ciphertext[MAX_PLAINTEXT_LEN];
    uint8_t authdata[MAX_AUTHDATA_LEN];
    uint8_t iv[16];
    uint8_t tag[16];
    size_t authsize;
    size_t datasize;
};

// Test vectors for Ascon128, generated with the reference Python version:
// https://github.com/meichlseder/pyascon
static TestVector const testVectorAscon128_1 PROGMEM = {
    .name        = "Ascon128 #1",
    .key         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .plaintext   = {0x61, 0x73, 0x63, 0x6f, 0x6e},
    .ciphertext  = {0x86, 0x88, 0x62, 0x14, 0x0e},
    .authdata    = {0x41, 0x53, 0x43, 0x4f, 0x4e},
    .iv          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .tag         = {0xad, 0x65, 0xf5, 0x94, 0x22, 0x58, 0xda, 0xd5,
                    0x3c, 0xaa, 0x7a, 0x56, 0xf3, 0xa2, 0x92, 0xd8},
    .authsize    = 5,
    .datasize    = 5
};
static TestVector const testVectorAscon128_2 PROGMEM = {
    .name        = "Ascon128 #2",
    .key         = {0x0d, 0x49, 0x29, 0x92, 0x65, 0x8b, 0xd8, 0xa3,
                    0xe4, 0x7b, 0xf9, 0x10, 0xd4, 0xc5, 0x87, 0xad},
    .plaintext   = {0x61},
    .ciphertext  = {0xc5},
    .authdata    = {0},
    .iv          = {0x5a, 0xcb, 0x17, 0x2a, 0x1a, 0x93, 0x3d, 0xb1,
                    0x8a, 0x6a, 0x40, 0xac, 0x6e, 0x4c, 0x68, 0xd0},
    .tag         = {0x2e, 0x0b, 0xf2, 0xb1, 0xfc, 0xd8, 0x64, 0x69,
                    0x01, 0x1c, 0x4f, 0x8b, 0x78, 0x4a, 0x65, 0x0d},
    .authsize    = 0,
    .datasize    = 1
};
static TestVector const testVectorAscon128_3 PROGMEM = {
    .name        = "Ascon128 #3",
    .key         = {0x91, 0xb3, 0x9d, 0x22, 0xf3, 0xb7, 0x7f, 0x51,
                    0x33, 0x0a, 0xa3, 0xa4, 0xea, 0x38, 0xea, 0xa2},
    .plaintext   = {0},
    .ciphertext  = {0},
    .authdata    = {0x64},
    .iv          = {0x2e, 0xec, 0x64, 0x25, 0xb3, 0xec, 0xf0, 0x63,
                    0xb4, 0x3e, 0x29, 0xc7, 0x68, 0x29, 0x3c, 0x49},
    .tag         = {0xfd, 0x24, 0x0e, 0x3c, 0x3d, 0xc4, 0x11, 0x0d,
                    0xe1, 0x54, 0x4c, 0xd5, 0x24, 0x18, 0xd9, 0x4c},
    .authsize    = 1,
    .datasize    = 0
};
static TestVector const testVectorAscon128_4 PROGMEM = {
    .name        = "Ascon128 #4",
    .key         = {0x72, 0xfd, 0x18, 0xde, 0xbd, 0xee, 0x86, 0x13,
                    0x4f, 0x7c, 0x44, 0x29, 0x84, 0x37, 0x56, 0x06},
    .plaintext   = {0x70, 0x6c, 0x61, 0x69, 0x6e, 0x74, 0x78, 0x74},
    .ciphertext  = {0x91, 0xd0, 0xc3, 0x88, 0xea, 0xc0, 0xe6, 0xd9},
    .authdata    = {0x61, 0x73, 0x73, 0x64, 0x61, 0x74, 0x31, 0x32},
    .iv          = {0x91, 0x5f, 0xf8, 0xff, 0xca, 0xd8, 0xae, 0x1d,
                    0xf4, 0x45, 0xeb, 0x03, 0xe2, 0x18, 0xfd, 0x25},
    .tag         = {0x16, 0x69, 0x74, 0xbf, 0xbd, 0x43, 0xd7, 0xa8,
                    0xfe, 0x43, 0xf0, 0xce, 0xe2, 0xdd, 0xb9, 0xf8},
    .authsize    = 8,
    .datasize    = 8
};
static TestVector const testVectorAscon128_5 PROGMEM = {
    .name        = "Ascon128 #5",
    .key         = {0x8a, 0xa5, 0xed, 0xc5, 0x88, 0x49, 0x75, 0xc8,
                    0xd1, 0xa1, 0xb8, 0x44, 0xd0, 0x15, 0x50, 0x5a},
    .plaintext   = {0x54, 0x68, 0x65, 0x20, 0x72, 0x61, 0x69, 0x6e,
                    0x20, 0x69, 0x6e, 0x20, 0x73, 0x70, 0x61, 0x69,
                    0x6e, 0x20, 0x66, 0x61, 0x6c, 0x6c, 0x73, 0x20,
                    0x6d, 0x61, 0x69, 0x6e, 0x6c, 0x79, 0x20, 0x6f,
                    0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x70, 0x6c,
                    0x61, 0x69, 0x6e},
    .ciphertext  = {0x4a, 0xb4, 0xe2, 0x87, 0x90, 0x07, 0x4b, 0x78,
                    0x88, 0x70, 0x71, 0xc0, 0x62, 0xd6, 0xab, 0x6b,
                    0x32, 0xd4, 0xb1, 0xec, 0xc7, 0xd8, 0x44, 0x93,
                    0x36, 0x9a, 0x38, 0x81, 0xd6, 0x65, 0x2f, 0x85,
                    0xaa, 0xf9, 0x70, 0x90, 0x61, 0x97, 0x3e, 0x1f,
                    0x60, 0x12, 0x66},
    .authdata    = {0x48, 0x6f, 0x77, 0x20, 0x6e, 0x6f, 0x77, 0x20,
                    0x62, 0x72, 0x6f, 0x77, 0x6e, 0x20, 0x63, 0x6f,
                    0x77},
    .iv          = {0xbc, 0x52, 0x27, 0xa5, 0x72, 0x58, 0xfe, 0x00,
                    0xcb, 0x7b, 0x0f, 0x31, 0xa4, 0xb6, 0xff, 0xda},
    .tag         = {0x92, 0xfe, 0x72, 0xf8, 0x69, 0xc9, 0x95, 0x41,
                    0x1f, 0xc4, 0x57, 0xde, 0xa6, 0xf2, 0xf9, 0x2d},
    .authsize    = 17,
    .datasize    = 43
};

TestVector testVector;

Ascon128 acorn;

byte buffer[128];

bool testCipher_N(Ascon128 *cipher, const struct TestVector *test, size_t inc)
{
    size_t posn, len;
    uint8_t tag[16];

    if (!inc)
        inc = 1;

    cipher->clear();
    if (!cipher->setKey(test->key, 16)) {
        Serial.print("setKey ");
        return false;
    }
    if (!cipher->setIV(test->iv, 16)) {
        Serial.print("setIV ");
        return false;
    }

    memset(buffer, 0xBA, sizeof(buffer));

    for (posn = 0; posn < test->authsize; posn += inc) {
        len = test->authsize - posn;
        if (len > inc)
            len = inc;
        cipher->addAuthData(test->authdata + posn, len);
    }

    for (posn = 0; posn < test->datasize; posn += inc) {
        len = test->datasize - posn;
        if (len > inc)
            len = inc;
        cipher->encrypt(buffer + posn, test->plaintext + posn, len);
    }

    if (memcmp(buffer, test->ciphertext, test->datasize) != 0) {
        Serial.print(buffer[0], HEX);
        Serial.print("->");
        Serial.print(test->ciphertext[0], HEX);
        return false;
    }

    cipher->computeTag(tag, sizeof(tag));
    if (memcmp(tag, test->tag, sizeof(tag)) != 0) {
        Serial.print("computed wrong tag ... ");
        return false;
    }

    cipher->setKey(test->key, 16);
    cipher->setIV(test->iv, 16);

    for (posn = 0; posn < test->authsize; posn += inc) {
        len = test->authsize - posn;
        if (len > inc)
            len = inc;
        cipher->addAuthData(test->authdata + posn, len);
    }

    for (posn = 0; posn < test->datasize; posn += inc) {
        len = test->datasize - posn;
        if (len > inc)
            len = inc;
        cipher->decrypt(buffer + posn, test->ciphertext + posn, len);
    }

    if (memcmp(buffer, test->plaintext, test->datasize) != 0)
        return false;

    if (!cipher->checkTag(tag, sizeof(tag))) {
        Serial.print("tag did not check ... ");
        return false;
    }

    return true;
}

void testCipher(Ascon128 *cipher, const struct TestVector *test)
{
    bool ok;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" ... ");

    ok  = testCipher_N(cipher, test, test->datasize);
    ok &= testCipher_N(cipher, test, 1);
    ok &= testCipher_N(cipher, test, 2);
    ok &= testCipher_N(cipher, test, 5);
    ok &= testCipher_N(cipher, test, 8);
    ok &= testCipher_N(cipher, test, 13);
    ok &= testCipher_N(cipher, test, 16);

    if (ok)
        Serial.println("Passed");
    else
        Serial.println("Failed");
}

void perfCipherSetKey(Ascon128 *cipher, const struct TestVector *test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" SetKey ... ");

    start = micros();
    for (count = 0; count < 1000; ++count) {
        cipher->setKey(test->key, 16);
        cipher->setIV(test->iv, 16);
    }
    elapsed = micros() - start;

    Serial.print(elapsed / 1000.0);
    Serial.print("us per operation, ");
    Serial.print((1000.0 * 1000000.0) / elapsed);
    Serial.println(" per second");
}

void perfCipherEncrypt(Ascon128 *cipher, const struct TestVector *test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" Encrypt ... ");

    cipher->setKey(test->key, 16);
    cipher->setIV(test->iv, 16);
    start = micros();
    for (count = 0; count < 500; ++count) {
        cipher->encrypt(buffer, buffer, 128);
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (128.0 * 500.0));
    Serial.print("us per byte, ");
    Serial.print((128.0 * 500.0 * 1000000.0) / elapsed);
    Serial.println(" bytes per second");
}

void perfCipherDecrypt(Ascon128 *cipher, const struct TestVector *test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" Decrypt ... ");

    cipher->setKey(test->key, 16);
    cipher->setIV(test->iv, 16);
    start = micros();
    for (count = 0; count < 500; ++count) {
        cipher->decrypt(buffer, buffer, 128);
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (128.0 * 500.0));
    Serial.print("us per byte, ");
    Serial.print((128.0 * 500.0 * 1000000.0) / elapsed);
    Serial.println(" bytes per second");
}

void perfCipherAddAuthData(Ascon128 *cipher, const struct TestVector *test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" AddAuthData ... ");

    cipher->setKey(test->key, 16);
    cipher->setIV(test->iv, 16);
    start = micros();
    memset(buffer, 0xBA, 128);
    for (count = 0; count < 500; ++count) {
        cipher->addAuthData(buffer, 128);
    }
    elapsed = micros() - start;

    Serial.print(elapsed / (128.0 * 500.0));
    Serial.print("us per byte, ");
    Serial.print((128.0 * 500.0 * 1000000.0) / elapsed);
    Serial.println(" bytes per second");
}

void perfCipherComputeTag(Ascon128 *cipher, const struct TestVector *test)
{
    unsigned long start;
    unsigned long elapsed;
    int count;

    memcpy_P(&testVector, test, sizeof(TestVector));
    test = &testVector;

    Serial.print(test->name);
    Serial.print(" ComputeTag ... ");

    cipher->setKey(test->key, 16);
    cipher->setIV(test->iv, 16);
    start = micros();
    for (count = 0; count < 1000; ++count) {
        cipher->computeTag(buffer, 16);
    }
    elapsed = micros() - start;

    Serial.print(elapsed / 1000.0);
    Serial.print("us per operation, ");
    Serial.print((1000.0 * 1000000.0) / elapsed);
    Serial.println(" per second");
}

void perfCipher(Ascon128 *cipher, const struct TestVector *test)
{
    perfCipherSetKey(cipher, test);
    perfCipherEncrypt(cipher, test);
    perfCipherDecrypt(cipher, test);
    perfCipherAddAuthData(cipher, test);
    perfCipherComputeTag(cipher, test);
}

void setup()
{
    Serial.begin(9600);

    Serial.println();

    Serial.print("State Size ... ");
    Serial.println(sizeof(Ascon128));
    Serial.println();

    Serial.println("Test Vectors:");
    testCipher(&acorn, &testVectorAscon128_1);
    testCipher(&acorn, &testVectorAscon128_2);
    testCipher(&acorn, &testVectorAscon128_3);
    testCipher(&acorn, &testVectorAscon128_4);
    testCipher(&acorn, &testVectorAscon128_5);

    Serial.println();

    Serial.println("Performance Tests:");
    perfCipher(&acorn, &testVectorAscon128_4);
}

void loop()
{
}
