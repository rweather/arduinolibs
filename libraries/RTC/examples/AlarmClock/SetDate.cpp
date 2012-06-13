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

#include "SetDate.h"
#include "Clock.h"

#define EDIT_DAY    0
#define EDIT_MONTH  1
#define EDIT_YEAR   2

SetDate::SetDate(Form &form, const String &label)
    : Field(form, label)
    , editField(EDIT_DAY)
{
    _value.day = 1;
    _value.month = 1;
    _value.year = 2012;
}

int SetDate::dispatch(int event)
{
    RTCDate newValue;
    if (event == LCD_BUTTON_UP) {
        newValue = _value;
        if (editField == EDIT_DAY)
            RTC::adjustDays(&newValue, RTC::INCREMENT | RTC::WRAP);
        else if (editField == EDIT_MONTH)
            RTC::adjustMonths(&newValue, RTC::INCREMENT | RTC::WRAP);
        else if (editField == EDIT_YEAR)
            RTC::adjustYears(&newValue, RTC::INCREMENT | RTC::WRAP);
        _value = newValue;
        printDate();
        rtc.writeDate(&_value);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        newValue = _value;
        if (editField == EDIT_DAY)
            RTC::adjustDays(&newValue, RTC::DECREMENT | RTC::WRAP);
        else if (editField == EDIT_MONTH)
            RTC::adjustMonths(&newValue, RTC::DECREMENT | RTC::WRAP);
        else if (editField == EDIT_YEAR)
            RTC::adjustYears(&newValue, RTC::DECREMENT | RTC::WRAP);
        _value = newValue;
        printDate();
        rtc.writeDate(&_value);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_LEFT) {
        if (editField != EDIT_DAY) {
            --editField;
            printDate();
            return 0;
        }
    } else if (event == LCD_BUTTON_RIGHT) {
        if (editField != EDIT_YEAR) {
            ++editField;
            printDate();
            return 0;
        }
    }
    return -1;
}

void SetDate::enterField(bool reverse)
{
    rtc.readDate(&_value);
    Field::enterField(reverse);
    if (reverse)
        editField = EDIT_YEAR;
    else
        editField = EDIT_DAY;
    printDate();
    lcd()->cursor();
}

void SetDate::exitField()
{
    lcd()->noCursor();
    Field::exitField();
}

void SetDate::setValue(const RTCDate &value)
{
    _value = value;
    if (isCurrent())
        printDate();
}

void SetDate::updateCurrentDate()
{
    if (isCurrent()) {
        rtc.readDate(&_value);
        printDate();
    }
}

extern const char *days[];      // Table of day names; e.g. "Mon, ".
extern const char *months[];    // Table of month names; e.g. " Jan ".

void SetDate::printDate()
{
    lcd()->setCursor(0, 1);
    int dayCol;
    int monthCol;
    int yearCol;
    int col = 0;
    lcd()->write(days[RTC::dayOfWeek(&_value) - 1]);
    col += 5;
    dayCol = col;
    if (_value.day < 10) {
        lcd()->write('0' + _value.day);
        ++col;
    } else {
        lcd()->write('0' + _value.day / 10);
        lcd()->write('0' + _value.day % 10);
        col += 2;
    }
    monthCol = col + 1;
    lcd()->print(months[_value.month - 1]);
    col += 5;
    yearCol = col;
    lcd()->write('0' + _value.year / 1000);
    lcd()->write('0' + (_value.year / 100) % 10);
    lcd()->write('0' + (_value.year / 10) % 10);
    lcd()->write('0' + _value.year % 10);
    lcd()->write(' ');
    if (editField == EDIT_DAY)
        lcd()->setCursor(dayCol, 1);
    else if (editField == EDIT_MONTH)
        lcd()->setCursor(monthCol, 1);
    else
        lcd()->setCursor(yearCol, 1);
}
