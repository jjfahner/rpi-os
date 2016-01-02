#pragma once

#include "rpi-base.h"


//
// Maximum mutex name length
//
#define MUTEX_NAME_LEN	32



//
// Mutex type
//
typedef struct mutex_t mutex_t;



//
// Create a mutex
//
EXTERN_C mutex_t* mutex_create(const char* name);



//
// Destroy a mutex
//
EXTERN_C void mutex_destroy(mutex_t* mutex);



//
// Try to lock a mutex
//
EXTERN_C uint32_t mutex_trylock(mutex_t* mutex);



//
// Lock a mutex
//
EXTERN_C void mutex_lock(mutex_t* mutex);



//
// Unlock a mutex
//
EXTERN_C void mutex_unlock(mutex_t* mutex);
