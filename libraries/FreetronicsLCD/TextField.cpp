#include "TextField.h"

/**
 * \class TextField TextField.h <TextField.h>
 * \brief Field that displays a read-only text value.
 *
 * This following example displays a text field with the label
 * "Form example" and a value() of "v1.0".
 *
 * \code
 * Form mainForm(lcd);
 * TextField welcomeField(mainForm, "Form example", "v1.0");
 * \endcode
 *
 * As well as static messages, TextField can be used to display read-only
 * information that is computed at runtime:
 *
 * \code
 * TextField timeField(mainForm, "Time since reset", "0");
 *
 * void loop() {
 *     timeField.setValue(millis() / 1000);
 *     mainForm.dispatch(lcd.getButton());
 * }
 * \endcode
 *
 * For writable fields, use BoolField, IntField, or TimeField.
 *
 * \sa Field
 */

/**
 * \brief Constructs a new text field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * The initial value() will be the empty string.
 *
 * \sa Form::addField()
 */
TextField::TextField(const String &label)
    : Field(label)
{
}

/**
 * \brief Constructs a new text field with a specific \a label and \a value
 * attaches it to a \a form.
 *
 * \sa value()
 */
TextField::TextField(Form &form, const String &label, const String &value)
    : Field(form, label)
    , _value(value)
{
}

void TextField::enterField(bool reverse)
{
    Field::enterField(reverse);
    lcd()->setCursor(0, 1);
    lcd()->print(_value);
}

/**
 * \fn const String &TextField::value() const
 * \brief Returns the text value that is currently displayed by this field.
 *
 * \sa setValue()
 */

/**
 * \brief Sets the text \a value that is displayed by this field.
 *
 * \sa value()
 */
void TextField::setValue(const String &value)
{
    if (isCurrent()) {
        unsigned int prevLen = _value.length();
        unsigned int newLen = value.length();
        _value = value;
        lcd()->setCursor(0, 1);
        lcd()->print(value);
        while (newLen++ < prevLen)
            lcd()->write(' ');
    } else {
        _value = value;
    }
}
