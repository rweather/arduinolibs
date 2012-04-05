#ifndef IntField_h
#define IntField_h

#include "Field.h"

class IntField : public Field {
public:
    explicit IntField(const String &label);
    IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value);
    IntField(Form &form, const String &label, int minValue, int maxValue, int stepValue, int value, const String &suffix);

    int dispatch(int event);

    void enterField(bool reverse);

    int minValue() const { return _minValue; }
    void setMinValue(int value) { _minValue = value; }

    int maxValue() const { return _maxValue; }
    void setMaxValue(int value) { _maxValue = value; }

    int stepValue() const { return _stepValue; }
    void setStepValue(int value) { _stepValue = value; }

    int value() const { return _value; }
    void setValue(int value);

    const String &suffix() const { return _suffix; }
    void setSuffix(const String &suffix);

private:
    int _minValue;
    int _maxValue;
    int _stepValue;
    int _value;
    int _printLen;
    String _suffix;

    void printValue();
};

#endif
