/*
Sketch that manipulates Arduino outputs to create Star Trek Enterprise style
running lights and LED chasers.

This example is placed into the public domain.
*/

#include <BlinkLED.h>
#include <ChaseLEDs.h>

#define NACELLE_RATE         A0    // Analog input for reading the nacelle chase rate
#define NAV_LIGHTS           A2    // Output pin for controlling the navigation lights
#define STROBE_LIGHT         A3    // Output pin for controlling the strobe

// Configurable parameters.
#define NAV_LIGHTS_ON        1000  // Time the navigation lights are on (milliseconds)
#define NAV_LIGHTS_OFF       1000  // Time the navigation lights are off (milliseconds)
#define STROBE_LIGHT_ON      70    // Time the strobe light is on (milliseconds)
#define STROBE_LIGHT_OFF     830   // Time the strobe light is off (milliseconds)
#define NACELLE_CHASE_LEN    6     // Length of nacelle chase, 1..6
#define NACELLE_MIN_PERIOD   25    // Minimum time to advance the nacelle chase (milliseconds)
#define NACELLE_MAX_PERIOD   250   // Maximum time to advance the nacelle chase (milliseconds)
#define NACELLE_DIM_VALUE    32    // Value for dimming previous LED in chase, 0..255

// Output pins to use for the nacelle chase
byte nacelleChasePins[6] = {3, 5, 6, 9, 10, 11};

class NacelleChaseLEDs : public ChaseLEDs
{
public:
  NacelleChaseLEDs(const byte *pins, int num)
    : ChaseLEDs(pins, num, 0) {}

protected:
  void advance(byte prevPin, byte nextPin) {
    digitalWrite(previousPin(2), LOW);
    analogWrite(prevPin, NACELLE_DIM_VALUE);
    digitalWrite(nextPin, HIGH);
    setAdvanceTime(map(analogRead(NACELLE_RATE), 0, 1023, NACELLE_MIN_PERIOD, NACELLE_MAX_PERIOD));
  }
};

NacelleChaseLEDs nacelleChase(nacelleChasePins, NACELLE_CHASE_LEN);

BlinkLED navLights(NAV_LIGHTS, NAV_LIGHTS_ON, NAV_LIGHTS_OFF);
BlinkLED strobeLight(STROBE_LIGHT, STROBE_LIGHT_ON, STROBE_LIGHT_OFF);

void setup() {
  // Turn off the status LED on the Arduino board (we don't need it).
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {
  navLights.loop();
  strobeLight.loop();
  nacelleChase.loop();
}

