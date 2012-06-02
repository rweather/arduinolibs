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

#include "SetMelody.h"
#include "LowPowerMelody.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

extern LowPowerMelody alarmMelody;
extern int defaultMelodyNotes[5];
extern byte defaultMelodyLengths[5];

static int haircutNotes[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3,
    NOTE_REST, NOTE_B3, NOTE_C4, NOTE_REST
};
static byte haircutLengths[] = {4, 8, 8, 4, 4, 4, 4, 4, 2};

static int sosNotes[] = {
    NOTE_C6, NOTE_C6, NOTE_C6, NOTE_REST,
    NOTE_C6, NOTE_C6, NOTE_C6, NOTE_REST,
    NOTE_C6, NOTE_C6, NOTE_C6, NOTE_REST
};
static byte sosLengths[] = {8, 8, 8, 8, 4, 4, 4, 8, 8, 8, 8, 2};

static const char item_FourBeeps[] PROGMEM = "Four beeps";
static const char item_Haircut[] PROGMEM = "Shave 'n haircut";
static const char item_SOS[] PROGMEM = "S.O.S.";
static const char item_Radio[] PROGMEM = "Radio";
static ListItem melodyNames[] PROGMEM = {
    item_FourBeeps,
    item_Haircut,
    item_SOS,
    item_Radio,
    0
};

SetMelody::SetMelody(Form &form, const String &label)
    : ListField(form, label, melodyNames)
    , needsPlay(false)
{
}

int SetMelody::dispatch(int event)
{
    int result = ListField::dispatch(event);
    if (result == FORM_CHANGED) {
        updateMelody();
        needsPlay = true;   // Play when we see the button release event.
    } else if (needsPlay && event < 0) {
        needsPlay = false;
        alarmMelody.playOnce();
    }
    return result;
}

void SetMelody::updateMelody()
{
    switch (value()) {
    case 0: default:
        alarmMelody.setMelody(defaultMelodyNotes, defaultMelodyLengths, sizeof(defaultMelodyLengths));
        alarmMelody.setRadioMode(false);
        break;
    case 1:
        alarmMelody.setMelody(haircutNotes, haircutLengths, sizeof(haircutLengths));
        alarmMelody.setRadioMode(false);
        break;
    case 2:
        alarmMelody.setMelody(sosNotes, sosLengths, sizeof(sosLengths));
        alarmMelody.setRadioMode(false);
        break;
    case 3:
        alarmMelody.setMelody(defaultMelodyNotes, defaultMelodyLengths, sizeof(defaultMelodyLengths));
        alarmMelody.setRadioMode(true);
        break;
    }
}
