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

#include "IntField.h"

/**
 * \class IntField IntField.h <IntField.h>
 * \brief Field that manages the input of an integer value.
 *
 * IntField is intended for field values that are modifiable by the user.
 * Pressing Up adds stepValue() to the current value and pressing Down
 * subtracts stepValue() from the current value.  The value is clamped to
 * the range minValue() to maxValue().
 *
 * The following example creates an integer field with the label "Iterations",
 * that ranges between 1 and 5, with a stepValue() of 1, and an initial
 * default value() of 2:
 *
 * \code
 * Form mainForm(lcd);
 * IntField iterField(mainForm, "Iterations", 1, 5, 1, 2);
 * \endcode
 *
 * IntField can be configured to show a suffix() on the screen after the
 * integer value().  This is intended for communicating the units in which
 * the value is expressed.  For example:
 *
 * \code
 * IntField volumeField(mainForm, "Volume", 0, 100, 5, 85, "%");
 * IntField speedField(mainForm, "Speed", 0, 2000, 15, 450, " rpm");
 * \endcode
 *
 * Use TextField for read-only fields that report integer values but
 * which are not modifiable by the user.
 *
 * \sa Field, TextField
 */

/**
 * \brief Constructs a new integer field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * Initially, value() is 0, minValue() is 0, maxValue() is 100,
 * stepValue() is 1, and suffix() is an empty string.
 *
 * \sa Form::addField()
 */
IntField::IntField(const String &label)
    : Field(label)
    , _minValue(0)
    , _maxValue(100)
    , _stepValue(1)
    , _value(0)
    , _printLen(0)
{
}

/**
 * \brief Constructs a new integer field with a specific \a label,
 * \a minValue, \a maxValue, \a stepValue, and \a value, and attaches
 * it to a \a form.
 *
 * The suffix() is initially set to an empty string.
 */
IntField::IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value)
    : Field(form, label)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _stepValue(stepValue)
    , _value(value)
    , _printLen(0)
{
}

/**
 * \brief Constructs a new integer field with a specific \a label,
 * \a minValue, \a maxValue, \a stepValue, \a value, and \a suffix
 * and attaches it to a \a form.
 */
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

/**
 * \fn int IntField::minValue() const
 * \brief Returns the minimum value for the input field.
 *
 * \sa setMinValue(), maxValue(), stepValue(), value()
 */

/**
 * \fn void IntField::setMinValue(int value)
 * \brief Sets the minimum \a value for the input field.
 *
 * The new minimum \a value will be used to clamp the field's value the
 * next time setValue() is called.
 *
 * \sa minValue(), setMaxValue(), setStepValue(), setValue()
 */

/**
 * \fn int IntField::maxValue() const
 * \brief Returns the maximum value for the input field.
 *
 * \sa setMaxValue(), minValue(), stepValue(), value()
 */

/**
 * \fn void IntField::setMaxValue(int value)
 * \brief Sets the maximum \a value for the input field.
 *
 * The new maximum \a value will be used to clamp the field's value the
 * next time setValue() is called.
 *
 * \sa maxValue(), setMinValue(), setStepValue(), setValue()
 */

/**
 * \fn int IntField::stepValue() const
 * \brief Returns the step value to use when increasing or decreasing the
 * value() due to Up and Down button presses.
 *
 * \sa setStepValue(), minValue(), maxValue(), value()
 */

/**
 * \fn void IntField::setStepValue(int value)
 * \brief Sets the step value \a value to use when increasing or decreasing
 * the value() due to Up and Down button presses.
 *
 * \sa stepValue(), setMinValue(), setMaxValue(), setValue()
 */

/**
 * \fn int IntField::value() const
 * \brief Returns the current value of this field.
 *
 * \sa setValue(), minValue(), maxValue(), stepValue()
 */

/**
 * \fn void IntField::setValue(int value)
 * \brief Sets the current \a value of this field.
 *
 * The \a value will be clamped to the range defined by minValue()
 * and maxValue().
 *
 * \sa value(), setMinValue(), setMaxValue(), setStepValue()
 */
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

/**
 * \fn const String &IntField::suffix() const
 * \brief Returns the suffix string to be displayed after the field's value.
 *
 * \sa setSuffix()
 */

/**
 * \brief Sets the \a suffix string to be displayed after the field's value.
 *
 * Suffixes are typically used to indicate the units that the value() is
 * expressed in.  For example:
 *
 * \code
 * field.setSuffix("%");
 * field.setSuffix(" rpm");
 * \endcode
 *
 * \sa suffix()
 */
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
