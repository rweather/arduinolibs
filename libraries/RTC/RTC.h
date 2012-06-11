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

#ifndef RTC_h
#define RTC_h

#include <inttypes.h>

struct RTCTime
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct RTCDate
{
    unsigned int year;
    uint8_t month;
    uint8_t day;
};

struct RTCAlarm
{
    uint8_t hour;
    uint8_t minute;
    uint8_t flags;
};

class RTC
{
public:
    RTC();
    ~RTC();

    enum DayOfWeek
    {
        Monday = 1,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday,
    };

    virtual bool hasUpdates();

    virtual void readTime(RTCTime *value);
    virtual void readDate(RTCDate *value);

    virtual void writeTime(const RTCTime *value);
    virtual void writeDate(const RTCDate *value);

    static const uint8_t ALARM_COUNT = 4;

    virtual void readAlarm(uint8_t alarmNum, RTCAlarm *value);
    virtual void writeAlarm(uint8_t alarmNum, const RTCAlarm *value);

    virtual int byteCount() const;
    virtual uint8_t readByte(uint8_t offset);
    virtual void writeByte(uint8_t offset, uint8_t value);

    static const int NO_TEMPERATURE = 32767;

    virtual int readTemperature();

    // Flags for adjustDays(), adjustMonths(), and adjustYears().
    static const uint8_t INCREMENT = 0x0000;
    static const uint8_t DECREMENT = 0x0001;
    static const uint8_t WRAP      = 0x0002;

    static void adjustDays(RTCDate *date, uint8_t flags);
    static void adjustMonths(RTCDate *date, uint8_t flags);
    static void adjustYears(RTCDate *date, uint8_t flags);

    static DayOfWeek dayOfWeek(const RTCDate *date);

private:
    unsigned long midnight;
    RTCDate date;
    RTCAlarm alarms[ALARM_COUNT];
    uint8_t *nvram;
};

#endif
