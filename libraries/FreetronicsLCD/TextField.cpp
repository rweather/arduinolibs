#include "TextField.h"

TextField::TextField(const String &label)
    : Field(label)
{
}

TextField::TextField(Form &form, const String &label)
    : Field(form, label)
{
}

TextField::TextField(Form &form, const String &label, const String &value)
    : Field(form, label)
    , _value(value)
{
}

void TextField::enterField(bool reverse)
{
    Field::enterField(reverse);
    lcd()->print(_value);
}

void TextField::setValue(const String &value)
{
    if (isCurrent()) {
        unsigned int prevLen = _value.length();
        unsigned int newLen = value.length();
        _value = value;
        lcd()->setCursor(0, 1);
        lcd()->print(value);
        while (newLen++ < prevLen)
            lcd()->write(' ');
    } else {
        _value = value;
    }
}
