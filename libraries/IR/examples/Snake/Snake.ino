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

#include <IRreceiver.h>
#include <DMD.h>
#include <Mono5x7.h>

#define SNAKE_ADVANCE_TIME  150
#define SNAKE_BLINK_TIME    500
#define SNAKE_INC_STEPS     5
#define SNAKE_MAX_LENGTH    50
#define SNAKE_START_LENGTH  10

struct Point
{
    int x, y;
};

DMD display;
IRreceiver ir;
bool paused;
bool gameOver;
bool waitForStart;
bool snakeDrawn;
unsigned long lastChange;
Point direction;
Point snakeParts[SNAKE_MAX_LENGTH];
int snakeLength;
int incStep;

ISR(TIMER1_OVF_vect)
{
    display.refresh();
}

void setup() {
    display.enableTimer1();
    display.setFont(Mono5x7);
    startGame();
}

void drawSnake(Bitmap::Color color) {
    for (int index = 0; index < snakeLength; ++index)
        display.setPixel(snakeParts[index].x, snakeParts[index].y, color);
}

void loop() {
    // Handle the "Game Over" state.  Any key press starts a new game.
    int cmd = ir.command();
    if (gameOver) {
        if (cmd != -1 && (cmd & IRreceiver::AUTO_REPEAT) == 0)
            startGame();
        return;
    }

    // Pause the game if waiting for the first start.  While waiting,
    // blink the location of the snake to help the player get their bearings.
    if (waitForStart) {
        if (cmd == -1) {
            if ((millis() - lastChange) >= SNAKE_BLINK_TIME) {
                snakeDrawn = !snakeDrawn;
                drawSnake(snakeDrawn ? Bitmap::White : Bitmap::Black);
                lastChange += SNAKE_BLINK_TIME;
            }
            return;
        }
        drawSnake(Bitmap::White);
        waitForStart = false;
        snakeDrawn = true;
        lastChange = millis();
    }

    // Process commands from the player.
    switch (cmd) {

    // Both arrow keys and numbers can be used to control the direction,
    // in case the remote control does not have arrow keys.
    case RC5_LEFT: case RC5_4:
        changeDirection(-1, 0);
        break;
    case RC5_RIGHT: case RC5_6:
        changeDirection(1, 0);
        break;
    case RC5_UP: case RC5_2:
        changeDirection(0, -1);
        break;
    case RC5_DOWN: case RC5_8:
        changeDirection(0, 1);
        break;

    case RC5_PAUSE: case RC5_PLAY: case RC5_0:
        // Pause or resume the game.
        paused = !paused;
        lastChange = millis();
        break;

    case RC5_STOP: case RC5_STANDBY:
        // Stop the game and start a new one.
        startGame();
        break;
    }

    // Advance the snake position if not paused and the timeout has expired.
    if (!paused && (millis() - lastChange) >= SNAKE_ADVANCE_TIME) {
        ++incStep;
        advanceSnake(incStep >= SNAKE_INC_STEPS);
        lastChange += SNAKE_ADVANCE_TIME;
    }
}

void startGame() {
    randomSeed(micros() + analogRead(A0)); // Analog read adds some noise.
    display.clear();
    display.drawRect(0, 0, display.width() - 1, display.height() - 1);
    for (int count = 0; count < 10; ++count) {
        int x, y;
        if (random(0, 2) == 0) {
            x = random(1, display.width() - 5);
            y = random(1, display.height() - 1);
            display.drawLine(x, y, x + 4, y);
        } else {
            x = random(1, display.width() - 1);
            y = random(1, display.height() - 3);
            display.drawLine(x, y, x, y + 2);
        }
    }
    paused = false;
    gameOver = false;
    waitForStart = true;
    snakeDrawn = true;
    lastChange = millis();
    direction.x = 1;
    direction.y = 0;
    incStep = 0;
    snakeLength = SNAKE_START_LENGTH;
    for (int index = 0; index < snakeLength; ++index) {
        snakeParts[index].x = 3 + index;
        snakeParts[index].y = 4;
        display.setPixel
            (snakeParts[index].x, snakeParts[index].y, Bitmap::White);
    }
}

void changeDirection(int x, int y) {
    direction.x = x;
    direction.y = y;
}

void advanceSnake(bool increase) {
    int x = snakeParts[snakeLength - 1].x + direction.x;
    int y = snakeParts[snakeLength - 1].y + direction.y;
    if (display.pixel(x, y) == Bitmap::White) {
        gameOver = true;
        display.clear();
        display.drawText(5, 0, "Game");
        display.drawText(3, 8, "Over!");
        return;
    }
    if (!increase || snakeLength >= SNAKE_MAX_LENGTH) {
        display.setPixel(snakeParts[0].x, snakeParts[0].y, Bitmap::Black);
        for (int index = 0; index < snakeLength - 1; ++index)
            snakeParts[index] = snakeParts[index + 1];
    } else {
        ++snakeLength;
    }
    snakeParts[snakeLength - 1].x = x;
    snakeParts[snakeLength - 1].y = y;
    display.setPixel(x, y, Bitmap::White);
    if (increase)
        incStep = 0;
}
