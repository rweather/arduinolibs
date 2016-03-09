/*
 * Copyright (C) 2016 Southern Storm Software, Pty Ltd.
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

#ifndef TERMINAL_h
#define TERMINAL_h

#include <Arduino.h>
#include <USBAPI.h>
#include "USBKeysExtra.h"

// Special key code that indicates that unicodeKey() contains the actual code.
#define KEY_UNICODE 0x1000

// Special key code that indicates that the window size has changed.
#define KEY_WINSIZE 0x1001

class Terminal : public Stream
{
public:
    Terminal();
    virtual ~Terminal();

    enum Mode
    {
        Serial,
        Telnet
    };

    void begin(Stream &stream, Mode mode = Serial);
    void end();

    Terminal::Mode mode() const { return (Terminal::Mode)mod; }

    virtual int available();
    virtual int peek();
    virtual int read();

    virtual void flush();

    virtual size_t write(uint8_t c);
    virtual size_t write(const uint8_t *buffer, size_t size);
    using Stream::write;

    void writeProgMem(const char *str);

    int readKey();

    long unicodeKey() const { return ucode; }

    size_t writeUnicode(long code);

    int columns() const { return ncols; }
    int rows() const { return nrows; }

    void setWindowSize(int columns, int rows);

    void clear();
    void clearToEOL();

    void cursorMove(int x, int y);
    void cursorLeft();
    void cursorRight();
    void cursorUp();
    void cursorDown();

    void backspace();

    void insertLine();
    void insertChar();
    void deleteLine();
    void deleteChar();

    void scrollUp();
    void scrollDown();

    void normal();
    void bold();
    void underline();
    void blink();
    void reverse();

    enum Color
    {
        Black               = 0x00,
        DarkRed             = 0x01,
        DarkGreen           = 0x02,
        DarkYellow          = 0x03,
        DarkBlue            = 0x04,
        DarkMagenta         = 0x05,
        DarkCyan            = 0x06,
        LightGray           = 0x07,
        DarkGray            = 0x08,
        Red                 = 0x09,
        Green               = 0x0A,
        Yellow              = 0x0B,
        Blue                = 0x0C,
        Magenta             = 0x0D,
        Cyan                = 0x0E,
        White               = 0x0F
    };

    void color(Color fg);
    void color(Color fg, Color bg);

    static bool isWideCharacter(long code);

    static size_t utf8Length(long code);
    static size_t utf8Format(uint8_t *buffer, long code);

private:
    Stream *_stream;
    long ucode;
    int ncols, nrows;
    unsigned long timer;
    uint16_t offset;
    uint8_t state;
    uint8_t utf8len;
    uint8_t mod;
    uint8_t sb[16];
    uint8_t flags;

    int matchEscape(int ch);
    void telnetCommand(uint8_t type, uint8_t option);
};

#endif
