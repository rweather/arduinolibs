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

#include "DS3231RTC.h"
#include "../I2C/I2CMaster.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class DS3231RTC DS3231RTC.h <DS3231RTC.h>
 * \brief Communicates with a DS3231 realtime clock chip via I2C.
 *
 * This class simplifies the process of reading and writing the time
 * and date information in a DS3231 realtime clock chip.  The class
 * also provides support for reading and writing information about
 * alarms and other clock settings.
 *
 * If there is no DS3231 chip on the I2C bus, this class will fall
 * back to the RTC class to simulate the current time and date based
 * on the value of millis().
 *
 * Alarms 0 and 1 can be set to generate an interrupt when they fire
 * using enableAlarmInterrupts(). The firedAlarm() function can be
 * used to determine which alarm has fired.
 *
 * The DS3231 uses a 2-digit year so this class is limited to dates
 * between 2000 and 2099 inclusive.
 *
 * The structure RTCAlarm is used to read and write the alarms. It has
 * a flags field which is analogous to the alarm mask bits found in
 * the DS3231 manual. Bit 7 signals whether this is an alarm structure
 * with settings for alarm 0 or 1 (as these are different, see
 * manual).
 *
 * \sa RTC, DS3232RTC, DS1307RTC
 */

// I2C address of the RTC chip (7-bit).
#define DS3231_I2C_ADDRESS  0x68

// Registers.
#define DS3231_SECOND       0x00
#define DS3231_MINUTE       0x01
#define DS3231_HOUR         0x02
#define DS3231_DAY_OF_WEEK  0x03
#define DS3231_DATE         0x04
#define DS3231_MONTH        0x05
#define DS3231_YEAR         0x06
#define DS3231_ALARM1_SEC   0x07
#define DS3231_ALARM1_MIN   0x08
#define DS3231_ALARM1_HOUR  0x09
#define DS3231_ALARM1_DAY   0x0A
#define DS3231_ALARM2_MIN   0x0B
#define DS3231_ALARM2_HOUR  0x0C
#define DS3231_ALARM2_DAY   0x0D
#define DS3231_CONTROL      0x0E
#define DS3231_STATUS       0x0F
#define DS3231_AGING_OFFSET 0x10
#define DS3231_TEMP_MSB     0x11
#define DS3231_TEMP_LSB     0x12

// Bits in the DS3231_CONTROL register.
#define DS3231_EOSC         0x80
#define DS3231_BBSQW        0x40
#define DS3231_CONV         0x20
#define DS3231_RS_1HZ       0x00
#define DS3231_RS_1024HZ    0x08
#define DS3231_RS_4096HZ    0x10
#define DS3231_RS_8192HZ    0x18
#define DS3231_INTCN        0x04
#define DS3231_A2IE         0x02
#define DS3231_A1IE         0x01

// Bits in the DS3231_STATUS register.
#define DS3231_OSF          0x80
#define DS3231_BB32KHZ      0x40
#define DS3231_EN32KHZ      0x08
#define DS3231_BSY          0x04
#define DS3231_A2F          0x02
#define DS3231_A1F          0x01

// Alarms 0 and 1
#define DS3231_ALARM_0      0x07
#define DS3231_ALARM_1      0x0B

/**
 * \brief Attaches to a realtime clock slave device on \a bus.
 *
 * If \a oneHzPin is not 255, then it indicates a digital input pin
 * that is connected to the 1 Hz square wave output on the realtime clock.
 * This input is used by hasUpdates() to determine if the time information
 * has changed in a non-trivial manner.
 *
 * If you wish to use enableAlarmInterrupts(), then \a oneHzPin must be 255.
 *
 * \sa hasUpdates(), enableAlarmInterrupts()
 */
DS3231RTC::DS3231RTC(I2CMaster &bus, uint8_t oneHzPin)
  : _bus(&bus)
  , _oneHzPin(oneHzPin)
  , prevOneHz(false)
  , _isRealTime(true)
  , alarmInterrupts(false) {
  // Probe the device and configure it for our use.
  _bus->startWrite(DS3231_I2C_ADDRESS);
  _bus->write(DS3231_CONTROL);
  if ( _bus->startRead(DS3231_I2C_ADDRESS, 1) ) {
    uint8_t value = _bus->read() & DS3231_CONV;
    if ( oneHzPin != 255 ) {
      value |= DS3231_BBSQW | DS3231_RS_1HZ;
    }
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_CONTROL);
    _bus->write(value);
    _bus->endWrite();
  }
  else {
    // Did not get an acknowledgement from the RTC chip.
    _isRealTime = false;
  }

  // Configure the 1 Hz square wave pin if required.
  if ( ( oneHzPin != 255) && _isRealTime ) {
    pinMode(oneHzPin, INPUT);
    digitalWrite(oneHzPin, HIGH);
  }
}

/**
 * \fn bool DS3231RTC::hasUpdates()
 * \brief Returns true if there are updates
 */

bool DS3231RTC::hasUpdates() {
  // If not using a 1 Hz pin or there is no RTC chip available,
  // then assume that there is an update available.
  if ( ( _oneHzPin == 255) || !_isRealTime ) {
    return true;
  }

  // The DS3231 updates the internal registers on the falling edge of the
  // 1 Hz clock.  The values should be ready to read on the rising edge.
  bool value = digitalRead(_oneHzPin);
  if ( value && !prevOneHz ) {
    prevOneHz = value;
    return true;
  }
  else {
    prevOneHz = value;
    return false;
  }
}

inline uint8_t fromBCD(uint8_t value) {
  return (value >> 4) * 10 + (value & 0x0F);
}

inline uint8_t fromHourBCD(uint8_t value) {
  if ( (value & 0x40) != 0 ) {
    // 12-hour mode.
    uint8_t result = ( (value >> 4) & 0x01 ) * 10 + (value & 0x0F);
    if ( (value & 0x20) != 0 ) {
      return (result == 12) ? 12 : (result + 12);           // PM
    }
    else {
      return (result == 12) ? 0 : result;                   // AM
    }
  }
  else {
    // 24-hour mode.
    return fromBCD(value);
  }
}

void DS3231RTC::readTime(RTCTime* value) {
  if ( _isRealTime ) {
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_SECOND);
    if ( _bus->startRead(DS3231_I2C_ADDRESS, 3) ) {
      value->second = fromBCD( _bus->read() );
      value->minute = fromBCD( _bus->read() );
      value->hour = fromHourBCD( _bus->read() );
    }
    else {
      // RTC chip is not responding.
      value->second = 0;
      value->minute = 0;
      value->hour = 0;
    }
  }
  else {
    RTC::readTime(value);
  }
}

void DS3231RTC::readDate(RTCDate* value) {
  if ( !_isRealTime ) {
    RTC::readDate(value);
    return;
  }
  _bus->startWrite(DS3231_I2C_ADDRESS);
  _bus->write(DS3231_DATE);
  if ( _bus->startRead(DS3231_I2C_ADDRESS, 3) ) {
    value->day = fromBCD( _bus->read() );
    value->month = fromBCD(_bus->read() & 0x7F);     // Strip century bit.
    value->year = fromBCD( _bus->read() ) + 2000;
  }
  else {
    // RTC chip is not responding.
    value->day = 1;
    value->month = 1;
    value->year = 2000;
  }
}

inline uint8_t toBCD(uint8_t value) {
  return ( (value / 10) << 4 ) + (value % 10);
}

void DS3231RTC::writeTime(const RTCTime* value) {
  if ( _isRealTime ) {
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_SECOND);
    _bus->write( toBCD(value->second) );
    _bus->write( toBCD(value->minute) );
    _bus->write( toBCD(value->hour) );      // Changes mode to 24-hour clock.
    _bus->endWrite();
  }
  else {
    RTC::writeTime(value);
  }
}

void DS3231RTC::writeDate(const RTCDate* value) {
  if ( _isRealTime ) {
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_DATE);
    _bus->write( toBCD(value->day) );
    _bus->write( toBCD(value->month) );
    _bus->write( toBCD(value->year % 100) );
    _bus->endWrite();
  }
  else {
    RTC::writeDate(value);
  }
}

void DS3231RTC::readAlarm(uint8_t alarmNum, RTCAlarm* value) {
  if ( _isRealTime ) {
    uint8_t reg_value = readRegister(DS3231_CONTROL);
    _bus->startWrite(DS3231_I2C_ADDRESS);
    if ( 0 == alarmNum ) {
      _bus->write(DS3231_ALARM_0);
      if ( _bus->startRead(DS3231_I2C_ADDRESS, 4) ) {
        alarmSecondValues(_bus->read(), value);
        alarmMinuteValues(_bus->read(), value);
        alarmHourValues(_bus->read(), value);
        alarmDayValues(_bus->read(), value);
        value->flags &= ~0x80;
        value->flags |= (reg_value & 0x01) << 6;
      }
      else {
        // RTC chip is not responding.
        value->day = 0;
        value->hour = 0;
        value->minute = 0;
        value->second = 0;
      }
    }
    else if ( 1 == alarmNum ) {
      _bus->write(DS3231_ALARM_1);
      if ( _bus->startRead(DS3231_I2C_ADDRESS, 3) ) {
        value->second = 0;
        alarmMinuteValues(_bus->read(), value);
        alarmHourValues(_bus->read(), value);
        alarmDayValues(_bus->read(), value);
        value->flags |= 0x80;
        value->flags |= (reg_value & 0x02) << 5;
      }
      else {
        // RTC chip is not responding.
        value->day = 0;
        value->hour = 0;
        value->minute = 0;
        value->second = 0;
      }
    }
  }
  else {
    RTC::readAlarm(alarmNum, value);
  }
}

void DS3231RTC::alarmSecondValues(uint8_t read_value, RTCAlarm* value) {
  uint8_t mask_bit = (read_value & 0x80) ? 0x01 : 0x00;

  value->second = fromBCD(read_value & 0x7F);
  value->flags |= mask_bit;
}

void DS3231RTC::alarmMinuteValues(uint8_t read_value, RTCAlarm* value) {
  uint8_t mask_bit = (read_value & 0x80) ? 0x02 : 0x00;

  value->minute = fromBCD(read_value & 0x7F);
  value->flags |= mask_bit;
}

void DS3231RTC::alarmHourValues(uint8_t read_value, RTCAlarm* value) {
  uint8_t mask_bit = (read_value & 0x80) ? 0x04 : 0x00;
  uint8_t is_ampm = read_value & 0x40;

  if ( is_ampm ) {
    uint8_t hour = fromBCD(read_value & ~0xE0);
    if ( read_value & 0x20 ) {
      hour += 12;
    }
    value->hour = hour;
  }
  else {
    value->hour = fromBCD(read_value & ~0xC0);
  }
  value->flags |= mask_bit;
}

void DS3231RTC::alarmDayValues(uint8_t read_value, RTCAlarm* value) {
  uint8_t mask_bit = (read_value & 0x80) ? 0x08 : 0x00;
  uint8_t is_dow = read_value & 0x40;

  if ( is_dow ) {
    value->day = 0;
    value->dow = fromBCD(read_value & 0x0F);
    value->flags |= 0x20;
  }
  else {
    value->dow = 0;
    value->day = fromBCD(read_value & 0x3F);
    value->flags &= ~0x20;
  }
  value->flags |= mask_bit;
}

void DS3231RTC::writeAlarm(uint8_t alarmNum, const RTCAlarm* value) {
  setAlarm(alarmNum, value);
  return;
}

void DS3231RTC::clearAlarm(uint8_t alarmNum) {
  if ( 0 == alarmNum ) {
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_ALARM_0);
    _bus->write(0);
    _bus->write(0);
    _bus->write(0);
    _bus->write(0x41);
    _bus->endWrite();
  }
  else if ( 1 == alarmNum ) {
    _bus->startWrite(DS3231_I2C_ADDRESS);
    _bus->write(DS3231_ALARM_1);
    _bus->write(0);
    _bus->write(0);
    _bus->write(0x41);
    _bus->endWrite();
  }
}

bool DS3231RTC::setAlarm(uint8_t alarmNum, const RTCAlarm* value) {
  if ( _isRealTime ) {
    uint8_t alarm_mask_bits = value->flags;

    if ( 0 == alarmNum ) {
      if ( alarm_mask_bits & 0x80 ) {
        return false;
      }

      _bus->startWrite(DS3231_I2C_ADDRESS);
      _bus->write(DS3231_ALARM_0);
      _bus->write( toBCD(value->second) | ( ( alarm_mask_bits & 0x01 ) << 7 ) );
      _bus->write( toBCD(value->minute) | ( ( alarm_mask_bits & 0x02 ) << 6 ) );
      // only support writing 24h
      _bus->write( toBCD(value->hour) | ( ( alarm_mask_bits & 0x04 ) << 5 ) );
      _bus->write( getAlarmDayValue(value) );
      _bus->endWrite();
      if ( value->flags & 0x40 ) {
        enableAlarm(alarmNum);
      }
    }
    else if ( 1 == alarmNum ) {
      if ( !(alarm_mask_bits & 0x80) ) {
        return false;
      }

      _bus->startWrite(DS3231_I2C_ADDRESS);
      _bus->write(DS3231_ALARM_1);
      _bus->write( toBCD(value->minute) | ( ( alarm_mask_bits & 0x02 ) << 6 ) );
      // only support writing 24h
      _bus->write( toBCD(value->hour) | ( ( alarm_mask_bits & 0x04 ) << 5 ) );
      _bus->write( getAlarmDayValue(value) );
      _bus->endWrite();
      if ( value->flags & 0x40 ) {
        enableAlarm(alarmNum);
      }
    }
  }
  else {
    RTC::writeAlarm(alarmNum, value);
    return false;
  }

  return true;
}

uint8_t DS3231RTC::getAlarmDayValue(const RTCAlarm* value) {
  uint8_t alarm_mask_bits = value->flags;
  uint8_t day_value_mask = ( alarm_mask_bits & 0x08 ) << 4;

  if ( alarm_mask_bits & 0x20 ) {
    // day of week
    day_value_mask |= 0x40;
    return (toBCD(value->dow) | day_value_mask);
  }
  else {
    // date
    day_value_mask &= ~0x40;
    return (toBCD(value->day) | day_value_mask);
  }
}

int DS3231RTC::readTemperature() {
  if ( _isRealTime ) {
    return ( ( (int)(signed char)readRegister(DS3231_TEMP_MSB) ) << 2 ) |
           (readRegister(DS3231_TEMP_LSB) >> 6);
  }
  else {
    return NO_TEMPERATURE;
  }
}

/**
 * \brief Enables the generation of interrupts for alarms 0 and 1.
 *
 * When the interrupt occurs, use firedAlarm() to determine which alarm
 * has fired.  The application is responsible for implementing the
 * interrupt service routine to watch for the interrupt.
 *
 * Note: this function does nothing if the 1 Hz pin was enabled in the
 * constructor, but firedAlarm() can still be used to determine which
 * alarm has fired when hasUpdates() reports that there is an update
 * available.
 *
 * \sa disableAlarmInterrupts(), firedAlarm()
 */
void DS3231RTC::enableAlarmInterrupts() {
  if ( ( _oneHzPin == 255) && _isRealTime ) {
    uint8_t value = readRegister(DS3231_CONTROL);
    value |= DS3231_INTCN;
    writeRegister(DS3231_CONTROL, value);
    alarmInterrupts = true;
  }
}

/**
 * \brief Disables the generation of interrupts for alarms 0 and 1.
 *
 * \sa enableAlarmInterrupts()
 */
void DS3231RTC::disableAlarmInterrupts() {
  if ( alarmInterrupts ) {
    uint8_t value = readRegister(DS3231_CONTROL);
    value &= ~(DS3231_INTCN | DS3231_A2IE | DS3231_A1IE);
    writeRegister(DS3231_CONTROL, value);
    alarmInterrupts = false;
  }
}

/**
 * \brief Determines which of alarms 0 or 1 have fired since the last call.
 *
 * Returns 0 if alarm 0 has fired, 1 if alarm 1 has fired, 2 if both alarms
 * have fired, or -1 if neither alarm has fired.
 *
 * The fired alarm state will be cleared, ready for the next call.
 *
 * This function cannot be used to determine if alarms 2 or 3 have fired
 * as they are stored in NVRAM and are not handled specially by the DS3231.
 *
 * \sa enableAlarmInterrupts()
 */
int DS3231RTC::firedAlarm() {
  if ( !_isRealTime ) {
    return -1;
  }
  uint8_t value = readRegister(DS3231_STATUS);
  int     alarm;
  if ( value & DS3231_A1F ) {
    if ( value & DS3231_A2F ) {
      alarm = 2;
    }
    else {
      alarm = 0;
    }
  }
  else if ( value & DS3231_A2F ) {
    alarm = 1;
  }
  else {
    alarm = -1;
  }
  if ( alarm != -1 ) {
    value &= ~(DS3231_A1F | DS3231_A2F);
    writeRegister(DS3231_STATUS, value);
  }
  return alarm;
}

/**
 * \brief Enables the 32 kHz output on the DS3231 chip.
 *
 * \sa disable32kHzOutput()
 */
void DS3231RTC::enable32kHzOutput() {
  if ( _isRealTime ) {
    uint8_t value = readRegister(DS3231_STATUS);
    value |= DS3231_BB32KHZ | DS3231_EN32KHZ;
    writeRegister(DS3231_STATUS, value);
  }
}

/**
 * \brief Disables the 32 kHz output on the DS3231 chip.
 *
 * \sa enable32kHzOutput()
 */
void DS3231RTC::disable32kHzOutput() {
  if ( _isRealTime ) {
    uint8_t value = readRegister(DS3231_STATUS);
    value &= ~(DS3231_BB32KHZ | DS3231_EN32KHZ);
    writeRegister(DS3231_STATUS, value);
  }
}

uint8_t DS3231RTC::readRegister(uint8_t reg) {
  _bus->startWrite(DS3231_I2C_ADDRESS);
  _bus->write(reg);
  if ( !_bus->startRead(DS3231_I2C_ADDRESS, 1) ) {
    return 0;       // RTC chip is not responding.
  }
  return _bus->read();
}

bool DS3231RTC::writeRegister(uint8_t reg, uint8_t value) {
  _bus->startWrite(DS3231_I2C_ADDRESS);
  _bus->write(reg);
  _bus->write(value);
  return _bus->endWrite();
}

void DS3231RTC::enableAlarm(uint8_t alarmNum) {
  uint8_t value = readRegister(DS3231_CONTROL);

  if ( 0 == alarmNum ) {
    value |= DS3231_A1IE;
  }
  else if ( 1 == alarmNum ) {
    value |= DS3231_A2IE;
  }
  writeRegister(DS3231_CONTROL, value);
}

void DS3231RTC::disableAlarm(uint8_t alarmNum) {
  uint8_t value = readRegister(DS3231_CONTROL);

  clearAlarm(alarmNum);

  if ( 0 == alarmNum ) {
    value &= ~DS3231_A1IE;
  }
  else if ( 1 == alarmNum ) {
    value &= ~DS3231_A2IE;
  }
  writeRegister(DS3231_CONTROL, value);
}
