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

#ifndef DS3232RTC_h
#define DS3232RTC_h

#include "RTC.h"

class I2CMaster;

class DS3232RTC : public RTC {
public:
    DS3232RTC(I2CMaster &bus, uint8_t oneHzPin = 255);

    bool isRealTime() const { return _isRealTime; }

    bool hasUpdates();

    void readTime(RTCTime *value);
    void readDate(RTCDate *value);

    void writeTime(const RTCTime *value);
    void writeDate(const RTCDate *value);

    void readAlarm(uint8_t alarmNum, RTCAlarm *value);
    void writeAlarm(uint8_t alarmNum, const RTCAlarm *value);

    int byteCount() const;
    uint8_t readByte(uint8_t offset);
    void writeByte(uint8_t offset, uint8_t value);

    int readTemperature();

    void enableAlarmInterrupts();
    void disableAlarmInterrupts();
    int firedAlarm();

    void enable32kHzOutput();
    void disable32kHzOutput();

private:
    I2CMaster *_bus;
    uint8_t _oneHzPin;
    bool prevOneHz;
    bool _isRealTime;
    bool alarmInterrupts;

    void initAlarms();

    uint8_t readRegister(uint8_t reg);
    bool writeRegister(uint8_t reg, uint8_t value);

    void updateAlarmInterrupts();
};

#endif
