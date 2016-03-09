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

#include "Terminal.h"
#include "TelnetDefs.h"

/**
 * \class Terminal Terminal.h <Terminal.h>
 * \brief Extended stream interface for terminal operations.
 *
 * This class extends the standard Arduino Stream class with functions that
 * are suitable for interfacing to VT100 terminal applications like PuTTY.
 *
 * The following example initializes a terminal running on the primary
 * Arduino serial port:
 *
 * \code
 * Terminal term;
 * void setup() {
 *     Serial.begin(9600);
 *     term.begin(Serial);
 * }
 * \endcode
 *
 * The readKey() function reads input from the underlying stream, decodes
 * any VT100 key escape sequences that it finds, and reports them to the
 * application using USB, ASCII, or Unicode key codes.  This is typically
 * used in the application's main loop as follows:
 *
 * \code
 * void loop() {
 *     int key = term.readKey();
 *     switch (key) {
 *     case -1:     break;      // No key available.
 *
 *     case KEY_LEFT_ARROW:
 *         // Left arrow key has been pressed.
 *         ...
 *         break;
 *
 *     case KEY_RIGHT_ARROW:
 *         // Right arrow key has been pressed.
 *         ...
 *         break;
 *
 *     case KEY_ESC:
 *         // Escape key has been pressed.
 *         ...
 *         break;
 *
 *     default:
 *         if (key >= 0x20 && key <= 7E) {
 *             // Visible ASCII character has been typed.
 *             ...
 *         }
 *         break;
 *     }
 * }
 * \endcode
 *
 * This class understands extended characters in the UTF-8 encoding,
 * allowing the full Unicode character set to be used in applications.
 * Extended Unicode characters are reported by readKey() with the key code
 * KEY_UNICODE, with the actual code point returned via unicodeKey().
 *
 * On the output side, UTF-8 strings can be written to the terminal
 * using write(), or writeUnicode() can be used to write a single
 * Unicode character in UTF-8.
 *
 * \note This class does not have any special support for right-to-left
 * scripts or composed characters.  Unicode characters are read and written
 * in the order in which they arrive.  Applications may need to alter
 * strings to display them correctly in such scripts.  Patches are
 * welcome to fix this.
 *
 * \sa Shell
 */

/** @cond */

// States for the key recognition state machine.
#define STATE_INIT      0       // Initial state.
#define STATE_CR        1       // Last character was CR, eat following LF.
#define STATE_ESC       2       // Last character was ESC.
#define STATE_MATCH     3       // Matching an escape sequence.
#define STATE_UTF8      4       // Recognizing a UTF-8 sequence.
#define STATE_IAC       5       // Recognizing telnet command after IAC (0xFF).
#define STATE_WILL      6       // Waiting for option code for WILL command.
#define STATE_WONT      7       // Waiting for option code for WONT command.
#define STATE_DO        8       // Waiting for option code for DO command.
#define STATE_DONT      9       // Waiting for option code for DONT command.
#define STATE_SB        10      // Option sub-negotiation.
#define STATE_SB_IAC    11      // Option sub-negotiation, byte after IAC.

// Number of milliseconds to wait after an ESC character before
// concluding that it is KEY_ESC rather than an escape sequence.
#define ESC_TIMEOUT_MS  40

// Number of milliseconds to wait for a new character within an
// escape sequence before concluding that the sequence was invalid
// or truncated, or not actually an escape sequence at all.
#define SEQ_TIMEOUT_MS  200

/** @endcond */

/**
 * \brief Constructs a terminal object.
 *
 * This constructor must be followed by a call to begin() to specify
 * the underlying stream to use for reading and writing.
 *
 * \sa begin()
 */
Terminal::Terminal()
    : _stream(0)
    , ucode(-1)
    , ncols(80)
    , nrows(24)
    , timer(0)
    , offset(0)
    , state(STATE_INIT)
    , utf8len(0)
    , mod(Terminal::Serial)
    , flags(0)
{
}

/**
 * \brief Destroys this terminal object.
 */
Terminal::~Terminal()
{
}

/**
 * \enum Terminal::Mode
 * \brief Mode to operate in, Serial or Telnet.
 */

/**
 * \var Terminal::Serial
 * \brief Operates the terminal in serial mode.
 */

/**
 * \var Terminal::Telnet
 * \brief Operates the terminal in telnet mode.
 */

/**
 * \brief Begins terminal operations on an underlying stream.
 *
 * \param stream The underlying stream, whether a serial port, TCP connection,
 * or some other stream.
 * \param mode The mode to operate in, either Serial or Telnet.
 *
 * If Telnet mode is selected, then embedded commands and options from the
 * telnet protocol (<a href="https://tools.ietf.org/html/rfc854">RFC 854</a>)
 * will be interpreted.  This is useful if the underlying \a stream is a TCP
 * connection on port 23.  The mode operates as a telnet server.
 *
 * \sa end(), mode()
 */
void Terminal::begin(Stream &stream, Mode mode)
{
    _stream = &stream;
    ucode = -1;
    state = STATE_INIT;
    flags = 0;
    mod = mode;
}

/**
 * \brief Ends terminal operations on an underlying stream.
 *
 * This function may be useful if you want to detach the terminal from
 * the underlying stream so that it can be used for something else.
 */
void Terminal::end()
{
    _stream = 0;
}

/**
 * \fn Terminal::Mode Terminal::mode() const
 * \brief Returns the mode this terminal is operating in, Serial or Telnet.
 *
 * \sa begin()
 */

/**
 * \brief Returns the number of bytes that are available for reading.
 *
 * \note It is possible for this function to return a positive value
 * while readKey() does not produce a new key.  This can happen with
 * VT100 key escape sequences and UTF-8 characters that extend over
 * multiple bytes.
 *
 * \sa readKey()
 */
int Terminal::available()
{
    return _stream ? _stream->available() : 0;
}

/**
 * \brief Peeks at the next byte from the underlying stream.
 *
 * \return The next byte or -1 if no bytes are available yet.
 *
 * \sa read()
 */
int Terminal::peek()
{
    return _stream ? _stream->peek() : -1;
}

/**
 * \brief Reads the next byte from the underlying stream.
 *
 * \return Returns 0x00 to 0xFF if a byte is ready, or -1 if none available.
 *
 * This function performs a low-level read on the underlying byte stream
 * without applying any specific interpretation to the byte.  In particular,
 * escape sequences corresponding to arrow and function keys will not
 * be recognized.
 *
 * Applications will usually want to call readKey() instead to handle
 * escape sequences for arrow and function keys.  This function is provided
 * as a convenience to implement the parent Stream interface.
 *
 * \sa readKey()
 */
int Terminal::read()
{
    // Clear the key recognition state because we are bypassing readKey().
    state = STATE_INIT;
    ucode = -1;

    // Read the next byte from the underlying stream.
    return _stream ? _stream->read() : -1;
}

/**
 * \brief Flushes all data in the underlying stream.
 */
void Terminal::flush()
{
    if (_stream)
        _stream->flush();
}

/**
 * \brief Writes a single byte to the underlying stream.
 *
 * \param c The byte to write.
 * \return The number of bytes written, zero on error.
 */
size_t Terminal::write(uint8_t c)
{
    return _stream ? _stream->write(c) : 0;
}

/**
 * \brief Writes a buffer of data to the underlying stream.
 *
 * \param buffer Points to the buffer to write.
 * \param size The number of bytes in the \a buffer.
 *
 * \return The number of bytes written, which may be short on error.
 */
size_t Terminal::write(const uint8_t *buffer, size_t size)
{
    return _stream ? _stream->write(buffer, size) : 0;
}

/**
 * \brief Writes a static string that is stored in program memory.
 *
 * \param str Points to the NUL-terminated string in program memory.
 *
 * This is a convenience function for printing static strings that
 * are stored in program memory.
 *
 * \sa write()
 */
void Terminal::writeProgMem(const char *str)
{
    uint8_t ch;
    if (!_stream)
        return;
    while ((ch = pgm_read_byte((const uint8_t *)str)) != 0) {
        _stream->write(ch);
        ++str;
    }
}

/**
 * \brief Determine if a character starts an escape sequence after ESC.
 *
 * \param ch The character to test.
 *
 * \return Returns true if \a ch starts an escape sequence; false otherwise.
 */
static bool escapeSequenceStart(int ch)
{
    if (ch == '[' || ch == '?')
        return true;
    else if (ch >= 'A' && ch <= 'Z')
        return true;
    else
        return false;
}

/**
 * \brief Reads the next key that was typed on this terminal.
 *
 * \return Returns -1 if there is no key ready yet; 0x00 to 0x7F for
 * an ASCII character; KEY_UNICODE for an extended Unicode code point,
 * or a USB keyboard code for special arrow or function keys.
 *
 * For example, if the user types the Home key, then this function
 * will return KEY_HOME.  If the user types the capital letter A,
 * then this function will return 0x41.
 *
 * If the user types an extended Unicode character (U+0080 and higher),
 * then this function will return KEY_UNICODE.  The application should
 * call unicodeKey() to retrieve the actual code point.  All Unicode
 * characters are assumed to be in the UTF-8 encoding on the stream.
 *
 * Some ASCII control characters correspond to special keys and will
 * be mapped appropriately:
 *
 * \li 0x08 (CTRL-H) and 0x7F (DEL) are mapped to KEY_BACKSPACE
 * \li 0x0D (CTRL-M) and 0x0A (CTRL-J) are mapped to KEY_RETURN
 * \li 0x09 (CTRL-I) is mapped to KEY_TAB
 * \li 0x1B (CTRL-[) is mapped to KEY_ESCAPE
 *
 * In all of these cases, the original ASCII code will be reported
 * by unicodeKey().  As a special case, if 0x0D is immediately followed
 * by 0x0A (that is, CRLF) then KEY_RETURN will be reported only once
 * with unicodeKey() set to 0x0D.  This ensures that all line ending
 * types are mapped to a single KEY_RETURN report.
 *
 * If the window size has changed due to a remote event, then KEY_WINSIZE
 * will be returned.  This can allow the caller to clear and redraw the
 * window in the new size.
 *
 * \sa unicodeKey(), read()
 */
int Terminal::readKey()
{
    int ch;

    // Bail out if there is no underlying stream.
    if (!_stream)
        return -1;

    // Read the next character and bail out if nothing yet.  Some special
    // peek-ahead handling is needed just after the ESC character.
    if (state == STATE_ESC) {
        ch = _stream->peek();
        if (ch < 0) {
            // We just saw an ESC.  If there has been a timeout
            // then the key is KEY_ESC rather than the start of a
            // VT100 escape sequence.
            if ((millis() - timer) >= ESC_TIMEOUT_MS) {
                state = STATE_INIT;
                ucode = 0x1B;
                return KEY_ESC;
            }
            ucode = -1;
            return -1;
        } else if (!escapeSequenceStart(ch)) {
            // The next character is not legitimate as the start of
            // an escape sequence, so the ESC must have been KEY_ESC.
            state = STATE_INIT;
            ucode = 0x1B;
            return KEY_ESC;
        } else {
            // Part of an escape sequence.  Read the character properly.
            ch = _stream->read();
        }
    } else {
        // Read the next character without any peek-ahead.
        ch = _stream->read();
    }
    if (ch < 0) {
        if (state == STATE_MATCH && (millis() - timer) >= SEQ_TIMEOUT_MS) {
            // Timeout while waiting for the next character in an
            // escape sequence.  Abort and return to the initial state.
            state = STATE_INIT;
        }
        ucode = -1;
        return -1;
    }

    // Determine what to do based on the key recognition state.
    switch (state) {
    case STATE_CR:
        // We just saw a CR, so check for CRLF and eat the LF.
        state = STATE_INIT;
        if (ch == 0x0A) {
            ucode = -1;
            return -1;
        } else if (ch == 0x00 && mod == Telnet) {
            // In telnet mode, CR NUL is a literal carriage return,
            // separate from the newline sequence CRLF.  Eat the NUL.
            // We already reported KEY_RETURN for the CR character.
            ucode = -1;
            return -1;
        }
        // Fall through to the next case.

    case STATE_INIT:
        if (ch >= 0x20 && ch <= 0x7E) {
            // Printable ASCII character.
            state = STATE_INIT;
            ucode = ch;
            return ch;
        } else if (ch == 0x1B) {
            // Start of an escape sequence, or the escape character itself.
            state = STATE_ESC;
            timer = millis();
        } else if (ch == 0x0D) {
            // CR which may be followed by an LF.
            state = STATE_CR;
            ucode = ch;
            return KEY_RETURN;
        } else if (ch == 0x0A) {
            // LF on its own without a preceding CR.
            ucode = ch;
            return KEY_RETURN;
        } else if (ch == 0x08 || ch == 0x7F) {
            // Backspace or DEL character.
            state = STATE_INIT;
            ucode = ch;
            return KEY_BACKSPACE;
        } else if (ch == 0x09) {
            // TAB character.
            state = STATE_INIT;
            ucode = ch;
            return KEY_TAB;
        } else if (ch < 0x80) {
            // Some other ASCII control character.
            state = STATE_INIT;
            ucode = ch;
            return ch;
        } else if (ch >= 0xC1 && ch <= 0xDF) {
            // Two-byte UTF-8 sequence.
            offset = ch & 0x1F;
            utf8len = 2;
            state = STATE_UTF8;
        } else if (ch >= 0xE1 && ch <= 0xEF) {
            // Three-byte UTF-8 sequence.
            offset = ch & 0x0F;
            utf8len = 3;
            state = STATE_UTF8;
        } else if (ch >= 0xF1 && ch <= 0xF7) {
            // Four-byte UTF-8 sequence.
            offset = ch & 0x07;
            utf8len = 4;
            state = STATE_UTF8;
        } else if (ch == 0xFF && mod == Telnet) {
            // Start of a telnet command (IAC byte).
            state = STATE_IAC;
        }
        break;

    case STATE_ESC:
        // Next character just after the ESC.  Start the escape
        // sequence matching engine at offset zero in the keymap table.
        state = STATE_MATCH;
        offset = 0;
        // Fall through to the next case.

    case STATE_MATCH:
        // In the middle of matching an escape sequence.
        if (ch == 0x1B) {
            // ESC character seen in the middle of an escape sequence.
            // The previous escape sequence is invalid so abort and restart.
            state = STATE_ESC;
            timer = millis();
            break;
        }
        ch = matchEscape(ch);
        if (ch == -1) {
            // Need more characters before knowing what this is.
            timer = millis();
        } else if (ch == -2) {
            // Invalid escape sequence so abort and restart.
            state = STATE_INIT;
        } else if (ch < 0x80) {
            // Escape sequence corresponds to a normal ASCII character.
            state = STATE_INIT;
            ucode = ch;
            return ch;
        } else {
            // Extended keycode for an arrow or function key.
            state = STATE_INIT;
            ucode = -1;
            return ch;
        }
        break;

    case STATE_UTF8:
        // Recognize a multi-byte UTF-8 character encoding.
        if ((ch & 0xC0) == 0x80) {
            if (utf8len <= 2) {
                // Final character in the sequence.
                ucode = (((long)offset) << 6) | (ch & 0x3F);
                state = STATE_INIT;
                if (ucode > 0x10FFFFL)
                    break;      // The code point is out of range.
                return KEY_UNICODE;
            } else {
                // More characters still yet to come.
                --utf8len;
                offset = (offset << 6) | (ch & 0x3F);
            }
        } else {
            // This character is invalid as part of a UTF-8 sequence.
            state = STATE_INIT;
        }
        break;

    case STATE_IAC:
        // Telnet command byte just after an IAC (0xFF) character.
        switch (ch) {
        case TelnetDefs::EndOfFile:
            // Convert EOF into CTRL-D.
            state = STATE_INIT;
            ucode = 0x04;
            return 0x04;

        case TelnetDefs::EndOfRecord:
            // Convert end of record markers into CR.
            state = STATE_INIT;
            ucode = 0x0D;
            return KEY_RETURN;

        case TelnetDefs::Interrupt:
            // Convert interrupt into CTRL-C.
            state = STATE_INIT;
            ucode = 0x03;
            return 0x03;

        case TelnetDefs::EraseChar:
            // Convert erase character into DEL.
            state = STATE_INIT;
            ucode = 0x7F;
            return KEY_BACKSPACE;

        case TelnetDefs::EraseLine:
            // Convert erase line into CTRL-U.
            state = STATE_INIT;
            ucode = 0x15;
            return 0x15;

        case TelnetDefs::SubStart:
            // Option sub-negotiation.
            utf8len = 0;
            state = STATE_SB;
            break;

        case TelnetDefs::WILL:
            // Option negotiation, WILL command.
            state = STATE_WILL;
            break;

        case TelnetDefs::WONT:
            // Option negotiation, WONT command.
            state = STATE_WONT;
            break;

        case TelnetDefs::DO:
            // Option negotiation, DO command.
            state = STATE_DO;
            break;

        case TelnetDefs::DONT:
            // Option negotiation, DONT command.
            state = STATE_DONT;
            break;

        case TelnetDefs::IAC:
            // IAC followed by IAC is the literal byte 0xFF,
            // but that isn't valid UTF-8 so we just drop it.
            state = STATE_INIT;
            break;

        default:
            // Everything else is treated as a NOP.
            state = STATE_INIT;
            break;
        }
        break;

    case STATE_WILL:
        // Telnet option negotiation, WILL command.  Note: We don't do any
        // loop detection.  We assume that the client will eventually break
        // the loop as it probably has more memory than us to store state.
        if (ch == TelnetDefs::WindowSize ||
                ch == TelnetDefs::RemoteFlowControl) {
            // Send a DO command in response - we accept this option.
            telnetCommand(TelnetDefs::DO, ch);
        } else {
            // Send a DONT command in response - we don't accept this option.
            telnetCommand(TelnetDefs::DONT, ch);
        }
        if (!(flags & 0x01)) {
            // The first time we see a WILL command from the client we
            // send a request back saying that we will handle echoing.
            flags |= 0x01;
            telnetCommand(TelnetDefs::WILL, TelnetDefs::Echo);
        }
        state = STATE_INIT;
        break;

    case STATE_WONT:
    case STATE_DONT:
        // Telnet option negotiation, WONT/DONT command.  The other side
        // is telling us that it does not understand this option or wants
        // us to stop using it.  For now there is nothing to do.
        state = STATE_INIT;
        break;

    case STATE_DO:
        // Telnet option negotiation, DO command.  Note: Other than Echo
        // we don't do any loop detection.  We assume that the client will
        // break the loop as it probably has more memory than us to store state.
        if (ch == TelnetDefs::Echo) {
            // Special handling needed for Echo - don't say WILL again
            // when the client acknowledges us with a DO command.
        } else if (ch == TelnetDefs::SuppressGoAhead) {
            // Send a WILL command in response - we accept this option.
            telnetCommand(TelnetDefs::WILL, ch);
        } else {
            // Send a WONT command in response - we don't accept this option.
            telnetCommand(TelnetDefs::WONT, ch);
        }
        state = STATE_INIT;
        break;

    case STATE_SB:
        // Telnet option sub-negotiation.  Collect up all bytes and
        // then execute the option once "IAC SubEnd" is seen.
        if (ch == TelnetDefs::IAC) {
            // IAC byte, which will be followed by either IAC or SubEnd.
            state = STATE_SB_IAC;
            break;
        }
        if (utf8len < sizeof(sb))
            sb[utf8len++] = 0xFF;
        break;

    case STATE_SB_IAC:
        // Telnet option sub-negotiation, byte after IAC.
        if (ch == TelnetDefs::IAC) {
            // Two IAC bytes in a row is a single escaped 0xFF byte.
            if (utf8len < sizeof(sb))
                sb[utf8len++] = 0xFF;
            state = STATE_SB;
            break;
        } else if (ch == TelnetDefs::SubEnd) {
            // End of the sub-negotiation field.  Handle window size changes.
            if (utf8len >= 5 && sb[0] == TelnetDefs::WindowSize) {
                int width  = (((int)(sb[1])) << 8) | sb[2];
                int height = (((int)(sb[3])) << 8) | sb[4];
                if (!width)         // Zero width or height means "unspecified".
                    width = ncols;
                if (!height)
                    height = nrows;

                // Filter out obviously bogus values.
                if (width >= 1 && height >= 1 && width <= 10000 && height <= 10000) {
                    if (width != ncols || height != nrows) {
                        // The window size has changed; notify the caller.
                        ncols = width;
                        nrows = height;
                        ucode = -1;
                        state = STATE_INIT;
                        return KEY_WINSIZE;
                    }
                }
            }
        }
        state = STATE_INIT;
        break;
    }

    // If we get here, then we're still waiting for a full sequence.
    ucode = -1;
    return -1;
}

/**
 * \fn long Terminal::unicodeKey() const
 * \brief Gets the Unicode version of the last key returned by readKey().
 *
 * If readKey() returned an ASCII character (0x00 to 0x7F) or KEY_UNICODE,
 * then this function can be used to query the full Unicode code point for
 * the key that was typed.  If some other key is typed, or no key was
 * typed, then this function will return -1.
 *
 * Unicode code points range between 0 and 0x10FFFF.
 *
 * \sa readKey(), writeUnicode()
 */

/**
 * \brief Writes a Unicode code point to the output in UTF-8 encoding.
 *
 * \param code The code point to be written between 0 and 0x10FFFF.
 * \return The number of bytes that were written to the underlying stream
 * to represent \a code.  Returns zero if \a code is not a valid code point.
 *
 * This function is useful when a specific Unicode character is desired;
 * for example the code point 0x2264 corresponds to the less than or
 * equal to operator "≤".  See the Unicode standard for more information.
 *
 * Unicode characters between 0x00 and 0x7F and strings that are already
 * in the UTF-8 encoding can also be written using write().
 *
 * \sa write()
 */
size_t Terminal::writeUnicode(long code)
{
    uint8_t utf8[4];
    size_t size = utf8Format(utf8, code);
    if (size > 0)
        write(utf8, size);
    return size;
}

/**
 * \fn int Terminal::columns() const
 * \brief Gets the number of columns in the window; defaults to 80.
 *
 * \sa rows(), setWindowSize(), cursorMove()
 */

/**
 * \fn int Terminal::rows() const
 * \brief Gets the number of rows in the window; defaults to 24.
 *
 * \sa columns(), setWindowSize(), cursorMove()
 */

/**
 * \brief Sets the number of columns and rows in the window.
 *
 * \param columns The number of columns between 1 and 10000.
 * \param rows The number of rows between 1 and 10000.
 *
 * This function should be used if the application has some information
 * about the actual window size.  For serial ports, this usually isn't
 * available but telnet and ssh sessions can get the window size from
 * the remote host.
 *
 * The window size defaults to 80x24 which is the standard default for
 * terminal programs like PuTTY that emulate a VT100.
 *
 * If the window size changes due to a remote event, readKey() will
 * return KEY_WINSIZE to inform the application.
 *
 * \sa columns(), rows(), readKey()
 */
void Terminal::setWindowSize(int columns, int rows)
{
    // Sanity-check the range first.
    if (columns < 1)
        columns = 1;
    else if (columns > 10000)
        columns = 10000;
    if (rows < 1)
        rows = 1;
    else if (rows > 10000)
        rows = 10000;
    ncols = columns;
    nrows = rows;
}

/**
 * \brief Move the cursor to the top-left position and clear the screen.
 */
void Terminal::clear()
{
    static char const escape[] PROGMEM = "\033[H\033[J";
    writeProgMem(escape);
}

/**
 * \brief Clears from the current cursor position to the end of the line.
 */
void Terminal::clearToEOL()
{
    static char const escape[] PROGMEM = "\033[K";
    writeProgMem(escape);
}

/**
 * \brief Moves the cursor to a specific location in the window.
 *
 * \param x The x position for the cursor between 0 and columns() - 1.
 * \param y The y position for the cursor between 0 and rows() - 1.
 *
 * \sa cursorLeft(), columns(), rows(), setWindowSize()
 */
void Terminal::cursorMove(int x, int y)
{
    if (!_stream)
        return;
    if (x < 0)
        x = 0;
    else if (x >= ncols)
        x = ncols - 1;
    if (y < 0)
        y = 0;
    else if (y >= nrows)
        y = nrows - 1;
    _stream->write((uint8_t)0x1B);
    _stream->write((uint8_t)'[');
    _stream->print(y + 1);
    _stream->write((uint8_t)';');
    _stream->print(x + 1);
    _stream->write((uint8_t)'H');
}

/**
 * \brief Moves the cursor left by one character.
 *
 * \sa cursorRight(), cursorUp(), cursorDown(), cursorMove()
 */
void Terminal::cursorLeft()
{
    static char const escape[] PROGMEM = "\033[D";
    writeProgMem(escape);
}

/**
 * \brief Moves the cursor right by one character.
 *
 * \sa cursorLeft(), cursorUp(), cursorDown(), cursorMove()
 */
void Terminal::cursorRight()
{
    static char const escape[] PROGMEM = "\033[C";
    writeProgMem(escape);
}

/**
 * \brief Moves the cursor up by one line.
 *
 * \sa cursorDown(), cursorLeft(), cursorRight(), cursorMove()
 */
void Terminal::cursorUp()
{
    static char const escape[] PROGMEM = "\033[A";
    writeProgMem(escape);
}

/**
 * \brief Moves the cursor down by one line.
 *
 * \sa cursorUp(), cursorLeft(), cursorRight(), cursorMove()
 */
void Terminal::cursorDown()
{
    static char const escape[] PROGMEM = "\033[B";
    writeProgMem(escape);
}

/**
 * \brief Backspaces over the last character.
 *
 * This function prints CTRL-H, a space, and another CTRL-H to backspace
 * over and erase the last character on the current line.
 *
 * If the last character was a wide Unicode character, then two calls to
 * this function are required to erase it.  See isWideCharacter() for
 * more information.
 *
 * \sa isWideCharacter()
 */
void Terminal::backspace()
{
    static char const escape[] PROGMEM = "\b \b";
    writeProgMem(escape);
}

/**
 * \brief Inserts a line at the cursor position.
 *
 * \sa insertChar(), deleteLine()
 */
void Terminal::insertLine()
{
    static char const escape[] PROGMEM = "\033[L";
    writeProgMem(escape);
}

/**
 * \brief Inserts a blank character at the cursor position.
 *
 * \sa insertLine(), deleteChar()
 */
void Terminal::insertChar()
{
    static char const escape[] PROGMEM = "\033[@";
    writeProgMem(escape);
}

/**
 * \brief Deletes a line at the cursor position.
 *
 * \sa deleteChar(), insertLine()
 */
void Terminal::deleteLine()
{
    static char const escape[] PROGMEM = "\033[M";
    writeProgMem(escape);
}

/**
 * \brief Deletes the character at the cursor position.
 *
 * \sa deleteLine(), insertChar()
 */
void Terminal::deleteChar()
{
    static char const escape[] PROGMEM = "\033[P";
    writeProgMem(escape);
}

/**
 * \brief Scrolls the contents of the window up one line.
 *
 * \sa scrollDown()
 */
void Terminal::scrollUp()
{
    static char const escape[] PROGMEM = "\033[S";
    writeProgMem(escape);
}

/**
 * \brief Scrolls the contents of the window down one line.
 *
 * \sa scrollUp()
 */
void Terminal::scrollDown()
{
    static char const escape[] PROGMEM = "\033[T";
    writeProgMem(escape);
}

/**
 * \brief Selects normal text with all attributes and colors off.
 *
 * \sa color(), bold(), underline(), blink(), reverse()
 */
void Terminal::normal()
{
    static char const escape[] PROGMEM = "\033[0m";
    writeProgMem(escape);
}

/**
 * \brief Enables bold text.
 *
 * \sa normal()
 */
void Terminal::bold()
{
    static char const escape[] PROGMEM = "\033[1m";
    writeProgMem(escape);
}

/**
 * \brief Enables underlined text.
 */
void Terminal::underline()
{
    static char const escape[] PROGMEM = "\033[4m";
    writeProgMem(escape);
}

/**
 * \brief Enables blinking text.
 */
void Terminal::blink()
{
    static char const escape[] PROGMEM = "\033[5m";
    writeProgMem(escape);
}

/**
 * \brief Reverse the foreground and background colors for inverted text.
 */
void Terminal::reverse()
{
    static char const escape[] PROGMEM = "\033[7m";
    writeProgMem(escape);
}

/**
 * \enum Terminal::Color
 * \brief Terminal foreground or background colors.
 */

/**
 * \var Terminal::Black
 * \brief Color is black.
 */

/**
 * \var Terminal::DarkRed
 * \brief Color is dark red.
 */

/**
 * \var Terminal::DarkGreen
 * \brief Color is dark green.
 */

/**
 * \var Terminal::DarkYellow
 * \brief Color is dark yellow.
 */

/**
 * \var Terminal::DarkBlue
 * \brief Color is dark blue.
 */

/**
 * \var Terminal::DarkMagenta
 * \brief Color is dark magenta.
 */

/**
 * \var Terminal::DarkCyan
 * \brief Color is dark cyan.
 */

/**
 * \var Terminal::LightGray
 * \brief Color is light gray.
 */

/**
 * \var Terminal::DarkGray
 * \brief Color is dark gray.
 */

/**
 * \var Terminal::Red
 * \brief Color is light red.
 */

/**
 * \var Terminal::Green
 * \brief Color is light green.
 */

/**
 * \var Terminal::Yellow
 * \brief Color is light yellow.
 */

/**
 * \var Terminal::Blue
 * \brief Color is light blue.
 */

/**
 * \var Terminal::Magenta
 * \brief Color is light magenta.
 */

/**
 * \var Terminal::Cyan
 * \brief Color is light cyan.
 */

/**
 * \var Terminal::White
 * \brief Color is white.
 */

/**
 * \brief Selects a text foreground color with the default background color.
 *
 * \param fg The foreground color to select.
 *
 * All other text attributes (reverse, underline, etc) are disabled.
 *
 * The following example displays a warning string with the initial
 * word in red and all following words in normal text:
 *
 * \code
 * term.color(Terminal::Red);
 * term.print("WARNING: ");
 * term.normal();
 * term.println("All files on the SD card will be deleted!");
 * \endcode
 *
 * \sa normal()
 */
void Terminal::color(Color fg)
{
    uint8_t code = (fg & 0x07);
    uint8_t bold = (fg & 0x08) ? 1 : 0;
    if (!_stream)
        return;
    _stream->write((uint8_t)0x1B);
    _stream->write((uint8_t)'[');
    _stream->write((uint8_t)'0'); // reset all attributes first
    _stream->write((uint8_t)';');
    _stream->write((uint8_t)'3');
    _stream->write((uint8_t)('0' + code));
    if (bold) {
        _stream->write((uint8_t)';');
        _stream->write((uint8_t)'1');
    }
    _stream->write((uint8_t)'m');
}

/**
 * \brief Selects text foreground and background colors.
 *
 * \param fg The foreground color to select.
 * \param bg The background color to select.
 *
 * All other text attributes (reverse, underline, etc) are disabled.
 *
 * \sa normal()
 */
void Terminal::color(Color fg, Color bg)
{
    uint8_t codefg = (fg & 0x07);
    uint8_t boldfg = (fg & 0x08) ? 1 : 0;
    uint8_t codebg = (bg & 0x07);
    if (!_stream)
        return;
    _stream->write((uint8_t)0x1B);
    _stream->write((uint8_t)'[');
    _stream->write((uint8_t)'0'); // reset all attributes first
    _stream->write((uint8_t)';');
    _stream->write((uint8_t)'3');
    _stream->write((uint8_t)('0' + codefg));
    if (boldfg) {
        _stream->write((uint8_t)';');
        _stream->write((uint8_t)'1');
    }
    _stream->write((uint8_t)';');
    _stream->write((uint8_t)'4');
    _stream->write((uint8_t)('0' + codebg));
    _stream->write((uint8_t)'m');
}

/**
 * \brief Determine if a Unicode character is wide.
 *
 * \param code The code point for the Unicode character.
 * \return Returns true if \a code is a wide character, false otherwise.
 *
 * Wide characters typically come from East Asian languages and occupy
 * two spaces in a terminal.  Two calls to backspace() are required to
 * erase such characters.
 *
 * References: http://www.unicode.org/reports/tr11/,
 * http://www.unicode.org/Public/UCD/latest/ucd/EastAsianWidth.txt
 *
 * \sa backspace(), writeUnicode()
 */
bool Terminal::isWideCharacter(long code)
{
    // This function was automatically generated by genwcwidth.c
    static unsigned char const range3000[32] PROGMEM = {
        0xF1, 0xFF, 0xF3, 0x3F, 0x01, 0x00, 0x01, 0x78,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88
    };
    static unsigned char const rangeFE00[64] PROGMEM = {
        0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0xE1, 0xFF,
        0x9F, 0x01, 0x00, 0x7F, 0x0C, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
        0x01, 0x00, 0x00, 0xF8, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00
    };
    unsigned c;
    if (code < 0x2300) {
        return false;
    } else if (code >= 0x3000 && code <= 0x30FF) {
        c = (unsigned)(code - 0x3000);
        return (pgm_read_byte(range3000 + (c / 8)) & (1 << (c % 8))) != 0;
    } else if (code >= 0xFE00 && code <= 0xFFFF) {
        c = (unsigned)(code - 0xFE00);
        return (pgm_read_byte(rangeFE00 + (c / 8)) & (1 << (c % 8))) != 0;
    } else if (code >= 0x3400 && code <= 0x4DBF) {
        return true;
    } else if (code >= 0x4E00 && code <= 0x9FFF) {
        return true;
    } else if (code >= 0xF900 && code <= 0xFAFF) {
        return true;
    } else if (code >= 0x20000 && code <= 0x2FFFD) {
        return true;
    } else if (code >= 0x30000 && code <= 0x3FFFD) {
        return true;
    } else if (code == 0x2329 ||
               code == 0x232A ||
               code == 0x3250 ||
               code == 0xA015) {
        return true;
    }
    return false;
}
/**
 * \brief Determines the length of a Unicode code point in the UTF-8 encoding.
 *
 * \param code The code point to be written between 0 and 0x10FFFF.
 * \return The number of bytes that makes up the UTF-8 encoding of \a code.
 * Returns zero if \a code is not a valid code point.
 *
 * \sa utf8Format(), writeUnicode()
 */
size_t Terminal::utf8Length(long code)
{
    // Reference: https://tools.ietf.org/html/rfc3629
    if (code < 0) {
        return 0;
    } else if (code <= 0x7FL) {
        return 1;
    } else if (code <= 0x07FFL) {
        return 2;
    } else if (code >= 0xD800L && code <= 0xDFFF) {
        // UTF-16 surrogate pairs are not valid in UTF-8.
        return 0;
    } else if (code <= 0xFFFFL) {
        return 3;
    } else if (code <= 0x10FFFFL) {
        return 4;
    } else {
        return 0;
    }
}

/**
 * \brief Formats a Unicode code point in a buffer in the UTF-8 encoding.
 *
 * \param buffer The buffer to write the UTF-8 encoding to.  At most 4
 * bytes will be written to this buffer.
 * \param code The code point to be written between 0 and 0x10FFFF.
 * \return The number of bytes that were written to \a buffer to represent
 * \a code.  Returns zero if \a code is not a valid code point.
 *
 * \sa utf8Length(), writeUnicode()
 */
size_t Terminal::utf8Format(uint8_t *buffer, long code)
{
    // Reference: https://tools.ietf.org/html/rfc3629
    if (code < 0) {
        return 0;
    } else if (code <= 0x7FL) {
        buffer[0] = (uint8_t)code;
        return 1;
    } else if (code <= 0x07FFL) {
        buffer[0] = 0xC0 | (uint8_t)(code >> 6);
        buffer[1] = 0x80 | (((uint8_t)code) & 0x3F);
        return 2;
    } else if (code >= 0xD800L && code <= 0xDFFF) {
        // UTF-16 surrogate pairs are not valid in UTF-8.
        return 0;
    } else if (code <= 0xFFFFL) {
        buffer[0] = 0xE0 | (uint8_t)(code >> 12);
        buffer[1] = 0x80 | (((uint8_t)(code >> 6)) & 0x3F);
        buffer[2] = 0x80 | (((uint8_t)code) & 0x3F);
        return 3;
    } else if (code <= 0x10FFFFL) {
        buffer[0] = 0xF0 | (uint8_t)(code >> 18);
        buffer[1] = 0x80 | (((uint8_t)(code >> 12)) & 0x3F);
        buffer[2] = 0x80 | (((uint8_t)(code >> 6)) & 0x3F);
        buffer[3] = 0x80 | (((uint8_t)code) & 0x3F);
        return 4;
    } else {
        return 0;
    }
}

// Keymap rule table.  Compact representation of a recognition tree.
// Each tree node is an array of entries of the following forms:
//      0           End of this tree level.
//      ch code     Leaf node: ASCII character (bit 7 clear) plus 8-bit keycode.
//      ch offset   Interior node: ASCII character with the high bit set
//                  plus a 16-bit offset to the first child node.
// This table was generated with the "genkeymap" tool.  Do not edit this
// table but rather edit the tool and rebuild the table from it.
static uint8_t const keymap[459] PROGMEM = {
    0xDB, 0x1A, 0x00, 0xCF, 0x57, 0x01, 0x41, 0xDA, 0x42, 0xD9, 0x43, 0xD7,
    0x44, 0xD8, 0xBF, 0xA2, 0x01, 0x50, 0xC2, 0x51, 0xC3, 0x52, 0xC4, 0x53,
    0xC5, 0x00, 0x41, 0xDA, 0x42, 0xD9, 0x43, 0xD7, 0x44, 0xD8, 0x48, 0xD2,
    0xB1, 0x42, 0x00, 0x46, 0xD5, 0xB4, 0xC9, 0x00, 0xB2, 0xCC, 0x00, 0xB3,
    0x2B, 0x01, 0xB5, 0x46, 0x01, 0xB6, 0x49, 0x01, 0xDB, 0x4C, 0x01, 0x5A,
    0x0B, 0x50, 0xD0, 0x47, 0xE5, 0x00, 0x7E, 0xD2, 0xB1, 0x5D, 0x00, 0xB2,
    0x6C, 0x00, 0xB3, 0x7B, 0x00, 0xB4, 0x88, 0x00, 0xB5, 0x95, 0x00, 0xB7,
    0xA2, 0x00, 0xB8, 0xAF, 0x00, 0xB9, 0xBC, 0x00, 0x00, 0x7E, 0xC2, 0xBB,
    0x65, 0x00, 0x5E, 0xFA, 0x00, 0xB2, 0x69, 0x00, 0x00, 0x7E, 0xF0, 0x00,
    0x7E, 0xC3, 0xBB, 0x74, 0x00, 0x5E, 0xFB, 0x00, 0xB2, 0x78, 0x00, 0x00,
    0x7E, 0xF1, 0x00, 0x7E, 0xC4, 0xBB, 0x81, 0x00, 0x00, 0xB2, 0x85, 0x00,
    0x00, 0x7E, 0xF2, 0x00, 0x7E, 0xC5, 0xBB, 0x8E, 0x00, 0x00, 0xB2, 0x92,
    0x00, 0x00, 0x7E, 0xF3, 0x00, 0x7E, 0xC6, 0xBB, 0x9B, 0x00, 0x00, 0xB2,
    0x9F, 0x00, 0x00, 0x7E, 0xF4, 0x00, 0x7E, 0xC7, 0xBB, 0xA8, 0x00, 0x00,
    0xB2, 0xAC, 0x00, 0x00, 0x7E, 0xF5, 0x00, 0x7E, 0xC8, 0xBB, 0xB5, 0x00,
    0x00, 0xB2, 0xB9, 0x00, 0x00, 0x7E, 0xF6, 0x00, 0x7E, 0xC9, 0xBB, 0xC2,
    0x00, 0x00, 0xB2, 0xC6, 0x00, 0x00, 0x7E, 0xF7, 0x00, 0x7E, 0xD5, 0x00,
    0x7E, 0xD1, 0xB0, 0xE7, 0x00, 0xB1, 0xF4, 0x00, 0xB3, 0x01, 0x01, 0xB4,
    0x10, 0x01, 0xB5, 0x1F, 0x01, 0xB6, 0x22, 0x01, 0xB8, 0x25, 0x01, 0xB9,
    0x28, 0x01, 0x00, 0x7E, 0xCA, 0xBB, 0xED, 0x00, 0x00, 0xB2, 0xF1, 0x00,
    0x00, 0x7E, 0xF8, 0x00, 0x7E, 0xCB, 0xBB, 0xFA, 0x00, 0x00, 0xB2, 0xFE,
    0x00, 0x00, 0x7E, 0xF9, 0x00, 0x7E, 0xCC, 0x24, 0xF8, 0xBB, 0x09, 0x01,
    0x00, 0xB2, 0x0D, 0x01, 0x00, 0x7E, 0xFA, 0x00, 0x7E, 0xCD, 0x24, 0xF9,
    0xBB, 0x18, 0x01, 0x00, 0xB2, 0x1C, 0x01, 0x00, 0x7E, 0xFB, 0x00, 0x7E,
    0xF0, 0x00, 0x7E, 0xF1, 0x00, 0x7E, 0xF2, 0x00, 0x7E, 0xF3, 0x00, 0x7E,
    0xD4, 0xB1, 0x3A, 0x01, 0xB2, 0x3D, 0x01, 0xB3, 0x40, 0x01, 0xB4, 0x43,
    0x01, 0x00, 0x7E, 0xF4, 0x00, 0x7E, 0xF5, 0x00, 0x7E, 0xF6, 0x00, 0x7E,
    0xF7, 0x00, 0x7E, 0xD3, 0x00, 0x7E, 0xD6, 0x00, 0x41, 0xC2, 0x42, 0xC3,
    0x43, 0xC4, 0x44, 0xC5, 0x45, 0xC6, 0x00, 0x41, 0xDA, 0x42, 0xD9, 0x43,
    0xD7, 0x44, 0xD8, 0x48, 0xD2, 0x46, 0xD5, 0x20, 0x20, 0x49, 0xB3, 0x4D,
    0xB0, 0x6A, 0x2A, 0x6B, 0x2B, 0x6C, 0x2C, 0x6D, 0x2D, 0x6E, 0x2E, 0x6F,
    0x2F, 0x70, 0x30, 0x71, 0x31, 0x72, 0x32, 0x73, 0x33, 0x74, 0x34, 0x75,
    0x35, 0x76, 0x36, 0x77, 0x37, 0x78, 0x38, 0x79, 0x39, 0x58, 0x3D, 0x50,
    0xC2, 0x51, 0xC3, 0x52, 0xC4, 0x53, 0xC5, 0xB2, 0x99, 0x01, 0x5A, 0x0B,
    0x00, 0x50, 0xF0, 0x51, 0xF1, 0x52, 0xF2, 0x53, 0xF3, 0x00, 0x20, 0x20,
    0x49, 0xB3, 0x4D, 0xB0, 0x6A, 0x2A, 0x6B, 0x2B, 0x6C, 0x2C, 0x6D, 0x2D,
    0x6E, 0x2E, 0x6F, 0x2F, 0x70, 0x30, 0x71, 0x31, 0x72, 0x32, 0x73, 0x33,
    0x74, 0x34, 0x75, 0x35, 0x76, 0x36, 0x77, 0x37, 0x78, 0x38, 0x79, 0x39,
    0x58, 0x3D, 0x00
};

/**
 * \brief Matches the next character in an escape sequence.
 *
 * \param ch The next character to match.
 * \return -1 if more characters are required, -2 if the escape sequence
 * is invalid, or a positive key code if the match is complete.
 */
int Terminal::matchEscape(int ch)
{
    uint8_t kch;
    for (;;) {
        kch = pgm_read_byte(keymap + offset);
        if (!kch) {
            // No match at this level, so the escape sequence is invalid.
            break;
        } else if (kch & 0x80) {
            // Interior node.
            if ((kch & 0x7F) == ch) {
                // Interior node matches.  Go down one tree level.
                offset = ((int)(pgm_read_byte(keymap + offset + 1))) |
                        (((int)(pgm_read_byte(keymap + offset + 2))) << 8);
                return -1;
            }
            offset += 3;
        } else {
            // Leaf node.
            if (kch == (uint8_t)ch) {
                // We have found a match on a full escape sequence.
                return pgm_read_byte(keymap + offset + 1);
            }
            offset += 2;
        }
    }
    return -2;
}

/**
 * \brief Sends a telnet command to the client.
 *
 * \param type The type of command: WILL, WONT, DO, or DONT.
 * \param option The telnet option the command applies to.
 */
void Terminal::telnetCommand(uint8_t type, uint8_t option)
{
    uint8_t buf[3];
    buf[0] = (uint8_t)TelnetDefs::IAC;
    buf[1] = type;
    buf[2] = option;
    _stream->write(buf, 3);
}
