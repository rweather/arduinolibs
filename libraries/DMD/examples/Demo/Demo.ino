/*
This example demonstrates how to use the DMD and related classes to
draw things on a Freetronics Large Dot Matrix Display.

This example is placed into the public domain.
*/

#include <DMD.h>
#include <DejaVuSans9.h>
#include <DejaVuSansBold9.h>
#include <DejaVuSansItalic9.h>
#include <Mono5x7.h>

DMD display;

ISR(TIMER1_OVF_vect)
{
    display.refresh();
}

void setup() {
    display.enableTimer1();
}

void loop() {
    drawShapes();
    delay(1000);

    drawBricks();
    delay(1000);

    drawStickFigures();
    delay(1000);

    drawText();
    delay(1000);

    drawBoldText();
    delay(1000);

    drawItalicText();
    delay(1000);

    drawMonoText();
    delay(1000);

    drawMarquee();
    delay(500);
}

void drawShapes()
{
    display.clear();
    display.drawCircle(6, 8, 3);
    display.drawFilledCircle(16, 8, 3);
    display.drawLine(22, 5, 28, 11);
    display.drawLine(28, 5, 22, 11);
    display.drawRect(0, 0, display.width() - 1, display.height() - 1);
}

void drawBricks()
{
    static const uint8_t bricks[] PROGMEM = {
        16, 6,
        B11111111, B11111111,
        B10000000, B10000000,
        B10000000, B10000000,
        B11111111, B11111111,
        B00001000, B00001000,
        B00001000, B00001000
    };
    display.fill(0, 0, display.width(), display.height(), bricks);
}

void drawStickFigures()
{
    static const uint8_t stickFigure[] PROGMEM = {
        9, 13,
        B00111110, B00000000,
        B01000001, B00000000,
        B01000001, B00000000,
        B00111110, B00000000,
        B00001000, B00000000,
        B00001000, B00000000,
        B11111111, B10000000,
        B00001000, B00000000,
        B00001000, B00000000,
        B00010100, B00000000,
        B00100010, B00000000,
        B01000001, B00000000,
        B10000000, B10000000
    };
    display.clear();
    display.drawBitmap(2, 1, stickFigure);
    display.drawInvertedBitmap(12, 1, stickFigure);
    display.drawBitmap(22, 1, stickFigure);
}

void drawText()
{
    display.clear();
    display.setFont(DejaVuSans9);
    display.drawText(0, 0, "Hello");
    display.drawText(9, 8, "World");
}

void drawBoldText()
{
    display.clear();
    display.setFont(DejaVuSansBold9);
    display.drawText(0, 0, "Hello");
    display.drawText(4, 8, "World");
}

void drawItalicText()
{
    display.clear();
    display.setFont(DejaVuSansItalic9);
    display.drawText(0, 0, "Hello");
    display.drawText(2, 8, "World");
}

void drawMonoText()
{
    display.clear();
    display.setFont(Mono5x7);
    display.drawText(0, 0, "Hello");
    display.drawText(3, 8, "World");
}

static const char message[] = "Eat at Joes!";

void drawMarquee()
{
    int width = display.width();
    display.setFont(DejaVuSans9);
    int msgWidth = display.textWidth(message);
    int fullScroll = msgWidth + width + 1;
    for (int x = 0; x < fullScroll; ++x) {
        display.clear();
        display.drawText(width - x, 3, message);
        delay(50);
    }
}
