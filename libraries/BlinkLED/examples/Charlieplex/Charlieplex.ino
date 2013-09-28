/* This example is placed into the public domain */

#include <Charlieplex.h>

byte pins[3] = {9, 10, 11};
Charlieplex charlie(pins, sizeof(pins));

void setup() {
    charlie.setLed(0, true);    // Turn on LED1
    charlie.setLed(3, true);    // Turn on LED4
    charlie.setPwmLed(5, 64);   // Set LED6 to one-quarter on
}

void loop() {
    charlie.loop();
}
