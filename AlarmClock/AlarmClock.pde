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
#include <DS1307RTC.h>

// I/O pins that are used by this sketch.
#define BUZZER                  12
#define SENSE_BATTERY           A1
#define RTC_DATA                A3
#define RTC_CLOCK               A4
#define RTC_ONE_HZ              A5

// Value to adjust for the voltage drop on D2.
#define VOLTAGE_DROP_ADJUST     70  // 0.7 volts

// Special characters for indicators.
#define IND_BATTERY_EMPTY       0
#define IND_BATTERY_20PCT       1
#define IND_BATTERY_40PCT       2
#define IND_BATTERY_60PCT       3
#define IND_BATTERY_80PCT       4
#define IND_BATTERY_FULL        5
#define IND_ALARM_ACTIVE1       6
#define IND_ALARM_ACTIVE2       7

// Initialize the LCD
FreetronicsLCD lcd;

// Activate the realtime clock chip.
BitBangI2C bus(RTC_DATA, RTC_CLOCK);
DS1307RTC rtc(bus, RTC_ONE_HZ);

bool isAlarmOn = false;

// Specialized time/date display field for the front screen of the clock.
class FrontScreenField : public Field
{
public:
    explicit FrontScreenField(Form &form);
    ~FrontScreenField();

    void enterField(bool reverse);

    RTCDate date() const { return _date; }
    void setDate(const RTCDate &date);

    RTCTime time() const { return _time; }
    void setTime(const RTCTime &time);

    int voltage() const { return _voltage; }
    void setVoltage(int voltage);

    bool isAlarmActive() const { return _alarmActive; }
    void setAlarmActive(bool active);

private:
    RTCDate _date;
    RTCTime _time;
    int _voltage;
    int _voltageTrunc;
    int _batteryBars;
    bool _alarmActive;

    void updateDate();
    void updateTime();
    void updateVoltage();
    void updateAlarm();
};

FrontScreenField::FrontScreenField(Form &form)
    : Field(form, "")
    , _voltage(360)
    , _voltageTrunc(36)
    , _batteryBars(IND_BATTERY_FULL)
    , _alarmActive(false)
{
    _date.day = 1;
    _date.month = 1;
    _date.year = 2012;
    _time.hour = 9;
    _time.minute = 0;
    _time.second = 0;
}

FrontScreenField::~FrontScreenField()
{
}

void FrontScreenField::enterField(bool reverse)
{
    updateDate();
    updateVoltage();
    updateTime();
    updateAlarm();
}

const char *months[] = {
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Oct ", " Nov ", " Dec "
};

uint8_t prevHour = 24;

void FrontScreenField::setDate(const RTCDate &date)
{
    if (date.day != _date.day || date.month != _date.month ||
            date.year != _date.year) {
        _date = date;
        if (isCurrent())
            updateDate();
    }
}

void FrontScreenField::setTime(const RTCTime &time)
{
    if (time.hour != _time.hour || time.minute != _time.minute ||
            time.second != _time.second) {
        _time = time;
        if (isCurrent())
            updateTime();
    }
}

void FrontScreenField::setVoltage(int voltage)
{
    // Normal voltage ranges between 2.7 and 3.6.  The power supply
    // for the clock will no longer function below 2.7 volts.
    if (_voltage == voltage)
        return;
    _voltage = voltage;
    int ind;
    if (voltage > 355)
        ind = IND_BATTERY_FULL;
    else if (voltage > 345)
        ind = IND_BATTERY_80PCT;
    else if (voltage > 325)
        ind = IND_BATTERY_60PCT;
    else if (voltage > 305)
        ind = IND_BATTERY_40PCT;
    else if (voltage > 285)
        ind = IND_BATTERY_20PCT;
    else
        ind = IND_BATTERY_EMPTY;
    int trunc = voltage / 10;
    if (ind != _batteryBars || trunc != _voltageTrunc) {
        _batteryBars = ind;
        _voltageTrunc = trunc;
        updateVoltage();
    }
}

void FrontScreenField::setAlarmActive(bool active)
{
    if (_alarmActive != active) {
        _alarmActive = active;
        if (isCurrent())
            updateAlarm();
    }
}

void FrontScreenField::updateDate()
{
    lcd()->setCursor(0, 0);
    if (_date.day < 10) {
        lcd()->write('0' + _date.day);
    } else {
        lcd()->write('0' + _date.day / 10);
        lcd()->write('0' + _date.day % 10);
    }
    lcd()->print(months[_date.month - 1]);
    lcd()->print(_date.year);
    lcd()->write(' ');
}

void FrontScreenField::updateTime()
{
    lcd()->setCursor(0, 1);
    bool pm;
    if (_time.hour == 0 || _time.hour == 12) {
        lcd()->write('1');
        lcd()->write('2');
        pm = (_time.hour == 12);
    } else if (_time.hour < 12) {
        lcd()->write('0' + _time.hour / 10);
        lcd()->write('0' + _time.hour % 10);
        pm = false;
    } else {
        int hour = _time.hour - 12;
        lcd()->write('0' + hour / 10);
        lcd()->write('0' + hour % 10);
        pm = true;
    }
    lcd()->write(':');
    lcd()->write('0' + _time.minute / 10);
    lcd()->write('0' + _time.minute % 10);
    lcd()->write(':');
    lcd()->write('0' + _time.second / 10);
    lcd()->write('0' + _time.second % 10);
    lcd()->print(pm ? "pm" : "am");
}

void FrontScreenField::updateVoltage()
{
    lcd()->setCursor(15, 0);
    lcd()->write(_batteryBars);

    lcd()->setCursor(12, 1);
    lcd()->write('0' + _voltageTrunc / 10);
    lcd()->write('.');
    lcd()->write('0' + _voltageTrunc % 10);
    lcd()->write('v');
}

void FrontScreenField::updateAlarm()
{
    lcd()->setCursor(13, 0);
    lcd()->write(_alarmActive ? IND_ALARM_ACTIVE1 : ' ');
    lcd()->write(_alarmActive ? IND_ALARM_ACTIVE2 : ' ');
}

// Create the main form and its fields.
Form mainForm(lcd);
FrontScreenField frontScreen(mainForm);

#define STATUS_LED 13

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
byte alarmActive1[8] = {
    B00100,
    B01001,
    B10010,
    B00000,
    B10010,
    B01001,
    B00100,
    B00000
};
byte alarmActive2[8] = {
    B11000,
    B10100,
    B10011,
    B10011,
    B10011,
    B10100,
    B11000,
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
    lcd.createChar(IND_ALARM_ACTIVE1, alarmActive1);
    lcd.createChar(IND_ALARM_ACTIVE2, alarmActive2);

    //lcd.enableScreenSaver();

    // Show the main form for the first time.
    mainForm.show();
}

void loop() {
    // Update the time and date every second based on the 1 Hz RTC output.
    if (rtc.hasUpdates() || prevHour >= 24) {
        RTCTime time;
        rtc.readTime(&time);
        frontScreen.setTime(time);
        if (time.hour < prevHour) {
            // Time has wrapped around, or date update has been forced.
            RTCDate date;
            rtc.readDate(&date);
            frontScreen.setDate(date);
        }
        prevHour = time.hour;

        // Update the battery status once a second also.
        int status = analogRead(SENSE_BATTERY);
        int voltage = (int)((status * 500L) / 1024L);   // e.g. 2.81V = 281
        voltage += VOLTAGE_DROP_ADJUST;
        if (voltage > 500)
            voltage = 500;
        frontScreen.setVoltage(voltage);
    }

    // Dispatch button events to the main form.
    int event = lcd.getButton();
    if (mainForm.dispatch(event) == FORM_CHANGED) {
        prevHour = 24;      // Force an update of the main screen.
    }

    // If the alarm is on and a button was pressed, then turn off the alarm.
    if (event != LC_BUTTON_NONE && isAlarmOn) {
        // TODO
    }
}
