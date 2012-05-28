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

#include "DS3232RTC.h"
#include "../I2C/I2CMaster.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class DS3232RTC DS3232RTC.h <DS3232RTC.h>
 * \brief Communicates with a DS3232 realtime clock chip via I2C.
 *
 * This class simplifies the process of reading and writing the time and
 * date information in a DS3232 realtime clock chip.  The class also
 * provides support for reading and writing information about alarms
 * and other clock settings.
 *
 * If there is no DS3232 chip on the I2C bus, this class will fall back to
 * the RTC class to simulate the current time and date based on the value
 * of millis().
 *
 * Alarms 0 and 1 can be set to generate an interrupt when they fire using
 * enableAlarmInterrupts().  The firedAlarm() function can be used to
 * determine which alarm has fired.  Alarms 2 and 3 cannot be monitored
 * with interrupts.
 *
 * The DS3232 uses a 2-digit year so this class is limited to dates between
 * 2000 and 2099 inclusive.
 *
 * Note: if this class has not been used with the DS3232 chip before,
 * then the contents of NVRAM will be cleared.  Any previous contents
 * will be lost.
 *
 * \sa RTC, DS1307RTC
 */

// I2C address of the RTC chip (7-bit).
#define DS3232_I2C_ADDRESS  0x68

// Registers.
#define DS3232_SECOND       0x00
#define DS3232_MINUTE       0x01
#define DS3232_HOUR         0x02
#define DS3232_DAY_OF_WEEK  0x03
#define DS3232_DATE         0x04
#define DS3232_MONTH        0x05
#define DS3232_YEAR         0x06
#define DS3232_ALARM1_SEC   0x07
#define DS3232_ALARM1_MIN   0x08
#define DS3232_ALARM1_HOUR  0x09
#define DS3232_ALARM1_DAY   0x0A
#define DS3232_ALARM2_MIN   0x0B
#define DS3232_ALARM2_HOUR  0x0C
#define DS3232_ALARM2_DAY   0x0D
#define DS3232_CONTROL      0x0E
#define DS3232_STATUS       0x0F
#define DS3232_AGING_OFFSET 0x10
#define DS3232_TEMP_MSB     0x11
#define DS3232_TEMP_LSB     0x12
#define DS3232_RESERVED     0x13
#define DS3232_NVRAM        0x14

// Bits in the DS3232_CONTROL register.
#define DS3232_EOSC         0x80
#define DS3232_BBSQW        0x40
#define DS3232_CONV         0x20
#define DS3232_RS_1HZ       0x00
#define DS3232_RS_1024HZ    0x08
#define DS3232_RS_4096HZ    0x10
#define DS3232_RS_8192HZ    0x18
#define DS3232_INTCN        0x04
#define DS3232_A2IE         0x02
#define DS3232_A1IE         0x01

// Bits in the DS3232_STATUS register.
#define DS3232_OSF          0x80
#define DS3232_BB32KHZ      0x40
#define DS3232_CRATE_64     0x00
#define DS3232_CRATE_128    0x10
#define DS3232_CRATE_256    0x20
#define DS3232_CRATE_512    0x30
#define DS3232_EN32KHZ      0x08
#define DS3232_BSY          0x04
#define DS3232_A2F          0x02
#define DS3232_A1F          0x01

// Alarm storage at the end of the RTC's NVRAM.
#define DS3232_ALARM_SIZE   3
#define DS3232_ALARMS       (256 - RTC::ALARM_COUNT * DS3232_ALARM_SIZE - 1)
#define DS3232_ALARM_MAGIC  255

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
DS3232RTC::DS3232RTC(I2CMaster &bus, uint8_t oneHzPin)
    : _bus(&bus)
    , _oneHzPin(oneHzPin)
    , prevOneHz(false)
    , _isRealTime(true)
    , alarmInterrupts(false)
{
    // Probe the device and configure it for our use.
    _bus->startWrite(DS3232_I2C_ADDRESS);
    _bus->write(DS3232_CONTROL);
    if (_bus->startRead(DS3232_I2C_ADDRESS, 1)) {
        uint8_t value = _bus->read() & DS3232_CONV;
        if (oneHzPin != 255)
            value |= DS3232_BBSQW | DS3232_RS_1HZ;
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_CONTROL);
        _bus->write(value);
        _bus->write(DS3232_CRATE_64);
        _bus->endWrite();
    } else {
        // Did not get an acknowledgement from the RTC chip.
        _isRealTime = false;
    }

    // Configure the 1 Hz square wave pin if required.
    if (oneHzPin != 255 && _isRealTime) {
        pinMode(oneHzPin, INPUT);
        digitalWrite(oneHzPin, HIGH);
    }

    // Initialize the alarms in the RTC chip's NVRAM.
    if (_isRealTime)
        initAlarms();
}

/**
 * \fn bool DS3232RTC::isRealTime() const
 * \brief Returns true if the realtime clock is on the I2C bus; false if the time and date are simulated.
 */

bool DS3232RTC::hasUpdates()
{
    // If not using a 1 Hz pin or there is no RTC chip available,
    // then assume that there is an update available.
    if (_oneHzPin == 255 || !_isRealTime)
        return true;

    // The DS3232 updates the internal registers on the falling edge of the
    // 1 Hz clock.  The values should be ready to read on the rising edge.
    bool value = digitalRead(_oneHzPin);
    if (value && !prevOneHz) {
        prevOneHz = value;
        return true;
    } else {
        prevOneHz = value;
        return false;
    }
}

inline uint8_t fromBCD(uint8_t value)
{
    return (value >> 4) * 10 + (value & 0x0F);
}

inline uint8_t fromHourBCD(uint8_t value)
{
    if ((value & 0x40) != 0) {
        // 12-hour mode.
        uint8_t result = ((value >> 4) & 0x01) * 10 + (value & 0x0F);
        if ((value & 0x20) != 0)
            return (result == 12) ? 12 : (result + 12);     // PM
        else
            return (result == 12) ? 0 : result;             // AM
    } else {
        // 24-hour mode.
        return fromBCD(value);
    }
}

void DS3232RTC::readTime(RTCTime *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_SECOND);
        if (_bus->startRead(DS3232_I2C_ADDRESS, 3)) {
            value->second = fromBCD(_bus->read());
            value->minute = fromBCD(_bus->read());
            value->hour = fromHourBCD(_bus->read());
        } else {
            // RTC chip is not responding.
            value->second = 0;
            value->minute = 0;
            value->hour = 0;
        }
    } else {
        RTC::readTime(value);
    }
}

void DS3232RTC::readDate(RTCDate *value)
{
    if (!_isRealTime) {
        RTC::readDate(value);
        return;
    }
    _bus->startWrite(DS3232_I2C_ADDRESS);
    _bus->write(DS3232_DATE);
    if (_bus->startRead(DS3232_I2C_ADDRESS, 3)) {
        value->day = fromBCD(_bus->read());
        value->month = fromBCD(_bus->read() & 0x7F); // Strip century bit.
        value->year = fromBCD(_bus->read()) + 2000;
    } else {
        // RTC chip is not responding.
        value->day = 1;
        value->month = 1;
        value->year = 2000;
    }
}

inline uint8_t toBCD(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}

void DS3232RTC::writeTime(const RTCTime *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_SECOND);
        _bus->write(toBCD(value->second));
        _bus->write(toBCD(value->minute));
        _bus->write(toBCD(value->hour));    // Changes mode to 24-hour clock.
        _bus->endWrite();
    } else {
        RTC::writeTime(value);
    }
}

void DS3232RTC::writeDate(const RTCDate *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_DATE);
        _bus->write(toBCD(value->day));
        _bus->write(toBCD(value->month));
        _bus->write(toBCD(value->year % 100));
        _bus->endWrite();
    } else {
        RTC::writeDate(value);
    }
}

void DS3232RTC::readAlarm(uint8_t alarmNum, RTCAlarm *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_ALARMS + alarmNum * DS3232_ALARM_SIZE);
        if (_bus->startRead(DS3232_I2C_ADDRESS, 3)) {
            value->hour = fromBCD(_bus->read());
            value->minute = fromBCD(_bus->read());
            value->flags = _bus->read();
        } else {
            // RTC chip is not responding.
            value->hour = 0;
            value->minute = 0;
            value->flags = 0;
        }
    } else {
        RTC::readAlarm(alarmNum, value);
    }
}

void DS3232RTC::writeAlarm(uint8_t alarmNum, const RTCAlarm *value)
{
    if (_isRealTime) {
        // Write the alarm details to NVRAM.
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_ALARMS + alarmNum * DS3232_ALARM_SIZE);
        _bus->write(toBCD(value->hour));
        _bus->write(toBCD(value->minute));
        _bus->write(value->flags);
        _bus->endWrite();

        // Keep the DS3232's built-in alarms in sync with the first two alarms.
        if (alarmNum == 0) {
            _bus->startWrite(DS3232_I2C_ADDRESS);
            _bus->write(DS3232_ALARM1_SEC);
            _bus->write(0);
            _bus->write(toBCD(value->minute));
            _bus->write(toBCD(value->hour));
            _bus->write(0x81);  // Match hours, mins, secs; day = 1
            _bus->endWrite();
            if (alarmInterrupts)
                updateAlarmInterrupts();
        } else if (alarmNum == 1) {
            _bus->startWrite(DS3232_I2C_ADDRESS);
            _bus->write(DS3232_ALARM2_MIN);
            _bus->write(toBCD(value->minute));
            _bus->write(toBCD(value->hour));
            _bus->write(0x81);  // Match hours, mins; day = 1
            _bus->endWrite();
            if (alarmInterrupts)
                updateAlarmInterrupts();
        }
    } else {
        RTC::writeAlarm(alarmNum, value);
    }
}

int DS3232RTC::byteCount() const
{
    return DS3232_ALARMS - DS3232_NVRAM;
}

uint8_t DS3232RTC::readByte(uint8_t offset)
{
    if (_isRealTime)
        return readRegister(DS3232_NVRAM + offset);
    else
        return RTC::readByte(offset);
}

void DS3232RTC::writeByte(uint8_t offset, uint8_t value)
{
    if (_isRealTime)
        writeRegister(DS3232_NVRAM + offset, value);
    else
        RTC::writeByte(offset, value);
}

int DS3232RTC::readTemperature()
{
    if (_isRealTime) {
        return (((int)(signed char)readRegister(DS3232_TEMP_MSB)) << 2) |
               (readRegister(DS3232_TEMP_LSB) >> 6);
    } else {
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
void DS3232RTC::enableAlarmInterrupts()
{
    if (_oneHzPin == 255 && _isRealTime) {
        updateAlarmInterrupts();
        alarmInterrupts = true;
    }
}

/**
 * \brief Disables the generation of interrupts for alarms 0 and 1.
 *
 * \sa enableAlarmInterrupts()
 */
void DS3232RTC::disableAlarmInterrupts()
{
    if (alarmInterrupts) {
        uint8_t value = readRegister(DS3232_CONTROL);
        value &= ~(DS3232_INTCN | DS3232_A2IE | DS3232_A1IE);
        writeRegister(DS3232_CONTROL, value);
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
 * as they are stored in NVRAM and are not handled specially by the DS3232.
 *
 * \sa enableAlarmInterrupts()
 */
int DS3232RTC::firedAlarm()
{
    if (!_isRealTime)
        return -1;
    uint8_t value = readRegister(DS3232_STATUS);
    int alarm;
    if (value & DS3232_A1F) {
        if (value & DS3232_A2F)
            alarm = 2;
        else
            alarm = 0;
    } else if (value & DS3232_A2F) {
        alarm = 1;
    } else {
        alarm = -1;
    }
    if (alarm != -1) {
        value &= ~(DS3232_A1F | DS3232_A2F);
        writeRegister(DS3232_STATUS, value);
    }
    return alarm;
}

/**
 * \brief Enables the 32 kHz output on the DS3232 chip.
 *
 * \sa disable32kHzOutput()
 */
void DS3232RTC::enable32kHzOutput()
{
    if (_isRealTime) {
        uint8_t value = readRegister(DS3232_STATUS);
        value |= DS3232_BB32KHZ | DS3232_EN32KHZ;
        writeRegister(DS3232_STATUS, value);
    }
}

/**
 * \brief Disables the 32 kHz output on the DS3232 chip.
 *
 * \sa enable32kHzOutput()
 */
void DS3232RTC::disable32kHzOutput()
{
    if (_isRealTime) {
        uint8_t value = readRegister(DS3232_STATUS);
        value &= ~(DS3232_BB32KHZ | DS3232_EN32KHZ);
        writeRegister(DS3232_STATUS, value);
    }
}

void DS3232RTC::initAlarms()
{
    uint8_t value = readRegister(DS3232_ALARM_MAGIC);
    if (value != (0xB0 + ALARM_COUNT)) {
        // This is the first time we have used this clock chip,
        // so initialize all alarms to their default state.
        RTCAlarm alarm;
        alarm.hour = 6;         // Default to 6am for alarms.
        alarm.minute = 0;
        alarm.flags = 0;
        for (uint8_t index = 0; index < ALARM_COUNT; ++index)
            writeAlarm(index, &alarm);
        writeRegister(DS3232_ALARM_MAGIC, 0xB0 + ALARM_COUNT);

        // Also clear the rest of NVRAM so that it is in a known state.
        // Otherwise we'll have whatever garbage was present at power-on.
        _bus->startWrite(DS3232_I2C_ADDRESS);
        _bus->write(DS3232_NVRAM);
        for (uint8_t index = DS3232_NVRAM; index < DS3232_ALARMS; ++index)
            _bus->write(0);
        _bus->endWrite();
    }
}

uint8_t DS3232RTC::readRegister(uint8_t reg)
{
    _bus->startWrite(DS3232_I2C_ADDRESS);
    _bus->write(reg);
    if (!_bus->startRead(DS3232_I2C_ADDRESS, 1))
        return 0;   // RTC chip is not responding.
    return _bus->read();
}

bool DS3232RTC::writeRegister(uint8_t reg, uint8_t value)
{
    _bus->startWrite(DS3232_I2C_ADDRESS);
    _bus->write(reg);
    _bus->write(value);
    return _bus->endWrite();
}

#define DS3232_ALARM1_FLAGS (DS3232_ALARMS + 2)
#define DS3232_ALARM2_FLAGS (DS3232_ALARMS + DS3232_ALARM_SIZE + 2)

void DS3232RTC::updateAlarmInterrupts()
{
    bool alarm1Enabled = ((readRegister(DS3232_ALARM1_FLAGS) & 0x01) != 0);
    bool alarm2Enabled = ((readRegister(DS3232_ALARM2_FLAGS) & 0x01) != 0);
    uint8_t value = readRegister(DS3232_CONTROL);
    value |= DS3232_INTCN;
    if (alarm1Enabled)
        value |= DS3232_A1IE;
    else
        value &= ~DS3232_A1IE;
    if (alarm2Enabled)
        value |= DS3232_A2IE;
    else
        value &= ~DS3232_A2IE;
    writeRegister(DS3232_CONTROL, value);
}
