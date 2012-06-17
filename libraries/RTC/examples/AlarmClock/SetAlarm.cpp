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

#include "SetAlarm.h"
#include "Clock.h"

#define EDIT_ENABLED        0
#define EDIT_HOUR           1
#define EDIT_MINUTE_TENS    2
#define EDIT_MINUTE         3
#define EDIT_DAYS           4

extern bool is24HourClock;

SetAlarm::SetAlarm(Form &form, const String &label, uint8_t alarmNum)
    : Field(form, label)
    , _alarmNum(alarmNum)
{
    RTCAlarm alarm;
    rtc.readAlarm(_alarmNum, &_value);
}

int SetAlarm::dispatch(int event)
{
    RTCAlarm newValue;
    if (event == LCD_BUTTON_UP) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue.hour = (newValue.hour + 1) % 24;
        } else if (editField == EDIT_MINUTE_TENS) {
            newValue.minute = (newValue.minute + 10) % 60;
        } else if (editField == EDIT_MINUTE) {
            newValue.minute = (newValue.minute + 1) % 60;
        } else if (editField == EDIT_ENABLED) {
            newValue.flags ^= 0x01;
            if (newValue.flags & 0x01)
                editField = EDIT_HOUR;
            setValue(newValue);
            return FORM_CHANGED;
        } else if (editField == EDIT_DAYS) {
            Days d = days();
            switch (d) {
            case AnyDay: d = SaturdayAndSunday; break;
            case MondayToFriday: d = AnyDay; break;
            case SaturdayAndSunday: d = MondayToFriday; break;
            }
            setDays(d);
            return FORM_CHANGED;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue.hour = (newValue.hour + 23) % 24;
        } else if (editField == EDIT_MINUTE_TENS) {
            newValue.minute = (newValue.minute + 50) % 60;
        } else if (editField == EDIT_MINUTE) {
            newValue.minute = (newValue.minute + 59) % 60;
        } else if (editField == EDIT_ENABLED) {
            newValue.flags ^= 0x01;
            if (newValue.flags & 0x01)
                editField = EDIT_HOUR;
            setValue(newValue);
            return FORM_CHANGED;
        } else if (editField == EDIT_DAYS) {
            Days d = days();
            switch (d) {
            case AnyDay: d = MondayToFriday; break;
            case MondayToFriday: d = SaturdayAndSunday; break;
            case SaturdayAndSunday: d = AnyDay; break;
            }
            setDays(d);
            return FORM_CHANGED;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_LEFT) {
        if (editField != EDIT_ENABLED) {
            --editField;
            printAlarm();
            return 0;
        }
    } else if (event == LCD_BUTTON_RIGHT) {
        if (isEnabled() && editField != EDIT_DAYS) {
            ++editField;
            printAlarm();
            return 0;
        }
    }
    return -1;
}

void SetAlarm::enterField(bool reverse)
{
    Field::enterField(reverse);
    if (isEnabled()) {
        if (reverse)
            editField = EDIT_DAYS;
        else
            editField = EDIT_ENABLED;
    } else {
        editField = EDIT_ENABLED;
    }
    printAlarm();
    lcd()->cursor();
}

void SetAlarm::exitField()
{
    lcd()->noCursor();
    Field::exitField();
}

void SetAlarm::setValue(const RTCAlarm &value)
{
    _value = value;
    rtc.writeAlarm(_alarmNum, &_value);
    if (isCurrent())
        printAlarm();
}

SetAlarm::Days SetAlarm::days() const
{
    return days(&_value);
}

void SetAlarm::setDays(Days days)
{
    _value.flags &= 0xE1;
    _value.flags |= (((uint8_t)days) << 1);
    rtc.writeAlarm(_alarmNum, &_value);
    if (isCurrent())
        printAlarm();
}

SetAlarm::Days SetAlarm::days(const RTCAlarm *alarm)
{
    // 4 bits are allocated for day indicators, to allow for later expansion.
    uint8_t d = (alarm->flags >> 1) & 0x0F;
    if (d > SaturdayAndSunday)
        d = AnyDay;
    return (Days)d;
}

// Combine the day flags for two alarms; e.g., if one is Mon-Fri and
// the other is Sat,Sun, then the result is AnyDay.
void SetAlarm::combineDays(RTCAlarm *alarm1, const RTCAlarm *alarm2)
{
    uint8_t d1 = (alarm1->flags >> 1) & 0x0F;
    if (d1 > SaturdayAndSunday)
        d1 = AnyDay;
    uint8_t d2 = (alarm2->flags >> 1) & 0x0F;
    if (d2 > SaturdayAndSunday)
        d2 = AnyDay;
    if (d1 != d2)
        d1 = AnyDay;
    alarm1->flags &= 0xE1;
    alarm1->flags |= (((uint8_t)d1) << 1);
}

static const char *daysShort[] = {" Mo-Su", " Mo-Fr", " Sa,Su"};
static const char *daysLong[] = {" Any day", " Mon-Fri", " Sat,Sun"};

void SetAlarm::printAlarm()
{
    lcd()->setCursor(0, 1);
    int hour = _value.hour;
    int minute = _value.minute;
    int timeCol = 3;
    int dayCol;
    bool pm;
    if (isEnabled()) {
        lcd()->print("On ");
    } else {
        lcd()->print("Off             ");
        lcd()->setCursor(0, 1);
        return;
    }
    if (is24HourClock) {
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = false;
    } else if (hour == 0 || hour == 12) {
        lcd()->write('1');
        lcd()->write('2');
        pm = (hour == 12);
    } else if (hour < 12) {
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = false;
    } else {
        hour -= 12;
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = true;
    }
    lcd()->write(':');
    lcd()->write('0' + minute / 10);
    lcd()->write('0' + minute % 10);
    if (is24HourClock) {
        dayCol = timeCol + 6;
        lcd()->print(daysLong[days()]);
    } else {
        lcd()->print(pm ? "pm" : "am");
        lcd()->print(daysShort[days()]);
        dayCol = timeCol + 8;
    }
    if (editField == EDIT_ENABLED)
        lcd()->setCursor(0, 1);
    else if (editField == EDIT_HOUR)
        lcd()->setCursor(timeCol + 1, 1);
    else if (editField == EDIT_MINUTE_TENS)
        lcd()->setCursor(timeCol + 3, 1);
    else if (editField == EDIT_MINUTE)
        lcd()->setCursor(timeCol + 4, 1);
    else if (editField == EDIT_DAYS)
        lcd()->setCursor(dayCol, 1);
}
