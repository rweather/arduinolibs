#ifndef TextField_h
#define TextField_h

#include "Field.h"

class TextField : public Field {
public:
    explicit TextField(const String &label);
    TextField(Form &form, const String &label);
    TextField(Form &form, const String &label, const String &value);

    void enterField(bool reverse);

    String value() const { return _value; }
    void setValue(const String &value);

private:
    String _value;
};

#endif
