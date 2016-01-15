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

// Special-purpose compiler that generates the AVR version of KeccakCore.

#include <stdio.h>
#include <stdarg.h>

// 1 to inline rotates, 0 to call to helper functions.
// 0 gives a smaller code size at a slight performance cost.
static int inline_rotates = 1;

static int indent = 4;

static int t_reg = 8;       // Temporary 64-bit value (any reg).

static int x_reg = 26;
static int y_reg = 28;
static int z_reg = 30;

static int const_reg = 21;  // For temporary constants (must be a high reg).

static int loop1_reg = 20;  // For keeping track of loop counters (high regs).
static int loop2_reg = 19;
static int loop3_reg = 18;

static int save1_reg = 17;  // Save registers (any reg).
static int save2_reg = 16;

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

void leftRotate1(int reg)
{
    insn_printf("lsl r%d", reg);
    insn_printf("rol r%d", reg + 1);
    insn_printf("rol r%d", reg + 2);
    insn_printf("rol r%d", reg + 3);
    insn_printf("rol r%d", reg + 4);
    insn_printf("rol r%d", reg + 5);
    insn_printf("rol r%d", reg + 6);
    insn_printf("rol r%d", reg + 7);
    insn_printf("adc r%d, __zero_reg__", reg);
}

void rightRotate1(int reg)
{
    insn_printf("bst r%d,0", reg);
    insn_printf("ror r%d", reg + 7);
    insn_printf("ror r%d", reg + 6);
    insn_printf("ror r%d", reg + 5);
    insn_printf("ror r%d", reg + 4);
    insn_printf("ror r%d", reg + 3);
    insn_printf("ror r%d", reg + 2);
    insn_printf("ror r%d", reg + 1);
    insn_printf("ror r%d", reg);
    insn_printf("bld r%d,7", reg + 7);
}

void adjust_pointer_reg(int reg, int delta)
{
    if (delta >= 64) {
        insn_printf("ldi %d,%d", const_reg, delta & 0xFF);
        insn_printf("add r%d,%d", reg, const_reg);
        if ((delta >> 8) != 0) {
            insn_printf("ldi %d,%d", const_reg, (delta >> 8) & 0xFF);
            insn_printf("adc r%d,r%d", reg + 1, const_reg);
        } else {
            insn_printf("adc r%d,__zero_reg__", reg + 1);
        }
    } else if (delta > 0) {
        insn_printf("adiw r%d,%d", reg, delta);
    } else if (delta <= -64) {
        delta = -delta;
        insn_printf("subi r%d,%d", reg, delta & 0xFF);
        if ((delta >> 8) != 0) {
            insn_printf("sbci r%d,%d", reg + 1, (delta >> 8) & 0xFF);
        } else {
            insn_printf("sbc r%d,__zero_reg__", reg + 1);
        }
    } else if (delta < 0) {
        insn_printf("sbiw r%d,%d", reg, -delta);
    }
}

void load64_from_z(int reg, int offset)
{
    if (offset == 0)
        insn_printf("ld r%d,Z", reg);
    else
        insn_printf("ldd r%d,Z+%d", reg, offset);
    insn_printf("ldd r%d,Z+%d", reg + 1, offset + 1);
    insn_printf("ldd r%d,Z+%d", reg + 2, offset + 2);
    insn_printf("ldd r%d,Z+%d", reg + 3, offset + 3);
    insn_printf("ldd r%d,Z+%d", reg + 4, offset + 4);
    insn_printf("ldd r%d,Z+%d", reg + 5, offset + 5);
    insn_printf("ldd r%d,Z+%d", reg + 6, offset + 6);
    insn_printf("ldd r%d,Z+%d", reg + 7, offset + 7);
}

void load64_from_y(int reg, int offset)
{
    if (offset == 0)
        insn_printf("ld r%d,Y", reg);
    else
        insn_printf("ldd r%d,Y+%d", reg, offset);
    insn_printf("ldd r%d,Y+%d", reg + 1, offset + 1);
    insn_printf("ldd r%d,Y+%d", reg + 2, offset + 2);
    insn_printf("ldd r%d,Y+%d", reg + 3, offset + 3);
    insn_printf("ldd r%d,Y+%d", reg + 4, offset + 4);
    insn_printf("ldd r%d,Y+%d", reg + 5, offset + 5);
    insn_printf("ldd r%d,Y+%d", reg + 6, offset + 6);
    insn_printf("ldd r%d,Y+%d", reg + 7, offset + 7);
}

void load64_from_z_combine(const char *op, int reg, int offset)
{
    int posn;
    for (posn = 0; posn < 8; ++posn, ++offset, ++reg) {
        if (offset == 0)
            insn_printf("ld __tmp_reg__,Z");
        else
            insn_printf("ldd __tmp_reg__,Z+%d", offset);
        insn_printf("%s r%d,__tmp_reg__", op, reg);
    }
}

void load64_from_y_combine(const char *op, int reg, int offset)
{
    int posn;
    for (posn = 0; posn < 8; ++posn, ++offset, ++reg) {
        if (offset == 0)
            insn_printf("ld __tmp_reg__,Y");
        else
            insn_printf("ldd __tmp_reg__,Y+%d", offset);
        insn_printf("%s r%d,__tmp_reg__", op, reg);
    }
}

void combine64_with_z(const char *op, int reg)
{
    int posn;
    for (posn = 0; posn < 8; ++posn, ++reg) {
        if (posn == 0)
            insn_printf("ld __tmp_reg__,Z");
        else
            insn_printf("ldd __tmp_reg__,Z+%d", posn);
        insn_printf("%s __tmp_reg__,r%d", op, reg);
        if (posn == 0)
            insn_printf("st Z,__tmp_reg__");
        else
            insn_printf("std Z+%d,__tmp_reg__", posn);
    }
}

void load64_from_x(int reg)
{
    insn_printf("ld r%d,X+", reg);
    insn_printf("ld r%d,X+", reg + 1);
    insn_printf("ld r%d,X+", reg + 2);
    insn_printf("ld r%d,X+", reg + 3);
    insn_printf("ld r%d,X+", reg + 4);
    insn_printf("ld r%d,X+", reg + 5);
    insn_printf("ld r%d,X+", reg + 6);
    insn_printf("ld r%d,X+", reg + 7);
}

void store64_to_x(int reg)
{
    insn_printf("st X+,r%d", reg);
    insn_printf("st X+,r%d", reg + 1);
    insn_printf("st X+,r%d", reg + 2);
    insn_printf("st X+,r%d", reg + 3);
    insn_printf("st X+,r%d", reg + 4);
    insn_printf("st X+,r%d", reg + 5);
    insn_printf("st X+,r%d", reg + 6);
    insn_printf("st X+,r%d", reg + 7);
}

void theta(void)
{
    int index;

    printf("\n");
    indent_printf("// Step mapping theta.  Compute C.\n");
    insn_printf("ldi r%d,5", loop1_reg);
    insn_printf("100:");

    // Load A[0][index] into t_reg.
    load64_from_z(t_reg, 0);

    // XOR with A[1][index] .. A[4][index]
    insn_printf("ldi r%d,4", loop2_reg);
    insn_printf("101:");
    adjust_pointer_reg(z_reg, 40);
    load64_from_z_combine("eor", t_reg, 0);
    insn_printf("dec r%d", loop2_reg);
    insn_printf("brne 101b");

    // Store into B[0][index].
    store64_to_x(t_reg);

    // End of the outer loop.
    adjust_pointer_reg(z_reg, -(160 - 8));
    insn_printf("dec r%d", loop1_reg);
    insn_printf("brne 100b");
    adjust_pointer_reg(z_reg, -40);
    adjust_pointer_reg(x_reg, -40);

    // Generate D[index] and XOR with every A[x][index] element.
    // To make this easier, we know that the original X value is also
    // in Y so we can use offsets relative to Y for the first row of B.
    printf("\n");
    indent_printf("// Step mapping theta.  Compute D and XOR with A.\n");
    for (index = 0; index < 5; ++index) {
        // Compute D[index] and put it into t_reg.
        load64_from_y(t_reg, ((index + 1) % 5) * 8);
        leftRotate1(t_reg);
        load64_from_y_combine("eor", t_reg, ((index + 4) % 5) * 8);

        // XOR the computed D[index] with all A[x][index] elements.
        insn_printf("ldi r%d,5", loop2_reg);
        insn_printf("%d:", 103 + index);
        combine64_with_z("eor", t_reg);
        adjust_pointer_reg(z_reg, 40);
        insn_printf("dec r%d", loop2_reg);
        insn_printf("brne %db", 103 + index);
        if (index != 4)
            adjust_pointer_reg(z_reg, -(200 - 8));
        else
            adjust_pointer_reg(z_reg, -(200 + 40 - 8));
    }
}

void rho_pi(void)
{
    typedef struct {
        int x, y, rot;
    } map;
    static map const Bmap[5][5] = { // indexed by y, x
        {
            {0, 0,  0},         // B[0][0]
            {0, 3, 28},         // B[1][0]
            {0, 1,  1},         // B[2][0]
            {0, 4, 27},         // B[3][0]
            {0, 2, 62}          // B[4][0]
        },
        {
            {1, 1, 44},         // B[0][1]
            {1, 4, 20},         // B[1][1]
            {1, 2,  6},         // B[2][1]
            {1, 0, 36},         // B[3][1]
            {1, 3, 55}          // B[4][1]
        },
        {
            {2, 2, 43},         // B[0][2]
            {2, 0,  3},         // B[1][2]
            {2, 3, 25},         // B[2][2]
            {2, 1, 10},         // B[3][2]
            {2, 4, 39}          // B[4][2]
        },
        {
            {3, 3, 21},         // B[0][3]
            {3, 1, 45},         // B[1][3]
            {3, 4,  8},         // B[2][3]
            {3, 2, 15},         // B[3][3]
            {3, 0, 41}          // B[4][3]
        },
        {
            {4, 4, 14},         // B[0][4]
            {4, 2, 61},         // B[1][4]
            {4, 0, 18},         // B[2][4]
            {4, 3, 56},         // B[3][4]
            {4, 1,  2}          // B[4][4]
        }
    };
    int Boffset = 0;
    int Aoffset = 0;
    int offset;
    int Bx, By, Ax, Ay, rot, adjust;
    printf("\n");
    indent_printf("// Step mappings rho and pi combined into one step.\n");
    for (By = 0; By < 5; ++By) {
        for (Bx = 0; Bx < 5; ++Bx) {
            // What do we need to load?
            Ax = Bmap[By][Bx].x;
            Ay = Bmap[By][Bx].y;
            rot = Bmap[By][Bx].rot;

            // Heading for this step.
            printf("\n");
            if (rot != 0) {
                indent_printf("// B[%d][%d] = leftRotate%d_64(A[%d][%d])\n",
                              Bx, By, rot, Ax, Ay);
            } else {
                indent_printf("// B[%d][%d] = A[%d][%d]\n",
                              Bx, By, Ax, Ay);
            }

            // Adjust rX and rZ to point at the new locations.
            offset = (Bx * 5 + By) * 8;
            adjust_pointer_reg(x_reg, offset - Boffset);
            Boffset = offset;
            offset = Ax * 5 * 8;
            adjust_pointer_reg(z_reg, offset - Aoffset);
            Aoffset = offset;
            offset = Ay * 8;

            // Load the A/rZ value into the temp regs.
            load64_from_z(t_reg, offset);

            // Rotate.
            adjust = (8 - rot / 8) % 8;
            switch (rot % 8) {
            case 0:
                break;
            case 1:
                if (inline_rotates) {
                    leftRotate1(t_reg);
                } else {
                    insn_printf("call 11f");
                }
                break;
            case 2:
                if (inline_rotates) {
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                } else {
                    insn_printf("call 12f");
                }
                break;
            case 3:
                if (inline_rotates) {
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                } else {
                    insn_printf("call 13f");
                }
                break;
            case 4:
                if (inline_rotates) {
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                    leftRotate1(t_reg);
                } else {
                    insn_printf("call 14f");
                }
                break;
            case 5:
                if (inline_rotates) {
                    rightRotate1(t_reg);
                    rightRotate1(t_reg);
                    rightRotate1(t_reg);
                } else {
                    insn_printf("call 23f");
                }
                adjust = (adjust + 7) % 8;
                break;
            case 6:
                if (inline_rotates) {
                    rightRotate1(t_reg);
                    rightRotate1(t_reg);
                } else {
                    insn_printf("call 22f");
                }
                adjust = (adjust + 7) % 8;
                break;
            case 7:
                if (inline_rotates) {
                    rightRotate1(t_reg);
                } else {
                    insn_printf("call 21f");
                }
                adjust = (adjust + 7) % 8;
                break;
            default:
                break;
            }

            // Perform byte rotations and store into B/rX.
            insn_printf("st X+,r%d", t_reg + adjust);
            insn_printf("st X+,r%d", t_reg + (adjust + 1) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 2) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 3) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 4) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 5) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 6) % 8);
            insn_printf("st X+,r%d", t_reg + (adjust + 7) % 8);
            Boffset += 8;   // rX has advanced by 8 while doing this.
        }
    }

    // Return rX and rZ to their starting values for the next step mapping.
    adjust_pointer_reg(x_reg, -Boffset);
    adjust_pointer_reg(z_reg, -Aoffset);
}

void load8_from_y(int reg, int offset)
{
    if (offset == 0)
        insn_printf("ld r%d,Y", reg);
    else
        insn_printf("ldd r%d,Y+%d", reg, offset);
}

void store8_to_z(int reg, int offset)
{
    if (offset == 0)
        insn_printf("st Z,r%d", reg);
    else
        insn_printf("std Z+%d,r%d", offset, reg);
}

void chi(void)
{
    int index;

    // Step mapping chi.  A is pointed to by Z and B is pointed to by X/Y.
    //      A[index2][index] =
    //          B[index2][index] ^
    //          ((~B[index2][(index + 1) % 5]) &
    //           B[index2][(index + 2) % 5]);
    // We compute this using an interleaving method.  We load five bytes
    // from the 5 words in a row of B and then compute the 5 output bytes
    // from that and store.  Then we move onto the next 5 bytes of each row.
    int in1 = t_reg;
    int in2 = t_reg + 1;
    int in3 = t_reg + 2;
    int in4 = t_reg + 3;
    int in5 = t_reg + 4;
    int out1 = t_reg + 5;
    int out2 = t_reg + 6;
    int out3 = t_reg + 7;
    int out4 = save1_reg;
    int out5 = save2_reg;
    printf("\n");
    indent_printf("// Step mapping chi.\n");
    insn_printf("ldi r%d,5", loop1_reg);
    insn_printf("50:");
    for (index = 0; index < 8; ++index) {
        load8_from_y(in1, index);
        load8_from_y(in2, index + 8);
        load8_from_y(in3, index + 16);
        load8_from_y(in4, index + 24);
        load8_from_y(in5, index + 32);
        insn_printf("mov r%d,r%d", out1, in2);
        insn_printf("com r%d", out1);
        insn_printf("and r%d,r%d", out1, in3);
        insn_printf("eor r%d,r%d", out1, in1);
        insn_printf("mov r%d,r%d", out2, in3);
        insn_printf("com r%d", out2);
        insn_printf("and r%d,r%d", out2, in4);
        insn_printf("eor r%d,r%d", out2, in2);
        insn_printf("mov r%d,r%d", out3, in4);
        insn_printf("com r%d", out3);
        insn_printf("and r%d,r%d", out3, in5);
        insn_printf("eor r%d,r%d", out3, in3);
        insn_printf("mov r%d,r%d", out4, in5);
        insn_printf("com r%d", out4);
        insn_printf("and r%d,r%d", out4, in1);
        insn_printf("eor r%d,r%d", out4, in4);
        insn_printf("mov r%d,r%d", out5, in1);
        insn_printf("com r%d", out5);
        insn_printf("and r%d,r%d", out5, in2);
        insn_printf("eor r%d,r%d", out5, in5);
        store8_to_z(out1, index);
        store8_to_z(out2, index + 8);
        store8_to_z(out3, index + 16);
        store8_to_z(out4, index + 24);
        store8_to_z(out5, index + 32);
    }
    adjust_pointer_reg(z_reg, 5 * 8);
    adjust_pointer_reg(y_reg, 5 * 8);
    insn_printf("dec r%d", loop1_reg);
    insn_printf("breq 51f");
    insn_printf("rjmp 50b");
    insn_printf("51:");

    // Restore Y and Z.  We don't need this yet because chi() is the
    // last thing we do so there's no point resetting the registers.
    //adjust_pointer_reg(y_reg, -200);
    //adjust_pointer_reg(z_reg, -200);
}

void outer_loop(void)
{
    // Save Y and then copy X into Y.  This way we can use displacements
    // relative to the Y register which we cannot do with X.
    insn_printf("push r%d", y_reg + 1);
    insn_printf("push r%d", y_reg);
    insn_printf("mov r%d,r%d", y_reg, x_reg);
    insn_printf("mov r%d,r%d", y_reg + 1, x_reg + 1);

    // Output the step mappings.
    theta();
    rho_pi();
    chi();
    // iota();

    // Restore Y.
    insn_printf("pop r%d", y_reg);
    insn_printf("pop r%d", y_reg + 1);

    // Dump the rotation utilities.
    if (!inline_rotates) {
        insn_printf("rjmp 3f");
        printf("\n");
        indent_printf("// Left rotate by 4 bits\n");
        insn_printf("14:");
        leftRotate1(t_reg);
        indent_printf("// Left rotate by 3 bits\n");
        insn_printf("13:");
        leftRotate1(t_reg);
        indent_printf("// Left rotate by 2 bits\n");
        insn_printf("12:");
        leftRotate1(t_reg);
        indent_printf("// Left rotate by 1 bit\n");
        insn_printf("11:");
        leftRotate1(t_reg);
        insn_printf("ret");
        printf("\n");
        indent_printf("// Right rotate by 3 bits\n");
        insn_printf("23:");
        rightRotate1(t_reg);
        indent_printf("// Right rotate by 2 bits\n");
        insn_printf("22:");
        rightRotate1(t_reg);
        indent_printf("// Right rotate by 1 bit\n");
        insn_printf("21:");
        rightRotate1(t_reg);
        insn_printf("ret");
    }

    // End of assembly.
    printf("\n");
    indent_printf("// Done\n");
    if (!inline_rotates) {
        insn_printf("3:");
    }
}

int main(int argc, char *argv[])
{
    indent_printf("__asm__ __volatile__ (\n");
    indent += 4;
    outer_loop();
    indent_printf(": : \"x\"(state.B), \"z\"(state.A)\n");
    indent_printf(": \"r%d\", \"r%d\", \"r%d\", \"r%d\", "
                    "\"r%d\", \"r%d\", \"r%d\", \"r%d\",\n",
                  t_reg, t_reg + 1, t_reg + 2, t_reg + 3,
                  t_reg + 4, t_reg + 5, t_reg + 6, t_reg + 7);
    indent_printf("  \"r%d\", \"r%d\", \"r%d\", \"r%d\", "
                    "\"r%d\", \"r%d\", \"memory\"\n",
                  save2_reg, save1_reg, loop3_reg, loop2_reg,
                  loop1_reg, const_reg);
    indent -= 4;
    indent_printf(");\n");
    return 0;
}
