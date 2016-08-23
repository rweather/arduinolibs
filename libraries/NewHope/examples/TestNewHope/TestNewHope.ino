/*
 * Copyright (C) 2016 Southern Storm Software, Pty Ltd.
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
This example runs tests on the NewHope class to verify correct behaviour.
*/

#include <Crypto.h>
#include <NewHope.h>
#include <RNG.h>
#include <string.h>

// Test vectors generated with the reference C version.  Note: Only the
// first 32 bytes of the public keys are included because the full values
// are massive (1834 bytes for Alice, 2048 bytes for Bob).
struct TestVector
{
    const char *name;
    uint8_t alice_random[64];
    uint8_t alice_public[32];
    uint8_t bob_random[32];
    uint8_t bob_public[32];
    uint8_t shared_key[32];
};
static struct TestVector const testNewHope1 = { // "ref" variant
    "New Hope #1",
    {0x93, 0x4d, 0x60, 0xb3, 0x56, 0x24, 0xd7, 0x40,
     0xb3, 0x0a, 0x7f, 0x22, 0x7a, 0xf2, 0xae, 0x7c,
     0x67, 0x8e, 0x4e, 0x04, 0xe1, 0x3c, 0x5f, 0x50,
     0x9e, 0xad, 0xe2, 0xb7, 0x9a, 0xea, 0x77, 0xe2,
     0x3e, 0x2a, 0x2e, 0xa6, 0xc9, 0xc4, 0x76, 0xfc,
     0x49, 0x37, 0xb0, 0x13, 0xc9, 0x93, 0xa7, 0x93,
     0xd6, 0xc0, 0xab, 0x99, 0x60, 0x69, 0x5b, 0xa8,
     0x38, 0xf6, 0x49, 0xda, 0x53, 0x9c, 0xa3, 0xd0},
    {0xa8, 0x57, 0xf3, 0xc1, 0x2d, 0x1e, 0xa4, 0x3c,
     0xcd, 0x04, 0xeb, 0xc9, 0xed, 0x8d, 0x78, 0x53,
     0x69, 0xe4, 0x7e, 0x76, 0x32, 0x5a, 0xac, 0x77,
     0x88, 0xdc, 0x74, 0x98, 0x67, 0x64, 0x52, 0xdb},
    {0xba, 0xc5, 0xba, 0x88, 0x1d, 0xd3, 0x5c, 0x59,
     0x71, 0x96, 0x70, 0x00, 0x46, 0x92, 0xd6, 0x75,
     0xb8, 0x3c, 0x98, 0xdb, 0x6a, 0x0e, 0x55, 0x80,
     0x0b, 0xaf, 0xeb, 0x7e, 0x70, 0x49, 0x1b, 0xf4},
    {0x49, 0xa5, 0x74, 0x02, 0xd4, 0x29, 0x88, 0xe2,
     0x98, 0x08, 0xf7, 0x86, 0x59, 0x89, 0xa0, 0x00,
     0xfa, 0x3b, 0x6a, 0x5d, 0x4f, 0xe6, 0x00, 0x9c,
     0x58, 0x49, 0x0d, 0xb2, 0xaf, 0x40, 0x03, 0x69},
    {0x47, 0x25, 0xa8, 0x62, 0x6c, 0x73, 0xa2, 0xf8,
     0x3f, 0x74, 0xfc, 0x1a, 0x8c, 0x29, 0x5e, 0x1a,
     0x2e, 0x7f, 0x49, 0x71, 0x4e, 0x25, 0xd5, 0x0f,
     0x9d, 0xc9, 0xa5, 0x35, 0x04, 0xc5, 0x55, 0x82}
};
static struct TestVector const testNewHope2 = { // "torref" variant
    "New Hope #2",
    {0x0f, 0xdb, 0xb1, 0x16, 0x9f, 0x78, 0x56, 0x69,
     0xa4, 0x06, 0x10, 0x33, 0x36, 0xa4, 0xa1, 0xd9,
     0x3f, 0xfa, 0x24, 0x26, 0x99, 0x70, 0xf5, 0x16,
     0x01, 0xdb, 0x53, 0x38, 0xad, 0x82, 0xd4, 0x6d,
     0xc7, 0x30, 0x0e, 0x2d, 0x89, 0x4b, 0x0e, 0xaa,
     0x40, 0xa6, 0xab, 0x25, 0x45, 0x06, 0xd8, 0xc1,
     0x17, 0x6a, 0x33, 0xc4, 0xa1, 0xb2, 0x87, 0x96,
     0x04, 0xb1, 0xb8, 0x0d, 0xf4, 0x8d, 0x31, 0xdd},
    {0xe5, 0x61, 0x8e, 0x97, 0xad, 0x1e, 0x54, 0x32,
     0xa0, 0x3d, 0x28, 0xf1, 0xdd, 0xae, 0xd6, 0x43,
     0x51, 0x9a, 0x09, 0xb8, 0x05, 0xfc, 0x21, 0xb1,
     0x08, 0xcf, 0xe8, 0x4a, 0x82, 0x12, 0x10, 0xf0},
    {0x0d, 0x39, 0x9d, 0xc9, 0x1d, 0x85, 0x30, 0xb7,
     0x2c, 0x5d, 0x9a, 0x99, 0x20, 0xf3, 0x3b, 0x43,
     0x33, 0x1b, 0x98, 0x3b, 0x95, 0x04, 0x7f, 0x96,
     0xb5, 0xb0, 0x99, 0xbe, 0x39, 0x93, 0x55, 0xce},
    {0x1e, 0x21, 0x5d, 0x83, 0x04, 0x59, 0x8e, 0xf2,
     0x07, 0xd3, 0xc2, 0x51, 0xa2, 0x3b, 0x6b, 0xeb,
     0x1c, 0xc4, 0x80, 0x74, 0x0c, 0xdb, 0xd4, 0xfc,
     0xc5, 0x86, 0x7a, 0x8a, 0x9e, 0xaa, 0xfa, 0x22},
    {0x5e, 0x29, 0x90, 0x76, 0xaf, 0x0f, 0xe2, 0x1b,
     0xf8, 0x83, 0xfd, 0x1a, 0xee, 0xd8, 0xd4, 0x79,
     0x62, 0x31, 0x0b, 0x6d, 0x46, 0x96, 0x50, 0xe8,
     0x3c, 0xfb, 0x28, 0xcc, 0xda, 0xe6, 0x36, 0x0c}
};

NewHopePrivateKey alice_private;
uint8_t alice_public[NEWHOPE_SENDABYTES];
uint8_t alice_shared[NEWHOPE_SHAREDBYTES];
uint8_t bob_public[NEWHOPE_SENDBBYTES];
uint8_t bob_shared[NEWHOPE_SHAREDBYTES];

void testNewHopePerf()
{
    unsigned long start;
    unsigned long elapsed;

    Serial.print("keygen ... ");
    start = micros();
    NewHope::keygen(alice_public, alice_private);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    Serial.print("sharedb ... ");
    start = micros();
    NewHope::sharedb(bob_shared, bob_public, alice_public);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    Serial.print("shareda ... ");
    start = micros();
    NewHope::shareda(alice_shared, alice_private, bob_public);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    if (!memcmp(alice_shared, bob_shared, NEWHOPE_SHAREDBYTES))
        Serial.println("ok ... shared secrets match");
    else
        Serial.println("failed ... shared secrets do not match");
}

void testNewHopeTorPerf()
{
    unsigned long start;
    unsigned long elapsed;

    Serial.print("torref keygen ... ");
    start = micros();
    NewHope::keygen(alice_public, alice_private, NewHope::Torref);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    Serial.print("torref sharedb ... ");
    start = micros();
    NewHope::sharedb(bob_shared, bob_public, alice_public, NewHope::Torref);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    Serial.print("torref shareda ... ");
    start = micros();
    NewHope::shareda(alice_shared, alice_private, bob_public);
    elapsed = micros() - start;
    Serial.print(elapsed);
    Serial.println(" us");

    if (!memcmp(alice_shared, bob_shared, NEWHOPE_SHAREDBYTES))
        Serial.println("ok ... shared secrets match");
    else
        Serial.println("failed ... shared secrets do not match");
}

void testNewHopeVector(const struct TestVector *test, NewHope::Variant variant)
{
    bool ok = true;
    Serial.print(test->name);
    Serial.print(" ... ");

    NewHope::keygen(alice_public, alice_private, variant, test->alice_random);
    if (memcmp(alice_public, test->alice_public, 32) != 0) {
        Serial.print("alice public doesn't match ... ");
        ok = false;
    }

    NewHope::sharedb(bob_shared, bob_public, alice_public, variant, test->bob_random);
    if (memcmp(bob_public, test->bob_public, 32) != 0) {
        Serial.print("bob public doesn't match ... ");
        ok = false;
    }
    if (memcmp(bob_shared, test->shared_key, 32) != 0) {
        Serial.print("bob shared doesn't match ... ");
        ok = false;
    }

    NewHope::shareda(alice_shared, alice_private, bob_public);
    if (memcmp(alice_shared, test->shared_key, 32) != 0) {
        Serial.print("alice shared doesn't match ... ");
        ok = false;
    }

    if (ok)
        Serial.println("ok");
    else
        Serial.println("failed");
}

void setup()
{
    Serial.begin(9600);

    // Start the random number generator.  We don't initialise a noise
    // source here because we don't need one for testing purposes.
    // Real applications should of course use a proper noise source.
    RNG.begin("TestNewHope 1.0", 950);

    // Perform the tests.
    Serial.println();
    testNewHopePerf();
    Serial.println();
    testNewHopeTorPerf();
    Serial.println();
    testNewHopeVector(&testNewHope1, NewHope::Ref);
    testNewHopeVector(&testNewHope2, NewHope::Torref);
}

void loop()
{
}
