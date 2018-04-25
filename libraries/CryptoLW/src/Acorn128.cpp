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

#include "Acorn128.h"
#include "Crypto.h"
#include "utility/EndianUtil.h"
#include <string.h>

/**
 * \class Acorn128 Acorn128.h <Acorn128.h>
 * \brief ACORN-128 authenticated cipher.
 *
 * Acorn128 is an authenticated cipher designed for memory-limited
 * environments with a 128-bit key, a 128-bit initialization vector,
 * and a 128-bit authentication tag.  It was one of the finalists
 * in the CAESAR AEAD competition.
 *
 * References: http://competitions.cr.yp.to/round3/acornv3.pdf,
 * http://competitions.cr.yp.to/caesar-submissions.html
 *
 * \sa AuthenticatedCipher
 */

/**
 * \brief Constructs a new Acorn128 authenticated cipher.
 */
Acorn128::Acorn128()
{
    state.authDone = 0;
}

/**
 * \brief Destroys this Acorn128 authenticated cipher.
 */
Acorn128::~Acorn128()
{
    clean(state);
}

/**
 * \brief Gets the size of the Acorn128 key in bytes.
 *
 * \return Always returns 16, indicating a 128-bit key.
 */
size_t Acorn128::keySize() const
{
    return 16;
}

/**
 * \brief Gets the size of the Acorn128 initialization vector in bytes.
 *
 * \return Always returns 16, indicating a 128-bit IV.
 *
 * Authentication tags may be truncated to 8 bytes, but the algorithm authors
 * recommend using a full 16-byte tag.
 */
size_t Acorn128::ivSize() const
{
    return 16;
}

/**
 * \brief Gets the size of the Acorn128 authentication tag in bytes.
 *
 * \return Always returns 16, indicating a 128-bit authentication tag.
 */
size_t Acorn128::tagSize() const
{
    return 16;
}

// Acorn128 constants for ca and cb.
#define CA_0 ((uint32_t)0x00000000)
#define CA_1 ((uint32_t)0xFFFFFFFF)
#define CB_0 ((uint32_t)0x00000000)
#define CB_1 ((uint32_t)0xFFFFFFFF)
#define CA_0_BYTE ((uint8_t)0x00)
#define CA_1_BYTE ((uint8_t)0xFF)
#define CB_0_BYTE ((uint8_t)0x00)
#define CB_1_BYTE ((uint8_t)0xFF)

#if defined(CRYPTO_ACORN128_DEFAULT) || defined(CRYPTO_DOC)

// maj() and ch() functions for mixing the state.
#define maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define ch(x, y, z)  (((x) & (y)) ^ ((~(x)) & (z)))

/**
 * \brief Encrypts an 8-bit byte using Acorn128.
 *
 * \param state The state for the Acorn128 cipher.
 * \param plaintext The plaintext byte.
 * \param ca The ca constant.
 * \param cb The cb constant.
 *
 * \return The ciphertext byte.
 */
static uint8_t acornEncrypt8
    (Acorn128State *state, uint8_t plaintext, uint8_t ca, uint8_t cb)
{
    // Extract out various sub-parts of the state as 8-bit bytes.
    #define s_extract_8(name, shift) \
        ((uint8_t)(state->name##_l >> (shift)))
    uint8_t s244 = s_extract_8(s6, 14);
    uint8_t s235 = s_extract_8(s6, 5);
    uint8_t s196 = s_extract_8(s5, 3);
    uint8_t s160 = s_extract_8(s4, 6);
    uint8_t s111 = s_extract_8(s3, 4);
    uint8_t s66  = s_extract_8(s2, 5);
    uint8_t s23  = s_extract_8(s1, 23);
    uint8_t s12  = s_extract_8(s1, 12);

    // Update the LFSR's.
    uint8_t s7_l = state->s7 ^ s235 ^ state->s6_l;
    state->s6_l ^= s196 ^ ((uint8_t)(state->s5_l));
    state->s5_l ^= s160 ^ ((uint8_t)(state->s4_l));
    state->s4_l ^= s111 ^ ((uint8_t)(state->s3_l));
    state->s3_l ^= s66  ^ ((uint8_t)(state->s2_l));
    state->s2_l ^= s23  ^ ((uint8_t)(state->s1_l));

    // Generate the next 8 keystream bits.
    // k = S[12] ^ S[154] ^ maj(S[235], S[61], S[193])
    //                    ^  ch(S[230], S[111], S[66])
    uint8_t ks = s12 ^ state->s4_l ^
                 maj(s235, state->s2_l, state->s5_l) ^
                 ch(state->s6_l, s111, s66);

    // Generate the next 8 non-linear feedback bits.
    // f = S[0] ^ ~S[107] ^ maj(S[244], S[23], S[160])
    //                    ^ (ca & S[196]) ^ (cb & ks)
    // f ^= plaintext
    uint8_t f = state->s1_l ^ (~state->s3_l) ^
                maj(s244, s23, s160) ^ (ca & s196) ^ (cb & ks);
    f ^= plaintext;

    // Shift the state downwards by 8 bits.
    #define s_shift_8(name1, name2, shift) \
        (state->name1##_l = (state->name1##_l >> 8) | \
                            (((uint32_t)(state->name1##_h)) << 24), \
         state->name1##_h = (state->name1##_h >> 8) | \
                            ((state->name2##_l & 0xFF) << ((shift) - 40)))
    #define s_shift_8_mixed(name1, name2, shift) \
        (state->name1##_l = (state->name1##_l >> 8) | \
                            (((uint32_t)(state->name1##_h)) << 24) | \
                            (state->name2##_l << ((shift) - 8)), \
         state->name1##_h = ((state->name2##_l & 0xFF) >> (40 - (shift))))
    s7_l ^= (f << 4);
    state->s7 = f >> 4;
    s_shift_8(s1, s2, 61);
    s_shift_8(s2, s3, 46);
    s_shift_8(s3, s4, 47);
    s_shift_8_mixed(s4, s5, 39);
    s_shift_8_mixed(s5, s6, 37);
    state->s6_l = (state->s6_l >> 8) | (state->s6_h << 24);
    state->s6_h = (state->s6_h >> 8) | (((uint32_t)s7_l) << 19);

    // Return the ciphertext byte to the caller.
    return plaintext ^ ks;
}

/**
 * \brief Encrypts a 32-bit word using Acorn128.
 *
 * \param state The state for the Acorn128 cipher.
 * \param plaintext The plaintext word.
 * \param ca The ca constant.
 * \param cb The cb constant.
 *
 * \return The ciphertext word.
 */
static uint32_t acornEncrypt32
    (Acorn128State *state, uint32_t plaintext, uint32_t ca, uint32_t cb)
{
    // Extract out various sub-parts of the state as 32-bit words.
    #define s_extract_32(name, shift) \
        ((state->name##_l >> (shift)) | \
         (((uint32_t)(state->name##_h)) << (32 - (shift))))
    uint32_t s244 = s_extract_32(s6, 14);
    uint32_t s235 = s_extract_32(s6, 5);
    uint32_t s196 = s_extract_32(s5, 3);
    uint32_t s160 = s_extract_32(s4, 6);
    uint32_t s111 = s_extract_32(s3, 4);
    uint32_t s66  = s_extract_32(s2, 5);
    uint32_t s23  = s_extract_32(s1, 23);
    uint32_t s12  = s_extract_32(s1, 12);

    // Update the LFSR's.
    uint32_t s7_l = state->s7 ^ s235 ^ state->s6_l;
    state->s6_l ^= s196 ^ state->s5_l;
    state->s5_l ^= s160 ^ state->s4_l;
    state->s4_l ^= s111 ^ state->s3_l;
    state->s3_l ^= s66  ^ state->s2_l;
    state->s2_l ^= s23  ^ state->s1_l;

    // Generate the next 32 keystream bits.
    // k = S[12] ^ S[154] ^ maj(S[235], S[61], S[193])
    //                    ^  ch(S[230], S[111], S[66])
    uint32_t ks = s12 ^ state->s4_l ^
                  maj(s235, state->s2_l, state->s5_l) ^
                  ch(state->s6_l, s111, s66);

    // Generate the next 32 non-linear feedback bits.
    // f = S[0] ^ ~S[107] ^ maj(S[244], S[23], S[160])
    //                    ^ (ca & S[196]) ^ (cb & ks)
    // f ^= plaintext
    uint32_t f = state->s1_l ^ (~state->s3_l) ^
                 maj(s244, s23, s160) ^ (ca & s196) ^ (cb & ks);
    f ^= plaintext;

    // Shift the state downwards by 32 bits.
    #define s_shift_32(name1, name2, shift) \
        (state->name1##_l = state->name1##_h | (state->name2##_l << (shift)), \
         state->name1##_h = (state->name2##_l >> (32 - (shift))))
    s7_l ^= (f << 4);
    state->s7 = (uint8_t)(f >> 28);
    s_shift_32(s1, s2, 29);
    s_shift_32(s2, s3, 14);
    s_shift_32(s3, s4, 15);
    s_shift_32(s4, s5, 7);
    s_shift_32(s5, s6, 5);
    state->s6_l = state->s6_h | (s7_l << 27);
    state->s6_h = s7_l >> 5;

    // Return the ciphertext word to the caller.
    return plaintext ^ ks;
}

/**
 * \brief Encrypts a 32-bit word using Acorn128.
 *
 * \param state The state for the Acorn128 cipher.
 * \param plaintext The plaintext word.
 *
 * \return The ciphertext word.
 *
 * This version assumes that ca = 1 and cb = 0.
 */
static inline uint32_t acornEncrypt32Fast
    (Acorn128State *state, uint32_t plaintext)
{
    // Extract out various sub-parts of the state as 32-bit words.
    #define s_extract_32(name, shift) \
        ((state->name##_l >> (shift)) | \
         (((uint32_t)(state->name##_h)) << (32 - (shift))))
    uint32_t s244 = s_extract_32(s6, 14);
    uint32_t s235 = s_extract_32(s6, 5);
    uint32_t s196 = s_extract_32(s5, 3);
    uint32_t s160 = s_extract_32(s4, 6);
    uint32_t s111 = s_extract_32(s3, 4);
    uint32_t s66  = s_extract_32(s2, 5);
    uint32_t s23  = s_extract_32(s1, 23);
    uint32_t s12  = s_extract_32(s1, 12);

    // Update the LFSR's.
    uint32_t s7_l = state->s7 ^ s235 ^ state->s6_l;
    state->s6_l ^= s196 ^ state->s5_l;
    state->s5_l ^= s160 ^ state->s4_l;
    state->s4_l ^= s111 ^ state->s3_l;
    state->s3_l ^= s66  ^ state->s2_l;
    state->s2_l ^= s23  ^ state->s1_l;

    // Generate the next 32 keystream bits.
    // k = S[12] ^ S[154] ^ maj(S[235], S[61], S[193])
    //                    ^  ch(S[230], S[111], S[66])
    uint32_t ks = s12 ^ state->s4_l ^
                  maj(s235, state->s2_l, state->s5_l) ^
                  ch(state->s6_l, s111, s66);

    // Generate the next 32 non-linear feedback bits.
    // f = S[0] ^ ~S[107] ^ maj(S[244], S[23], S[160])
    //                    ^ (ca & S[196]) ^ (cb & ks)
    // f ^= plaintext
    // Note: ca will always be 1 and cb will always be 0.
    uint32_t f = state->s1_l ^ (~state->s3_l) ^ maj(s244, s23, s160) ^ s196;
    f ^= plaintext;

    // Shift the state downwards by 32 bits.
    #define s_shift_32(name1, name2, shift) \
        (state->name1##_l = state->name1##_h | (state->name2##_l << (shift)), \
         state->name1##_h = (state->name2##_l >> (32 - (shift))))
    s7_l ^= (f << 4);
    state->s7 = (uint8_t)(f >> 28);
    s_shift_32(s1, s2, 29);
    s_shift_32(s2, s3, 14);
    s_shift_32(s3, s4, 15);
    s_shift_32(s4, s5, 7);
    s_shift_32(s5, s6, 5);
    state->s6_l = state->s6_h | (s7_l << 27);
    state->s6_h = s7_l >> 5;

    // Return the ciphertext word to the caller.
    return plaintext ^ ks;
}

/**
 * \brief Decrypts an 8-bit byte using Acorn128.
 *
 * \param state The state for the Acorn128 cipher.
 * \param ciphertext The ciphertext byte.
 *
 * \return The plaintext byte.
 */
static inline uint8_t acornDecrypt8(Acorn128State *state, uint8_t ciphertext)
{
    // Extract out various sub-parts of the state as 8-bit bytes.
    #define s_extract_8(name, shift) \
        ((uint8_t)(state->name##_l >> (shift)))
    uint8_t s244 = s_extract_8(s6, 14);
    uint8_t s235 = s_extract_8(s6, 5);
    uint8_t s196 = s_extract_8(s5, 3);
    uint8_t s160 = s_extract_8(s4, 6);
    uint8_t s111 = s_extract_8(s3, 4);
    uint8_t s66  = s_extract_8(s2, 5);
    uint8_t s23  = s_extract_8(s1, 23);
    uint8_t s12  = s_extract_8(s1, 12);

    // Update the LFSR's.
    uint8_t s7_l = state->s7 ^ s235 ^ state->s6_l;
    state->s6_l ^= s196 ^ ((uint8_t)(state->s5_l));
    state->s5_l ^= s160 ^ ((uint8_t)(state->s4_l));
    state->s4_l ^= s111 ^ ((uint8_t)(state->s3_l));
    state->s3_l ^= s66  ^ ((uint8_t)(state->s2_l));
    state->s2_l ^= s23  ^ ((uint8_t)(state->s1_l));

    // Generate the next 8 keystream bits and decrypt the ciphertext.
    // k = S[12] ^ S[154] ^ maj(S[235], S[61], S[193])
    //                    ^  ch(S[230], S[111], S[66])
    uint8_t ks = s12 ^ state->s4_l ^
                 maj(s235, state->s2_l, state->s5_l) ^
                 ch(state->s6_l, s111, s66);
    uint8_t plaintext = ciphertext ^ ks;

    // Generate the next 8 non-linear feedback bits.
    // f = S[0] ^ ~S[107] ^ maj(S[244], S[23], S[160])
    //                    ^ (ca & S[196]) ^ (cb & ks)
    // f ^= plaintext
    // Note: ca will always be 1 and cb will always be 0.
    uint8_t f = state->s1_l ^ (~state->s3_l) ^ maj(s244, s23, s160) ^ s196;
    f ^= plaintext;

    // Shift the state downwards by 8 bits.
    #define s_shift_8(name1, name2, shift) \
        (state->name1##_l = (state->name1##_l >> 8) | \
                            (((uint32_t)(state->name1##_h)) << 24), \
         state->name1##_h = (state->name1##_h >> 8) | \
                            ((state->name2##_l & 0xFF) << ((shift) - 40)))
    #define s_shift_8_mixed(name1, name2, shift) \
        (state->name1##_l = (state->name1##_l >> 8) | \
                            (((uint32_t)(state->name1##_h)) << 24) | \
                            (state->name2##_l << ((shift) - 8)), \
         state->name1##_h = ((state->name2##_l & 0xFF) >> (40 - (shift))))
    s7_l ^= (f << 4);
    state->s7 = f >> 4;
    s_shift_8(s1, s2, 61);
    s_shift_8(s2, s3, 46);
    s_shift_8(s3, s4, 47);
    s_shift_8_mixed(s4, s5, 39);
    s_shift_8_mixed(s5, s6, 37);
    state->s6_l = (state->s6_l >> 8) | (state->s6_h << 24);
    state->s6_h = (state->s6_h >> 8) | (((uint32_t)s7_l) << 19);

    // Return the plaintext byte to the caller.
    return plaintext;
}

/**
 * \brief Decrypts a 32-bit word using Acorn128.
 *
 * \param state The state for the Acorn128 cipher.
 * \param ciphertext The ciphertext word.
 *
 * \return The plaintext word.
 */
static inline uint32_t acornDecrypt32(Acorn128State *state, uint32_t ciphertext)
{
    // Extract out various sub-parts of the state as 32-bit words.
    #define s_extract_32(name, shift) \
        ((state->name##_l >> (shift)) | \
         (((uint32_t)(state->name##_h)) << (32 - (shift))))
    uint32_t s244 = s_extract_32(s6, 14);
    uint32_t s235 = s_extract_32(s6, 5);
    uint32_t s196 = s_extract_32(s5, 3);
    uint32_t s160 = s_extract_32(s4, 6);
    uint32_t s111 = s_extract_32(s3, 4);
    uint32_t s66  = s_extract_32(s2, 5);
    uint32_t s23  = s_extract_32(s1, 23);
    uint32_t s12  = s_extract_32(s1, 12);

    // Update the LFSR's.
    uint32_t s7_l = state->s7 ^ s235 ^ state->s6_l;
    state->s6_l ^= s196 ^ state->s5_l;
    state->s5_l ^= s160 ^ state->s4_l;
    state->s4_l ^= s111 ^ state->s3_l;
    state->s3_l ^= s66  ^ state->s2_l;
    state->s2_l ^= s23  ^ state->s1_l;

    // Generate the next 32 keystream bits and decrypt the ciphertext.
    // k = S[12] ^ S[154] ^ maj(S[235], S[61], S[193])
    //                    ^  ch(S[230], S[111], S[66])
    uint32_t ks = s12 ^ state->s4_l ^
                  maj(s235, state->s2_l, state->s5_l) ^
                  ch(state->s6_l, s111, s66);
    uint32_t plaintext = ciphertext ^ ks;

    // Generate the next 32 non-linear feedback bits.
    // f = S[0] ^ ~S[107] ^ maj(S[244], S[23], S[160])
    //                    ^ (ca & S[196]) ^ (cb & ks)
    // f ^= plaintext
    // Note: ca will always be 1 and cb will always be 0.
    uint32_t f = state->s1_l ^ (~state->s3_l) ^ maj(s244, s23, s160) ^ s196;
    f ^= plaintext;

    // Shift the state downwards by 32 bits.
    #define s_shift_32(name1, name2, shift) \
        (state->name1##_l = state->name1##_h | (state->name2##_l << (shift)), \
         state->name1##_h = (state->name2##_l >> (32 - (shift))))
    s7_l ^= (f << 4);
    state->s7 = (uint8_t)(f >> 28);
    s_shift_32(s1, s2, 29);
    s_shift_32(s2, s3, 14);
    s_shift_32(s3, s4, 15);
    s_shift_32(s4, s5, 7);
    s_shift_32(s5, s6, 5);
    state->s6_l = state->s6_h | (s7_l << 27);
    state->s6_h = s7_l >> 5;

    // Return the plaintext word to the caller.
    return plaintext;
}

#elif defined(CRYPTO_ACORN128_AVR)

// Import definitions from Acorn128AVR.cpp
extern uint32_t acornEncrypt32
    (Acorn128State *state, uint32_t plaintext, uint32_t ca, uint32_t cb);

#endif // CRYPTO_ACORN128_AVR

/**
 * \brief Adds 256 bits of padding to the Acorn128 state.
 *
 * \param state The state for the Acorn128 cipher.
 * \param cb The cb constant for the padding block.
 */
void acornPad(Acorn128State *state, uint32_t cb)
{
    acornEncrypt32(state, 1, CA_1, cb);
    acornEncrypt32(state, 0, CA_1, cb);
    acornEncrypt32(state, 0, CA_1, cb);
    acornEncrypt32(state, 0, CA_1, cb);
    acornEncrypt32(state, 0, CA_0, cb);
    acornEncrypt32(state, 0, CA_0, cb);
    acornEncrypt32(state, 0, CA_0, cb);
    acornEncrypt32(state, 0, CA_0, cb);
}

bool Acorn128::setKey(const uint8_t *key, size_t len)
{
    // We cannot initialize the key block until we also have the IV.
    // So we simply validate the length and save the key away for later.
    if (len == 16) {
        memcpy(state.k, key, 16);
#if !defined(CRYPTO_LITTLE_ENDIAN)
        state.k[0] = le32toh(state.k[0]);
        state.k[1] = le32toh(state.k[1]);
        state.k[2] = le32toh(state.k[2]);
        state.k[3] = le32toh(state.k[3]);
#endif
        return true;
    } else {
        return false;
    }
}

bool Acorn128::setIV(const uint8_t *iv, size_t len)
{
    if (len != 16)
        return false;

    // Unpack the iv into four 32-bit words.
    uint32_t ivwords[4];
    memcpy(ivwords, iv, 16);
#if !defined(CRYPTO_LITTLE_ENDIAN)
    ivwords[0] = le32toh(ivwords[0]);
    ivwords[1] = le32toh(ivwords[1]);
    ivwords[2] = le32toh(ivwords[2]);
    ivwords[3] = le32toh(ivwords[3]);
#endif

    // Initialize the state to zero.
    state.s1_l = 0;
    state.s1_h = 0;
    state.s2_l = 0;
    state.s2_h = 0;
    state.s3_h = 0;
    state.s3_l = 0;
    state.s4_l = 0;
    state.s4_h = 0;
    state.s5_h = 0;
    state.s5_l = 0;
    state.s6_l = 0;
    state.s6_h = 0;
    state.s7 = 0;
    state.authDone = 0;

    // Run the cipher for 1792 steps, 32 at a time,
    // which mixes the key and IV into the cipher state.
    acornEncrypt32(&state, state.k[0], CA_1, CB_1);
    acornEncrypt32(&state, state.k[1], CA_1, CB_1);
    acornEncrypt32(&state, state.k[2], CA_1, CB_1);
    acornEncrypt32(&state, state.k[3], CA_1, CB_1);
    acornEncrypt32(&state, ivwords[0], CA_1, CB_1);
    acornEncrypt32(&state, ivwords[1], CA_1, CB_1);
    acornEncrypt32(&state, ivwords[2], CA_1, CB_1);
    acornEncrypt32(&state, ivwords[3], CA_1, CB_1);
    acornEncrypt32(&state, state.k[0] ^ 0x00000001, CA_1, CB_1);
    acornEncrypt32(&state, state.k[1], CA_1, CB_1);
    acornEncrypt32(&state, state.k[2], CA_1, CB_1);
    acornEncrypt32(&state, state.k[3], CA_1, CB_1);
    for (uint8_t i = 0; i < 11; ++i) {
        acornEncrypt32(&state, state.k[0], CA_1, CB_1);
        acornEncrypt32(&state, state.k[1], CA_1, CB_1);
        acornEncrypt32(&state, state.k[2], CA_1, CB_1);
        acornEncrypt32(&state, state.k[3], CA_1, CB_1);
    }

    // Clean up and exit.
    clean(ivwords);
    return true;
}

#if defined(CRYPTO_ACORN128_DEFAULT) || defined(CRYPTO_DOC)

void Acorn128::encrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    if (!state.authDone) {
        acornPad(&state, CB_1);
        state.authDone = 1;
    }
    while (len >= 4) {
        uint32_t temp = ((uint32_t)input[0]) |
                       (((uint32_t)input[1]) << 8) |
                       (((uint32_t)input[2]) << 16) |
                       (((uint32_t)input[3]) << 24);
        temp = acornEncrypt32Fast(&state, temp);
        output[0] = (uint8_t)temp;
        output[1] = (uint8_t)(temp >> 8);
        output[2] = (uint8_t)(temp >> 16);
        output[3] = (uint8_t)(temp >> 24);
        input += 4;
        output += 4;
        len -= 4;
    }
    while (len > 0) {
        *output++ = acornEncrypt8(&state, *input++, CA_1_BYTE, CB_0_BYTE);
        --len;
    }
}

void Acorn128::decrypt(uint8_t *output, const uint8_t *input, size_t len)
{
    if (!state.authDone) {
        acornPad(&state, CB_1);
        state.authDone = 1;
    }
    while (len >= 4) {
        uint32_t temp = ((uint32_t)input[0]) |
                       (((uint32_t)input[1]) << 8) |
                       (((uint32_t)input[2]) << 16) |
                       (((uint32_t)input[3]) << 24);
        temp = acornDecrypt32(&state, temp);
        output[0] = (uint8_t)temp;
        output[1] = (uint8_t)(temp >> 8);
        output[2] = (uint8_t)(temp >> 16);
        output[3] = (uint8_t)(temp >> 24);
        input += 4;
        output += 4;
        len -= 4;
    }
    while (len > 0) {
        *output++ = acornDecrypt8(&state, *input++);
        --len;
    }
}

void Acorn128::addAuthData(const void *data, size_t len)
{
    // Cannot add any more auth data if we've started to encrypt or decrypt.
    if (state.authDone)
        return;

    // Encrypt the auth data with ca = 1, cb = 1.
    const uint8_t *input = (const uint8_t *)data;
    while (len >= 4) {
        uint32_t temp = ((uint32_t)input[0]) |
                       (((uint32_t)input[1]) << 8) |
                       (((uint32_t)input[2]) << 16) |
                       (((uint32_t)input[3]) << 24);
        acornEncrypt32(&state, temp, CA_1, CB_1);
        input += 4;
        len -= 4;
    }
    while (len > 0) {
        acornEncrypt8(&state, *input++, CA_1_BYTE, CB_1_BYTE);
        --len;
    }
}

#endif // CRYPTO_ACORN128_DEFAULT

void Acorn128::computeTag(void *tag, size_t len)
{
    // Finalize the data and apply padding.
    if (!state.authDone)
        acornPad(&state, CB_1);
    acornPad(&state, CB_0);

    // Encrypt 768 zero bits and extract the last 128 for the tag.
    uint32_t temp[4];
    for (uint8_t i = 0; i < 20; ++i)
        acornEncrypt32(&state, 0, CA_1, CB_1);
    temp[0] = acornEncrypt32(&state, 0, CA_1, CB_1);
    temp[1] = acornEncrypt32(&state, 0, CA_1, CB_1);
    temp[2] = acornEncrypt32(&state, 0, CA_1, CB_1);
    temp[3] = acornEncrypt32(&state, 0, CA_1, CB_1);
#if !defined(CRYPTO_LITTLE_ENDIAN)
    temp[0] = htole32(temp[0]);
    temp[1] = htole32(temp[1]);
    temp[2] = htole32(temp[2]);
    temp[3] = htole32(temp[3]);
#endif

    // Truncate to the requested length and return the value.
    if (len > 16)
        len = 16;
    memcpy(tag, temp, len);
    clean(temp);
}

bool Acorn128::checkTag(const void *tag, size_t len)
{
    // Can never match if the expected tag length is too long.
    if (len > 16)
        return false;

    // Compute the authentication tag and check it.
    uint8_t temp[16];
    computeTag(temp, len);
    bool equal = secure_compare(temp, tag, len);
    clean(temp);
    return equal;
}

/**
 * \brief Clears all security-sensitive state from this cipher object.
 */
void Acorn128::clear()
{
    clean(state);
}
