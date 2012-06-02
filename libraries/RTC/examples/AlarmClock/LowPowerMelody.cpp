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

#include "LowPowerMelody.h"
#include <avr/power.h>

extern void findNextAlarm();
extern void turnRadioOn();
extern void turnRadioOff();

bool LowPowerMelody::isPlaying() const
{
    if (radioMode)
        return radioAlarmActive;
    else
        return Melody::isPlaying();
}

void LowPowerMelody::play()
{
    if (radioMode) {
        turnRadioOn();
        radioAlarmActive = true;
        return;
    }

    // Turn on Timer2.
    power_timer2_enable();

    // Start the melody playing.
    Melody::play();
}

void LowPowerMelody::playOnce()
{
    if (radioMode)
        return;

    // Turn on Timer2.
    power_timer2_enable();

    // Start the melody playing.
    Melody::playOnce();
}

void LowPowerMelody::stop()
{
    if (radioMode) {
        turnRadioOff();
        radioAlarmActive = false;
        return;
    }

    // Stop the melody playing.
    Melody::stop();

    // Turn off Timer2.
    power_timer2_disable();

    // Find the next alarm to be triggered.
    findNextAlarm();
}
