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

#include <Arduino.h>
#include <stdio.h>
#include <time.h>

void setup();
void loop();

SerialClass Serial;

void SerialClass::begin(int baud)
{
}

void SerialClass::print(const char *str)
{
    printf("%s", str);
}

void SerialClass::print(uint8_t value)
{
    printf("%d", value);
}

void SerialClass::print(uint8_t value, int base)
{
    if (base == HEX)
        printf("%x", value);
    else
        printf("%d", value);
}

void SerialClass::print(long value)
{
    printf("%ld", value);
}

void SerialClass::print(long value, int base)
{
    if (base == HEX)
        printf("%lx", value);
    else
        printf("%ld", value);
}

void SerialClass::print(unsigned long value)
{
    printf("%lu", value);
}

void SerialClass::print(unsigned long value, int base)
{
    if (base == HEX)
        printf("%lx", value);
    else
        printf("%lu", value);
}

void SerialClass::print(double value)
{
    printf("%f", value);
}

void SerialClass::print(char ch)
{
    putc(ch, stdout);
}

void SerialClass::println(const char *str)
{
    printf("%s\n", str);
}

void SerialClass::println(uint8_t value)
{
    printf("%d\n", value);
}

void SerialClass::println(uint8_t value, int base)
{
    if (base == HEX)
        printf("%x\n", value);
    else
        printf("%d\n", value);
}

void SerialClass::println(long value)
{
    printf("%ld\n", value);
}

void SerialClass::println(long value, int base)
{
    if (base == HEX)
        printf("%lx\n", value);
    else
        printf("%ld\n", value);
}

void SerialClass::println(unsigned long value)
{
    printf("%lu\n", value);
}

void SerialClass::println(unsigned long value, int base)
{
    if (base == HEX)
        printf("%lx\n", value);
    else
        printf("%lu\n", value);
}

void SerialClass::println(double value)
{
    printf("%f\n", value);
}

void SerialClass::println(char ch)
{
    putc(ch, stdout);
    putc('\n', stdout);
}

void SerialClass::println()
{
    printf("\n");
}

void SerialClass::flush()
{
    fflush(stdout);
}

unsigned long micros()
{
    struct timespec tv;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv);
    return tv.tv_sec * 1000000UL + tv.tv_nsec / 1000UL;
}

unsigned long millis()
{
    struct timespec tv;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tv);
    return tv.tv_sec * 1000UL + tv.tv_nsec / 1000000UL;
}

int main(int argc, char *argv[])
{
    // At the moment, just run the setup() function.
    setup();
    return 0;
}
