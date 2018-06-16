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

#include "KeyRing.h"
#include "Crypto.h"
#include "SHA256.h"
#include <Arduino.h>
#include <string.h>

/**
 * \class KeyRingClass KeyRing.h <KeyRing.h>
 * \brief Permanent storage for key material.
 *
 * This class provides permanent storage for local key pairs, remote public
 * keys, shared symmetric keys, and other data on a device.  Other data may
 * be certificates, configuration data, or any other information that
 * isn't strictly key material.
 *
 * As an example, the following code will generate a new Curve25519
 * key pair and save it into permanent storage as the default key pair
 * for the device:
 *
 * \code
 * uint8_t keyPair[64];
 * Curve25519::generateKeyPair(keyPair);
 * KeyRing.setLocalKeyPair(KeyRing::LocalCurve25519Default, keyPair, 64);
 * \endcode
 *
 * Later, when the application needs the key pair it does the following:
 *
 * \code
 * uint8_t keyPair[64];
 * if (!KeyRing.getLocalKeyPair(KeyRing::LocalCurve25519Default, keyPair, 64)) {
 *     Serial.println("Curve25519 key pair not found!");
 * } else {
 *     // Make use of the key pair.
 *     ...
 * }
 * \endcode
 *
 * By default, the key ring data is not encrypted in the permanent storage,
 * so the data may be compromised if the device is captured.  The application
 * can activate encryption by passing a passphrase or key to begin():
 *
 * \code
 * // Initialize the key ring with a passphrase.
 * KeyRing.begin("Hello There!");
 *
 * // Initialize the key ring with a binary key value.
 * uint8_t key[32] = {...};
 * KeyRing.begin(key, sizeof(key));
 * \endcode
 *
 * The data is not encrypted by default because it creates a chicken-and-egg
 * problem: where is the encryption key itself stored if the device does
 * not have some way to enter a passphrase at runtime?  The application can
 * call begin() if it has some way to manage this.
 *
 * When encryption is in use, local key pairs and shared symmetric keys
 * are encrypted but remote public keys and other data remain in the clear.
 * If the wrong encryption key/passphrase is supplied, then local key
 * pairs and shared symmetric keys will decrypt to rubbish.
 *
 * There is no mechanism to determine if the key/passphrase is incorrect.
 * If the application cares about this, it can hash the plaintext value and
 * store both the value and the hash in permanent storage.  If the hash is
 * correct after decryption, then the correct key was supplied.
 *
 * The key ring uses the tweakable block cipher
 * <a href="https://eprint.iacr.org/2016/660.pdf">Mantis-8</a>
 * with a 128-bit key to encrypt the contents of permanent storage.
 * The application-supplied passphrase or key is hashed down to 128 bits
 * using SHA256 when begin() is called.
 *
 * The amount of key data that can be stored is limited by the size of
 * the permanent storage mechanism on the device:
 *
 * \li AVR devices will use no more than 3/4 of the available EEPROM
 * space, starting at the end of EEPROM and working backwards.  For example,
 * the 1K of EEPROM on the Arduino Uno has space for twenty 32-byte keys or
 * ten 64-byte keys.  256 bytes at the start of EEPROM will be reserved
 * for other application use.
 * \li ESP8266 and ESP32 allocate up to 3K of the simulated EEPROM space,
 * starting at the end of 4K of simulated EEPROM and working backwards.
 * This can hold just over eighty 32-byte keys or forty 64-byte keys.
 * The first 1K of simulated EEPROM is reserved for other application use.
 * \li Arduino Due allocates up to 4K of flash memory for key data, which
 * can hold just over one hundred 32-byte keys or fifty 64-byte keys.
 *
 * If a device is not currently supported, then requests to store values
 * in the key ring will fail.  Requests to retrieve values from the key
 * ring will return nothing.
 *
 * \note The key identifier zero is reserved and cannot be used to
 * store values in the key ring.
 */

#if defined(__AVR__)

// Use EEPROM on AVR devices to store the key data.  We leave some
// room at the end for the saved RNG seed.
#include "RNG.h"
#include <avr/eeprom.h>
#define KEY_RING_AVR_EEPROM 1
#define KEY_RING_EEPROM_SIZE (E2END + 1)
#define KEY_RING_EEPROM_START (KEY_RING_EEPROM_SIZE / 4)
#define KEY_RING_EEPROM_END (KEY_RING_EEPROM_SIZE - RNGClass::SEED_SIZE)

#elif defined(ESP32) || defined(ESP8266)

// ESP32 and ESP8266 simulate EEPROM in the standard EEPROM class.
// Build our key ring storage mechanism on top of that simulation.
#include <EEPROM.h>
#if defined(ESP8266)
extern "C" {
#include "spi_flash.h" // For the definition of SPI_FLASH_SEC_SIZE.
}
#endif
#define KEY_RING_SIMUL_EEPROM 1
#define KEY_RING_EEPROM_SIZE SPI_FLASH_SEC_SIZE
#define KEY_RING_EEPROM_START (SPI_FLASH_SEC_SIZE / 4)
#define KEY_RING_EEPROM_END SPI_FLASH_SEC_SIZE

#elif defined (__arm__) && defined (__SAM3X8E__)

// Store keys at the end of flash memory on the Arduino Due.
#include "utility/SamFlashUtil.h"
#define KEY_RING_DUE                1
#define KEY_RING_FLASH              1
#define KEY_RING_CHUNKS_PER_PAGE    (SAM_FLASH_PAGE_SIZE / ChunkSize)

#elif defined(HOST_BUILD)

// Simulate 4K of EEPROM on the host build in a memory buffer.
#define KEY_RING_AVR_EEPROM 1
#define KEY_RING_EEPROM_SIZE 4096
#define KEY_RING_EEPROM_START ChunkSize // Must be >= ChunkSize.
#define KEY_RING_EEPROM_END KEY_RING_EEPROM_SIZE
static uint8_t key_ring_eeprom[KEY_RING_EEPROM_SIZE];
#define eeprom_read_block(dst,src,size) \
    memcpy((dst), key_ring_eeprom + (uintptr_t)(src), (size))
#define eeprom_write_block(src,dst,size) \
    memcpy(key_ring_eeprom + (uintptr_t)(dst), (src), (size))

#else

#warning "KeyRing is not ported to this platform yet"

#endif

/**
 * \brief Global key ring instance.
 *
 * \sa KeyRingClass
 */
KeyRingClass KeyRing;

/**
 * \var KeyRingClass::LocalCurve25519Default
 * \brief Identifier for the default local Curve25519 key pair on the device.
 */

/**
 * \var KeyRingClass::RemoteCurve25519Default
 * \brief Identifier for the default remote Curve25519 public key for the
 * primary remote device that this device will be communicating with.
 */

/**
 * \var KeyRingClass::LocalEd25519Default
 * \brief Identifier for the default local Ed25519 key pair on the device.
 */

/**
 * \var KeyRingClass::RemoteEd25519Default
 * \brief Identifier for the default remote Ed25519 public key for the
 * primary remote device that this device will be communicating with.
 */

/** @cond mantis8 */

// 32-bit version of Mantis-8, extracted from the Skinny-C repository:
// https://github.com/rweather/skinny-c

// Extract the 32 bits for a row from a 64-bit round constant.
#define RC_EXTRACT_ROW(x,shift) \
    (((((uint32_t)((x) >> ((shift) + 24))) & 0xFF)) | \
     ((((uint32_t)((x) >> ((shift) + 16))) & 0xFF) <<  8) | \
     ((((uint32_t)((x) >> ((shift) +  8))) & 0xFF) << 16) | \
     ((((uint32_t)((x) >> ((shift))))      & 0xFF) << 24))

// Alpha constant for adjusting k1 for the inverse rounds.
#define ALPHA      0x243F6A8885A308D3ULL
#define ALPHA_ROW0 (RC_EXTRACT_ROW(ALPHA, 32))
#define ALPHA_ROW1 (RC_EXTRACT_ROW(ALPHA, 0))

// Extract the rows from a 64-bit round constant.
#define RC(x) {RC_EXTRACT_ROW((x), 32), RC_EXTRACT_ROW((x), 0)}

// Round constants for Mantis, split up into 32-bit row values.
static uint32_t const rc[8][2] = {
    RC(0x13198A2E03707344ULL),
    RC(0xA4093822299F31D0ULL),
    RC(0x082EFA98EC4E6C89ULL),
    RC(0x452821E638D01377ULL),
    RC(0xBE5466CF34E90C6CULL),
    RC(0xC0AC29B7C97C50DDULL),
    RC(0x3F84D5B5B5470917ULL),
    RC(0x9216D5D98979FB1BULL)
};

static void mantis_unpack_block(uint32_t *block, const uint8_t *buf)
{
    block[0] = ((uint32_t)(buf[0])) |
              (((uint32_t)(buf[1])) << 8) |
              (((uint32_t)(buf[2])) << 16) |
              (((uint32_t)(buf[3])) << 24);
    block[1] = ((uint32_t)(buf[4])) |
              (((uint32_t)(buf[5])) << 8) |
              (((uint32_t)(buf[6])) << 16) |
              (((uint32_t)(buf[7])) << 24);
}

static void mantis_unpack_rotated_block(uint32_t *block, const uint8_t *buf)
{
    uint8_t rotated[8];
    uint8_t index;
    uint8_t next;
    uint8_t carry = buf[7];
    for (index = 0; index < 8; ++index) {
        next = buf[index];
        rotated[index] = (carry << 7) | (next >> 1);
        carry = next;
    }
    rotated[7] ^= (buf[0] >> 7);
    mantis_unpack_block(block, rotated);
    clean(rotated);
}

typedef union
{
    uint16_t row[4];
    uint32_t lrow[2];

} MantisCells_t;

static uint32_t mantis_sbox(uint32_t d)
{
    /*
     * MIDORI Sb0 from section 4.2 of https://eprint.iacr.org/2015/1142.pdf
     *
     * {a, b, c, d} -> {aout, bout, cout, dout} where a/aout is the MSB.
     *
     * aout = NAND(NAND(~c, NAND(a, b)), (a | d))
     * bout = NAND(NOR(NOR(a, d), (b & c)), NAND((a & c), d))
     * cout = NAND(NAND(b, d), (NOR(b, d) | a))
     * dout = NOR(NOR(a, (b | c)), NAND(NAND(a, b), (c | d)))
     */
    uint32_t a = (d >> 3);
    uint32_t b = (d >> 2);
    uint32_t c = (d >> 1);
    uint32_t not_a = ~a;
    uint32_t ab = not_a | (~b);
    uint32_t ad = not_a & (~d);
    uint32_t aout = (((~c) & ab) | ad);
    uint32_t bout = ad | (b & c) | (a & c & d);
    uint32_t cout = (b & d) | ((b | d) & not_a);
    uint32_t dout = (a | b | c) & ab & (c | d);
    return ((aout & 0x11111111U) << 3) | ((bout & 0x11111111U) << 2) |
           ((cout & 0x11111111U) << 1) |  (dout & 0x11111111U);
}

static void mantis_update_tweak(MantisCells_t *tweak)
{
    /* h = [6, 5, 14, 15, 0, 1, 2, 3, 7, 12, 13, 4, 8, 9, 10, 11] */
    uint16_t row1 = tweak->row[1];
    uint16_t row3 = tweak->row[3];
    tweak->row[1] = tweak->row[0];
    tweak->row[3] = tweak->row[2];
    tweak->row[0] = ((row1 >>  8) & 0x00F0U) |
                     (row1        & 0x000FU) |
                     (row3        & 0xFF00U);
    tweak->row[2] = ((row1 <<  4) & 0x0F00U) |
                    ((row1 >>  4) & 0x00F0U) |
                    ((row3 >>  4) & 0x000FU) |
                    ((row3 << 12) & 0xF000U);
}

static void mantis_update_tweak_inverse(MantisCells_t *tweak)
{
    /* h' = [4, 5, 6, 7, 11, 1, 0, 8, 12, 13, 14, 15, 9, 10, 2, 3] */
    uint16_t row0 = tweak->row[0];
    uint16_t row2 = tweak->row[2];
    tweak->row[0] = tweak->row[1];
    tweak->row[2] = tweak->row[3];
    tweak->row[1] = ((row2 >>  4) & 0x00F0U) |
                    ((row2 <<  4) & 0x0F00U) |
                     (row0        & 0x000FU) |
                    ((row0 <<  8) & 0xF000U);
    tweak->row[3] =  (row0        & 0xFF00U) |
                    ((row2 <<  4) & 0x00F0U) |
                    ((row2 >> 12) & 0x000FU);
}

static void mantis_shift_rows(MantisCells_t *state)
{
    /* P = [0, 11, 6, 13, 10, 1, 12, 7, 5, 14, 3, 8, 15, 4, 9, 2] */
    uint16_t row0 = state->row[0];
    uint16_t row1 = state->row[1];
    uint16_t row2 = state->row[2];
    uint16_t row3 = state->row[3];
    state->row[0] =  (row0        & 0x00F0U) |
                     (row1        & 0xF000U) |
                    ((row2 >>  8) & 0x000FU) |
                    ((row3 <<  8) & 0x0F00U);
    state->row[1] =  (row0        & 0x000FU) |
                     (row1        & 0x0F00U) |
                    ((row2 >>  8) & 0x00F0U) |
                    ((row3 <<  8) & 0xF000U);
    state->row[2] = ((row0 <<  4) & 0xF000U) |
                    ((row1 <<  4) & 0x00F0U) |
                    ((row2 <<  4) & 0x0F00U) |
                    ((row3 >> 12) & 0x000FU);
    state->row[3] = ((row0 >>  4) & 0x0F00U) |
                    ((row1 >>  4) & 0x000FU) |
                    ((row2 << 12) & 0xF000U) |
                    ((row3 >>  4) & 0x00F0U);
}

static void mantis_shift_rows_inverse(MantisCells_t *state)
{
    /* P' = [0, 5, 15, 10, 13, 8, 2, 7, 11, 14, 4, 1, 6, 3, 9, 12] */
    uint16_t row0 = state->row[0];
    uint16_t row1 = state->row[1];
    uint16_t row2 = state->row[2];
    uint16_t row3 = state->row[3];
    state->row[0] =  (row0        & 0x00F0U) |
                     (row1        & 0x000FU) |
                    ((row2 >>  4) & 0x0F00U) |
                    ((row3 <<  4) & 0xF000U);
    state->row[1] =  (row0        & 0xF000U) |
                     (row1        & 0x0F00U) |
                    ((row2 >>  4) & 0x000FU) |
                    ((row3 <<  4) & 0x00F0U);
    state->row[2] = ((row0 <<  8) & 0x0F00U) |
                    ((row1 <<  8) & 0xF000U) |
                    ((row2 >>  4) & 0x00F0U) |
                    ((row3 >> 12) & 0x000FU);
    state->row[3] = ((row0 >>  8) & 0x000FU) |
                    ((row1 >>  8) & 0x00F0U) |
                    ((row2 << 12) & 0xF000U) |
                    ((row3 <<  4) & 0x0F00U);
}

inline void mantis_mix_columns(MantisCells_t *state)
{
    uint16_t t0 = state->row[0];
    uint16_t t1 = state->row[1];
    uint16_t t2 = state->row[2];
    uint16_t t3 = state->row[3];
    state->row[0] = t1 ^ t2 ^ t3;
    state->row[1] = t0 ^ t2 ^ t3;
    state->row[2] = t0 ^ t1 ^ t3;
    state->row[3] = t0 ^ t1 ^ t2;
}

/**
 * \brief Encrypts or decrypts a block with Mantis-8.
 *
 * \param t The tweak for the block.
 * \param ks The key schedule to use, consisting of k0, k1, and k0prime.
 * \param block The block to be processed, both input and output.
 *
 * Reference: https://eprint.iacr.org/2016/660.pdf
 */
static void mantis8(const uint32_t t[2], uint32_t ks[6], uint8_t block[8])
{
    const uint32_t *r = rc[0];
    MantisCells_t tweak;
    MantisCells_t state;
    uint8_t index;

    // Copy the initial tweak value into local variables.
    tweak.lrow[0] = t[0];
    tweak.lrow[1] = t[1];

    // Read the input block and convert little-endian to host-endian.
    mantis_unpack_block(state.lrow, block);

    // XOR the initial whitening key k0 with the state,
    // together with k1 and the initial tweak value.
    state.lrow[0] ^= ks[0] ^ ks[2] ^ tweak.lrow[0];
    state.lrow[1] ^= ks[1] ^ ks[3] ^ tweak.lrow[1];

    // Perform all eight forward rounds.
    for (index = 8; index > 0; --index) {
        // Update the tweak with the forward h function.
        mantis_update_tweak(&tweak);

        // Apply the S-box.
        state.lrow[0] = mantis_sbox(state.lrow[0]);
        state.lrow[1] = mantis_sbox(state.lrow[1]);

        // Add the round constant.
        state.lrow[0] ^= r[0];
        state.lrow[1] ^= r[1];
        r += 2;

        // XOR with the key and tweak.
        state.lrow[0] ^= ks[2] ^ tweak.lrow[0];
        state.lrow[1] ^= ks[3] ^ tweak.lrow[1];

        // Shift the rows.
        mantis_shift_rows(&state);

        // Mix the columns.
        mantis_mix_columns(&state);
    }

    // Half-way there: sbox, mix, sbox.
    state.lrow[0] = mantis_sbox(state.lrow[0]);
    state.lrow[1] = mantis_sbox(state.lrow[1]);
    mantis_mix_columns(&state);
    state.lrow[0] = mantis_sbox(state.lrow[0]);
    state.lrow[1] = mantis_sbox(state.lrow[1]);

    // Convert k1 into k1 XOR alpha for the reverse rounds.
    ks[2] ^= ALPHA_ROW0;
    ks[3] ^= ALPHA_ROW1;

    // Perform all eight reverse rounds.
    for (index = 8; index > 0; --index) {
        // Inverse mix of the columns (same as the forward mix).
        mantis_mix_columns(&state);

        // Inverse shift of the rows.
        mantis_shift_rows_inverse(&state);

        /* XOR with the key and tweak */
        state.lrow[0] ^= ks[2] ^ tweak.lrow[0];
        state.lrow[1] ^= ks[3] ^ tweak.lrow[1];

        // Add the round constant.
        r -= 2;
        state.lrow[0] ^= r[0];
        state.lrow[1] ^= r[1];

        // Apply the inverse S-box (which is the same as the forward S-box).
        state.lrow[0] = mantis_sbox(state.lrow[0]);
        state.lrow[1] = mantis_sbox(state.lrow[1]);

        // Update the tweak with the reverse h function.
        mantis_update_tweak_inverse(&tweak);
    }

    // XOR the final whitening key k0prime with the state,
    // together with k1alpha and the final tweak value.
    state.lrow[0] ^= ks[4] ^ ks[2] ^ tweak.lrow[0];
    state.lrow[1] ^= ks[5] ^ ks[3] ^ tweak.lrow[1];

    // Restore k1 to its original condition.
    ks[2] ^= ALPHA_ROW0;
    ks[3] ^= ALPHA_ROW1;

    // Convert host-endian back into little-endian in the output block.
    uint32_t x = state.lrow[0];
    block[0] = (uint8_t)x;
    block[1] = (uint8_t)(x >> 8);
    block[2] = (uint8_t)(x >> 16);
    block[3] = (uint8_t)(x >> 24);
    x = state.lrow[1];
    block[4] = (uint8_t)x;
    block[5] = (uint8_t)(x >> 8);
    block[6] = (uint8_t)(x >> 16);
    block[7] = (uint8_t)(x >> 24);
}

/** @endcond */

/**
 * \brief Encrypts or decrypts a chunk with Mantis-8.
 *
 * \param k The 128-bit encryption key for Mantis-8.
 * \param chunk The chunk to be encrypted or decrypted.
 * \param posn The position within EEPROM/Flash where the chunk is stored.
 * \param encrypt Set to true to encrypt or false to decrypt.
 */
static void cryptChunk
    (uint8_t k[16], uint8_t chunk[36], unsigned posn, bool encrypt)
{
    // Nothing to do if the chunk identifier is zero (erased chunk).
    if (chunk[0] == 0 && chunk[1] == 0)
        return;

    // We only encrypt local key pairs (type 0) and symmetric keys (type 2).
    if ((chunk[2] & 0x20) != 0)
        return;

    // Construct the 64-bit tweak value using the id, type, and position
    // to bind the encryption to the chunk's location in memory.  The
    // bottom bits of t[0] are incremented for each of the 4 blocks in
    // the 32-byte chunk payload to give each block a different tweak.
    uint32_t t[2];
    t[0] = (((uint32_t)(chunk[0])) << 8) |
           (((uint32_t)(chunk[1])) << 16) |
           (((uint32_t)(chunk[2])) << 24);
    t[1] = (uint32_t)posn;

    // Construct the key schedule, consisting of k0, k1, and k0prime.
    uint32_t ks[6];
    if (encrypt) {
        // Encryption key schedule: unpack k0, k1, and k0prime from the key.
        mantis_unpack_block(ks, k);
        mantis_unpack_block(ks + 2, k + 8);
        mantis_unpack_rotated_block(ks + 4, k);
    } else {
        // Decryption key schedule: swap k0 and k0prime and XOR
        // k1 with the alpha constant.
        mantis_unpack_block(ks + 4, k);
        mantis_unpack_block(ks + 2, k + 8);
        mantis_unpack_rotated_block(ks, k);
        ks[2] ^= ALPHA_ROW0;
        ks[3] ^= ALPHA_ROW1;
    }

    // Encrypt or decrypt the four 64-bit blocks in the chunk using Mantis-8.
    mantis8(t, ks, chunk + 3);
    ++(t[0]);
    mantis8(t, ks, chunk + 3 + 8);
    ++(t[0]);
    mantis8(t, ks, chunk + 3 + 16);
    ++(t[0]);
    mantis8(t, ks, chunk + 3 + 24);

    // Clean up the key schedule.
    clean(ks);
}

/**
 * \brief Constructs a new key ring access object.
 */
KeyRingClass::KeyRingClass()
    : crypt(0)
{
}

/**
 * \brief Destroys this key ring access object.
 */
KeyRingClass::~KeyRingClass()
{
    clean(k);
}

/**
 * \brief Begins using this key ring without an encryption/decryption key.
 *
 * All values in the key ring are stored and returned as-is, under the
 * assumption that they are not encrypted.  This is also the default
 * behaviour if begin() was never called.
 *
 * \sa end()
 */
void KeyRingClass::begin()
{
    crypt = 0;
}

/**
 * \brief Begins using this key ring with a specific encryption/decryption key.
 *
 * \param key Points to the key value.
 * \param size Number of bytes in the key value.
 *
 * \note The key value will be hashed with SHA256 before use in case the
 * key is low entropy.  Hashing the value spreads the entropy uniformly
 * amongst the actual key bits.
 *
 * \sa end()
 */
void KeyRingClass::begin(const void *key, size_t size)
{
    SHA256 hash;
    hash.update(key, size);
    hash.finalize(k, sizeof(k));
    crypt = cryptChunk;
}

/**
 * \brief Begins using this key ring with a specific encryption/decryption
 * passphrase.
 *
 * \param passphrase Points to the passphrase to use, or NULL for an
 * empty passphrase.
 *
 * \note The passphrase will be hashed with SHA256 before use in case it
 * has low entropy.  Hashing the value spreads the entropy uniformly
 * amongst the actual key bits.
 *
 * Usually it is a good idea to salt the passphrase and to hash it over
 * and over for many iterations to slow down dictionary attacks.  This class
 * does not do this: the application needs to do this itself if necessary.
 *
 * \sa end()
 */
void KeyRingClass::begin(const char *passphrase)
{
    if (passphrase)
        begin(passphrase, strlen(passphrase));
    else
        begin(0, 0);
}

/**
 * \brief Ends using this key ring.
 *
 * If the key ring was using an encryption key, then this function will
 * destroy the key.  The begin() function must be called again with the
 * decryption passphrase or key to continue using the key ring.
 *
 * \sa begin()
 */
void KeyRingClass::end()
{
    clean(k);
    crypt = 0;
}

/**
 * \fn bool KeyRingClass::setLocalKeyPair(uint16_t id, const void *pair, size_t size)
 * \brief Sets a local key pair in permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.  Cannot be zero.
 * \param pair Points to the key pair value, which is assumed to be the
 * private key followed by the public key.
 * \param size Size of the key pair, including both the private and public
 * components.
 *
 * \return Returns true if the key pair was stored, or false if there is
 * insufficient space to store the key pair, or this platform does not have
 * permanent storage available.
 *
 * \sa getLocalKeyPair()
 */

/**
 * \fn size_t KeyRingClass::getLocalKeyPair(uint16_t id, void *pair, size_t maxSize)
 * \brief Gets a local key pair from permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.  Cannot be zero.
 * \param pair Points to the buffer to retrieve the key pair value, which
 * is assumed to consist of the private key followed by the public key.
 * \param maxSize Maximum size of the \a pair buffer which may be larger
 * than the actual key pair size, to support the retrieval of variable-length
 * key pairs.
 *
 * \return The actual number of bytes in the key pair, or zero if the key
 * pair was not found.
 *
 * The companion function getLocalKeyPairSize() can be used to query the
 * size of the key pair before it is retrieved.
 *
 * \sa getLocalKeyPairSize(), setLocalKeyPair()
 */

/**
 * \fn size_t KeyRingClass::getLocalKeyPairSize(uint16_t id)
 * \brief Gets the size of a local key pair in permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.  Cannot be zero.
 *
 * \return The number of bytes of data that are stored for the key pair,
 * or zero if there is no key pair currently associated with \a id.
 *
 * \sa getLocalKeyPair()
 */

/**
 * \fn void KeyRingClass::removeLocalKeyPair(uint16_t id)
 * \brief Removes a local key pair from permanent storage.
 *
 * \param id Identifier for the key pair to distinguish multiple key pairs
 * in the permanent storage.  Cannot be zero.
 *
 * The key pair value will be overwritten with zero bytes to erase it.
 * However, this may not be sufficient to completely remove all trace of
 * the key pair from flash memory or EEPROM.  If the underlying storage
 * mechanism is performing wear-levelling, then it may leave older copies
 * of the value in unused pages when new values are written.
 */

/**
 * \fn bool KeyRingClass::setRemotePublicKey(uint16_t id, const void *key, size_t size)
 * \brief Sets a remote public key in permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.  Cannot be zero.
 * \param key Points to the public key value.
 * \param size Size of the public key value in bytes.
 *
 * \return Returns true if the public key was stored, or false if there is
 * insufficient space to store the public key, or this platform does not have
 * permanent storage available.
 *
 * \sa getRemotePublicKey()
 */

/**
 * \fn size_t KeyRingClass::getRemotePublicKey(uint16_t id, void *key, size_t maxSize)
 * \brief Gets a remote public key from permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.  Cannot be zero.
 * \param key Points to the buffer to retrieve the public key value.
 * \param maxSize Maximum size of the \a key buffer which may be larger
 * than the actual public key size, to support the retrieval of
 * variable-length public keys.
 *
 * \return The actual number of bytes in the public key, or zero if the
 * public key was not found.
 *
 * The companion function getRemotePublicKeySize() can be used to query the
 * size of the public key before it is retrieved.
 *
 * \sa getRemotePublicKeySize(), setRemotePublicKey()
 */

/**
 * \fn size_t KeyRingClass::getRemotePublicKeySize(uint16_t id)
 * \brief Gets the size of a remote public key in permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.  Cannot be zero.
 *
 * \return The number of bytes of data that are stored for the public key,
 * or zero if there is no public key currently associated with \a id.
 *
 * \sa getRemotePublicKey()
 */

/**
 * \fn void KeyRingClass::removeRemotePublicKey(uint16_t id)
 * \brief Removes a remote public key from permanent storage.
 *
 * \param id Identifier for the remote public key to distinguish multiple
 * public keys in the permanent storage.  Cannot be zero.
 */

/**
 * \fn bool KeyRingClass::setSharedSymmetricKey(uint16_t id, const void *key, size_t size)
 * \brief Sets a shared symmetric key in permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.  Cannot be zero.
 * \param key Points to the shared symmetric key value.
 * \param size Size of the shared symmetric key value in bytes.
 *
 * \return Returns true if the shared symmetric key was stored, or false if
 * there is insufficient space to store the key, or this platform does not
 * have permanent storage available.
 *
 * \sa getSharedSymmetricKey()
 */

/**
 * \fn size_t KeyRingClass::getSharedSymmetricKey(uint16_t id, void *key, size_t maxSize)
 * \brief Gets a shared symmetric key from permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.  Cannot be zero.
 * \param key Points to the buffer to retrieve the shared symmetric key value.
 * \param maxSize Maximum size of the \a key buffer which may be larger
 * than the actual shared symmetric key size, to support the retrieval of
 * variable-length keys.
 *
 * \return The actual number of bytes in the shared symmetric key,
 * or zero if the key was not found.
 *
 * The companion function getSharedSymmetricKeySize() can be used to
 * query the size of the shared symmetric key before it is retrieved.
 *
 * \sa getSharedSymmetricKeySize(), setSharedSymmetricKey()
 */

/**
 * \fn size_t KeyRingClass::getSharedSymmetricKeySize(uint16_t id)
 * \brief Gets the size of a shared symmetric key in permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.  Cannot be zero.
 *
 * \return The number of bytes of data that are stored for the shared
 * symmetric key, or zero if there is no key currently associated with \a id.
 *
 * \sa getSharedSymmetricKey()
 */

/**
 * \fn void KeyRingClass::removeSharedSymmetricKey(uint16_t id)
 * \brief Removes a shared symmetric key from permanent storage.
 *
 * \param id Identifier for the shared symmetric key to distinguish multiple
 * keys in the permanent storage.  Cannot be zero.
 *
 * The symmetric key value will be overwritten with zero bytes to erase it.
 * However, this may not be sufficient to completely remove all trace of
 * the key from flash memory or EEPROM.  If the underlying storage mechanism
 * is performing wear-levelling, then it may leave older copies of the value
 * in unused pages when new values are written.
 */

/**
 * \fn bool KeyRingClass::setOtherData(uint16_t id, const void *data, size_t size);
 * \brief Sets an arbitrary data value in permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.  Cannot be zero.
 * \param data Points to the data value to store.
 * \param size Size of the data value in bytes.
 *
 * \return Returns true if the data value was stored, or false if there is
 * insufficient space to store the value, or this platform does not have
 * permanent storage available.
 *
 * \sa getOtherData()
 */

/**
 * \fn size_t KeyRingClass::getOtherData(uint16_t id, void *data, size_t maxSize)
 * \brief Gets an arbitrary data value from permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.  Cannot be zero.
 * \param data Points to the buffer to retrieve the data value.
 * \param maxSize Maximum size of the \a data buffer which may be larger
 * than the actual data value size, to support the retrieval of
 * variable-length values.
 *
 * \return The actual number of bytes in the data value, or zero if the value
 * was not found.
 *
 * The companion function getOtherDataSize() can be used to query the size of
 * the data value before it is retrieved.
 *
 * \sa getOtherDataSize(), setOtherData()
 */

/**
 * \fn size_t KeyRingClass::getOtherDataSize(uint16_t id)
 * \brief Gets the size of an arbitrary data value in permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.  Cannot be zero.
 *
 * \return The number of bytes of data that are stored for the value,
 * or zero if there is no value currently associated with \a id.
 *
 * \sa getOtherData()
 */

/**
 * \fn void KeyRingClass::removeOtherData(uint16_t id)
 * \brief Removes an arbitrary data value from permanent storage.
 *
 * \param id Identifier for the data value to distinguish multiple values
 * in the permanent storage.  Cannot be zero.
 */

/**
 * \brief Remove all keys and other data from permanent storage.
 *
 * This is intended to "clean" a device prior to re-purposing it,
 * or to re-initialize the key ring if it has become corrupted.
 */
void KeyRingClass::removeAll()
{
    unsigned posn;
    readStart(posn);
    while (eraseChunk(posn))
        ; // Keep erasing until we get to the end of storage.
    readWriteEnd();
}

/*

Storage format
--------------

The key data is stored in EEPROM or Flash memory as a list of 36 byte
chunks, each of which may contain up to 32 bytes of data.  If a key is
larger than 32 bytes, then multiple chunks are used.

Each chunk starts with a 3 byte header and ends with a 1 byte CRC-8 value.
The CRC-8 value does not provide any real security.  It exists only to
distinguish a valid chunk from the end of the key storage area.

The first 2 bytes of the header are the "id" value for the key.  The third
byte contains several bit fields:

    Bit     Description
    0-4     Length of the data in this chunk, where 0 means 32.
    5-6     The "type" value for the key, between 0 and 3.
    7       Set to 1 if this is the last chunk for the key or 0 if
            there are more chunks that make up the total key value.

If the key in the chunk has been removed, then the first 2 bytes will
be set to zero.

*/

// Imported from Crypto.cpp.
extern uint8_t crypto_crc8(uint8_t tag, const void *data, unsigned size);

bool KeyRingClass::set(uint16_t id, uint8_t type, const void *data, size_t size)
{
    const uint8_t *d = (const uint8_t *)data;
    size_t written = 0;
    size_t chunkSize;
    uint8_t chunk[ChunkSize];
    unsigned posn;
    bool includeEmpty = true;
    type = (type << 5) & 0x60;
    if (!id || !data || !size)
        return false;
    readStart(posn);
    while (readChunk(posn, chunk, id, type, includeEmpty, false)) {
        if (written < size) {
            // We still have more data to write from the incoming value.
            chunkSize = size - written;
            if (chunkSize > 32)
                chunkSize = 32;
            chunk[0] = (uint8_t)id;
            chunk[1] = (uint8_t)(id >> 8);
            chunk[2] = (chunkSize & 0x1F) | type;
            written += chunkSize;
            if (written >= size) {
                chunk[2] |= (uint8_t)0x80;
                includeEmpty = false;
            }
            memcpy(chunk + 3, d, chunkSize);
            memset(chunk + 3 + chunkSize, 0xFF, 32 - chunkSize);
            writeChunk(posn, chunk);
            d += chunkSize;
        } else {
            // We've run out of data.  Erase any remaining chunks that
            // have the same identifier and type in case we are replacing
            // the original value with one that is shorter.
            memset(chunk, 0, sizeof(chunk));
            writeChunk(posn, chunk);
        }
    }
    while (written < size) {
        // We still have data to write but we have reached the end
        // of the existing values that are stored in the key ring.
        // Add some extra chunks to the end of the key ring.
        chunkSize = size - written;
        if (chunkSize > 32)
            chunkSize = 32;
        chunk[0] = (uint8_t)id;
        chunk[1] = (uint8_t)(id >> 8);
        chunk[2] = (chunkSize & 0x1F) | type;
        written += chunkSize;
        if (written >= size)
            chunk[2] |= (uint8_t)0x80;
        memcpy(chunk + 3, d, chunkSize);
        memset(chunk + 3 + chunkSize, 0xFF, 32 - chunkSize);
        if (!writeExtraChunk(posn, chunk)) {
            // We have run out of space in permanent storage.  Because the
            // value is now corrupt we have no choice but to remove it.
            remove(id, type >> 5);
            readWriteEnd();
            return false;
        }
        d += chunkSize;
    }
    readWriteEnd();
    return true;
}

size_t KeyRingClass::get(uint16_t id, uint8_t type, void *data, size_t maxSize)
{
    uint8_t *d = (uint8_t *)data;
    size_t size = 0;
    size_t chunkSize;
    uint8_t chunk[ChunkSize];
    unsigned posn;
    type = (type << 5) & 0x60;
    if (!data)
        return 0; // Invalid buffer, so cannot return anything.
    readStart(posn);
    while (readChunk(posn, chunk, id, type, false, true)) {
        // Get the length of the data in the chunk and check that we
        // haven't exceeded the maximum size in the return buffer.
        chunkSize = (chunk[2] & 0x1F);
        if (!chunkSize)
            chunkSize = 32;
        if ((size + chunkSize) > maxSize)
            break;

        // Copy the data out of the chunk.
        memcpy(d, chunk + 3, chunkSize);
        size += chunkSize;
        d += chunkSize;

        // Stop if this is the last chunk for the key identifier / type.
        if (chunk[2] & 0x80) {
            readWriteEnd();
            clean(chunk);
            return size;
        }
    }

    // The request failed either because the identifier was not found
    // or the value was truncated before we found the last chunk.
    //
    // Clear the return value to avoid accidentally returning random
    // data from system RAM.  If the caller fails to check the return
    // value then they might accidentally send the value out on the wire
    // as a public key value or similar.  This is not desirable.
    memset(data, 0, maxSize);
    clean(chunk);
    readWriteEnd();
    return 0;
}

size_t KeyRingClass::getSize(uint16_t id, uint8_t type)
{
    size_t size = 0;
    size_t chunkSize;
    uint8_t chunk[ChunkSize];
    unsigned posn;
    type = (type << 5) & 0x60;
    readStart(posn);
    while (readChunk(posn, chunk, id, type, false, false)) {
        // Get the length of the data in the chunk and add to the total.
        chunkSize = (chunk[2] & 0x1F);
        if (!chunkSize)
            chunkSize = 32;
        size += chunkSize;

        // Stop if this is the last chunk for the key identifier / type.
        if (chunk[2] & 0x80) {
            readWriteEnd();
            clean(chunk);
            return size;
        }
    }
    readWriteEnd();
    clean(chunk);
    return 0;
}

void KeyRingClass::remove(uint16_t id, uint8_t type)
{
    uint8_t chunk[ChunkSize];
    unsigned posn;
    type = (type << 5) & 0x60;
    readStart(posn);
    while (readChunk(posn, chunk, id, type, false, false)) {
        memset(chunk, 0, sizeof(chunk));
        writeChunk(posn, chunk);
    }
    readWriteEnd();
    clean(chunk);
}

#if defined(KEY_RING_DUE)

// Buffer for temporarily holding the contents of a page during modification.
static uint32_t pageBuffer[SAM_FLASH_PAGE_SIZE / sizeof(uint32_t)];

// Number of the page that is cached.
static unsigned pageNum = 0;

// Set to true when the contents of a page has been cached into "pageBuffer".
static bool pageCached = false;

// Set to true if the page needs to be flushed when we move onto a new
// page or when we finish a key write operation.
static bool pageDirty = false;

// Flushes the cached page if it is dirty.
static void flushDirtyPage()
{
    if (pageDirty) {
        // Unlock the flash memory region containing the page.
        crypto_sam_unlock_page(pageNum);

        // Load the contents of the page buffer into the latch registers.
        uint32_t *addr = (uint32_t *)
            (SAM_FLASH_ADDR + pageNum * SAM_FLASH_PAGE_SIZE);
        for (unsigned posn = 0;
                posn < (SAM_FLASH_PAGE_SIZE / sizeof(uint32_t)); ++posn)
            addr[posn] = pageBuffer[posn];

        // Erase the page and write its new contents.
        crypto_sam_erase_and_write(pageNum);

        // Lock the flash memory region containing the page.
        crypto_sam_lock_page(pageNum);
    }
    pageDirty = false;
    pageCached = false;
}

// Ensures that a specific page is cached in RAM for easy access/modification.
static void ensureCached(unsigned page)
{
    // The page number is an offset into the key ring storage area, starting
    // at the last page in the area.  Turn it into a physical page number.
    page = KEY_RING_STORAGE_FIRST_PAGE + KEY_RING_STORAGE_NUM_PAGES - 1 - page;

    // If the page is already cached in the buffer, then we are done.
    if (pageCached && page == pageNum)
        return;

    // Flush the previous cached page if it is dirty.
    flushDirtyPage();

    // Cache the new page.
    void *addr = (void *)(SAM_FLASH_ADDR + page * SAM_FLASH_PAGE_SIZE);
    memcpy(pageBuffer, addr, SAM_FLASH_PAGE_SIZE);
    pageNum = page;
    pageCached = true;
    pageDirty = false;
}

#endif // KEY_RING_DUE

#if defined(ESP32)

#define EEPROM_readBytes(address, value, maxLen) \
    EEPROM.readBytes((address), (value), (maxLen))
#define EEPROM_writeBytes(address, value, maxLen) \
    EEPROM.writeBytes((address), (value), (maxLen))

#elif defined(KEY_RING_SIMUL_EEPROM)

// Simulate EEPROM.readBytes() on platforms that don't have it.
static size_t EEPROM_readBytes(int address, void *value, size_t len)
{
    if (address < 0 || address >= (int)EEPROM.length())
        return 0;
    if (!value || !len)
        return 0;
    if (((size_t)(EEPROM.length() - address)) < len)
        return 0;
    for (int posn = 0; posn < (int)len; ++posn)
        ((uint8_t *)value)[posn] = EEPROM.read(address + posn);
    return len;
}

// Simulate EEPROM.writeBytes() on platforms that don't have it.
static size_t EEPROM_writeBytes(int address, const void *value, size_t len)
{
    if (address < 0 || address >= (int)EEPROM.length())
        return 0;
    if (!value || !len)
        return 0;
    if (((size_t)(EEPROM.length() - address)) < len)
        return 0;
    for (int posn = 0; posn < (int)len; ++posn)
        EEPROM.write(address + posn, ((const uint8_t *)value)[posn]);
    return len;
}

#endif

void KeyRingClass::readStart(unsigned &posn)
{
#if defined(KEY_RING_AVR_EEPROM)
    // Start at the end of EEPROM and work backwards from there.
    posn = KEY_RING_EEPROM_END;
#elif defined(KEY_RING_SIMUL_EEPROM)
    // Initialize the EEPROM simulation on this platform.  The API for the
    // ESP8266 and ESP32 ports of EEPROM for Arduino makes it a little
    // difficult to determine if the application was already making use
    // the EEPROM when we were called.  On ESP8266 we can use length()
    // but on ESP32 that function is defined poorly so we use readBytes()
    // to probe the last location in simulated EEPROM to see if it is there.
#if defined(ESP32)
    uint8_t temp = 0;
    if (EEPROM_readBytes(KEY_RING_EEPROM_SIZE - 1, &temp, 1) != 1) {
#else
    if (EEPROM.length() != KEY_RING_EEPROM_SIZE) {
#endif
        EEPROM.commit();
        EEPROM.end();
        EEPROM.begin(KEY_RING_EEPROM_SIZE);
    }

    // Start at the end of EEPROM and work backwards from there.
    posn = KEY_RING_EEPROM_END;
#elif defined(KEY_RING_DUE)
    crypto_sam_flash_init();
    posn = 0;
#else
    posn = 0;
#endif
}

bool KeyRingClass::readChunk
    (unsigned &posn, uint8_t chunk[ChunkSize], unsigned &actual)
{
#if defined(KEY_RING_AVR_EEPROM)
    posn -= ChunkSize;
    if (posn < KEY_RING_EEPROM_START)
        return false;
    eeprom_read_block(chunk, (const void *)(uintptr_t)posn, ChunkSize);
    if (crypto_crc8('K', chunk, ChunkSize - 1) != chunk[ChunkSize - 1])
        return false;
    actual = posn;
    return true;
#elif defined(KEY_RING_SIMUL_EEPROM)
    posn -= ChunkSize;
    if (posn < KEY_RING_EEPROM_START)
        return false;
    EEPROM_readBytes(posn, chunk, ChunkSize);
    if (crypto_crc8('K', chunk, ChunkSize - 1) != chunk[ChunkSize - 1])
        return false;
    actual = posn;
    return true;
#elif defined(KEY_RING_FLASH)
    unsigned page = posn / KEY_RING_CHUNKS_PER_PAGE;
    unsigned offset = posn % KEY_RING_CHUNKS_PER_PAGE;
    actual = posn;
    if (page >= KEY_RING_STORAGE_NUM_PAGES)
        return false;
    ensureCached(page);
    memcpy(chunk, ((uint8_t *)pageBuffer) + offset * ChunkSize, ChunkSize);
    if (crypto_crc8('K', chunk, ChunkSize - 1) != chunk[ChunkSize - 1])
        return false;
    ++posn;
    return true;
#else
    return false;
#endif
}

bool KeyRingClass::readChunk
    (unsigned &posn, uint8_t chunk[ChunkSize],
     uint16_t id, uint8_t type, bool setMode, bool decrypt)
{
    if (!id)
        return false; // Zero is not a valid key identifier.
    unsigned actual = 0;
    while (readChunk(posn, chunk, actual)) {
        if (setMode) {
            // In set mode we also return erased chunks with a zero id.
            if (chunk[0] == (uint8_t)0x00 && chunk[1] == (uint8_t)0x00)
                return true;
        }
        if (chunk[0] != (uint8_t)id || chunk[1] != (uint8_t)(id >> 8))
            continue;
        if ((chunk[2] & 0x60) != type)
            continue;
        if (crypt) {
            // Decrypt the chunk's contents using its actual position
            // in permanent storage as part of the chunk-specific tweak.
            crypt(k, chunk, actual, false);
        }
        return true;
    }
    return false;
}

bool KeyRingClass::writeChunk(unsigned posn, uint8_t chunk[ChunkSize])
{
#if defined(KEY_RING_AVR_EEPROM)
    if (posn < KEY_RING_EEPROM_START)
        return false;
    if (crypt)
        crypt(k, chunk, posn, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    eeprom_write_block(chunk, (void *)(uintptr_t)posn, ChunkSize);
    return true;
#elif defined(KEY_RING_SIMUL_EEPROM)
    if (posn < KEY_RING_EEPROM_START)
        return false;
    if (crypt)
        crypt(k, chunk, posn, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    EEPROM_writeBytes(posn, chunk, ChunkSize);
    return true;
#elif defined(KEY_RING_FLASH)
    unsigned page = (posn - 1) / KEY_RING_CHUNKS_PER_PAGE;
    unsigned offset = (posn - 1) % KEY_RING_CHUNKS_PER_PAGE;
    if (page >= KEY_RING_STORAGE_NUM_PAGES)
        return false;
    if (crypt)
        crypt(k, chunk, posn - 1, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    ensureCached(page);
    memcpy(((uint8_t *)pageBuffer) + offset * ChunkSize, chunk, ChunkSize);
    pageDirty = true;
    return true;
#else
    return false;
#endif
}

bool KeyRingClass::writeExtraChunk(unsigned &posn, uint8_t chunk[ChunkSize])
{
#if defined(KEY_RING_AVR_EEPROM)
    if (posn < KEY_RING_EEPROM_START)
        return false;
    if (crypt)
        crypt(k, chunk, posn, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    eeprom_write_block(chunk, (void *)(uintptr_t)posn, ChunkSize);
    posn -= ChunkSize;
    return true;
#elif defined(KEY_RING_SIMUL_EEPROM)
    if (posn < KEY_RING_EEPROM_START)
        return false;
    if (crypt)
        crypt(k, chunk, posn, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    EEPROM_writeBytes(posn, chunk, ChunkSize);
    posn -= ChunkSize;
    return true;
#elif defined(KEY_RING_FLASH)
    unsigned page = posn / KEY_RING_CHUNKS_PER_PAGE;
    unsigned offset = posn % KEY_RING_CHUNKS_PER_PAGE;
    if (page >= KEY_RING_STORAGE_NUM_PAGES)
        return false;
    if (crypt)
        crypt(k, chunk, posn, true); // Encrypt the chunk's contents.
    chunk[ChunkSize - 1] = crypto_crc8('K', chunk, ChunkSize - 1);
    ensureCached(page);
    memcpy(((uint8_t *)pageBuffer) + offset * ChunkSize, chunk, ChunkSize);
    pageDirty = true;
    ++posn;
    return true;
#else
    return false;
#endif
}

bool KeyRingClass::eraseChunk(unsigned &posn)
{
    uint8_t chunk[ChunkSize];
    memset(chunk, 0xFF, sizeof(chunk));
#if defined(KEY_RING_AVR_EEPROM)
    posn -= ChunkSize;
    if (posn < KEY_RING_EEPROM_START)
        return false;
    eeprom_write_block(chunk, (void *)(uintptr_t)posn, ChunkSize);
    return true;
#elif defined(KEY_RING_SIMUL_EEPROM)
    posn -= ChunkSize;
    if (posn < KEY_RING_EEPROM_START)
        return false;
    EEPROM_writeBytes(posn, chunk, ChunkSize);
    return true;
#elif defined(KEY_RING_FLASH)
    unsigned page = posn / KEY_RING_CHUNKS_PER_PAGE;
    unsigned offset = posn % KEY_RING_CHUNKS_PER_PAGE;
    if (page >= KEY_RING_STORAGE_NUM_PAGES)
        return false;
    ensureCached(page);
    memcpy(((uint8_t *)pageBuffer) + offset * ChunkSize, chunk, ChunkSize);
    ++posn;
    pageDirty = true;
    return true;
#else
    return false;
#endif
}

void KeyRingClass::readWriteEnd()
{
#if defined(KEY_RING_SIMUL_EEPROM)
    EEPROM.commit();
#elif defined(KEY_RING_FLASH)
    flushDirtyPage();
    clean(pageBuffer, sizeof(pageBuffer));
#endif
}
