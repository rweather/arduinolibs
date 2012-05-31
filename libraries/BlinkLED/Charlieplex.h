/*
 * Copyright (C) 2012 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef Charlieplex_h
#define Charlieplex_h

#include <inttypes.h>

class Charlieplex
{
public:
    Charlieplex(const uint8_t *pins, uint8_t numPins);
    ~Charlieplex();

    int count() const { return _count; }

    bool led(int index) const { return _values[index] != 0; }
    void setLed(int index, bool value) { _values[index] = (value ? 255 : 0); }

    uint8_t pwmLed(int index) const { return _values[index]; }
    void setPwmLed(int index, uint8_t value) { _values[index] = value; }

    unsigned long holdTime() const { return _holdTime; }
    void setHoldTime(unsigned long us) { _holdTime = us; }

    void loop();
    void refresh();

private:
    int _count;
    uint8_t *_pins1;
    uint8_t *_pins2;
    uint8_t *_values;
    unsigned long _holdTime;
    unsigned long _lastTime;
    int _currentIndex;
    uint8_t _pwmPhase;
};

#endif
