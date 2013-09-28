/*
Sketch that manipulates Arduino outputs to create the "Cylon Eyes" effect from
Battlestar Galactica.  It uses the ChaseLEDs utility class.

This example is placed into the public domain.
*/

#include <ChaseLEDs.h>

class CylonChase : public ChaseLEDs
{
public:
  CylonChase(const byte *pins, int num, unsigned long advanceTime)
    : ChaseLEDs(pins, num, advanceTime) {}

protected:
  void advance(byte prevPin, byte nextPin) {
    digitalWrite(previousPin(2), LOW);
    analogWrite(prevPin, 32);
    digitalWrite(nextPin, HIGH);
    setAdvanceTime(map(analogRead(A0), 0, 1023, 25, 250));
  }
};

byte pins[] = {3, 5, 6, 9, 10, 11, 10, 9, 6, 5};
CylonChase cylonEyes(pins, sizeof(pins), 100);

void setup() {}

void loop() {
  cylonEyes.loop();
}

