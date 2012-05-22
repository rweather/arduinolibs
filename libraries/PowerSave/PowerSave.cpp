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

#include "PowerSave.h"
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>

/**
 * \defgroup power_save Power saving utility functions
 *
 * The functions in this module assist with reducing power consumption on
 * Arduino boards by disabling features that are not used or putting the
 * device to sleep when it is inactive.
 */
/*\@{*/

/**
 * \fn void unusedPin(uint8_t pin)
 * \brief Marks an I/O \a pin as unused.
 * \ingroup power_save
 *
 * This function sets \a pin to be an input with pullups enabled, which will
 * reduce power consumption compared to pins that are left floating.
 */

/** @cond */
ISR(WDT_vect)
{
    wdt_disable();
}
/** @endcond */

/**
 * \enum SleepDuration
 * \brief Duration to put the CPU to sleep with sleepFor().
 * \ingroup power_save
 *
 * \sa sleepFor()
 */

/**
 * \var SLEEP_15_MS
 * \brief Sleep for 15 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_30_MS
 * \brief Sleep for 30 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_60_MS
 * \brief Sleep for 60 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_120_MS
 * \brief Sleep for 120 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_250_MS
 * \brief Sleep for 250 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_500_MS
 * \brief Sleep for 500 milliseconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_1_SEC
 * \brief Sleep for 1 second.
 * \ingroup power_save
 */

/**
 * \var SLEEP_2_SEC
 * \brief Sleep for 2 seconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_4_SEC
 * \brief Sleep for 4 seconds.
 * \ingroup power_save
 */

/**
 * \var SLEEP_8_SEC
 * \brief Sleep for 8 seconds.
 * \ingroup power_save
 */

/**
 * \brief Puts the CPU to sleep for a specific \a duration.
 * \ingroup power_save
 *
 * The analog to digital converter and the brown out detector will
 * be disabled during sleep mode.
 *
 * The \a mode parameter indicates the mode to use when the device is
 * sleeping.  The default is SLEEP_MODE_IDLE.
 */
void sleepFor(SleepDuration duration, uint8_t mode)
{
    // Turn off the analog to digital converter.
    ADCSRA &= ~(1 << ADEN);
    power_adc_disable();

    // Turn on the watchdog timer for the desired duration.
    wdt_enable(duration);
    WDTCSR |= (1 << WDIE);

    // Put the device to sleep, including turning off the Brown Out Detector.
    set_sleep_mode(mode);
    cli();
    sleep_enable();
#if defined(sleep_bod_disable)
    sleep_bod_disable();
#endif
    sei();
    sleep_cpu();
    sleep_disable();
    sei();

    // Turn the analog to digital converter back on.
    power_adc_enable();
    ADCSRA |= (1 << ADEN);
}

/*\@}*/
