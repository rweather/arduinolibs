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
This example runs tests on the Acorn128 implementation to verify
correct behaviour.
*/

#include <Crypto.h>
#include <CryptoLW.h>
#include <Acorn128.h>
#include "utility/ProgMemUtil.h"

#define MAX_PLAINTEXT_LEN 73
#define MAX_AUTHDATA_LEN 39

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

// Test vectors for Acorn128 from the specification.
static TestVector const testVectorAcorn128_1 PROGMEM = {
    .name        = "Acorn128 #1",
    .key         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .plaintext   = {0},
    .ciphertext  = {0},
    .authdata    = {0},
    .iv          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .tag         = {0x83, 0x5e, 0x53, 0x17, 0x89, 0x6e, 0x86, 0xb2,
                    0x44, 0x71, 0x43, 0xc7, 0x4f, 0x6f, 0xfc, 0x1e},
    .authsize    = 0,
    .datasize    = 0
};
static TestVector const testVectorAcorn128_2 PROGMEM = {
    .name        = "Acorn128 #2",
    .key         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .plaintext   = {0x01},
    .ciphertext  = {0x2b},
    .authdata    = {0},
    .iv          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .tag         = {0x4b, 0x60, 0x64, 0x0e, 0x26, 0xf0, 0xa9, 0x9d,
                    0xd0, 0x1f, 0x93, 0xbf, 0x63, 0x49, 0x97, 0xcb},
    .authsize    = 0,
    .datasize    = 1
};
static TestVector const testVectorAcorn128_3 PROGMEM = {
    .name        = "Acorn128 #3",
    .key         = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .plaintext   = {0},
    .ciphertext  = {0},
    .authdata    = {0x01},
    .iv          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .tag         = {0x98, 0x2e, 0xf7, 0xd1, 0xbb, 0xa7, 0xf8, 0x9a,
                    0x15, 0x75, 0x29, 0x7a, 0x09, 0x5c, 0xd7, 0xf2},
    .authsize    = 1,
    .datasize    = 0
};
static TestVector const testVectorAcorn128_4 PROGMEM = {
    .name        = "Acorn128 #4",
    .key         = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    .plaintext   = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    .ciphertext  = {0x86, 0x80, 0x1f, 0xa8, 0x9e, 0x33, 0xd9, 0x92,
                    0x35, 0xdd, 0x4d, 0x1a, 0x72, 0xce, 0x00, 0x1a},
    .authdata    = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    .iv          = {0x00, 0x03, 0x06, 0x09, 0x0c, 0x0f, 0x12, 0x15,
                    0x18, 0x1b, 0x1e, 0x21, 0x24, 0x27, 0x2a, 0x2d},
    .tag         = {0xd9, 0xc6, 0x6b, 0x4a, 0xdb, 0x3c, 0xde, 0x07,
                    0x3e, 0x63, 0x50, 0xcc, 0x7e, 0x23, 0x7e, 0x01},
    .authsize    = 16,
    .datasize    = 16
};
static TestVector const testVectorAcorn128_5 PROGMEM = {
    .name        = "Acorn128 #5",
    .key         = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
    .plaintext   = {0x00, 0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31,
                    0x38, 0x3f, 0x46, 0x4d, 0x54, 0x5b, 0x62, 0x69,
                    0x70, 0x77, 0x7e, 0x85, 0x8c, 0x93, 0x9a, 0xa1,
                    0xa8, 0xaf, 0xb6, 0xbd, 0xc4, 0xcb, 0xd2, 0xd9,
                    0xe0, 0xe7, 0xee, 0xf5, 0xfc, 0x03, 0x0a, 0x11,
                    0x18, 0x1f, 0x26, 0x2d, 0x34, 0x3b, 0x42, 0x49,
                    0x50, 0x57, 0x5e, 0x65, 0x6c, 0x73, 0x7a, 0x81,
                    0x88, 0x8f, 0x96, 0x9d, 0xa4, 0xab, 0xb2, 0xb9,
                    0xc0, 0xc7, 0xce, 0xd5, 0xdc, 0xe3, 0xea, 0xf1,
                    0xf8},
    .ciphertext  = {0xe7, 0xef, 0x31, 0x63, 0x78, 0x44, 0x46, 0x44,
                    0x70, 0x5c, 0x43, 0x81, 0xc8, 0x88, 0x83, 0x3b,
                    0x6d, 0x62, 0xa7, 0x49, 0x00, 0x5a, 0xb8, 0xfa,
                    0x14, 0x6a, 0x85, 0x90, 0x4d, 0x5e, 0x5a, 0xb7,
                    0x7c, 0x57, 0x58, 0x21, 0x58, 0x39, 0x5d, 0x8f,
                    0xe6, 0xb6, 0x66, 0xe6, 0xc8, 0x51, 0x77, 0x64,
                    0x8a, 0xeb, 0x77, 0x84, 0xcf, 0x2e, 0xea, 0xed,
                    0x3c, 0x22, 0xe7, 0xe9, 0x6b, 0xf5, 0x90, 0x09,
                    0xcd, 0x7a, 0xd2, 0x1b, 0xa5, 0xdf, 0x1a, 0x0f,
                    0xc0},
    .authdata    = {0x00, 0x05, 0x0a, 0x0f, 0x14, 0x19, 0x1e, 0x23,
                    0x28, 0x2d, 0x32, 0x37, 0x3c, 0x41, 0x46, 0x4b,
                    0x50, 0x55, 0x5a, 0x5f, 0x64, 0x69, 0x6e, 0x73,
                    0x78, 0x7d, 0x82, 0x87, 0x8c, 0x91, 0x96, 0x9b,
                    0xa0, 0xa5, 0xaa, 0xaf, 0xb4, 0xb9, 0xbe},
    .iv          = {0x00, 0x03, 0x06, 0x09, 0x0c, 0x0f, 0x12, 0x15,
                    0x18, 0x1b, 0x1e, 0x21, 0x24, 0x27, 0x2a, 0x2d},
    .tag         = {0x51, 0xb4, 0xbd, 0x86, 0xc6, 0x8c, 0xcf, 0x06,
                    0x82, 0xf5, 0x69, 0x5d, 0x26, 0x67, 0xd5, 0x35},
    .authsize    = 39,
    .datasize    = 73
};

TestVector testVector;

Acorn128 acorn;

byte buffer[128];

bool testCipher_N(Acorn128 *cipher, const struct TestVector *test, size_t inc)
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

void testCipher(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipherSetKey(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipherEncrypt(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipherDecrypt(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipherAddAuthData(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipherComputeTag(Acorn128 *cipher, const struct TestVector *test)
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

void perfCipher(Acorn128 *cipher, const struct TestVector *test)
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
    Serial.println(sizeof(Acorn128));
    Serial.println();

    Serial.println("Test Vectors:");
    testCipher(&acorn, &testVectorAcorn128_1);
    testCipher(&acorn, &testVectorAcorn128_2);
    testCipher(&acorn, &testVectorAcorn128_3);
    testCipher(&acorn, &testVectorAcorn128_4);
    testCipher(&acorn, &testVectorAcorn128_5);

    Serial.println();

    Serial.println("Performance Tests:");
    perfCipher(&acorn, &testVectorAcorn128_4);
}

void loop()
{
}
