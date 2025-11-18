#include "coleco_core.h"
#include "audio.h"

#include <esp_heap_caps.h>
#include <lilka.h>

extern "C" {
#include "core/emuapi.h"
}

using namespace lileco;

extern "C" {

void emu_printf(char* text) {
    if (!text) return;
    lilka::serial.log("%s", text);
}

void emu_printi(int val) {
    lilka::serial.log("%d", val);
}

void* emu_Malloc(int size) {
    return heap_caps_malloc_prefer(size, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
}

void emu_Free(void* ptr) {
    if (ptr) heap_caps_free(ptr);
}

int emu_FileOpen(char*) {
    return -1;
}

int emu_FileRead(char*, int) {
    return -1;
}

unsigned char emu_FileGetc(void) {
    return 0;
}

int emu_FileSeek(int) {
    return -1;
}

void emu_FileClose(void) {
}

int emu_FileSize(char*) {
    return 0;
}

int emu_LoadFile(char* filename, char* buf, int size) {
    auto* core = ColecoCore::getActive();
    if (!core) return 0;
    return core->loadFile(filename, reinterpret_cast<uint8_t*>(buf), size);
}

int emu_LoadFileSeek(char*, char*, int, int) {
    return 0;
}

void emu_InitJoysticks(void) {
}

int emu_SwapJoysticks(int) {
    return 0;
}

unsigned short emu_DebounceLocalKeys(void) {
    return 0;
}

int emu_ReadKeys(void) {
    auto* core = ColecoCore::getActive();
    return core ? core->currentKeyMask() : 0;
}

int emu_GetPad(void) {
    auto* core = ColecoCore::getActive();
    return core ? core->keypadValue() : 0;
}

int emu_ReadAnalogJoyX(int, int) {
    return 0;
}

int emu_ReadAnalogJoyY(int, int) {
    return 0;
}

int emu_ReadI2CKeyboard(void) {
    return 0;
}

int emu_setKeymap(int) {
    return 0;
}

void emu_sndInit(void) {
    lileco::ColecoAudio::instance().init();
}

void emu_sndPlaySound(int chan, int volume, int freq) {
    lileco::ColecoAudio::instance().setChannel(chan, freq, volume);
}

void emu_sndPlayBuzz(int, int) {
}

void emu_SetPaletteEntry(unsigned char r, unsigned char g, unsigned char b, int index) {
    auto* core = ColecoCore::getActive();
    if (core) core->updatePalette(index, r, g, b);
}

void emu_DrawScreen(unsigned char* VBuf, int width, int height, int stride) {
    auto* core = ColecoCore::getActive();
    if (core) core->drawFrame(VBuf, width, height, stride);
}

void emu_DrawLine(unsigned char* VBuf, int width, int, int line) {
    auto* core = ColecoCore::getActive();
    if (core) core->drawLine(VBuf, width, line);
}

void emu_DrawVsync(void) {
    auto* core = ColecoCore::getActive();
    if (core) core->presentFrame();
}

int emu_FrameSkip(void) {
    return 0;
}

void* emu_LineBuffer(int) {
    return nullptr;
}

} // extern "C"
