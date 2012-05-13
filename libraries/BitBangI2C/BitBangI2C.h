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

#ifndef BitBangI2C_h
#define BitBangI2C_h

#include <inttypes.h>

class BitBangI2C {
public:
    BitBangI2C(uint8_t dataPin, uint8_t clockPin);

    typedef bool ack_t;

    static const ack_t ACK = false;
    static const ack_t NACK = true;

    void start();
    void stop();

    ack_t startWrite(unsigned int address);
    ack_t startRead(unsigned int address);

    ack_t write(uint8_t value);
    uint8_t read(ack_t ack = ACK);

private:
    uint8_t _dataPin;
    uint8_t _clockPin;
    bool started;

    void writeBit(bool bit);
    bool readBit();
};

#endif
