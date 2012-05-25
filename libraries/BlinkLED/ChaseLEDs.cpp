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

#include "ChaseLEDs.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class ChaseLEDs ChaseLEDs.h <ChaseLEDs.h>
 * \brief Chase LED's on output pins in a defined sequence.
 *
 * The following example performs a LED chase over the 6 PWM outputs
 * on the Arduino Uno, with a 150 millisecond delay between each LED:
 *
 * \code
 * uint8_t pins[] = {3, 5, 6, 9, 10, 11};
 * ChaseLEDs chaser(pins, sizeof(pins), 150);
 *
 * void loop() {
 *     chaser.loop();
 * }
 * \endcode
 *
 * After pin 11 is lit, the pattern will repeat at pin 3.  To cause the
 * chase to oscillate back and forth instead, extend the sequence as follows:
 *
 * \code
 * uint8_t pins[] = {3, 5, 6, 9, 10, 11, 10, 9, 6, 5};
 * ChaseLEDs chaser(pins, sizeof(pins), 150);
 * \endcode
 *
 * See the \ref blink_cylon "Cylon" example for more information on
 * how to use the ChaseLEDs class in a practical application.
 */

/**
 * \brief Initializes the LED chaser.
 *
 * The chase sequence consists of \a num pins, whose names are given by
 * the \a pins array.  Each LED is lit for \a advanceTime milliseconds
 * before advancing to the next LED.
 *
 * This constructor configures all of the pins for output and sets their
 * state to be LOW.  The first LED will be lit when the program first
 * calls loop().
 *
 * \sa loop()
 */
ChaseLEDs::ChaseLEDs(const uint8_t *pins, int num, unsigned long advanceTime)
    : _pins(pins)
    , _numPins(num)
    , _currentIndex(-1)
    , _advanceTime(advanceTime)
    , _lastChange(millis())
{
    for (uint8_t index = 0; index < _numPins; ++index) {
        pinMode(_pins[index], OUTPUT);
        digitalWrite(_pins[index], LOW);
    }
}

/**
 * Perform a single iteration of the control loop for this LED chaser.
 */
void ChaseLEDs::loop()
{
    if (_currentIndex >= 0) {
        if ((millis() - _lastChange) >= _advanceTime) {
            // Advance to the next LED in sequence.
            _currentIndex = (_currentIndex + 1) % _numPins;
            _lastChange += _advanceTime;
            advance(previousPin(1), _pins[_currentIndex]);
        }
    } else {
        // First time - light the first LED.
        _currentIndex = 0;
        _lastChange = millis();
        advance(previousPin(1), _pins[_currentIndex]);
    }
}

/**
 * \fn unsigned long ChaseLEDs::advanceTime() const
 * \brief Returns the number of milliseconds that each LED will be
 * lit in the chase sequence.
 *
 * \sa setAdvanceTime(), advance()
 */

/**
 * \fn void ChaseLEDs::setAdvanceTime(unsigned long advanceTime)
 * \brief Sets the number of milliseconds to advance between LED's to
 * \a advanceTime.
 *
 * \sa advanceTime(), advance()
 */

/**
 * \brief Advances to the next LED in sequence, turning off \a prevPin,
 * and turning on \a nextPin.
 *
 * The default implementation is equivalent to the following code:
 *
 * \code
 * digitalWrite(prevPin, LOW);
 * digitalWrite(nextPin, HIGH);
 * \endcode
 *
 * This method may be overridden in subclasses to provide special effects.
 * See the documentation for previousPin() for some example effects.
 *
 * \sa previousPin()
 */
void ChaseLEDs::advance(uint8_t prevPin, uint8_t nextPin)
{
    digitalWrite(prevPin, LOW);
    digitalWrite(nextPin, HIGH);
}

/**
 * \fn uint8_t ChaseLEDs::previousPin(int n) const
 * \brief Returns the pin that is \a n steps back in the sequence.
 *
 * If \a n is zero, then the current pin is returned; if \a n is 1,
 * then the previous pin is returned; and so on.
 *
 * This function may be called by subclasses in their advance() method
 * to manipulate pins that are further back in the chase sequence than
 * the immediately previous pin.
 *
 * For example, the following code implements a LED chaser that lights
 * two pins at a time:
 *
 * \code
 * void DoubleChaser::advance(uint8_t prevPin, uint8_t nextPin)
 * {
 *      digitalWrite(previousPin(2), LOW);
 *      digitalWrite(prevPin, HIGH);
 *      digitalWrite(nextPin, HIGH);
 * }
 * \endcode
 *
 * As another exmaple, the following code uses PWM outputs to fade out
 * the previous pin rather than turn it off immediately:
 *
 * \code
 * void FadingChaser::advance(uint8_t prevPin, uint8_t nextPin)
 * {
 *      digitalWrite(previousPin(2), LOW);
 *      analogWrite(prevPin, 32);
 *      digitalWrite(nextPin, HIGH);
 * }
 * \endcode
 *
 * Note: it is possible to retrieve the \em following pin in sequence using
 * previousPin(-1).  This could be used to fade in the LED that follows
 * \a nextPin.
 *
 * \sa advance()
 */
