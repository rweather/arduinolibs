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

#ifndef IntField_h
#define IntField_h

#include "Field.h"

class IntField : public Field {
public:
    explicit IntField(const String &label);
    IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value);
    IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value, const String &suffix);

    int dispatch(int event);

    void enterField(bool reverse);

    int minValue() const { return _minValue; }
    void setMinValue(int value) { _minValue = value; }

    int maxValue() const { return _maxValue; }
    void setMaxValue(int value) { _maxValue = value; }

    int stepValue() const { return _stepValue; }
    void setStepValue(int value) { _stepValue = value; }

    int value() const { return _value; }
    void setValue(int value);

    const String &suffix() const { return _suffix; }
    void setSuffix(const String &suffix);

private:
    int _minValue;
    int _maxValue;
    int _stepValue;
    int _value;
    int _printLen;
    String _suffix;

    void printValue();
};

#endif
