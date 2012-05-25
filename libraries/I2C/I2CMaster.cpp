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

#include "I2CMaster.h"

/**
 * \class I2CMaster I2CMaster.h <I2CMaster.h>
 * \brief Abstract base class for I2C master implementations.
 *
 * \sa SoftI2C
 */

/**
 * \fn unsigned int I2CMaster::maxTransferSize() const
 * \brief Returns the maximum number of bytes that can be read or written in a single request by this bus master.
 */

/**
 * \fn void I2CMaster::startWrite(unsigned int address)
 * \brief Starts a write operation by sending a start condition and the I2C control byte.
 *
 * The \a address must be the 7-bit or 10-bit address of the I2C slave
 * on the bus.
 *
 * \sa write(), endWrite(), startRead()
 */

/**
 * \fn void I2CMaster::write(uint8_t value)
 * \brief Writes a single byte \a value on the I2C bus.
 *
 * \sa startWrite(), endWrite()
 */

/**
 * \fn bool I2CMaster::endWrite()
 * \brief Ends the current write operation.
 *
 * Returns true if the write operation was acknowledged; false otherwise.
 *
 * \sa startWrite(), write()
 */

/**
 * \fn bool I2CMaster::startRead(unsigned int address, unsigned int count)
 * \brief Starts a read operation for \a count bytes by sending the start condition and the I2C control byte.
 *
 * The \a address must be the 7-bit or 10-bit address of the I2C slave
 * on the bus.
 *
 * Returns true if the read request was acknowledged by the I2C slave
 * or false otherwise.  If true, this function should be followed by
 * \a count calls to read() to fetch the bytes.
 *
 * \sa available(), read(), startWrite()
 */

/**
 * \fn unsigned int I2CMaster::available()
 * \brief Returns the number of bytes that are still available for reading.
 *
 * \sa startRead(), read()
 */

/**
 * \fn uint8_t I2CMaster::read()
 * \brief Reads a single byte from the I2C bus.
 *
 * \sa startRead(), available()
 */
