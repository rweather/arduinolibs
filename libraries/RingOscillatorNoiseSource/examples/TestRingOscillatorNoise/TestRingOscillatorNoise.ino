
// This example dumps the raw data from a ring oscillator noise source
// without any of the whitening normally performed by the RNG class.

#include <Crypto.h>
#include <RingOscillatorNoiseSource.h>

char const hexchars[] = "0123456789ABCDEF";

class RawNoiseSource : public RingOscillatorNoiseSource
{
public:
    RawNoiseSource() : RingOscillatorNoiseSource() {}

protected:
    void output(const uint8_t *data, size_t len, unsigned int credit)
    {
        for (size_t posn = 0; posn < len; ++posn) {
            uint8_t value = data[posn];
            Serial.print(hexchars[(value >> 4) & 0x0F]);
            Serial.print(hexchars[value & 0x0F]);
        }
        Serial.println();
    }
};

RawNoiseSource noise;
bool calibrating = true;

void setup() {
    Serial.begin(9600);
    Serial.println();
    Serial.println("calibrating");
}

void loop() {
    noise.stir();
    bool nowCalibrating = noise.calibrating();
    if (nowCalibrating != calibrating) {
        calibrating = nowCalibrating;
        if (calibrating)
            Serial.println("calibrating");
    }
}
