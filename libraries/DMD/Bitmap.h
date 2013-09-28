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

#ifndef Bitmap_h
#define Bitmap_h

#include <inttypes.h>
#include <avr/pgmspace.h>

class DMD;
class String;

class Bitmap
{
public:
    Bitmap(int width, int height);
    ~Bitmap();

    bool isValid() const { return fb != 0; }

    typedef uint8_t Color;
    typedef PGM_VOID_P ProgMem;
    typedef PGM_VOID_P Font;

    static const Color Black  = 0;
    static const Color White  = 1;
    static const Color NoFill = 2;

    int width() const { return _width; }
    int height() const { return _height; }
    int stride() const { return _stride; }
    int bitsPerPixel() const { return 1; }

    uint8_t *data() { return fb; }
    const uint8_t *data() const { return fb; }

    void clear(Color color = Black);

    Color pixel(int x, int y) const;
    void setPixel(int x, int y, Color color);

    void drawLine(int x1, int y1, int x2, int y2, Color color = White);
    void drawRect(int x1, int y1, int x2, int y2, Color borderColor = White, Color fillColor = NoFill);
    void drawFilledRect(int x1, int y1, int x2, int y2, Color color = White);
    void drawCircle(int centerX, int centerY, int radius, Color borderColor = White, Color fillColor = NoFill);
    void drawFilledCircle(int centerX, int centerY, int radius, Color color = White);

    void drawBitmap(int x, int y, const Bitmap &bitmap, Color color = White);
    void drawBitmap(int x, int y, Bitmap::ProgMem bitmap, Color color = White);
    void drawInvertedBitmap(int x, int y, const Bitmap &bitmap);
    void drawInvertedBitmap(int x, int y, Bitmap::ProgMem bitmap);

    Font font() const { return _font; }
    void setFont(Font font) { _font = font; }

    Color textColor() const { return _textColor; }
    void setTextColor(Color color) { _textColor = color; }

    void drawText(int x, int y, const char *str, int len = -1);
    void drawText(int x, int y, const String &str, int start = 0, int len = -1);

    int drawChar(int x, int y, char ch);

    int charWidth(char ch) const;
    int textWidth(const char *str, int len = -1) const;
    int textWidth(const String &str, int start = 0, int len = -1) const;
    int textHeight() const;

    void copy(int x, int y, int width, int height, Bitmap *dest, int destX, int destY);
    void fill(int x, int y, int width, int height, Color color);
    void fill(int x, int y, int width, int height, Bitmap::ProgMem pattern, Color color = White);

    void scroll(int dx, int dy, Color fillColor = Black);
    void scroll(int x, int y, int width, int height, int dx, int dy, Color fillColor = Black);

    void invert(int x, int y, int width, int height);

private:
    // Disable copy constructor and operator=().
    Bitmap(const Bitmap &) {}
    Bitmap &operator=(const Bitmap &) { return *this; }

    int _width;
    int _height;
    int _stride;
    uint8_t *fb;
    Font _font;
    Color _textColor;

    friend class DMD;

    void blit(int x1, int y1, int x2, int y2, int x3, int y3);
    void drawCirclePoints(int centerX, int centerY, int radius, int x, int y, Color borderColor, Color fillColor);
};

inline void Bitmap::drawFilledRect(int x1, int y1, int x2, int y2, Color color)
{
    drawRect(x1, y1, x2, y2, color, color);
}

inline void Bitmap::drawFilledCircle(int centerX, int centerY, int radius, Color color)
{
    drawCircle(centerX, centerY, radius, color, color);
}

inline void Bitmap::drawInvertedBitmap(int x, int y, const Bitmap &bitmap)
{
    drawBitmap(x, y, bitmap, Black);
}

inline void Bitmap::drawInvertedBitmap(int x, int y, Bitmap::ProgMem bitmap)
{
    drawBitmap(x, y, bitmap, Black);
}

inline void Bitmap::scroll(int dx, int dy, Color fillColor)
{
    scroll(0, 0, _width, _height, dx, dy, fillColor);
}

#endif
