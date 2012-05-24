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

#ifndef LCD_h
#define LCD_h

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

class LCD : public LiquidCrystal {
public:
    LCD() : LiquidCrystal(8, 9, 4, 5, 6, 7) { init(); }
    LCD(uint8_t pin9) : LiquidCrystal(8, pin9, 4, 5, 6, 7) { init(); }

    void display();
    void noDisplay();

    enum ScreenSaverMode
    {
        DisplayOff,
        BacklightOff,
        BacklightOnSelect
    };

    ScreenSaverMode screenSaverMode() const { return mode; }
    void setScreenSaverMode(ScreenSaverMode mode);

    void enableScreenSaver(int timeoutSecs = 10);
    void disableScreenSaver();
    bool isScreenSaved() const { return screenSaved; }

    int getButton();

private:
    int prevButton;
    int debounceButton;
    unsigned long timeout;
    unsigned long lastRestore;
    unsigned long lastDebounce;
    bool screenSaved;
    bool eatRelease;
    ScreenSaverMode mode;

    void init();
};

#endif
