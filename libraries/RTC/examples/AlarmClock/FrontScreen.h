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

#ifndef FrontScreen_h
#define FrontScreen_h

#include <Field.h>
#include <RTC.h>

class FrontScreenField : public Field
{
public:
    explicit FrontScreenField(Form &form);
    ~FrontScreenField();

    void enterField(bool reverse);

    RTCDate date() const { return _date; }
    void setDate(const RTCDate &date);

    RTCTime time() const { return _time; }
    void setTime(const RTCTime &time);

    int voltage() const { return _voltage; }
    void setVoltage(int voltage);

    bool isAlarmActive() const { return _alarmActive; }
    void setAlarmActive(bool active);

    bool is24HourMode() const { return _hourMode; }
    void set24HourMode(bool value);

private:
    RTCDate _date;
    RTCTime _time;
    int _voltage;
    int _voltageTrunc;
    int _batteryBars;
    bool _alarmActive;
    bool _hourMode;

    void updateDate();
    void updateTime();
    void updateVoltage();
    void updateAlarm();

    void registerIndicators();
};

#endif
