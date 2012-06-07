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

#ifndef IRreceiver_h
#define IRreceiver_h

#include <inttypes.h>
#include "RC5.h"

class IRreceiver
{
public:
    explicit IRreceiver(int interruptNumber = 0);

    static const int AUTO_REPEAT = 128;

    int command();
    int system() const { return _system; }

    int systemFilter() const { return _systemFilter; }
    void setSystemFilter(int system) { _systemFilter = system; }

private:
    int _system;
    int _systemFilter;
    uint8_t pin;
    bool started;
    bool halfChange;    // Value last changed half-way through bit cycle time.
    unsigned long lastChange;
    unsigned bits;
    int8_t bitCount;
    volatile unsigned buffer;
    unsigned lastBuffer;

    void handleInterrupt();

    friend void _IR_receive_interrupt(void);
};

#endif
