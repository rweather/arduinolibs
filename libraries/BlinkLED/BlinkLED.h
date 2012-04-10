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

#ifndef BlinkLED_h
#define BlinkLED_h

#include <inttypes.h>

class BlinkLED
{
public:
    BlinkLED(uint8_t pin, unsigned long onTime, unsigned long offTime, bool initialState = false);

    void loop();

    unsigned long onTime() const { return _onTime; }
    unsigned long offTime() const { return _offTime; }
    void setBlinkRate(unsigned long onTime, unsigned long offTime);

    bool state() const { return _state; }
    void setState(bool state);

    void pause() { _paused = true; }
    void resume();
    bool isPaused() const { return _paused; }

private:
    uint8_t _pin;
    bool _state;
    bool _paused;
    unsigned long _onTime;
    unsigned long _offTime;
    unsigned long _lastChange;
};

#endif
