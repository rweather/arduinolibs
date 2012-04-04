#include "BoolField.h"

BoolField::BoolField(const String &label)
    : Field(label)
    , _printLen(0)
    , _value(false)
{
}

BoolField::BoolField(Form &form, const String &label)
    : Field(form, label)
    , _printLen(0)
    , _value(false)
{
}

BoolField::BoolField(Form &form, const String &label, const String &trueLabel, const String &falseLabel, bool value)
    : Field(form, label)
    , _trueLabel(trueLabel)
    , _falseLabel(falseLabel)
    , _printLen(0)
    , _value(value)
{
}

int BoolField::dispatch(int event)
{
    if (event == LCD_BUTTON_UP || event == LCD_BUTTON_DOWN ||
            event == LCD_BUTTON_SELECT) {
        setValue(!_value);
        return FORM_CHANGED;
    } else {
        return -1;
    }
}

void BoolField::enterField(bool reverse)
{
    Field::enterField(reverse);
    printValue();
}

void BoolField::setValue(bool value)
{
    if (value != _value) {
        _value = value;
        if (isCurrent())
            printValue();
    }
}

void BoolField::setTrueLabel(const String &trueLabel)
{
    _trueLabel = trueLabel;
    if (isCurrent())
        printValue();
}

void BoolField::setFalseLabel(const String &falseLabel)
{
    _falseLabel = falseLabel;
    if (isCurrent())
        printValue();
}

void BoolField::printValue()
{
    unsigned int len;
    lcd()->setCursor(0, 1);
    if (_value) {
        lcd()->print(_trueLabel);
        len = _trueLabel.length();
        while (len++ < _printLen)
            lcd()->write(' ');
        _printLen = _trueLabel.length();
    } else {
        lcd()->print(_falseLabel);
        len = _falseLabel.length();
        while (len++ < _printLen)
            lcd()->write(' ');
        _printLen = _falseLabel.length();
    }
}
