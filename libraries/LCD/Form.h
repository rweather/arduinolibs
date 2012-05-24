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

#ifndef Form_h
#define Form_h

#include "LCD.h"

class Field;

#define FORM_CHANGED    1

class Form {
public:
    explicit Form(LiquidCrystal &lcd);
    ~Form();

    int dispatch(int event);

    void nextField();
    void prevField();
    void defaultField();

    void addField(Field *field);
    void removeField(Field *field);

    Field *currentField() const { return current; }
    void setCurrentField(Field *field);

    bool isCurrent(Field &field) const { return current == &field; }

    void show();
    void hide();
    bool isVisible() const { return visible; }

private:
    LiquidCrystal *_lcd;
    Field *first;
    Field *last;
    Field *current;
    bool visible;

    friend class Field;
};

#endif
