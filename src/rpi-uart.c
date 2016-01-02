/*
	Licensed to the Apache Software Foundation (ASF) under one
	or more contributor license agreements.  See the NOTICE file
	distributed with this work for additional information
	regarding copyright ownership.  The ASF licenses this file
	to you under the Apache License, Version 2.0 (the
	"License"); you may not use this file except in compliance
	with the License.  You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing,
	software distributed under the License is distributed on an
	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
	KIND, either express or implied.  See the License for the
	specific language governing permissions and limitations
	under the License.
*/
#include <stdint.h>
#include "rpi-uart.h"
#include "rpi-gpio.h"
#include "rpi-mutex.h"
#include "rpi-thread.h"
#include "rpi-systimer.h"
#include "asm-functions.h"

#include <stdio.h>


//
// Enable the UART mutex
//
#define UART_USE_LOCK



#ifdef UART_USE_LOCK

//
// A mutex to protect the UART
//
static mutex_t* uart_mutex = 0;

#endif



//
// Enable the PL011 UART
// 
void uart_enable() 
{
	// Disable UART0
	rpi_uart->cr = 0;
	_spin(150);

	// Disable pull up/down for all GPIO pins & _spin for 150 cycles
	rpi_gpio->GPPUD = 0;
	_spin(150);

	// Disable pull up/down for pin 14,15 & _spin for 150 cycles
	rpi_gpio->GPPUDCLK0 = (1 << 14) | (1 << 15);
	_spin(150);

	// Write 0 to GPPUDCLK0 to make it take effect
	rpi_gpio->GPPUDCLK0 = 0;
	_spin(150);

	// Clear pending interrupts
	rpi_uart->icr = 0x7FF;

	// Set integer & fractional part of baud rate
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 3000000; Baud = 115200

	// Divider = 3000000/(16 * 115200) = 1.627 = ~1
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40
	rpi_uart->ibrd = 1;
	rpi_uart->fbrd = 40;

	// Enable FIFO & 8 bit data transmission (1 stop bit, no parity)
	rpi_uart->lcrh = (1 << 4) | (1 << 5) | (1 << 6);

	// Mask all interrupts
	rpi_uart->imsc = UART0_OEIM | UART0_BEIM | UART0_PEIM | UART0_FEIM | 
		UART0_RTIM | UART0_TXIM | UART0_RXIM | UART0_CTSMIM;

	// Enable UART0, receive & transfer part of UART
	rpi_uart->cr = (1 << 0) | (1 << 8) | (1 << 9);

	// Disable buffering of stdout since it's redirected to the UART
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

#ifdef UART_USE_LOCK
	// Create the mutex
	uart_mutex = mutex_create("UART");
#endif

	TRACE("Enabled PL011 UART");
}



//
// Close the UART
//
void uart_term()
{
	// Disable UART0.
	rpi_uart->cr = 0;
	_spin(150);

	// Disable pull up/down for all GPIO pins & _spin for 150 cycles.
	rpi_gpio->GPPUD = 0;
	_spin(150);

	// Enable pull up/down for pin 14,15 & _spin for 150 cycles.
	rpi_gpio->GPPUDCLK1 = (1 << 14) | (1 << 15);
	_spin(150);

	// Write 0 to GPPUDCLK1 to make it take effect.
	rpi_gpio->GPPUDCLK1 = 0;
	_spin(150);
}



//
// Enable the RX interrupt
//
void uart_enable_rx_interrupt(irq_handler_t handler)
{
	// Clear interrupt status
	rpi_uart->icr = 0x7FF;

	// Register the handler for the uart interrupt
	register_irq_handler(57, handler);

	// Enable the RX interrupt
	rpi_uart->imsc &= ~UART0_RXIM;
}



//
// Disable the RX interrupt
//
void uart_disable_rx_interrupt()
{
	// Disable the interrupt
	rpi_uart->imsc |= UART0_RXIM;

	// Unregister the handler
	unregister_irq_handler(57);
}



//////////////////////////////////////////////////////////////////////////
//
// Non-locking implementation
//
// Not exposed in header, but used for assert/trace etc.
//
//////////////////////////////////////////////////////////////////////////



//
// Try to retrieve a character or return 0
//
uint8_t uart_trygetc_nolock(uint8_t* byte)
{
	if ((rpi_uart->fr & (1 << 4)) != 0)
		return 0;

	*byte = rpi_uart->dr;
	return 1;
}



//
// Get a character from the UART
//
uint8_t uart_getc_nolock(void)
{
	uint8_t ch;

	while (!uart_trygetc_nolock(&ch))
		;

	return ch;
}



//
// Try to write a byte
//
uint8_t uart_tryputc_nolock(uint8_t byte)
{
	if ((rpi_uart->fr  & (1 << 5)) != 0)
		return 0;

	rpi_uart->dr = byte;
	return 1;
}



//
// Write a byte
//
void uart_putc_nolock(uint8_t byte)
{
	while (!uart_tryputc_nolock(byte))
		;
}



//
// Write a non-null-terminated string to the UART
//
void uart_puts_len_nolock(const char *str, int len)
{
	while (len--)
		uart_putc_nolock(*str++);
}



//
// Write a null-terminated string to the UART
//
void uart_puts_nolock(const char *str)
{
	while (*str)
		uart_putc_nolock(*str++);
}



//////////////////////////////////////////////////////////////////////////
//
// Locking wrapper functions
//
//////////////////////////////////////////////////////////////////////////



//
// Mutex lock/unlock for UART
//
#ifdef UART_USE_LOCK
	#define UART_TRY_LOCK() if (!mutex_trylock(uart_mutex)) return 0
	#define UART_LOCK()		mutex_lock(uart_mutex)
	#define UART_UNLOCK()	mutex_unlock(uart_mutex)
#else
	#define UART_TRY_LOCK()
	#define UART_LOCK()
	#define UART_UNLOCK()
#endif



//
// Try to retrieve a character or return 0
//
uint8_t uart_trygetc(uint8_t* byte)
{
	UART_TRY_LOCK();

	uint8_t result = uart_trygetc_nolock(byte);

	UART_UNLOCK();

	return result;
}



//
// Get a character from the UART
//
uint8_t uart_getc(void)
{
	UART_LOCK();

	uint8_t ch;
	while (!uart_trygetc_nolock(&ch))
		thread_yield();

	UART_UNLOCK();

	return ch;
}



//
// Try to write a byte
//
uint8_t uart_tryputc(uint8_t byte)
{
	UART_TRY_LOCK();

	uint8_t result = uart_tryputc_nolock(byte);

	UART_UNLOCK();

	return result;
}



//
// Write a byte
//
void uart_putc(uint8_t byte) 
{
	UART_LOCK();

	while (!uart_tryputc_nolock(byte))
		thread_yield();

	UART_UNLOCK();
}



//
// Write a non-null-terminated string to the uart
//
void uart_puts_len(const char *str, int len)
{
#ifdef UART_USE_LOCK
	UART_LOCK();

	while (len--)
	{
		while (!uart_tryputc_nolock(*str))
			thread_yield();
		str++;
	}

	UART_UNLOCK();
#else
	uart_puts_len_nolock(str, len);
#endif
}



//
// Write a null-terminated string to the UART
//
void uart_puts(const char *str) 
{
#ifdef UART_USE_LOCK
	UART_LOCK();

	while (*str)
	{
		while (!uart_tryputc_nolock(*str))
			thread_yield();
		str++;
	}

	UART_UNLOCK();
#else
	uart_puts_nolock(str);
#endif
}
