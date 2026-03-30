#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
//  Thread Manager (Base class of any system manager)
//////////////////////////////////////////////////////////////////////////////
// Purpose of this component to synchronise all persistent in a system
// threads, allowing us to define diferent managers doing pretty much similar
// job, but also providing additional function.
//
// For example AppManager is a ThreadManager doing exactly same, but allowing
// to draw different things on a screen, while ServiceManager provides a
// function of global persistence.
//
// Cause any thread can gave a birth to another threads, some of which can't
// be simply defined as an Application or Service, someone still have to
// care about them as well. This task is to be done here.
//////////////////////////////////////////////////////////////////////////////

// System components;
#include "keira/mutex.h"
#include "keira/thread.h"
// Libraries:
#include <vector>

#include "keira/bits/threadmanager.h"
// Uncomment to get debug information
//#define KEIRA_THREADMANAGER_DEBUG

#define KT_PCAST(X) reinterpret_cast<KeiraThread*>(X)

class ThreadManager : public KeiraThread {
public:
    ~ThreadManager();
    ThreadManager();

    // Spawns a new thread. Thread to be launched on next update
    virtual void spawn(KeiraThread* thread, bool autoSuspend = false);

    // Seek thread by name
    KeiraThread* operator[](const char* name);

    virtual void threadsClean();
    virtual void threadsRun();

    std::vector<KeiraThread*>::iterator begin() {
        return threads.begin();
    }

    std::vector<KeiraThread*>::iterator end() {
        return threads.end();
    }

    // Performs launch of thread updates. run in a separate thread
    void run() override;

protected:
    std::vector<KeiraThread*> threads;
    std::vector<KeiraThread*> threadsToRun;
    // Used to protect our data from external threads
    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
};
