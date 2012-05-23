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

#include "ListField.h"
#include <string.h>

/**
 * \class ListField ListField.h <ListField.h>
 * \brief Field that manages selection from a static list of items.
 *
 * ListField is intended for selecting an element from a list of items.
 * Each items is represented by a string within program memory, with the
 * list terminated by null.  For example:
 *
 * \code
 * const char item_Eggs[] PROGMEM = "Eggs";
 * const char item_Cheese[] PROGMEM = "Cheese";
 * const char item_Pumpkin[] PROGMEM = "Pumpkin";
 * ListItem const ingredients[] PROGMEM = {
 *    item_Eggs,
 *    item_Cheese,
 *    item_Pumpkin,
 *    0
 * };
 *
 * Form mainForm(lcd);
 * ListField ingredient(mainForm, "Select ingredient", ingredients);
 * \endcode
 *
 * If there are only two items in the list, then BoolField can be used instead.
 *
 * \sa Field, BoolField
 */

/**
 * \brief Constructs a new list field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * Initially, items() is null and value() is -1.
 *
 * \sa Form::addField()
 */
ListField::ListField(const String &label)
    : Field(label)
    , _items(0)
    , _itemCount(0)
    , _value(-1)
    , _printLen(0)
{
}

/**
 * \brief Constructs a new list field with a specific \a label,
 * list of \a items, and \a value, and attaches it to a \a form.
 */
ListField::ListField(Form &form, const String &label, ListItems items, int value)
    : Field(form, label)
    , _items(0)
    , _itemCount(0)
    , _value(value)
    , _printLen(0)
{
    setItems(items);
}

int ListField::dispatch(int event)
{
    if (event == LCD_BUTTON_DOWN) {
        if (_value >= (_itemCount - 1))
            setValue(0);
        else
            setValue(_value + 1);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_UP) {
        if (_value <= 0)
            setValue(_itemCount - 1);
        else
            setValue(_value - 1);
        return FORM_CHANGED;
    }
    return -1;
}

void ListField::enterField(bool reverse)
{
    Field::enterField(reverse);
    _printLen = 0;
    printValue();
}

/**
 * \fn ListItems ListField::items() const
 * \brief Returns the array of items in this list.
 *
 * \sa setItems()
 */

/**
 * \brief Sets the array of \a items for this list.
 *
 * The \a items must be stored within program memory and terminated by null;
 * for example:
 *
 * \code
 * const char item_Eggs[] PROGMEM = "Eggs";
 * const char item_Cheese[] PROGMEM = "Cheese";
 * const char item_Pumpkin[] PROGMEM = "Pumpkin";
 * ListItem const ingredients[] PROGMEM = {
 *    item_Eggs,
 *    item_Cheese,
 *    item_Pumpkin,
 *    0
 * };
 *
 * list.setItems(ingredients);
 * \endcode
 *
 * \sa items()
 */
void ListField::setItems(ListItems items)
{
    _items = items;
    _itemCount = 0;
    if (items) {
        for (;;) {
            ListItem item = (ListItem)pgm_read_word(items);
            if (!item)
                break;
            ++items;
            ++_itemCount;
        }
    }
    if (_value >= _itemCount)
        _value = _itemCount - 1;
    if (isCurrent())
        printValue();
}

/**
 * \fn int ListField::value() const
 * \brief Returns the value of this list; i.e. the index within items() of the
 * selected item.
 *
 * Returns -1 if the items() array is empty or null.
 *
 * \sa setValue(), items()
 */

/**
 * \brief Sets the \a value of this list; i.e. the index within items() of the
 * selected item.
 *
 * The \a value will be clamped to the range of items().
 *
 * \sa value(), items()
 */
void ListField::setValue(int value)
{
    if (_value != value) {
        _value = value;
        if (_value < 0)
            _value = 0;
        if (_value >= _itemCount)
            _value = _itemCount - 1;
        if (isCurrent())
            printValue();
    }
}

void ListField::printValue()
{
    lcd()->setCursor(0, 1);
    int len = 0;
    if (_value >= 0) {
        ListItem str = (ListItem)pgm_read_word(&(_items[_value]));
        char ch;
        while ((ch = pgm_read_byte(str)) != 0) {
            lcd()->write(ch);
            ++len;
            ++str;
        }
    }
    while (_printLen-- > len)
        lcd()->write(' ');
    _printLen = len;
}
