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

#include "Charlieplex.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
#include <stdlib.h>
#include <string.h>

/**
 * \class Charlieplex Charlieplex.h <Charlieplex.h>
 * \brief Manage an array of LED's in a charlieplexed arrangement.
 *
 * <a href="http://en.wikipedia.org/wiki/Charlieplexing">Charlieplexing</a>
 * is a technique for multiplexing large numbers of LED's on a small
 * number of microcontroller output pins.  LED's are arranged in
 * complementary pairs; the simplest being for two output pins:
 *
 * \image html charlieplex2pin.png
 *
 * When Pin1 is 1 and Pin2 is 0, LED1 will be lit.  When Pin1 is 0 and
 * Pin2 is 1, then LED2 will be lit.  The technique extends to 3 pins
 * as follows:
 *
 * \image html charlieplex3pin.png
 *
 * In this case, LED5 is lit when Pin1 is 1, Pin3 is 0, and Pin2 is set
 * to a high-impedance input to "disconnect" it.
 *
 * Charlieplex presents a simple array of led() values that indicate whether
 * each LED is on, off, or in an intermediate PWM state (if setPwmLed()
 * is used).  The application must call loop() or refresh() on a regular
 * basis to ensure that the multiplexed display is kept up to date.
 * The following example drives 6 LED's connected to the output pins
 * D9, D10, and D11:
 *
 * \dontinclude BlinkLED/examples/Charlieplex/Charlieplex.ino
 * \skip #include
 * \until charlie.loop
 * \until }
 *
 * The following diagram extends the circuit for 4 output pins and 12 LED's:
 *
 * \image html charlieplex4pin.png
 *
 * The following diagram extends the circuit for 5 output pins and 20 LED's:
 *
 * \image html charlieplex5pin.png
 *
 * Circuits for higher numbers of LED's get increasingly complex.  For those
 * cases it can be easier to use traditional multiplexing matrix arrangements
 * and shift registers.  The DMD class does this for a specific kind of
 * large dot matrix display.  Otherwise, use the following pseudocode to
 * determine how to connect the LED's for higher numbers of pins:
 *
 * \code
 * n = 1
 * for Pass = 1 to NumPins-1:
 *     for Pin = 1 to NumPins-Pass:
 *         LED[n] is connected between Pin (anode) and Pin+Pass (cathode)
 *         LED[n+1] is connected between Pin+Pass (anode) and Pin (cathode)
 *         n = n + 2
 * \endcode
 *
 * Note: while the above circuit diagrams and psuedocode use 1-based
 * numbering for LED's, Charlieplex uses 0-based numbering in the led(),
 * setLed(), pwmLed(), and setPwmLed() functions.
 *
 * It isn't necessary to wire up all LED's.  If you only need 10 LED's,
 * then use the 4-output circuit and omit LED11 and LED12.  Charlieplex
 * only drives LED's that are lit; LED's that are unlit or unused will
 * be skipped during the refresh scan.  The maximum number of LED's that
 * that can be driven by a specific number of pins is given by the
 * following table:
 *
 * <table>
 * <tr><td>Number of Pins</td><td>Number of LED's</td></tr>
 * <tr><td>2</td><td>2</td></tr>
 * <tr><td>3</td><td>6</td></tr>
 * <tr><td>4</td><td>12</td></tr>
 * <tr><td>5</td><td>20</td></tr>
 * <tr><td>6</td><td>30</td></tr>
 * <tr><td>7</td><td>42</td></tr>
 * <tr><td>8</td><td>56</td></tr>
 * <tr><td>9</td><td>72</td></tr>
 * <tr><td>10</td><td>90</td></tr>
 * <tr><td>n</td><td>n * (n - 1)</td></tr>
 * </table>
 */

/**
 * \brief Constructs a new charliexplexing array where the output pins
 * are specified by the \a numPins entries in \a pins.
 *
 * Note: \a numPins must be 2 or greater for correct operation.
 *
 * \sa count(), setLed()
 */
Charlieplex::Charlieplex(const uint8_t *pins, uint8_t numPins)
    : _count(((int)numPins) * (numPins - 1))
    , _lastTime(micros())
    , _currentIndex(-1)
    , _pwmPhase(0xC0)
{
    // Determine the best hold time for 50 Hz refresh when all LED's
    // are lit.  Divide it again by 4 (to get 200 Hz) to manage the
    // simulated PWM in refresh().
    _holdTime = 20000 / _count / 4;

    // Allocate the pin arrays and populate them.  Doing this now makes
    // refresh() more efficient later, at the expense of some memory.
    _pins1 = (uint8_t *)malloc(_count);
    _pins2 = (uint8_t *)malloc(_count);
    int n = 0;
    for (uint8_t pass = 1; pass < numPins; ++pass) {
        for (uint8_t pin = 0; pin < (numPins - pass); ++pin) {
            _pins1[n] = _pins2[n + 1] = pins[pin];
            _pins2[n] = _pins1[n + 1] = pins[pin + pass];
            n += 2;
        }
    }

    // Allocate space for the LED value array and zero it.
    _values = (uint8_t *)malloc(_count);
    memset(_values, 0, _count);

    // Start with all pins configured as floating inputs (all LED's off).
    for (uint8_t pin = 0; pin < numPins; ++pin) {
        digitalWrite(pins[pin], LOW);
        pinMode(pins[pin], INPUT);
    }
}

/**
 * \brief Destroys this charlieplexed array.
 */
Charlieplex::~Charlieplex()
{
    free(_pins1);
    free(_pins2);
    free(_values);
}

/**
 * \fn int Charlieplex::count() const
 * \brief Returns the number of LED's in this charlieplexed array based
 * on the number of pins.
 *
 * <table>
 * <tr><td>Number of Pins</td><td>Number of LED's</td></tr>
 * <tr><td>2</td><td>2</td></tr>
 * <tr><td>3</td><td>6</td></tr>
 * <tr><td>4</td><td>12</td></tr>
 * <tr><td>5</td><td>20</td></tr>
 * <tr><td>6</td><td>30</td></tr>
 * <tr><td>7</td><td>42</td></tr>
 * <tr><td>8</td><td>56</td></tr>
 * <tr><td>9</td><td>72</td></tr>
 * <tr><td>10</td><td>90</td></tr>
 * <tr><td>n</td><td>n * (n - 1)</td></tr>
 * </table>
 *
 * \sa led()
 */

/**
 * \fn bool Charlieplex::led(int index) const
 * \brief Returns the value of the LED at \a index in the charplexed array;
 * true if lit; false if not lit.
 *
 * If the LED is displaying a PWM value, then this function will return
 * true for any non-zero PWM value.
 *
 * \sa setLed(), pwmLed()
 */

/**
 * \fn void Charlieplex::setLed(int index, bool value)
 * \brief Sets the \a value of the LED at \a index in the charliplexed array.
 *
 * The brightness of the LED will be proportional to the number of LED's
 * that are currently lit, as the holdTime() refresh rate will cause the
 * LED to appear to dim; the more LED's that are lit the less overall time
 * each individual LED is held on.  For best results, only a single LED should
 * be lit at once or higher-brightness LED's should be used.
 *
 * \sa led(), setPwmLed()
 */

/**
 * \fn uint8_t Charlieplex::pwmLed(int index) const
 * \brief Returns the PWM value of the LED at \a index in the charplexed array;
 * between 0 and 255.
 *
 * \sa setPwmLed(), led()
 */

/**
 * \fn void Charlieplex::setPwmLed(int index, uint8_t value)
 * \brief Sets the PWM \a value of the LED at \a index in the charliplexed
 * array; between 0 and 255.
 *
 * If this function is used, then it is assumed that the output pins are
 * capable of PWM output.
 *
 * The PWM-specified brightness of the LED will also be affected to the
 * number of LED's that are currently lit, as the holdTime() refresh rate
 * will cause the LED to appear to dim; the more LED's that are lit the
 * less overall time each individual LED is held on.  For best results,
 * only a single LED should be lit at once or higher-brightness LED's
 * should be used.
 *
 * \sa pwmLed(), setLed()
 */

/**
 * \fn unsigned long Charlieplex::holdTime() const
 * \brief Returns the number of microseconds that each LED should be
 * held on for before moving onto the next in loop().
 *
 * The default value is calculated so that all LED's can be refreshed
 * with a rate of at least 200 Hz, which is necessary for handling PWM
 * output on multiple LED's.  The less LED's that are lit at once,
 * the faster the display will refresh.
 *
 * \sa setHoldTime(), loop()
 */

/**
 * \fn void Charlieplex::setHoldTime(unsigned long us)
 * \brief Sets the number of microseconds that each LED should be
 * held on for before moving onto the next in loop() to \a us.
 *
 * \sa holdTime(), loop()
 */

/**
 * \brief Runs the multiplexing loop, to display the LED states on
 * the charlieplexed array.
 *
 * If holdTime() microseconds have elapsed since the last call to loop(),
 * then the current LED is turned off and the next LED that needs to be
 * lit is turned on.
 *
 * LED's that do not need to be lit are skipped.  The total time for a
 * single pass through all lit LED's may be very short if only a few
 * LED's are lit at once.  If all LED's are lit, then the total time for
 * a single pass will be count() * holdTime() microseconds.
 *
 * If the application is using timer interrupts to drive the multiplexing
 * process, then use refresh() instead of loop().
 *
 * \sa led(), pwmLed(), holdTime(), refresh()
 */
void Charlieplex::loop()
{
    unsigned long us = micros();
    if ((us - _lastTime) >= _holdTime) {
        _lastTime = us;
        refresh();
    }
}

/**
 * \brief Refreshes the charlieplexed array by advancing to the next LED
 * that needs to be lit.
 *
 * This function is intended to be called from a timer interrupt service
 * routine to advance the multiplexing state without the main application
 * having to explicitly call loop().
 *
 * \sa loop()
 */
void Charlieplex::refresh()
{
    // Find the next LED to be lit.
    int prevIndex = _currentIndex;
    int limit = _count;
    while (limit >= 0) {
        _currentIndex = (_currentIndex + 1) % _count;
        if (_values[_currentIndex] != 0)
            break;
        --limit;
    }
    if (limit < 0) {
        // No LED's are lit.  Turn off the previous LED and exit.
        if (prevIndex != -1) {
            digitalWrite(_pins1[prevIndex], LOW);
            digitalWrite(_pins2[prevIndex], LOW);
            pinMode(_pins1[prevIndex], INPUT);
            pinMode(_pins2[prevIndex], INPUT);
        }
        _currentIndex = -1;
        return;
    }

    // Light the current LED.
    uint8_t value = _values[_currentIndex];
    uint8_t pin1 = _pins1[_currentIndex];
    uint8_t pin2 = _pins2[_currentIndex];
    _pwmPhase += 0x40;
    if (prevIndex != _currentIndex) {
        // Turn off the previous LED.
        if (prevIndex != -1) {
            digitalWrite(_pins1[prevIndex], LOW);
            digitalWrite(_pins2[prevIndex], LOW);
            pinMode(_pins1[prevIndex], INPUT);
            pinMode(_pins2[prevIndex], INPUT);
        }

        // We simulate PWM using a phase counter because analogWrite()
        // combined with holdTime() causes too much flickering if more
        // than one LED is lit.  This reduces the PWM resolution to 1 in 4.
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
        if (value > _pwmPhase)
            digitalWrite(pin1, HIGH);
        else
            digitalWrite(pin1, LOW);
    } else {
        // Same LED as previous.  Since there is only a single LED
        // that is lit, we can use analogWrite() to set the PWM state.
        if (value == 255)
            digitalWrite(pin1, HIGH);
        else
            analogWrite(pin1, value);
    }
}
