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

#include "Melody.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/**
 * \class Melody Melody.h <Melody.h>
 * \brief Plays a melody on a digital output pin using tone().
 *
 * The following example plays a simple tone three times on digital pin 8:
 *
 * \code
 * #include <Melody.h>
 *
 * int notes[] = {
 *     NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3,
 *     NOTE_REST, NOTE_B3, NOTE_C4, NOTE_REST
 * };
 * byte lengths[] = {4, 8, 8, 4, 4, 4, 4, 4, 2};
 *
 * Melody melody(8);
 *
 * void setup() {
 *     melody.setMelody(notes, lengths, sizeof(lengths));
 *     melody.setLoopCount(3);
 *     melody.play();
 * }
 *
 * void loop() {
 *     melody.run();
 * }
 * \endcode
 *
 * The \c notes array contains the frequency of the notes to be played,
 * with the special value \c NOTE_REST indicating a rest where no notes
 * are playing.  The \c lengths array contains the lengths of each of the
 * notes; a value of 4 indicates a quarter note, a value of 8 indicates
 * an eighth note, etc.
 *
 * The run() method must be called from the application's main
 * <tt>loop()</tt> method to ensure that the melody advances from
 * one note to the next.  It will not block the application while
 * notes are playing.
 *
 * The number of loops can also be specified with setLoopDuration() which
 * sets a maximum amount of time that the melody will play before stopping.
 * The following example plays the melody for no more than 60 seconds:
 *
 * \code
 * void setup() {
 *     melody.setMelody(notes, lengths, sizeof(lengths));
 *     melody.setLoopDuration(60000UL);
 *     melody.play();
 * }
 * \endcode
 */

/**
 * \brief Constructs a new melody playing object for \a pin.
 */
Melody::Melody(uint8_t pin)
    : _pin(pin)
    , playing(false)
    , _loopCount(0)
    , loopsLeft(0)
    , notes(0)
    , lengths(0)
    , size(0)
    , posn(0)
    , duration(0)
    , startNote(0)
{
}

/**
 * \fn bool Melody::isPlaying() const
 * \brief Returns true if the melody is currently playing; false if not.
 */

/**
 * \fn int Melody::loopCount() const
 * \brief Returns the number of times the melody should loop before stopping.
 *
 * The default value is zero, indicating that the melody will loop
 * indefinitely.
 *
 * \sa setLoopCount(), setLoopDuration(), play()
 */

/**
 * \fn void Melody::setLoopCount(int count)
 * \brief Sets the number of times the melody should loop to \a count.
 *
 * If \a count is zero, then the melody will loop indefinitely.
 *
 * \sa loopCount(), setLoopDuration()
 */

/**
 * \brief Sets the maximum number of loops to last no longer than \a ms milliseconds.
 *
 * This function must be called after the melody is specified with setMelody()
 * as it uses the length of the melody and \a ms to determine the loopCount().
 *
 * \sa loopCount(), setLoopCount()
 */
void Melody::setLoopDuration(unsigned long ms)
{
    unsigned long duration = 0;
    for (unsigned int index = 0; index < size; ++index)
        duration += (1000 / lengths[index]) * 13 / 10;
    _loopCount = (int)(ms / duration);
    if (!_loopCount)
        _loopCount = 1;     // Play the melody at least once.
}

/**
 * \brief Starts playing the melody, or restarts it if already playing.
 *
 * \sa playOnce(), setMelody(), stop(), loopCount()
 */
void Melody::play()
{
    stop();
    if (size == 0)
        return;         // No melody to play.
    loopsLeft = _loopCount;
    posn = 0;
    playing = true;
    nextNote();
}

/**
 * \brief Plays the melody once and then stops.
 *
 * \sa play(), stop()
 */
void Melody::playOnce()
{
    stop();
    if (size == 0)
        return;         // No melody to play.
    loopsLeft = 1;
    posn = 0;
    playing = true;
    nextNote();
}

/**
 * \brief Stops playing the melody.
 *
 * \sa play()
 */
void Melody::stop()
{
    if (!playing)
        return;
    playing = false;
    noTone(_pin);
}

/**
 * \brief Sets the melody to the \a size elements of \a notes and \a lengths.
 *
 * If a melody is currently playing, then this function will stop playback.
 *
 * The \a notes array contains the frequency of the notes to be played,
 * with the special value \c NOTE_REST indicating a rest where no notes
 * are playing.  The \a lengths array contains the lengths of each of the
 * notes; a value of 4 indicates a quarter note, a value of 8 indicates
 * an eighth note, etc.
 *
 * \sa play()
 */
void Melody::setMelody(const int *notes, const uint8_t *lengths, unsigned int size)
{
    stop();
    this->notes = notes;
    this->lengths = lengths;
    this->size = size;
}

/**
 * \brief Runs the melody control loop.
 *
 * This function must be called by the application's main <tt>loop()</tt>
 * function to cause the melody to advance from note to note.  It will not
 * block the application while notes are playing.
 */
void Melody::run()
{
    if (!playing)
        return;
    if ((millis() - startNote) >= duration) {
        noTone(_pin);
        nextNote();
    }
}

void Melody::nextNote()
{
    if (posn >= size) {
        if (loopsLeft != 0 && --loopsLeft <= 0) {
            stop();
            return;
        }
        posn = 0;
    }
    duration = 1000 / lengths[posn];
    if (notes[posn] != NOTE_REST)
        tone(_pin, notes[posn], duration);
    ++posn;
    duration = duration * 13 / 10;      // i.e., duration * 1.3
    startNote = millis();
}
