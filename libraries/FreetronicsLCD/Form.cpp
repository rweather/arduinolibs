#include "Form.h"
#include "Field.h"

Form::Form(LiquidCrystal &lcd)
    : _lcd(&lcd)
    , first(0)
    , last(0)
    , current(0)
{
}

Form::~Form()
{
    Field *field = first;
    Field *next;
    while (field != 0) {
        next = field->next;
        field->_form = 0;
        field->next = 0;
        field->prev = 0;
        field = next;
    }
}

int Form::dispatch(int event)
{
    if (current) {
        int exitval = current->dispatch(event);
        if (exitval >= 0)
            return exitval;
    }
    if (event == LCD_BUTTON_LEFT)
        prevField();
    else if (event == LCD_BUTTON_RIGHT)
        nextField();
    return 0;
}

void Form::nextField()
{
    Field *field = current;
    if (!field)
        field = first;
    if (field && field->next)
        field = field->next;
    else
        field = first;
    setCurrentField(field);
}

void Form::prevField()
{
    Field *field = current;
    if (!field)
        field = last;
    if (field && field->prev)
        field = field->prev;
    else
        field = last;
    setCurrentField(field);
}

void Form::defaultField()
{
    setCurrentField(first);
}

void Form::addField(Field *field)
{
    if (field->_form)
        return; // Already added to a form.
    field->_form = this;
    field->next = 0;
    field->prev = last;
    if (last)
        last->next = field;
    else
        first = field;
    last = field;
}

void Form::removeField(Field *field)
{
    if (field->_form != this)
        return; // Not a member of this form.
    if (current == field) {
        if (field->next)
            setCurrentField(field->next);
        else if (field->prev)
            setCurrentField(field->prev);
        else
            setCurrentField(0);
    }
    if (field->next)
        field->next->prev = field->prev;
    else
        last = field->prev;
    if (field->prev)
        field->prev->next = field->next;
    else
        first = field->next;
    field->_form = 0;
    field->next = 0;
    field->prev = 0;
}

void Form::setCurrentField(Field *field)
{
    if (field && field->_form != this)
        return;     // Wrong form.
    if (visible) {
        bool reverse = false;
        if (current) {
            current->exitField();
            if (field->next == current)
                reverse = true;
            else if (!field->next && current == first)
                reverse = true;
        }
        current = field;
        _lcd->clear();
        if (current)
            current->enterField(reverse);
    } else {
        current = field;
    }
}

void Form::show()
{
    if (!visible) {
        if (!current)
            current = first;
        visible = true;
        _lcd->clear();
        if (current)
            current->enterField(false);
    }
}

void Form::hide()
{
    if (visible) {
        if (current)
            current->exitField();
        visible = false;
        _lcd->clear();
    }
}
