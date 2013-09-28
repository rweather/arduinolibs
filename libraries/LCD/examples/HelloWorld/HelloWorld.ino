/*
This example demonstrates how to use the LCD library, which extends the
standard LiquidCrystal library to provide support for the Freetronics back light
and Up/Down/Left/Right/Select buttons.  More information on the shield here:

http://www.freetronics.com/pages/16x2-lcd-shield-quickstart-guide

This example is placed into the public domain.
*/

#include <LCD.h>
LCD lcd;

// Note: if you are using the USBDroid and have reassigned pin D9 on the LCD shield to some
// other pin (e.g. A1), then you will need to initialize the shield with something like:
// LCD lcd(A1);
// See also: http://www.freetronics.com/pages/combining-the-lcd-keypad-shield-and-the-usbdroid

void setup() {
  lcd.enableScreenSaver();
  lcd.print("hello, world!");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);

  lcd.setCursor(8, 1);
  int button = lcd.getButton();
  if (button == LCD_BUTTON_LEFT)
    lcd.print("LEFT");
  else if (button == LCD_BUTTON_RIGHT)
    lcd.print("RIGHT");
  else if (button == LCD_BUTTON_UP)
    lcd.print("UP");
  else if (button == LCD_BUTTON_DOWN)
    lcd.print("DOWN");
  else if (button == LCD_BUTTON_SELECT)
    lcd.print("SELECT");
  else if (button < 0) // button release
    lcd.print("      ");
}

