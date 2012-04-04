#ifndef Form_h
#define Form_h

#include "FreetronicsLCD.h"

class Field;

#define FORM_CHANGED    1

class Form {
public:
    explicit Form(LiquidCrystal &lcd);
    ~Form();

    int dispatch(int event);

    void nextField();
    void prevField();
    void defaultField();

    void addField(Field *field);
    void removeField(Field *field);

    Field *currentField() const { return current; }
    void setCurrentField(Field *field);

    bool isCurrent(Field &field) const { return current == &field; }

    void show();
    void hide();
    bool isVisible() const { return visible; }

private:
    LiquidCrystal *_lcd;
    Field *first;
    Field *last;
    Field *current;
    bool visible;

    friend class Field;
};

#endif
