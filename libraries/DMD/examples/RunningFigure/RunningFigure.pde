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

#include <DMD.h>

DMD display;

// Running stick figure pictures are loosely based on those from this tutorial:
// http://www.fluidanims.com/FAelite/phpBB3/viewtopic.php?f=10&t=102

byte const run1[] PROGMEM = {
    16, 16,
    B00000000, B00001100,
    B00000000, B00011110,
    B00000111, B11111110,
    B00001111, B11111110,
    B00011100, B11111100,
    B00000001, B11111100,
    B00000001, B11110000,
    B00000011, B11111000,
    B00000111, B00011000,
    B00001110, B01110000,
    B00011100, B01100000,
    B00111000, B00000000,
    B01110000, B00000000,
    B01100000, B00000000,
    B01000000, B00000000,
    B00000000, B00000000
};

byte const run2[] PROGMEM = {
    18, 16,
    B00000000, B01110011, B10000000,
    B00000000, B11111111, B10000000,
    B00000000, B00011111, B10000000,
    B00000000, B00111111, B11000000,
    B00000000, B01111011, B11000000,
    B00000000, B11110011, B10000000,
    B00000001, B11100000, B00000000,
    B00000011, B11100000, B00000000,
    B00000111, B01110000, B00000000,
    B01111110, B00111000, B00000000,
    B11111100, B00011100, B00000000,
    B00000000, B00001110, B00000000,
    B00000000, B00000111, B00000000,
    B00000000, B00000011, B10000000,
    B00000000, B00000001, B00000000,
    B00000000, B00000000, B00000000
};

byte const run3[] PROGMEM = {
    18, 16,
    B00000000, B00110000, B00000000,
    B00000000, B01111000, B00000000,
    B00000000, B00011111, B00000000,
    B00000000, B00011111, B00000000,
    B00000000, B00111111, B10000000,
    B00000000, B01111111, B11000000,
    B00000000, B11100011, B10000000,
    B00000001, B11000000, B00000000,
    B00000011, B11100000, B00000000,
    B11111111, B01110000, B00000000,
    B11111110, B00111000, B00000000,
    B00000000, B00011000, B00000000,
    B00000000, B00011100, B00000000,
    B00000000, B00001110, B00000000,
    B00000000, B00000100, B00000000,
    B00000000, B00000000, B00000000
};

byte const run4[] PROGMEM = {
    16, 16,
    B00000001, B11100000,
    B00000011, B11111100,
    B00000000, B00111110,
    B00000000, B01111110,
    B00000000, B11111100,
    B00000001, B10011111,
    B00000011, B00001110,
    B00000011, B00000000,
    B00000011, B10000000,
    B11111111, B10000000,
    B11111000, B11000000,
    B00000001, B11000000,
    B00000011, B10000000,
    B00000111, B00000000,
    B00000110, B00000000,
    B00000100, B00000000
};

const prog_uint8_t *frames[] = {
    run1,
    run2,
    run3,
    run4
};
#define NUM_FRAMES  (sizeof(frames) / sizeof(frames[0]))
#define ADVANCE_MS  (1000 / NUM_FRAMES)
unsigned int frame = 0;
unsigned long lastFrame;

void setup() {
    display.drawBitmap(8, 0, run1);
    lastFrame = millis();
}

void loop() {
    if ((millis() - lastFrame) >= ADVANCE_MS) {
        frame = (frame + 1) % NUM_FRAMES;
        display.clear();
        int x = (32 - pgm_read_byte(frames[frame])) / 2;
        display.drawBitmap(x, 0, frames[frame]);
        lastFrame += ADVANCE_MS;
    }
    display.loop();
}
