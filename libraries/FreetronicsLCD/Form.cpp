#include "Form.h"
#include "Field.h"

/**
 * \class Form Form.h <Form.h>
 * \brief Manager for a form containing data input/output fields.
 */

/**
 * \brief Constructs a new form and associates it with \a lcd.
 *
 * This constructor is typically followed by calls to construct Field
 * values for each of the fields on the form.  For example:
 *
 * \code
 * Form mainForm(lcd);
 * TextField welcomeField(mainForm, "Form example", "v1.0");
 * \endcode
 */
Form::Form(LiquidCrystal &lcd)
    : _lcd(&lcd)
    , first(0)
    , last(0)
    , current(0)
{
}

/**
 * \brief Detaches all remaining fields and destroys this form.
 */
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

/**
 * \brief Dispatches \a event to the currently active field using
 * Field::dispatch().
 *
 * The \a event is usually obtained from FreetronicsLCD::getButton().
 *
 * Returns zero if the \a event has been handled and no further action
 * is required.
 *
 * Returns FORM_CHANGED if one of the fields on the form has changed value
 * due to the \a event, perhaps requiring the application to take further
 * action based on the new field value.  Use currentField() or isCurrent()
 * to determine which field has changed.
 *
 * \code
 * int event = lcd.getButton();
 * if (mainForm.dispatch(event) == FORM_CHANGED) {
 *     if (mainForm.isCurrent(volumeField)) {
 *         // Adjust the volume to match the field.
 *         setVolume(volumeField.value());
 *     }
 * }
 * \endcode
 *
 * This function handles the Left and Right buttons to navigate between fields.
 *
 * \sa Field::dispatch(), FreetronicsLCD::getButton(), currentField(), isCurrent()
 */
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

/**
 * \brief Changes to the next field in the "tab order".
 *
 * \sa prevField(), defaultField(), currentField()
 */
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

/**
 * \brief Changes to the previous field in the "tab order".
 *
 * \sa nextField(), defaultField(), currentField()
 */
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

/**
 * \brief Changes to default field (i.e., the first field).
 *
 * \sa nextField(), prevField(), currentField()
 */
void Form::defaultField()
{
    setCurrentField(first);
}

/**
 * \brief Adds \a field to this form.
 *
 * Usually this function is not required because the field's constructor
 * will add the field to the form automatically.
 *
 * \sa removeField()
 */
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

/**
 * \brief Removes \a field from this form.
 *
 * If \a field is the current field on-screen, then either the next or
 * previous field will be made current.
 *
 * \sa addField()
 */
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

/**
 * \fn Field *Form::currentField() const
 * \brief Returns the current field that is displayed on-screen.
 *
 * Returns null if the form has no fields, or setCurrentField() explicitly
 * set the current field to null.
 *
 * \sa setCurrentField(), isCurrent()
 */

/**
 * \brief Sets the current \a field that is displayed on-screen.
 *
 * Use this function to programmatically force the form to display a
 * specific field on-screen.
 *
 * \sa currentField(), isCurrent()
 */
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

/**
 * \fn bool Form::isCurrent(Field &field) const
 * \brief Returns true if \a field is currently displayed on-screen, false otherwise.
 *
 * This function is typically called after dispatch() returns FORM_CHANGED
 * to determine which field has changed.
 *
 * \sa currentField(), setCurrentField()
 */

/**
 * \brief Shows the form, or does nothing if the form is already on-screen.
 *
 * When the form is shown, the screen will be cleared and the currentField()
 * will be drawn.
 *
 * If the form was previously hidden, then the field that was previously
 * current will be shown again.  Call defaultField() before show() to reset
 * the form to show the first field instead.
 *
 * \sa hide(), isVisible(), defaultField()
 */
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

/**
 * \brief Hides the form, or does nothing if the form is not on-screen.
 *
 * The screen will be cleared to remove the contents of the current field.
 *
 * \sa show(), isVisible()
 */
void Form::hide()
{
    if (visible) {
        if (current)
            current->exitField();
        visible = false;
        _lcd->clear();
    }
}

/**
 * \fn bool Form::isVisible() const
 * \brief Returns true if the form is shown; false if the form is hidden.
 *
 * \sa show(), hide()
 */
