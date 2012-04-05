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
    FreetronicsLCD(uint8_t pin9) : LiquidCrystal(8, pin9, 4, 5, 6, 7) { init(); }

    void display();
    void noDisplay();

    void enableScreenSaver(int timeoutSecs = 10);
    void disableScreenSaver();
    bool isScreenSaved() const { return screenSaved; }

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
