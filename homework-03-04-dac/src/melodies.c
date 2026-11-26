#include "melodies.h"

#define LENGTH(array) (sizeof(array) / sizeof(array[0]))

// C D E | C D E | C D E
static const note_t baby_shark[] = {
    { NOTE_C4, QUARTER }, { NOTE_D4, QUARTER }, { NOTE_E4, HALF },
    { NOTE_C4, QUARTER }, { NOTE_D4, QUARTER }, { NOTE_E4, HALF },
    { NOTE_C4, QUARTER }, { NOTE_D4, QUARTER }, { NOTE_E4, WHOLE },
};

// E E E | E E E | E G C D E
static const note_t jingle_bells[] = {
    { NOTE_E4, QUARTER }, { NOTE_E4, QUARTER }, { NOTE_E4, HALF },
    { NOTE_E4, QUARTER }, { NOTE_E4, QUARTER }, { NOTE_E4, HALF },
    { NOTE_E4, QUARTER }, { NOTE_G4, QUARTER }, { NOTE_C4, QUARTER },
    { NOTE_D4, QUARTER }, { NOTE_E4, WHOLE },
};

// C C D | C C D | C C D E
// Ритм "тук-тук-плеск": дві короткі ноти, довга, і пауза
static const note_t we_will_rock_you[] = {
    { NOTE_C4, EIGHTH }, { NOTE_C4, EIGHTH }, { NOTE_D4, QUARTER }, { REST, QUARTER },
    { NOTE_C4, EIGHTH }, { NOTE_C4, EIGHTH }, { NOTE_D4, QUARTER }, { REST, QUARTER },
    { NOTE_C4, EIGHTH }, { NOTE_C4, EIGHTH }, { NOTE_D4, QUARTER }, { NOTE_E4, HALF },
};

// C C G G | A A G
static const note_t twinkle_twinkle[] = {
    { NOTE_C4, QUARTER }, { NOTE_C4, QUARTER },
    { NOTE_G4, QUARTER }, { NOTE_G4, QUARTER },
    { NOTE_A4, QUARTER }, { NOTE_A4, QUARTER },
    { NOTE_G4, HALF },
};

// G G G D E E D
static const note_t old_macdonald[] = {
    { NOTE_G4, QUARTER }, { NOTE_G4, QUARTER }, { NOTE_G4, QUARTER },
    { NOTE_D4, QUARTER }, { NOTE_E4, QUARTER }, { NOTE_E4, QUARTER },
    { NOTE_D4, HALF },
};

const melody_t melodies[] = {
    { "Baby Shark",                  baby_shark,       LENGTH(baby_shark) },
    { "Jingle Bells",                jingle_bells,     LENGTH(jingle_bells) },
    { "We Will Rock You",            we_will_rock_you, LENGTH(we_will_rock_you) },
    { "Twinkle Twinkle Little Star", twinkle_twinkle,  LENGTH(twinkle_twinkle) },
    { "Old MacDonald",               old_macdonald,    LENGTH(old_macdonald) },
};

const size_t melodies_count = LENGTH(melodies);
