/*
 * Copyright (C) 2016,2018 Southern Storm Software, Pty Ltd.
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

#include <NoiseProtocol.h>
#include "json-reader.h"
#include <setjmp.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_MESSAGES 32
#define MAX_MESSAGE_SIZE 4096
#define MAX_PSKS 8

/**
 * \brief Information about a single test vector.
 */
typedef struct
{
    long line_number;               /**< Line number for the "name" */
    char *name;                     /**< Full name of the test case */
    char *protocol_name;            /**< Full name of the protocol */
    uint8_t *init_static;           /**< Initiator's static private key */
    size_t init_static_len;         /**< Length of init_static in bytes */
    uint8_t *init_public_static;    /**< Initiator's public key known to responder */
    size_t init_public_static_len;  /**< Length of init_public_static in bytes */
    uint8_t *resp_static;           /**< Responder's static private key */
    size_t resp_static_len;         /**< Length of resp_static in bytes */
    uint8_t *resp_public_static;    /**< Responder's public key known to initiator */
    size_t resp_public_static_len;  /**< Length of resp_public_static in bytes */
    uint8_t *init_ephemeral;        /**< Initiator's ephemeral key */
    size_t init_ephemeral_len;      /**< Length of init_ephemeral in bytes */
    uint8_t *resp_ephemeral;        /**< Responder's ephemeral key */
    size_t resp_ephemeral_len;      /**< Length of resp_ephemeral in bytes */
    uint8_t *init_hybrid;           /**< Initiator's hybrid ephemeral key */
    size_t init_hybrid_len;         /**< Length of init_hybrid in bytes */
    uint8_t *resp_hybrid;           /**< Responder's hybrid ephemeral key */
    size_t resp_hybrid_len;         /**< Length of resp_hybrid in bytes */
    uint8_t *init_prologue;         /**< Initiator's prologue data */
    size_t init_prologue_len;       /**< Length of init_prologue in bytes */
    uint8_t *resp_prologue;         /**< Responder's prologue data */
    size_t resp_prologue_len;       /**< Length of resp_prologue in bytes */
    uint8_t init_psks[MAX_PSKS][32];/**< Initiator pre-shared keys */
    size_t num_init_psks;           /**< Number of initiator PSK's */
    uint8_t resp_psks[MAX_PSKS][32];/**< Responder pre-shared keys */
    size_t num_resp_psks;           /**< Number of responder PSK's */
    uint8_t *handshake_hash;        /**< Hash at the end of the handshake */
    size_t handshake_hash_len;      /**< Length of handshake_hash in bytes */
    int fail;                       /**< Failure expected on last message */
    int fallback;                   /**< Handshake involves IK to XXfallback */
    char *fallback_pattern;         /**< Name of the pattern to fall back to */
    int is_one_way;                 /**< True if the base pattern is one-way */
    struct {
        uint8_t *payload;           /**< Payload for this message */
        size_t payload_len;         /**< Length of payload in bytes */
        uint8_t *ciphertext;        /**< Ciphertext for this message */
        size_t ciphertext_len;      /**< Length of ciphertext in bytes */
    } messages[MAX_MESSAGES];       /**< All test messages */
    size_t num_messages;            /**< Number of test messages */

} TestVector;

/**
 * \brief Frees the memory for a test vector.
 *
 * \param vec The test vector.
 */
static void test_vector_free(TestVector *vec)
{
    size_t index;
    #define free_field(name) do { if (vec->name) free(vec->name); } while (0)
    free_field(name);
    free_field(protocol_name);
    free_field(init_static);
    free_field(init_public_static);
    free_field(resp_static);
    free_field(resp_public_static);
    free_field(init_ephemeral);
    free_field(resp_ephemeral);
    free_field(init_hybrid);
    free_field(resp_hybrid);
    free_field(init_prologue);
    free_field(resp_prologue);
    free_field(handshake_hash);
    free_field(fallback_pattern);
    for (index = 0; index < vec->num_messages; ++index) {
        if (vec->messages[index].payload)
            free(vec->messages[index].payload);
        if (vec->messages[index].ciphertext)
            free(vec->messages[index].ciphertext);
    }
    memset(vec, 0, sizeof(TestVector));
}

static jmp_buf test_jump_back;

/**
 * \brief Immediate fail of the test.
 *
 * \param message The failure message to print.
 */
#define _fail(message)   \
    do { \
        printf("%s, failed at " __FILE__ ":%d\n", (message), __LINE__); \
        longjmp(test_jump_back, 1); \
    } while (0)
#define fail(message) _fail((message))

/**
 * \brief Skips the current test.
 */
#define skip() longjmp(test_jump_back, 2)

/**
 * \brief Verifies that a condition is true, failing the test if not.
 *
 * \param condition The boolean condition to test.
 */
#define _verify(condition)   \
    do { \
        if (!(condition)) { \
            printf(#condition " failed at " __FILE__ ":%d\n", __LINE__); \
            longjmp(test_jump_back, 1); \
        } \
    } while (0)
#define verify(condition) _verify((condition))

/**
 * \brief Compares two integer values for equality, failing the test if not.
 *
 * \param actual The actual value that was computed by the code under test.
 * \param expected The value that is expected.
 */
#define compare(actual, expected) \
    do { \
        long long _actual = (long long)(actual); \
        long long _expected = (long long)(expected); \
        if (_actual != _expected) { \
            printf(#actual " != " #expected " at " __FILE__ ":%d\n", __LINE__); \
            printf("    actual  : %lld (0x%llx)\n", _actual, _actual); \
            printf("    expected: %lld (0x%llx)\n", _expected, _expected); \
            longjmp(test_jump_back, 1); \
        } \
    } while (0)

static void dump_block(uint8_t *block, size_t len)
{
    size_t index;
    if (len > 16)
        printf("\n       ");
    for (index = 0; index < len; ++index) {
        printf(" %02x", block[index]);
        if ((index % 16) == 15 && len > 16)
            printf("\n       ");
    }
    printf("\n");
}

#define compare_blocks(name, actual, actual_len, expected, expected_len)  \
    do { \
        if ((actual_len) != (expected_len) || \
                memcmp((actual), (expected), (actual_len)) != 0) { \
            printf("%s wrong at " __FILE__ ":%d\n", (name), __LINE__); \
            printf("    actual  :"); \
            dump_block((actual), (actual_len)); \
            printf("    expected:"); \
            dump_block((expected), (expected_len)); \
            longjmp(test_jump_back, 1); \
        } \
    } while (0)

/**
 * \brief Creates a handshake object for a specific protocol.
 *
 * \pararm protocol The name of the protocol.
 */
static NoiseHandshakeState *create_handshake(const char *protocol)
{
    if (!strcmp(protocol, "Noise_IK_25519_AESGCM_SHA256"))
        return new NoiseHandshakeState_IK_25519_AESGCM_SHA256();
    if (!strcmp(protocol, "Noise_IK_25519_ChaChaPoly_BLAKE2s"))
        return new NoiseHandshakeState_IK_25519_ChaChaPoly_BLAKE2s();
    if (!strcmp(protocol, "Noise_IK_25519_ChaChaPoly_SHA256"))
        return new NoiseHandshakeState_IK_25519_ChaChaPoly_SHA256();

    if (!strcmp(protocol, "Noise_NNpsk0_25519_AESGCM_SHA256"))
        return new NoiseHandshakeState_NNpsk0_25519_AESGCM_SHA256();
    if (!strcmp(protocol, "Noise_NNpsk0_25519_ChaChaPoly_BLAKE2s"))
        return new NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_BLAKE2s();
    if (!strcmp(protocol, "Noise_NNpsk0_25519_ChaChaPoly_SHA256"))
        return new NoiseHandshakeState_NNpsk0_25519_ChaChaPoly_SHA256();

    if (!strcmp(protocol, "Noise_XX_25519_AESGCM_SHA256"))
        return new NoiseHandshakeState_XX_25519_AESGCM_SHA256();
    if (!strcmp(protocol, "Noise_XX_25519_ChaChaPoly_BLAKE2s"))
        return new NoiseHandshakeState_XX_25519_ChaChaPoly_BLAKE2s();
    if (!strcmp(protocol, "Noise_XX_25519_ChaChaPoly_SHA256"))
        return new NoiseHandshakeState_XX_25519_ChaChaPoly_SHA256();

    fail(protocol);
    return 0;
}

/**
 * \brief Test a connection between an initiator and a responder.
 *
 * \param vec The test vector.
 */
static void test_connection(const TestVector *vec)
{
    NoiseHandshakeState *initiator = 0;
    NoiseHandshakeState *responder = 0;
    NoiseHandshakeState *send;
    NoiseHandshakeState *recv;
#if 0
    NoiseCipherState *c1init;
    NoiseCipherState *c2init;
    NoiseCipherState *c1resp;
    NoiseCipherState *c2resp;
    NoiseCipherState *csend;
    NoiseCipherState *crecv;
#endif
    uint8_t message[MAX_MESSAGE_SIZE];
    uint8_t payload[MAX_MESSAGE_SIZE];
    int result;
    size_t index;
    //size_t mac_len;
    Noise::Party role;

    /* Create the two ends of the connection */
    initiator = create_handshake(vec->protocol_name);
    responder = create_handshake(vec->protocol_name);

    /* Should be able to start the handshake now on both sides */
    initiator->start
        (Noise::Initiator, vec->init_prologue, vec->init_prologue_len);
    responder->start
        (Noise::Responder, vec->resp_prologue, vec->resp_prologue_len);
    compare(initiator->state(), Noise::Write);
    compare(responder->state(), Noise::Read);

    /* Set all keys that we need to use.  We do this after start()
       because otherwise it will clear the test ephemeral keys we're
       about to set on the objects */
    if (vec->init_static) {
        verify(initiator->setParameter
            (Noise::LocalStaticPrivateKey,
             vec->init_static, vec->init_static_len));
    }
    if (vec->init_public_static) {
        verify(responder->setParameter
            (Noise::RemoteStaticPublicKey,
             vec->init_public_static, vec->init_public_static_len));
    }
    if (vec->resp_static) {
        verify(responder->setParameter
            (Noise::LocalStaticPrivateKey,
             vec->resp_static, vec->resp_static_len));
    }
    if (vec->resp_public_static) {
        verify(initiator->setParameter
            (Noise::RemoteStaticPublicKey,
             vec->resp_public_static, vec->resp_public_static_len));
    }
    if (vec->init_ephemeral) {
        verify(initiator->setParameter
            (Noise::LocalEphemPrivateKey,
             vec->init_ephemeral, vec->init_ephemeral_len));
    }
    if (vec->resp_ephemeral) {
        verify(responder->setParameter
            (Noise::LocalEphemPrivateKey,
             vec->resp_ephemeral, vec->resp_ephemeral_len));
    }
    if (vec->num_init_psks) {
        /* For now we only support one PSK per handshake */
        verify(initiator->setParameter
            (Noise::PreSharedKey, vec->init_psks[0], 32));
    }
    if (vec->num_resp_psks) {
        verify(responder->setParameter
            (Noise::PreSharedKey, vec->resp_psks[0], 32));
    }

    /* Work through the messages one by one until both sides "split" */
    role = Noise::Initiator;
    for (index = 0; index < vec->num_messages; ++index) {
        if (initiator->state() == Noise::Split &&
            responder->state() == Noise::Split) {
            break;
        }
        if (role == Noise::Initiator) {
            /* Send on the initiator, receive on the responder */
            send = initiator;
            recv = responder;
            role = Noise::Responder;
        } else {
            /* Send on the responder, receive on the initiator */
            send = responder;
            recv = initiator;
            role = Noise::Initiator;
        }
        compare(send->state(), Noise::Write);
        compare(recv->state(), Noise::Read);

        memset(message, 0xAA, sizeof(message));
        result = send->write(message, sizeof(message),
                             vec->messages[index].payload,
                             vec->messages[index].payload_len);
        verify(result >= 0);
        compare_blocks("ciphertext", message, (size_t)result,
                       vec->messages[index].ciphertext,
                       vec->messages[index].ciphertext_len);

        memset(payload, 0xBB, sizeof(payload));
        result = recv->read(payload, sizeof(payload), message, result);
        verify(result >= 0);
        compare_blocks("plaintext", payload, (size_t)result,
                       vec->messages[index].payload,
                       vec->messages[index].payload_len);
    }

#if 0
    /* Handshake finished.  Check the handshake hash values */
#if 0
    if (vec->handshake_hash_len) {
        memset(payload, 0xAA, sizeof(payload));
        compare(noise_handshakestate_get_handshake_hash
                    (initiator, payload, vec->handshake_hash_len),
                NOISE_ERROR_NONE);
        compare_blocks("handshake_hash", payload, vec->handshake_hash_len,
                       vec->handshake_hash, vec->handshake_hash_len);
        memset(payload, 0xAA, sizeof(payload));
        compare(noise_handshakestate_get_handshake_hash
                    (responder, payload, vec->handshake_hash_len),
                NOISE_ERROR_NONE);
        compare_blocks("handshake_hash", payload, vec->handshake_hash_len,
                       vec->handshake_hash, vec->handshake_hash_len);
    }
#endif

    /* Now handle the data transport */
    compare(noise_handshakestate_split(initiator, &c1init, &c2init),
            NOISE_ERROR_NONE);
    compare(noise_handshakestate_split(responder, &c2resp, &c1resp),
            NOISE_ERROR_NONE);
    mac_len = noise_cipherstate_get_mac_length(c1init);
    for (; index < vec->num_messages; ++index) {
        if (role == NOISE_ROLE_INITIATOR) {
            /* Send on the initiator, receive on the responder */
            csend = c1init;
            crecv = c1resp;
            if (!is_one_way)
                role = NOISE_ROLE_RESPONDER;
        } else {
            /* Send on the responder, receive on the initiator */
            csend = c2resp;
            crecv = c2init;
            role = NOISE_ROLE_INITIATOR;
        }
        verify(sizeof(message) >= (vec->messages[index].payload_len + mac_len));
        memcpy(message, vec->messages[index].payload,
               vec->messages[index].payload_len);
        noise_buffer_set_inout(mbuf, message, vec->messages[index].payload_len,
                               sizeof(message));
        compare(noise_cipherstate_encrypt(csend, &mbuf),
                NOISE_ERROR_NONE);
        compare_blocks("ciphertext", mbuf.data, mbuf.size,
                       vec->messages[index].ciphertext,
                       vec->messages[index].ciphertext_len);
        compare(noise_cipherstate_decrypt(crecv, &mbuf),
                NOISE_ERROR_NONE);
        compare_blocks("plaintext", mbuf.data, mbuf.size,
                       vec->messages[index].payload,
                       vec->messages[index].payload_len);
    }
#endif

    /* Clean up */
    delete initiator;
    delete responder;

#if 0
    compare(noise_cipherstate_free(c1init), NOISE_ERROR_NONE);
    compare(noise_cipherstate_free(c2init), NOISE_ERROR_NONE);
    compare(noise_cipherstate_free(c1resp), NOISE_ERROR_NONE);
    compare(noise_cipherstate_free(c2resp), NOISE_ERROR_NONE);
#endif
}

/**
 * \brief Runs a fully parsed test vector.
 *
 * \param reader The input stream, for error reporting.
 * \param vec The test vector.
 *
 * \return Non-zero if the test succeeded, zero if it failed.
 */
static int test_vector_run(JSONReader *reader, const TestVector *vec)
{
    int value;
    printf("%s ... ", vec->name);
    fflush(stdout);
    if ((value = setjmp(test_jump_back)) == 0) {
        test_connection(vec);
        printf("ok\n");
        return 1;
    } else if (value == 2) {
        printf("skipped\n");
        return 1;
    } else {
        printf("-> test data at %s:%ld\n", reader->filename, vec->line_number);
        return 0;
    }
}

/**
 * \brief Look for a specific token next in the input stream.
 *
 * \param reader The input stream.
 * \param token The token code.
 * \param name The token name for error reporting.
 */
static void expect_token(JSONReader *reader, JSONToken token, const char *name)
{
    if (reader->errors)
        return;
    if (reader->token == token)
        json_next_token(reader);
    else
        json_error(reader, "Expecting '%s'", name);
}

/**
 * \brief Look for a specific field name next in the input stream,
 * followed by a colon.
 *
 * \param reader The input stream.
 * \param name The field name.
 */
static void expect_name(JSONReader *reader, const char *name)
{
    if (reader->errors)
        return;
    if (json_is_name(reader, name)) {
        json_next_token(reader);
        expect_token(reader, JSON_TOKEN_COLON, ":");
    } else {
        json_error(reader, "Expecting \"%s\"", name);
    }
}

/**
 * \brief Look for a field with a string value.
 *
 * \param reader The input stream.
 * \param value The location where to place the string value.
 */
static void expect_string_field(JSONReader *reader, char **value)
{
    json_next_token(reader);
    expect_token(reader, JSON_TOKEN_COLON, ":");
    if (!reader->errors && reader->token == JSON_TOKEN_STRING) {
        *value = reader->str_value;
        reader->str_value = 0;
        json_next_token(reader);
        if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
            json_next_token(reader);
    }
}

/**
 * \brief Converts an ASCII character into a hexadecimal digit.
 *
 * \param ch The ASCII character.
 *
 * \return The digit between 0 and 15, or -1 if \a ch is not hexadecimal.
 */
static int from_hex_digit(int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    else
        return -1;
}

/**
 * \brief Look for a field with a binary value.
 *
 * \param reader The input stream.
 * \param value The location where to place the binary value.
 *
 * \return The size of the binary value in bytes.
 */
static size_t expect_binary_field(JSONReader *reader, uint8_t **value)
{
    size_t size = 0;
    size_t posn;
    const char *hex;
    int digit1, digit2;
    json_next_token(reader);
    expect_token(reader, JSON_TOKEN_COLON, ":");
    if (!reader->errors && reader->token == JSON_TOKEN_STRING) {
        size = strlen(reader->str_value) / 2;
        *value = (uint8_t *)calloc(1, size + 1);
        if (!(*value)) {
            json_error(reader, "Out of memory");
            return 0;
        }
        hex = reader->str_value;
        for (posn = 0; posn < size; ++posn) {
            digit1 = from_hex_digit(hex[posn * 2]);
            digit2 = from_hex_digit(hex[posn * 2 + 1]);
            if (digit1 < 0 || digit2 < 0) {
                json_error(reader, "Invalid hexadecimal data");
                return 0;
            }
            (*value)[posn] = digit1 * 16 + digit2;
        }
        json_next_token(reader);
        if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
            json_next_token(reader);
    }
    return size;
}

/**
 * \brief Look for a field with a boolean value.
 *
 * \param reader The input stream.
 * \return The boolean value.
 */
static int expect_boolean_field(JSONReader *reader)
{
    int result = 0;
    json_next_token(reader);
    expect_token(reader, JSON_TOKEN_COLON, ":");
    if (!reader->errors && (reader->token == JSON_TOKEN_TRUE ||
                            reader->token == JSON_TOKEN_FALSE)) {
        result = (reader->token == JSON_TOKEN_TRUE);
        json_next_token(reader);
        if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
            json_next_token(reader);
    }
    return result;
}

/**
 * \brief Parse a list of PSK's from a JSON input stream.
 *
 * \param reader The input stream.
 * \param Array to receive the PSK's.
 * \return The number of PSK's that were parsed.
 */
static size_t parse_psk_list(JSONReader *reader, uint8_t psks[MAX_PSKS][32])
{
    size_t count = 0;
    json_next_token(reader);
    expect_token(reader, JSON_TOKEN_COLON, ":");
    expect_token(reader, JSON_TOKEN_LSQUARE, "[");
    while (!reader->errors && reader->token == JSON_TOKEN_STRING) {
        const char *hex = reader->str_value;
        size_t size = strlen(hex) / 2;
        size_t posn;
        if (size != 32) {
            json_error(reader, "PSK is not 32 bytes in size");
            return 0;
        }
        if (count >= MAX_PSKS) {
            json_error(reader, "Too many PSK's");
            return 0;
        }
        for (posn = 0; posn < size; ++posn) {
            int digit1 = from_hex_digit(hex[posn * 2]);
            int digit2 = from_hex_digit(hex[posn * 2 + 1]);
            if (digit1 < 0 || digit2 < 0) {
                json_error(reader, "Invalid hexadecimal data");
                return 0;
            }
            psks[count][posn] = digit1 * 16 + digit2;
        }
        ++count;
        json_next_token(reader);
        if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
            json_next_token(reader);
    }
    expect_token(reader, JSON_TOKEN_RSQUARE, "]");
    if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
        json_next_token(reader);
    return reader->errors ? 0 : count;
}

/**
 * \brief Processes a single test vector from an input stream.
 *
 * \param reader The reader representing the input stream.
 *
 * \return Non-zero if the test succeeded, zero if it failed.
 */
static int process_test_vector(JSONReader *reader)
{
    TestVector vec;
    int retval = 1;
    memset(&vec, 0, sizeof(TestVector));
    while (!reader->errors && reader->token == JSON_TOKEN_STRING) {
        if (json_is_name(reader, "name")) {
            vec.line_number = reader->line_number;
            expect_string_field(reader, &(vec.name));
        } else if (json_is_name(reader, "protocol_name")) {
            vec.line_number = reader->line_number;
            expect_string_field(reader, &(vec.protocol_name));
        } else if (json_is_name(reader, "init_static")) {
            vec.init_static_len =
                expect_binary_field(reader, &(vec.init_static));
        } else if (json_is_name(reader, "init_remote_static")) {
            /* Refers to the initiator have pre-knowledge of the responder's
               public key, which is "resp_public_static" in TestVector */
            vec.resp_public_static_len =
                expect_binary_field(reader, &(vec.resp_public_static));
        } else if (json_is_name(reader, "resp_static")) {
            vec.resp_static_len =
                expect_binary_field(reader, &(vec.resp_static));
        } else if (json_is_name(reader, "resp_remote_static")) {
            /* Refers to the responder have pre-knowledge of the initiator's
               public key, which is "init_public_static" in TestVector */
            vec.init_public_static_len =
                expect_binary_field(reader, &(vec.init_public_static));
        } else if (json_is_name(reader, "init_ephemeral")) {
            vec.init_ephemeral_len =
                expect_binary_field(reader, &(vec.init_ephemeral));
        } else if (json_is_name(reader, "resp_ephemeral")) {
            vec.resp_ephemeral_len =
                expect_binary_field(reader, &(vec.resp_ephemeral));
        } else if (json_is_name(reader, "init_hybrid_ephemeral")) {
            vec.init_hybrid_len =
                expect_binary_field(reader, &(vec.init_hybrid));
        } else if (json_is_name(reader, "resp_hybrid_ephemeral")) {
            vec.resp_hybrid_len =
                expect_binary_field(reader, &(vec.resp_hybrid));
        } else if (json_is_name(reader, "init_prologue")) {
            vec.init_prologue_len =
                expect_binary_field(reader, &(vec.init_prologue));
        } else if (json_is_name(reader, "resp_prologue")) {
            vec.resp_prologue_len =
                expect_binary_field(reader, &(vec.resp_prologue));
        } else if (json_is_name(reader, "init_psks")) {
            vec.num_init_psks = parse_psk_list(reader, vec.init_psks);
        } else if (json_is_name(reader, "resp_psks")) {
            vec.num_resp_psks = parse_psk_list(reader, vec.resp_psks);
        } else if (json_is_name(reader, "handshake_hash")) {
            vec.handshake_hash_len =
                expect_binary_field(reader, &(vec.handshake_hash));
        } else if (json_is_name(reader, "fail")) {
            vec.fail = expect_boolean_field(reader);
        } else if (json_is_name(reader, "fallback")) {
            vec.fallback = expect_boolean_field(reader);
        } else if (json_is_name(reader, "fallback_pattern")) {
            expect_string_field(reader, &(vec.fallback_pattern));
        } else if (json_is_name(reader, "messages")) {
            json_next_token(reader);
            expect_token(reader, JSON_TOKEN_COLON, ":");
            expect_token(reader, JSON_TOKEN_LSQUARE, "[");
            while (!reader->errors && reader->token == JSON_TOKEN_LBRACE) {
                if (vec.num_messages >= MAX_MESSAGES) {
                    json_error(reader, "Too many messages for test vector");
                    break;
                }
                expect_token(reader, JSON_TOKEN_LBRACE, "{");
                while (!reader->errors && reader->token == JSON_TOKEN_STRING) {
                    if (json_is_name(reader, "payload")) {
                        vec.messages[vec.num_messages].payload_len =
                            expect_binary_field
                                (reader, &(vec.messages[vec.num_messages].payload));
                    } else if (json_is_name(reader, "ciphertext")) {
                        vec.messages[vec.num_messages].ciphertext_len =
                            expect_binary_field
                                (reader, &(vec.messages[vec.num_messages].ciphertext));
                    } else {
                        json_error(reader, "Unknown message field '%s'",
                                   reader->str_value);
                    }
                }
                if (!vec.messages[vec.num_messages].payload)
                    json_error(reader, "Missing payload for message");
                if (!vec.messages[vec.num_messages].ciphertext)
                    json_error(reader, "Missing ciphertext for message");
                ++(vec.num_messages);
                expect_token(reader, JSON_TOKEN_RBRACE, "}");
                if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
                    json_next_token(reader);
            }
            expect_token(reader, JSON_TOKEN_RSQUARE, "]");
            if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
                json_next_token(reader);
        } else {
            json_error(reader, "Unknown field '%s'", reader->str_value);
        }
    }
    if (!vec.protocol_name) {
        json_error(reader, "Missing 'protocol_name' field");
    } else if (!vec.name) {
        vec.name = strdup(vec.protocol_name);
    }
    if (!reader->errors) {
        retval = test_vector_run(reader, &vec);
    }
    test_vector_free(&vec);
    return retval;
}

/**
 * \brief Processes the test vectors from an input stream.
 *
 * \param reader The reader representing the input stream.
 */
static void process_test_vectors(JSONReader *reader)
{
    int ok = 1;
    printf("--------------------------------------------------------------\n");
    printf("Processing vectors from %s\n", reader->filename);
    json_next_token(reader);
    expect_token(reader, JSON_TOKEN_LBRACE, "{");
    expect_name(reader, "vectors");
    expect_token(reader, JSON_TOKEN_LSQUARE, "[");
    while (!reader->errors && reader->token != JSON_TOKEN_RSQUARE) {
        expect_token(reader, JSON_TOKEN_LBRACE, "{");
        if (!process_test_vector(reader))
            ok = 0;
        expect_token(reader, JSON_TOKEN_RBRACE, "}");
        if (!reader->errors && reader->token == JSON_TOKEN_COMMA)
            expect_token(reader, JSON_TOKEN_COMMA, ",");
    }
    expect_token(reader, JSON_TOKEN_RSQUARE, "]");
    expect_token(reader, JSON_TOKEN_RBRACE, "}");
    expect_token(reader, JSON_TOKEN_END, "EOF");
    printf("--------------------------------------------------------------\n");
    if (!ok) {
        /* Some of the test vectors failed, so report a global failure */
        ++(reader->errors);
    }
}

static int process_file(const char *filename)
{
    int retval = 0;
    FILE *file = fopen(filename, "r");
    if (file) {
        JSONReader reader;
        json_init(&reader, filename, file);
        process_test_vectors(&reader);
        if (reader.errors > 0)
            retval = 1;
        json_free(&reader);
        fclose(file);
    } else {
        perror(filename);
        retval = 1;
    }
    return retval;
}

int main(int argc, char *argv[])
{
    int retval = 0;
    char *srcdir = getenv("srcdir");
    if (argc <= 1 && !srcdir) {
        fprintf(stderr, "Usage: %s vectors1.txt vectors2.txt ...\n", argv[0]);
        return 1;
    } else if (argc > 1) {
        while (argc > 1) {
            retval |= process_file(argv[1]);
            --argc;
            ++argv;
        }
    } else {
        if (chdir(srcdir) < 0) {
            perror(srcdir);
            return 1;
        }
        retval |= process_file("cacophony.txt");
        retval |= process_file("noise-c-basic.txt");
        retval |= process_file("noise-c-fallback.txt");
        retval |= process_file("noise-c-hybrid.txt");
    }
    return retval;
}
