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

#ifndef EEPROM24_h
#define EEPROM24_h

#include <inttypes.h>
#include <stddef.h>

class I2CMaster;

// Block select modes.
#define EE_BSEL_NONE            0
#define EE_BSEL_8BIT_ADDR       1
#define EE_BSEL_17BIT_ADDR      2
#define EE_BSEL_17BIT_ADDR_ALT  3

// Create an EEPROM descriptor from byte size, page size, and block select mode.
#define _EE24(byteSize, pageSize, mode) \
    (((byteSize) / (pageSize)) | (((unsigned long)(pageSize)) << 16) | \
     (((unsigned long)(mode)) << 28))

// Type descriptors for the 24LCXX range of EEPROM's.
#define EEPROM_24LC00   _EE24(16UL, 1, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC01   _EE24(128UL, 8, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC014  _EE24(128UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC02   _EE24(256UL, 8, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC024  _EE24(256UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC025  _EE24(256UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC04   _EE24(512UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC08   _EE24(1024UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC16   _EE24(2048UL, 16, EE_BSEL_8BIT_ADDR)
#define EEPROM_24LC32   _EE24(4096UL, 32, EE_BSEL_NONE)
#define EEPROM_24LC64   _EE24(8192UL, 32, EE_BSEL_NONE)
#define EEPROM_24LC128  _EE24(16384UL, 32, EE_BSEL_NONE)
#define EEPROM_24LC256  _EE24(32768UL, 64, EE_BSEL_NONE)
#define EEPROM_24LC512  _EE24(65536UL, 128, EE_BSEL_NONE)
#define EEPROM_24LC1025 _EE24(131072UL, 128, EE_BSEL_17BIT_ADDR_ALT)
#define EEPROM_24LC1026 _EE24(131072UL, 128, EE_BSEL_17BIT_ADDR)

class EEPROM24
{
public:
    EEPROM24(I2CMaster &bus, unsigned long type, uint8_t bank = 0);

    unsigned long size() const { return _size; }
    unsigned long pageSize() const { return _pageSize; }

    bool available();

    uint8_t read(unsigned long address);
    size_t read(unsigned long address, void *data, size_t length);

    bool write(unsigned long address, uint8_t value);
    size_t write(unsigned long address, const void *data, size_t length);

private:
    I2CMaster *_bus;
    unsigned long _size;
    unsigned long _pageSize;
    uint8_t _mode;
    uint8_t i2cAddress;

    void writeAddress(unsigned long address);
    bool waitForWrite();
};

#endif
