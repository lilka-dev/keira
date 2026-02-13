#include "sound.h"
#include <cstring>

namespace lilka {

Sound::Sound(uint8_t* data, size_t size, const char* type) : data(data), size(size) {
    strncpy(this->type, type, sizeof(this->type) - 1);
    this->type[sizeof(this->type) - 1] = '\0';
}

Sound::~Sound() {
    delete[] data;
}

} // namespace lilka
