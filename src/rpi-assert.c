#include "rpi-assert.h"
#include "rpi-led.h"
#include "rpi-thread.h"

#include <stdio.h>


//
// Use the non-locking version of puts for assertions
//
void uart_puts_nolock(const char *str);



//
// Assertion handler
//
void assert_fail_handler(const char* exp, const char* file, int line)
{
	// Blink up front, just in case the UART is broken
	led_blink(3, 500000);

	// Format assertion
	char buf[1024];
	sprintf(buf, "%s(%d): (%s) Assertion failed: '%s'\n", file, line, thread_name(thread_get_id()), exp);

	// Write to non-locking uart
	uart_puts_nolock(buf);

	// Continue error pulse
	led_error_pulse(3);
}
