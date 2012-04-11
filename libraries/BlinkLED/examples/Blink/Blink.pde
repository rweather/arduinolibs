/*
Blink the status LED using the BlinkLED utility class.

This example is placed into the public domain.
*/

#include <BlinkLED.h>

BlinkLED statusBlink(13, 70, 930);

void setup() {}

void loop() {
  statusBlink.loop();
}

