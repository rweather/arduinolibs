/*
 * Copyright (C) 2012 Southern Storm Software, Pty Ltd.
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

#include <stdio.h>

int main(int argc, char *argv[])
{
    int value;
    printf("static const uint8_t flipBits[256] PROGMEM = {\n");
    for (value = 0; value < 256; ++value) {
        int flipped =
            ((value & 0x01) << 7) | ((value & 0x02) << 5) |
            ((value & 0x04) << 3) | ((value & 0x08) << 1) |
            ((value & 0x10) >> 1) | ((value & 0x20) >> 3) |
            ((value & 0x40) >> 5) | ((value & 0x80) >> 7);
        if ((value % 12) == 0)
            printf("    ");
        else
            printf(" ");
        if (value != 255)
            printf("0x%02X,", flipped);
        else
            printf("0x%02X", flipped);
        if ((value % 12) == 11)
            printf("\n");
    }
    printf("\n};\n");
    return 0;
}
