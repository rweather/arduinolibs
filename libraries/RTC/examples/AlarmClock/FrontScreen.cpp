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

#include "FrontScreen.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// Special characters for indicators.
#define IND_RADIO_ON            0
#define IND_ALARM_SNOOZE        5
#define IND_ALARM_ACTIVE1       6
#define IND_ALARM_ACTIVE2       7

FrontScreenField::FrontScreenField(Form &form)
    : Field(form, "")
    , _alarmMode(FrontScreenField::AlarmOff)
    , _hourMode(false)
    , _radioOn(false)
{
    _date.day = 1;
    _date.month = 1;
    _date.year = 2012;
    _time.hour = 9;
    _time.minute = 0;
    _time.second = 0;
    registerIndicators();
}

FrontScreenField::~FrontScreenField()
{
}

void FrontScreenField::enterField(bool reverse)
{
    updateDate();
    updateTime();
    updateIndicators();
}

const char *days[] = {
    "Mon, ", "Tue, ", "Wed, ", "Thu, ", "Fri, ", "Sat, ", "Sun, "
};

const char *months[] = {
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Oct ", " Nov ", " Dec "
};

void FrontScreenField::setDate(const RTCDate &date)
{
    if (date.day != _date.day || date.month != _date.month ||
            date.year != _date.year) {
        _date = date;
        if (isCurrent())
            updateDate();
    }
}

void FrontScreenField::setTime(const RTCTime &time)
{
    if (time.hour != _time.hour || time.minute != _time.minute ||
            time.second != _time.second) {
        _time = time;
        if (isCurrent())
            updateTime();
    }
}

static uint8_t alarmActive1[8] = {
    B00100,
    B01001,
    B10010,
    B00000,
    B10010,
    B01001,
    B00100,
    B00000
};
static uint8_t alarmActive2[8] = {
    B11000,
    B10100,
    B10011,
    B10011,
    B10011,
    B10100,
    B11000,
    B00000
};
static uint8_t alarmSnooze[8] = {
    B11110,
    B00100,
    B01000,
    B11110,
    B00000,
    B00000,
    B00000,
    B00000
};
static uint8_t radioIndicator[8] = {
    B11111,
    B10101,
    B01110,
    B00100,
    B00100,
    B00100,
    B00100,
    B00000
};

void FrontScreenField::setAlarmMode(AlarmMode mode)
{
    if (_alarmMode != mode) {
        _alarmMode = mode;
        if (isCurrent())
            updateIndicators();
    }
}

void FrontScreenField::set24HourMode(bool value)
{
    if (_hourMode != value) {
        _hourMode = value;
        if (isCurrent())
            updateTime();
    }
}

void FrontScreenField::setRadioOn(bool value)
{
    if (_radioOn != value) {
        _radioOn = value;
        if (isCurrent())
            updateIndicators();
    }
}

void FrontScreenField::updateDate()
{
    lcd()->setCursor(0, 0);
    lcd()->write(days[RTC::dayOfWeek(&_date) - 1]);
    if (_date.day < 10) {
        lcd()->write('0' + _date.day);
    } else {
        lcd()->write('0' + _date.day / 10);
        lcd()->write('0' + _date.day % 10);
    }
    lcd()->print(months[_date.month - 1]);
    lcd()->print(_date.year);
    lcd()->write(' ');
}

void FrontScreenField::updateTime()
{
    lcd()->setCursor(0, 1);
    bool pm;
    if (_hourMode) {
        lcd()->write('0' + _time.hour / 10);
        lcd()->write('0' + _time.hour % 10);
        pm = false;
    } else if (_time.hour == 0 || _time.hour == 12) {
        lcd()->write('1');
        lcd()->write('2');
        pm = (_time.hour == 12);
    } else if (_time.hour < 12) {
        lcd()->write('0' + _time.hour / 10);
        lcd()->write('0' + _time.hour % 10);
        pm = false;
    } else {
        int hour = _time.hour - 12;
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = true;
    }
    lcd()->write(':');
    lcd()->write('0' + _time.minute / 10);
    lcd()->write('0' + _time.minute % 10);
    lcd()->write(':');
    lcd()->write('0' + _time.second / 10);
    lcd()->write('0' + _time.second % 10);
    if (!_hourMode)
        lcd()->print(pm ? "pm" : "am");
}

void FrontScreenField::updateIndicators()
{
    lcd()->setCursor(13, 1);
    lcd()->print("   ");
    int col = 16;
    if (_radioOn) {
        --col;
        lcd()->setCursor(col, 1);
        lcd()->write((uint8_t)IND_RADIO_ON);
    }
    if (_alarmMode != AlarmOff) {
        col -= 2;
        lcd()->setCursor(col, 1);
        lcd()->write(_alarmMode == Snooze ? IND_ALARM_SNOOZE : IND_ALARM_ACTIVE1);
        lcd()->write(IND_ALARM_ACTIVE2);
    }
}

void FrontScreenField::registerIndicators()
{
    lcd()->createChar(IND_RADIO_ON, radioIndicator);
    lcd()->createChar(IND_ALARM_SNOOZE, alarmSnooze);
    lcd()->createChar(IND_ALARM_ACTIVE1, alarmActive1);
    lcd()->createChar(IND_ALARM_ACTIVE2, alarmActive2);
}
