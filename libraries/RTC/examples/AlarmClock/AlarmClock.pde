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
#include <BoolField.h>
#include <SoftI2C.h>
#include <DS1307RTC.h>
#include <Melody.h>
#include <PowerSave.h>
#include "FrontScreen.h"
#include "EditTime.h"

// I/O pins that are used by this sketch.
#define BUZZER                  12
#define SENSE_BATTERY           A1
#define RTC_DATA                A4
#define RTC_CLOCK               A5
#define RTC_ONE_HZ              A3

// Value to adjust for the voltage drop on D2.
#define VOLTAGE_DROP_ADJUST     70  // 0.7 volts

// Offsets of settings in the realtime clock's NVRAM.
#define SETTING_24HOUR          0   // 0: 12 hour, 1: 24 hour

// Initialize the LCD
FreetronicsLCD lcd;

// Activate the realtime clock chip.
SoftI2C bus(RTC_DATA, RTC_CLOCK);
DS1307RTC rtc(bus, RTC_ONE_HZ);

// Melody to play when the alarm sounds.
int alarmNotes[] = {NOTE_C6, NOTE_C6, NOTE_C6, NOTE_C6, NOTE_REST};
byte alarmLengths[] = {8, 8, 8, 8, 2};
Melody alarmMelody(BUZZER);

uint8_t prevHour = 24;

// Create the main form and its fields.
Form mainForm(lcd);
FrontScreenField frontScreen(mainForm);
BoolField hourMode(mainForm, "Hour display", "24 hour clock", "12 hour clock", false);
EditTime alarm1(mainForm, "Alarm 1");
EditTime alarm2(mainForm, "Alarm 2");
EditTime alarm3(mainForm, "Alarm 3");
EditTime alarm4(mainForm, "Alarm 4");

void setup() {
    // Reduce power consumption on I/O pins we don't need.
    unusedPin(A2);
    unusedPin(0);
    unusedPin(1);
    unusedPin(2);
    unusedPin(10);
    unusedPin(11);
    unusedPin(13);

    // Enable the screen saver.
    lcd.setScreenSaverMode(FreetronicsLCD::BacklightOnSelect);
    lcd.enableScreenSaver(3);

    // Initialize the alarm melody.
    alarmMelody.setMelody(alarmNotes, alarmLengths, sizeof(alarmLengths));
    alarmMelody.setLoopDuration(120000UL);
    //alarmMelody.play();

    // Read the clock settings from the realtime clock's NVRAM.
    hourMode.setValue(rtc.readByte(SETTING_24HOUR) != 0);
    frontScreen.set24HourMode(hourMode.value());
    RTCAlarm alarm;
    rtc.readAlarm(0, &alarm);
    alarm1.setAlarmValue(alarm);
    rtc.readAlarm(1, &alarm);
    alarm2.setAlarmValue(alarm);
    rtc.readAlarm(2, &alarm);
    alarm3.setAlarmValue(alarm);
    rtc.readAlarm(3, &alarm);
    alarm4.setAlarmValue(alarm);

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
        if (hourMode.isCurrent()) {
            frontScreen.set24HourMode(hourMode.value());
            rtc.writeByte(SETTING_24HOUR, (byte)hourMode.value());
        }
        prevHour = 24;      // Force an update of the main screen.
    }

    // If the alarm is playing and a button was pressed, then turn it off.
    if (alarmMelody.isPlaying()) {
        if (event != LCD_BUTTON_NONE)
            alarmMelody.stop();
        alarmMelody.run();
    } else {
        // No alarm playing, so put the device to sleep to save power.
        sleepFor(SLEEP_15_MS);
    }
}
