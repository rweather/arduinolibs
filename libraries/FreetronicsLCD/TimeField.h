#ifndef TimeField_h
#define TimeField_h

#include "Field.h"

#define TIMEFIELD_READ_ONLY     true
#define TIMEFIELD_READ_WRITE    false

class TimeField : public Field {
public:
    explicit TimeField(const String &label);
    TimeField(Form &form, const String &label, int maxHours, bool readOnly);

    int dispatch(int event);

    void enterField(bool reverse);
    void exitField();

    unsigned long value() const { return _value; }
    void setValue(unsigned long value);

    int maxHours() const { return _maxHours; }
    void setMaxHours(int maxHours) { _maxHours = maxHours; }

    bool readOnly() const { return _readOnly; }
    void setReadOnly(bool value);

private:
    unsigned long _value;
    int _maxHours;
    int _printLen;
    bool _readOnly;
    uint8_t editField;

    void printTime();
    int printField(unsigned long value);
};

#endif
