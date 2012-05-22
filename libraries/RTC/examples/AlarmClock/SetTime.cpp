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

#include "SetTime.h"
#include <DS1307RTC.h>

extern DS1307RTC rtc;

SetTime::SetTime(Form &form, const String &label)
    : EditTime(form, label)
{
}

int SetTime::dispatch(int event)
{
    int result = EditTime::dispatch(event);
    if (result == FORM_CHANGED) {
        // Update the realtime clock with the new value.
        RTCTime time = value();
        rtc.writeTime(&time);
    }
    return result;
}

void SetTime::enterField(bool reverse)
{
    // Read the current time when the field is entered.
    rtc.readTime(&_value);
    EditTime::enterField(reverse);
}

void SetTime::updateCurrentTime()
{
    if (isCurrent()) {
        RTCTime time;
        rtc.readTime(&time);
        if (time.hour != _value.hour || time.minute != _value.minute) {
            _value = time;
            printTime();
        }
    }
}
