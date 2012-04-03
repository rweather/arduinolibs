#ifndef FreetronicsLCD_h
#define FreetronicsLCD_h

// Extended version of the LiquidCrystal library that works specifically
// with Freetronics' 16x2 LCD display, including support for the back
// light and the Up/Down/Left/Right/Select buttons.  More info:
//
// http://www.freetronics.com/pages/16x2-lcd-shield-quickstart-guide

// Include a copy of the standard LiquidCrystal library so we can extend it.
#include "utility/LiquidCrystal.h"

// Button event codes.
#define LCD_BUTTON_NONE             0
#define LCD_BUTTON_LEFT             1
#define LCD_BUTTON_RIGHT            2
#define LCD_BUTTON_UP               3
#define LCD_BUTTON_DOWN             4
#define LCD_BUTTON_SELECT           5
#define LCD_BUTTON_LEFT_RELEASED    -1
#define LCD_BUTTON_RIGHT_RELEASED   -2
#define LCD_BUTTON_UP_RELEASED      -3
#define LCD_BUTTON_DOWN_RELEASED    -4
#define LCD_BUTTON_SELECT_RELEASED  -5

class FreetronicsLCD : public LiquidCrystal {
public:
    FreetronicsLCD() : LiquidCrystal(8, 9, 4, 5, 6, 7) { init(); }

    // For USBDroid where D9 needs to be reassigned to some other pin.
    FreetronicsLCD(uint8_t pin9) : LiquidCrystal(8, pin9, 4, 5, 6, 7) { init(); }

    // Turn on the display, including the back light and the text.
    // This will also reset the screen saver timeout.
    void display();

    // Turn off the display, including the back light and the text,
    // effectively forcing the screen saver to activate immediately.
    void noDisplay();

    // Enable the screen saver and activate it after timeout seconds
    // of inactivity on the buttons.  Note: the screen saver is activated
    // during the call to getButton() so it must be polled regularly.
    void enableScreenSaver(int timeoutSecs = 10);

    // Disable the screen saver and turn the screen back on.
    void disableScreenSaver();

    // Determine if the screen saver is active.
    bool isScreenSaved() const { return screenSaved; }

    // Get the next button event, or LCD_BUTTON_NONE if no change
    // since the last event.  If the screen saver is active, then
    // pressing a button will disable the screen saver and return
    // LCD_BUTTON_NONE as the button code.  Thus, any button can
    // be used to wake up the screen.
    int getButton();

private:
    int prevButton;
    unsigned long timeout;
    unsigned long lastRestore;
    bool screenSaved;
    bool eatRelease;

    void init();
};

#endif
