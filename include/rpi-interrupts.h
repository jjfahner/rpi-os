#pragma once

#include "rpi-base.h"

//
// An interrupt handler
//
typedef void (*irq_handler_t)(void);



//
// Enable IRQs
//
EXTERN_C void enable_interrupts(void);


//
// Disable IRQs
//
EXTERN_C void disable_interrupts(void);


//
// Register (and enable) an interrupt handler
//
EXTERN_C void register_irq_handler(uint8_t irq, irq_handler_t handler);

//
// Unregister (and disable) an interrupt handler
//
EXTERN_C void unregister_irq_handler(uint8_t irq);
