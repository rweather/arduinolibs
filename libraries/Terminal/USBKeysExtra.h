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

#ifndef USBKEYSEXTRA_h
#define USBKEYSEXTRA_h

// Extra key codes that are not included in the standard USBAPI.h header.
// Reference: http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
// Note: USBAPI.h shifts the Hut codes by adding 136 (0x88) so that
// they don't intersect with ASCII.  We do that here as well so
// that these codes can be used with Keyboard.press().  We use #ifndef
// here in case the core Arduino libraries add these in the future.

#ifndef KEY_PRINT_SCREEN
#define KEY_PRINT_SCREEN        (0x46 + 0x88)
#endif
#ifndef KEY_SCROLL_LOCK
#define KEY_SCROLL_LOCK         (0x47 + 0x88)
#endif
#ifndef KEY_PAUSE
#define KEY_PAUSE               (0x48 + 0x88)
#endif
#ifndef KEY_NUM_LOCK
#define KEY_NUM_LOCK            (0x53 + 0x88)
#endif
#ifndef KEY_NUMPAD_5
#define KEY_NUMPAD_5            (0x5D + 0x88)
#endif
#ifndef KEY_F13
#define KEY_F13                 (0x68 + 0x88)
#endif
#ifndef KEY_F14
#define KEY_F14                 (0x69 + 0x88)
#endif
#ifndef KEY_F15
#define KEY_F15                 (0x6A + 0x88)
#endif
#ifndef KEY_F16
#define KEY_F16                 (0x6B + 0x88)
#endif
#ifndef KEY_F17
#define KEY_F17                 (0x6C + 0x88)
#endif
#ifndef KEY_F18
#define KEY_F18                 (0x6D + 0x88)
#endif
#ifndef KEY_F19
#define KEY_F19                 (0x6E + 0x88)
#endif
#ifndef KEY_F20
#define KEY_F20                 (0x6F + 0x88)
#endif
#ifndef KEY_F21
#define KEY_F21                 (0x70 + 0x88)
#endif
#ifndef KEY_F22
#define KEY_F22                 (0x71 + 0x88)
#endif
#ifndef KEY_F23
#define KEY_F23                 (0x72 + 0x88)
#endif
#ifndef KEY_F24
#define KEY_F24                 (0x73 + 0x88)
#endif

// USB does not have a code for "Back Tab" as it is usually Shift-TAB.
// For convenience, we map it to the ASCII vertical tab character (0x0B).
#define KEY_BACK_TAB            0x0B

#ifndef KEY_RETURN

// If the Arduino variant does not support USB, then USBAPI.h will not
// define the key codes that we need.  So we define them here instead.

#define KEY_RETURN              (0x28 + 0x88)
#define KEY_ESC                 (0x29 + 0x88)
#define KEY_BACKSPACE           (0x2A + 0x88)
#define KEY_TAB                 (0x2B + 0x88)
#define KEY_CAPS_LOCK           (0x39 + 0x88)
#define KEY_F1                  (0x3A + 0x88)
#define KEY_F2                  (0x3B + 0x88)
#define KEY_F3                  (0x3C + 0x88)
#define KEY_F4                  (0x3D + 0x88)
#define KEY_F5                  (0x3E + 0x88)
#define KEY_F6                  (0x3F + 0x88)
#define KEY_F7                  (0x40 + 0x88)
#define KEY_F8                  (0x41 + 0x88)
#define KEY_F9                  (0x42 + 0x88)
#define KEY_F10                 (0x43 + 0x88)
#define KEY_F11                 (0x44 + 0x88)
#define KEY_F12                 (0x45 + 0x88)
#define KEY_INSERT              (0x49 + 0x88)
#define KEY_HOME                (0x4A + 0x88)
#define KEY_PAGE_UP             (0x4B + 0x88)
#define KEY_DELETE              (0x4C + 0x88)
#define KEY_END                 (0x4D + 0x88)
#define KEY_PAGE_DOWN           (0x4E + 0x88)
#define KEY_RIGHT_ARROW         (0x4F + 0x88)
#define KEY_LEFT_ARROW          (0x50 + 0x88)
#define KEY_DOWN_ARROW          (0x51 + 0x88)
#define KEY_UP_ARROW            (0x52 + 0x88)

#endif

#endif
