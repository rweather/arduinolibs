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

#include "BoolField.h"

/**
 * \class BoolField BoolField.h <BoolField.h>
 * \brief Field that manages the input of a boolean value.
 *
 * BoolField is intended for field values that are modifiable by the user.
 * Pressing one of Up, Down, or Select will toggle the field's current value.
 *
 * The following example creates a boolean field that shows the state
 * of the status LED on D13.  When the LED is on (the default), the string
 * "On" will be displayed on the LCD screen.  When the LED is off, the
 * string "Off" will be displayed instead.
 *
 * \code
 * Form mainForm(lcd);
 * BoolField ledField(mainForm, "Status LED", "On", "Off", true);
 * \endcode
 *
 * \image html FormBool.png
 *
 * To actually toggle the LED, the application's main loop() function
 * should contain the following code:
 *
 * \code
 * int event = lcd.getButton();
 * if (mainForm.dispatch(event) == FORM_CHANGED) {
 *     if (mainForm.isCurrent(ledField)) {
 *         if (ledField.value())
 *             digitalWrite(STATUS_LED, HIGH);
 *         else
 *             digitalWrite(STATUS_LED, LOW);
 *     }
 * }
 * \endcode
 *
 * Use TextField for read-only fields that report boolean values but
 * which are not modifiable by the user.
 *
 * \sa Field, TextField
 */

/**
 * \brief Constructs a new boolean field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * The initial value() will be false.
 *
 * \sa Form::addField()
 */
BoolField::BoolField(const String &label)
    : Field(label)
    , _printLen(0)
    , _value(false)
{
}

/**
 * \brief Constructs a new boolean field with a specific \a label and
 * attaches it to a \a form.
 *
 * The initial value() of the field is set to the parameter \a value.
 * When value() is true, \a trueLabel will be displayed on the screen.
 * When value() is false, \a falseLabel will be displayed on the screen.
 *
 * \sa value()
 */
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

/**
 * \fn bool BoolField::value() const
 * \brief Returns the current value of this field, true or false.
 *
 * \sa setValue()
 */

/**
 * \brief Sets the current value of this field to \a value.
 *
 * \sa value()
 */
void BoolField::setValue(bool value)
{
    if (value != _value) {
        _value = value;
        if (isCurrent())
            printValue();
    }
}

/**
 * \fn const String &BoolField::trueLabel() const
 * \brief Returns the string that is displayed when value() is true.
 *
 * \sa setTrueLabel(), falseLabel()
 */

/**
 * \brief Sets the string that is displayed when value() is true to
 * \a trueLabel.
 *
 * \sa trueLabel(), setFalseLabel()
 */
void BoolField::setTrueLabel(const String &trueLabel)
{
    _trueLabel = trueLabel;
    if (isCurrent())
        printValue();
}

/**
 * \fn const String &BoolField::falseLabel() const
 * \brief Returns the string that is displayed when value() is false.
 *
 * \sa setFalseLabel(), trueLabel()
 */

/**
 * \brief Sets the string that is displayed when value() is false to
 * \a falseLabel.
 *
 * \sa falseLabel(), setTrueLabel()
 */
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
