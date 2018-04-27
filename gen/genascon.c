/*
 * Copyright (C) 2018 Southern Storm Software, Pty Ltd.
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

// Special-purpose compiler that generates the AVR version of Ascon128.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static int indent = 4;

static const int temp_reg = 0; // Register number for the AVR "__tmp_reg__".

// Registers that can be used for temporary values, in the best
// order to allocate them.  High registers are listed first.
static int regs[] = {
    16, 17, 18, 19, 20, 21, 22, 23, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
#define num_regs (sizeof(regs) / sizeof(regs[0]))

// Which registers are currently in use?
static int reg_in_use[num_regs] = {0};

// Which registers did we use while generating the code?
static int reg_used[num_regs] = {0};

// Information about a set of registers storing a 64-bit quantity.
typedef struct
{
    int first;              // First register in the set for 8-bit rotates.
    int reg[8];             // Register numbers.

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

// Allocate an unused register, starting with high registers.
void alloc_high_reg(int *reg)
{
    unsigned index;
    if (*reg != -1) {
        fprintf(stderr, "Temporary register wasn't previously released\n");
        exit(1);
    }
    for (index = 0; index < num_regs; ++index) {
        if (!reg_in_use[index]) {
            reg_in_use[index] = 1;
            reg_used[index] = 1;
            *reg = regs[index];
            if (*reg < 16) {
                fprintf(stderr, "Ran out of temporary high registers\n");
                exit(1);
            }
            return;
        }
    }
    fprintf(stderr, "Ran out of temporary registers\n");
    exit(1);
}

// Allocate an unused register, starting with low registers
// because we know we won't need the value in a high reg later.
static void alloc_low_reg(int *reg)
{
    unsigned index;
    if (*reg != -1) {
        fprintf(stderr, "Temporary register wasn't previously released\n");
        exit(1);
    }
    for (index = num_regs; index > 0; --index) {
        if (!reg_in_use[index - 1]) {
            reg_in_use[index - 1] = 1;
            reg_used[index - 1] = 1;
            *reg = regs[index - 1];
            return;
        }
    }
    fprintf(stderr, "Ran out of temporary registers\n");
    exit(1);
}

// Release a register back to the allocation pool.
static void release_reg(int *reg)
{
    unsigned index;
    for (index = 0; index < num_regs; ++index) {
        if (regs[index] == *reg && reg_in_use[index]) {
            reg_in_use[index] = 0;
            *reg = -1;
            return;
        }
    }
    fprintf(stderr, "Released a register that was not in use\n");
    exit(1);
}

// Check that we have a high register when we need one.
void check_high_reg(int *reg)
{
    if ((*reg) < 16) {
        fprintf(stderr, "r%d is not a high register\n", *reg);
        exit(1);
    }
}

// Check that all temporary registers have been released.
static void check_regs(void)
{
    unsigned index;
    for (index = 0; index < num_regs; ++index) {
        if (reg_in_use[index]) {
            fprintf(stderr, "Register r%d has not been released\n",
                    regs[index]);
            exit(1);
        }
    }
}

// Allocate a 64-bit register set.
static void alloc_reg64(Reg64 *reg)
{
    reg->first = 0;
    reg->reg[0] = -1;
    reg->reg[1] = -1;
    reg->reg[2] = -1;
    reg->reg[3] = -1;
    reg->reg[4] = -1;
    reg->reg[5] = -1;
    reg->reg[6] = -1;
    reg->reg[7] = -1;
    alloc_low_reg(&(reg->reg[0]));
    alloc_low_reg(&(reg->reg[1]));
    alloc_low_reg(&(reg->reg[2]));
    alloc_low_reg(&(reg->reg[3]));
    alloc_low_reg(&(reg->reg[4]));
    alloc_low_reg(&(reg->reg[5]));
    alloc_low_reg(&(reg->reg[6]));
    alloc_low_reg(&(reg->reg[7]));
}

// Release a 64-bit register set.
static void release_reg64(Reg64 *reg)
{
    release_reg(&(reg->reg[0]));
    release_reg(&(reg->reg[1]));
    release_reg(&(reg->reg[2]));
    release_reg(&(reg->reg[3]));
    release_reg(&(reg->reg[4]));
    release_reg(&(reg->reg[5]));
    release_reg(&(reg->reg[6]));
    release_reg(&(reg->reg[7]));
}

#define REGn(r, n) ((r)->reg[((n) + (r)->first) % 8])

// Rotate a 64-bit register left by 1 bit.
static void rotate_reg64_left_1(Reg64 *reg)
{
    insn_printf("lsl r%d", REGn(reg, 0));
    insn_printf("rol r%d", REGn(reg, 1));
    insn_printf("rol r%d", REGn(reg, 2));
    insn_printf("rol r%d", REGn(reg, 3));
    insn_printf("rol r%d", REGn(reg, 4));
    insn_printf("rol r%d", REGn(reg, 5));
    insn_printf("rol r%d", REGn(reg, 6));
    insn_printf("rol r%d", REGn(reg, 7));
    insn_printf("adc r%d,__zero_reg__", REGn(reg, 0));
}

// Rotate a 64-bit register left by 8 bits.
static void rotate_reg64_left_8(Reg64 *reg)
{
    reg->first = (reg->first + 7) % 8;
}

// Rotate a 64-bit register right by 1 bit.
static void rotate_reg64_right_1(Reg64 *reg)
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

// Rotate a 64-bit register right by 8 bits.
static void rotate_reg64_right_8(Reg64 *reg)
{
    reg->first = (reg->first + 1) % 8;
}

// Rotate a 64-bit register right by a number of bits.
static void rotate_reg64_right(Reg64 *reg, int bits)
{
    int bytes;
    if ((bits % 8) < 4) {
        // Rotate in the right direction.
        bytes = bits / 8;
        while (bytes > 0) {
            rotate_reg64_right_8(reg);
            --bytes;
        }
        bits %= 8;
        while (bits > 0) {
            rotate_reg64_right_1(reg);
            --bits;
        }
    } else {
        // Quicker to rotate in the left direction.
        bits = 64 - bits;
        bytes = bits / 8;
        while (bytes > 0) {
            rotate_reg64_left_8(reg);
            --bytes;
        }
        bits %= 8;
        while (bits > 0) {
            rotate_reg64_left_1(reg);
            --bits;
        }
    }
}

// Load a 64-bit register from a Z pointer offset.
static void load_reg64(Reg64 *reg, int offset)
{
    int index;
    for (index = 0; index < 8; ++index, ++offset) {
        if (offset != 0)
            insn_printf("ldd r%d,Z+%d", REGn(reg, index), offset);
        else
            insn_printf("ld r%d,Z", REGn(reg, index));
    }
}

// Store a 64-bit register to a Z pointer offset.
static void store_reg64(Reg64 *reg, int offset)
{
    int index;
    for (index = 0; index < 8; ++index, ++offset) {
        if (offset != 0)
            insn_printf("std Z+%d,r%d", offset, REGn(reg, index));
        else
            insn_printf("st Z,r%d", REGn(reg, index));
    }
}

// Copy one 64-bit register into another.
static void copy_reg64(Reg64 *dst, Reg64 *src)
{
    int index;
    for (index = 0; index < 8; ++index)
        insn_printf("mov r%d,r%d", REGn(dst, index), REGn(src, index));
}

// XOR two 64-bit registers.
static void xor_reg64(Reg64 *dst, Reg64 *src)
{
    int index;
    for (index = 0; index < 8; ++index)
        insn_printf("eor r%d,r%d", REGn(dst, index), REGn(src, index));
}

// Print the names of the temporary registers that we used.
static void temp_regs(void)
{
    unsigned index;
    int first = 1;
    indent_printf(": ");
    for (index = 0; index < num_regs; ++index) {
        if (reg_used[index]) {
            if (first) {
                first = 0;
                printf("\"r%d\"", regs[index]);
            } else {
                printf(", \"r%d\"", regs[index]);
            }
        }
    }
    printf(", \"memory\"\n");
}

// Handle one byte within the substitution layer.
static void substitute(int offset, int reg)
{
    int x0 = -1;
    int x1 = -1;
    int x2 = reg;
    int x3 = -1;
    int x4 = -1;
    int t0 = -1;
    int t1 = -1;
    int t2 = -1;
    int t3 = -1;
    int t4 = -1;

    // Allocate five temporary registers for the five words.
    // When processing the zero byte, we already have x2 loaded.
    alloc_low_reg(&x0);
    alloc_low_reg(&x1);
    if (reg == -1)
        alloc_low_reg(&x2);
    alloc_low_reg(&x3);
    alloc_low_reg(&x4);

    // Need five more registers for intermediate results.
    alloc_low_reg(&t0);
    alloc_low_reg(&t1);
    alloc_low_reg(&t2);
    alloc_low_reg(&t3);
    alloc_low_reg(&t4);

    // Read the five bytes from the state.
    if (offset != 0)
        insn_printf("ldd r%d,Z+%d", x0, offset);
    else
        insn_printf("ld r%d,Z", x0);
    insn_printf("ldd r%d,Z+%d", x1, 8 + offset);
    if (reg == -1)
        insn_printf("ldd r%d,Z+%d", x2, 16 + offset);
    insn_printf("ldd r%d,Z+%d", x3, 24 + offset);
    insn_printf("ldd r%d,Z+%d", x4, 32 + offset);

    // Mix the bytes together.
    // x0 ^= x4;   x4 ^= x3;   x2 ^= x1;
    // t0 = ~x0;   t1 = ~x1;   t2 = ~x2;   t3 = ~x3;   t4 = ~x4;
    // t0 &= x1;   t1 &= x2;   t2 &= x3;   t3 &= x4;   t4 &= x0;
    // x0 ^= t1;   x1 ^= t2;   x2 ^= t3;   x3 ^= t4;   x4 ^= t0;
    // x1 ^= x0;   x0 ^= x4;   x3 ^= x2;   x2 = ~x2;
    insn_printf("eor r%d,r%d", x0, x4);
    insn_printf("eor r%d,r%d", x4, x3);
    insn_printf("eor r%d,r%d", x2, x1);
    insn_printf("mov r%d,r%d", t0, x0);
    insn_printf("com r%d", t0);
    insn_printf("and r%d,r%d", t0, x1);
    insn_printf("mov r%d,r%d", t1, x1);
    insn_printf("com r%d", t1);
    insn_printf("and r%d,r%d", t1, x2);
    insn_printf("mov r%d,r%d", t2, x2);
    insn_printf("com r%d", t2);
    insn_printf("and r%d,r%d", t2, x3);
    insn_printf("mov r%d,r%d", t3, x3);
    insn_printf("com r%d", t3);
    insn_printf("and r%d,r%d", t3, x4);
    insn_printf("mov r%d,r%d", t4, x4);
    insn_printf("com r%d", t4);
    insn_printf("and r%d,r%d", t4, x0);
    insn_printf("eor r%d,r%d", x0, t1);
    insn_printf("eor r%d,r%d", x1, t2);
    insn_printf("eor r%d,r%d", x2, t3);
    insn_printf("eor r%d,r%d", x3, t4);
    insn_printf("eor r%d,r%d", x4, t0);
    insn_printf("eor r%d,r%d", x1, x0);
    insn_printf("eor r%d,r%d", x0, x4);
    insn_printf("eor r%d,r%d", x3, x2);
    insn_printf("com r%d", x2);

    // Write the five bytes back to the state.
    if (offset != 0)
        insn_printf("std Z+%d,r%d", offset, x0);
    else
        insn_printf("st Z,r%d", x0);
    insn_printf("std Z+%d,r%d", 8 + offset, x1);
    insn_printf("std Z+%d,r%d", 16 + offset, x2);
    insn_printf("std Z+%d,r%d", 24 + offset, x3);
    insn_printf("std Z+%d,r%d", 32 + offset, x4);

    // Release the temporary registers.
    release_reg(&x0);
    release_reg(&x1);
    if (reg == -1)
        release_reg(&x2);
    release_reg(&x3);
    release_reg(&x4);
    release_reg(&t0);
    release_reg(&t1);
    release_reg(&t2);
    release_reg(&t3);
    release_reg(&t4);
}

static void diffuse(int n, int shift1, int shift2)
{
    Reg64 x, t;
    int offset = n * 8;

    // Allocate the temporary registers that we need.
    alloc_reg64(&x);
    alloc_reg64(&t);

    // Read the 64-bit word into the x registers.
    load_reg64(&x, offset);

    // Rotate step 1: t = x >>> shift1, t ^= x
    copy_reg64(&t, &x);
    rotate_reg64_right(&t, shift1);
    xor_reg64(&t, &x);

    // Rotate step 2: x = x >>> shift2, x ^= t
    rotate_reg64_right(&x, shift2);
    xor_reg64(&x, &t);

    // Store the 64-bit result back to the state.
    store_reg64(&x, offset);

    // Release the temporary registers.
    release_reg64(&x);
    release_reg64(&t);
}

static void permute(void)
{
    int reg = -1;

    // Print the function header.
    printf("void Ascon128::permute(uint8_t first)\n");
    printf("{\n");
    indent_printf("// AVR version generated by the genascon tool.\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Output the top of the loop.
    insn_printf("1:");

    // XOR the low byte of x2 with the round constant.
    alloc_low_reg(&reg);
    insn_printf("ldd r%d,Z+16", reg);
    insn_printf("eor r%d,%%1", reg);

    // Substitution layer.  Mix the words one byte at a time.
    // For the first byte we already have x2 loaded into "reg".
    substitute(0, reg);
    release_reg(&reg);
    substitute(1, -1);
    substitute(2, -1);
    substitute(3, -1);
    substitute(4, -1);
    substitute(5, -1);
    substitute(6, -1);
    substitute(7, -1);

    // Linear diffusion layer.
    diffuse(0, 19, 28);
    diffuse(1, 61, 39);
    diffuse(2,  1,  6);
    diffuse(3, 10, 17);
    diffuse(4,  7, 41);

    // Output the bottom of the loop.
    insn_printf("subi %%1,0x0F");
    insn_printf("cpi %%1,0x3C");
    insn_printf("breq 2f");
    insn_printf("rjmp 1b");
    insn_printf("2:");

    // Declare the registers that we need.
    indent_printf(":: \"z\"(state.S), \"d\"((uint8_t)(0xF0 - (first << 4) + first))\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    printf("}\n\n");
    check_regs();
}

int main(int argc, char *argv[])
{
    permute();
    return 0;
}
