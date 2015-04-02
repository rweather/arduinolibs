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

#include "Curve25519.h"
#include "Crypto.h"
#include "RNG.h"
#include "utility/ProgMemUtil.h"
#include <string.h>

/**
 * \class Curve25519 Curve25519.h <Curve25519.h>
 * \brief Diffie-Hellman key agreement based on the elliptic curve
 * modulo 2^255 - 19.
 *
 * \note The public functions in this class need a substantial amount of
 * stack space to store intermediate results while the curve function is
 * being evaluated.  About 1k of free stack space is recommended for safety.
 *
 * References: http://cr.yp.to/ecdh.html
 * https://tools.ietf.org/html/draft-irtf-cfrg-curves-02
 */

// Number of limbs in a value from the field modulo 2^255 - 19.
// We assume that sizeof(limb_t) is a power of 2: 1, 2, 4, etc.
#define NUM_LIMBS   (32 / sizeof(limb_t))

// Number of bits in limb_t.
#define LIMB_BITS   (8 * sizeof(limb_t))

// The overhead of clean() calls in mul(), reduceQuick(), etc can
// add up to a lot of processing time during eval().  Only do such
// cleanups if strict mode has been enabled.  Other implementations
// like curve25519-donna don't do any cleaning at all so the value
// of cleaning up the stack is dubious at best anyway.
#if defined(CURVE25519_STRICT_CLEAN)
#define strict_clean(x)     clean(x)
#else
#define strict_clean(x)     do { ; } while (0)
#endif

/**
 * \brief Evaluates the raw Curve25519 function.
 *
 * \param result The result of evaluating the curve function.
 * \param s The S parameter to the curve function.
 * \param x The X(Q) parameter to the curve function.  If this pointer is
 * NULL then the value 9 is used for \a x.
 *
 * This function is provided to assist with implementating other
 * algorithms with the curve.  Normally applications should use dh1()
 * and dh2() directly instead.
 *
 * \return Returns true if the function was evaluated; false if \a x is
 * not a proper member of the field modulo (2^255 - 19).
 *
 * Reference: https://tools.ietf.org/html/draft-irtf-cfrg-curves-02
 *
 * \sa dh1(), dh2()
 */
bool Curve25519::eval(uint8_t result[32], const uint8_t s[32], const uint8_t x[32])
{
    limb_t x_1[NUM_LIMBS];
    limb_t x_2[NUM_LIMBS];
    limb_t x_3[NUM_LIMBS];
    limb_t z_2[NUM_LIMBS];
    limb_t z_3[NUM_LIMBS];
    limb_t A[NUM_LIMBS];
    limb_t B[NUM_LIMBS];
    limb_t C[NUM_LIMBS];
    limb_t D[NUM_LIMBS];
    limb_t E[NUM_LIMBS];
    limb_t AA[NUM_LIMBS];
    limb_t BB[NUM_LIMBS];
    limb_t DA[NUM_LIMBS];
    limb_t CB[NUM_LIMBS];
    uint8_t mask;
    uint8_t sposn;
    uint8_t select;
    uint8_t swap;
    bool retval;

    // Unpack the "x" argument into the limb representation
    // which also masks off the high bit.  NULL means 9.
    if (x) {
        unpack(x_1, x);                 // x_1 = x
    } else {
        memset(x_1, 0, sizeof(x_1));    // x_1 = 9
        x_1[0] = 9;
    }

    // Check that "x" is within the range of the modulo field.
    // We can do this with a reduction - if there was no borrow
    // then the value of "x" was out of range.  Timing is sensitive
    // here so that we don't reveal anything about the value of "x".
    // If there was a reduction, then continue executing the rest
    // of this function with the (now) in-range "x" value and
    // report the failure at the end.
    retval = (bool)(reduceQuick(x_1) & 0x01);

    // Initialize the other temporary variables.
    memset(x_2, 0, sizeof(x_2));        // x_2 = 1
    x_2[0] = 1;
    memset(z_2, 0, sizeof(z_2));        // z_2 = 0
    memcpy(x_3, x_1, sizeof(x_1));      // x_3 = x
    memcpy(z_3, x_2, sizeof(x_2));      // z_3 = 1

    // Iterate over all 255 bits of "s" from the highest to the lowest.
    // We ignore the high bit of the 256-bit representation of "s".
    mask = 0x40;
    sposn = 31;
    swap = 0;
    for (uint8_t t = 255; t > 0; --t) {
        // Conditional swaps on entry to this bit but only if we
        // didn't swap on the previous bit.
        select = s[sposn] & mask;
        swap ^= select;
        cswap(swap, x_2, x_3);
        cswap(swap, z_2, z_3);

        // Evaluate the curve.
        add(A, x_2, z_2);               // A = x_2 + z_2
        square(AA, A);                  // AA = A^2
        sub(B, x_2, z_2);               // B = x_2 - z_2
        square(BB, B);                  // BB = B^2
        sub(E, AA, BB);                 // E = AA - BB
        add(C, x_3, z_3);               // C = x_3 + z_3
        sub(D, x_3, z_3);               // D = x_3 - z_3
        mul(DA, D, A);                  // DA = D * A
        mul(CB, C, B);                  // CB = C * B
        add(x_3, DA, CB);               // x_3 = (DA + CB)^2
        square(x_3, x_3);
        sub(z_3, DA, CB);               // z_3 = x_1 * (DA - CB)^2
        square(z_3, z_3);
        mul(z_3, z_3, x_1);
        mul(x_2, AA, BB);               // x_2 = AA * BB
        mulA24(z_2, E);                 // z_2 = E * (AA + a24 * E)
        add(z_2, z_2, AA);
        mul(z_2, z_2, E);

        // Move onto the next lower bit of "s".
        mask >>= 1;
        if (!mask) {
            --sposn;
            mask = 0x80;
            swap = select << 7;
        } else {
            swap = select >> 1;
        }
    }

    // Final conditional swaps.
    cswap(swap, x_2, x_3);
    cswap(swap, z_2, z_3);

    // Compute x_2 * (z_2 ^ (p - 2)) where p = 2^255 - 19.
    recip(z_3, z_2);
    mul(x_2, x_2, z_3);

    // Pack the result into the return array.
    pack(result, x_2);

    // Clean up and exit.
    clean(x_1);
    clean(x_2);
    clean(x_3);
    clean(z_2);
    clean(z_3);
    clean(A);
    clean(B);
    clean(C);
    clean(D);
    clean(E);
    clean(AA);
    clean(BB);
    clean(DA);
    clean(CB);
    return retval;
}

/**
 * \brief Performs phase 1 of a Diffie-Hellman key exchange using Curve25519.
 *
 * \param k The key value to send to the other party as part of the exchange.
 * \param f The generated secret value for this party.  This must not be
 * transmitted to any party or stored in permanent storage.  It only needs
 * to be kept in memory until dh2() is called.
 *
 * The \a f value is generated with \link RNGClass::rand() RNG.rand()\endlink.
 * It is the caller's responsibility to ensure that the global random number
 * pool has sufficient entropy to generate the 32 bytes of \a f safely
 * before calling this function.
 *
 * The following example demonstrates how to perform a full Diffie-Hellman
 * key exchange using dh1() and dh2():
 *
 * \code
 * uint8_t f[32];
 * uint8_t k[32];
 *
 * // Generate the secret value "f" and the public value "k".
 * Curve25519::dh1(k, f);
 *
 * // Send "k" to the other party.
 * ...
 *
 * // Read the "k" value that the other party sent to us.
 * ...
 *
 * // Generate the shared secret in "k" using the previous secret value "f".
 * if (!Curve25519::dh2(k, f)) {
 *     // The received "k" value was invalid - abort the session.
 *     ...
 * }
 *
 * // The "k" value can now be used to generate session keys for encryption.
 * ...
 * \endcode
 *
 * Reference: https://tools.ietf.org/html/draft-irtf-cfrg-curves-02
 *
 * \sa dh2()
 */
void Curve25519::dh1(uint8_t k[32], uint8_t f[32])
{
    do {
        // Generate a random "f" value and then adjust the value to make
        // it valid as an "s" value for eval().  According to the specification
        // we need to mask off the 3 right-most bits of f[0], mask off the
        // left-most bit of f[31], and set the second to left-most bit of f[31].
        RNG.rand(f, 32);
        f[0] &= 0xF8;
        f[31] = (f[31] & 0x7F) | 0x40;

        // Evaluate the curve function: k = Curve25519::eval(f, 9).
        // We pass NULL to eval() to indicate the value 9.  There is no
        // need to check the return value from eval() because we know
        // that 9 is a valid field element.
        eval(k, f, 0);

        // If "k" is weak for contributory behaviour then reject it,
        // generate another "f" value, and try again.  This case is
        // highly unlikely but we still perform the check just in case.
    } while (isWeakPoint(k));
}

/**
 * \brief Performs phase 2 of a Diffie-Hellman key exchange using Curve25519.
 *
 * \param k On entry, this is the key value that was received from the other
 * party as part of the exchange.  On exit, this will be the shared secret.
 * \param f The secret value for this party that was generated by dh1().
 * The \a f value will be destroyed by this function.
 *
 * \return Returns true if the key exchange was successful, or false if
 * the \a k value is invalid.
 *
 * Reference: https://tools.ietf.org/html/draft-irtf-cfrg-curves-02
 *
 * \sa dh1()
 */
bool Curve25519::dh2(uint8_t k[32], uint8_t f[32])
{
    uint8_t weak;

    // Evaluate the curve function: k = Curve25519::eval(f, k).
    // If "k" is weak for contributory behaviour before or after
    // the curve evaluation, then fail the exchange.  For safety
    // we perform every phase of the weak checks even if we could
    // bail out earlier so that the execution takes the same
    // amount of time for weak and non-weak "k" values.
    weak  = isWeakPoint(k);                     // Is "k" weak before?
    weak |= ((eval(k, f, k) ^ 0x01) & 0x01);    // Is "k" weak during?
    weak |= isWeakPoint(k);                     // Is "k" weak after?
    clean(f, 32);
    return (bool)((weak ^ 0x01) & 0x01);
}

/**
 * \brief Determines if a Curve25519 point is weak for contributory behaviour.
 *
 * \param k The point to check.
 * \return Returns 1 if \a k is weak for contributory behavior or
 * returns zero if \a k is not weak.
 */
uint8_t Curve25519::isWeakPoint(const uint8_t k[32])
{
    // List of weak points from http://cr.yp.to/ecdh.html
    // That page lists some others but they are variants on these
    // of the form "point + i * (2^255 - 19)" for i = 0, 1, 2.
    // Here we mask off the high bit and eval() catches the rest.
    static const uint8_t points[5][32] PROGMEM = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0xE0, 0xEB, 0x7A, 0x7C, 0x3B, 0x41, 0xB8, 0xAE,
         0x16, 0x56, 0xE3, 0xFA, 0xF1, 0x9F, 0xC4, 0x6A,
         0xDA, 0x09, 0x8D, 0xEB, 0x9C, 0x32, 0xB1, 0xFD,
         0x86, 0x62, 0x05, 0x16, 0x5F, 0x49, 0xB8, 0x00},
        {0x5F, 0x9C, 0x95, 0xBC, 0xA3, 0x50, 0x8C, 0x24,
         0xB1, 0xD0, 0xB1, 0x55, 0x9C, 0x83, 0xEF, 0x5B,
         0x04, 0x44, 0x5C, 0xC4, 0x58, 0x1C, 0x8E, 0x86,
         0xD8, 0x22, 0x4E, 0xDD, 0xD0, 0x9F, 0x11, 0x57},
        {0xEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F}
    };

    // Check each of the weak points in turn.  We perform the
    // comparisons carefully so as not to reveal the value of "k"
    // in the instruction timing.  If "k" is indeed weak then
    // we still check everything so as not to reveal which
    // weak point it is.
    uint8_t result = 0;
    for (uint8_t posn = 0; posn < 5; ++posn) {
        const uint8_t *point = points[posn];
        uint8_t check = (pgm_read_byte(point + 31) ^ k[31]) & 0x7F;
        for (uint8_t index = 31; index > 0; --index)
            check |= (pgm_read_byte(point + index - 1) ^ k[index - 1]);
        result |= (uint8_t)((((uint16_t)0x0100) - check) >> 8);
    }

    // The "result" variable will be non-zero if there was a match.
    return result;
}

/**
 * \brief Reduces a number modulo 2^255 - 19.
 *
 * \param result The array that will contain the result when the
 * function exits.  Must be NUM_LIMBS limbs in size.
 * \param x The number to be reduced, which must be NUM_LIMBS * 2 limbs in
 * size and less than or equal to square(2^255 - 19 - 1).  This array will
 * be modified by the reduction process.
 * \param size The size of the high order half of \a x.  This indicates
 * the size of \a x in limbs.  If it is shorter than NUM_LIMBS then the
 * reduction can be performed quicker.
 */
void Curve25519::reduce(limb_t *result, limb_t *x, uint8_t size)
{
    /*
    Note: This explaination is best viewed with a UTF-8 text viewer.

    To help explain what this function is doing, the following describes
    how to efficiently compute reductions modulo a base of the form (2ⁿ - b)
    where b is greater than zero and (b + 1)² <= 2ⁿ.

    Here we are interested in reducing the result of multiplying two
    numbers that are less than or equal to (2ⁿ - b - 1).  That is,
    multiplying numbers that have already been reduced.

    Given some x less than or equal to (2ⁿ - b - 1)², we want to find a
    y less than (2ⁿ - b) such that:

        y ≡ x mod (2ⁿ - b)

    We know that for all integer values of k >= 0:

        y ≡ x - k * (2ⁿ - b)
          ≡ x - k * 2ⁿ + k * b

    In our case we choose k = ⌊x / 2ⁿ⌋ and then let:

        w = (x mod 2ⁿ) + ⌊x / 2ⁿ⌋ * b

    The value w will either be the answer y or y can be obtained by
    repeatedly subtracting (2ⁿ - b) from w until it is less than (2ⁿ - b).
    At most b subtractions will be required.

    In our case b is 19 which is more subtractions than we would like to do,
    but we can handle that by performing the above reduction twice and then
    performing a single trial subtraction:

        w = (x mod 2ⁿ) + ⌊x / 2ⁿ⌋ * b
        y = (w mod 2ⁿ) + ⌊w / 2ⁿ⌋ * b
        if y >= (2ⁿ - b)
            y -= (2ⁿ - b)

    The value y is the answer we want for reducing x modulo (2ⁿ - b).
    */

    dlimb_t carry;
    uint8_t posn;

    // Calculate (x mod 2^255) + ((x / 2^255) * 19) which will
    // either produce the answer we want or it will produce a
    // value of the form "answer + j * (2^255 - 19)".
    carry = ((dlimb_t)(x[NUM_LIMBS - 1] >> (LIMB_BITS - 1))) * 19U;
    x[NUM_LIMBS - 1] &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
    for (posn = 0; posn < size; ++posn) {
        carry += ((dlimb_t)(x[posn + NUM_LIMBS])) * 38U;
        carry += x[posn];
        x[posn] = (limb_t)carry;
        carry >>= LIMB_BITS;
    }
    if (size < NUM_LIMBS) {
        // The high order half of the number is short; e.g. for mulA24().
        // Propagate the carry through the rest of the low order part.
        for (posn = size; posn < NUM_LIMBS; ++posn) {
            carry += x[posn];
            x[posn] = (limb_t)carry;
            carry >>= LIMB_BITS;
        }
    }

    // The "j" value may still be too large due to the final carry-out.
    // We must repeat the reduction.  If we already have the answer,
    // then this won't do any harm but we must still do the calculation
    // to preserve the overall timing.
    carry *= 38U;
    carry += ((dlimb_t)(x[NUM_LIMBS - 1] >> (LIMB_BITS - 1))) * 19U;
    x[NUM_LIMBS - 1] &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        carry += x[posn];
        x[posn] = (limb_t)carry;
        carry >>= LIMB_BITS;
    }

    // At this point "x" will either be the answer or it will be the
    // answer plus (2^255 - 19).  Perform a trial subtraction which
    // is equivalent to adding 19 and subtracting 2^255.  We put the
    // trial answer into the top-most limbs of the original "x" array.
    // We add 19 here; the subtraction of 2^255 occurs in the next step.
    carry = 19U;
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        carry += x[posn];
        x[posn + NUM_LIMBS] = (limb_t)carry;
        carry >>= LIMB_BITS;
    }

    // If there was a borrow, then the bottom-most limbs of "x" are the
    // correct answer.  If there was no borrow, then the top-most limbs
    // of "x" are the correct answer.  Select the correct answer but do
    // it in a way that instruction timing will not reveal which value
    // was selected.  Borrow will occur if the high bit of the previous
    // result is 0: turn the high bit into a selection mask.
    limb_t mask = (limb_t)(((slimb_t)(x[NUM_LIMBS * 2 - 1])) >> (LIMB_BITS - 1));
    limb_t nmask = ~mask;
    x[NUM_LIMBS * 2 - 1] &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        result[posn] = (x[posn] & nmask) | (x[posn + NUM_LIMBS] & mask);
    }
}

/**
 * \brief Quickly reduces a number modulo 2^255 - 19.
 *
 * \param x The number to be reduced, which must be NUM_LIMBS limbs in size
 * and less than or equal to 2 * (2^255 - 19 - 1).
 * \return Zero if \a x was greater than or equal to (2^255 - 19).
 *
 * The answer is also put into \a x and will consist of NUM_LIMBS limbs.
 *
 * This function is intended for reducing the result of additions where
 * the caller knows that \a x is within the described range.  A single
 * trial subtraction is all that is needed to reduce the number.
 */
limb_t Curve25519::reduceQuick(limb_t *x)
{
    limb_t temp[NUM_LIMBS];
    dlimb_t carry;
    uint8_t posn;
    limb_t *xx;
    limb_t *tt;
    
    // Perform a trial subtraction of (2^255 - 19) from "x" which is
    // equivalent to adding 19 and subtracting 2^255.  We add 19 here;
    // the subtraction of 2^255 occurs in the next step.
    carry = 19U;
    xx = x;
    tt = temp;
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        carry += *xx++;
        *tt++ = (limb_t)carry;
        carry >>= LIMB_BITS;
    }

    // If there was a borrow, then the original "x" is the correct answer.
    // If there was no borrow, then "temp" is the correct answer.  Select the
    // correct answer but do it in a way that instruction timing will not
    // reveal which value was selected.  Borrow will occur if the high bit
    // of "temp" is 0: turn the high bit into a selection mask.
    limb_t mask = (limb_t)(((slimb_t)(temp[NUM_LIMBS - 1])) >> (LIMB_BITS - 1));
    limb_t nmask = ~mask;
    temp[NUM_LIMBS - 1] &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
    xx = x;
    tt = temp;
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        *xx = ((*xx) & nmask) | ((*tt++) & mask);
        ++xx;
    }

    // Clean up "temp".
    strict_clean(temp);

    // Return a zero value if we actually subtracted (2^255 - 19) from "x".
    return nmask;
}

/**
 * \brief Multiplies two values and then reduces the result modulo 2^255 - 19.
 *
 * \param result The result, which must be NUM_LIMBS limbs in size and can
 * be the same array as \a x or \a y.
 * \param x The first value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 * \param y The second value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.  This can be the same array as \a x.
 */
void Curve25519::mul(limb_t *result, const limb_t *x, const limb_t *y)
{
    limb_t temp[NUM_LIMBS * 2];
    uint8_t i, j;
    dlimb_t carry;
    limb_t word;
    const limb_t *yy;
    limb_t *tt;

    // Multiply the lowest word of x by y.
    carry = 0;
    word = x[0];
    yy = y;
    tt = temp;
    for (i = 0; i < NUM_LIMBS; ++i) {
        carry += ((dlimb_t)(*yy++)) * word;
        *tt++ = (limb_t)carry;
        carry >>= LIMB_BITS;
    }
    *tt = (limb_t)carry;

    // Multiply and add the remaining words of x by y.
    for (i = 1; i < NUM_LIMBS; ++i) {
        word = x[i];
        carry = 0;
        yy = y;
        tt = temp + i;
        for (j = 0; j < NUM_LIMBS; ++j) {
            carry += ((dlimb_t)(*yy++)) * word;
            carry += *tt;
            *tt++ = (limb_t)carry;
            carry >>= LIMB_BITS;
        }
        *tt = (limb_t)carry;
    }

    // Reduce the intermediate result modulo 2^255 - 19.
    reduce(result, temp, NUM_LIMBS);
    strict_clean(temp);
}

/**
 * \fn void Curve25519::square(limb_t *result, const limb_t *x)
 * \brief Squares a value and then reduces it modulo 2^255 - 19.
 *
 * \param result The result, which must be NUM_LIMBS limbs in size and
 * can be the same array as \a x.
 * \param x The value to square, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 */

/**
 * \brief Multiplies a value by the a24 constant and then reduces the result
 * modulo 2^255 - 19.
 *
 * \param result The result, which must be NUM_LIMBS limbs in size and can
 * be the same array as \a x.
 * \param x The value to multiply by a24, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 */
void Curve25519::mulA24(limb_t *result, const limb_t *x)
{
    // The constant a24 = 121665 (0x1DB41) as a limb array.
#if BIGNUMBER_LIMB_8BIT
    static limb_t const a24[3] PROGMEM = {0x41, 0xDB, 0x01};
    #define pgm_read_a24(index) (pgm_read_byte(&(a24[(index)])))
#elif BIGNUMBER_LIMB_16BIT
    static limb_t const a24[2] PROGMEM = {0xDB41, 0x0001};
    #define pgm_read_a24(index) (pgm_read_word(&(a24[(index)])))
#elif BIGNUMBER_LIMB_32BIT
    static limb_t const a24[1] PROGMEM = {0x0001DB41};
    #define pgm_read_a24(index) (pgm_read_dword(&(a24[(index)])))
#else
    #error "limb_t must be 8, 16, or 32 bits in size"
#endif
    #define NUM_A24_LIMBS   (sizeof(a24) / sizeof(limb_t))

    // Multiply the lowest limb of a24 by x and zero-extend into the result.
    limb_t temp[NUM_LIMBS * 2];
    uint8_t i, j;
    dlimb_t carry = 0;
    limb_t word = pgm_read_a24(0);
    const limb_t *xx = x;
    limb_t *tt = temp;
    for (i = 0; i < NUM_LIMBS; ++i) {
        carry += ((dlimb_t)(*xx++)) * word;
        *tt++ = (limb_t)carry;
        carry >>= LIMB_BITS;
    }
    *tt = (limb_t)carry;

    // Multiply and add the remaining limbs of a24.
    for (i = 1; i < NUM_A24_LIMBS; ++i) {
        word = pgm_read_a24(i);
        carry = 0;
        xx = x;
        tt = temp + i;
        for (j = 0; j < NUM_LIMBS; ++j) {
            carry += ((dlimb_t)(*xx++)) * word;
            carry += *tt;
            *tt++ = (limb_t)carry;
            carry >>= LIMB_BITS;
        }
        *tt = (limb_t)carry;
    }

    // Reduce the intermediate result modulo 2^255 - 19.
    reduce(result, temp, NUM_A24_LIMBS);
    strict_clean(temp);
}

/**
 * \brief Adds two values and then reduces the result modulo 2^255 - 19.
 *
 * \param result The result, which must be NUM_LIMBS limbs in size and can
 * be the same array as \a x or \a y.
 * \param x The first value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 * \param y The second value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 */
void Curve25519::add(limb_t *result, const limb_t *x, const limb_t *y)
{
    dlimb_t carry = 0;
    uint8_t posn;
    limb_t *rr = result;

    // Add the two arrays to obtain the intermediate result.
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        carry += *x++;
        carry += *y++;
        *rr++ = (limb_t)carry;
        carry >>= LIMB_BITS;
    }

    // Reduce the result using the quick trial subtraction method.
    reduceQuick(result);
}

/**
 * \brief Subtracts two values and then reduces the result modulo 2^255 - 19.
 *
 * \param result The result, which must be NUM_LIMBS limbs in size and can
 * be the same array as \a x or \a y.
 * \param x The first value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 * \param y The second value to multiply, which must be NUM_LIMBS limbs in size
 * and less than 2^255 - 19.
 */
void Curve25519::sub(limb_t *result, const limb_t *x, const limb_t *y)
{
    dlimb_t borrow;
    uint8_t posn;
    limb_t *rr = result;

    // Subtract y from x to generate the intermediate result.
    borrow = 0;
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        borrow = ((dlimb_t)(*x++)) - (*y++) - ((borrow >> LIMB_BITS) & 0x01);
        *rr++ = (limb_t)borrow;
    }

    // If we had a borrow, then the result has gone negative and we
    // have to add 2^255 - 19 to the result to make it positive again.
    // The top bits of "borrow" will be all 1's if there is a borrow
    // or it will be all 0's if there was no borrow.  Easiest is to
    // conditionally subtract 19 and then mask off the high bit.
    rr = result;
    borrow = (borrow >> LIMB_BITS) & 19U;
    borrow = ((dlimb_t)(*rr)) - borrow;
    *rr++ = (limb_t)borrow;
    for (posn = 1; posn < NUM_LIMBS; ++posn) {
        borrow = ((dlimb_t)(*rr)) - ((borrow >> LIMB_BITS) & 0x01);
        *rr++ = (limb_t)borrow;
    }
    *(--rr) &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
}

/**
 * \brief Conditionally swaps two values if a selection value is non-zero.
 *
 * \param select Non-zero to swap \a x and \a y, zero to leave them unchanged.
 * \param x The first value to conditionally swap.
 * \param y The second value to conditionally swap.
 *
 * The swap is performed in a way that it should take the same amount of
 * time irrespective of the value of \a select.
 */
void Curve25519::cswap(uint8_t select, limb_t *x, limb_t *y)
{
    uint8_t posn;
    limb_t dummy;
    limb_t sel;

    // Turn "select" into an all-zeroes or all-ones mask.  We don't care
    // which bit or bits is set in the original "select" value.
    sel = (limb_t)(((((dlimb_t)1) << LIMB_BITS) - select) >> LIMB_BITS);
    --sel;

    // Swap the two values based on "select".  Algorithm from:
    // https://tools.ietf.org/html/draft-irtf-cfrg-curves-02
    for (posn = 0; posn < NUM_LIMBS; ++posn) {
        dummy = sel & (x[posn] ^ y[posn]);
        x[posn] ^= dummy;
        y[posn] ^= dummy;
    }
}

/**
 * \brief Computes the reciprocal of a number modulo 2^255 - 19.
 *
 * \param result The result as a array of NUM_LIMBS limbs in size.  This can
 * be the same array as \a x.
 * \param x The number to compute the reciprocal for.
 */
void Curve25519::recip(limb_t *result, const limb_t *x)
{
    limb_t t1[NUM_LIMBS];
    uint8_t i, j;

    // The reciprocal is the same as x ^ (p - 2) where p = 2^255 - 19.
    // The big-endian hexadecimal expansion of (p - 2) is:
    // 7FFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFEB
    //
    // The naive implementation needs to do 2 multiplications per 1 bit and
    // 1 multiplication per 0 bit.  We can improve upon this by creating a
    // pattern 0000000001 ... 0000000001.  If we square and multiply the
    // pattern by itself we can turn the pattern into the partial results
    // 0000000011 ... 0000000011, 0000000111 ... 0000000111, etc.
    // This averages out to about 1.1 multiplications per 1 bit instead of 2.

    // Build a pattern of 250 bits in length of repeated copies of 0000000001.
    #define RECIP_GROUP_SIZE 10
    #define RECIP_GROUP_BITS 250    // Must be a multiple of RECIP_GROUP_SIZE.
    square(t1, x);
    for (j = 0; j < (RECIP_GROUP_SIZE - 1); ++j)
        square(t1, t1);
    mul(result, t1, x);
    for (i = 0; i < ((RECIP_GROUP_BITS / RECIP_GROUP_SIZE) - 2); ++i) {
        for (j = 0; j < RECIP_GROUP_SIZE; ++j)
            square(t1, t1);
        mul(result, result, t1);
    }

    // Multiply bit-shifted versions of the 0000000001 pattern into
    // the result to "fill in" the gaps in the pattern.
    square(t1, result);
    mul(result, result, t1);
    for (j = 0; j < (RECIP_GROUP_SIZE - 2); ++j) {
        square(t1, t1);
        mul(result, result, t1);
    }

    // Deal with the 5 lowest bits of (p - 2), 01011, from highest to lowest.
    square(result, result);
    square(result, result);
    mul(result, result, x);
    square(result, result);
    square(result, result);
    mul(result, result, x);
    square(result, result);
    mul(result, result, x);

    // Clean up and exit.
    clean(t1);
}

/**
 * \brief Unpacks the little-endian byte representation of a field element
 * into a limb array.
 *
 * \param result The limb array.
 * \param x The byte representation.
 *
 * The top-most bit of \a result will be set to zero so that the value
 * is guaranteed to be 255 bits rather than 256.
 *
 * \sa pack()
 */
void Curve25519::unpack(limb_t *result, const uint8_t *x)
{
#if BIGNUMBER_LIMB_8BIT
    memcpy(result, x, 32);
    result[31] &= 0x7F;
#elif BIGNUMBER_LIMB_16BIT
    for (uint8_t posn = 0; posn < 16; ++posn) {
        result[posn] = ((limb_t)x[posn * 2]) | (((limb_t)x[posn * 2 + 1]) << 8);
    }
    result[15] &= 0x7FFF;
#elif BIGNUMBER_LIMB_32BIT
    for (uint8_t posn = 0; posn < 8; ++posn) {
        result[posn] = ((limb_t)x[posn * 4]) |
                      (((limb_t)x[posn * 4 + 1]) << 8) |
                      (((limb_t)x[posn * 4 + 2]) << 16) |
                      (((limb_t)x[posn * 4 + 3]) << 24);
    }
    result[7] &= 0x7FFFFFFF;
#endif
}

/**
 * \brief Packs the limb array representation of a field element into a
 * byte array.
 *
 * \param result The byte array.
 * \param x The limb representation.
 *
 * \sa unpack()
 */
void Curve25519::pack(uint8_t *result, const limb_t *x)
{
#if BIGNUMBER_LIMB_8BIT
    memcpy(result, x, 32);
#elif BIGNUMBER_LIMB_16BIT
    for (uint8_t posn = 0; posn < 16; ++posn) {
        limb_t value = x[posn];
        result[posn * 2]     = (uint8_t)value;
        result[posn * 2 + 1] = (uint8_t)(value >> 8);
    }
#elif BIGNUMBER_LIMB_32BIT
    for (uint8_t posn = 0; posn < 8; ++posn) {
        limb_t value = x[posn];
        result[posn * 4]     = (uint8_t)value;
        result[posn * 4 + 1] = (uint8_t)(value >> 8);
        result[posn * 4 + 2] = (uint8_t)(value >> 16);
        result[posn * 4 + 3] = (uint8_t)(value >> 24);
    }
#endif
}
