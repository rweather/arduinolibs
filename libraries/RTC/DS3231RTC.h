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

/*
 * Adapted from DS3232RTC library for DS3231 RTC chip by Thijs Oppermann
 * 2014-12-07
 */

#ifndef DS3231RTC_h
#define DS3231RTC_h

#include "RTC.h"

class I2CMaster;

class DS3231RTC : public RTC {
 public:
  DS3231RTC(I2CMaster &bus, uint8_t oneHzPin = 255);

  bool isRealTime() const {
    return _isRealTime;
  }

  bool hasUpdates();

  void readTime(RTCTime* value);
  void readDate(RTCDate* value);

  void writeTime(const RTCTime* value);
  void writeDate(const RTCDate* value);

  void readAlarm(uint8_t alarmNum, RTCAlarm* value);
  void writeAlarm(uint8_t alarmNum, const RTCAlarm* value);
  bool setAlarm(uint8_t alarmNum, const RTCAlarm* value);

  int  readTemperature();

  void enableAlarmInterrupts();
  void disableAlarmInterrupts();
  int  firedAlarm();

  void enable32kHzOutput();
  void disable32kHzOutput();

  void enableAlarm(uint8_t alarmNum);
  void disableAlarm(uint8_t alarmNum);

 private:
  I2CMaster* _bus;
  uint8_t    _oneHzPin;
  bool       prevOneHz;
  bool       _isRealTime;
  bool       alarmInterrupts;

  void    alarmSecondValues(uint8_t read_value, RTCAlarm* value);
  void    alarmMinuteValues(uint8_t read_value, RTCAlarm* value);
  void    alarmHourValues(uint8_t read_value, RTCAlarm* value);
  void    alarmDayValues(uint8_t read_value, RTCAlarm* value);

  uint8_t getAlarmDayValue(const RTCAlarm* value);

  void    clearAlarm(uint8_t alarmNum);

  uint8_t readRegister(uint8_t reg);
  bool    writeRegister(uint8_t reg, uint8_t value);
};

#endif
