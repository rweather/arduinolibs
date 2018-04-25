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

// Special-purpose compiler that generates the AVR version of Acorn128.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static int indent = 4;

static const int temp_reg = 0; // Register number for the AVR "__tmp_reg__".

// Information about the positions and lengths of the LFSR's.
typedef struct
{
    int start;      // Bit position where the LFSR starts.
    int len;        // Length of the LFSR in bits.
    int offsetl;    // Offset of the low word of the LFSR in the state.
    int offseth;    // Offset of the high word of the LFSR in the state.

} LFSR;
#define num_lfsrs 7
static LFSR const lfsr[num_lfsrs] = {
    {0,     61,     0,      4},
    {61,    46,     8,      12},
    {107,   47,     16,     14},
    {154,   39,     20,     24},
    {193,   37,     28,     26},
    {230,   59,     32,     36},
    {289,   4,      40,     40},
};

// LFSR byte offset for generating 32-bit versions of the code.
static int lfsr_offset = 0;

// Non-zero to generate the 32-bit version.
static int is_32bit_version = 0;

// Registers that can be used for temporary values, in the best
// order to allocate them.  High registers are listed first.
static int regs[] = {
    16, 17, 18, 19, 20, 21, 22, 23, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};
#define num_regs (sizeof(regs) / sizeof(regs[0]))

// Which registers are currently in use?
static int reg_in_use[num_regs] = {0};

// Which registers did we use while generating the code?
static int reg_used[num_regs] = {0};

// Registers that have been allocated to specific purposes.
static int s244 = -1;
static int s235 = -1;
static int s196 = -1;
static int s160 = -1;
static int s111 = -1;
static int s66 = -1;
static int s23 = -1;
static int s12 = -1;
static int s1_l = -1;
static int s2_l = -1;
static int s3_l = -1;
static int s4_l = -1;
static int s5_l = -1;
static int s6_l = -1;
static int s7_l = -1;
static int s7_l_prev[3] = {-1, -1, -1};
static int ks = -1;
static int f = -1;

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
static void alloc_high_reg(int *reg)
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
static void check_high_reg(int *reg)
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

// Find the information about a specific LFSR.
static const LFSR *find_lfsr(int bit)
{
    unsigned index;
    for (index = 0; index < num_lfsrs; ++index) {
        if (bit >= lfsr[index].start &&
                bit < (lfsr[index].start + lfsr[index].len))
            return &(lfsr[index]);
    }
    return &(lfsr[num_lfsrs - 1]);
}

// Gets the information for a specific LFSR from 0 to 6.
static const LFSR *get_lfsr(int num)
{
    return &(lfsr[num]);
}

// Shift a two-register value left by a number of bits.
static void shift_left_2_regs(int reg1, int reg2, int count)
{
    while (count > 0) {
        insn_printf("lsl r%d", reg2);
        insn_printf("rol r%d", reg1);
        --count;
    }
}

// Shift a three-register value left by a number of bits.
static void shift_left_3_regs(int reg1, int reg2, int reg3, int count)
{
    while (count > 0) {
        insn_printf("lsl r%d", reg3);
        insn_printf("rol r%d", reg2);
        insn_printf("rol r%d", reg1);
        --count;
    }
}

// Shift a five-register value left by a number of bits.
static void shift_left_5_regs
    (int reg1, int reg2, int reg3, int reg4, int reg5, int count)
{
    while (count > 0) {
        insn_printf("lsl r%d", reg5);
        insn_printf("rol r%d", reg4);
        insn_printf("rol r%d", reg3);
        insn_printf("rol r%d", reg2);
        insn_printf("rol r%d", reg1);
        --count;
    }
}

// Shift a two-register value right by a number of bits.
static void shift_right_2_regs(int reg1, int reg2, int count)
{
    while (count > 0) {
        insn_printf("lsr r%d", reg1);
        insn_printf("ror r%d", reg2);
        --count;
    }
}

// Shift a five-register value right by a number of bits.
static void shift_right_5_regs
    (int reg1, int reg2, int reg3, int reg4, int reg5, int count)
{
    while (count > 0) {
        insn_printf("lsr r%d", reg1);
        insn_printf("ror r%d", reg2);
        insn_printf("ror r%d", reg3);
        insn_printf("ror r%d", reg4);
        insn_printf("ror r%d", reg5);
        --count;
    }
}

// Extracts one part from the state as a byte.
static void extract_one_part(int reg, int bit)
{
    const LFSR *lfsr = find_lfsr(bit);
    int offset;
    bit -= lfsr->start;
    offset = lfsr->offsetl + lfsr_offset + (bit / 8);
    bit %= 8;
    if (bit < 4) {
        insn_printf("ldd r%d,Z+%d", reg, offset);
        insn_printf("ldd r%d,Z+%d", temp_reg, offset + 1);
        shift_right_2_regs(temp_reg, reg, bit);
    } else if (bit > 4) {
        insn_printf("ldd r%d,Z+%d", temp_reg, offset);
        insn_printf("ldd r%d,Z+%d", reg, offset + 1);
        shift_left_2_regs(reg, temp_reg, 8 - bit);
    } else {
        int extra_reg = -1;
        alloc_high_reg(&extra_reg);
        insn_printf("ldd r%d,Z+%d", reg, offset);
        insn_printf("ldd r%d,Z+%d", extra_reg, offset + 1);
        insn_printf("swap r%d", reg);
        insn_printf("swap r%d", extra_reg);
        check_high_reg(&reg);
        check_high_reg(&extra_reg);
        insn_printf("andi r%d,0x0F", reg);       // Assumes reg and extra_reg
        insn_printf("andi r%d,0xF0", extra_reg); // are high registers.
        insn_printf("or r%d,r%d", reg, extra_reg);
        release_reg(&extra_reg);
    }
}

// Extracts two parts from the state as bytes.  Both parts are
// assumed to be within the same 3-byte region within the state
// and that they don't overlap.  We also assume that bit1 > bit2.
static void extract_two_parts(int reg1, int reg2, int bit1, int bit2)
{
    const LFSR *lfsr = find_lfsr(bit1);
    int offset, count;
    bit1 -= lfsr->start;
    bit2 -= lfsr->start;
    offset = lfsr->offsetl + lfsr_offset + (bit2 / 8);
    insn_printf("ldd r%d,Z+%d", reg1, offset + 2);
    insn_printf("ldd r%d,Z+%d", reg2, offset + 1);
    insn_printf("ldd r%d,Z+%d", temp_reg, offset);
    count = 8 - (bit1 % 8);
    shift_left_3_regs(reg1, reg2, temp_reg, count);
    count = bit1 - (bit2 + 8);
    shift_left_2_regs(reg2, temp_reg, count);
}

// Extract out various sub-parts of the state as 8-bit bytes.
// We do this by extracting two bytes around the one we want
// and then shifting it left or right until it is byte-aligned.
// Sometimes there is overlap and we can extract 3 bytes and shift.
static void extract_sub_parts(void)
{
    // LFSR6
    alloc_low_reg(&s244);
    alloc_low_reg(&s235);
    extract_two_parts(s244, s235, 244, 235);

    // LFSR5
    alloc_low_reg(&s196);
    extract_one_part(s196, 196);

    // LFSR4
    alloc_low_reg(&s160);
    extract_one_part(s160, 160);

    // LFSR3
    alloc_high_reg(&s111);
    extract_one_part(s111, 111);

    // LFSR2
    alloc_low_reg(&s66);
    extract_one_part(s66, 66);

    // LFSR1
    alloc_low_reg(&s23);
    alloc_low_reg(&s12);
    extract_two_parts(s23, s12, 23, 12);
}

// Update the LFSR's.
static void update_lfsrs(void)
{
    int offset;

    // LFSR7: if the offset is non-zero then we still have the s7_l
    // value from a previous shift_down_step() call in a register.
    if (lfsr_offset == 0) {
        alloc_low_reg(&s7_l);
        insn_printf("ldd r%d,Z+%d", s7_l, get_lfsr(6)->offsetl + lfsr_offset);
    }
    insn_printf("eor r%d,r%d", s7_l, s235);
    alloc_low_reg(&s6_l);
    insn_printf("ldd r%d,Z+%d", s6_l, get_lfsr(5)->offsetl + lfsr_offset);
    insn_printf("eor r%d,r%d", s7_l, s6_l);

    // LFSR6
    alloc_low_reg(&s5_l);
    insn_printf("eor r%d,r%d", s6_l, s196);
    insn_printf("ldd r%d,Z+%d", s5_l, get_lfsr(4)->offsetl + lfsr_offset);
    insn_printf("eor r%d,r%d", s6_l, s5_l);

    // LFSR5
    alloc_low_reg(&s4_l);
    insn_printf("eor r%d,r%d", s5_l, s160);
    insn_printf("ldd r%d,Z+%d", s4_l, get_lfsr(3)->offsetl + lfsr_offset);
    insn_printf("eor r%d,r%d", s5_l, s4_l);

    // LFSR4
    alloc_low_reg(&s3_l);
    insn_printf("eor r%d,r%d", s4_l, s111);
    insn_printf("ldd r%d,Z+%d", s3_l, get_lfsr(2)->offsetl + lfsr_offset);
    insn_printf("eor r%d,r%d", s4_l, s3_l);

    // LFSR3
    alloc_low_reg(&s2_l);
    insn_printf("eor r%d,r%d", s3_l, s66);
    insn_printf("ldd r%d,Z+%d", s2_l, get_lfsr(1)->offsetl + lfsr_offset);
    insn_printf("eor r%d,r%d", s3_l, s2_l);

    // LFSR2
    alloc_low_reg(&s1_l);
    insn_printf("eor r%d,r%d", s2_l, s23);
    offset = get_lfsr(0)->offsetl + lfsr_offset;
    if (offset != 0)
        insn_printf("ldd r%d,Z+%d", s1_l, offset);
    else
        insn_printf("ld r%d,Z", s1_l);
    insn_printf("eor r%d,r%d", s2_l, s1_l);
}

// Generate the next 8 keystream bits.
static void generate_keystream(void)
{
    // ks = s12 ^ state->s4_l ^
    //      maj(s235, state->s2_l, state->s5_l) ^
    //      ch(state->s6_l, s111, s66);
    ks = s12;
    s12 = -1;
    insn_printf("eor r%d,r%d", ks, s4_l);

    // ks ^= maj(s235, state->s2_l, state->s5_l)
    insn_printf("mov r%d,r%d", temp_reg, s235); // ks ^= (s235 & s2_l)
    insn_printf("and r%d,r%d", temp_reg, s2_l);
    insn_printf("eor r%d,r%d", ks, temp_reg);
    insn_printf("and r%d,r%d", s235, s5_l);     // ks ^= (s235 & s5_l)
    insn_printf("eor r%d,r%d", ks, s235);
    insn_printf("mov r%d,r%d", temp_reg, s2_l); // ks ^= (s2_l & s5_l)
    insn_printf("and r%d,r%d", temp_reg, s5_l);
    insn_printf("eor r%d,r%d", ks, temp_reg);
    release_reg(&s235);

    // ks ^= ch(state->s6_l, s111, s66)
    insn_printf("and r%d,r%d", s111, s6_l);
    insn_printf("eor r%d,r%d", ks, s111);
    insn_printf("mov r%d,r%d", temp_reg, s6_l);
    insn_printf("com r%d", temp_reg);
    insn_printf("and r%d,r%d", s66, temp_reg);
    insn_printf("eor r%d,r%d", ks, s66);
    release_reg(&s111);
    release_reg(&s66);
}

// Generate the next 8 non-linear feedback bits.
static void generate_feedback(int input_is_ciphertext)
{
    // f = state->s1_l ^ (~state->s3_l) ^
    //     maj(s244, s23, s160) ^ (ca & s196) ^ (cb & ks);
    // f ^= plaintext;
    alloc_high_reg(&f); // Needs to be a high register for shift_down().
    insn_printf("mov r%d,r%d", f, s3_l);
    insn_printf("com r%d", f);
    insn_printf("eor r%d,r%d", f, s1_l);
    release_reg(&s1_l); // Don't need the low byte of s1 any more.

    // f ^= maj(s244, s23, s160)
    insn_printf("mov r%d,r%d", temp_reg, s244); // f ^= (s244 & s23)
    insn_printf("and r%d,r%d", temp_reg, s23);
    insn_printf("eor r%d,r%d", f, temp_reg);
    insn_printf("and r%d,r%d", s23, s160);      // f ^= (s23 & s160)
    insn_printf("eor r%d,r%d", f, s23);
    insn_printf("and r%d,r%d", s244, s160);     // f ^= (s244 & s160)
    insn_printf("eor r%d,r%d", f, s244);
    release_reg(&s244);
    release_reg(&s23);
    release_reg(&s160);

    // f ^= (ca & s196). Note that when decrypting, ca is always 1.
    if (!input_is_ciphertext)
        insn_printf("and r%d,%%3", s196);       // s196 &= ca
    insn_printf("eor r%d,r%d", f, s196);        // f ^= s196
    release_reg(&s196);

    // f ^= (cb & ks) ^ plaintext
    // If we are processing the ciphertext, then we need to first
    // decrypt the input with ks.  We leave the plaintext in "ks".
    // Note that when decrypting, cb is always 0.
    if (!is_32bit_version) {
        // Plaintext and ciphertext are 8-bit values in registers.
        if (input_is_ciphertext) {
            insn_printf("eor r%d,%%2", ks);     // plaintext = ciphertext ^ ks
            insn_printf("eor r%d,r%d", f, ks);  // f ^= plaintext
        } else {
            insn_printf("mov r%d,r%d", temp_reg, ks);   // f ^= (cb & ks)
            insn_printf("and r%d,%%4", temp_reg);
            insn_printf("eor r%d,r%d", f, temp_reg);
            insn_printf("eor r%d,%%2", f);              // f ^= plaintext
        }
    } else {
        // Plaintext and ciphertext are 32-bit values in the stack frame.
        static const char * const out_regs[] = {"%A0", "%B0", "%C0", "%D0"};
        static const char * const in_regs[]  = {"%A2", "%B2", "%C2", "%D2"};
        if (input_is_ciphertext) {
            insn_printf("ldd r%d,%s", temp_reg, in_regs[lfsr_offset]);
            insn_printf("eor r%d,r%d", ks, temp_reg);
            insn_printf("eor r%d,r%d", f, ks);
            insn_printf("std %s,r%d", out_regs[lfsr_offset], ks);
        } else {
            insn_printf("mov r%d,r%d", temp_reg, ks);
            insn_printf("and r%d,%%4", temp_reg);
            insn_printf("eor r%d,r%d", f, temp_reg);
            insn_printf("ldd r%d,%s", temp_reg, in_regs[lfsr_offset]);
            insn_printf("eor r%d,r%d", f, temp_reg);
            insn_printf("eor r%d,r%d", temp_reg, ks);
            insn_printf("std %s,r%d", out_regs[lfsr_offset], temp_reg);
        }
        release_reg(&ks);
    }
}

// Shift a LFSR downwards by 8 bits and rotate in a register.
static void shift_down_lfsr(const LFSR *lfsr, int reg)
{
    int bit;
    int offset1;
    int offset2;
    int extra = -1;

    // Shift all bytes but the last down.  We assume that the low byte
    // is already cached in a register from the update_lfsrs() function
    // so we don't need to worry about saving it away now.
    for (bit = 0; bit < (lfsr->len - 16); bit += 8) {
        if (bit < 32)
            offset2 = lfsr->offsetl + (bit / 8);
        else
            offset2 = lfsr->offseth + ((bit - 32) / 8);
        if ((bit + 8) < 32)
            offset1 = lfsr->offsetl + ((bit + 8) / 8);
        else
            offset1 = lfsr->offseth + ((bit + 8 - 32) / 8);
        insn_printf("ldd r%d,Z+%d", temp_reg, offset1);
        if (offset2 != 0)
            insn_printf("std Z+%d,r%d", offset2, temp_reg);
        else
            insn_printf("st Z,r%d", temp_reg);
    }

    // Rotate the register value from the next-higher LFSR into the high byte.
    if (bit < 32)
        offset2 = lfsr->offsetl + (bit / 8);
    else
        offset2 = lfsr->offseth + ((bit - 32) / 8);
    if ((bit + 8) < 32)
        offset1 = lfsr->offsetl + ((bit + 8) / 8);
    else
        offset1 = lfsr->offseth + ((bit + 8 - 32) / 8);
    insn_printf("ldd r%d,Z+%d", temp_reg, offset1);
    alloc_low_reg(&extra);
    bit = lfsr->len % 8;
    if (bit <= 4) {
        insn_printf("clr r%d", extra);
        shift_left_2_regs(extra, reg, bit);
        insn_printf("or r%d,r%d", temp_reg, reg);
        insn_printf("std Z+%d,r%d", offset2, temp_reg);
        insn_printf("std Z+%d,r%d", offset1, extra);
    } else {
        insn_printf("clr r%d", extra);
        shift_right_2_regs(reg, extra, 8 - bit);
        insn_printf("or r%d,r%d", temp_reg, extra);
        insn_printf("std Z+%d,r%d", offset2, temp_reg);
        insn_printf("std Z+%d,r%d", offset1, reg);
    }
    release_reg(&extra);
}

// Shift the state downwards by 8 bits.
static void shift_down(void)
{
    int extra = -1;

    // LFSR7: s7_l ^= (f << 4), state->s7 = (f >> 4)
    alloc_high_reg(&extra);
    check_high_reg(&f);
    insn_printf("swap r%d", f);
    insn_printf("mov r%d,r%d", extra, f);
    insn_printf("andi r%d,0xF0", extra);    // Assumes extra is a high reg.
    insn_printf("eor r%d,r%d", s7_l, extra);
    insn_printf("andi r%d,0x0F", f);        // Assumes f is a high reg.
    insn_printf("std Z+%d,r%d", get_lfsr(6)->offsetl, f);
    release_reg(&f);
    release_reg(&extra);

    // LFSR6 down to LFSR1
    shift_down_lfsr(get_lfsr(5), s7_l);
    release_reg(&s7_l);
    shift_down_lfsr(get_lfsr(4), s6_l);
    release_reg(&s6_l);
    shift_down_lfsr(get_lfsr(3), s5_l);
    release_reg(&s5_l);
    shift_down_lfsr(get_lfsr(2), s4_l);
    release_reg(&s4_l);
    shift_down_lfsr(get_lfsr(1), s3_l);
    release_reg(&s3_l);
    shift_down_lfsr(get_lfsr(0), s2_l);
    release_reg(&s2_l);
}

// Shift the state downwards by 8 bits as one step within a 32-bit word.
static void shift_down_step(void)
{
    int extra = -1;
    int s7_l_next = -1;

    // LFSR7: s7_l ^= (f << 4), state->s7 = (f >> 4)
    // We keep the previous s7_l in a register for now and
    // make use of it during shift_down_final().  Allocate
    // a new register to hold the next s7_l value.
    s7_l_prev[lfsr_offset] = s7_l;
    alloc_high_reg(&extra);
    check_high_reg(&f);
    alloc_low_reg(&s7_l_next);
    insn_printf("swap r%d", f);
    insn_printf("mov r%d,r%d", extra, f);
    insn_printf("andi r%d,0xF0", extra);    // Assumes extra is a high reg.
    insn_printf("eor r%d,r%d", s7_l, extra);
    insn_printf("andi r%d,0x0F", f);        // Assumes f is a high reg.
    insn_printf("mov r%d,r%d", s7_l_next, f);
    release_reg(&f);
    release_reg(&extra);
    s7_l = s7_l_next;

    // Write s2 to s6 back to the state.  We don't need to store s1
    // because shift_down_final() will be throwing the value away.
    // The generate_feedback() function already released the register.
    insn_printf("std Z+%d,r%d", get_lfsr(5)->offsetl + lfsr_offset, s6_l);
    release_reg(&s6_l);
    insn_printf("std Z+%d,r%d", get_lfsr(4)->offsetl + lfsr_offset, s5_l);
    release_reg(&s5_l);
    insn_printf("std Z+%d,r%d", get_lfsr(3)->offsetl + lfsr_offset, s4_l);
    release_reg(&s4_l);
    insn_printf("std Z+%d,r%d", get_lfsr(2)->offsetl + lfsr_offset, s3_l);
    release_reg(&s3_l);
    insn_printf("std Z+%d,r%d", get_lfsr(1)->offsetl + lfsr_offset, s2_l);
    release_reg(&s2_l);
}

// Shift the state downwards by 8 bits as the final step within a 32-bit word.
static void shift_down_final(void)
{
    int extra = -1;
    int extra2 = -1;
    int extra3 = -1;
    int extra4 = -1;
    const LFSR *lfsr;

    // LFSR7: s7_l ^= (f << 4), state->s7 = (f >> 4)
    alloc_high_reg(&extra);
    check_high_reg(&f);
    insn_printf("swap r%d", f);
    insn_printf("mov r%d,r%d", extra, f);
    insn_printf("andi r%d,0xF0", extra);    // Assumes extra is a high reg.
    insn_printf("eor r%d,r%d", s7_l, extra);
    insn_printf("andi r%d,0x0F", f);        // Assumes f is a high reg.
    insn_printf("std Z+%d,r%d", get_lfsr(6)->offsetl, f);
    release_reg(&f);

    // We currently have the 32-bit s7 word in four registers:
    // s7_l_prev[0], s7_l_prev[1], s7_l_prev[2], and s7_l.
    // We also have the third byte of s2..s6 in the s2_l..s6_l regs.
    // Everything else is stored within the Acorn128 state structure.

    // LFSR1: state->s1_l = state->s1_h | (state->s2_l << 29)
    //        state->s1_h = state->s2_l >> 3
    lfsr = get_lfsr(1);
    alloc_low_reg(&extra2);
    alloc_low_reg(&extra3);
    alloc_low_reg(&extra4);
    insn_printf("clr r%d", temp_reg);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offsetl);  // load s2_l[0..2]
    insn_printf("ldd r%d,Z+%d", extra2, lfsr->offsetl + 1);
    insn_printf("ldd r%d,Z+%d", extra3, lfsr->offsetl + 2);
    shift_right_5_regs(s2_l, extra3, extra2, extra, temp_reg, 3);
    lfsr = get_lfsr(0);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth);
    insn_printf("st Z,r%d", extra4); // offset is zero.
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth + 1);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, extra4);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth + 2);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra4);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth + 3);
    insn_printf("or r%d,r%d", extra4, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offseth, extra);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 1, extra2);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 2, extra3);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 3, s2_l);
    release_reg(&s2_l);

    // LFSR2: state->s2_l = state->s2_h | (state->s3_l << 14)
    //        state->s2_h = state->s3_l >> 18
    lfsr = get_lfsr(2);
    insn_printf("clr r%d", temp_reg);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offsetl);  // load s3_l[0..2]
    insn_printf("ldd r%d,Z+%d", extra2, lfsr->offsetl + 1);
    insn_printf("ldd r%d,Z+%d", extra3, lfsr->offsetl + 2);
    shift_right_5_regs(s3_l, extra3, extra2, extra, temp_reg, 2);
    lfsr = get_lfsr(1);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth); // s2_h[0]
    insn_printf("std Z+%d,r%d", lfsr->offsetl, extra4);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth + 1); // s2_h[1]
    insn_printf("or r%d,r%d", temp_reg, extra4);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, extra2);
    insn_printf("std Z+%d,r%d", lfsr->offseth, extra3);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 1, s3_l);
    release_reg(&s3_l);

    // LFSR3: state->s3_l = state->s3_h | (state->s4_l << 15)
    //        state->s3_h = state->s4_l >> 17
    lfsr = get_lfsr(3);
    insn_printf("clr r%d", temp_reg);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offsetl);  // load s4_l[0..2]
    insn_printf("ldd r%d,Z+%d", extra2, lfsr->offsetl + 1);
    insn_printf("ldd r%d,Z+%d", extra3, lfsr->offsetl + 2);
    shift_right_5_regs(s4_l, extra3, extra2, extra, temp_reg, 1);
    lfsr = get_lfsr(2);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth); // s3_h[0]
    insn_printf("std Z+%d,r%d", lfsr->offsetl, extra4);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth + 1); // s3_h[1]
    insn_printf("or r%d,r%d", temp_reg, extra4);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, extra2);
    insn_printf("std Z+%d,r%d", lfsr->offseth, extra3);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 1, s4_l);
    release_reg(&s4_l);

    // LFSR4: state->s4_l = state->s4_h | (state->s5_l << 7)
    //        state->s4_h = state->s5_l >> 25
    lfsr = get_lfsr(4);
    insn_printf("clr r%d", temp_reg);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offsetl);  // load s5_l[0..2]
    insn_printf("ldd r%d,Z+%d", extra2, lfsr->offsetl + 1);
    insn_printf("ldd r%d,Z+%d", extra3, lfsr->offsetl + 2);
    shift_right_5_regs(s5_l, extra3, extra2, extra, temp_reg, 1);
    lfsr = get_lfsr(3);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth); // s4_h
    insn_printf("or r%d,r%d", temp_reg, extra4);
    insn_printf("std Z+%d,r%d", lfsr->offsetl, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, extra);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra2);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, extra3);
    insn_printf("std Z+%d,r%d", lfsr->offseth, s5_l);
    release_reg(&s5_l);

    // LFSR5: state->s5_l = state->s5_h | (state->s6_l << 5)
    //        state->s5_h = state->s6_l >> 27
    lfsr = get_lfsr(5);
    insn_printf("clr r%d", temp_reg);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offsetl);  // load s6_l[0..2]
    insn_printf("ldd r%d,Z+%d", extra2, lfsr->offsetl + 1);
    insn_printf("ldd r%d,Z+%d", extra3, lfsr->offsetl + 2);
    shift_right_5_regs(s6_l, extra3, extra2, extra, temp_reg, 3);
    lfsr = get_lfsr(4);
    insn_printf("ldd r%d,Z+%d", extra4, lfsr->offseth); // s5_h
    insn_printf("or r%d,r%d", temp_reg, extra4);
    insn_printf("std Z+%d,r%d", lfsr->offsetl, temp_reg);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, extra);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra2);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, extra3);
    insn_printf("std Z+%d,r%d", lfsr->offseth, s6_l);
    release_reg(&s6_l);

    // LFSR6: state->s6_l = state->s6_h | (s7_l << 27)
    //        state->s6_h = s7_l >> 5
    lfsr = get_lfsr(5);
    insn_printf("clr r%d", temp_reg);
    shift_left_5_regs
        (temp_reg, s7_l, s7_l_prev[2], s7_l_prev[1], s7_l_prev[0], 3);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offseth);
    insn_printf("std Z+%d,r%d", lfsr->offsetl, extra);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offseth + 1);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 1, extra);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offseth + 2);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 2, extra);
    insn_printf("ldd r%d,Z+%d", extra, lfsr->offseth + 3);
    insn_printf("or r%d,r%d", extra, s7_l_prev[0]);
    insn_printf("std Z+%d,r%d", lfsr->offsetl + 3, extra);
    insn_printf("std Z+%d,r%d", lfsr->offseth, s7_l_prev[1]);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 1, s7_l_prev[2]);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 2, s7_l);
    insn_printf("std Z+%d,r%d", lfsr->offseth + 3, temp_reg);
    release_reg(&s7_l_prev[0]);
    release_reg(&s7_l_prev[1]);
    release_reg(&s7_l_prev[2]);
    release_reg(&s7_l);

    // Release temporary registers.
    release_reg(&extra);
    release_reg(&extra2);
    release_reg(&extra3);
    release_reg(&extra4);
}

static void encrypt8(void)
{
    // Print the function header.
    printf("static uint8_t acornEncrypt8\n");
    printf("    (Acorn128State *state, uint8_t plaintext, uint8_t ca, uint8_t cb)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genacorn tool.\n");
    indent_printf("uint8_t ciphertext;\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Output the body of the function.
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(0);
    shift_down();

    // Generate the final ciphertext.
    insn_printf("mov %%0,%%2");
    insn_printf("eor %%0,r%d", ks);
    release_reg(&ks);

    // Declare the registers that we need.
    indent_printf(": \"=r\"(ciphertext)\n");
    indent_printf(": \"z\"(&state->s1_l), \"r\"(plaintext), \"r\"((uint8_t)ca), \"r\"((uint8_t)cb)\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    indent_printf("return ciphertext;\n");
    printf("}\n\n");
    check_regs();
}

static void decrypt8(void)
{
    // Print the function header.
    printf("static uint8_t acornDecrypt8(Acorn128State *state, uint8_t ciphertext)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genacorn tool.\n");
    indent_printf("uint8_t plaintext;\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Output the body of the function.
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(1);
    shift_down();

    // Generate the final plaintext.
    insn_printf("mov %%0,r%d", ks);
    release_reg(&ks);

    // Declare the registers that we need.
    indent_printf(": \"=r\"(plaintext)\n");
    indent_printf(": \"z\"(&state->s1_l), \"r\"(ciphertext)\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    indent_printf("return plaintext;\n");
    printf("}\n\n");
    check_regs();
}

static void encrypt32(void)
{
    // Print the function header.
    printf("uint32_t acornEncrypt32\n");
    printf("    (Acorn128State *state, uint32_t plaintext, uint32_t ca, uint32_t cb)\n");
    printf("{\n");
    indent_printf("// Automatically generated by the genacorn tool.\n");
    indent_printf("uint32_t ciphertext;\n");
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;

    // Output the body of the function as 4 rounds for each byte in the word.
    // The shift-down step is delayed until after all 4 rounds are complete.
    lfsr_offset = 0;
    is_32bit_version = 1;
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(0);
    shift_down_step();
    lfsr_offset = 1;
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(0);
    shift_down_step();
    lfsr_offset = 2;
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(0);
    shift_down_step();
    lfsr_offset = 3;
    extract_sub_parts();
    update_lfsrs();
    generate_keystream();
    generate_feedback(0);
    shift_down_final();
    lfsr_offset = 0;
    is_32bit_version = 0;

    // Declare the registers that we need.
    indent_printf(": \"=Q\"(ciphertext)\n");
    indent_printf(": \"z\"(&state->s1_l), \"Q\"(plaintext), \"r\"((uint8_t)ca), \"r\"((uint8_t)cb)\n");
    temp_regs();
    indent -= 4;
    indent_printf(");\n");
    indent_printf("return ciphertext;\n");
    printf("}\n\n");
    check_regs();
}

int main(int argc, char *argv[])
{
    encrypt8();
    decrypt8();
    encrypt32();
    return 0;
}
