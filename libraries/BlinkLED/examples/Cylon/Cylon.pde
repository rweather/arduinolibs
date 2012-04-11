/*
Sketch that manipulates Arduino outputs to create the "Cylon Eyes" effect from
Battlestar Galactica.  It uses the ChaseLEDs utility class.

This example is placed into the public domain.
*/

#include <ChaseLEDs.h>

byte pins[] = {3, 5, 6, 9, 10, 11, 10, 9, 6, 5};
ChaseLEDs cylonEyes(pins, sizeof(pins), 100);

void setup() {}

void loop() {
  cylonEyes.loop();
}

