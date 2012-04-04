#include "IntField.h"

IntField::IntField(const String &label)
    : Field(label)
    , _minValue(0)
    , _maxValue(100)
    , _stepValue(1)
    , _value(0)
    , _printLen(0)
{
}

IntField::IntField(Form &form, const String &label)
    : Field(form, label)
    , _minValue(0)
    , _maxValue(100)
    , _stepValue(1)
    , _value(0)
    , _printLen(0)
{
}

IntField::IntField(Form &form, const String &label, int minValue, int maxValue, int value)
    : Field(form, label)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _stepValue(1)
    , _value(value)
    , _printLen(0)
{
}

IntField::IntField(Form &form, const String &label, int minValue, int maxValue, int value, const String &suffix)
    : Field(form, label)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _stepValue(1)
    , _value(value)
    , _printLen(0)
    , _suffix(suffix)
{
}

IntField::IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value, const String &suffix)
    : Field(form, label)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _stepValue(stepValue)
    , _value(value)
    , _printLen(0)
    , _suffix(suffix)
{
}

int IntField::dispatch(int event)
{
    if (event == LCD_BUTTON_UP) {
        setValue(_value + _stepValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        setValue(_value - _stepValue);
        return FORM_CHANGED;
    }
    return -1;
}

void IntField::enterField(bool reverse)
{
    Field::enterField(reverse);
    printValue();
}

void IntField::setValue(int value)
{
    if (value < _minValue)
        value = _minValue;
    else if (value > _maxValue)
        value = _maxValue;
    if (value != _value) {
        _value = value;
        if (isCurrent())
            printValue();
    }
}

void IntField::setSuffix(const String &suffix)
{
    _suffix = suffix;
    if (isCurrent())
        printValue();
}

void IntField::printValue()
{
    String str(_value);
    if (_suffix.length())
        str += _suffix;
    lcd()->setCursor(0, 1);
    lcd()->print(str);
    unsigned int len = str.length();
    while (len++ < _printLen)
        lcd()->write(' ');
    _printLen = str.length();
}
