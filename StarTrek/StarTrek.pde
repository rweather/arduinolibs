/*
Sketch that manipulates Arduino outputs to create Star Trek Enterprise style
running lights and LED chasers.  There are 4 types of lights on the Enterprise:

   * Static lights in windows, engines, and the deflector dish.
   * Navigation lights on the left and right of the saucer with a 1 second period.
   * Strobe light that comes on briefly every second.
   * Nacelle lights that perform a LED chase in the front of the nacelle.
     There is provision for a chase of between 1 and 6 LED's.  A typical configuration
     is 5 LED's in sequence (clockwise in one nacelle, counter-clockwise in the other).
     Another configuration uses a 3 LED sequence, driving 6 LED's organised in
     opposite pairs.

The static lights are not handled by this sketch.  They are assumed to be
connected directly to the Vcc and GND supply lines.

The nacelle lights use the Arduino's PWM outputs, so that the previous LED in the
chase can be dimmed slightly rather than completely turned off, to give a trailing
flame effect.  NACELLE_DIM_VALUE can be used to set the amount of dimming
(0 = completely off, 255 = completely on).

All outputs should be connected to the base of an NPN transistor (e.g. BC548)
via a 10K resistor to drive the actual LED's, as there will typically be multiple
LED's on each output.  For example, there are typically 4 navigation lights:
red/green on the top of the saucer and red/green on the bottom of the saucer.
Nacelle lights with a 3 LED chase will have 4 LED's on each output - two in
each nacelle.
*/

#include <BlinkLED.h>

#define NAV_LIGHTS    A2    // Red/green navigational lights
#define STROBE_LIGHT  A3    // Strobe light
#define NACELLE_1     3     // Nacelle twirl chase LED 1
#define NACELLE_2     5     // Nacelle twirl chase LED 2
#define NACELLE_3     6     // Nacelle twirl chase LED 3
#define NACELLE_4     9     // Nacelle twirl chase LED 4
#define NACELLE_5     10    // Nacelle twirl chase LED 5
#define NACELLE_6     11    // Nacelle twirl chase LED 6
#define NACELLE_RATE  A0    // Analog input that defines the rate of the nacelle chase

#define NAV_LIGHTS_ON        1000
#define NAV_LIGHTS_OFF       1000
#define STROBE_LIGHT_ON      70
#define STROBE_LIGHT_OFF     830
#define NACELLE_MIN_PERIOD   25
#define NACELLE_MAX_PERIOD   250
#define NACELLE_DIM_VALUE    32    // Value for dimming previous LED in chase, 0..255

byte nacelleChase[6] = {
  NACELLE_1,
  NACELLE_2,
  NACELLE_3,
  NACELLE_4,
  NACELLE_5,
  NACELLE_6
};
byte nacelleChaseLen = 6; // Select the length of the nacelle chase: 1 to 6.

unsigned long startTime;
unsigned long lastNacelleTime = 0;
unsigned long nacellePeriod = 0;
byte index = 0;

BlinkLED navLights(NAV_LIGHTS, NAV_LIGHTS_ON, NAV_LIGHTS_OFF);
BlinkLED strobeLight(STROBE_LIGHT, STROBE_LIGHT_ON, STROBE_LIGHT_OFF);

void setup() {
  // Configure the outputs and set them to be initially LOW.
  for (int index = 0; index < nacelleChaseLen; ++index) {
    byte pin = nacelleChase[index];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  
  // Turn off the status LED on the Arduino board (we don't need it).
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // Initialize the analog input for the nacelle chaser rate.
  pinMode(NACELLE_RATE, INPUT);
  digitalWrite(NACELLE_RATE, LOW);
  
  // Record the starting time, for controlling the chase and blinks.
  startTime = millis();
  index = nacelleChaseLen - 1;
}

// Note: there will be discontinuity in the chase/blink rate when millis()
// wraps around after 49 days.  Should automatically resync after a second or two.

void loop() {
  // How long since the application started?
  unsigned long sinceStart = millis() - startTime;
  
  // Update the navigation and strobe lights.
  navLights.loop();
  strobeLight.loop();

  // Update the nacelle lights - uniform LED chase of length 1 to 6.
  if ((sinceStart - lastNacelleTime) >= nacellePeriod) {
    // Advance to the next pin in sequence.
    index = (index + 1) % nacelleChaseLen;
    int currentPin = nacelleChase[index];
    int prevPin1 = nacelleChase[(index + nacelleChaseLen - 1) % nacelleChaseLen];
    int prevPin2 = nacelleChase[(index + nacelleChaseLen - 2) % nacelleChaseLen];
    digitalWrite(prevPin2, LOW);
    analogWrite(prevPin1, NACELLE_DIM_VALUE);
    digitalWrite(currentPin, HIGH);
    
    // Read the chase rate from the trimpot on A0 and determine the next timeout period.
    lastNacelleTime = sinceStart;
    int val = analogRead(NACELLE_RATE);
    nacellePeriod = map(val, 0, 1023, NACELLE_MIN_PERIOD, NACELLE_MAX_PERIOD);
  }
}

