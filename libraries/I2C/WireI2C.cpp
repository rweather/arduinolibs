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

#include "WireI2C.h"
#include <WProgram.h>

namespace wireI2C {
// XXX: Apparently Arduino libraries cannot directly include each other,
// so we need to work around that by directly pulling in the twi code.
// We hide it in its own namespace just in case the application uses
// Wire directly.  Take advantage of the include path to find the
// system-installed twi library.
#include "../../../../libraries/Wire/utility/twi.c"
};

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define SDA_PIN 20
#define SCL_PIN 21
#else
#define SDA_PIN A4
#define SCL_PIN A5
#endif

/**
 * \class WireI2C WireI2C.h <WireI2C.h>
 * \brief Implementation of an I2C master using the Arduino two-wire interface.
 *
 * This class implements the I2C master protocol on pre-defined DATA and
 * CLOCK pins (A4 and A5 on most boards, D20 and D21 for Arduino Mega).
 * For other non-standard pins, use the SoftI2C class instead.
 *
 * This implementation only implements the master side of the protocol.
 * Use the standard Arduino Wire library for slave I2C implementations.
 *
 * \sa I2CMaster, SoftI2C
 */

/**
 * \brief Constructs a new I2C bus master using the Arduino two-wire interface.
 *
 * If \a useInternalPullups is true (the default) then internal pullups
 * will be enabled on the DATA and CLOCK pins.  If \a useInternalPullups
 * is false, then the external circuit will need to provide pullup resistors.
 */
WireI2C::WireI2C(bool useInternalPullups)
    : slaveAddr(0xFF)
    , buflen(0)
    , bufposn(0)
{
    pinMode(SDA_PIN, INPUT);
    pinMode(SCL_PIN, INPUT);
    if (useInternalPullups) {
        digitalWrite(SDA_PIN, HIGH);
        digitalWrite(SCL_PIN, HIGH);
    }
    wireI2C::twi_init();
}

unsigned int WireI2C::maxTransferSize() const
{
    return sizeof(buffer);
}

void WireI2C::startWrite(unsigned int address)
{
    slaveAddr = (uint8_t)address;
    buflen = 0;
}

void WireI2C::write(uint8_t value)
{
    if (buflen < sizeof(buffer))
        buffer[buflen++] = value;
}

bool WireI2C::endWrite()
{
    uint8_t result = wireI2C::twi_writeTo(slaveAddr, buffer, buflen, 1);
    slaveAddr = 0xFF;
    return result == 0;
}

bool WireI2C::startRead(unsigned int address, unsigned int count)
{
    if (slaveAddr != 0xFF) {
        // There is a write operation open, so flush it first.
        if (!endWrite())
            return false;
    }
    if (count > sizeof(buffer))
        count = sizeof(buffer);
    bufposn = 0;
    buflen = wireI2C::twi_readFrom((uint8_t)address, buffer, (uint8_t)count);
    return true;
}

unsigned int WireI2C::available()
{
    return buflen - bufposn;
}

uint8_t WireI2C::read()
{
    if (bufposn < buflen)
        return buffer[bufposn++];
    else
        return 0;
}
