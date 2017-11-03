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

// Special-purpose compiler that generates the AVR version of Speck*.

#include <stdio.h>
#include <stdarg.h>

static int indent = 4;

static int t1_reg = 8;      // Temporary 64-bit value (any reg).
static int t2_reg = 16;     // Temporary 64-bit value (any reg).

static int x_reg = 26;
//static int y_reg = 28;
static int z_reg = 30;

static int const_reg = 24;  // For temporary constants (must be a high reg).

static int temp_reg = 25;   // Spare temporary register.

// Information about a set of registers storing a 64-bit quantity.
typedef struct
{
    int first;              // First register in the set.
    int offset;             // Offset for multiple of 8 rotations.

} Reg64;

// Indent the code and print a string.
void indent_printf(const char *format, ...)
{
    va_list va;
    int posn;
    va_start(va, format);
    for (posn = 0; posn < indent; ++posn)
        putc(' ', stdout);
    vfprintf(stdout, format, va);
    va_end(va);
}

// Print an assembler instruction within quotes.
void insn_printf(const char *format, ...)
{
    va_list va;
    int posn;
    va_start(va, format);
    for (posn = 0; posn < indent; ++posn)
        putc(' ', stdout);
    putc('"', stdout);
    vfprintf(stdout, format, va);
    putc('\\', stdout);
    putc('n', stdout);
    putc('"', stdout);
    putc('\n', stdout);
    va_end(va);
}

#define REGn(reg, n)    ((reg)->first + ((n) + (reg)->offset) % 8)

void leftRotate1(const Reg64 *reg)
{
    insn_printf("lsl r%d", REGn(reg, 0));
    insn_printf("rol r%d", REGn(reg, 1));
    insn_printf("rol r%d", REGn(reg, 2));
    insn_printf("rol r%d", REGn(reg, 3));
    insn_printf("rol r%d", REGn(reg, 4));
    insn_printf("rol r%d", REGn(reg, 5));
    insn_printf("rol r%d", REGn(reg, 6));
    insn_printf("rol r%d", REGn(reg, 7));
    insn_printf("adc r%d, __zero_reg__", REGn(reg, 0));
}

void leftRotate3(const Reg64 *reg)
{
    leftRotate1(reg);
    leftRotate1(reg);
    leftRotate1(reg);
}

void leftRotate8(Reg64 *reg)
{
    reg->offset = (reg->offset + 7) % 8;
}

void rightRotate1(const Reg64 *reg)
{
    insn_printf("bst r%d,0", REGn(reg, 0));
    insn_printf("ror r%d", REGn(reg, 7));
    insn_printf("ror r%d", REGn(reg, 6));
    insn_printf("ror r%d", REGn(reg, 5));
    insn_printf("ror r%d", REGn(reg, 4));
    insn_printf("ror r%d", REGn(reg, 3));
    insn_printf("ror r%d", REGn(reg, 2));
    insn_printf("ror r%d", REGn(reg, 1));
    insn_printf("ror r%d", REGn(reg, 0));
    insn_printf("bld r%d,7", REGn(reg, 7));
}

void rightRotate3(const Reg64 *reg)
{
    rightRotate1(reg);
    rightRotate1(reg);
    rightRotate1(reg);
}

void rightRotate8(Reg64 *reg)
{
    reg->offset = (reg->offset + 1) % 8;
}

void add64(const Reg64 *dst, const Reg64 *src)
{
    insn_printf("add r%d,r%d", REGn(dst, 0), REGn(src, 0));
    insn_printf("adc r%d,r%d", REGn(dst, 1), REGn(src, 1));
    insn_printf("adc r%d,r%d", REGn(dst, 2), REGn(src, 2));
    insn_printf("adc r%d,r%d", REGn(dst, 3), REGn(src, 3));
    insn_printf("adc r%d,r%d", REGn(dst, 4), REGn(src, 4));
    insn_printf("adc r%d,r%d", REGn(dst, 5), REGn(src, 5));
    insn_printf("adc r%d,r%d", REGn(dst, 6), REGn(src, 6));
    insn_printf("adc r%d,r%d", REGn(dst, 7), REGn(src, 7));
}

void sub64(const Reg64 *dst, const Reg64 *src)
{
    insn_printf("sub r%d,r%d", REGn(dst, 0), REGn(src, 0));
    insn_printf("sbc r%d,r%d", REGn(dst, 1), REGn(src, 1));
    insn_printf("sbc r%d,r%d", REGn(dst, 2), REGn(src, 2));
    insn_printf("sbc r%d,r%d", REGn(dst, 3), REGn(src, 3));
    insn_printf("sbc r%d,r%d", REGn(dst, 4), REGn(src, 4));
    insn_printf("sbc r%d,r%d", REGn(dst, 5), REGn(src, 5));
    insn_printf("sbc r%d,r%d", REGn(dst, 6), REGn(src, 6));
    insn_printf("sbc r%d,r%d", REGn(dst, 7), REGn(src, 7));
}

void eor64(const Reg64 *dst, const Reg64 *src)
{
    insn_printf("eor r%d,r%d", REGn(dst, 0), REGn(src, 0));
    insn_printf("eor r%d,r%d", REGn(dst, 1), REGn(src, 1));
    insn_printf("eor r%d,r%d", REGn(dst, 2), REGn(src, 2));
    insn_printf("eor r%d,r%d", REGn(dst, 3), REGn(src, 3));
    insn_printf("eor r%d,r%d", REGn(dst, 4), REGn(src, 4));
    insn_printf("eor r%d,r%d", REGn(dst, 5), REGn(src, 5));
    insn_printf("eor r%d,r%d", REGn(dst, 6), REGn(src, 6));
    insn_printf("eor r%d,r%d", REGn(dst, 7), REGn(src, 7));
}

void eor64Schedule(Reg64 *reg)
{
    // XOR with the schedule.
    insn_printf("ld __tmp_reg__,Z+");
    insn_printf("eor __tmp_reg__,r%d", REGn(reg, 0));
    insn_printf("ld r%d,Z+", REGn(reg, 0));
    insn_printf("eor r%d,r%d", REGn(reg, 0), REGn(reg, 1));
    insn_printf("ld r%d,Z+", REGn(reg, 1));
    insn_printf("eor r%d,r%d", REGn(reg, 1), REGn(reg, 2));
    insn_printf("ld r%d,Z+", REGn(reg, 2));
    insn_printf("eor r%d,r%d", REGn(reg, 2), REGn(reg, 3));
    insn_printf("ld r%d,Z+", REGn(reg, 3));
    insn_printf("eor r%d,r%d", REGn(reg, 3), REGn(reg, 4));
    insn_printf("ld r%d,Z+", REGn(reg, 4));
    insn_printf("eor r%d,r%d", REGn(reg, 4), REGn(reg, 5));
    insn_printf("ld r%d,Z+", REGn(reg, 5));
    insn_printf("eor r%d,r%d", REGn(reg, 5), REGn(reg, 6));
    insn_printf("ld r%d,Z+", REGn(reg, 6));
    insn_printf("eor r%d,r%d", REGn(reg, 6), REGn(reg, 7));
    insn_printf("mov r%d,__tmp_reg__", REGn(reg, 7));

    // The above operations also implicitly perform a right-rotation.
    // Undo it by left-shifting back into the correct position.
    leftRotate8(reg);
}

void eor64ScheduleReversePtr(Reg64 *reg, const char *ptrReg)
{
    // XOR with the schedule.
    insn_printf("ld __tmp_reg__,-%s", ptrReg);
    insn_printf("eor __tmp_reg__,r%d", REGn(reg, 7));
    insn_printf("ld r%d,-%s", REGn(reg, 7), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 7), REGn(reg, 6));
    insn_printf("ld r%d,-%s", REGn(reg, 6), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 6), REGn(reg, 5));
    insn_printf("ld r%d,-%s", REGn(reg, 5), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 5), REGn(reg, 4));
    insn_printf("ld r%d,-%s", REGn(reg, 4), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 4), REGn(reg, 3));
    insn_printf("ld r%d,-%s", REGn(reg, 3), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 3), REGn(reg, 2));
    insn_printf("ld r%d,-%s", REGn(reg, 2), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 2), REGn(reg, 1));
    insn_printf("ld r%d,-%s", REGn(reg, 1), ptrReg);
    insn_printf("eor r%d,r%d", REGn(reg, 1), REGn(reg, 0));
    insn_printf("mov r%d,__tmp_reg__", REGn(reg, 0));

    // The above operations also implicitly perform a left-rotation.
    // Undo it by right-shifting back into the correct position.
    // We have to do this twice because the following step will be
    // apply a left-rotation to put everything back where it belongs.
    rightRotate8(reg);
    rightRotate8(reg);
}

void eor64ScheduleReverse(Reg64 *reg)
{
    eor64ScheduleReversePtr(reg, "Z");
}

void eor64ScheduleReverseX(Reg64 *reg)
{
    eor64ScheduleReversePtr(reg, "X");
}

// Unpack the input block and convert from big-endian to little-endian.
static void unpack_input(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    insn_printf("ld r%d,X+", REGn(&xreg, 7));
    insn_printf("ld r%d,X+", REGn(&xreg, 6));
    insn_printf("ld r%d,X+", REGn(&xreg, 5));
    insn_printf("ld r%d,X+", REGn(&xreg, 4));
    insn_printf("ld r%d,X+", REGn(&xreg, 3));
    insn_printf("ld r%d,X+", REGn(&xreg, 2));
    insn_printf("ld r%d,X+", REGn(&xreg, 1));
    insn_printf("ld r%d,X+", REGn(&xreg, 0));

    insn_printf("ld r%d,X+", REGn(&yreg, 7));
    insn_printf("ld r%d,X+", REGn(&yreg, 6));
    insn_printf("ld r%d,X+", REGn(&yreg, 5));
    insn_printf("ld r%d,X+", REGn(&yreg, 4));
    insn_printf("ld r%d,X+", REGn(&yreg, 3));
    insn_printf("ld r%d,X+", REGn(&yreg, 2));
    insn_printf("ld r%d,X+", REGn(&yreg, 1));
    insn_printf("ld r%d,X", REGn(&yreg, 0));
}

static void load_from_x(Reg64 *reg)
{
    insn_printf("ld r%d,X+", REGn(reg, 0));
    insn_printf("ld r%d,X+", REGn(reg, 1));
    insn_printf("ld r%d,X+", REGn(reg, 2));
    insn_printf("ld r%d,X+", REGn(reg, 3));
    insn_printf("ld r%d,X+", REGn(reg, 4));
    insn_printf("ld r%d,X+", REGn(reg, 5));
    insn_printf("ld r%d,X+", REGn(reg, 6));
    insn_printf("ld r%d,X+", REGn(reg, 7));
}

static void store_to_x(Reg64 *reg)
{
    insn_printf("st X+,r%d", REGn(reg, 0));
    insn_printf("st X+,r%d", REGn(reg, 1));
    insn_printf("st X+,r%d", REGn(reg, 2));
    insn_printf("st X+,r%d", REGn(reg, 3));
    insn_printf("st X+,r%d", REGn(reg, 4));
    insn_printf("st X+,r%d", REGn(reg, 5));
    insn_printf("st X+,r%d", REGn(reg, 6));
    insn_printf("st X+,r%d", REGn(reg, 7));
}

static void load_from_z(Reg64 *reg)
{
    insn_printf("ld r%d,Z+", REGn(reg, 0));
    insn_printf("ld r%d,Z+", REGn(reg, 1));
    insn_printf("ld r%d,Z+", REGn(reg, 2));
    insn_printf("ld r%d,Z+", REGn(reg, 3));
    insn_printf("ld r%d,Z+", REGn(reg, 4));
    insn_printf("ld r%d,Z+", REGn(reg, 5));
    insn_printf("ld r%d,Z+", REGn(reg, 6));
    insn_printf("ld r%d,Z+", REGn(reg, 7));
}

static void store_to_z(Reg64 *reg)
{
    insn_printf("st Z+,r%d", REGn(reg, 0));
    insn_printf("st Z+,r%d", REGn(reg, 1));
    insn_printf("st Z+,r%d", REGn(reg, 2));
    insn_printf("st Z+,r%d", REGn(reg, 3));
    insn_printf("st Z+,r%d", REGn(reg, 4));
    insn_printf("st Z+,r%d", REGn(reg, 5));
    insn_printf("st Z+,r%d", REGn(reg, 6));
    insn_printf("st Z+,r%d", REGn(reg, 7));
}

static void push64(Reg64 *reg)
{
    reg->offset = 0;
    insn_printf("push r%d", REGn(reg, 0));
    insn_printf("push r%d", REGn(reg, 1));
    insn_printf("push r%d", REGn(reg, 2));
    insn_printf("push r%d", REGn(reg, 3));
    insn_printf("push r%d", REGn(reg, 4));
    insn_printf("push r%d", REGn(reg, 5));
    insn_printf("push r%d", REGn(reg, 6));
    insn_printf("push r%d", REGn(reg, 7));
}

static void pop64(Reg64 *reg)
{
    reg->offset = 0;
    insn_printf("pop r%d", REGn(reg, 7));
    insn_printf("pop r%d", REGn(reg, 6));
    insn_printf("pop r%d", REGn(reg, 5));
    insn_printf("pop r%d", REGn(reg, 4));
    insn_printf("pop r%d", REGn(reg, 3));
    insn_printf("pop r%d", REGn(reg, 2));
    insn_printf("pop r%d", REGn(reg, 1));
    insn_printf("pop r%d", REGn(reg, 0));
}

// Main loop for Speck::encryptBlock().
static void full_enc_main_loop(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    // Top of the main loop.
    insn_printf("1:");

    // x = (rightRotate8_64(x) + y) ^ *s++;
    rightRotate8(&xreg);
    add64(&xreg, &yreg);
    eor64Schedule(&xreg);

    // y = leftRotate3_64(y) ^ x;
    leftRotate3(&yreg);
    eor64(&yreg, &xreg);

    // Bottom of the main loop.
    insn_printf("dec %%2");
    insn_printf("breq 2f");
    insn_printf("rjmp 1b");
    insn_printf("2:");
}

// Main loop for Speck::decryptBlock().
static void full_dec_main_loop(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    // Top of the main loop.
    insn_printf("1:");

    // y = rightRotate3_64(x ^ y);
    eor64(&yreg, &xreg);
    rightRotate3(&yreg);

    // x = leftRotate8_64((x ^ *s--) - y);
    eor64ScheduleReverse(&xreg);
    leftRotate8(&xreg);
    sub64(&xreg, &yreg);

    // Bottom of the main loop.
    insn_printf("dec %%2");
    insn_printf("breq 2f");
    insn_printf("rjmp 1b");
    insn_printf("2:");
}

// Pack the output block and convert from little-endian to big-endian.
static void pack_output(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    insn_printf("ldd r%d,%%A3", x_reg);
    insn_printf("ldd r%d,%%B3", x_reg + 1);

    insn_printf("st X+,r%d", REGn(&xreg, 7));
    insn_printf("st X+,r%d", REGn(&xreg, 6));
    insn_printf("st X+,r%d", REGn(&xreg, 5));
    insn_printf("st X+,r%d", REGn(&xreg, 4));
    insn_printf("st X+,r%d", REGn(&xreg, 3));
    insn_printf("st X+,r%d", REGn(&xreg, 2));
    insn_printf("st X+,r%d", REGn(&xreg, 1));
    insn_printf("st X+,r%d", REGn(&xreg, 0));

    insn_printf("st X+,r%d", REGn(&yreg, 7));
    insn_printf("st X+,r%d", REGn(&yreg, 6));
    insn_printf("st X+,r%d", REGn(&yreg, 5));
    insn_printf("st X+,r%d", REGn(&yreg, 4));
    insn_printf("st X+,r%d", REGn(&yreg, 3));
    insn_printf("st X+,r%d", REGn(&yreg, 2));
    insn_printf("st X+,r%d", REGn(&yreg, 1));
    insn_printf("st X,r%d", REGn(&yreg, 0));
}

static void temp_regs(void)
{
    indent_printf(": \"r%d\", \"r%d\", \"r%d\", \"r%d\", "
                    "\"r%d\", \"r%d\", \"r%d\", \"r%d\",\n",
                  t1_reg, t1_reg + 1, t1_reg + 2, t1_reg + 3,
                  t1_reg + 4, t1_reg + 5, t1_reg + 6, t1_reg + 7);
    indent_printf("  \"r%d\", \"r%d\", \"r%d\", \"r%d\", "
                    "\"r%d\", \"r%d\", \"r%d\", \"r%d\", \"memory\"\n",
                  t2_reg, t2_reg + 1, t2_reg + 2, t2_reg + 3,
                  t2_reg + 4, t2_reg + 5, t2_reg + 6, t2_reg + 7);
}

static void full_setkey(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    printf("void Speck::setKey(const uint8_t *key, size_t len)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genspeck tool.\n");

    // Validate the key length.
    indent_printf("uint64_t l[4];\n");
    indent_printf("uint8_t m, mb;\n");
    indent_printf("if (len == 32) {\n");
    indent_printf("    m = 4;\n");
    indent_printf("    mb = 3 * 8;\n");
    indent_printf("} else if (len == 24) {\n");
    indent_printf("    m = 3;\n");
    indent_printf("    mb = 2 * 8;\n");
    indent_printf("} else if (len == 16) {\n");
    indent_printf("    m = 2;\n");
    indent_printf("    mb = 8;\n");
    indent_printf("} else {\n");
    indent_printf("    return false;\n");
    indent_printf("}\n");
    indent_printf("rounds = 30 + m;\n");
    indent_printf("uint8_t r = rounds - 1;\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Copy the key into k[0] and l while converting endianness.
    insn_printf("ld __tmp_reg__,-X");       // k[0] = last 8 bytes of the key
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("ld __tmp_reg__,-X");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("sbiw r%d,8", z_reg);       // Set Z back to beginning of k
    insn_printf("movw r%d,r%d", t1_reg + 2, z_reg); // Save Z
    insn_printf("movw r%d,%%A2", z_reg);    // Z = l
    insn_printf("ldd r%d,%%3", t1_reg);
    insn_printf("1:");
    insn_printf("ld __tmp_reg__,-X");       // Copy first mb bytes from key
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("dec r%d", t1_reg);
    insn_printf("brne 1b");
    insn_printf("movw r%d,%%A2", x_reg);            // X = l
    insn_printf("movw r%d,r%d", z_reg, t1_reg + 2); // Z = k

    // Expand the key to the full key schedule.
    // Note: We can use %A2 and %B2 as spare temporary registers now.
    insn_printf("clr %%A2");                // %A2 = li_in = 0
    insn_printf("ldd %%B2,%%3");            // %B2 = li_out = mb (= (m - 1) * 8)
    insn_printf("clr r%d", temp_reg);       // i = 0
    load_from_z(&yreg);                     // y = k[i]
    insn_printf("2:");

    // l[li_out] = (k[i] + rightRotate8_64(l[li_in])) ^ i
    insn_printf("add r%d,%%A2", x_reg);     // x = rightRotate8_64(l[li_in])
    insn_printf("adc r%d,__zero_reg__", x_reg + 1);
    xreg.offset = 7;
    load_from_x(&xreg);
    xreg.offset = 0;
    insn_printf("sub r%d,%%A2", x_reg);     // restore X to point at base of l
    insn_printf("sbc r%d,__zero_reg__", x_reg + 1);
    insn_printf("sbiw r%d,8", x_reg);
    add64(&xreg, &yreg);                    // x += y
    insn_printf("eor r%d,r%d", REGn(&xreg, 0), temp_reg); // x ^= i
    insn_printf("add r%d,%%B2", x_reg);     // l[li_out] = x
    insn_printf("adc r%d,__zero_reg__", x_reg + 1);
    store_to_x(&xreg);
    insn_printf("sub r%d,%%B2", x_reg);     // restore X to point at base of l
    insn_printf("sbc r%d,__zero_reg__", x_reg + 1);
    insn_printf("sbiw r%d,8", x_reg);

    // k[i + 1] = leftRotate3_64(k[i]) ^ l[li_out];
    leftRotate3(&yreg);                     // y = leftRotate3(y)
    eor64(&yreg, &xreg);                    // y ^= x
    store_to_z(&yreg);                      // k[i + 1] = y

    // Advance li_in and li_out, wrapping around at the end of l.
    insn_printf("ldi r%d,8", const_reg);
    insn_printf("add %%A2,r%d", const_reg);
    insn_printf("add %%B2,r%d", const_reg);
    insn_printf("ldi r%d,0x1F", const_reg);
    insn_printf("and %%A2,r%d", const_reg);
    insn_printf("and %%B2,r%d", const_reg);

    // Bottom of the loop.
    insn_printf("ldd r%d,%%4", t1_reg);     // r8 = rounds - 1
    insn_printf("inc r%d", temp_reg);       // ++i
    insn_printf("cp r%d,r%d", temp_reg, t1_reg);
    insn_printf("breq 3f");
    insn_printf("rjmp 2b");
    insn_printf("3:");

    // Clean the l array.  X register should still be pointing to it.
    insn_printf("ldi r%d,32", const_reg);
    insn_printf("4:");
    insn_printf("st X+,__zero_reg__");
    insn_printf("dec r%d", const_reg);
    insn_printf("brne 4b");

    // Declare the registers that we need.
    indent_printf(": : \"z\"(k), \"x\"(key + len), \"r\"(l), \"Q\"(mb), \"Q\"(r)\n");
    temp_regs();
    indent_printf(", \"r%d\", \"r%d\"\n", const_reg, temp_reg);
    indent -= 4;
    indent_printf(");\n");

    // End of function.
    indent_printf("return true;\n");
    printf("}\n\n");
}

static void full_enc(void)
{
    printf("void Speck::encryptBlock(uint8_t *output, const uint8_t *input)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genspeck tool.\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;
    unpack_input();
    full_enc_main_loop();
    pack_output();
    indent_printf(": : \"x\"(input), \"z\"(k), \"r\"(rounds), \"Q\"(output)\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    printf("}\n\n");
}

static void full_dec(void)
{
    printf("void Speck::decryptBlock(uint8_t *output, const uint8_t *input)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genspeck tool.\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;
    unpack_input();
    full_dec_main_loop();
    pack_output();
    indent_printf(": : \"x\"(input), \"z\"(k + rounds), \"r\"(rounds), \"Q\"(output)\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    printf("}\n\n");
}

static void tiny_enc(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    printf("void SpeckTiny::encryptBlock(uint8_t *output, const uint8_t *input)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genspeck tool.\n");
    indent_printf("uint64_t l[5];\n");
    indent_printf("uint8_t r = rounds;\n");
    indent_printf("uint8_t mb = (r - 31) * 8;\n");

    // Copy the "k" array into the "l" array.  The first element is "s"
    // and the rest of the elements make up the normal l[0..3] values.
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;
    insn_printf("movw r%d,r%d", t1_reg, z_reg); // Save Z
    insn_printf("ldd r%d,%%4", t2_reg);
    insn_printf("ldi r%d,8", const_reg);
    insn_printf("add r%d,r%d", t2_reg, const_reg);
    insn_printf("1:");
    insn_printf("ld __tmp_reg__,X+");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("dec r%d", t2_reg);
    insn_printf("brne 1b");
    insn_printf("movw r%d,r%d", z_reg, t1_reg); // Restore Z to point at l

    // Unpack the input.  %A2 and %B2 are free temporary registers after this.
    insn_printf("movw r%d,%%A2", x_reg);
    unpack_input();

    // Top of the loop.
    insn_printf("clr %%A2");                // %A2 = li_in = 0
    insn_printf("ldd %%B2,%%4");            // %B2 = li_out = mb
    insn_printf("clr r%d", temp_reg);       // i = 0
    insn_printf("2:");

    // Adjust x and y for this round using the key schedule word s (in l[0]).
    // x = (rightRotate8_64(x) + y) ^ s;
    rightRotate8(&xreg);
    add64(&xreg, &yreg);
    eor64Schedule(&xreg);
    // y = leftRotate3_64(y) ^ x;
    leftRotate3(&yreg);
    eor64(&yreg, &xreg);
    // At this point, Z has been incremented to point at l[1] which
    // is the start of the actual l[0] from the original formulation.

    // If this is the last round, then we are done.  There is no
    // point calculating another key schedule element.
    insn_printf("mov __tmp_reg__,r%d", temp_reg);
    insn_printf("inc __tmp_reg__");
    insn_printf("ldd r%d,%%5", const_reg);
    insn_printf("cp __tmp_reg__,r%d", const_reg);
    insn_printf("brne 3f");
    insn_printf("rjmp 4f");
    insn_printf("3:");

    // Save x and y on the stack - we need the registers to
    // help us compute the next key schedule element.
    push64(&xreg);
    push64(&yreg);

    // Compute the key schedule word s for the next round.
    insn_printf("sbiw r%d,8", z_reg);       // Point Z back at l[0]
    // l[li_out] = (s + rightRotate8_64(l[li_in])) ^ i;
    insn_printf("ld r%d,Z", REGn(&yreg, 0)); // y = s
    insn_printf("ldd r%d,Z+1", REGn(&yreg, 1));
    insn_printf("ldd r%d,Z+2", REGn(&yreg, 2));
    insn_printf("ldd r%d,Z+3", REGn(&yreg, 3));
    insn_printf("ldd r%d,Z+4", REGn(&yreg, 4));
    insn_printf("ldd r%d,Z+5", REGn(&yreg, 5));
    insn_printf("ldd r%d,Z+6", REGn(&yreg, 6));
    insn_printf("ldd r%d,Z+7", REGn(&yreg, 7));
    insn_printf("add r%d,%%A2", z_reg);     // Z = &(l[li_in]) - 8
    insn_printf("adc r%d,__zero_reg__", z_reg + 1);
    leftRotate8(&xreg);                     // x = rightRotate8(l[li_in])
    insn_printf("ldd r%d,Z+8", REGn(&xreg, 0));
    insn_printf("ldd r%d,Z+9", REGn(&xreg, 1));
    insn_printf("ldd r%d,Z+10", REGn(&xreg, 2));
    insn_printf("ldd r%d,Z+11", REGn(&xreg, 3));
    insn_printf("ldd r%d,Z+12", REGn(&xreg, 4));
    insn_printf("ldd r%d,Z+13", REGn(&xreg, 5));
    insn_printf("ldd r%d,Z+14", REGn(&xreg, 6));
    insn_printf("ldd r%d,Z+15", REGn(&xreg, 7));
    rightRotate8(&xreg);
    add64(&xreg, &yreg);                    // x += y
    insn_printf("eor r%d,r%d", REGn(&xreg, 0), temp_reg); // x ^= i
    insn_printf("sub r%d,%%A2", z_reg);     // Z = &(l[li_out]) - 8
    insn_printf("sbc r%d,__zero_reg__", z_reg + 1);
    insn_printf("add r%d,%%B2", z_reg);
    insn_printf("adc r%d,__zero_reg__", z_reg + 1);
    insn_printf("std Z+8,r%d", REGn(&xreg, 0)); // l[li_out] = x
    insn_printf("std Z+9,r%d", REGn(&xreg, 1));
    insn_printf("std Z+10,r%d", REGn(&xreg, 2));
    insn_printf("std Z+11,r%d", REGn(&xreg, 3));
    insn_printf("std Z+12,r%d", REGn(&xreg, 4));
    insn_printf("std Z+13,r%d", REGn(&xreg, 5));
    insn_printf("std Z+14,r%d", REGn(&xreg, 6));
    insn_printf("std Z+15,r%d", REGn(&xreg, 7));
    insn_printf("sub r%d,%%B2", z_reg);     // Restore Z to base of l array
    insn_printf("sbc r%d,__zero_reg__", z_reg + 1);
    // s = leftRotate3_64(s) ^ l[li_out];
    leftRotate3(&yreg);
    eor64(&yreg, &xreg);
    insn_printf("st Z,r%d", REGn(&yreg, 0));
    insn_printf("std Z+1,r%d", REGn(&yreg, 1));
    insn_printf("std Z+2,r%d", REGn(&yreg, 2));
    insn_printf("std Z+3,r%d", REGn(&yreg, 3));
    insn_printf("std Z+4,r%d", REGn(&yreg, 4));
    insn_printf("std Z+5,r%d", REGn(&yreg, 5));
    insn_printf("std Z+6,r%d", REGn(&yreg, 6));
    insn_printf("std Z+7,r%d", REGn(&yreg, 7));

    // Advance li_in and li_out, wrapping around at the end of l.
    insn_printf("ldi r%d,8", const_reg);
    insn_printf("add %%A2,r%d", const_reg);
    insn_printf("add %%B2,r%d", const_reg);
    insn_printf("ldi r%d,0x1F", const_reg);
    insn_printf("and %%A2,r%d", const_reg);
    insn_printf("and %%B2,r%d", const_reg);

    // Restore the original x and y.
    pop64(&yreg);
    pop64(&xreg);

    // Bottom of the loop.
    insn_printf("inc r%d", temp_reg);       // i++
    insn_printf("rjmp 2b");
    insn_printf("4:");

    // Pack the results into the output buffer.
    pack_output();

    // Declare the registers that we need.
    indent_printf(": : \"x\"(k), \"z\"(l), \"r\"(input), \"Q\"(output), \"Q\"(mb), \"Q\"(r)\n");
    temp_regs();
    indent_printf(", \"r%d\", \"r%d\"\n", const_reg, temp_reg);
    indent -= 4;
    indent_printf(");\n");
    printf("}\n\n");
}

static void small_dec(void)
{
    Reg64 xreg = {t1_reg, 0};
    Reg64 yreg = {t2_reg, 0};

    printf("void SpeckSmall::decryptBlock(uint8_t *output, const uint8_t *input)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genspeck tool.\n");
    indent_printf("uint64_t l[5];\n");
    indent_printf("uint8_t r = rounds;\n");
    indent_printf("uint8_t li_in = ((r + 3) & 0x03) * 8;\n");
    indent_printf("uint8_t li_out = ((((r - 31) & 0x03) * 8) + li_in) & 0x1F;\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Copy the this->l array into the local l array.  Then copy
    // the "s" value from l[li_out] to l[4].
    insn_printf("ldd r%d,%%4", temp_reg);   // r25 = li_out
    insn_printf("ldi r%d,32", const_reg);   // Copy 32 bytes from this->l.
    insn_printf("1:");
    insn_printf("ld __tmp_reg__,X+");
    insn_printf("st Z+,__tmp_reg__");
    insn_printf("dec r%d", const_reg);
    insn_printf("brne 1b");
    insn_printf("movw r%d,r%d", x_reg, z_reg); // X = Z + 32
    insn_printf("sbiw r%d,32", z_reg);      // Z = &(l[li_out])
    insn_printf("add r%d,r%d", z_reg, temp_reg);
    insn_printf("adc r%d,__zero_reg__", z_reg + 1);
    insn_printf("ld __tmp_reg__,Z");        // Copy l[li_out] to l[4]
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+1");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+2");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+3");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+4");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+5");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+6");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("ldd __tmp_reg__,Z+7");
    insn_printf("st X+,__tmp_reg__");
    insn_printf("sub r%d,r%d", z_reg, temp_reg); // Z = &(l[0])
    insn_printf("sbc r%d,__zero_reg__", z_reg + 1);

    // Unpack the input.  %A2 and %B2 are free temporary registers after this.
    insn_printf("movw r%d,%%A2", x_reg);
    unpack_input();

    // Top of the loop.
    insn_printf("ldd %%A2,%%6");            // %A2 = li_in
    insn_printf("mov %%B2,r%d", temp_reg);  // %B2 = li_out
    insn_printf("ldd r%d,%%5", temp_reg);   // i = rounds - 1
    insn_printf("dec r%d", temp_reg);
    insn_printf("movw r%d,r%d", x_reg, z_reg); // X = Z + 40 = &(l[5])
    insn_printf("adiw r%d,40", x_reg);      // i.e. point to end of l[4]
    insn_printf("2:");

    // Adjust x and y for this round using the key schedule word s (in l[4]).
    // y = rightRotate3_64(x ^ y);
    eor64(&yreg, &xreg);
    rightRotate3(&yreg);
    // x = leftRotate8_64((x ^ s) - y);
    eor64ScheduleReverseX(&xreg);
    leftRotate8(&xreg);
    sub64(&xreg, &yreg);

    // If this is the last round, then we are done.  There is no
    // point calculating another key schedule element.
    insn_printf("or r%d,r%d", temp_reg, temp_reg); // if (i == 0)
    insn_printf("brne 3f");
    insn_printf("rjmp 4f");
    insn_printf("3:");
    insn_printf("dec r%d", temp_reg);       // --i

    // Save x and y on the stack - we need the registers to
    // help us compute the next key schedule element.
    push64(&xreg);
    push64(&yreg);

    // Move li_in and li_out backwards, wrapping around at the start of l.
    insn_printf("ldi r%d,24", const_reg);
    insn_printf("add %%A2,r%d", const_reg);
    insn_printf("add %%B2,r%d", const_reg);
    insn_printf("ldi r%d,0x1F", const_reg);
    insn_printf("and %%A2,r%d", const_reg);
    insn_printf("and %%B2,r%d", const_reg);

    // Compute the key schedule word s for the next round.
    // s = rightRotate3_64(s ^ l[li_out]);
    insn_printf("ld r%d,X+", REGn(&yreg, 0)); // y = s = l[4]
    insn_printf("ld r%d,X+", REGn(&yreg, 1));
    insn_printf("ld r%d,X+", REGn(&yreg, 2));
    insn_printf("ld r%d,X+", REGn(&yreg, 3));
    insn_printf("ld r%d,X+", REGn(&yreg, 4));
    insn_printf("ld r%d,X+", REGn(&yreg, 5));
    insn_printf("ld r%d,X+", REGn(&yreg, 6));
    insn_printf("ld r%d,X+", REGn(&yreg, 7));
    insn_printf("add r%d,%%B2", z_reg);     // Z = &(l[li_out])
    insn_printf("adc r%d,__zero_reg__", z_reg + 1);
    insn_printf("ld r%d,Z", REGn(&xreg, 0)); // x = l[li_out]
    insn_printf("ldd r%d,Z+1", REGn(&xreg, 1));
    insn_printf("ldd r%d,Z+2", REGn(&xreg, 2));
    insn_printf("ldd r%d,Z+3", REGn(&xreg, 3));
    insn_printf("ldd r%d,Z+4", REGn(&xreg, 4));
    insn_printf("ldd r%d,Z+5", REGn(&xreg, 5));
    insn_printf("ldd r%d,Z+6", REGn(&xreg, 6));
    insn_printf("ldd r%d,Z+7", REGn(&xreg, 7));
    insn_printf("sub r%d,%%B2", z_reg);     // Z = &(l[0])
    insn_printf("sbc r%d,__zero_reg__", z_reg + 1);
    eor64(&yreg, &xreg);
    rightRotate3(&yreg);
    insn_printf("st -X,r%d", REGn(&yreg, 7)); // store s back into l[4]
    insn_printf("st -X,r%d", REGn(&yreg, 6));
    insn_printf("st -X,r%d", REGn(&yreg, 5));
    insn_printf("st -X,r%d", REGn(&yreg, 4));
    insn_printf("st -X,r%d", REGn(&yreg, 3));
    insn_printf("st -X,r%d", REGn(&yreg, 2));
    insn_printf("st -X,r%d", REGn(&yreg, 1));
    insn_printf("st -X,r%d", REGn(&yreg, 0));
    insn_printf("adiw r%d,8", x_reg);       // X = &(l[5])
    // l[li_in] = leftRotate8_64((l[li_out] ^ i) - s);
    insn_printf("eor r%d,r%d", t1_reg, temp_reg); // x ^= i
    sub64(&xreg, &yreg);                    // x -= s
    leftRotate8(&xreg);                     // x = leftRotate8(x)
    insn_printf("add r%d,%%A2", z_reg);     // Z = &(l[li_in])
    insn_printf("adc r%d,__zero_reg__", z_reg + 1);
    insn_printf("st Z,r%d", REGn(&xreg, 0)); // l[li_in] = x
    insn_printf("std Z+1,r%d", REGn(&xreg, 1));
    insn_printf("std Z+2,r%d", REGn(&xreg, 2));
    insn_printf("std Z+3,r%d", REGn(&xreg, 3));
    insn_printf("std Z+4,r%d", REGn(&xreg, 4));
    insn_printf("std Z+5,r%d", REGn(&xreg, 5));
    insn_printf("std Z+6,r%d", REGn(&xreg, 6));
    insn_printf("std Z+7,r%d", REGn(&xreg, 7));
    insn_printf("sub r%d,%%A2", z_reg);     // Z = &(l[0])
    insn_printf("sbc r%d,__zero_reg__", z_reg + 1);

    // Restore the original x and y.
    pop64(&yreg);
    pop64(&xreg);

    // Bottom of the loop.
    insn_printf("rjmp 2b");
    insn_printf("4:");

    // Pack the results into the output buffer.
    pack_output();

    // Declare the registers that we need.
    indent_printf(": : \"x\"(this->l), \"z\"(l), \"r\"(input), \"Q\"(output), \"Q\"(li_out), \"Q\"(r), \"Q\"(li_in)\n");
    temp_regs();
    indent_printf(", \"r%d\", \"r%d\"\n", const_reg, temp_reg);
    indent -= 4;
    indent_printf(");\n");
    printf("}\n\n");
}

int main(int argc, char *argv[])
{
    full_setkey();
    full_enc();
    full_dec();

    tiny_enc();

    small_dec();
    return 0;
}
