#pragma once

// Uncomment this line to get mutex debug log

//#define KEIRA_MUTEX_DEBUG

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
