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

#ifndef PowerSave_h
#define PowerSave_h

#include <WProgram.h>

inline void unusedPin(uint8_t pin)
{
    pinMode(pin, INPUT);
    digitalWrite(pin, HIGH);
}

enum SleepDuration
{
    SLEEP_15_MS,
    SLEEP_30_MS,
    SLEEP_60_MS,
    SLEEP_120_MS,
    SLEEP_250_MS,
    SLEEP_500_MS,
    SLEEP_1_SEC,
    SLEEP_2_SEC,
    SLEEP_4_SEC,
    SLEEP_8_SEC
};

void sleepFor(SleepDuration duration, uint8_t mode = 0);

#endif
