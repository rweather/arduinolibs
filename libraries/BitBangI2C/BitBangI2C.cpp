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

#include "BitBangI2C.h"
#include <WProgram.h>

/**
 * \class BitBangI2C BitBangI2C.h <BitBangI2C.h>
 * \brief Bit-banged implementation of an I2C master.
 *
 * This class implements the I2C master protocol on any arbitrary pair
 * of data and clock pins.  It is not restricted to pre-defined pins as
 * is the case for the standard \c Wire library.
 *
 * This implementation only implements the master side of the protocol.
 * It assumes that there is a single bus master, no arbitration, and
 * no clock stretching.
 */

#define i2cDelay()  delayMicroseconds(5)

/**
 * \brief Constructs a new bit-banged I2C master on \a dataPin and \a clockPin.
 */
BitBangI2C::BitBangI2C(uint8_t dataPin, uint8_t clockPin)
    : _dataPin(dataPin)
    , _clockPin(clockPin)
    , started(false)
{
    // Initially set the CLOCK and DATA lines to be outputs in the high state.
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_clockPin, HIGH);
    digitalWrite(_dataPin, HIGH);
}

/**
 * \typedef BitBangI2C::ack_t
 * \brief Type that represents an I2C ACK or NACK indication.
 */

/**
 * \var BitBangI2C::ACK
 * \brief Indicates that an I2C operation was acknowledged.
 */

/**
 * \var BitBangI2C::NACK
 * \brief Indicates that an I2C operation was not acknowledged.
 */

/**
 * \brief Transmits an I2C start condition on the bus.
 *
 * \sa startRead(), startWrite()
 */
void BitBangI2C::start()
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
}

/**
 * \brief Transmits an I2C stop condition on the bus.
 */
void BitBangI2C::stop()
{
    pinMode(_dataPin, OUTPUT);
    digitalWrite(_dataPin, LOW);
    digitalWrite(_clockPin, HIGH);
    i2cDelay();
    digitalWrite(_dataPin, HIGH);
    i2cDelay();
    started = false;
}

#define I2C_WRITE   0x00
#define I2C_WRITE10 0xF0
#define I2C_READ    0x01
#define I2C_READ10  0xF1

/**
 * \brief Starts a write operation by sending a start condition and the I2C control byte.
 *
 * The \a address must be the 7-bit or 10-bit address of the I2C slave
 * on the bus.
 *
 * Returns BitBangI2C::ACK or BitBangI2C::NACK to indicate whether the
 * read operation was acknowledged by the slave or not.
 *
 * \sa startWrite()
 */
BitBangI2C::ack_t BitBangI2C::startWrite(unsigned int address)
{
    start();
    if (address < 0x80) {
        // 7-bit address.
        return write((uint8_t)((address << 1) | I2C_WRITE));
    } else {
        // 10-bit address.
        if (write((uint8_t)(((address >> 7) & 0x06)) | I2C_WRITE10) == NACK)
            return NACK;
        return write((uint8_t)address);
    }
}

/**
 * \brief Starts a read operation by sending the start condition and the I2C control byte.
 *
 * The \a address must be the 7-bit or 10-bit address of the I2C slave
 * on the bus.
 *
 * Returns BitBangI2C::ACK or BitBangI2C::NACK to indicate whether the
 * read operation was acknowledged by the slave or not.
 *
 * \sa startWrite()
 */
BitBangI2C::ack_t BitBangI2C::startRead(unsigned int address)
{
    start();
    if (address < 0x80) {
        // 7-bit address.
        return write((uint8_t)((address << 1) | I2C_READ));
    } else {
        // 10-bit address.
        if (write((uint8_t)(((address >> 7) & 0x06)) | I2C_READ10) == NACK)
            return NACK;
        return write((uint8_t)address);
    }
}

/**
 * \brief Writes a single byte \a value on the I2C bus.
 *
 * Returns BitBangI2C::ACK or BitBangI2C::NACK to indicate whether the
 * slave acknowledged the byte or not.
 */
BitBangI2C::ack_t BitBangI2C::write(uint8_t value)
{
    uint8_t mask = 0x80;
    while (mask != 0) {
        writeBit((value & mask) != 0);
        mask >>= 1;
    }
    return readBit();
}

/**
 * \brief Reads a single byte from the I2C bus.
 *
 * If \a ack is BitBangI2C::ACK, then the byte is acknowledged.  Otherwise
 * the byte will not be acknowledged.
 */
uint8_t BitBangI2C::read(ack_t ack)
{
    uint8_t value = 0;
    for (uint8_t bit = 0; bit < 8; ++bit)
        value = (value << 1) | readBit();
    writeBit(ack);
    return value;
}

void BitBangI2C::writeBit(bool bit)
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

bool BitBangI2C::readBit()
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
