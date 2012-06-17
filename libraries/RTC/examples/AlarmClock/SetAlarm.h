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

#ifndef SetAlarm_h
#define SetAlarm_h

#include "Field.h"
#include <RTC.h>

class SetAlarm : public Field {
public:
    SetAlarm(Form &form, const String &label, uint8_t alarmNum);

    int dispatch(int event);

    void enterField(bool reverse);
    void exitField();

    RTCAlarm value() const { return _value; }
    void setValue(const RTCAlarm &value);

    enum Days
    {
        AnyDay,
        MondayToFriday,
        SaturdayAndSunday
    };

    Days days() const;
    void setDays(Days days);

    static Days days(const RTCAlarm *alarm);

    static void combineDays(RTCAlarm *alarm1, const RTCAlarm *alarm2);

private:
    uint8_t _alarmNum;
    RTCAlarm _value;
    uint8_t editField;

    bool isEnabled() const { return (_value.flags & 0x01) != 0; }

    void printAlarm();
};

#endif
