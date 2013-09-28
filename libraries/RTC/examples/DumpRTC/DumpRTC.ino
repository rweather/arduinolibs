/*
This example demonstrates how to read time, date, and other information
from the DS3232 realtime clock chip.

This example is placed into the public domain.
*/

#include <SoftI2C.h>
#include <DS3232RTC.h>

SoftI2C i2c(A4, A5);
DS3232RTC rtc(i2c);

const char *days[] = {
    "Mon, ", "Tue, ", "Wed, ", "Thu, ", "Fri, ", "Sat, ", "Sun, "
};

const char *months[] = {
    " Jan ", " Feb ", " Mar ", " Apr ", " May ", " Jun ",
    " Jul ", " Aug ", " Sep ", " Oct ", " Nov ", " Dec "
};

void setup() {
    Serial.begin(9600);

    RTCAlarm alarm;
    for (byte alarmNum = 0; alarmNum < RTC::ALARM_COUNT; ++alarmNum) {
        rtc.readAlarm(alarmNum, &alarm);
        Serial.print("Alarm ");
        Serial.print(alarmNum + 1, DEC);
        Serial.print(": ");
        if (alarm.flags & 0x01) {
            printDec2(alarm.hour);
            Serial.print(':');
            printDec2(alarm.minute);
            Serial.println();
        } else {
            Serial.println("Off");
        }
    }
    Serial.println();
}

void loop() {
    RTCTime time;
    RTCDate date;
    rtc.readTime(&time);
    rtc.readDate(&date);

    Serial.print("Time: ");
    printDec2(time.hour);
    Serial.print(':');
    printDec2(time.minute);
    Serial.print(':');
    printDec2(time.second);
    Serial.println();

    Serial.print("Date: ");
    Serial.print(days[RTC::dayOfWeek(&date) - 1]);
    Serial.print(date.day, DEC);
    Serial.print(months[date.month - 1]);
    Serial.print(date.year);
    Serial.println();

    Serial.print("Temp: ");
    int temp = rtc.readTemperature();
    if (temp != RTC::NO_TEMPERATURE) {
        Serial.print(temp / 4.0);
        Serial.println(" celcius");
    } else {
        Serial.println("not available");
    }

    Serial.println();

    delay(1000);
}

void printDec2(int value)
{
    Serial.print((char)('0' + (value / 10)));
    Serial.print((char)('0' + (value % 10)));
}
