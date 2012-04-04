#include "TimeField.h"

#define EDIT_HOUR           0
#define EDIT_MINUTE_TENS    1
#define EDIT_MINUTE         2
#define EDIT_SECOND_TENS    3
#define EDIT_SECOND         4

TimeField::TimeField(const String &label)
    : Field(label)
    , _value(0)
    , _maxHours(24)
    , _printLen(0)
    , _readOnly(false)
    , editField(EDIT_HOUR)
{
}

TimeField::TimeField(Form &form, const String &label)
    : Field(form, label)
    , _value(0)
    , _maxHours(24)
    , _printLen(0)
    , _readOnly(false)
    , editField(EDIT_HOUR)
{
}

TimeField::TimeField(Form &form, const String &label, int maxHours, bool readOnly)
    : Field(form, label)
    , _value(0)
    , _maxHours(maxHours)
    , _printLen(0)
    , _readOnly(readOnly)
    , editField(EDIT_HOUR)
{
}

int TimeField::dispatch(int event)
{
    unsigned long newValue;
    if (_readOnly)
        return -1;
    if (event == LCD_BUTTON_UP) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue += 60 * 60;
        } else if (editField == EDIT_MINUTE_TENS) {
            if (((newValue / 60) % 60) >= 50)
                newValue -= 50 * 60;
            else
                newValue += 10 * 60;
        } else if (editField == EDIT_MINUTE) {
            if (((newValue / 60) % 60) == 59)
                newValue -= 59 * 60;
            else
                newValue += 60;
        } else if (editField == EDIT_SECOND_TENS) {
            if ((newValue % 60) >= 50)
                newValue -= 50;
            else
                newValue += 10;
        } else {
            if ((newValue % 60) == 59)
                newValue -= 59;
            else
                newValue += 1;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            if (newValue < 60 * 60)
                newValue += ((unsigned long)(_maxHours - 1)) * 60 * 60;
            else
                newValue -= 60 * 60;
        } else if (editField == EDIT_MINUTE_TENS) {
            if (((newValue / 60) % 60) < 10)
                newValue += 50 * 60;
            else
                newValue -= 10 * 60;
        } else if (editField == EDIT_MINUTE) {
            if (((newValue / 60) % 60) == 0)
                newValue += 59 * 60;
            else
                newValue -= 60;
        } else if (editField == EDIT_SECOND_TENS) {
            if ((newValue % 60) < 10)
                newValue += 50;
            else
                newValue -= 10;
        } else {
            if ((newValue % 60) == 0)
                newValue += 59;
            else
                newValue -= 1;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_LEFT) {
        if (editField != EDIT_HOUR) {
            --editField;
            printTime();
            return 0;
        }
    } else if (event == LCD_BUTTON_RIGHT) {
        if (editField != EDIT_SECOND) {
            ++editField;
            printTime();
            return 0;
        }
    }
    return -1;
}

void TimeField::enterField(bool reverse)
{
    Field::enterField(reverse);
    if (reverse)
        editField = EDIT_SECOND;
    else
        editField = EDIT_HOUR;
    printTime();
    if (!_readOnly)
        lcd()->cursor();
}

void TimeField::exitField()
{
    if (!_readOnly)
        lcd()->noCursor();
    Field::exitField();
}

void TimeField::setValue(unsigned long value)
{
    unsigned long maxSecs = ((unsigned long)_maxHours) * 60 * 60;
    value %= maxSecs;
    if (value != _value) {
        _value = value;
        if (isCurrent())
            printTime();
    }
}

void TimeField::setReadOnly(bool value)
{
    if (_readOnly != value) {
        _readOnly = value;
        printTime();
        if (isCurrent()) {
            if (value)
                lcd()->cursor();
            else
                lcd()->noCursor();
        }
    }
}

void TimeField::printTime()
{
    lcd()->setCursor(0, 1);
    int col = printField(_value / (60 * 60));
    int hourCol = col - 1;
    lcd()->write(':');
    ++col;
    col += printField((_value / 60) % 60);
    int minuteCol = col - 1;
    lcd()->write(':');
    ++col;
    col += printField(_value % 60);
    int secondCol = col - 1;
    int tempCol = col;
    while (tempCol++ < _printLen)
        lcd()->write(' ');
    _printLen = col;
    if (!_readOnly) {
        if (editField == EDIT_HOUR)
            lcd()->setCursor(hourCol, 1);
        else if (editField == EDIT_MINUTE_TENS)
            lcd()->setCursor(minuteCol - 1, 1);
        else if (editField == EDIT_MINUTE)
            lcd()->setCursor(minuteCol, 1);
        else if (editField == EDIT_SECOND_TENS)
            lcd()->setCursor(secondCol - 1, 1);
        else
            lcd()->setCursor(secondCol, 1);
    }
}

int TimeField::printField(unsigned long value)
{
    if (value < 100) {
        lcd()->write('0' + (int)(value / 10));
        lcd()->write('0' + (int)(value % 10));
        return 2;
    }
    unsigned long divisor = 100;
    while ((value / divisor) >= 10)
        divisor *= 10;
    int digits = 0;
    while (divisor > 0) {
        lcd()->write('0' + (int)((value / divisor) % 10));
        divisor /= 10;
        ++digits;
    }
    return digits;
}
