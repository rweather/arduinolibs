/* This example is placed into the public domain */

#include <Charlieplex.h>

byte pins[3] = {9, 10, 11};
Charlieplex charlie(pins, sizeof(pins));

int previous = 1;
int current = 0;
int step = 1;
unsigned long lastTime;

void setup() {
    lastTime = millis();
    charlie.setLed(current, true);
    charlie.setPwmLed(previous, 64);
}

void loop() {
    if ((millis() - lastTime) >= 100) {
        charlie.setLed(previous, false);
        charlie.setPwmLed(current, 64);
        previous = current;
        current += step;
        if (current < 0) {
            current = 1;
            step = 1;
        } else if (current >= charlie.count()) {
            current = charlie.count() - 2;
            step = -1;
        }
        charlie.setLed(current, true);
        lastTime += 100;
    }
    charlie.loop();
}
