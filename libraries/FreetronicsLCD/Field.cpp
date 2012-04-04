#include "Field.h"

Field::Field(const String &label)
    : _label(label)
    , _form(0)
    , next(0)
    , prev(0)
{
}

Field::Field(Form &form, const String &label)
    : _label(label)
    , _form(0)
    , next(0)
    , prev(0)
{
    form.addField(this);
}

Field::~Field()
{
    if (_form)
        _form->removeField(this);
}

int Field::dispatch(int event)
{
    // Nothing to do here.
    return -1;
}

void Field::enterField(bool reverse)
{
    // Print the label and then position the cursor on the second line.
    // We assume that the screen has just been cleared.
    lcd()->print(_label);
    lcd()->setCursor(0, 1);
}

void Field::exitField()
{
    // Nothing to do here.
}

void Field::setLabel(const String &label)
{
    if (isCurrent()) {
        unsigned int prevLen = _label.length();
        unsigned int newLen = label.length();
        _label = label;
        lcd()->setCursor(0, 0);
        lcd()->print(label);
        while (newLen++ < prevLen)
            lcd()->write(' ');
        updateCursor();
    } else {
        _label = label;
    }
}

bool Field::isCurrent() const
{
    if (!_form->isVisible())
        return false;
    return _form->currentField() == this;
}

void Field::updateCursor()
{
    // Nothing to do here.
}
