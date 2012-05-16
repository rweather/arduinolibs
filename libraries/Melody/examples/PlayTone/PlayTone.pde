#include <Melody.h>

int notes[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3,
    NOTE_REST, NOTE_B3, NOTE_C4, NOTE_REST
};
byte lengths[] = {4, 8, 8, 4, 4, 4, 4, 4, 2};

Melody melody(8);

void setup() {
    melody.setMelody(notes, lengths, sizeof(lengths));
    melody.setLoopCount(3);
    melody.play();
}

void loop() {
    melody.run();
}
