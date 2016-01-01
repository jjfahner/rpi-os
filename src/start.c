#include "rpi-uart.h"
#include "rpi-led.h"
#include "rpi-systimer.h"
#include "rpi-interrupts.h"
#include "rpi-thread.h"

#include <stdio.h>



//
// Declare the preinit/init/fini arrays, defined by the linker
//
extern void(*__preinit_array_start[]) (void) __attribute__((weak));
extern void(*__preinit_array_end[]) (void) __attribute__((weak));
extern void(*__init_array_start[]) (void) __attribute__((weak));
extern void(*__init_array_end[]) (void) __attribute__((weak));
extern void(*__fini_array_start[]) (void) __attribute__((weak));
extern void(*__fini_array_end[]) (void) __attribute__((weak));



//
// Execute global constructor lists
//
static void call_init()
{
	// Execute __preinit
	int preinit_len = __preinit_array_end - __preinit_array_start;
	for (int i = 0; i < preinit_len; i++)
		if (__preinit_array_start[i])
			__preinit_array_start[i]();

	// Execute __init
	int init_len = __init_array_end - __init_array_start;
	for (int i = 0; i < init_len; i++)
		if (__init_array_start[i])
			__init_array_start[i]();
}



//
// Declare extern main function
//
extern void rpi_main(uint32_t);



//
// Implementation of kernel startup
//
void _cmain(unsigned int r0, unsigned int r1, unsigned int r2)
{
	// Execute __preinit and __init
	call_init();

	// Initialize the UART to allow tracing
	uart_enable();

	// Initialize on-board led to allow led error signaling
	led_enable();

	// Enable interrupts so exceptions can be caught and signaled
	enable_interrupts();

	// Enable the ARM system timer interrupt
	sys_timer_enable();

	// Create the thread that invokes main
	thread_create(0x10000, "main_thread", &rpi_main, 0);

	// Run the thread scheduler. This call will not return.
	thread_scheduler();
}
