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

#include "FreetronicsLCD.h"
#include <WProgram.h>

#define LCD_BACK_LIGHT          3        // LCD backlight is on D3
#define LCD_BUTTON_PIN          A0       // Button state is on A0

#define LCD_BUTTON_VALUE_GAP    10
#define LCD_BUTTON_RIGHT_VALUE  0
#define LCD_BUTTON_UP_VALUE     145
#define LCD_BUTTON_DOWN_VALUE   329
#define LCD_BUTTON_LEFT_VALUE   505
#define LCD_BUTTON_SELECT_VALUE 741

#define DEBOUNCE_DELAY          10      // Delay in ms to debounce buttons

/**
 * \class FreetronicsLCD FreetronicsLCD.h <FreetronicsLCD.h>
 * \brief Enhanced library for Freetronics 16x2 LCD shields
 *
 * This class extends the standard Arduino LiquidCrystal library with
 * extra functionality for the Freetronics 16x2 LCD shield:
 *
 * http://www.freetronics.com/pages/16x2-lcd-shield-quickstart-guide
 *
 * The Freetronics LCD has an additional back light, which is turned
 * on and off with the display() and noDisplay() functions.  The user
 * can also call enableScreenSaver() to cause the display and back light
 * to automatically turn off after a specific timeout.
 * The setScreenSaverMode() function controls which of the display and
 * back light are disabled when the screen saver activates.
 *
 * The Freetronics LCD also has 5 push buttons for Left, Right, Up, Down,
 * and Select, to assist with the creation of interactive sketches.
 * The user can call getButton() to get the current button state.
 * One of the following values may be returned:
 *
 * \li LCD_BUTTON_NONE - No button has been pressed, or a button has been
 * pressed but not yet released.
 * \li LCD_BUTTON_LEFT - Left button was pressed.
 * \li LCD_BUTTON_RIGHT - Right button was pressed.
 * \li LCD_BUTTON_UP - Up button was pressed.
 * \li LCD_BUTTON_DOWN - Down button was pressed.
 * \li LCD_BUTTON_SELECT - Select button was pressed.
 * \li LCD_BUTTON_LEFT_RELEASED - Left button was released.
 * \li LCD_BUTTON_RIGHT_RELEASED - Right button was released.
 * \li LCD_BUTTON_UP_RELEASED - Up button was released.
 * \li LCD_BUTTON_DOWN_RELEASED - Down button was released.
 * \li LCD_BUTTON_SELECT_RELEASED - Select button was released.
 *
 * For convenience, all RELEASED button codes are the negation of their
 * pressed counterparts.  That is, LCD_BUTTON_LEFT_RELEASED == -LCD_BUTTON_LEFT.
 * LCD_BUTTON_NONE is defined to be zero.  Thus, you can check if a
 * generic button has been pressed with <tt>button &gt; 0</tt> and if a
 * generic button has been released with <tt>button &lt; 0</tt>.
 *
 * See the \ref lcd_hello_world "Hello World" example for more
 * information on using the FreetronicsLCD class.
 *
 * \sa Form
 */

/**
 * \fn FreetronicsLCD::FreetronicsLCD()
 * \brief Initialize the Freetronics LCD display with the default
 * pin assignment.
 *
 * The following example shows how to initialize the Freetronics
 * LCD shield:
 *
 * \code
 * FreetronicsLCD lcd;
 * \endcode
 */

/**
 * \fn FreetronicsLCD::FreetronicsLCD(uint8_t pin9)
 * \brief Initialize the Freetronics LCD display for USBDroid.
 *
 * On the USBDroid, the D9 pin is used for USB Host functionality.
 * Either the USB Host's use of D9 must be reassigned to another pin,
 * or the Freetronics LCD shield must be modified.  The following Web
 * page describes the modifications that are necessary:
 * http://www.freetronics.com/pages/combining-the-lcd-keypad-shield-and-the-usbdroid
 *
 * If you choose to modify the LCD shield, then you must use this version
 * of the constructor to initialize the shield, passing the alternative
 * pin as the \a pin9 parameter.  Using the recommended pin from the above
 * Web page of A1, you would initialize the LCD as follows:
 *
 * \code
 * FreetronicsLCD lcd(A1);
 * \endcode
 */

void FreetronicsLCD::init()
{
    // The Freetronics display is 16x2.
    begin(16, 2);

    // Set the LCD back light to be initially on.
    pinMode(LCD_BACK_LIGHT, OUTPUT);
    digitalWrite(LCD_BACK_LIGHT, HIGH);

    // Initialise button input.
    pinMode(LCD_BUTTON_PIN, INPUT);
    digitalWrite(LCD_BUTTON_PIN, LOW);
    prevButton = LCD_BUTTON_NONE;
    debounceButton = LCD_BUTTON_NONE;
    lastDebounce = 0;
    eatRelease = false;

    // Initialize screen saver.
    timeout = 0;
    lastRestore = millis();
    screenSaved = false;
    mode = DisplayOff;
}

/**
 * \brief Turns on the display of text on the LCD and the back light.
 *
 * If the screen saver is active, then calling this function will
 * deactivate the screen saver and reset the timeout.  Thus, this
 * function can be called for force the screen to restore.
 *
 * \sa noDisplay(), enableScreenSaver(), setScreenSaverMode()
 */
void FreetronicsLCD::display()
{
    LiquidCrystal::display();
    if (mode != BacklightOnSelect)
        digitalWrite(LCD_BACK_LIGHT, HIGH);
    else
        digitalWrite(LCD_BACK_LIGHT, LOW);
    screenSaved = false;
    lastRestore = millis();
}

/**
 * \brief Turns off the display of text on the LCD and the back light.
 *
 * This function can be called to force the screen saver to activate.
 *
 * \sa display(), enableScreenSaver(), setScreenSaverMode()
 */
void FreetronicsLCD::noDisplay()
{
    if (mode == DisplayOff)
        LiquidCrystal::noDisplay();
    digitalWrite(LCD_BACK_LIGHT, LOW);
    screenSaved = true;
}

/**
 * \enum FreetronicsLCD::ScreenSaverMode
 * \brief Screen saver mode that controls the display and back light.
 */

/**
 * \var FreetronicsLCD::DisplayOff
 * \brief Turn off both the display and the backlight when the screen saver
 * is activated.
 */

/**
 * \var FreetronicsLCD::BacklightOff
 * \brief Turn off the back light but leave the display on when the screen
 * saver is activated.
 */

/**
 * \var FreetronicsLCD::BacklightOnSelect
 * \brief Same as BacklightOff but the screen saver is only deactivated when
 * Select is pressed; other buttons have no effect.
 */

/**
 * \fn ScreenSaverMode FreetronicsLCD::screenSaverMode() const
 * \brief Returns the current screen saver mode; default is DisplayOff.
 *
 * \sa setScreenSaverMode(), enableScreenSaver()
 */

/**
 * \brief Sets the current screen saver \a mode.
 *
 * \sa screenSaverMode(), enableScreenSaver()
 */
void FreetronicsLCD::setScreenSaverMode(ScreenSaverMode mode)
{
    if (this->mode != mode) {
        this->mode = mode;
        if (screenSaved)
            noDisplay();
        else
            display();
    }
}

/**
 * \brief Enables the screen saver and causes it to activate after
 * \a timeoutSecs of inactivity on the buttons.
 *
 * If \a timeoutSecs is less than or equal to zero, then the call
 * is equivalent to calling disableScreenSaver().
 *
 * For the screen saver to work, the application must regularly call
 * getButton() to fetch the LCD's button state even if no buttons
 * are pressed.
 *
 * If the \a timeoutSecs parameter is not supplied, the default is 10 seconds.
 *
 * \sa disableScreenSaver(), display(), getButton(), isScreenSaved()
 */
void FreetronicsLCD::enableScreenSaver(int timeoutSecs)
{
    if (timeoutSecs < 0)
        timeout = 0;
    else
        timeout = ((unsigned long)timeoutSecs) * 1000;
    display();
}

/**
 * \brief Disables the screen saver.
 *
 * \sa enableScreenSaver(), display(), isScreenSaved()
 */
void FreetronicsLCD::disableScreenSaver()
{
    timeout = 0;
    display();
}

/**
 * \fn bool FreetronicsLCD::isScreenSaved() const
 * \brief Returns true if the screen has been saved; false otherwise.
 *
 * \sa enableScreenSaver()
 */

/**
 * \brief Gets the next button press, release, or idle event.
 *
 * If no buttons are pressed, this function will return LCD_BUTTON_NONE.
 *
 * When a button is pressed, this function will return one of
 * LCD_BUTTON_LEFT, LCD_BUTTON_RIGHT, LCD_BUTTON_UP, LCD_BUTTON_DOWN,
 * or LCD_BUTTON_SELECT.  While the button is pressed, this function
 * will return LCD_BUTTON_NONE until the button is released.  When the
 * button is released, this function will return one of
 * LCD_BUTTON_LEFT_RELEASED, LCD_BUTTON_RIGHT_RELEASED,
 * LCD_BUTTON_UP_RELEAED, LCD_BUTTON_DOWN_RELEASED,
 * or LCD_BUTTON_SELECT_RELEASED.
 *
 * If the screen saver is currently active, then it will be deactivated
 * by this function whenever a button is pressed.  If screenSaverMode() is
 * DisplayOff, the function will "eat" the button press and return
 * LCD_BUTTON_NONE.  The scrren saver can also be deactivated under
 * program control by calling display().
 *
 * This function debounces the button state automatically so there is no
 * need for the caller to worry about spurious button events.
 *
 * \sa enableScreenSaver(), display(), Form::dispatch()
 */
int FreetronicsLCD::getButton()
{
    // Read the currently pressed button.
    int value = analogRead(LCD_BUTTON_PIN);
    int button;
    if (value < (LCD_BUTTON_RIGHT_VALUE + LCD_BUTTON_VALUE_GAP))
        button = LCD_BUTTON_RIGHT;
    else if (value >= (LCD_BUTTON_UP_VALUE - LCD_BUTTON_VALUE_GAP) &&
             value <= (LCD_BUTTON_UP_VALUE + LCD_BUTTON_VALUE_GAP))
        button = LCD_BUTTON_UP;
    else if (value >= (LCD_BUTTON_DOWN_VALUE - LCD_BUTTON_VALUE_GAP) &&
             value <= (LCD_BUTTON_DOWN_VALUE + LCD_BUTTON_VALUE_GAP))
        button = LCD_BUTTON_DOWN;
    else if (value >= (LCD_BUTTON_LEFT_VALUE - LCD_BUTTON_VALUE_GAP) &&
             value <= (LCD_BUTTON_LEFT_VALUE + LCD_BUTTON_VALUE_GAP))
        button = LCD_BUTTON_LEFT;
    else if (value >= (LCD_BUTTON_SELECT_VALUE - LCD_BUTTON_VALUE_GAP) &&
             value <= (LCD_BUTTON_SELECT_VALUE + LCD_BUTTON_VALUE_GAP))
        button = LCD_BUTTON_SELECT;
    else
        button = LCD_BUTTON_NONE;

    // Debounce the button state.
    unsigned long currentTime = millis();
    if (button != debounceButton)
        lastDebounce = currentTime;
    debounceButton = button;
    if ((currentTime - lastDebounce) < DEBOUNCE_DELAY)
        button = prevButton;

    // Process the button event if the state has changed.
    if (prevButton == LCD_BUTTON_NONE && button != LCD_BUTTON_NONE) {
        prevButton = button;
        if (screenSaved) {
            // Button pressed when screen saver active.
            display();
            if (mode == DisplayOff) {
                eatRelease = true;
                return LCD_BUTTON_NONE;
            }
        }
        eatRelease = false;
        lastRestore = currentTime;
        return button;
    } else if (prevButton != LCD_BUTTON_NONE && button == LCD_BUTTON_NONE) {
        button = -prevButton;
        prevButton = LCD_BUTTON_NONE;
        lastRestore = currentTime;
        if (eatRelease) {
            eatRelease = false;
            return LCD_BUTTON_NONE;
        }
        return button;
    } else {
        if (!screenSaved && prevButton == LCD_BUTTON_NONE &&
                timeout != 0 && (currentTime - lastRestore) >= timeout)
            noDisplay();    // Activate screen saver.
        return LCD_BUTTON_NONE;
    }
}
