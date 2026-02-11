#include <lilka.h>

#include "lualilka_audio.h"
#include "keira/ksound/audioplayer.h"
#include "keira/ksound/sound.h"

int lualilka_audio_play(lua_State* L) {
    // Arg: table with "pointer" (lightuserdata to lilka::Sound*)
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "pointer");
    if (!lua_islightuserdata(L, -1)) {
        return luaL_error(L, "Невірний аудіо-ресурс");
    }
    lilka::Sound* sound = static_cast<lilka::Sound*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!lilka::AudioPlayer::getInstance()->play(sound)) {
        return luaL_error(L, "Непідтримуваний формат аудіо: %s", sound->type);
    }
    return 0;
}

int lualilka_audio_stop(lua_State* L) {
    lilka::AudioPlayer::getInstance()->stop();
    return 0;
}

int lualilka_audio_pause(lua_State* L) {
    lilka::AudioPlayer::getInstance()->pause();
    return 0;
}

int lualilka_audio_resume(lua_State* L) {
    lilka::AudioPlayer::getInstance()->resume();
    return 0;
}

int lualilka_audio_set_volume(lua_State* L) {
    float gain = luaL_checknumber(L, 1);
    lilka::AudioPlayer::getInstance()->setGain(gain);
    return 0;
}

int lualilka_audio_get_volume(lua_State* L) {
    lua_pushnumber(L, lilka::AudioPlayer::getInstance()->getGain());
    return 1;
}

int lualilka_audio_is_playing(lua_State* L) {
    lua_pushboolean(L, lilka::AudioPlayer::getInstance()->isPlaying());
    return 1;
}

static const luaL_Reg lualilka_audio[] = {
    {"play", lualilka_audio_play},
    {"stop", lualilka_audio_stop},
    {"pause", lualilka_audio_pause},
    {"resume", lualilka_audio_resume},
    {"set_volume", lualilka_audio_set_volume},
    {"get_volume", lualilka_audio_get_volume},
    {"is_playing", lualilka_audio_is_playing},
    {NULL, NULL},
};

int lualilka_audio_register(lua_State* L) {
    luaL_newlib(L, lualilka_audio);
    lua_setglobal(L, "audio");
    return 0;
}

void lualilka_audio_cleanup() {
    lilka::AudioPlayer::getInstance()->cleanup();
}
