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

#include "DS1307RTC.h"
#include <WProgram.h>
#include <stdlib.h>
#include <string.h>

/**
 * \class DS1307RTC DS1307RTC.h <DS1307RTC.h>
 * \brief Communicates with a DS1307 realtime clock chip via I2C.
 *
 * This class simplifies the process of reading and writing the time and
 * date information in a DS1307 realtime clock chip.  The class also
 * provides support for reading and writing information about alarms
 * and other clock settings.
 *
 * If there is no DS1307 chip on the I2C bus, this class will simulate the
 * current time and date based on the value of millis().  At startup the
 * simulated time and date is set to 9am on the 1st of January, 2012.
 *
 * The DS1307 uses a 2-digit year so this class is limited to dates between
 * 2000 and 2099 inclusive.
 *
 * \sa RTCTime, RTCDate, RTCAlarm
 */

// I2C address of the RTC chip (7-bit).
#define DS1307_I2C_ADDRESS  0x68

// Registers.
#define DS1307_SECOND       0x00
#define DS1307_MINUTE       0x01
#define DS1307_HOUR         0x02
#define DS1307_DAY_OF_WEEK  0x03
#define DS1307_DATE         0x04
#define DS1307_MONTH        0x05
#define DS1307_YEAR         0x06
#define DS1307_CONTROL      0x07
#define DS1307_NVRAM        0x08

// Alarm storage at the end of the RTC's NVRAM.
#define DS1307_NUM_ALARMS   4
#define DS1307_ALARM_SIZE   3
#define DS1307_ALARMS       (64 - DS1307_NUM_ALARMS * DS1307_ALARM_SIZE - 1)
#define DS1307_ALARM_MAGIC  63

#define MILLIS_PER_DAY      86400000UL
#define MILLIS_PER_SECOND   1000UL
#define MILLIS_PER_MINUTE   60000UL
#define MILLIS_PER_HOUR     3600000UL

static uint8_t monthLengths[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

inline bool isLeapYear(unsigned int year)
{
    if ((year % 100) == 0)
        return (year % 400) == 0;
    else
        return (year % 4) == 0;
}

inline uint8_t monthLength(const RTCDate *date)
{
    if (date->month != 2 || !isLeapYear(date->year))
        return monthLengths[date->month - 1];
    else
        return 29;
}

/**
 * \brief Attaches to a realtime clock slave device on \a bus.
 *
 * If \a oneHzPin is not 255, then it indicates a digital input pin
 * that is connected to the 1 Hz square wave output on the realtime clock.
 * This input is used by hasUpdates() to determine if the time information
 * has changed in a non-trivial manner.
 *
 * \sa hasUpdates()
 */
DS1307RTC::DS1307RTC(BitBangI2C &bus, uint8_t oneHzPin)
    : _bus(&bus)
    , _oneHzPin(oneHzPin)
    , _alarmCount(0)
    , _alarmOffset(0)
    , prevOneHz(false)
    , _isRealTime(true)
    , midnight(millis() - 9 * MILLIS_PER_HOUR) // Simulated clock starts at 9am
    , alarms(0)
    , nvram(0)
{
    // Start the simulated date at 1 Jan, 2012.
    date.day = 1;
    date.month = 1;
    date.year = 2012;

    // Make sure the CH bit in register 0 is off or the clock won't update.
    if (_bus->startWrite(DS1307_I2C_ADDRESS) == BitBangI2C::ACK) {
        _bus->write(DS1307_SECOND);
        _bus->startRead(DS1307_I2C_ADDRESS);
        uint8_t value = _bus->read(BitBangI2C::NACK);
        _bus->stop();
        if ((value & 0x80) != 0) {
            _bus->startWrite(DS1307_I2C_ADDRESS);
            _bus->write(DS1307_SECOND);
            _bus->write(value & 0x7F);
            _bus->stop();
        }
    } else {
        // Did not get an acknowledgement from the RTC chip.
        _isRealTime = false;
        _bus->stop();
    }

    // Turn on the 1 Hz square wave signal if required.
    if (oneHzPin != 255 && _isRealTime) {
        pinMode(oneHzPin, INPUT);
        digitalWrite(oneHzPin, HIGH);
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_CONTROL);
        _bus->write(0x10);
        _bus->stop();
    }
}

DS1307RTC::~DS1307RTC()
{
    if (alarms)
        free(alarms);
    if (nvram)
        free(nvram);
}

/**
 * \fn bool DS1307RTC::isRealTime() const
 * \brief Returns true if the realtime clock is on the I2C bus; false if the time and date are simulated.
 */

/**
 * \brief Returns true if the realtime clock has updated since the last call to this function.
 *
 * If the object is not using the 1 Hz pin, then this function will always
 * return true.  Otherwise it only returns true when the 1 Hz square wave
 * output pin on the DS1307 changes from low to high.
 */
bool DS1307RTC::hasUpdates()
{
    // If not using a 1 Hz pin or there is no RTC chip available,
    // then assume that there is an update available.
    if (_oneHzPin == 255 || !_isRealTime)
        return true;

    // The DS1307 updates the internal registers on the falling edge of the
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

/**
 * \brief Reads the current time from the realtime clock into \a value.
 *
 * \sa writeTime(), readDate()
 */
void DS1307RTC::readTime(RTCTime *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_SECOND);
        _bus->startRead(DS1307_I2C_ADDRESS);
        value->second = fromBCD(_bus->read() & 0x7F);
        value->minute = fromBCD(_bus->read());
        value->hour = fromHourBCD(_bus->read(BitBangI2C::NACK));
        _bus->stop();
    } else {
        // Determine the number of seconds since the last midnight event.
        unsigned long sinceMidnight = millis() - midnight;
        if (sinceMidnight >= MILLIS_PER_DAY) {
            // We have overflowed into the next day.  Readjust midnight.
            midnight += MILLIS_PER_DAY;
            sinceMidnight -= MILLIS_PER_DAY;

            // Increment the simulated date.
            adjustDays(&date, INCREMENT);
        }
        value->second = (uint8_t)(((sinceMidnight / MILLIS_PER_SECOND) % 60));
        value->minute = (uint8_t)(((sinceMidnight / MILLIS_PER_MINUTE) % 60));
        value->hour = (uint8_t)(sinceMidnight / MILLIS_PER_HOUR);
    }
}

/**
 * \brief Reads the current date from the realtime clock into \a value.
 *
 * \sa writeDate(), readTime()
 */
void DS1307RTC::readDate(RTCDate *value)
{
    if (!_isRealTime) {
        *value = date;
        return;
    }
    _bus->startWrite(DS1307_I2C_ADDRESS);
    _bus->write(DS1307_DATE);
    _bus->startRead(DS1307_I2C_ADDRESS);
    value->day = fromBCD(_bus->read());
    value->month = fromBCD(_bus->read());
    value->year = fromBCD(_bus->read(BitBangI2C::NACK)) + 2000;
    _bus->stop();
}

inline uint8_t toBCD(uint8_t value)
{
    return ((value / 10) << 4) + (value % 10);
}

/**
 * \brief Updates the time in the realtime clock to match \a value.
 *
 * \sa readTime(), writeDate()
 */
void DS1307RTC::writeTime(const RTCTime *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_SECOND);
        _bus->write(toBCD(value->second));
        _bus->write(toBCD(value->minute));
        _bus->write(toBCD(value->hour));    // Changes mode to 24-hour clock.
        _bus->stop();
    } else {
        // Adjust the position of the last simulated midnight event.
        unsigned long sinceMidnight =
            value->second * MILLIS_PER_SECOND +
            value->minute * MILLIS_PER_MINUTE +
            value->hour   * MILLIS_PER_HOUR;
        midnight = millis() - sinceMidnight;
    }
}

/**
 * \brief Updates the date in the realtime clock to match \a value.
 *
 * \sa readDate(), writeTime()
 */
void DS1307RTC::writeDate(const RTCDate *value)
{
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_DATE);
        _bus->write(toBCD(value->day));
        _bus->write(toBCD(value->month));
        _bus->write(toBCD(value->year % 100));
        _bus->stop();
    } else {
        date = *value;
    }
}

/**
 * \brief Returns the number of alarms that can be stored in the realtime clock's non-volatile memory.
 *
 * At least 4 alarms will be provided by this class.
 *
 * \sa readAlarm(), writeAlarm()
 */
uint8_t DS1307RTC::alarmCount()
{
    initAlarms();
    return _alarmCount;
}

/**
 * \brief Reads the details of the alarm with index \a alarmNum into \a value.
 *
 * Alarm details are stored at the end of the realtime clock's non-volatile
 * memory.
 *
 * \sa writeAlarm(), alarmCount()
 */
void DS1307RTC::readAlarm(uint8_t alarmNum, RTCAlarm *value)
{
    initAlarms();
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_ALARMS + alarmNum * DS1307_ALARM_SIZE);
        _bus->startRead(DS1307_I2C_ADDRESS);
        value->hour = fromBCD(_bus->read());
        value->minute = fromBCD(_bus->read());
        value->flags = _bus->read(BitBangI2C::NACK);
        _bus->stop();
    } else if (alarms) {
        *value = alarms[alarmNum];
    } else {
        // Alarms default to 6am when simulated.
        value->hour = 6;
        value->minute = 0;
        value->flags = 0;
    }
}

/**
 * \brief Updates the details of the alarm with index \a alarmNum from \a value.
 *
 * Alarm details are stored at the end of the realtime clock's non-volatile
 * memory.
 *
 * \sa readAlarm(), alarmCount()
 */
void DS1307RTC::writeAlarm(uint8_t alarmNum, const RTCAlarm *value)
{
    initAlarms();
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_ALARMS + alarmNum * DS1307_ALARM_SIZE);
        _bus->write(toBCD(value->hour));
        _bus->write(toBCD(value->minute));
        _bus->write(value->flags);
        _bus->stop();
    } else if (alarms) {
        alarms[alarmNum] = *value;
    }
}

/**
 * \brief Reads the byte at \a offset within the realtime clock's non-volatile memory.
 *
 * \sa writeByte()
 */
uint8_t DS1307RTC::readByte(uint8_t offset)
{
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_NVRAM + offset);
        _bus->startRead(DS1307_I2C_ADDRESS);
        uint8_t value = _bus->read(BitBangI2C::NACK);
        _bus->stop();
        return value;
    } else if (nvram) {
        return nvram[offset];
    } else {
        return 0xFF;
    }
}

/**
 * \brief Writes \a value to \a offset within the realtime clock's non-volatile memory.
 *
 * \sa readByte()
 */
void DS1307RTC::writeByte(uint8_t offset, uint8_t value)
{
    if (_isRealTime) {
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_NVRAM + offset);
        _bus->write(value);
        _bus->stop();
    } else if (nvram) {
        nvram[offset] = value;
    } else {
        nvram = (uint8_t *)malloc(DS1307_ALARMS - DS1307_NVRAM);
        if (nvram) {
            memset(nvram, 0xFF, DS1307_ALARMS - DS1307_NVRAM);
            nvram[offset] = value;
        }
    }
}

/**
 * \var DS1307RTC::INCREMENT
 * \brief Increment the day, month, or year.
 */

/**
 * \var DS1307RTC::DECREMENT
 * \brief Decrement the day, month, or year.
 */

/**
 * \var DS1307RTC::WRAP
 * \brief Wrap around to the beginning of the current month/year rather than advance to the next one.
 */

/**
 * \brief Adjusts \a date up or down one day according to \a flags.
 *
 * \sa adjustMonths(), adjustYears()
 */
void DS1307RTC::adjustDays(RTCDate *date, uint8_t flags)
{
    if (flags & DECREMENT) {
        --(date->day);
        if (date->day == 0) {
            if (!(flags & WRAP)) {
                --(date->month);
                if (date->month == 0)
                    date->month = 12;
            }
            date->day = monthLength(date);
        }
    } else {
        ++(date->day);
        if (date->day > monthLength(date)) {
            if (!(flags & WRAP)) {
                ++(date->month);
                if (date->month == 13)
                    date->month = 1;
            }
            date->day = 1;
        }
    }
}

/**
 * \brief Adjusts \a date up or down one month according to \a flags.
 *
 * \sa adjustDays(), adjustYears()
 */
void DS1307RTC::adjustMonths(RTCDate *date, uint8_t flags)
{
    if (flags & DECREMENT) {
        --(date->month);
        if (date->month == 0) {
            date->month = 12;
            if (!(flags & WRAP) && date->year > 2000)
                --(date->year);
        }
    } else {
        ++(date->month);
        if (date->month == 13) {
            date->month = 1;
            if (!(flags & WRAP) && date->year < 2099)
                ++(date->year);
        }
    }
    uint8_t len = monthLength(date);
    if (date->day > len)
        date->day = len;
}

/**
 * \brief Adjusts \a date up or down one year according to \a flags.
 *
 * \sa adjustDays(), adjustMonths()
 */
void DS1307RTC::adjustYears(RTCDate *date, uint8_t flags)
{
    if (flags & DECREMENT) {
        --(date->year);
        if (date->year < 2000)
            date->year = 2000;
    } else {
        ++(date->year);
        if (date->year > 2099)
            date->year = 2099;
    }
    uint8_t len = monthLength(date);
    if (date->day > len)
        date->day = len;
}

void DS1307RTC::initAlarms()
{
    if (_alarmCount != 0)
        return;
    if (!_isRealTime) {
        // No RTC clock available, so fake the alarms.
        alarms = (RTCAlarm *)malloc(sizeof(RTCAlarm) * DS1307_NUM_ALARMS);
        RTCAlarm alarm;
        alarm.hour = 6;         // Default to 6am for alarms.
        alarm.minute = 0;
        alarm.flags = 0;
        for (uint8_t index = 0; index < _alarmCount; ++index)
            writeAlarm(index, &alarm);
        _alarmCount = DS1307_NUM_ALARMS;
        return;
    }
    _bus->startWrite(DS1307_I2C_ADDRESS);
    _bus->write(DS1307_ALARM_MAGIC);
    _bus->startRead(DS1307_I2C_ADDRESS);
    uint8_t value = _bus->read(BitBangI2C::NACK);
    _bus->stop();
    if ((value & 0xF0) == 0xB0) {
        // RTC chip was previously initialized with alarm information.
        _alarmCount = value & 0x0F;
    } else {
        // This is the first time we have used this clock chip,
        // so initialize all alarms to their default state.
        // Note: if the number of alarms is changed in the future,
        // then the high nibble will still be 0xB, with the low
        // nibble the actual number of alarms that are stored.
        _alarmCount = DS1307_NUM_ALARMS;
        RTCAlarm alarm;
        alarm.hour = 6;         // Default to 6am for alarms.
        alarm.minute = 0;
        alarm.flags = 0;
        for (uint8_t index = 0; index < _alarmCount; ++index)
            writeAlarm(index, &alarm);
        _bus->startWrite(DS1307_I2C_ADDRESS);
        _bus->write(DS1307_ALARM_MAGIC);
        _bus->write(0xB0 + _alarmCount);
        _bus->stop();
    }
    _alarmOffset = DS1307_ALARM_MAGIC - _alarmCount * DS1307_ALARM_SIZE;
}

/**
 * \class RTCTime DS1307RTC.h <DS1307RTC.h>
 * \brief Stores time information from a realtime clock chip.
 *
 * \sa RTCDate, RTCAlarm, DS1307RTC
 */

/**
 * \var RTCTime::hour
 * \brief Hour of the day (0-23)
 */

/**
 * \var RTCTime::minute
 * \brief Minute within the hour (0-59)
 */

/**
 * \var RTCTime::second
 * \brief Second within the minute (0-59)
 */

/**
 * \class RTCDate DS1307RTC.h <DS1307RTC.h>
 * \brief Stores date information from a realtime clock chip.
 *
 * \sa RTCTime, RTCAlarm, DS1307RTC
 */

/**
 * \var RTCDate::year
 * \brief Year (4-digit)
 */

/**
 * \var RTCDate::month
 * \brief Month of the year (1-12)
 */

/**
 * \var RTCDate::day
 * \brief Day of the month (1-31)
 */

/**
 * \class RTCAlarm DS1307RTC.h <DS1307RTC.h>
 * \brief Stores alarm information from a realtime clock chip.
 *
 * \sa RTCTime, RTCDate, DS1307RTC
 */

/**
 * \var RTCAlarm::hour
 * \brief Hour of the day for the alarm (0-23).
 */

/**
 * \var RTCAlarm::minute
 * \brief Minute of the hour for the alarm (0-59).
 */

/**
 * \var RTCAlarm::flags
 * \brief Additional flags for the alarm.
 *
 * The least significant bit will be 0 if the alarm is disabled or
 * 1 if the alarm is enabled.  Other bits can be used by the application
 * for any purpose.
 */
