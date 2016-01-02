#pragma once

#include "rpi-base.h"
#include "rpi-interrupts.h"


//
// Init functions
//
EXTERN_C void uart_enable();
EXTERN_C void uart_term();
EXTERN_C void uart_enable_rx_interrupt(irq_handler_t handler);
EXTERN_C void uart_disable_rx_interrupt();



//
// Input/output functions
//
EXTERN_C uint8_t uart_trygetc(uint8_t* byte);
EXTERN_C uint8_t uart_getc(void);

EXTERN_C uint8_t uart_tryputc(uint8_t byte);
EXTERN_C void uart_putc(uint8_t byte);

EXTERN_C void uart_puts_len(const char *str, int len);
EXTERN_C void uart_puts(const char *str);
