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
    eatRelease = false;

    // Initialize screen saver.
    timeout = 0;
    lastRestore = millis();
    screenSaved = false;
}

void FreetronicsLCD::display()
{
    LiquidCrystal::display();
    digitalWrite(LCD_BACK_LIGHT, HIGH);

    screenSaved = false;
    lastRestore = millis();
}

void FreetronicsLCD::noDisplay()
{
    LiquidCrystal::noDisplay();
    digitalWrite(LCD_BACK_LIGHT, LOW);
    screenSaved = true;
}

void FreetronicsLCD::enableScreenSaver(int timeoutSecs)
{
    if (timeoutSecs < 0)
        timeout = 0;
    else
        timeout = ((unsigned long)timeoutSecs) * 1000;
    display();
}

void FreetronicsLCD::disableScreenSaver()
{
    timeout = 0;
    display();
}

int FreetronicsLCD::getButton()
{
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
    if (prevButton == LCD_BUTTON_NONE && button != LCD_BUTTON_NONE) {
        prevButton = button;
        if (screenSaved) {
            // Button pressed when screen saver active.
            display();
            eatRelease = true;
            return LCD_BUTTON_NONE;
        }
        eatRelease = false;
        lastRestore = millis();
        return button;
    } else if (prevButton != LCD_BUTTON_NONE && button == LCD_BUTTON_NONE) {
        button = -prevButton;
        prevButton = LCD_BUTTON_NONE;
        lastRestore = millis();
        if (eatRelease) {
            eatRelease = false;
            return LCD_BUTTON_NONE;
        }
        return button;
    } else {
        if (!screenSaved && prevButton == LCD_BUTTON_NONE &&
                timeout != 0 && (millis() - lastRestore) >= timeout)
            noDisplay();    // Activate screen saver.
        return LCD_BUTTON_NONE;
    }
}
