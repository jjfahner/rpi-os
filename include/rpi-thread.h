#pragma once

#include "rpi-base.h"
#include "rpi-mutex.h"


//
// The maximum number of threads supported by the system
//
// Note: this must always be a power of two.
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
// Invalid thread id
//
#define THREAD_INVALID_ID				((thread_id_t)0)



//
// Thread ID of the main thread
//
#define THREAD_SCHEDULER_THREAD_ID		((thread_id_t)1)



//
// Thread function
//
typedef void(*thread_fun_t)(uint32_t);



//
// The thread scheduler function
//
EXTERN_C void thread_scheduler();



//
// Returns whether this is the scheduler thread
//
EXTERN_C uint32_t thread_is_scheduler_thread();



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
// Get the current thread id
//
EXTERN_C thread_id_t thread_get_id(void);



//
// Get the thread name for a thread id
//
EXTERN_C const char* thread_name(thread_id_t thread_id);



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
