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

#ifndef ListField_h
#define ListField_h

#include "Field.h"
#include <avr/pgmspace.h>

typedef PGM_P ListItem;
typedef const PROGMEM ListItem *ListItems;

class ListField : public Field {
public:
    explicit ListField(const String &label);
    ListField(Form &form, const String &label, ListItems items, int value = 0);

    int dispatch(int event);

    void enterField(bool reverse);

    ListItems items() const { return _items; }
    void setItems(ListItems items);

    int value() const { return _value; }
    void setValue(int value);

private:
    ListItems _items;
    int _itemCount;
    int _value;
    int _printLen;

    void printValue();
};

#endif
