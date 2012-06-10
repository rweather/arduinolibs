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

#include "EEPROM24.h"
#include "I2CMaster.h"

/**
 * \class EEPROM24 EEPROM24.h <EEPROM24.h>
 * \brief Reading and writing EEPROM's from the 24LCXX family.
 *
 * The 24LCXX family of EEPROM's provide a variety of memory sizes from
 * 16 bytes up to 128 kBytes that can be accessed via the I2C protocol.
 * These chips can be used to augment the 1 kByte or so of builtin EEPROM
 * memory that is typical on Arduino boards.  The EEPROM should be wired
 * to an Arduino Uno as follows:
 *
 * \image html eeprom_circuit.png
 *
 * Access to a 24LCXX chip is initialized as follows:
 *
 * \code
 * SoftI2C i2c(A4, A5);
 * EEPROM24 eeprom(i2c, EEPROM_24LC256);
 * \endcode
 *
 * Once initialized, read() and write() can be used to manipulate the
 * contents of the EEPROM's memory.
 *
 * The following EEPROM types are supported by this class:
 *
 * <table>
 * <tr><td>Chip</td><td>Type</td><td>Size</td></tr>
 * <tr><td>24lc00</td><td>\c EEPROM_24LC00</td><td>16 bytes</td></tr>
 * <tr><td>24lc01</td><td>\c EEPROM_24LC01</td><td>128 bytes</td></tr>
 * <tr><td>24lc014</td><td>\c EEPROM_24LC014</td><td>128 bytes</td></tr>
 * <tr><td>24lc02</td><td>\c EEPROM_24LC02</td><td>256 bytes</td></tr>
 * <tr><td>24lc024</td><td>\c EEPROM_24LC024</td><td>256 bytes</td></tr>
 * <tr><td>24lc025</td><td>\c EEPROM_24LC025</td><td>256 bytes</td></tr>
 * <tr><td>24lc04</td><td>\c EEPROM_24LC04</td><td>512 bytes</td></tr>
 * <tr><td>24lc08</td><td>\c EEPROM_24LC08</td><td>1 kByte</td></tr>
 * <tr><td>24lc16</td><td>\c EEPROM_24LC16</td><td>2 kBytes</td></tr>
 * <tr><td>24lc32</td><td>\c EEPROM_24LC32</td><td>4 kBytes</td></tr>
 * <tr><td>24lc64</td><td>\c EEPROM_24LC64</td><td>8 kBytes</td></tr>
 * <tr><td>24lc128</td><td>\c EEPROM_24LC128</td><td>16 kBytes</td></tr>
 * <tr><td>24lc256</td><td>\c EEPROM_24LC256</td><td>32 kBytes</td></tr>
 * <tr><td>24lc512</td><td>\c EEPROM_24LC512</td><td>64 kBytes</td></tr>
 * <tr><td>24lc1025</td><td>\c EEPROM_24LC1025</td><td>128 kBytes</td></tr>
 * <tr><td>24lc1026</td><td>\c EEPROM_24LC1026</td><td>128 kBytes</td></tr>
 * </table>
 *
 * There can be multiple 24LCXX chips on the same I2C bus, as long as their
 * A0, A1, and A2 address pins are set to different values.  For example,
 * two 24LC256 chips can be used to provide the same memory capacity as a
 * single 24LC512 chip.  The optional <i>bank</i> parameter to the constructor
 * is used to assign different bank addresses to each chip:
 *
 * \code
 * SoftI2C i2c(A4, A5);
 * EEPROM24 eeprom0(i2c, EEPROM_24LC256, 0);
 * EEPROM24 eeprom1(i2c, EEPROM_24LC256, 1);
 * \endcode
 *
 * \sa I2CMaster
 */

/**
 * \brief Constructs a new EEPROM access object on \a bus for an EEPROM
 * of the specified \a type.
 *
 * The \a bank can be used to choose between multiple EEPROM's on
 * \a bus of the specified \a type.  The \a bank corresponds to the value
 * that is set on the EEPROM's A0, A1, and A2 address pins.  Note that
 * some EEPROM's have less than 3 address pins; consult the datasheet
 * for more information.
 */
EEPROM24::EEPROM24(I2CMaster &bus, unsigned long type, uint8_t bank)
    : _bus(&bus)
    , _size((type & 0xFFFF) * ((type >> 16) & 0x0FFF))
    , _pageSize((type >> 16) & 0x0FFF)
    , _mode((uint8_t)((type >> 28) & 0x0F))
    , i2cAddress(0x50)
{
    // Adjust the I2C address for the memory bank of the chip.
    switch (_mode) {
    case EE_BSEL_NONE:
        i2cAddress += (bank & 0x07);
        break;
    case EE_BSEL_8BIT_ADDR: {
        uint8_t addrBits = 8;
        unsigned long size = 0x0100;
        while (size < _size) {
            ++addrBits;
            size <<= 1;
        }
        if (addrBits < 11)
            i2cAddress += ((bank << (addrBits - 8)) & 0x07);
        break; }
    case EE_BSEL_17BIT_ADDR:
        i2cAddress += ((bank << 1) & 0x06);
        break;
    case EE_BSEL_17BIT_ADDR_ALT:
        i2cAddress += bank & 0x03;
        break;
    }
}

/**
 * \fn unsigned long EEPROM24::size() const
 * \brief Returns the size of the EEPROM in bytes.
 *
 * \sa pageSize()
 */

/**
 * \fn unsigned long EEPROM24::pageSize() const
 * \brief Returns the size of a single EEPROM page in bytes.
 *
 * Writes that are a multiple of the page size and aligned on a page
 * boundary will typically be more efficient than non-aligned writes.
 *
 * \sa size()
 */

/**
 * \brief Returns true if the EEPROM is available on the I2C bus;
 * false otherwise.
 *
 * This function can be used to probe the I2C bus to determine if the
 * EEPROM is present or not.
 *
 * \sa read(), write()
 */
bool EEPROM24::available()
{
    // Perform a "Current Address Read" on the EEPROM.  We don't care about
    // the returned byte.  We only care if the read request was ACK'ed or not.
    if (!_bus->startRead(i2cAddress, 1))
        return false;
    _bus->read();
    return true;
}

/**
 * \brief Reads a single byte from the EEPROM at \a address.
 *
 * \sa write()
 */
uint8_t EEPROM24::read(unsigned long address)
{
    if (address >= _size)
        return 0;
    writeAddress(address);
    if (!_bus->startRead(i2cAddress, 1))
        return 0;
    return _bus->read();
}

/**
 * \brief Reads a block of \a length bytes from the EEPROM at \a address
 * into the specified \a data buffer.
 *
 * Returns the number of bytes that were read, which may be short if
 * \a address + \a length is greater than size() or the EEPROM is
 * not available on the I2C bus.
 *
 * \sa write(), available()
 */
size_t EEPROM24::read(unsigned long address, void *data, size_t length)
{
    if (address >= _size || !length)
        return 0;
    if ((address + length) > _size)
        length = (size_t)(_size - address);
    writeAddress(address);
    if (!_bus->startRead(i2cAddress, length))
        return 0;
    uint8_t *d = (uint8_t *)data;
    unsigned int count = 0;
    while (_bus->available()) {
        *d++ = _bus->read();
        ++count;
    }
    return count;
}

/**
 * \brief Writes a byte \a value to \a address in the EEPROM.
 *
 * Returns true if the byte was written successfully, or false if
 * \a address is out of range or the EEPROM is not available on the I2C bus.
 *
 * \sa read(), available()
 */
bool EEPROM24::write(unsigned long address, uint8_t value)
{
    if (address >= _size)
        return false;
    writeAddress(address);
    _bus->write(value);
    return waitForWrite();
}

/**
 * \brief Writes \a length bytes from a \a data buffer to \a address
 * in the EEPROM.
 *
 * Returns the number of bytes that were written, which may be short if
 * \a address + \a length is greater than size() or the EEPROM is not
 * available on the I2C bus.
 *
 * Best performance will be achieved if \a address and \a length are a
 * multiple of pageSize().
 *
 * \sa read(), available(), pageSize()
 */
size_t EEPROM24::write(unsigned long address, const void *data, size_t length)
{
    if (address >= _size)
        return 0;
    if ((address + length) > _size)
        length = (size_t)(_size - address);
    bool needAddress = true;
    size_t result = 0;
    size_t page = 0;
    const uint8_t *d = (const uint8_t *)data;
    while (length > 0) {
        if (needAddress) {
            writeAddress(address);
            needAddress = false;
        }
        _bus->write(*d++);
        ++address;
        ++page;
        if ((address & (_pageSize - 1)) == 0) {
            // At the end of a page, so perform a flush.
            if (!waitForWrite())
                return result;  // Could not write this page.
            needAddress = true;
            result += page;
            page = 0;
        }
        --length;
    }
    if (!needAddress) {
        if (!waitForWrite())
            return result;  // Could not write the final page.
    }
    return result + page;
}

void EEPROM24::writeAddress(unsigned long address)
{
    switch (_mode) {
    case EE_BSEL_NONE:
        _bus->startWrite(i2cAddress);
        _bus->write((uint8_t)(address >> 8));
        _bus->write((uint8_t)address);
        break;
    case EE_BSEL_8BIT_ADDR:
        _bus->startWrite(i2cAddress | (((uint8_t)(address >> 8)) & 0x07));
        _bus->write((uint8_t)address);
        break;
    case EE_BSEL_17BIT_ADDR:
        _bus->startWrite(i2cAddress | (((uint8_t)(address >> 16)) & 0x01));
        _bus->write((uint8_t)(address >> 8));
        _bus->write((uint8_t)address);
        break;
    case EE_BSEL_17BIT_ADDR_ALT:
        _bus->startWrite(i2cAddress | (((uint8_t)(address >> 14)) & 0x04));
        _bus->write((uint8_t)(address >> 8));
        _bus->write((uint8_t)address);
        break;
    }
}

bool EEPROM24::waitForWrite()
{
    // 1000 iterations is going to be approximately 100ms when the I2C
    // clock is 100 kHz.  If there has been no response in that time
    // then we assume that the write has failed and timeout.
    if (!_bus->endWrite())
        return false;
    unsigned count = 1000;
    while (count > 0) {
        _bus->startWrite(i2cAddress);
        if (_bus->endWrite())
            return true;
        --count;
    }
    return false;
}
