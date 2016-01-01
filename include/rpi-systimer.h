#pragma once

#include "rpi-base.h"

//
// Wait functions
//
EXTERN_C void sys_timer_wait_us(uint32_t us);
EXTERN_C void sys_timer_wait_ms(uint32_t ms);



//
// Enable the system timer
//
EXTERN_C void sys_timer_enable();



//
// System timer callback
//
typedef void(*sys_timer_proc_t)(uint32_t timer_id);



//
// Set a timer
//
// Interval is in microseconds, restricted in precision by the arm system timer.
// Count is the number of times the timer will be invoked, pass 0 for infinite.
// Callback is the function invoked when the timer elapsed. Called from the interrupt handler,
// so code must limit register use and cycle count.
//
EXTERN_C uint32_t sys_timer_install(uint32_t interval, uint32_t count, sys_timer_proc_t callback);
