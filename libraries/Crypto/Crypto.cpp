/*
 * Copyright (C) 2014 Southern Storm Software, Pty Ltd.
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

#include "Crypto.h"

/**
 * \brief Cleans a block of bytes.
 *
 * \param dest The destination block to be cleaned.
 * \param size The size of the destination to be cleaned in bytes.
 *
 * Unlike memset(), this function attempts to prevent the compiler
 * from optimizing away the clear on a memory buffer.
 */
void clean(void *dest, size_t size)
{
    // Force the use of volatile so that we actually clear the memory.
    // Otherwise the compiler might optimise the entire contents of this
    // function away, which will not be secure.
    volatile uint8_t *d = (volatile uint8_t *)dest;
    while (size > 0) {
        *d++ = 0;
        --size;
    }
}

/**
 * \fn void clean(T &var)
 * \brief Template function that cleans a variable.
 *
 * \param var A reference to the variable to clean.
 *
 * The variable will be cleared to all-zeroes in a secure manner.
 * Unlike memset(), this function attempts to prevent the compiler
 * from optimizing away the variable clear.
 */
