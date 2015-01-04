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

#endif
