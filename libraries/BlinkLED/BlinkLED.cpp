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

#include "BlinkLED.h"
#include <WProgram.h>

/**
 * \class BlinkLED BlinkLED.h <BlinkLED.h>
 * \brief Blink a LED on a digital output pin.
 *
 * BlinkLED simplies the process of blinking a LED by encapsulating the
 * control logic into a single class.  The following example strobes the
 * status LED on D13 with a period of 70 milliseconds on, 930 milliseconds off
 * (the LED is initially off):
 *
 * \code
 * #include <BlinkLED.h>
 *
 * BlinkLED statusBlink(13, 70, 930);
 *
 * void setup() {}
 *
 * void loop() {
 *     statusBlink.loop();
 * }
 * \endcode
 *
 * The current state() of the LED can be changed immediately by calling
 * setState().  The blink rate can be modified with setBlinkRate().
 * And the blink cycle can be suspended and restarted with pause()
 * and resume().
 */

/**
 * \brief Initialize a blinking LED on the specified \a pin.
 *
 * The LED will blink with a rate defined by \a onTime and \a offTime
 * (in milliseconds).  Initially the LED's state is given by \a initialState,
 * where true means initially on and false means initially off.
 */
BlinkLED::BlinkLED(uint8_t pin, unsigned long onTime, unsigned long offTime, bool initialState)
    : _pin(pin)
    , _state(initialState)
    , _paused(false)
    , _onTime(onTime)
    , _offTime(offTime)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, initialState ? HIGH : LOW);
    _lastChange = millis();
}

/**
 * Perform a single iteration of the blink loop for this LED.
 */
void BlinkLED::loop()
{
    if (_paused)
        return;
    unsigned long currentTime = millis();
    if (_state) {
        if ((currentTime - _lastChange) >= _onTime) {
            digitalWrite(_pin, LOW);
            _lastChange += _onTime;
            _state = false;
        }
    } else {
        if ((currentTime - _lastChange) >= _offTime) {
            digitalWrite(_pin, HIGH);
            _lastChange += _offTime;
            _state = true;
        }
    }
}

/**
 * \fn unsigned long BlinkLED::onTime() const
 * \brief Returns the number of milliseconds the LED will be on.
 *
 * \sa offTime(), setBlinkRate()
 */

/**
 * \fn unsigned long BlinkLED::offTime() const
 * \brief Returns the number of milliseconds the LED will be off.
 *
 * \sa onTime(), setBlinkRate()
 */

/**
 * \brief Sets the \a onTime and \a offTime (in milliseconds).
 *
 * The change takes effect immediately.  If the current onTime() or
 * offTime() has now expired, then the LED will immediately switch to
 * the opposite state().
 *
 * \sa onTime(), offTime()
 */
void BlinkLED::setBlinkRate(unsigned long onTime, unsigned long offTime)
{
    _onTime = onTime;
    _offTime = offTime;
}

/**
 * \fn bool BlinkLED::state() const
 * \brief Returns the current state of the LED; true is on, false is off.
 *
 * \sa setState()
 */

/**
 * \brief Sets the current \a state of the LED, where true is on, false is off.
 *
 * If the LED is already set to \a state, then it will complete its current
 * cycle of onTime() or offTime().  Otherwise the LED is immediately set to
 * \a state and a new cycle begins.
 *
 * \sa state()
 */

void BlinkLED::setState(bool state)
{
    if (_state != state) {
        digitalWrite(_pin, state ? HIGH : LOW);
        _state = state;
        _lastChange = millis();
    }
}

/**
 * \fn void BlinkLED::pause()
 * \brief Pauses the LED blink cycle in its current state().
 *
 * \sa resume(), isPaused()
 */

/**
 * \brief Resumes the LED blink cycle after a pause().
 *
 * The LED will complete its current onTime() or offTime() and then
 * will switch to the opposite state().  If onTime() or offTime() has
 * already expired, then the LED will immediately switch state.
 *
 * \sa pause(), isPaused()
 */
void BlinkLED::resume()
{
    if (_paused) {
        _paused = false;
        unsigned long currentTime = millis();
        if (_state) {
            if ((currentTime - _lastChange) >= _onTime) {
                digitalWrite(_pin, LOW);
                _lastChange = currentTime;
                _state = false;
            }
        } else {
            if ((currentTime - _lastChange) >= _offTime) {
                digitalWrite(_pin, HIGH);
                _lastChange = currentTime;
                _state = true;
            }
        }
    }
}

/**
 * \fn bool BlinkLED::isPaused()
 * \brief Returns true if the LED blink cycle is paused; false otherwise.
 *
 * \sa pause(), resume()
 */
