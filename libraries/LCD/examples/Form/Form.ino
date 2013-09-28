/*
This example demonstrates how to use the Form and Field classes from the
LCD library to provide a simple UI on the 16x2 LCD display.

This example is placed into the public domain.
*/

// include the library code:
#include <LCD.h>
#include <Form.h>
#include <TextField.h>
#include <TimeField.h>
#include <IntField.h>
#include <BoolField.h>

// Initialize the LCD
LCD lcd;

// Note: if you are using the USBDroid and have reassigned pin D9 on the LCD shield to some
// other pin (e.g. A1), then you will need to initialize the shield with something like:
// LCD lcd(A1);
// See also: http://www.freetronics.com/pages/combining-the-lcd-keypad-shield-and-the-usbdroid

// Create the main form and its fields.
Form mainForm(lcd);
TextField welcomeField(mainForm, "Form example", "v1.0");
TimeField timeField(mainForm, "Time since reset", 24, TIMEFIELD_READ_ONLY);
IntField volumeField(mainForm, "Volume", 0, 100, 5, 85, "%");
BoolField ledField(mainForm, "Status LED", "On", "Off", true);
TimeField durationField(mainForm, "Timer duration", 24, TIMEFIELD_READ_WRITE);

#define STATUS_LED 13

void setup() {
  // Status LED initially on.
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  
  // Enable the screen saver, which will automatically blank the screen after 10 seconds.
  // The screen will wake up again when a button is pressed or lcd.display() is called.
  lcd.enableScreenSaver();
  
  // Show the main form for the first time.
  mainForm.show();
}

void loop() {
  // Update the number of seconds since reset:
  timeField.setValue(millis() / 1000);

  // Dispatch button events to the main form.
  int event = lcd.getButton();
  if (mainForm.dispatch(event) == FORM_CHANGED) {
    if (mainForm.isCurrent(ledField)) {
      if (ledField.value())
        digitalWrite(STATUS_LED, HIGH);
      else
        digitalWrite(STATUS_LED, LOW);
    }
  }
}

