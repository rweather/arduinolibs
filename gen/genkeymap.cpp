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

// Generates the keymap state machine table for the Terminal class.

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "../libraries/Terminal/USBKeysExtra.h"

typedef struct {
    const char *sequence;
    int code;
} EscKey;
static EscKey const escKeys[] = {
    // Based on the key sequence tables from:
    // http://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-PC-Style-Function-Keys
    // http://aperiodic.net/phil/archives/Geekery/term-function-keys.html
    // Also some miscellaneous codes from other sources like the Linux console.

    // Cursor control keys.
    {"[A",          KEY_UP_ARROW},
    {"OA",          KEY_UP_ARROW},
    {"A",           KEY_UP_ARROW},
    {"[B",          KEY_DOWN_ARROW},
    {"OB",          KEY_DOWN_ARROW},
    {"B",           KEY_DOWN_ARROW},
    {"[C",          KEY_RIGHT_ARROW},
    {"OC",          KEY_RIGHT_ARROW},
    {"C",           KEY_RIGHT_ARROW},
    {"[D",          KEY_LEFT_ARROW},
    {"OD",          KEY_LEFT_ARROW},
    {"D",           KEY_LEFT_ARROW},
    {"[H",          KEY_HOME},
    {"OH",          KEY_HOME},
    {"[1~",         KEY_HOME},
    {"[F",          KEY_END},
    {"OF",          KEY_END},
    {"[4~",         KEY_END},
    {"[2~",         KEY_INSERT},
    {"[3~",         KEY_DELETE},
    {"[5~",         KEY_PAGE_UP},
    {"[6~",         KEY_PAGE_DOWN},

    // Numeric keypad.  Mostly mapped back to ASCII.
    {"O ",          ' '},
    {"? ",          ' '},
    {"OI",          KEY_TAB},
    {"?I",          KEY_TAB},
    {"OM",          KEY_RETURN},
    {"?M",          KEY_RETURN},
    {"Oj",          '*'},
    {"?j",          '*'},
    {"Ok",          '+'},
    {"?k",          '+'},
    {"Ol",          ','},
    {"?l",          ','},
    {"Om",          '-'},
    {"?m",          '-'},
    {"On",          '.'},
    {"?n",          '.'},
    {"Oo",          '/'},
    {"?o",          '/'},
    {"Op",          '0'},
    {"?p",          '0'},
    {"Oq",          '1'},
    {"?q",          '1'},
    {"Or",          '2'},
    {"?r",          '2'},
    {"Os",          '3'},
    {"?s",          '3'},
    {"Ot",          '4'},
    {"?t",          '4'},
    {"Ou",          '5'},
    {"?u",          '5'},
    {"Ov",          '6'},
    {"?v",          '6'},
    {"Ow",          '7'},
    {"?w",          '7'},
    {"Ox",          '8'},
    {"?x",          '8'},
    {"Oy",          '9'},
    {"?y",          '9'},
    {"OX",          '='},
    {"?X",          '='},

    // Function keys.
    {"[11~",        KEY_F1},
    {"P",           KEY_F1},
    {"OP",          KEY_F1},
    {"[[A",         KEY_F1},
    {"[12~",        KEY_F2},
    {"Q",           KEY_F2},
    {"OQ",          KEY_F2},
    {"[[B",         KEY_F2},
    {"[13~",        KEY_F3},
    {"R",           KEY_F3},
    {"OR",          KEY_F3},
    {"[[C",         KEY_F3},
    {"[14~",        KEY_F4},
    {"S",           KEY_F4},
    {"OS",          KEY_F4},
    {"[[D",         KEY_F4},
    {"[15~",        KEY_F5},
    {"[[E",         KEY_F5},
    {"[17~",        KEY_F6},
    {"[18~",        KEY_F7},
    {"[19~",        KEY_F8},
    {"[20~",        KEY_F9},
    {"[21~",        KEY_F10},
    {"[23~",        KEY_F11},
    {"[24~",        KEY_F12},
    {"[25~",        KEY_F13},
    {"[11;2~",      KEY_F13},
    {"O2P",         KEY_F13},
    {"[26~",        KEY_F14},
    {"[12;2~",      KEY_F14},
    {"O2Q",         KEY_F14},
    {"[28~",        KEY_F15},
    {"[13;2~",      KEY_F15},
    {"O2R",         KEY_F15},
    {"[29~",        KEY_F16},
    {"[14;2~",      KEY_F16},
    {"O2S",         KEY_F16},
    {"[31~",        KEY_F17},
    {"[15;2~",      KEY_F17},
    {"[32~",        KEY_F18},
    {"[17;2~",      KEY_F18},
    {"[33~",        KEY_F19},
    {"[18;2~",      KEY_F19},
    {"[34~",        KEY_F20},
    {"[19;2~",      KEY_F20},
    {"[20;2~",      KEY_F21},
    {"[23$",        KEY_F21},
    {"[21;2~",      KEY_F22},
    {"[24$",        KEY_F22},
    {"[23;2~",      KEY_F23},
    {"[11^",        KEY_F23},
    {"[24;2~",      KEY_F24},
    {"[12^",        KEY_F24},

    // Other keys.
    {"[Z",          KEY_BACK_TAB},
    {"OZ",          KEY_BACK_TAB},
    {"[P",          KEY_PAUSE},
    {"[G",          KEY_NUMPAD_5},
};
#define numEscKeys  (sizeof(escKeys) / sizeof(escKeys[0]))

class Node
{
public:
    explicit Node(Node *parent = 0);
    ~Node();

    void add(const char *str, int code);
    void dumpRules(std::vector<uint8_t> *vec);

    int ch;
    int code;
    int offset;
    Node *firstChild;
    Node *lastChild;
    Node *nextChild;
};

Node::Node(Node *parent)
    : ch(0)
    , code(-1)
    , offset(0)
    , firstChild(0)
    , lastChild(0)
    , nextChild(0)
{
    if (parent) {
        if (parent->lastChild)
            parent->lastChild->nextChild = this;
        else
            parent->firstChild = this;
        parent->lastChild = this;
    }
}

Node::~Node()
{
    Node *child = firstChild;
    Node *next;
    while (child != 0) {
        next = child->nextChild;
        delete child;
        child = next;
    }
}

void Node::add(const char *str, int code)
{
    int ch = str[0] & 0xFF;
    Node *child = firstChild;
    while (child != 0) {
        if (child->ch == ch)
            break;
        child = child->nextChild;
    }
    if (!child) {
        child = new Node(this);
        child->ch = ch;
    }
    if (str[1] == '\0') {
        // Leaf node at the end of a string.
        child->code = code;
    } else {
        // Interior node with more children.
        child->add(str + 1, code);
    }
}

void Node::dumpRules(std::vector<uint8_t> *vec)
{
    Node *child;

    // First pass: Output the recognizers for this level.
    offset = vec->size();
    child = firstChild;
    while (child != 0) {
        if (child->firstChild) {
            // Interior nodes need 3 bytes for the character and offset.
            vec->push_back((uint8_t)(child->ch | 0x80));
            vec->push_back(0);
            vec->push_back(0);
        } else {
            // Leaf nodes need 2 bytes for the character and code.
            vec->push_back((uint8_t)(child->ch));
            vec->push_back((uint8_t)(child->code));
            if (child->code > 255)
                printf("Code 0x%X exceeds 255 - need to change table format\n", child->code);
        }
        child = child->nextChild;
    }
    vec->push_back(0);  // Terminate this level.

    // Second pass: Output the recognizers for the child levels.
    child = firstChild;
    while (child != 0) {
        if (child->firstChild)
            child->dumpRules(vec);
        child = child->nextChild;
    }

    // Third pass: Back-patch the links to the child recognizers.
    int posn = offset;
    child = firstChild;
    while (child != 0) {
        if (child->firstChild) {
            int value = child->offset;
            (*vec)[posn + 1] = (uint8_t)value;
            (*vec)[posn + 2] = (uint8_t)(value >> 8);
            posn += 3;
        } else {
            posn += 2;
        }
        child = child->nextChild;
    }
}

int main(int argc, char *argv[])
{
    Node *root = new Node();
    for (unsigned index = 0; index < numEscKeys; ++index)
        root->add(escKeys[index].sequence, escKeys[index].code);
    std::vector<uint8_t> vec;
    root->dumpRules(&vec);
    printf("static uint8_t const keymap[%d] PROGMEM = {\n", (int)vec.size());
    for (unsigned index = 0; index < vec.size(); ++index) {
        if ((index % 12) == 0)
            printf("    ");
        printf("0x%02X", (int)(vec[index]));
        if ((index % 12) == 11)
            printf(",\n");
        else
            printf(", ");
    }
    printf("};\n");
    delete root;
    return 0;
}
