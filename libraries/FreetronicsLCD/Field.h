#ifndef Field_h
#define Field_h

#include "Form.h"

class Field {
public:
    explicit Field(const String &label);
    Field(Form &form, const String &label);
    ~Field();

    Form *form() const { return _form; }

    virtual int dispatch(int event);

    virtual void enterField(bool reverse);
    virtual void exitField();

    String label() const { return _label; }
    void setLabel(const String &label);

    bool isCurrent() const;

protected:
    LiquidCrystal *lcd() const { return _form->_lcd; }

    virtual void updateCursor();

private:
    String _label;
    Form *_form;
    Field *next;
    Field *prev;

    friend class Form;
};

#endif
