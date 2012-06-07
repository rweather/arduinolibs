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

#include "IRreceiver.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class IRreceiver IRreceiver.h <IRreceiver.h>
 * \brief Manages the reception of RC-5 commands from an infrared remote control.
 *
 * IRreceiver recognizes commands in the <a href="http://en.wikipedia.org/wiki/RC-5">Philips RC-5 protocol</a>.
 * This is a fairly common infrared protocol, supported by most universal
 * remote controls.  Program the universal remote to simulate a Philips
 * TV, VCR, CD player, etc.
 *
 * This class uses interrupts to process incoming bits from a standard 3-pin
 * infrared receiver:
 *
 * \image html irchip.jpg
 *
 * Typically, pin 1 of the receiver should be connected to the Arduino
 * interrupt pin (e.g. D2), pin 2 should be connected to GND, and pin 3
 * should be connected to 5V.  Consult the datasheet for your receiver to
 * be sure though; some receivers may have different pin assignments.
 *
 * The receiver is initialized by constructing an instance of the
 * IRreceiver class:
 *
 * \code
 * IRreceiver ir;
 * \endcode
 *
 * By default, interrupt 0 on pin D2 is used.  To change to another interrupt,
 * pass its number to the constructor:
 *
 * \code
 * IRreceiver ir(1);    // Interrupt 1 on pin D3
 * \endcode
 *
 * Currently this class can only handle a single instance of IRreceiver being
 * active in the application.  It isn't possible to have separate IRreceiver
 * instances on different pins.  Usually this won't be a problem because
 * the same receiver can process inputs from multiple remotes.
 *
 * The application retrieves incoming infrared commands by calling the
 * command() function.  The return value indicates the type of command:
 *
 * \code
 * void loop() {
 *     switch (ir.command()) {
 *     case RC5_0: case RC5_1: case RC5_2: case RC5_3: case RC5_4:
 *     case RC5_5: case RC5_6: case RC5_7: case RC5_8: case RC5_9:
 *         // Process a digit
 *         ...
 *         break;
 *
 *     case RC5_ERASE:
 *         // Backspace/erase last digit.
 *         ...
 *         break;
 *
 *     case RC5_STANDBY:
 *         // Power on/off button.
 *         ...
 *         break;
 *     }
 * }
 * \endcode
 *
 * If the command is an auto-repeat of a previous button press, then the
 * \ref AUTO_REPEAT flag will be set in the value returned from command().
 * The application can choose to ignore all auto-repeats, process all
 * auto-repeats, or choose which button to auto-repeat based on its code:
 *
 * \code
 * void loop() {
 *     switch (ir.command()) {
 *     case RC5_INC_VOLUME:
 *     case IRreceiver::AUTO_REPEAT | RC5_INC_VOLUME:
 *         // Volume increase button pressed or held.
 *         ...
 *         break;
 *
 *     case RC5_DEC_VOLUME:
 *     case IRreceiver::AUTO_REPEAT | RC5_DEC_VOLUME:
 *         // Volume decrease button pressed or held.
 *         ...
 *         break;
 *
 *     case RC5_MUTE:
 *         // Mute button (ignore auto-repeat).
 *         ...
 *         break;
 *     }
 * }
 * \endcode
 *
 * By default, command codes will be generated for every type of RC-5 remote
 * control, be it a TV, VCR, CD player, or something else.  The application
 * can distinguish between the remote controls using system(); noting that
 * command() must be called before system() for the system value to be valid.
 * For example, the following code could be used in a two-player video game
 * where the first player's remote is configured as a TV and the second
 * player's remote is configured as a VCR:
 *
 * \code
 * void loop() {
 *     int cmd = ir.command();
 *     int sys = ir.system();
 *     if (sys == RC5_SYS_TV)
 *         player1(cmd);
 *     else if (sys == RC5_SYS_VCR)
 *         player2(cmd);
 *
 *     ...
 * }
 * \endcode
 *
 * If the application only cares about a single system and wishes to ignore
 * all other systems, it can configure a system filter at startup:
 *
 * \code
 * IRreceiver ir;
 *
 * void setup() {
 *     ir.setSystemFilter(RC5_SYS_VCR);
 * }
 * \endcode
 *
 * The complete list of RC-5 system numbers and command codes is given in
 * the RC5.h header file.
 *
 * \sa \ref ir_dumpir "DumpIR Example"
 */

static IRreceiver *receiver = 0;

void _IR_receive_interrupt(void)
{
    receiver->handleInterrupt();
}

/**
 * \var IRreceiver::AUTO_REPEAT
 * \brief Flag that is added to the output of command() when the command
 * is an auto-repeated button press rather than the original button press.
 */

/**
 * \brief Constructs a new infrared remote control receiver that is attached
 * to \a interruptNumber.
 */
IRreceiver::IRreceiver(int interruptNumber)
    : _system(0)
    , _systemFilter(-1)
    , started(false)
    , halfChange(false)
    , lastChange(0)
    , bits(0)
    , bitCount(0)
    , buffer(0)
    , lastBuffer(0)
{
    switch (interruptNumber) {
    case 0: default:    pin = 2; break;
    case 1:             pin = 3; break;
    case 2:             pin = 21; break;    // Arduino Mega only
    case 3:             pin = 20; break;    // Arduino Mega only
    case 4:             pin = 19; break;    // Arduino Mega only
    case 5:             pin = 18; break;    // Arduino Mega only
    }
    receiver = this;
    attachInterrupt(interruptNumber, _IR_receive_interrupt, CHANGE);
}

/**
 * \brief Returns the next command from the remote control.
 *
 * Returns -1 if there is no new command, or the number between 0 and 127
 * corresponding to the command.  If the command is an auto-repeat button
 * press rather than an original button press, then the \ref AUTO_REPEAT
 * flag will be set.
 *
 * The companion function system() will return the system number for the
 * command indicating whether the command is for a TV, VCR, CD player, etc.
 * By default, all systems are reported; use setSystemFilter() to filter
 * out commands from all but a specific system.
 *
 * The next call to command() will return -1 or the code for the next
 * button press.
 *
 * The header file <tt>RC5.h</tt> contains a list of command codes for
 * common remote controls.
 *
 * \sa system(), setSystemFilter()
 */
int IRreceiver::command()
{
    unsigned buf;

    // Read the last-delivered sequence from the buffer and clear it.
    cli();
    buf = buffer;
    buffer = 0;
    sei();

    // Bail out if no sequence or it is not for us.
    if (!buf) {
        _system = -1;
        return -1;
    }
    if (_systemFilter != -1) {
        if (((buf >> 6) & 0x1F) != _systemFilter) {
            _system = -1;
            return -1;
        }
    }

    // Extract the command.
    int cmd = buf & 0x3F;
    if ((buf & 0x1000) == 0)
        cmd += 64;

    // Is this a new command or an auto-repeat of the previous command?
    // Bit 11 will toggle whenever a new button press is started.
    if (lastBuffer == buf)
        cmd += AUTO_REPEAT;
    else
        lastBuffer = buf;
    _system = (buf >> 6) & 0x1F;
    return cmd;
}

/**
 * \fn int IRreceiver::system() const
 * \brief Returns the system number of the previous command(), indicating
 * whether the command was for a TV, VCR, CD player, etc.
 *
 * The return value from this function is valid only after a call to
 * command().  The next call to command() will clear the system value,
 * possibly to -1 if there is no new command.
 *
 * The header file <tt>RC5.h</tt> contains a list of system numbers for
 * common remote controls.
 *
 * \sa command(), setSystemFilter()
 */

/**
 * \fn int IRreceiver::systemFilter() const
 * \brief Returns the system to filter commands against, or -1 if no
 * filter is set.
 *
 * If this value is -1, then all received systems are returned via command()
 * and system() irrespective of whether they are for a TV, VCR, CD player,
 * or some other type of system.  If this value is set to anything other
 * than -1, then only commands for that system are returned via command().
 *
 * \sa setSystemFilter(), system(), command()
 */

/**
 * \fn void IRreceiver::setSystemFilter(int system)
 * \brief Sets the \a system to filter commands against, or -1 to turn
 * off the system filter.
 *
 * If \a system is -1, then all received systems are returned via command()
 * and system() irrespective of whether they are for a TV, VCR, CD player,
 * or some other type of system.  If \a system is set to anything other
 * than -1, then only commands for that system are returned via command().
 * For example:
 *
 * \code
 * IRreceiver ir;
 * ir.setSystemFilter(RC5_SYS_VCR);
 * \endcode
 *
 * \sa systemFilter(), system(), command()
 */

// Number of microseconds that the signal is HIGH or LOW for
// indicating a bit.  A 1 bit is transmitted as LOW for 889us
// followed by HIGH for 889us.  A 0 bit is HIGH, then LOW.
#define IR_BIT_TIME 889

// Number of microseconds to detect a long gap in the coding
// corresponding to 2 time units HIGH or LOW.  We actually check
// for at least 1.5 time units to allow for slight variations
// in timing on different remote controls.
#define IR_LONG_BIT_TIME (889 * 6 / 4)

// Maximum timeout for a single bit.  If we don't see a rising edge
// within this time, then we have lost sync and need to restart.
#define IR_MAX_TIME (IR_BIT_TIME * 4)

// Protocol details from http://en.wikipedia.org/wiki/RC-5
void IRreceiver::handleInterrupt()
{
    bool value = digitalRead(pin);
    unsigned long currentTime = micros();
    if (!value) {
        // Rising edge (input is active-LOW)
        if (started && (currentTime - lastChange) > IR_MAX_TIME) {
            // Too long since the last received bit, so restart the process.
            started = false;
        }
        if (started) {
            // We recognize bits on the falling edges, so merely
            // adjust the "changed at last half-cycle" flag.
            if ((currentTime - lastChange) > IR_LONG_BIT_TIME) {
                // Long time since last falling edge indicates that the
                // next bit will definitely be a 1.
                halfChange = true;
            } else {
                halfChange = !halfChange;
            }
            lastChange = currentTime;
        } else {
            // Encountered the start bit - start receiving up to 14 bits.
            lastChange = currentTime;
            started = true;
            halfChange = true;
            bits = 0;
            bitCount = 14;
        }
    } else if (started) {
        // Falling edge
        if ((currentTime - lastChange) > IR_LONG_BIT_TIME) {
            // Long time since last rise indicates 1 followed by 0.
            bits = (bits << 2) | 0x02;
            --bitCount;
            halfChange = true;
        } else if (halfChange) {
            // Rise was halfway through, so falling edge indicates a 1.
            bits = (bits << 1) | 0x01;
            halfChange = false;
        } else {
            // Rise was at the start, so falling edge indicates a 0.
            bits <<= 1;
            halfChange = true;
        }
        lastChange = currentTime;
        --bitCount;
        if (bitCount <= 0) {
            // All 14 bits have been received, so deliver the value.
            started = false;
            buffer = bits;
        }
    }
}
