/*
 * Copyright (C) 2015 Southern Storm Software, Pty Ltd.
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

#include "RingOscillatorNoiseSource.h"
#include "Crypto.h"
#include "RNG.h"
#include <Arduino.h>

/**
 * \class RingOscillatorNoiseSource RingOscillatorNoiseSource.h <RingOscillatorNoiseSource.h>
 * \brief Processes the signal from a ring oscillator based noise source.
 *
 * This class processes input from a ring oscillator noise source, such as
 * that described \ref crypto_rng_ring "here".
 *
 * \note The output from a ring oscillator is not generally as good as a
 * "true" noise source.  The oscillation can easily settle into regular
 * patterns or sync up with other clock sources on the board.  It is even
 * possible to "hack" a ring oscillator by injecting chosen frequencies
 * on the power supply rails to force the oscillation into a predictable
 * waveform (see <a href="http://www.cl.cam.ac.uk/~atm26/papers/markettos-ches2009-inject-trng.pdf">this paper</a> for an example).
 * It is very important that the output of this class be whitened with
 * \link RNGClass RNG\endlink before it is used for cryptography and that
 * the device is isolated from attacker-controlled sources of power.
 * Unless you have a very good reason to use a ring oscillator,
 * TransistorNoiseSource is usually a better option.
 *
 * The noise is read from an input capture pin on the Arduino and stirred
 * into the random number pool on a regular basis.  The following pins are
 * used on different Arduino variants:
 *
 * <table>
 * <tr><td>Variant</td><td>Arduino Pin / AVR Pin</td><td>Timer</td></tr>
 * <tr><td>Arduino Uno</td><td>D8 / PB0</td><td>Timer 1</td></tr>
 * <tr><td>Arduino Leonardo</td><td>D4 / PD4</td><td>Timer 1</td></tr>
 * <tr><td>Arduino Mega or Mega 2560</td><td>D49 / PL0</td><td>Timer 4</td></tr>
 * </table>
 *
 * If your board is not pin-compatible with one of the above, then the
 * source for the RingOscillatorNoiseSource class will need to be modified
 * to use a different pin/timer combination.  Also, when the timer is in
 * use by this class it cannot be used for other application tasks.
 *
 * The example below shows how to initialize a ring oscillator based noise
 * source and use it with \link RNGClass RNG\endlink:
 *
 * \code
 * #include <Crypto.h>
 * #include <RNG.h>
 * #include <RingOscillatorNoiseSource.h>
 *
 * // Noise source to seed the random number generator.
 * RingOscillatorNoiseSource noise;
 *
 * void setup() {
 *     // Initialize the random number generator with the application tag
 *     // "MyApp 1.0" and load the previous seed from EEPROM address 500.
 *     RNG.begin("MyApp 1.0", 500);
 *
 *     // Add the noise source to the list of sources known to RNG.
 *     RNG.addNoiseSource(noise);
 *
 *     // ...
 * }
 *
 * void loop() {
 *     // ...
 *
 *     // Perform regular housekeeping on the random number generator.
 *     RNG.loop();
 *
 *     // ...
 * }
 * \endcode
 *
 * For more information, see the documentation for \link RNGClass RNG\endlink.
 *
 * \sa \link RNGClass RNG\endlink, NoiseSource, TransistorNoiseSource
 */

// Choose the input capture timer and pin to use for this board.
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
// Arduino Mega or Mega 2560 - input capture on TIMER4 and D49/PL0.
#define RING_TIMER      4
#define RING_PIN        49
#define RING_CAPT_vect  TIMER4_CAPT_vect
#define RING_ICR        ICR4
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
// Arduino Leonardo - input capture on Timer1 and D4/PD4.
#define RING_TIMER      1
#define RING_PIN        4
#define RING_CAPT_vect  TIMER1_CAPT_vect
#define RING_ICR        ICR1
#else
// Assuming Arduino Uno or equivalent - input capture on TIMER1 and D8/PB0.
#define RING_TIMER      1
#define RING_PIN        8
#define RING_CAPT_vect  TIMER1_CAPT_vect
#define RING_ICR        ICR1
#endif

// Calibration states.
#define NOISE_NOT_CALIBRATING   0
#define NOISE_CALIBRATING       1

// If there is no capture event for this many milliseconds,
// then assume that the oscillator is stopped or disconnected.
#define RING_DISCONNECT_TIME    200

RingOscillatorNoiseSource::RingOscillatorNoiseSource()
    : calState(NOISE_CALIBRATING)
    , lastSignal(millis())
{
    // Initialize the bit collection routines.
    restart();

    // Set up the capture pin as an input with no pull-ups.
    pinMode(RING_PIN, INPUT);
    digitalWrite(RING_PIN, LOW);

#if RING_TIMER == 1
    // Set up TIMER1 to perform input capture on PB8/D8.
    TCCR1B = 0;                 // Turn off TIMER1.
    TIMSK1 = 0;                 // Turn off TIMER1 interrupts.
    TCNT1 = 0;                  // Zero the timer.
    TCCR1A = 0;                 // Turn off output compare.
    TCCR1B |= (1 << ICES1);     // Input capture on rising edge.
    TIMSK1 |= (1 << ICIE1);     // Input capture interrupts enabled.

    // Start TIMER1 at the highest frequency with no prescaling.
    TCCR1B |= (1 << CS10);
#elif RING_TIMER == 4
    // Set up TIMER4 to perform input capture on PL0/D49.
    TCCR4B = 0;                 // Turn off TIMER4.
    TIMSK4 = 0;                 // Turn off TIMER4 interrupts.
    TCNT4 = 0;                  // Zero the timer.
    TCCR4A = 0;                 // Turn off output compare.
    TCCR4B |= (1 << ICES4);     // Input capture on rising edge.
    TIMSK4 |= (1 << ICIE4);     // Input capture interrupts enabled.

    // Start TIMER4 at the highest frequency with no prescaling.
    TCCR4B |= (1 << CS10);
#endif
}

RingOscillatorNoiseSource::~RingOscillatorNoiseSource()
{
    // Turn off the capture timer.
#if RING_TIMER == 1
    TCCR1B = 0;
#elif RING_TIMER == 4
    TCCR4B = 0;
#endif

    // Clean up.
    clean(buffer);
}

bool RingOscillatorNoiseSource::calibrating() const
{
    return calState == NOISE_CALIBRATING;
}

static uint16_t volatile out = 0;
static uint8_t volatile outBits = 0;

// Interrupt service routine for the timer's input capture interrupt.
ISR(RING_CAPT_vect)
{
    // We are interested in the jitter; that is the difference in
    // time between one rising edge and the next in the signal.
    // Extract a single bit from the jitter and add it to the
    // rolling "out" buffer for the main code to process later.
    // If the buffer overflows, we discard bits and keep going.
    static uint16_t prev = 0;
    uint16_t next = RING_ICR;
    out = (out << 1) | ((next - prev) & 1);
    prev = next;
    ++outBits;
}

void RingOscillatorNoiseSource::stir()
{
    // If the "out" buffer is full, then convert the bits.  Turn off
    // interrupts while we read the "out" buffer and reset "outBits".
    unsigned long now = millis();
    cli();
    if (outBits >= 16) {
        uint16_t bits = out;
        outBits = 0;
        sei();
        for (uint8_t index = 0; index < 8; ++index) {
            // Collect two bits of input and remove bias using the Von Neumann
            // method.  If both bits are the same, then discard both.
            // Otherwise choose one of the bits and output that one.
            // We have to do this carefully so that instruction timing does
            // not reveal the value of the bit that is chosen.
            if ((bits ^ (bits << 1)) & 0x8000) {
                // The bits are different: add the top-most to the buffer.
                if (posn < sizeof(buffer)) {
                    buffer[posn] = (buffer[posn] >> 1) |
                                   (((uint8_t)(bits >> 8)) & (uint8_t)0x80);
                    if (++bitNum >= 8) {
                        ++posn;
                        bitNum = 0;
                    }
                }
            }
            bits = bits << 2;
        }
    } else {
        // The "out" buffer isn't full yet.  Re-enable interrupts.
        sei();

        // If it has been too long since the last useful block,
        // then go back to calibrating.  The oscillator may be
        // stopped or disconnected.
        if (calState == NOISE_NOT_CALIBRATING) {
            if ((now - lastSignal) >= RING_DISCONNECT_TIME) {
                restart();
                calState = NOISE_CALIBRATING;
            }
        }
    }

    // If the buffer is full, then stir it into the random number pool.
    // We credit 1 bit of entropy for every 8 bits of output because
    // ring oscillators aren't quite as good as a true noise source.
    // We have to collect a lot more data to get something random enough.
    if (posn >= sizeof(buffer)) {
        output(buffer, posn, posn);
        restart();
        calState = NOISE_NOT_CALIBRATING;
        lastSignal = now;
    }
}

void RingOscillatorNoiseSource::restart()
{
    clean(buffer);
    prevBit = 0;
    posn = 0;
    bitNum = 0;
}
