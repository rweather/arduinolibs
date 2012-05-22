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

#include "EditTime.h"
#include <RTC.h>

#define EDIT_ENABLED        0
#define EDIT_HOUR           1
#define EDIT_MINUTE_TENS    2
#define EDIT_MINUTE         3

extern bool is24HourClock;

EditTime::EditTime(Form &form, const String &label)
    : Field(form, label)
    , isAlarm(false)
    , alarmEnabled(false)
    , editField(EDIT_HOUR)
{
    _value.hour = 0;
    _value.minute = 0;
    _value.second = 0;
}

int EditTime::dispatch(int event)
{
    RTCTime newValue;
    if (event == LCD_BUTTON_UP) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue.hour = (newValue.hour + 1) % 24;
        } else if (editField == EDIT_MINUTE_TENS) {
            newValue.minute = (newValue.minute + 10) % 60;
        } else if (editField == EDIT_MINUTE) {
            newValue.minute = (newValue.minute + 1) % 60;
        } else if (editField == EDIT_ENABLED) {
            alarmEnabled = !alarmEnabled;
            if (alarmEnabled)
                editField = EDIT_HOUR;
            printTime();
            return FORM_CHANGED;
        }
        newValue.second = 0;
        _value = newValue;
        printTime();
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue.hour = (newValue.hour + 23) % 24;
        } else if (editField == EDIT_MINUTE_TENS) {
            newValue.minute = (newValue.minute + 50) % 60;
        } else if (editField == EDIT_MINUTE) {
            newValue.minute = (newValue.minute + 59) % 60;
        } else if (editField == EDIT_ENABLED) {
            alarmEnabled = !alarmEnabled;
            if (alarmEnabled)
                editField = EDIT_HOUR;
            printTime();
            return FORM_CHANGED;
        }
        newValue.second = 0;
        _value = newValue;
        printTime();
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_LEFT) {
        if (isAlarm) {
            if (editField != EDIT_ENABLED) {
                --editField;
                printTime();
                return 0;
            }
        } else {
            if (editField != EDIT_HOUR) {
                --editField;
                printTime();
                return 0;
            }
        }
    } else if (event == LCD_BUTTON_RIGHT) {
        if (isAlarm) {
            if (alarmEnabled) {
                if (editField != EDIT_MINUTE) {
                    ++editField;
                    printTime();
                    return 0;
                }
            }
        } else {
            if (editField != EDIT_MINUTE) {
                ++editField;
                printTime();
                return 0;
            }
        }
    }
    return -1;
}

void EditTime::enterField(bool reverse)
{
    Field::enterField(reverse);
    if (isAlarm) {
        if (alarmEnabled) {
            if (reverse)
                editField = EDIT_MINUTE;
            else
                editField = EDIT_ENABLED;
        } else {
            editField = EDIT_ENABLED;
        }
    } else {
        if (reverse)
            editField = EDIT_MINUTE;
        else
            editField = EDIT_HOUR;
    }
    printTime();
    lcd()->cursor();
}

void EditTime::exitField()
{
    lcd()->noCursor();
    Field::exitField();
}

void EditTime::setValue(const RTCTime &value)
{
    _value = value;
    isAlarm = false;
    if (isCurrent())
        printTime();
}

RTCAlarm EditTime::alarmValue() const
{
    RTCAlarm alarm;
    alarm.hour = _value.hour;
    alarm.minute = _value.minute;
    alarm.flags = alarmEnabled ? 1 : 0;
    return alarm;
}

void EditTime::setAlarmValue(const RTCAlarm &value)
{
    _value.hour = value.hour;
    _value.minute = value.minute;
    _value.second = 0;
    isAlarm = true;
    alarmEnabled = ((value.flags & 0x01) != 0);
    if (isCurrent())
        printTime();
}

void EditTime::printTime()
{
    lcd()->setCursor(0, 1);
    int hour = _value.hour;
    int minute = _value.minute;
    int timeCol = 0;
    bool pm;
    if (isAlarm) {
        if (alarmEnabled) {
            lcd()->print("On ");
            timeCol = 3;
        } else {
            lcd()->print("Off       ");
            lcd()->setCursor(0, 1);
            return;
        }
    }
    if (is24HourClock) {
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = false;
    } else if (hour == 0 || hour == 12) {
        lcd()->write('1');
        lcd()->write('2');
        pm = (hour == 12);
    } else if (hour < 12) {
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = false;
    } else {
        hour -= 12;
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = true;
    }
    lcd()->write(':');
    lcd()->write('0' + minute / 10);
    lcd()->write('0' + minute % 10);
    if (!is24HourClock)
        lcd()->print(pm ? "pm" : "am");
    if (editField == EDIT_ENABLED)
        lcd()->setCursor(0, 1);
    else if (editField == EDIT_HOUR)
        lcd()->setCursor(timeCol + 1, 1);
    else if (editField == EDIT_MINUTE_TENS)
        lcd()->setCursor(timeCol + 3, 1);
    else if (editField == EDIT_MINUTE)
        lcd()->setCursor(timeCol + 4, 1);
}
