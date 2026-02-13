#include "lualilka_resources.h"
#include "lualilka_audio.h"
#include "keira/ksound/sound.h"
#include "keira/ksound/audioplayer.h"

static bool lualilka_resources_removeFromRegistry(lua_State* L, const char* registryKey, const void* ptr) {
    lua_getfield(L, LUA_REGISTRYINDEX, registryKey);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        if (lua_islightuserdata(L, -1) && lua_touserdata(L, -1) == ptr) {
            lua_pop(L, 1);
            lua_pushvalue(L, -1);
            lua_pushnil(L);
            lua_rawset(L, -4);
            lua_pop(L, 1);
            return true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return false;
}

int lualilka_resources_loadImage(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    // Get dir from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    const char* dir = lua_tostring(L, -1);
    lua_pop(L, 1);
    String fullPath = String(dir) + "/" + path;
    // 2nd argument is optional transparency color (16-bit 5-6-5)
    int32_t transparencyColor = -1;
    if (lua_gettop(L) > 1) {
        if (lua_isnumber(L, 2)) {
            transparencyColor = lua_tointeger(L, 2);
        }
    }
    int32_t pivotX = 0;
    int32_t pivotY = 0;
    if (lua_gettop(L) > 3) {
        // Pivot X/Y
        pivotX = luaL_checkinteger(L, 3);
        pivotY = luaL_checkinteger(L, 4);
    }

    lilka::Image* image = lilka::resources.loadImage(fullPath, transparencyColor, pivotX, pivotY);

    if (!image) {
        return luaL_error(L, "Не вдалося завантажити зображення %s", fullPath.c_str());
    }

    lilka::serial.log(
        "lua: loaded image %s, size: %d x %d, pivot: %d,%d", path, image->width, image->height, pivotX, pivotY
    );

    // Append image to images table in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushlightuserdata(L, image);
    lua_setfield(L, -2, path);
    lua_pop(L, 1);

    // Create and return table that contains image width, height and pointer
    lua_newtable(L);
    lua_pushinteger(L, image->width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, image->height);
    lua_setfield(L, -2, "height");
    lua_pushlightuserdata(L, image);
    lua_setfield(L, -2, "pointer");

    return 1;
}

int lualilka_resources_rotateImage(lua_State* L) {
    // Args are image table and angle in degrees
    // First argument is table that contains image width, height and pointer. We need all of them.
    // Second argument is angle in degrees.
    // Third argument is blank color for pixels that are not covered by the image after rotation.
    lua_getfield(L, 1, "pointer");
    // Check if value is a valid pointer
    if (!lua_islightuserdata(L, -1)) {
        return luaL_error(L, "Невірне зображення");
    }
    lilka::Image* image = (lilka::Image*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    int16_t angle = luaL_checknumber(L, 2);
    int32_t blankColor = luaL_checkinteger(L, 3);

    // Instantiate a new image
    lilka::Image* rotatedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    // Rotate the image
    image->rotate(angle, rotatedImage, blankColor);

    // Append rotatedImage to images table in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushlightuserdata(L, rotatedImage);
    lua_setfield(L, -2, (String("rotatedImage-") + random(100000)).c_str());
    lua_pop(L, 1);

    // Create and return table that contains image width, height and pointer
    lua_newtable(L);
    lua_pushinteger(L, rotatedImage->width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, rotatedImage->height);
    lua_setfield(L, -2, "height");
    lua_pushlightuserdata(L, rotatedImage);
    lua_setfield(L, -2, "pointer");

    return 1;
}

int lualilka_resources_flipImageX(lua_State* L) {
    // Arg is image table
    // First argument is table that contains image width, height and pointer. We need all of them.
    lua_getfield(L, 1, "pointer");
    // Check if value is a valid pointer
    if (!lua_islightuserdata(L, -1)) {
        return luaL_error(L, "Невірне зображення");
    }
    lilka::Image* image = (lilka::Image*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    // Instantiate a new image
    lilka::Image* flippedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    // Rotate the image
    image->flipX(flippedImage);

    // Append rotatedImage to images table in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushlightuserdata(L, flippedImage);
    lua_setfield(L, -2, (String("xFlippedImage-") + random(100000)).c_str());
    lua_pop(L, 1);

    // Create and return table that contains image width, height and pointer
    lua_newtable(L);
    lua_pushinteger(L, flippedImage->width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, flippedImage->height);
    lua_setfield(L, -2, "height");
    lua_pushlightuserdata(L, flippedImage);
    lua_setfield(L, -2, "pointer");

    return 1;
}

int lualilka_resources_flipImageY(lua_State* L) {
    // Arg is image table
    // First argument is table that contains image width, height and pointer. We need all of them.
    lua_getfield(L, 1, "pointer");
    // Check if value is a valid pointer
    if (!lua_islightuserdata(L, -1)) {
        return luaL_error(L, "Невірне зображення");
    }
    lilka::Image* image = (lilka::Image*)lua_touserdata(L, -1);
    lua_pop(L, 1);

    // Instantiate a new image
    lilka::Image* flippedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    // Rotate the image
    image->flipY(flippedImage);

    // Append rotatedImage to images table in registry
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushlightuserdata(L, flippedImage);
    lua_setfield(L, -2, (String("yFlippedImage-") + random(100000)).c_str());
    lua_pop(L, 1);

    // Create and return table that contains image width, height and pointer
    lua_newtable(L);
    lua_pushinteger(L, flippedImage->width);
    lua_setfield(L, -2, "width");
    lua_pushinteger(L, flippedImage->height);
    lua_setfield(L, -2, "height");
    lua_pushlightuserdata(L, flippedImage);
    lua_setfield(L, -2, "pointer");

    return 1;
}

int lualilka_resources_loadAudio(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    // Get dir from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    const char* dir = lua_tostring(L, -1);
    lua_pop(L, 1);
    String fullPath = String(dir) + "/" + path;

    // Detect audio type by extension
    String lowerPath = fullPath;
    lowerPath.toLowerCase();
    const char* type = NULL;
    if (lowerPath.endsWith(".mod")) {
        type = "mod";
    } else if (lowerPath.endsWith(".wav")) {
        type = "wav";
    } else if (lowerPath.endsWith(".mp3")) {
        type = "mp3";
    } else if (lowerPath.endsWith(".aac")) {
        type = "aac";
    } else if (lowerPath.endsWith(".flac")) {
        type = "flac";
    } else {
        return luaL_error(L, "Непідтримуваний формат аудіо: %s", fullPath.c_str());
    }

    FILE* file = fopen(fullPath.c_str(), "rb");
    if (!file) {
        return luaL_error(L, "Не вдалося відкрити аудіо-файл %s", fullPath.c_str());
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize == 0) {
        fclose(file);
        return luaL_error(L, "Аудіо-файл порожній: %s", fullPath.c_str());
    }

    uint8_t* data = new (std::nothrow) uint8_t[fileSize];
    if (!data) {
        fclose(file);
        return luaL_error(L, "Недостатньо пам'яті для аудіо-файлу %s (%d байт)", fullPath.c_str(), fileSize);
    }

    size_t bytesRead = fread(data, 1, fileSize, file);
    fclose(file);

    if (bytesRead != fileSize) {
        delete[] data;
        return luaL_error(L, "Не вдалося прочитати аудіо-файл %s", fullPath.c_str());
    }

    lilka::serial.log("lua: loaded audio %s, size: %d bytes", path, fileSize);

    // Create Sound object (takes ownership of data)
    lilka::Sound* sound = new lilka::Sound(data, fileSize, type);

    // Store Sound* in "sounds" registry table
    lua_getfield(L, LUA_REGISTRYINDEX, "sounds");
    lua_pushlightuserdata(L, sound);
    lua_setfield(L, -2, path);
    lua_pop(L, 1);

    // Return table: { size = <number>, type = <string>, pointer = <lightuserdata to Sound*> }
    lua_newtable(L);
    lua_pushinteger(L, fileSize);
    lua_setfield(L, -2, "size");
    lua_pushstring(L, type);
    lua_setfield(L, -2, "type");
    lua_pushlightuserdata(L, sound);
    lua_setfield(L, -2, "pointer");

    return 1;
}

int lualilka_resources_delete(lua_State* L) {
    lua_getfield(L, 1, "pointer");
    if (!lua_islightuserdata(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }
    void* ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);

    if (lualilka_resources_removeFromRegistry(L, "images", ptr)) {
        delete (lilka::Image*)ptr;
    } else if (lualilka_resources_removeFromRegistry(L, "sounds", ptr)) {
        lilka::Sound* sound = static_cast<lilka::Sound*>(ptr);
        if (lilka::audioPlayer.getPlayingSound() == sound) {
            lilka::audioPlayer.stop();
        }
        delete sound;
    }

    lua_pushnil(L);
    lua_setfield(L, 1, "pointer");

    return 0;
}

int lualilka_resources_readFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    // Get dir from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    const char* dir = lua_tostring(L, -1);
    lua_pop(L, 1);
    String fullPath = String(dir) + "/" + path;

    String fileContent;
    int result = lilka::resources.readFile(fullPath, fileContent);
    if (result) {
        return luaL_error(L, "Не вдалося прочитати файл %s", fullPath.c_str());
    }

    lua_pushstring(L, fileContent.c_str());
    return 1;
}

int lualilka_resources_writeFile(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);
    const char* content = luaL_checkstring(L, 2);
    // Get dir from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "dir");
    const char* dir = lua_tostring(L, -1);
    lua_pop(L, 1);
    String fullPath = String(dir) + "/" + path;

    int result = lilka::resources.writeFile(fullPath, content);
    if (result) {
        return luaL_error(L, "Не вдалося записати файл %s", fullPath.c_str());
    }

    return 0;
}

static const luaL_Reg lualilka_resources[] = {
    {"load_image", lualilka_resources_loadImage},
    {"rotate_image", lualilka_resources_rotateImage},
    {"flip_image_x", lualilka_resources_flipImageX},
    {"flip_image_y", lualilka_resources_flipImageY},
    {"load_audio", lualilka_resources_loadAudio},
    {"delete", lualilka_resources_delete},
    {"read_file", lualilka_resources_readFile},
    {"write_file", lualilka_resources_writeFile},
    {NULL, NULL},
};

int lualilka_resources_register(lua_State* L) {
    // Create global "resources" table that contains all resources functions
    luaL_newlib(L, lualilka_resources);
    lua_setglobal(L, "resources");
    return 0;
}
