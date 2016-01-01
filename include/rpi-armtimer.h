#pragma once

#include "rpi-base.h"


//
// Enable the ARM timer
//
EXTERN_C void arm_timer_enable(uint32_t interval);



//
// Disable the ARM timer
//
EXTERN_C void arm_timer_disable(void);



//
// Invoked from the IRQ handler when the arm timer elapses
//
EXTERN_C void arm_timer_interrupt(void);
