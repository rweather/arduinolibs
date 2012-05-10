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

// include the library code:
#include <FreetronicsLCD.h>
#include <Form.h>
#include <Field.h>

// Initialize the LCD
FreetronicsLCD lcd;

// Special characters for indicators.
#define IND_BATTERY_EMPTY       0
#define IND_BATTERY_20PCT       1
#define IND_BATTERY_40PCT       2
#define IND_BATTERY_60PCT       3
#define IND_BATTERY_80PCT       4
#define IND_BATTERY_FULL        5

// Specialized time/date display field for the front screen of the clock.
class FrontScreenField : public Field
{
public:
    explicit FrontScreenField(Form &form);
    ~FrontScreenField();

    void enterField(bool reverse);

    int day() const { return _day; }
    int month() const { return _month; }
    int year() const { return _year; }
    void setDate(int day, int month, int year);

    unsigned long time() const { return _time; }
    void setTime(unsigned long time);

    int batteryStatus() const { return _batteryStatus; }
    void setBatteryStatus(int batteryStatus);

private:
    int _day, _month, _year;
    unsigned long _time;
    int _batteryStatus;
    int _batteryBars;

    void updateDate();
    void updateTime();
    void updateBatteryStatus();
};

FrontScreenField::FrontScreenField(Form &form)
    : Field(form, "")
    , _day(1), _month(1), _year(2012)
    , _time(9 * 60 * 60)
    , _batteryStatus(100)
    , _batteryBars(IND_BATTERY_FULL)
{
}

FrontScreenField::~FrontScreenField()
{
}

void FrontScreenField::enterField(bool reverse)
{
    updateDate();
    updateBatteryStatus();
    updateTime();
}

const char *months[] = {
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Oct ", " Nov ", " Dec "
};
uint8_t monthLengths[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

inline bool isLeapYear(int year)
{
    if ((year % 100) == 0)
        return (year % 400) == 0;
    else
        return (year % 4) == 0;
}

void FrontScreenField::setDate(int day, int month, int year)
{
    if (day != _day || month != _month || year != _year) {
        if (day < 1) {
            // Rolled back into the previous month.
            if (month == 1)
                month = 12;
            else
                --month;
            if (month == 2 && isLeapYear(year))
                day = 29 + day;
            else
                day = monthLengths[month - 1] + day;
        } if (month == 2 && isLeapYear(year)) {
            if (day > 29) {
                // Rolled forward from Feb into Mar in a leap year.
                month = 3;
                day -= 29;
            }
        } else if (day > monthLengths[month - 1]) {
            // Rolled forward into the next month.
            day -= monthLengths[month - 1];
            if (month == 12)
                month = 1;
            else
                ++month;
        }
        _day = day;
        _month = month;
        _year = year;
        if (isCurrent())
            updateDate();
    }
}

void FrontScreenField::setTime(unsigned long time)
{
    if (time != _time) {
        _time = time;
        if (isCurrent())
            updateTime();
    }
}

void FrontScreenField::setBatteryStatus(int batteryStatus)
{
    _batteryStatus = batteryStatus;
    int ind;
    if (batteryStatus >= 85)
        ind = IND_BATTERY_FULL;
    else if (batteryStatus >= 75)
        ind = IND_BATTERY_80PCT;
    else if (batteryStatus >= 55)
        ind = IND_BATTERY_60PCT;
    else if (batteryStatus >= 35)
        ind = IND_BATTERY_40PCT;
    else if (batteryStatus >= 15)
        ind = IND_BATTERY_20PCT;
    else
        ind = IND_BATTERY_EMPTY;
    if (ind != _batteryBars) {
        _batteryBars = ind;
        updateBatteryStatus();
    }
}

void FrontScreenField::updateDate()
{
    lcd()->setCursor(0, 0);
    if (_day < 10) {
        lcd()->write('0' + _day);
    } else {
        lcd()->write('0' + _day / 10);
        lcd()->write('0' + _day % 10);
    }
    lcd()->print(months[_month - 1]);
    lcd()->print(_year);
    lcd()->write(' ');
}

void FrontScreenField::updateTime()
{
    lcd()->setCursor(0, 1);
    int hour = (int)(_time / (60 * 60));
    int minute = ((int)(_time / 60)) % 60;
    int second = (int)(_time % 60);
    bool pm;
    if (hour == 0 || hour == 12) {
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
    lcd()->write(':');
    lcd()->write('0' + second / 10);
    lcd()->write('0' + second % 10);
    lcd()->print(pm ? " PM" : " AM");
}

void FrontScreenField::updateBatteryStatus()
{
    lcd()->setCursor(15, 0);
    lcd()->write(_batteryBars);
}

// Create the main form and its fields.
Form mainForm(lcd);
FrontScreenField frontScreen(mainForm);

#define STATUS_LED 13

#define MILLIS_PER_DAY      86400000UL
#define MILLIS_PER_SECOND   1000UL
#define MILLIS_PER_HOUR     3600000UL

unsigned long midnightTime;

byte batteryEmpty[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111,
    B00000
};
byte battery20Pct[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111,
    B00000
};
byte battery40Pct[8] = {
    B01110,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B00000
};
byte battery60Pct[8] = {
    B01110,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};
byte battery80Pct[8] = {
    B01110,
    B10001,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};
byte batteryFull[8] = {
    B01110,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B00000
};

void setup() {
    // Turn off the status LED.  Don't need it.
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW);

    // We need some special characters for battery status and other indicators.
    lcd.createChar(IND_BATTERY_EMPTY, batteryEmpty);
    lcd.createChar(IND_BATTERY_20PCT, battery20Pct);
    lcd.createChar(IND_BATTERY_40PCT, battery40Pct);
    lcd.createChar(IND_BATTERY_60PCT, battery60Pct);
    lcd.createChar(IND_BATTERY_80PCT, battery80Pct);
    lcd.createChar(IND_BATTERY_FULL,  batteryFull);

    //lcd.enableScreenSaver();

    // At startup, make "now" be 9am.  TODO: Read from an RTC chip instead.
    midnightTime = millis() - MILLIS_PER_HOUR * 9;

    // Show the main form for the first time.
    mainForm.show();
}

void loop() {
    // Update the number of seconds since the last midnight event.
    unsigned long sinceMidnight = millis() - midnightTime;
    if (sinceMidnight >= MILLIS_PER_DAY) {
        // We have overflowed into the next day.  Readjust midnight.
        midnightTime += MILLIS_PER_DAY;
        sinceMidnight -= MILLIS_PER_DAY;

        // Increment the date using the rollover logic.
        frontScreen.setDate(frontScreen.day() + 1,
                            frontScreen.month(),
                            frontScreen.year());
    }
    frontScreen.setTime(sinceMidnight / MILLIS_PER_SECOND);

    // Dispatch button events to the main form.
    int event = lcd.getButton();
    if (mainForm.dispatch(event) == FORM_CHANGED) {
        // TODO
    }
}
