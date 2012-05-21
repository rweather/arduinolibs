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

// Utility for generating the button mapping table in FreetronicsLCD.cpp.

#include <stdio.h>

#define LCD_BUTTON_NONE         0
#define LCD_BUTTON_LEFT         1
#define LCD_BUTTON_RIGHT        2
#define LCD_BUTTON_UP           3
#define LCD_BUTTON_DOWN         4
#define LCD_BUTTON_SELECT       5

#define LCD_BUTTON_VALUE_GAP    10
#define LCD_BUTTON_RIGHT_VALUE  0
#define LCD_BUTTON_UP_VALUE     145
#define LCD_BUTTON_DOWN_VALUE   329
#define LCD_BUTTON_LEFT_VALUE   505
#define LCD_BUTTON_SELECT_VALUE 741

int main(int argc, char *argv[])
{
    char rawTest[1024];
    int value, value2, value3, bits;
    char button;

    // Determine the actual values for each of the 1024 inputs.
    for (value = 0; value < 1024; ++value) {
        if (value < (LCD_BUTTON_RIGHT_VALUE + LCD_BUTTON_VALUE_GAP))
            button = LCD_BUTTON_RIGHT;
        else if (value >= (LCD_BUTTON_UP_VALUE - LCD_BUTTON_VALUE_GAP) &&
                 value <= (LCD_BUTTON_UP_VALUE + LCD_BUTTON_VALUE_GAP))
            button = LCD_BUTTON_UP;
        else if (value >= (LCD_BUTTON_DOWN_VALUE - LCD_BUTTON_VALUE_GAP) &&
                 value <= (LCD_BUTTON_DOWN_VALUE + LCD_BUTTON_VALUE_GAP))
            button = LCD_BUTTON_DOWN;
        else if (value >= (LCD_BUTTON_LEFT_VALUE - LCD_BUTTON_VALUE_GAP) &&
                 value <= (LCD_BUTTON_LEFT_VALUE + LCD_BUTTON_VALUE_GAP))
            button = LCD_BUTTON_LEFT;
        else if (value >= (LCD_BUTTON_SELECT_VALUE - LCD_BUTTON_VALUE_GAP) &&
                 value <= (LCD_BUTTON_SELECT_VALUE + LCD_BUTTON_VALUE_GAP))
            button = LCD_BUTTON_SELECT;
        else
            button = LCD_BUTTON_NONE;
        rawTest[value] = button;
    }

    // Dump the accuracy of different bit truncations.
    for (bits = 4; bits < 10; ++bits) {
        int count = 0;
        for (value = 0; value < 1024; ++value) {
            value2 = value & (((1 << bits) - 1) << (10 - bits));
            if (rawTest[value] == rawTest[value2])
                ++count;
        }
        printf("bits = %d, accuracy = %g%%\n", bits, 100.0 * count / 1024.0);
    }
    printf("\n");

    // Dump the button mapping table for the selected bit count.
    bits = 5;
    printf("static prog_uint8_t const buttonMappings[] PROGMEM = {\n");
    for (value2 = 0; value2 < (1 << bits); ++value2) {
        value = value2 << (10 - bits);
        value3 = value + (1 << (10 - bits)) - 1;
        button = 0;
        while (!button && value <= value3) {
            button = rawTest[value];
            ++value;
        }
        if ((value2 & 0x0F) != 0)
            printf(", ");
        else
            printf("    ");
        printf("%d", button);
        if ((value2 & 0x0F) == 0x0F) {
            if (value2 < ((1 << bits - 1)))
                printf(",\n");
            else
                printf("\n");
        }
    }
    printf("};\n");
    printf("#define mapButton(value) (pgm_read_byte(&(buttonMappings[(value) >> %d])))\n", 10 - bits);

    return 0;
}
