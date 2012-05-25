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

#include "SoftI2C.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class SoftI2C SoftI2C.h <SoftI2C.h>
 * \brief Bit-banged implementation of an I2C master.
 *
 * This class implements the I2C master protocol on any arbitrary pair
 * of data and clock pins.  It is not restricted to pre-defined pins as
 * is the case for the standard Arduino two-wire interface.
 *
 * This implementation only implements the master side of the protocol.
 * It assumes that there is a single bus master, no arbitration, and
 * no clock stretching.
 *
 * \sa I2CMaster
 */

#define i2cDelay()  delayMicroseconds(5)

/**
 * \brief Constructs a new software I2C master on \a dataPin and \a clockPin.
 */
SoftI2C::SoftI2C(uint8_t dataPin, uint8_t clockPin)
    : _dataPin(dataPin)
    , _clockPin(clockPin)
    , started(false)
    , acked(true)
    , inWrite(false)
    , readCount(0)
{
    // Initially set the CLOCK and DATA lines to be outputs in the high state.
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_dataPin, HIGH);
}

unsigned int SoftI2C::maxTransferSize() const
{
    return 0xFFFF;
}

void SoftI2C::start()
{
    pinMode(_dataPin, OUTPUT);
    if (started) {
        // Already started, so send a restart condition.
        digitalWrite(_dataPin, HIGH);
        digitalWrite(_clockPin, HIGH);
        i2cDelay();
    }
    digitalWrite(_dataPin, LOW);
    i2cDelay();
    digitalWrite(_clockPin, LOW);
    i2cDelay();
    started = true;
    acked = true;
}

void SoftI2C::stop()
{
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, HIGH);
    i2cDelay();
    digitalWrite(_dataPin, HIGH);
    i2cDelay();
    started = false;
    inWrite = false;
}

#define I2C_WRITE   0x00
#define I2C_WRITE10 0xF0
#define I2C_READ    0x01
#define I2C_READ10  0xF1

void SoftI2C::startWrite(unsigned int address)
{
    start();
    inWrite = true;
    if (address < 0x80) {
        // 7-bit address.
        write((uint8_t)((address << 1) | I2C_WRITE));
    } else {
        // 10-bit address.
        write((uint8_t)(((address >> 7) & 0x06)) | I2C_WRITE10);
        write((uint8_t)address);
    }
}

void SoftI2C::write(uint8_t value)
{
    uint8_t mask = 0x80;
    while (mask != 0) {
        writeBit((value & mask) != 0);
        mask >>= 1;
    }
    if (readBit())  // 0: ACK, 1: NACK
        acked = false;
}

bool SoftI2C::endWrite()
{
    stop();
    return acked;
}

bool SoftI2C::startRead(unsigned int address, unsigned int count)
{
    start();
    inWrite = false;
    if (address < 0x80) {
        // 7-bit address.
        write((uint8_t)((address << 1) | I2C_READ));
    } else {
        // 10-bit address.
        write((uint8_t)(((address >> 7) & 0x06)) | I2C_READ10);
        write((uint8_t)address);
    }
    if (!acked) {
        readCount = 0;
        return false;
    }
    readCount = count;
    return true;
}

unsigned int SoftI2C::available()
{
    return readCount;
}

uint8_t SoftI2C::read()
{
    uint8_t value = 0;
    for (uint8_t bit = 0; bit < 8; ++bit)
        value = (value << 1) | readBit();
    if (readCount > 1) {
        // More bytes left to read - send an ACK.
        writeBit(false);
        --readCount;
    } else {
        // Last byte - send the NACK and a stop condition.
        writeBit(true);
        stop();
        readCount = 0;
    }
    return value;
}

void SoftI2C::writeBit(bool bit)
{
    pinMode(_dataPin, OUTPUT);
    if (bit)
        digitalWrite(_dataPin, HIGH);
    else
        digitalWrite(_dataPin, LOW);
    i2cDelay();
    digitalWrite(_clockPin, HIGH);
    i2cDelay();
    digitalWrite(_clockPin, LOW);
    i2cDelay();
}

bool SoftI2C::readBit()
{
    pinMode(_dataPin, INPUT);
    digitalWrite(_dataPin, HIGH);
    digitalWrite(_clockPin, HIGH);
    bool bit = digitalRead(_dataPin);
    i2cDelay();
    digitalWrite(_clockPin, LOW);
    i2cDelay();
    return bit;
}
