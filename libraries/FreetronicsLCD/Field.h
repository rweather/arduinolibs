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

#ifndef Field_h
#define Field_h

#include "Form.h"

class Field {
public:
    explicit Field(const String &label);
    Field(Form &form, const String &label);
    ~Field();

    Form *form() const { return _form; }

    virtual int dispatch(int event);

    virtual void enterField(bool reverse);
    virtual void exitField();

    const String &label() const { return _label; }
    void setLabel(const String &label);

    bool isCurrent() const;

protected:
    LiquidCrystal *lcd() const { return _form->_lcd; }

    virtual void updateCursor();

private:
    String _label;
    Form *_form;
    Field *next;
    Field *prev;

    friend class Form;
};

#endif
