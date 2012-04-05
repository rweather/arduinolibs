#include "TimeField.h"

/**
 * \class TimeField TimeField.h <TimeField.h>
 * \brief Field that manages the display and editing of a time value.
 *
 * TimeField is suitable for displaying wall clock time in 24-hour format,
 * or for displaying timeouts and durations in seconds.  Times are specified
 * in seconds as an <tt>unsigned long</tt> value.  They are displayed
 * as <tt>HH:MM:SS</tt>, for hours, minutes, and seconds.
 *
 * The time field can be either read-only or read-write.  When read-write,
 * the Up, Down, Left, and Right buttons can be used to modify the hour,
 * minute, and second components of the time value.
 *
 * The following example displays the number of hours, minutes, and seconds
 * since the device was reset, wrapping around after 24 hours:
 *
 * \code
 * Form mainForm(lcd);
 * TimeField timeField(mainForm, "Time since reset", 24, TIMEFIELD_READ_ONLY);
 *
 * void loop() {
 *     timeField.setValue(millis() / 1000);
 *     mainForm.dispatch(lcd.getButton());
 * }
 * \endcode
 *
 * A read-write field can be used to ask the user for the duration of an
 * application count-down timer:
 *
 * \code
 * TimeField durationField(mainForm, "Timer duration", 24, TIMEFIELD_READ_WRITE);
 * \endcode
 *
 * \sa Field
 */

#define EDIT_HOUR           0
#define EDIT_MINUTE_TENS    1
#define EDIT_MINUTE         2
#define EDIT_SECOND_TENS    3
#define EDIT_SECOND         4

/**
 * \brief Constructs a new time field with a specific \a label.
 *
 * The field is initially not associated with a Form.  The field can be
 * added to a form later using Form::addField().
 *
 * Initially value() is 0, maxHours() is 24, and isReadOnly() is
 * TIMEFIELD_READ_WRITE.
 *
 * \sa Form::addField()
 */
TimeField::TimeField(const String &label)
    : Field(label)
    , _value(0)
    , _maxHours(24)
    , _printLen(0)
    , _readOnly(false)
    , editField(EDIT_HOUR)
{
}

/**
 * \brief Constructs a new boolean field with a specific \a label and
 * attaches it to a \a form.
 *
 * The initial value() of the field will be 0.  The value() will be limited
 * to be less than \a maxHours * 60 * 60 seconds.
 *
 * If \a readOnly is TIMEFIELD_READ_ONLY, then the field will display times
 * but not allow them to be modified by the user.  If \a readOnly is
 * TIMEFIELD_READ_WRITE, then the field will modifiable by the user.
 *
 * \sa value()
 */
TimeField::TimeField(Form &form, const String &label, int maxHours, bool readOnly)
    : Field(form, label)
    , _value(0)
    , _maxHours(maxHours)
    , _printLen(0)
    , _readOnly(readOnly)
    , editField(EDIT_HOUR)
{
}

int TimeField::dispatch(int event)
{
    unsigned long newValue;
    if (_readOnly)
        return -1;
    if (event == LCD_BUTTON_UP) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            newValue += 60 * 60;
        } else if (editField == EDIT_MINUTE_TENS) {
            if (((newValue / 60) % 60) >= 50)
                newValue -= 50 * 60;
            else
                newValue += 10 * 60;
        } else if (editField == EDIT_MINUTE) {
            if (((newValue / 60) % 60) == 59)
                newValue -= 59 * 60;
            else
                newValue += 60;
        } else if (editField == EDIT_SECOND_TENS) {
            if ((newValue % 60) >= 50)
                newValue -= 50;
            else
                newValue += 10;
        } else {
            if ((newValue % 60) == 59)
                newValue -= 59;
            else
                newValue += 1;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_DOWN) {
        newValue = _value;
        if (editField == EDIT_HOUR) {
            if (newValue < 60 * 60)
                newValue += ((unsigned long)(_maxHours - 1)) * 60 * 60;
            else
                newValue -= 60 * 60;
        } else if (editField == EDIT_MINUTE_TENS) {
            if (((newValue / 60) % 60) < 10)
                newValue += 50 * 60;
            else
                newValue -= 10 * 60;
        } else if (editField == EDIT_MINUTE) {
            if (((newValue / 60) % 60) == 0)
                newValue += 59 * 60;
            else
                newValue -= 60;
        } else if (editField == EDIT_SECOND_TENS) {
            if ((newValue % 60) < 10)
                newValue += 50;
            else
                newValue -= 10;
        } else {
            if ((newValue % 60) == 0)
                newValue += 59;
            else
                newValue -= 1;
        }
        setValue(newValue);
        return FORM_CHANGED;
    } else if (event == LCD_BUTTON_LEFT) {
        if (editField != EDIT_HOUR) {
            --editField;
            printTime();
            return 0;
        }
    } else if (event == LCD_BUTTON_RIGHT) {
        if (editField != EDIT_SECOND) {
            ++editField;
            printTime();
            return 0;
        }
    }
    return -1;
}

void TimeField::enterField(bool reverse)
{
    Field::enterField(reverse);
    if (reverse)
        editField = EDIT_SECOND;
    else
        editField = EDIT_HOUR;
    printTime();
    if (!_readOnly)
        lcd()->cursor();
}

void TimeField::exitField()
{
    if (!_readOnly)
        lcd()->noCursor();
    Field::exitField();
}

/**
 * \fn unsigned long TimeField::value() const
 * \brief Returns the current value of this time field, in seconds.
 *
 * \sa setValue()
 */

/**
 * \brief Sets the \a value of this time field, in seconds.
 *
 * If \a value is greater than or equal to maxHours() * 60 * 60,
 * then it will be wrapped around to fall within the valid range.
 *
 * \sa value(), maxHours()
 */
void TimeField::setValue(unsigned long value)
{
    unsigned long maxSecs = ((unsigned long)_maxHours) * 60 * 60;
    value %= maxSecs;
    if (value != _value) {
        _value = value;
        if (isCurrent())
            printTime();
    }
}

/**
 * \fn int TimeField::maxHours() const
 * \brief Returns the maximum number of hours before the field wraps around.
 *
 * \sa setMaxHours(), setValue()
 */

/**
 * \fn void TimeField::setMaxHours(int maxHours)
 * \brief Sets the maximum number of hours before the field wraps around
 * to \a maxHours.
 *
 * \sa maxHours(), setValue()
 */

/**
 * \fn bool TimeField::readOnly() const
 * \brief Returns TIMEFIELD_READ_ONLY (true) or TIMEFIELD_READ_WRITE (false).
 *
 * \sa setReadOnly()
 */

/**
 * \brief Sets the read-only state of this field to \a value.
 *
 * The \a value should be one of TIMEFIELD_READ_ONLY (true) or
 * TIMEFIELD_READ_WRITE (false).  Use of the named constants is recommended.
 *
 * \sa readOnly()
 */
void TimeField::setReadOnly(bool value)
{
    if (_readOnly != value) {
        _readOnly = value;
        printTime();
        if (isCurrent()) {
            if (value)
                lcd()->cursor();
            else
                lcd()->noCursor();
        }
    }
}

void TimeField::printTime()
{
    lcd()->setCursor(0, 1);
    int col = printField(_value / (60 * 60));
    int hourCol = col - 1;
    lcd()->write(':');
    ++col;
    col += printField((_value / 60) % 60);
    int minuteCol = col - 1;
    lcd()->write(':');
    ++col;
    col += printField(_value % 60);
    int secondCol = col - 1;
    int tempCol = col;
    while (tempCol++ < _printLen)
        lcd()->write(' ');
    _printLen = col;
    if (!_readOnly) {
        if (editField == EDIT_HOUR)
            lcd()->setCursor(hourCol, 1);
        else if (editField == EDIT_MINUTE_TENS)
            lcd()->setCursor(minuteCol - 1, 1);
        else if (editField == EDIT_MINUTE)
            lcd()->setCursor(minuteCol, 1);
        else if (editField == EDIT_SECOND_TENS)
            lcd()->setCursor(secondCol - 1, 1);
        else
            lcd()->setCursor(secondCol, 1);
    }
}

int TimeField::printField(unsigned long value)
{
    if (value < 100) {
        lcd()->write('0' + (int)(value / 10));
        lcd()->write('0' + (int)(value % 10));
        return 2;
    }
    unsigned long divisor = 100;
    while ((value / divisor) >= 10)
        divisor *= 10;
    int digits = 0;
    while (divisor > 0) {
        lcd()->write('0' + (int)((value / divisor) % 10));
        divisor /= 10;
        ++digits;
    }
    return digits;
}
