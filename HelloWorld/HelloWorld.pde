/*
This example demonstrates how to use the FreetronicsLCD library, which extends the
standard LiquidCrystal library to provide support for the Freetronics back light
and Up/Down/Left/Right/Select buttons.  More information on the shield here:

http://www.freetronics.com/pages/16x2-lcd-shield-quickstart-guide
*/

// include the library code:
#include <FreetronicsLCD.h>

// initialize the library
FreetronicsLCD lcd;

// Note: if you are using the USBDroid and have reassigned pin D9 on the LCD shield to some
// other pin (e.g. A1), then you will need to initialize the shield with something like:
// FreetronicsLCD lcd(A1);
// See also: http://www.freetronics.com/pages/combining-the-lcd-keypad-shield-and-the-usbdroid

void setup() {
  // Enable the screen saver, which will automatically blank the screen after 10 seconds.
  // The screen will wake up again when a button is pressed or lcd.display() is called.
  lcd.enableScreenSaver();
  
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis()/1000);

  // print the name of the button that is currently pressed  
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

