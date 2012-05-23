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
#include <WProgram.h>

// Special characters for indicators.
#define IND_BATTERY_EMPTY       0
#define IND_BATTERY_20PCT       1
#define IND_BATTERY_40PCT       2
#define IND_BATTERY_60PCT       3
#define IND_BATTERY_80PCT       4
#define IND_BATTERY_FULL        5
#define IND_ALARM_ACTIVE1       6
#define IND_ALARM_ACTIVE2       7

FrontScreenField::FrontScreenField(Form &form)
    : Field(form, "")
#ifdef USE_VOLTAGE_MONITOR
    , _voltage(360)
    , _voltageTrunc(36)
    , _batteryBars(IND_BATTERY_FULL)
#endif
    , _alarmActive(false)
    , _hourMode(false)
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
#ifdef USE_VOLTAGE_MONITOR
    updateVoltage();
#endif
    updateTime();
    updateAlarm();
}

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

#ifdef USE_VOLTAGE_MONITOR

void FrontScreenField::setVoltage(int voltage)
{
    // Normal voltage ranges between 2.7 and 3.6.  The power supply
    // for the clock will no longer function below 2.7 volts.
    if (_voltage == voltage)
        return;
    _voltage = voltage;
    int ind;
    if (voltage > 355)
        ind = IND_BATTERY_FULL;
    else if (voltage > 345)
        ind = IND_BATTERY_80PCT;
    else if (voltage > 325)
        ind = IND_BATTERY_60PCT;
    else if (voltage > 305)
        ind = IND_BATTERY_40PCT;
    else if (voltage > 285)
        ind = IND_BATTERY_20PCT;
    else
        ind = IND_BATTERY_EMPTY;
    int trunc = voltage / 10;
    if (ind != _batteryBars || trunc != _voltageTrunc) {
        _batteryBars = ind;
        _voltageTrunc = trunc;
        if (isCurrent())
            updateVoltage();
    }
}

#endif

void FrontScreenField::setAlarmActive(bool active)
{
    if (_alarmActive != active) {
        _alarmActive = active;
        if (isCurrent())
            updateAlarm();
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

void FrontScreenField::updateDate()
{
    lcd()->setCursor(0, 0);
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

#ifdef USE_VOLTAGE_MONITOR

void FrontScreenField::updateVoltage()
{
    lcd()->setCursor(15, 0);
    lcd()->write(_batteryBars);

    lcd()->setCursor(12, 1);
    lcd()->write('0' + _voltageTrunc / 10);
    lcd()->write('.');
    lcd()->write('0' + _voltageTrunc % 10);
    lcd()->write('v');
}

#endif

void FrontScreenField::updateAlarm()
{
#ifdef USE_VOLTAGE_MONITOR
    lcd()->setCursor(13, 0);
#else
    lcd()->setCursor(14, 0);
#endif
    lcd()->write(_alarmActive ? IND_ALARM_ACTIVE1 : ' ');
    lcd()->write(_alarmActive ? IND_ALARM_ACTIVE2 : ' ');
}

#ifdef USE_VOLTAGE_MONITOR
static uint8_t batteryEmpty[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111,
    B00000
};
static uint8_t battery20Pct[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111,
    B00000
};
static uint8_t battery40Pct[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B00000
};
static uint8_t battery60Pct[8] = {
    B01110,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};
static uint8_t battery80Pct[8] = {
    B01110,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};
static uint8_t batteryFull[8] = {
    B01110,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};
#endif
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

void FrontScreenField::registerIndicators()
{
#ifdef USE_VOLTAGE_MONITOR
    lcd()->createChar(IND_BATTERY_EMPTY, batteryEmpty);
    lcd()->createChar(IND_BATTERY_20PCT, battery20Pct);
    lcd()->createChar(IND_BATTERY_40PCT, battery40Pct);
    lcd()->createChar(IND_BATTERY_60PCT, battery60Pct);
    lcd()->createChar(IND_BATTERY_80PCT, battery80Pct);
    lcd()->createChar(IND_BATTERY_FULL,  batteryFull);
#endif
    lcd()->createChar(IND_ALARM_ACTIVE1, alarmActive1);
    lcd()->createChar(IND_ALARM_ACTIVE2, alarmActive2);
}
