/*
This example tests the Terminal class by displaying the VT100 keys that
are pressed.  A real terminal program like PuTTY will be needed.

This example is placed into the public domain.
*/

#include <Terminal.h>
#include <avr/pgmspace.h>

Terminal term;

#define K(name) {name, #name}
struct KeyInfo
{
    uint8_t code;
    char name[19];
};
struct KeyInfo const keys[] PROGMEM = {
    K(KEY_RETURN),
    K(KEY_ESC),
    K(KEY_BACKSPACE),
    K(KEY_TAB),
    K(KEY_BACK_TAB),
    K(KEY_CAPS_LOCK),
    K(KEY_F1),
    K(KEY_F2),
    K(KEY_F3),
    K(KEY_F4),
    K(KEY_F5),
    K(KEY_F6),
    K(KEY_F7),
    K(KEY_F8),
    K(KEY_F9),
    K(KEY_F10),
    K(KEY_F11),
    K(KEY_F12),
    K(KEY_F13),
    K(KEY_F14),
    K(KEY_F15),
    K(KEY_F16),
    K(KEY_F17),
    K(KEY_F18),
    K(KEY_F19),
    K(KEY_F20),
    K(KEY_F21),
    K(KEY_F22),
    K(KEY_F23),
    K(KEY_F24),
    K(KEY_INSERT),
    K(KEY_HOME),
    K(KEY_PAGE_UP),
    K(KEY_DELETE),
    K(KEY_END),
    K(KEY_PAGE_DOWN),
    K(KEY_RIGHT_ARROW),
    K(KEY_LEFT_ARROW),
    K(KEY_DOWN_ARROW),
    K(KEY_UP_ARROW),
    K(KEY_PRINT_SCREEN),
    K(KEY_SCROLL_LOCK),
    K(KEY_PAUSE),
    K(KEY_NUM_LOCK),
    K(KEY_NUMPAD_5),
    {0, ""}
};

void setup()
{
    Serial.begin(9600);
    term.begin(Serial);

    term.println("Press keys to see their codes ...");
    term.println();
}

void loop()
{
    int key = term.readKey();
    if (key >= 0x21 && key <= 0x7E) {
        // Printable ASCII character.
        term.print("ASCII: ");
        term.write((uint8_t)key);
        term.println();
    } else if (key == 0x20) {
        // Space.
        term.println("ASCII: SPACE");
    } else if (key == KEY_UNICODE) {
        // Extended Unicode character.
        term.print("Unicode: U+");
        term.print(term.unicodeKey(), 16);
        term.println();
    } else if (key >= 0 && key <= 0xFF) {
        // Special arrow or function key.
        const uint8_t *table = (const uint8_t *)keys;
        int code;
        while ((code = pgm_read_byte(table)) != 0) {
            if (code == key) {
                term.print("Special: ");
                term.writeProgMem((const char *)(table + 1));
                term.println();
                break;
            }
            table += sizeof(struct KeyInfo);
        }
        if (!code) {
            // Non-printable ASCII or unknown key.
            if (key < 0x20)
                term.print("ASCII: 0x");
            else
                term.print("Unknown: 0x");
            term.print(key, 16);
            term.println();
        }
    } else if (key >= 0) {
        // Unknown keycode.  Print in hex.
        term.print("Unknown: 0x");
        term.print(key, 16);
        term.println();
    }
}
