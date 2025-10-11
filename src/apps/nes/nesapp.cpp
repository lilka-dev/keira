#include "nesapp.h"
#include "driver.h"

NesApp::NesApp(String path) : App("NES", 0, 0, lilka::display.width(), lilka::display.height()) {
    argv[0] = new char[path.length() + 1];
    strcpy(argv[0], path.c_str());
#ifdef NESAPP_INTERLACED
    setFlags(static_cast<AppFlags>(AppFlags::APP_FLAG_FULLSCREEN | AppFlags::APP_FLAG_INTERLACED));
#else
    setFlags(static_cast<AppFlags>(AppFlags::APP_FLAG_FULLSCREEN));
#endif
}

NesApp::~NesApp() {
    delete[] argv[0];
}

void NesApp::run() {
    // Load the ROM
    Driver::setNesApp(this);
    nofrendo_main(1, argv);
    // TODO: Figure out how to terminate the emulator without crashing

    // well, cause our emulator always checking keys in osd_getinput()
    // we could use it to pass to emulator own events, whose aren't
    // really triggered by buttons. so if we pass event_osd_quit
    // it would end this main_loop() we're always runing.

    // memory whose our nes asked to allocate we could track inside our osd.cpp/mem_alloc()
    // just push all allocated blocks to some sort of vector with pointers in global scope
    // to then heap_caps_free_or_something()
    // there's a chance that our memory corrupted already, actually our emulator throws in
    // us that fancy message, so there's a chance we would die immediately on this FREE action
    // which of course sucks, but at least a minimal plan

    // in case that's true, and we're in this pretty bad situation, I think we could reuse that
    // cool osd_ based api to build implementation for like idk, normal x86(it's pretty gcc buildable)
    // to then use tools like valgrind to detect and resolve dat problem, maybe even optimize it better
}
