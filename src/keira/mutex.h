#pragma once
#include <lilka/serial.h>

// Uncomment this line to get mutex debug log

// #define KEIRA_MUTEX_DEBUG

// Debug/Release mutex implementation
#ifdef KEIRA_MUTEX_DEBUG
#    define KMTX_LOCK(X)                                                                       \
        {                                                                                      \
            lilka::serial.log("LOCKED %s:%d " #X " %p", __FILE__, __LINE__, (const void*)(X)); \
            xSemaphoreTake(X, portMAX_DELAY);                                                  \
        }
#    define KMTX_UNLOCK(X)                                                                       \
        {                                                                                        \
            lilka::serial.log("UNLOCKED %s:%d " #X " %p", __FILE__, __LINE__, (const void*)(X)); \
            xSemaphoreGive(X);                                                                   \
        }
#else
#    define KMTX_LOCK(X)   xSemaphoreTake(X, portMAX_DELAY)

#    define KMTX_UNLOCK(X) xSemaphoreGive(X);
#endif

#define NVS_LOCK   KMTX_LOCK(ksystem.nvsMTX);
#define NVS_UNLOCK KMTX_UNLOCK(ksystem.nvsMTX);

// Mutex protected geter boilerplate generator
#define KMTX_GETER(TYPE, VAR, MTX) \
    TYPE get##VAR() {              \
        KMTX_LOCK(MTX);            \
        auto tmp##VAR = VAR;       \
        KMTX_UNLOCK(MTX);          \
        return tmp##VAR;           \
    }

// Mutex protected seter boilerplate generator
#define KMTX_SETER(TYPE, VAR, MTX) \
    void set##VAR(TYPE VAR) {      \
        KMTX_LOCK(MTX);            \
        this->VAR = VAR;           \
        KMTX_UNLOCK(MTX);          \
    }

// Generate both getter and setter
#define KMTX_SETER_GETER(TYPE, VAR, MTX) \
    KMTX_GETER(TYPE, VAR, MTX)           \
    KMTX_SETER(TYPE, VAR, MTX)
// easiest way to make c++ Rusty :D
