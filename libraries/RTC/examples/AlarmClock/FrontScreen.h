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

    enum AlarmMode
    {
        AlarmOff,
        AlarmOn,
        Snooze
    };

    AlarmMode isAlarmMode() const { return _alarmMode; }
    void setAlarmMode(AlarmMode mode);

    bool is24HourMode() const { return _hourMode; }
    void set24HourMode(bool value);

    bool isRadioOn() const { return _radioOn; }
    void setRadioOn(bool value);

private:
    RTCDate _date;
    RTCTime _time;
    AlarmMode _alarmMode;
    bool _hourMode;
    bool _radioOn;

    void updateDate();
    void updateTime();
    void updateIndicators();

    void registerIndicators();
};

#endif
