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

#ifndef DS1307RTC_h
#define DS1307RTC_h

#include "RTC.h"
#include "BitBangI2C.h"

class DS1307RTC : public RTC {
public:
    DS1307RTC(BitBangI2C &bus, uint8_t oneHzPin = 255);
    ~DS1307RTC();

    bool isRealTime() const { return _isRealTime; }

    bool hasUpdates();

    void readTime(RTCTime *value);
    void readDate(RTCDate *value);

    void writeTime(const RTCTime *value);
    void writeDate(const RTCDate *value);

    void readAlarm(uint8_t alarmNum, RTCAlarm *value);
    void writeAlarm(uint8_t alarmNum, const RTCAlarm *value);

    uint8_t readByte(uint8_t offset);
    void writeByte(uint8_t offset, uint8_t value);

private:
    BitBangI2C *_bus;
    uint8_t _oneHzPin;
    bool prevOneHz;
    bool _isRealTime;

    void initAlarms();
};

#endif
