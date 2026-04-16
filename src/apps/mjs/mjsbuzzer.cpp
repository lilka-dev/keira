#include "mjsbuzzer.h"
#include <lilka.h>
#include "mjs.h"

// buzzer.play(freq[, duration])
static void mjs_buzzer_play(struct mjs* mjs) {
    int freq = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_val_t dur_arg = mjs_arg(mjs, 1);
    if (mjs_is_number(dur_arg)) {
        int duration = mjs_get_int(mjs, dur_arg);
        lilka::buzzer.play(freq, duration);
    } else {
        lilka::buzzer.play(freq);
    }
    mjs_return(mjs, mjs_mk_undefined());
}

// buzzer.play_melody(tones_array, tempo)
// tones_array: [[freq, size], [freq, size], ...]
static void mjs_buzzer_play_melody(struct mjs* mjs) {
    mjs_val_t arr = mjs_arg(mjs, 0);
    int tempo = mjs_get_int(mjs, mjs_arg(mjs, 1));

    int size = mjs_array_length(mjs, arr);
    lilka::Tone* melody = new lilka::Tone[size];
    for (int i = 0; i < size; i++) {
        mjs_val_t tone = mjs_array_get(mjs, arr, i);
        melody[i].frequency = mjs_get_int(mjs, mjs_array_get(mjs, tone, 0));
        melody[i].size = mjs_get_int(mjs, mjs_array_get(mjs, tone, 1));
    }
    lilka::buzzer.playMelody(melody, size, tempo);
    delete[] melody;
    mjs_return(mjs, mjs_mk_undefined());
}

// buzzer.stop()
static void mjs_buzzer_stop(struct mjs* mjs) {
    lilka::buzzer.stop();
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_buzzer_register(struct mjs* mjs) {
    mjs_val_t buzzer = mjs_mk_object(mjs);
    mjs_set(mjs, buzzer, "play", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_buzzer_play));
    mjs_set(mjs, buzzer, "play_melody", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_buzzer_play_melody));
    mjs_set(mjs, buzzer, "stop", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_buzzer_stop));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "buzzer", ~0, buzzer);

    // Register notes constants
    // clang-format off
    const lilka::Note noteValues[] = {
        lilka::NOTE_B0,
        lilka::NOTE_C1, lilka::NOTE_CS1, lilka::NOTE_D1, lilka::NOTE_DS1, lilka::NOTE_E1, lilka::NOTE_F1, lilka::NOTE_FS1, lilka::NOTE_G1, lilka::NOTE_GS1, lilka::NOTE_A1, lilka::NOTE_AS1, lilka::NOTE_B1,
        lilka::NOTE_C2, lilka::NOTE_CS2, lilka::NOTE_D2, lilka::NOTE_DS2, lilka::NOTE_E2, lilka::NOTE_F2, lilka::NOTE_FS2, lilka::NOTE_G2, lilka::NOTE_GS2, lilka::NOTE_A2, lilka::NOTE_AS2, lilka::NOTE_B2,
        lilka::NOTE_C3, lilka::NOTE_CS3, lilka::NOTE_D3, lilka::NOTE_DS3, lilka::NOTE_E3, lilka::NOTE_F3, lilka::NOTE_FS3, lilka::NOTE_G3, lilka::NOTE_GS3, lilka::NOTE_A3, lilka::NOTE_AS3, lilka::NOTE_B3,
        lilka::NOTE_C4, lilka::NOTE_CS4, lilka::NOTE_D4, lilka::NOTE_DS4, lilka::NOTE_E4, lilka::NOTE_F4, lilka::NOTE_FS4, lilka::NOTE_G4, lilka::NOTE_GS4, lilka::NOTE_A4, lilka::NOTE_AS4, lilka::NOTE_B4,
        lilka::NOTE_C5, lilka::NOTE_CS5, lilka::NOTE_D5, lilka::NOTE_DS5, lilka::NOTE_E5, lilka::NOTE_F5, lilka::NOTE_FS5, lilka::NOTE_G5, lilka::NOTE_GS5, lilka::NOTE_A5, lilka::NOTE_AS5, lilka::NOTE_B5,
        lilka::NOTE_C6, lilka::NOTE_CS6, lilka::NOTE_D6, lilka::NOTE_DS6, lilka::NOTE_E6, lilka::NOTE_F6, lilka::NOTE_FS6, lilka::NOTE_G6, lilka::NOTE_GS6, lilka::NOTE_A6, lilka::NOTE_AS6, lilka::NOTE_B6,
        lilka::NOTE_C7, lilka::NOTE_CS7, lilka::NOTE_D7, lilka::NOTE_DS7, lilka::NOTE_E7, lilka::NOTE_F7, lilka::NOTE_FS7, lilka::NOTE_G7, lilka::NOTE_GS7, lilka::NOTE_A7, lilka::NOTE_AS7, lilka::NOTE_B7,
        lilka::NOTE_C8, lilka::NOTE_CS8, lilka::NOTE_D8, lilka::NOTE_DS8, lilka::REST,
    };
    const char* noteNames[] = {
        "B0",
        "C1", "CS1", "D1", "DS1", "E1", "F1", "FS1", "G1", "GS1", "A1", "AS1", "B1",
        "C2", "CS2", "D2", "DS2", "E2", "F2", "FS2", "G2", "GS2", "A2", "AS2", "B2",
        "C3", "CS3", "D3", "DS3", "E3", "F3", "FS3", "G3", "GS3", "A3", "AS3", "B3",
        "C4", "CS4", "D4", "DS4", "E4", "F4", "FS4", "G4", "GS4", "A4", "AS4", "B4",
        "C5", "CS5", "D5", "DS5", "E5", "F5", "FS5", "G5", "GS5", "A5", "AS5", "B5",
        "C6", "CS6", "D6", "DS6", "E6", "F6", "FS6", "G6", "GS6", "A6", "AS6", "B6",
        "C7", "CS7", "D7", "DS7", "E7", "F7", "FS7", "G7", "GS7", "A7", "AS7", "B7",
        "C8", "CS8", "D8", "DS8", "REST",
    };
    // clang-format on
    mjs_val_t notes = mjs_mk_object(mjs);
    for (int i = 0; i < (int)(sizeof(noteValues) / sizeof(noteValues[0])); i++) {
        mjs_set(mjs, notes, noteNames[i], ~0, mjs_mk_number(mjs, noteValues[i]));
    }
    mjs_set(mjs, global, "notes", ~0, notes);
}
