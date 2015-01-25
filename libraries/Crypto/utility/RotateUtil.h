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

#ifndef CRYPTO_ROTATEUTIL_H
#define CRYPTO_ROTATEUTIL_H

#include <inttypes.h>

#if defined(__AVR__)

// Rotation functions that are optimised for best performance on AVR.
// The most efficient rotations are where the number of bits is 1 or a
// multiple of 8, so we compose the efficient rotations to produce all
// other rotation counts of interest.  All macros below assume arguments
// of 32 bits in size.

// Generic left rotate - best performance when "bits" is 1 or a multiple of 8.
#define leftRotate(a, bits) \
    (__extension__ ({ \
        uint32_t _temp = (a); \
        (_temp << (bits)) | (_temp >> (32 - (bits))); \
    }))

// Generic right rotate - best performance when "bits" is 1 or a multiple of 8.
#define rightRotate(a, bits) \
    (__extension__ ({ \
        uint32_t _temp = (a); \
        (_temp >> (bits)) | (_temp << (32 - (bits))); \
    }))

// Left rotate by 1.
#define leftRotate1(a)  (leftRotate((a), 1))

// Left rotate by 2.
#define leftRotate2(a)  (leftRotate(leftRotate((a), 1), 1))

// Left rotate by 3.
#define leftRotate3(a)  (leftRotate(leftRotate(leftRotate((a), 1), 1), 1))

// Left rotate by 4.
#define leftRotate4(a)  (leftRotate(leftRotate(leftRotate(leftRotate((a), 1), 1), 1), 1))

// Left rotate by 5: Rotate left by 8, then right by 3.
#define leftRotate5(a)  (rightRotate(rightRotate(rightRotate(leftRotate((a), 8), 1), 1), 1))

// Left rotate by 6: Rotate left by 8, then right by 2.
#define leftRotate6(a)  (rightRotate(rightRotate(leftRotate((a), 8), 1), 1))

// Left rotate by 7: Rotate left by 8, then right by 1.
#define leftRotate7(a)  (rightRotate(leftRotate((a), 8), 1))

// Left rotate by 8.
#define leftRotate8(a)  (leftRotate((a), 8))

// Left rotate by 9: Rotate left by 8, then left by 1.
#define leftRotate9(a)  (leftRotate(leftRotate((a), 8), 1))

// Left rotate by 10: Rotate left by 8, then left by 2.
#define leftRotate10(a) (leftRotate(leftRotate(leftRotate((a), 8), 1), 1))

// Left rotate by 11: Rotate left by 8, then left by 3.
#define leftRotate11(a) (leftRotate(leftRotate(leftRotate(leftRotate((a), 8), 1), 1), 1))

// Left rotate by 12: Rotate left by 16, then right by 4.
#define leftRotate12(a) (rightRotate(rightRotate(rightRotate(rightRotate(leftRotate((a), 16), 1), 1), 1), 1))

// Left rotate by 13: Rotate left by 16, then right by 3.
#define leftRotate13(a) (rightRotate(rightRotate(rightRotate(leftRotate((a), 16), 1), 1), 1))

// Left rotate by 14: Rotate left by 16, then right by 2.
#define leftRotate14(a) (rightRotate(rightRotate(leftRotate((a), 16), 1), 1))

// Left rotate by 15: Rotate left by 16, then right by 1.
#define leftRotate15(a) (rightRotate(leftRotate((a), 16), 1))

// Left rotate by 16.
#define leftRotate16(a) (leftRotate((a), 16))

// Left rotate by 17: Rotate left by 16, then left by 1.
#define leftRotate17(a) (leftRotate(leftRotate((a), 16), 1))

// Left rotate by 18: Rotate left by 16, then left by 2.
#define leftRotate18(a) (leftRotate(leftRotate(leftRotate((a), 16), 1), 1))

// Left rotate by 19: Rotate left by 16, then left by 3.
#define leftRotate19(a) (leftRotate(leftRotate(leftRotate(leftRotate((a), 16), 1), 1), 1))

// Left rotate by 20: Rotate left by 16, then left by 4.
#define leftRotate20(a) (leftRotate(leftRotate(leftRotate(leftRotate(leftRotate((a), 16), 1), 1), 1), 1))

// Left rotate by 21: Rotate left by 24, then right by 3.
#define leftRotate21(a) (rightRotate(rightRotate(rightRotate(leftRotate((a), 24), 1), 1), 1))

// Left rotate by 22: Rotate left by 24, then right by 2.
#define leftRotate22(a) (rightRotate(rightRotate(leftRotate((a), 24), 1), 1))

// Left rotate by 23: Rotate left by 24, then right by 1.
#define leftRotate23(a) (rightRotate(leftRotate((a), 24), 1))

// Left rotate by 24.
#define leftRotate24(a) (leftRotate((a), 24))

// Left rotate by 25: Rotate left by 24, then left by 1.
#define leftRotate25(a) (leftRotate(leftRotate((a), 24), 1))

// Left rotate by 26: Rotate left by 24, then left by 2.
#define leftRotate26(a) (leftRotate(leftRotate(leftRotate((a), 24), 1), 1))

// Left rotate by 27: Rotate left by 24, then left by 3.
#define leftRotate27(a) (leftRotate(leftRotate(leftRotate(leftRotate((a), 24), 1), 1), 1))

// Left rotate by 28: Rotate right by 4.
#define leftRotate28(a) (rightRotate(rightRotate(rightRotate(rightRotate((a), 1), 1), 1), 1))

// Left rotate by 29: Rotate right by 3.
#define leftRotate29(a) (rightRotate(rightRotate(rightRotate((a), 1), 1), 1))

// Left rotate by 30: Rotate right by 2.
#define leftRotate30(a) (rightRotate(rightRotate((a), 1), 1))

// Left rotate by 31: Rotate right by 1.
#define leftRotate31(a) (rightRotate((a), 1))

// Define the right rotations in terms of left rotations.
#define rightRotate1(a)  (leftRotate31((a)))
#define rightRotate2(a)  (leftRotate30((a)))
#define rightRotate3(a)  (leftRotate29((a)))
#define rightRotate4(a)  (leftRotate28((a)))
#define rightRotate5(a)  (leftRotate27((a)))
#define rightRotate6(a)  (leftRotate26((a)))
#define rightRotate7(a)  (leftRotate25((a)))
#define rightRotate8(a)  (leftRotate24((a)))
#define rightRotate9(a)  (leftRotate23((a)))
#define rightRotate10(a) (leftRotate22((a)))
#define rightRotate11(a) (leftRotate21((a)))
#define rightRotate12(a) (leftRotate20((a)))
#define rightRotate13(a) (leftRotate19((a)))
#define rightRotate14(a) (leftRotate18((a)))
#define rightRotate15(a) (leftRotate17((a)))
#define rightRotate16(a) (leftRotate16((a)))
#define rightRotate17(a) (leftRotate15((a)))
#define rightRotate18(a) (leftRotate14((a)))
#define rightRotate19(a) (leftRotate13((a)))
#define rightRotate20(a) (leftRotate12((a)))
#define rightRotate21(a) (leftRotate11((a)))
#define rightRotate22(a) (leftRotate10((a)))
#define rightRotate23(a) (leftRotate9((a)))
#define rightRotate24(a) (leftRotate8((a)))
#define rightRotate25(a) (leftRotate7((a)))
#define rightRotate26(a) (leftRotate6((a)))
#define rightRotate27(a) (leftRotate5((a)))
#define rightRotate28(a) (leftRotate4((a)))
#define rightRotate29(a) (leftRotate3((a)))
#define rightRotate30(a) (leftRotate2((a)))
#define rightRotate31(a) (leftRotate1((a)))

#else

// Generic rotation functions.  All bit shifts are considered to have
// similar performance.  Usually true of 32-bit and higher platforms.

// Generic left rotate.
#define leftRotate(a, bits) \
    (__extension__ ({ \
        uint32_t _temp = (a); \
        (_temp << (bits)) | (_temp >> (32 - (bits))); \
    }))

// Generic right rotate.
#define rightRotate(a, bits) \
    (__extension__ ({ \
        uint32_t _temp = (a); \
        (_temp >> (bits)) | (_temp << (32 - (bits))); \
    }))

// Left rotate by a specific number of bits.
#define leftRotate1(a)  (leftRotate((a), 1))
#define leftRotate2(a)  (leftRotate((a), 2))
#define leftRotate3(a)  (leftRotate((a), 3))
#define leftRotate4(a)  (leftRotate((a), 4))
#define leftRotate5(a)  (leftRotate((a), 5))
#define leftRotate6(a)  (leftRotate((a), 6))
#define leftRotate7(a)  (leftRotate((a), 7))
#define leftRotate8(a)  (leftRotate((a), 8))
#define leftRotate9(a)  (leftRotate((a), 9))
#define leftRotate10(a) (leftRotate((a), 10))
#define leftRotate11(a) (leftRotate((a), 11))
#define leftRotate12(a) (leftRotate((a), 12))
#define leftRotate13(a) (leftRotate((a), 13))
#define leftRotate14(a) (leftRotate((a), 14))
#define leftRotate15(a) (leftRotate((a), 15))
#define leftRotate16(a) (leftRotate((a), 16))
#define leftRotate17(a) (leftRotate((a), 17))
#define leftRotate18(a) (leftRotate((a), 18))
#define leftRotate19(a) (leftRotate((a), 19))
#define leftRotate20(a) (leftRotate((a), 20))
#define leftRotate21(a) (leftRotate((a), 21))
#define leftRotate22(a) (leftRotate((a), 22))
#define leftRotate23(a) (leftRotate((a), 23))
#define leftRotate24(a) (leftRotate((a), 24))
#define leftRotate25(a) (leftRotate((a), 25))
#define leftRotate26(a) (leftRotate((a), 26))
#define leftRotate27(a) (leftRotate((a), 27))
#define leftRotate28(a) (leftRotate((a), 28))
#define leftRotate29(a) (leftRotate((a), 29))
#define leftRotate30(a) (leftRotate((a), 30))
#define leftRotate31(a) (leftRotate((a), 31))

// Right rotate by a specific number of bits.
#define rightRotate1(a)  (rightRotate((a), 1))
#define rightRotate2(a)  (rightRotate((a), 2))
#define rightRotate3(a)  (rightRotate((a), 3))
#define rightRotate4(a)  (rightRotate((a), 4))
#define rightRotate5(a)  (rightRotate((a), 5))
#define rightRotate6(a)  (rightRotate((a), 6))
#define rightRotate7(a)  (rightRotate((a), 7))
#define rightRotate8(a)  (rightRotate((a), 8))
#define rightRotate9(a)  (rightRotate((a), 9))
#define rightRotate10(a) (rightRotate((a), 10))
#define rightRotate11(a) (rightRotate((a), 11))
#define rightRotate12(a) (rightRotate((a), 12))
#define rightRotate13(a) (rightRotate((a), 13))
#define rightRotate14(a) (rightRotate((a), 14))
#define rightRotate15(a) (rightRotate((a), 15))
#define rightRotate16(a) (rightRotate((a), 16))
#define rightRotate17(a) (rightRotate((a), 17))
#define rightRotate18(a) (rightRotate((a), 18))
#define rightRotate19(a) (rightRotate((a), 19))
#define rightRotate20(a) (rightRotate((a), 20))
#define rightRotate21(a) (rightRotate((a), 21))
#define rightRotate22(a) (rightRotate((a), 22))
#define rightRotate23(a) (rightRotate((a), 23))
#define rightRotate24(a) (rightRotate((a), 24))
#define rightRotate25(a) (rightRotate((a), 25))
#define rightRotate26(a) (rightRotate((a), 26))
#define rightRotate27(a) (rightRotate((a), 27))
#define rightRotate28(a) (rightRotate((a), 28))
#define rightRotate29(a) (rightRotate((a), 29))
#define rightRotate30(a) (rightRotate((a), 30))
#define rightRotate31(a) (rightRotate((a), 31))

#endif

#endif
