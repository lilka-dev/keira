#pragma once

// Uncomment this line to get mutex debug log

//#define KEIRA_MUTEX_DEBUG

// Debug/Release mutex implementation
#ifdef KEIRA_MUTEX_DEBUG

#    define KMTX_LOCK(X)                            \
        {                                           \
            lilka::serial.log("LOCKED %p mtx", &X); \
            xSemaphoreTake(X, portMAX_DELAY);       \
        }
#    define KMTX_UNLOCK(X)                            \
        {                                             \
            lilka::serial.log("UNLOCKED %p mtx", &X); \
            xSemaphoreGive(X);                        \
        }
#else
#    define KMTX_LOCK(X)   xSemaphoreTake(X, portMAX_DELAY)

#    define KMTX_UNLOCK(X) xSemaphoreGive(X);
#endif

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
        this->VAR == VAR;          \
        KMTX_UNLOCK(MTX);          \
    }

// Generate both getter and setter
#define KMTX_SETER_GETER(TYPE, VAR, MTX) \
    KMTX_GETER(TYPE, VAR, MTX)           \
    KMTX_SETER(TYPE, VAR, MTX)
// easiest way to make c++ Rusty :D
