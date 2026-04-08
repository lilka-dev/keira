#include "mjsaudio.h"
#include <lilka.h>
#include "mjs.h"
#include "keira/ksound/audioplayer.h"
#include "keira/ksound/sound.h"

// audio.play(sound) - sound is {pointer, size, type}
static void mjs_audio_play(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Sound* sound = static_cast<lilka::Sound*>(mjs_get_ptr(mjs, ptr_val));
    lilka::audioPlayer.play(sound);
    mjs_return(mjs, mjs_mk_undefined());
}

// audio.stop()
static void mjs_audio_stop(struct mjs* mjs) {
    lilka::audioPlayer.stop();
    mjs_return(mjs, mjs_mk_undefined());
}

// audio.pause()
static void mjs_audio_pause(struct mjs* mjs) {
    lilka::audioPlayer.pause();
    mjs_return(mjs, mjs_mk_undefined());
}

// audio.resume()
static void mjs_audio_resume(struct mjs* mjs) {
    lilka::audioPlayer.resume();
    mjs_return(mjs, mjs_mk_undefined());
}

// audio.set_volume(gain)
static void mjs_audio_set_volume(struct mjs* mjs) {
    float gain = mjs_get_double(mjs, mjs_arg(mjs, 0));
    lilka::audioPlayer.setGain(gain);
    mjs_return(mjs, mjs_mk_undefined());
}

// audio.get_volume() -> number
static void mjs_audio_get_volume(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, lilka::audioPlayer.getGain()));
}

// audio.is_playing() -> boolean
static void mjs_audio_is_playing(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_boolean(mjs, lilka::audioPlayer.isPlaying()));
}

void mjs_audio_register(struct mjs* mjs) {
    mjs_val_t audio = mjs_mk_object(mjs);
    mjs_set(mjs, audio, "play", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_play));
    mjs_set(mjs, audio, "stop", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_stop));
    mjs_set(mjs, audio, "pause", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_pause));
    mjs_set(mjs, audio, "resume", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_resume));
    mjs_set(mjs, audio, "set_volume", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_set_volume));
    mjs_set(mjs, audio, "get_volume", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_get_volume));
    mjs_set(mjs, audio, "is_playing", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_audio_is_playing));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "audio", ~0, audio);
}
