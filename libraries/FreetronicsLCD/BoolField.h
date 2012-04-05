#ifndef BoolField_h
#define BoolField_h

#include "Field.h"

class BoolField : public Field {
public:
    explicit BoolField(const String &label);
    BoolField(Form &form, const String &label, const String &trueLabel, const String &falseLabel, bool value);

    int dispatch(int event);

    void enterField(bool reverse);

    bool value() const { return _value; }
    void setValue(bool value);

    const String &trueLabel() const { return _trueLabel; }
    void setTrueLabel(const String &trueLabel);

    const String &falseLabel() const { return _falseLabel; }
    void setFalseLabel(const String &falseLabel);

private:
    String _trueLabel;
    String _falseLabel;
    int _printLen;
    bool _value;

    void printValue();
};

#endif
