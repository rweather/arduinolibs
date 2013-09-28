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

#include "Bitmap.h"
#include <WString.h>
#include <string.h>
#include <stdlib.h>

/**
 * \class Bitmap Bitmap.h <Bitmap.h>
 * \brief Represents a monochrome bitmap within main memory.
 *
 * Bitmaps are a rectangular arrangement of width() x height() pixels,
 * with each pixel set to either \ref Black or \ref White.  The co-ordinate
 * system has origin (0, 0) at the top-left of the bitmap.
 *
 * Functions within this class can be used to draw various shapes into
 * the bitmap's data() buffer; e.g. drawLine(), drawRect(), drawBitmap(),
 * drawText(), clear(), fill(), etc.
 *
 * \sa DMD
 */

/**
 * \typedef Bitmap::Color
 * \brief Type that represents the color of a pixel in a bitmap.
 *
 * \sa Black, White
 */

/**
 * \typedef Bitmap::ProgMem
 * \brief Type that represents a bitmap within program memory.
 */

/**
 * \typedef Bitmap::Font
 * \brief Type that represents a font within program memory.
 */

/**
 * \var Bitmap::Black
 * \brief Color value corresponding to "black".
 */

/**
 * \var Bitmap::White
 * \brief Color value corresponding to "white".  If the bitmap is
 * displayed on a LED array, then it may have a different physical color.
 *
 * Note: while the value of this constant is 1, the bitmap itself stores
 * white pixels as 0 and black as 1 because the DMD display uses 1 to
 * indicate a pixel being off.
 */

/**
 * \var Bitmap::NoFill
 * \brief Special color value that is used with drawRect() and drawCircle()
 * to indicate that the interior of the shape should not be filled.
 * For all other uses, \ref NoFill is equivalent to \ref White.
 */

/**
 * \brief Constructs a new in-memory bitmap that is \a width x \a height
 * pixels in size.
 *
 * \sa width(), height(), isValid()
 */
Bitmap::Bitmap(int width, int height)
    : _width(width)
    , _height(height)
    , _stride((width + 7) / 8)
    , fb(0)
    , _font(0)
    , _textColor(White)
{
    // Allocate memory for the framebuffer and clear it (1 = pixel off).
    unsigned int size = _stride * _height;
    fb = (uint8_t *)malloc(size);
    if (fb)
        memset(fb, 0xFF, size);
}

/**
 * \brief Destroys this bitmap.
 */
Bitmap::~Bitmap()
{
    if (fb)
        free(fb);
}

/**
 * \fn bool Bitmap::isValid() const
 * \brief Returns true if the memory for this bitmap is valid; false otherwise.
 *
 * This function can be called just after the constructor to determine if
 * the memory for the bitmap was allocated successfully.
 */

/**
 * \fn int Bitmap::width() const
 * \brief Returns the width of the bitmap in pixels.
 *
 * \sa height(), stride(), bitsPerPixel()
 */

/**
 * \fn int Bitmap::height() const
 * \brief Returns the height of the bitmap in pixels.
 *
 * \sa width(), bitsPerPixel()
 */

/**
 * \fn int Bitmap::stride() const
 * \brief Returns the number of bytes in each line of the bitmap's data() buffer.
 *
 * \sa width(), bitsPerPixel(), data()
 */

/**
 * \fn int Bitmap::bitsPerPixel() const
 * \brief Returns the number of bits per pixel for the bitmap; always 1.
 *
 * \sa width(), height()
 */

/**
 * \fn uint8_t *Bitmap::data()
 * \brief Returns a pointer to the start of the bitmap's data buffer.
 *
 * The data is organized as height() lines of stride() bytes, laid out
 * horizontally across the extent of width() pixels.  The most significant
 * bit in each byte has the lowest x value.
 *
 * Note: bits within the data are 1 for \ref Black and 0 for \ref White,
 * which is the reverse of the constant values.  This differs from pixel()
 * which returns the correct constant.
 *
 * \sa pixel(), stride()
 */

/**
 * \fn const uint8_t *Bitmap::data() const
 * \brief Returns a constant pointer to the start of the bitmap's data buffer.
 * \overload
 */

/**
 * \brief Clears the entire bitmap to the specified \a color.
 *
 * \sa fill()
 */
void Bitmap::clear(Color color)
{
    unsigned int size = _stride * _height;
    if (color == Black)
        memset(fb, 0xFF, size);
    else
        memset(fb, 0x00, size);
}

/**
 * \brief Returns the color of the pixel at (\a x, \a y); either \ref Black
 * or \ref White.
 *
 * Returns \a Black if \a x or \a y is out of range.
 *
 * \sa setPixel(), data()
 */
Bitmap::Color Bitmap::pixel(int x, int y) const
{
    if (((unsigned int)x) >= ((unsigned int)_width) ||
            ((unsigned int)y) >= ((unsigned int)_height))
        return Black;
    uint8_t *ptr = fb + y * _stride + (x >> 3);
    if (*ptr & ((uint8_t)0x80) >> (x & 0x07))
        return Black;
    else
        return White;
}

/**
 * \brief Sets the pixel at (\a x, \a y) to \a color.
 *
 * \sa pixel()
 */
void Bitmap::setPixel(int x, int y, Color color)
{
    if (((unsigned int)x) >= ((unsigned int)_width) ||
            ((unsigned int)y) >= ((unsigned int)_height))
        return;     // Pixel is off-screen.
    uint8_t *ptr = fb + y * _stride + (x >> 3);
    if (color)
        *ptr &= ~(((uint8_t)0x80) >> (x & 0x07));
    else
        *ptr |= (((uint8_t)0x80) >> (x & 0x07));
}

/**
 * \brief Draws a line from (\a x1, \a y1) to (\a x2, \a y2) in \a color.
 *
 * \sa drawRect(), drawCircle()
 */
void Bitmap::drawLine(int x1, int y1, int x2, int y2, Color color)
{
    // Midpoint line scan-conversion algorithm from "Computer Graphics:
    // Principles and Practice", Second Edition, Foley, van Dam, et al.
    int dx = x2 - x1;
    int dy = y2 - y1;
    int xstep, ystep;
    int d, incrE, incrNE;
    if (dx < 0) {
        xstep = -1;
        dx = -dx;
    } else {
        xstep = 1;
    }
    if (dy < 0) {
        ystep = -1;
        dy = -dy;
    } else {
        ystep = 1;
    }
    if (dx >= dy) {
        d = 2 * dy - dx;
        incrE = 2 * dy;
        incrNE = 2 * (dy - dx);
        setPixel(x1, y1, color);
        while (x1 != x2) {
            if (d <= 0) {
                d += incrE;
            } else {
                d += incrNE;
                y1 += ystep;
            }
            x1 += xstep;
            setPixel(x1, y1, color);
        }
    } else {
        d = 2 * dx - dy;
        incrE = 2 * dx;
        incrNE = 2 * (dx - dy);
        setPixel(x1, y1, color);
        while (y1 != y2) {
            if (d <= 0) {
                d += incrE;
            } else {
                d += incrNE;
                x1 += xstep;
            }
            y1 += ystep;
            setPixel(x1, y1, color);
        }
    }
}

/**
 * \brief Draws a rectangle from (\a x1, \a y1) to (\a x2, \a y2), with the
 * outline in \a borderColor and the interior filled with \a fillColor.
 *
 * If \a fillColor is \ref NoFill, then the interior is not filled.
 *
 * \sa drawFilledRect(), drawLine(), drawCircle(), fill()
 */
void Bitmap::drawRect(int x1, int y1, int x2, int y2, Color borderColor, Color fillColor)
{
    int temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    if (fillColor == borderColor) {
        fill(x1, y1, x2 - x1 + 1, y2 - y1 + 1, fillColor);
    } else {
        drawLine(x1, y1, x2, y1, borderColor);
        if (y1 < y2)
            drawLine(x2, y1 + 1, x2, y2, borderColor);
        if (x1 < x2)
            drawLine(x2 - 1, y2, x1, y2, borderColor);
        if (y1 < (y2 - 1))
            drawLine(x1, y2 - 1, x1, y1 + 1, borderColor);
        if (fillColor != NoFill)
            fill(x1 + 1, y1 + 1, x2 - x1 - 1, y2 - y1 - 1, fillColor);
    }
}

/**
 * \fn void Bitmap::drawFilledRect(int x1, int y1, int x2, int y2, Color color)
 * \brief Draws a filled rectangle from (\a x1, \a y1) to (\a x2, \a y2)
 * in \a color.
 *
 * This is a convenience function that is equivalent to
 * drawRect(\a x1, \a y1, \a x2, \a y2, \a color, \a color).
 *
 * \sa drawRect(), drawFilledCircle()
 */

/**
 * \brief Draws a circle with a specific center (\a centerX, \a centerY)
 * and \a radius, with the outline in \a borderColor and the interior
 * filled with \a fillColor.
 *
 * If \a fillColor is \ref NoFill, then the interior is not filled.
 *
 * \sa drawFilledCircle(), drawLine(), drawRect()
 */
void Bitmap::drawCircle(int centerX, int centerY, int radius, Color borderColor, Color fillColor)
{
    // Midpoint circle scan-conversion algorithm using second-order
    // differences from "Computer Graphics: Principles and Practice",
    // Second Edition, Foley, van Dam, et al.
    if (radius < 0)
        radius = -radius;
    int x = 0;
    int y = radius;
    int d = 1 - radius;
    int deltaE = 3;
    int deltaSE = 5 - 2 * radius;
    drawCirclePoints(centerX, centerY, radius, x, y, borderColor, fillColor);
    while (y > x) {
        if (d < 0) {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        } else {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            --y;
        }
        ++x;
        drawCirclePoints(centerX, centerY, radius, x, y, borderColor, fillColor);
    }
}

/**
 * \fn void Bitmap::drawFilledCircle(int centerX, int centerY, int radius, Color color)
 * \brief Draws a filled circle with a specific center (\a centerX, \a centerY)
 * and \a radius in \a color.
 *
 * This is a convenience function that is equivalent to
 * drawCircle(\a centerX, \a centerY, \a radius, \a color, \a color).
 *
 * \sa drawCircle(), drawFilledRect()
 */

/**
 * \brief Draws \a bitmap at (\a x, \a y) in \a color.
 *
 * Bits that are set to \ref White in the \a bitmap are drawn with \a color.
 * Bits that are set to \ref Black in the \a bitmap are drawn with the
 * inverse of \a color.  The pixel at (\a x, \a y) will be the top-left
 * corner of the drawn image.
 *
 * Note: \a bitmap must not be the same as this object or the behaviour will
 * be undefined.  To copy a region of a bitmap to elsewhere within the
 * same bitmap, use copy() instead.
 *
 * \sa drawInvertedBitmap(), copy()
 */
void Bitmap::drawBitmap(int x, int y, const Bitmap &bitmap, Color color)
{
    int w = bitmap.width();
    int s = bitmap.stride();
    int h = bitmap.height();
    Color invColor = !color;
    for (uint8_t by = 0; by < h; ++by) {
        const uint8_t *line = bitmap.data() + by * s;
        uint8_t mask = 0x80;
        uint8_t value = *line++;
        for (uint8_t bx = 0; bx < w; ++bx) {
            if (value & mask)
                setPixel(x + bx, y + by, invColor);
            else
                setPixel(x + bx, y + by, color);
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                value = *line++;
            }
        }
    }
}

/**
 * \brief Draws \a bitmap at (\a x, \a y) in \a color.
 *
 * The \a bitmap must point to program memory.  The first two bytes are the
 * width and height of the bitmap in pixels.  The rest of the data contains
 * the pixels for the bitmap, with lines byte-aligned.
 *
 * Bits that are 1 in the \a bitmap are drawn with \a color.  Bits that are
 * 0 in the \a bitmap are drawn with the inverse of \a color.  The pixel at
 * (\a x, \a y) will be the top-left corner of the drawn image.
 *
 * \sa drawInvertedBitmap(), fill()
 */
void Bitmap::drawBitmap(int x, int y, Bitmap::ProgMem bitmap, Color color)
{
    uint8_t w = pgm_read_byte(bitmap);
    uint8_t s = (w + 7) >> 3;
    uint8_t h = pgm_read_byte(bitmap + 1);
    Color invColor = !color;
    for (uint8_t by = 0; by < h; ++by) {
        const uint8_t *line = ((const uint8_t *)bitmap) + 2 + by * s;
        uint8_t mask = 0x80;
        uint8_t value = pgm_read_byte(line);
        for (uint8_t bx = 0; bx < w; ++bx) {
            if (value & mask)
                setPixel(x + bx, y + by, color);
            else
                setPixel(x + bx, y + by, invColor);
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                ++line;
                value = pgm_read_byte(line);
            }
        }
    }
}

/**
 * \fn void Bitmap::drawInvertedBitmap(int x, int y, const Bitmap &bitmap)
 * \brief Draws \a bitmap at (\a x, \a y) in inverted colors.
 *
 * This is a convenience function that is equivalent to
 * drawBitmap(\a x, \a y, \a bitmap, \ref Black).
 *
 * \sa drawBitmap()
 */

/**
 * \fn void Bitmap::drawInvertedBitmap(int x, int y, Bitmap::ProgMem bitmap)
 * \brief Draws \a bitmap at (\a x, \a y) in inverted colors.
 *
 * This is a convenience function that is equivalent to
 * drawBitmap(\a x, \a y, \a bitmap, \ref Black).
 *
 * \sa drawBitmap()
 */

/**
 * \fn Font Bitmap::font() const
 * \brief Returns the currently selected font, or null if none selected.
 *
 * \sa setFont(), drawText(), drawChar(), charWidth()
 */

/**
 * \fn void Bitmap::setFont(Font font)
 * \brief Sets the \a font for use with drawText() and drawChar().
 *
 * \code
 * #include <DejaVuSans9.h>
 *
 * display.setFont(DejaVuSans9);
 * display.drawText(0, 0, "Hello");
 * \endcode
 *
 * New fonts can be generated with <a href="https://code.google.com/p/glcd-arduino/downloads/detail?name=GLCDFontCreator2.zip&can=2&q=">GLCDFontCreator2</a>.
 *
 * \sa font(), drawText(), drawChar()
 */

/**
 * \fn Color Bitmap::textColor() const
 * \brief Returns the color that will be used for drawing text with
 * drawText() and drawChar().  The default is \ref White.
 *
 * \sa setTextColor(), drawText(), drawChar()
 */

/**
 * \fn void Bitmap::setTextColor(Color textColor)
 * \brief Sets the \a color that will be used for drawing text with
 * drawText() and drawChar().
 *
 * \sa textColor(), drawText(), drawChar()
 */

#define fontIsFixed(font)   (pgm_read_byte((font)) == 0 && \
                             pgm_read_byte((font) + 1) == 0)
#define fontWidth(font)     (pgm_read_byte((font) + 2))
#define fontHeight(font)    (pgm_read_byte((font) + 3))
#define fontFirstChar(font) (pgm_read_byte((font) + 4))
#define fontCharCount(font) (pgm_read_byte((font) + 5))

/**
 * \brief Draws the \a len characters of \a str at (\a x, \a y).
 *
 * If \a len is less than zero, then the actual length of \a str will be used.
 *
 * The position (\a x, \a y) will be the upper-left pixel of the first
 * character that is drawn.
 *
 * \sa drawChar(), textColor(), font()
 */
void Bitmap::drawText(int x, int y, const char *str, int len)
{
    if (!_font)
        return;
    uint8_t height = fontHeight(_font);
    if (len < 0)
        len = strlen(str);
    while (len-- > 0) {
        x += drawChar(x, y, *str++);
        if (len > 0) {
            fill(x, y, 1, height, !_textColor);
            ++x;
        }
        if (x >= _width)
            break;
    }
}

/**
 * \brief Draws \a len characters starting at \a start from \a str to the
 * screen at (\a x, \a y).
 *
 * If \a len is less than zero, then the actual length of \a str will be used.
 *
 * The position (\a x, \a y) will be the upper-left pixel of the first
 * character that is drawn.
 *
 * \sa drawChar(), textColor(), font()
 */
void Bitmap::drawText(int x, int y, const String &str, int start, int len)
{
    if (!_font)
        return;
    uint8_t height = fontHeight(_font);
    if (len < 0)
        len = str.length() - start;
    while (len-- > 0) {
        x += drawChar(x, y, str[start++]);
        if (len > 0) {
            fill(x, y, 1, height, !_textColor);
            ++x;
        }
        if (x >= _width)
            break;
    }
}

/**
 * \brief Draws a single character \a ch at (\a x, \a y).
 *
 * Returns the width of the character in pixels so that higher-order functions
 * like drawText() can advance \a x to the location of the next character
 * to be drawn.  The width does not include inter-character spacing.
 *
 * The position (\a x, \a y) will be the upper-left pixel of the drawn
 * character.
 *
 * \sa drawText(), textColor(), font(), charWidth()
 */
int Bitmap::drawChar(int x, int y, char ch)
{
    uint8_t height = fontHeight(_font);
    if (ch == ' ') {
        // Font may not have space, or it is zero-width.  Calculate
        // the real size and fill the space.
        int spaceWidth = charWidth('n');
        fill(x, y, spaceWidth, height, !_textColor);
        return spaceWidth;
    }
    uint8_t first = fontFirstChar(_font);
    uint8_t count = fontCharCount(_font);
    uint8_t index = (uint8_t)ch;
    if (index < first || index >= (first + count))
        return 0;
    index -= first;
    uint8_t heightBytes = (height + 7) >> 3;;
    uint8_t width;
    const uint8_t *image;
    if (fontIsFixed(_font)) {
        // Fixed-width font.
        width = fontWidth(_font);
        image = ((const uint8_t *)_font) + 6 + index * heightBytes * width;
    } else {
        // Variable-width font.
        width = pgm_read_byte(_font + 6 + index);
        image = ((const uint8_t *)_font) + 6 + count;
        for (uint8_t temp = 0; temp < index; ++temp) {
            // Scan through all previous characters to find the starting
            // location for this one.
            image += pgm_read_byte(_font + 6 + temp) * heightBytes;
        }
    }
    if ((x + width) <= 0 || (y + height) <= 0)
        return width;   // Character is off the top or left of the screen.
    Color invColor = !_textColor;
    for (uint8_t cx = 0; cx < width; ++cx) {
        for (uint8_t cy = 0; cy < heightBytes; ++cy) {
            uint8_t value = pgm_read_byte(image + cy * width + cx);
            int posn;
            if (heightBytes > 1 && cy == (heightBytes - 1))
                posn = height - 8;
            else
                posn = cy * 8;
            for (uint8_t bit = 0; bit < 8; ++bit) {
                if ((posn + bit) >= (cy * 8) && (posn + bit) <= height) {
                    if (value & 0x01)
                        setPixel(x + cx, y + posn + bit, _textColor);
                    else
                        setPixel(x + cx, y + posn + bit, invColor);
                }
                value >>= 1;
            }
        }
    }
    return width;
}

/**
 * \brief Returns the width in pixels of \a ch in the current font().
 *
 * Returns zero if font() is not set, or \a ch is not present in font().
 *
 * \sa drawChar(), font(), textWidth(), textHeight()
 */
int Bitmap::charWidth(char ch) const
{
    uint8_t index = (uint8_t)ch;
    if (!_font)
        return 0;
    uint8_t first = fontFirstChar(_font);
    uint8_t count = fontCharCount(_font);
    if (index == ' ')
        index = 'n';    // In case the font does not contain space.
    if (index < first || index >= (first + count))
        return 0;
    if (fontIsFixed(_font))
        return fontWidth(_font);
    else
        return pgm_read_byte(_font + 6 + (index - first));
}

/**
 * \brief Returns the width in pixels of the \a len characters of \a str
 * in the current font(), including inter-character spacing.
 *
 * If \a len is less than zero, then the actual length of \a str will be used.
 *
 * \sa drawText(), charWidth(), textHeight()
 */
int Bitmap::textWidth(const char *str, int len) const
{
    int width = 0;
    if (len < 0)
        len = strlen(str);
    while (len-- > 0) {
        width += charWidth(*str++);
        if (len > 0)
            ++width;
    }
    return width;
}

/**
 * \brief Returns the width in pixels of the \a len characters of \a str
 * in the current font(), starting at \a start, including inter-character
 * spacing.
 *
 * If \a len is less than zero, then the actual length of \a str will be used.
 *
 * \sa drawText(), charWidth(), textHeight()
 */
int Bitmap::textWidth(const String &str, int start, int len) const
{
    int width = 0;
    if (len < 0)
        len = str.length() - start;
    while (len-- > 0) {
        width += charWidth(str[start++]);
        if (len > 0)
            ++width;
    }
    return width;
}

/**
 * \brief Returns the height in pixels of the current text drawing font();
 * or zero if font() is not set.
 *
 * \sa font(), charWidth(), textWidth()
 */
int Bitmap::textHeight() const
{
    if (_font)
        return fontHeight(_font);
    else
        return 0;
}

/**
 * \brief Copies the \a width x \a height pixels starting at top-left
 * corner (\a x, \a y) to (\a destX, \a destY) in the bitmap \a dest.
 *
 * The \a dest bitmap can be the same as this object, in which case the copy
 * will be performed in a manner that correctly handles overlapping regions.
 *
 * If some part of the source region is outside the bounds of this object,
 * then the value \ref Black will be copied to \a dest for those pixels.
 * This can be used to produce a behaviour similar to scroll() when
 * \a bitmap is the same as this object.
 *
 * \sa drawBitmap(), fill(), scroll()
 */
void Bitmap::copy(int x, int y, int width, int height, Bitmap *dest, int destX, int destY)
{
    if (dest == this) {
        // Copying to within the same bitmap, so copy in a direction
        // that will prevent problems with overlap.
        blit(x, y, x + width - 1, y + height - 1, destX, destY);
    } else {
        // Copying to a different bitmap.
        while (height > 0) {
            for (int tempx = 0; tempx < width; ++tempx)
                dest->setPixel(destX + tempx, destY, pixel(x + tempx, y));
            ++y;
            ++destY;
            --height;
        }
    }
}

/**
 * \brief Fills the \a width x \a height pixels starting at top-left
 * corner (\a x, \a y) with \a color.
 *
 * \sa copy(), clear(), invert(), drawRect()
 */
void Bitmap::fill(int x, int y, int width, int height, Color color)
{
    while (height > 0) {
        for (int temp = 0; temp < width; ++temp)
            setPixel(x + temp, y, color);
        ++y;
        --height;
    }
}

/**
 * \brief Fills the \a width x \a height pixels starting at top-left
 * corner (\a x, \a y) with the contents of \a pattern.
 *
 * The \a pattern must point to program memory.  The first two bytes are the
 * width and height of the pattern in pixels.  The rest of the data contains
 * the pixels for the pattern, with lines byte-aligned.
 *
 * Bits that are 1 in the \a pattern are drawn with \a color.  Bits that are
 * 0 in the \a pattern are drawn with the inverse of \a color.
 *
 * \sa drawBitmap(), clear(), invert()
 */
void Bitmap::fill(int x, int y, int width, int height, Bitmap::ProgMem pattern, Color color)
{
    uint8_t w = pgm_read_byte(pattern);
    uint8_t s = (w + 7) >> 3;
    uint8_t h = pgm_read_byte(pattern + 1);
    if (!w || !h)
        return;
    Color invColor = !color;
    for (int tempy = 0; tempy < height; ++tempy) {
        const uint8_t *startLine = ((const uint8_t *)pattern) + 2 + (tempy % h) * s;
        const uint8_t *line = startLine;
        uint8_t mask = 0x80;
        uint8_t value = pgm_read_byte(line++);
        int bit = 0;
        for (int tempx = 0; tempx < width; ++tempx) {
            if (value & mask)
                setPixel(x + tempx, y + tempy, color);
            else
                setPixel(x + tempx, y + tempy, invColor);
            if (++bit >= w) {
                mask = 0x80;
                line = startLine;
                value = pgm_read_byte(line++);
                bit = 0;
            } else {
                mask >>= 1;
                if (!mask) {
                    mask = 0x80;
                    value = pgm_read_byte(line++);
                }
            }
        }
    }
}

/**
 * \fn void Bitmap::scroll(int dx, int dy, Color fillColor)
 * \brief Scrolls the entire contents of the bitmap by \a dx and \a dy.
 *
 * If \a dx is 2 and \a dy is -1, then the region will be scrolled two
 * pixels to the right and one pixel up.  Pixels that are uncovered
 * by the scroll are filled with \a fillColor.
 *
 * \sa copy(), fill()
 */

/**
 * \brief Scrolls the \a width x \a height pixels starting at top-left
 * corner (\a x, \a y) by \a dx and \a dy.
 *
 * If \a dx is 2 and \a dy is -1, then the region will be scrolled two
 * pixels to the right and one pixel up.  Pixels that are uncovered
 * by the scroll are filled with \a fillColor.
 *
 * \sa copy(), fill()
 */
void Bitmap::scroll(int x, int y, int width, int height, int dx, int dy, Color fillColor)
{
    // Bail out if no scrolling at all.
    if (!dx && !dy)
        return;

    // Clamp the scroll region to the extents of the bitmap.
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if ((x + width) > _width)
        width = _width - x;
    if ((y + height) > _height)
        height = _height - y;
    if (width <= 0 || height <= 0)
        return;

    // Scroll the region in the specified direction.
    if (dy < 0) {
        if (dx < 0)
            blit(x - dx, y - dy, x + width - 1 + dx, y + height - 1 + dy, x, y);
        else
            blit(x, y - dy, x + width - 1 - dx, y + height - 1 + dy, x + dx, y);
    } else {
        if (dx < 0)
            blit(x - dx, y, x + width - 1 + dx, y + height - 1 - dy, x, y + dy);
        else
            blit(x, y, x + width - 1 - dx, y + height - 1 - dy, x + dx, y + dy);
    }

    // Fill the pixels that were uncovered by the scroll.
    if (dy < 0) {
        fill(x, y + height + dy, width, -dy, fillColor);
        if (dx < 0)
            fill(x + width + dx, y, -dx, height + dy, fillColor);
        else if (dx > 0)
            fill(x, y, dx, height + dy, fillColor);
    } else if (dy > 0) {
        fill(x, y, width, -dy, fillColor);
        if (dx < 0)
            fill(x + width + dx, y + dy, -dx, height - dy, fillColor);
        else if (dx > 0)
            fill(x, y + dy, dx, height - dy, fillColor);
    } else if (dx < 0) {
        fill(x + width + dx, y, -dx, height, fillColor);
    } else if (dx > 0) {
        fill(x, y, dx, height, fillColor);
    }
}

/**
 * \brief Inverts the \a width x \a height pixels starting at top-left
 * corner (\a x, \a y).
 *
 * \sa fill()
 */
void Bitmap::invert(int x, int y, int width, int height)
{
    while (height > 0) {
        for (int tempx = x + width - 1; tempx >= x; --tempx)
            setPixel(tempx, y, !pixel(tempx, y));
        --height;
        ++y;
    }
}

void Bitmap::blit(int x1, int y1, int x2, int y2, int x3, int y3)
{
    if (y3 < y1 || (y1 == y3 && x3 <= x1)) {
        for (int tempy = y1; tempy <= y2; ++tempy) {
            int y = y1 - tempy + y3;
            int x = x3 - x1;
            for (int tempx = x1; tempx <= x2; ++tempx)
                setPixel(x + tempx, y, pixel(tempx, tempy));
        }
    } else {
        for (int tempy = y2; tempy >= y1; --tempy) {
            int y = y1 - tempy + y3;
            int x = x3 - x1;
            for (int tempx = x2; tempx >= x1; --tempx)
                setPixel(x + tempx, y, pixel(tempx, tempy));
        }
    }
}

void Bitmap::drawCirclePoints(int centerX, int centerY, int radius, int x, int y, Color borderColor, Color fillColor)
{
    if (x != y) {
        setPixel(centerX + x, centerY + y, borderColor);
        setPixel(centerX + y, centerY + x, borderColor);
        setPixel(centerX + y, centerY - x, borderColor);
        setPixel(centerX + x, centerY - y, borderColor);
        setPixel(centerX - x, centerY - y, borderColor);
        setPixel(centerX - y, centerY - x, borderColor);
        setPixel(centerX - y, centerY + x, borderColor);
        setPixel(centerX - x, centerY + y, borderColor);
        if (fillColor != NoFill) {
            if (radius > 1) {
                drawLine(centerX - x + 1, centerY + y, centerX + x - 1, centerY + y, fillColor);
                drawLine(centerX - y + 1, centerY + x, centerX + y - 1, centerY + x, fillColor);
                drawLine(centerX - x + 1, centerY - y, centerX + x - 1, centerY - y, fillColor);
                drawLine(centerX - y + 1, centerY - x, centerX + y - 1, centerY - x, fillColor);
            } else if (radius == 1) {
                setPixel(centerX, centerY, fillColor);
            }
        }
    } else {
        setPixel(centerX + x, centerY + y, borderColor);
        setPixel(centerX + y, centerY - x, borderColor);
        setPixel(centerX - x, centerY - y, borderColor);
        setPixel(centerX - y, centerY + x, borderColor);
        if (fillColor != NoFill) {
            if (radius > 1) {
                drawLine(centerX - x + 1, centerY + y, centerX + x - 1, centerY + y, fillColor);
                drawLine(centerX - x + 1, centerY - y, centerX + x - 1, centerY - y, fillColor);
            } else if (radius == 1) {
                setPixel(centerX, centerY, fillColor);
            }
        }
    }
}
