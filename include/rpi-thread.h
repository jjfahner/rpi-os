#pragma once

#include "rpi-base.h"
#include "rpi-mutex.h"


//
// The maximum number of threads supported by the system
//
#define THREAD_MAX_COUNT			128



//
// Maximum thread name length
//
#define THREAD_NAME_LEN				32



//
// Thread id
//
typedef uint32_t thread_id_t;



//
// Thread function
//
typedef void(*thread_fun_t)(uint32_t);



//
// Create the main thread object
//
EXTERN_C void thread_scheduler();



//
// Create a thread
//
EXTERN_C thread_id_t thread_create(uint32_t stack_size, char const* name, thread_fun_t thread_fun, uint32_t thread_arg);



//
// Exit the current thread
//
// Note: this destroys the thread stack without unwinding, so any memory or resources held on the stack
//       will leak.
//
EXTERN_C void thread_exit();



//
// Get the current thread.
//
EXTERN_C thread_id_t thread_current(void);



//
// Sleep thread
//
EXTERN_C void thread_sleep_us(uint32_t microseconds);



//
// Sleep thread
//
EXTERN_C void thread_sleep_ms(uint32_t milliseconds);



//
// Wait for a mutex
//
EXTERN_C void thread_wait_mutex(struct mutex_t* mutex);



//
// Yield thread time slice
//
EXTERN_C void thread_yield();



//
// Suspend current thread
//
EXTERN_C void thread_suspend();
