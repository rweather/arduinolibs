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

#include "RTC.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <stdlib.h>
#include <string.h>

/**
 * \class RTC RTC.h <RTC.h>
 * \brief Base class for realtime clock handlers.
 *
 * This class simplifies the process of reading and writing the time and
 * date information in a realtime clock chip.  The class also provides
 * support for reading and writing information about alarms and other
 * clock settings.
 *
 * It is intended that the application will instantiate a subclass of this
 * class to handle the specific realtime clock chip in the system.  The default
 * implementation in RTC simulates a clock based on the value of millis(),
 * with alarms and clock settings stored in main memory.
 *
 * Because the common DS1307 and DS3232 realtime clock chips use a
 * 2-digit year, this class is also limited to dates between 2000
 * and 2099 inclusive.
 *
 * \sa RTCTime, RTCDate, RTCAlarm, DS1307RTC, DS3232RTC
 */

/**
 * \var RTC::ALARM_COUNT
 * \brief Number of alarms that are supported by RTC::readAlarm() and RTC::writeAlarm().
 */

#define DEFAULT_BYTE_COUNT  43  // Default simulates DS1307 NVRAM size.

#define MILLIS_PER_DAY      86400000UL
#define MILLIS_PER_SECOND   1000UL
#define MILLIS_PER_MINUTE   60000UL
#define MILLIS_PER_HOUR     3600000UL

static uint8_t monthLengths[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static unsigned int monthOffsets[] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
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
 * \brief Constructs a new realtime clock handler.
 *
 * \sa hasUpdates()
 */
RTC::RTC()
    : midnight(millis() - 9 * MILLIS_PER_HOUR) // Simulated clock starts at 9am
    , nvram(0)
{
    // Start the simulated date at 1 Jan, 2000.
    date.day = 1;
    date.month = 1;
    date.year = 2000;

    // Set all simulated alarms to 6am by default.
    for (uint8_t index = 0; index < ALARM_COUNT; ++index) {
        alarms[index].hour = 6;
        alarms[index].minute = 0;
        alarms[index].flags = 0;
    }
}

RTC::~RTC()
{
    if (nvram)
        free(nvram);
}

/**
 * \brief Returns true if the realtime clock has updated since the last call to this function.
 *
 * The default implementation returns true, indicating that an update is
 * always available to be read.
 */
bool RTC::hasUpdates()
{
    return true;
}

/**
 * \brief Reads the current time from the realtime clock into \a value.
 *
 * \sa writeTime(), readDate()
 */
void RTC::readTime(RTCTime *value)
{
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

/**
 * \brief Reads the current date from the realtime clock into \a value.
 *
 * The time should be read first with readTime() as the default implementation
 * only advances the date when the time is read and it crosses midnight.
 *
 * \sa writeDate(), readTime()
 */
void RTC::readDate(RTCDate *value)
{
    *value = date;
}

/**
 * \brief Updates the time in the realtime clock to match \a value.
 *
 * \sa readTime(), writeDate()
 */
void RTC::writeTime(const RTCTime *value)
{
    // Adjust the position of the last simulated midnight event.
    unsigned long sinceMidnight =
        value->second * MILLIS_PER_SECOND +
        value->minute * MILLIS_PER_MINUTE +
        value->hour   * MILLIS_PER_HOUR;
    midnight = millis() - sinceMidnight;
}

/**
 * \brief Updates the date in the realtime clock to match \a value.
 *
 * \sa readDate(), writeTime()
 */
void RTC::writeDate(const RTCDate *value)
{
    date = *value;
}

/**
 * \brief Reads the details of the alarm with index \a alarmNum into \a value.
 *
 * The \a alarmNum parameter must be between 0 and \ref ALARM_COUNT - 1.
 *
 * Alarm details are stored at the end of the realtime clock's non-volatile
 * memory.
 *
 * \sa writeAlarm(), alarmCount()
 */
void RTC::readAlarm(uint8_t alarmNum, RTCAlarm *value)
{
    *value = alarms[alarmNum];
}

/**
 * \brief Updates the details of the alarm with index \a alarmNum from \a value.
 *
 * The \a alarmNum parameter must be between 0 and \ref ALARM_COUNT - 1.
 *
 * Alarm details are stored at the end of the realtime clock's non-volatile
 * memory.
 *
 * \sa readAlarm(), alarmCount()
 */
void RTC::writeAlarm(uint8_t alarmNum, const RTCAlarm *value)
{
    alarms[alarmNum] = *value;
}

/**
 * \brief Returns the number of bytes of non-volatile memory that can be
 * used for storage of arbitrary settings, excluding storage used by alarms.
 *
 * \sa readByte(), writeByte()
 */
int RTC::byteCount() const
{
    return DEFAULT_BYTE_COUNT;
}

/**
 * \brief Reads the byte at \a offset within the realtime clock's non-volatile memory.
 *
 * The \a offset parameter must be between 0 and byteCount() - 1.
 *
 * \sa writeByte(), byteCount()
 */
uint8_t RTC::readByte(uint8_t offset)
{
    if (nvram)
        return nvram[offset];
    else
        return 0;
}

/**
 * \brief Writes \a value to \a offset within the realtime clock's non-volatile memory.
 *
 * The \a offset parameter must be between 0 and byteCount() - 1.
 *
 * \sa readByte(), byteCount()
 */
void RTC::writeByte(uint8_t offset, uint8_t value)
{
    if (nvram) {
        nvram[offset] = value;
    } else {
        nvram = (uint8_t *)malloc(DEFAULT_BYTE_COUNT);
        if (nvram) {
            memset(nvram, 0, DEFAULT_BYTE_COUNT);
            nvram[offset] = value;
        }
    }
}

/**
 * \var RTC::NO_TEMPERATURE
 * \brief Value that is returned from readTemperature() if the realtime
 * clock chip cannot determine the temperature.
 */

/**
 * \brief Reads the value of the temperature sensor and returns the
 * temperature in quarters of a degree celcius.
 *
 * Returns the value NO_TEMPERATURE if the realtime clock chip cannot
 * determine the temperature.
 */
int RTC::readTemperature()
{
    return NO_TEMPERATURE;
}

/**
 * \var RTC::INCREMENT
 * \brief Increment the day, month, or year in a call to adjustDays(), adjustMonths(), or adjustYears().
 */

/**
 * \var RTC::DECREMENT
 * \brief Decrement the day, month, or year in a call to adjustDays(), adjustMonths(), or adjustYears().
 */

/**
 * \var RTC::WRAP
 * \brief Wrap around to the beginning of the current month/year rather than advance to the next one.
 */

/**
 * \brief Adjusts \a date up or down one day according to \a flags.
 *
 * \sa adjustMonths(), adjustYears()
 */
void RTC::adjustDays(RTCDate *date, uint8_t flags)
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
void RTC::adjustMonths(RTCDate *date, uint8_t flags)
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
void RTC::adjustYears(RTCDate *date, uint8_t flags)
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

/**
 * \enum RTC::DayOfWeek
 * \brief Day of the week corresponding to a date.
 *
 * \sa dayOfWeek()
 */

/**
 * \brief Returns the day of the week corresponding to \a date.
 *
 * This function is only guaranteed to produce meaningful values
 * for years between 2000 and 2099.
 */
RTC::DayOfWeek RTC::dayOfWeek(const RTCDate *date)
{
    // The +4 here adjusts for Jan 1, 2000 being a Saturday.
    unsigned long daynum = date->day + 4;
    daynum += monthOffsets[date->month - 1];
    if (date->month > 2 && isLeapYear(date->year))
        ++daynum;
    daynum += 365UL * (date->year - 2000);
    if (date->year > 2000)
        daynum += ((date->year - 2001) / 4) + 1;
    return (DayOfWeek)((daynum % 7) + 1);
}

/**
 * \class RTCTime RTC.h <RTC.h>
 * \brief Stores time information from a realtime clock chip.
 *
 * \sa RTCDate, RTCAlarm, RTC
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
 * \class RTCDate RTC.h <RTC.h>
 * \brief Stores date information from a realtime clock chip.
 *
 * \sa RTCTime, RTCAlarm, RTC
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
 * \class RTCAlarm RTC.h <RTC.h>
 * \brief Stores alarm information from a realtime clock chip.
 *
 * \sa RTCTime, RTCDate, RTC
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
