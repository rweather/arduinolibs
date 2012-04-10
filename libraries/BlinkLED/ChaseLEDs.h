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

#ifndef ChaseLEDs_h
#define ChaseLEDs_h

#include <inttypes.h>

class ChaseLEDs
{
public:
    ChaseLEDs(const uint8_t *pins, int num, unsigned long advanceTime);

    void loop();

    unsigned long advanceTime() const { return _advanceTime; }
    void setAdvanceTime(unsigned long advanceTime) { _advanceTime = advanceTime; }

protected:
    virtual void advance(uint8_t prevPin, uint8_t nextPin);
    uint8_t previousPin(int n) const
        { return _pins[(_currentIndex + _numPins - n) % _numPins]; }

private:
    const uint8_t *_pins;
    int _numPins;
    int _currentIndex;
    unsigned long _advanceTime;
    unsigned long _lastChange;
};

#endif
