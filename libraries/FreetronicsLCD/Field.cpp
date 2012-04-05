#include "Field.h"

/**
 * \class Field Field.h <Field.h>
 * \brief Manages a single data input/output field within a Form.
 *
 * \sa Form, BoolField, IntField, TextField, TimeField
 */

/**
 * \brief Constructs a new field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * \sa Form::addField()
 */
Field::Field(const String &label)
    : _label(label)
    , _form(0)
    , next(0)
    , prev(0)
{
}

/**
 * \brief Constructs a new field with a specific \a label and attaches
 * it to a \a form.
 */
Field::Field(Form &form, const String &label)
    : _label(label)
    , _form(0)
    , next(0)
    , prev(0)
{
    form.addField(this);
}

/**
 * \brief Destroys this field and removes it from its owning Form.
 *
 * \sa Form::removeField()
 */
Field::~Field()
{
    if (_form)
        _form->removeField(this);
}

/**
 * \fn Form *Field::form() const
 * \brief Returns the Form that owns this field; null if not associated
 * with a Form.
 */

/**
 * \brief Dispatches \a event via this field.
 *
 * The \a event is usually obtained from FreetronicsLCD::getButton().
 *
 * Returns zero if the \a event has been handled and no further action
 * is required.
 *
 * Returns FORM_CHANGED if the \a event has changed the value of this
 * field in a manner that may require the application to take further
 * action based on the new field value.
 *
 * Returns -1 if the \a event is not handled by this field, and should
 * be handled by the Form itself (particularly for Left and Right buttons).
 * The default implementation returns -1 for all events.
 *
 * \sa Form::dispatch(), FreetronicsLCD::getButton()
 */
int Field::dispatch(int event)
{
    // Nothing to do here.
    return -1;
}

/**
 * \brief Enters the field due to form navigation.
 *
 * This function is typically called when the user presses Left and Right
 * buttons to navigate to the field.  If \a reverse is true, then navigation
 * was due to the Left button being pressed.
 *
 * This function can assume that the display has been cleared and the
 * cursor is positioned at (0, 0).
 *
 * The default implementation prints the label().
 *
 * \sa exitField()
 */
void Field::enterField(bool reverse)
{
    lcd()->print(_label);
}

/**
 * \brief Exits the field due to form navigation.
 *
 * This function is typically called when the user presses Left and Right
 * buttons to navigate from the field.
 *
 * \sa enterField()
 */
void Field::exitField()
{
    // Nothing to do here.
}

/**
 * \fn const String &Field::label() const
 * \brief Returns the label to display in the first line of this field.
 *
 * \sa setLabel()
 */

/**
 * \brief Sets the \a label to display in the first line of this field.
 *
 * \sa label()
 */
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

/**
 * \brief Returns true if this field is the currently-displayed field in
 * its owning form; false otherwise.
 *
 * This function should be called from property setters in subclasses to
 * determine if the screen should be updated when a property is modified.
 */
bool Field::isCurrent() const
{
    if (!_form->isVisible())
        return false;
    return _form->currentField() == this;
}

/**
 * \fn LiquidCrystal *Field::lcd() const
 * \brief Returns the LCD that this field is being drawn on.
 */

/**
 * \brief Updates the cursor position after the label has been drawn
 * by setLabel().
 *
 * The default implementation does nothing.  Subclasses that use an LCD
 * cursor may override this to ensure that the cursor position stays
 * valid after the label is modified.
 *
 * \sa setLabel()
 */
void Field::updateCursor()
{
    // Nothing to do here.
}
