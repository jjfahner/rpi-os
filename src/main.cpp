#include "rpi-uart.h"
#include "rpi-led.h"
#include "rpi-systimer.h"
#include "rpi-mailbox-interface.h"
#include "rpi-thread.h"
#include "asm-functions.h"

#include <stdio.h>


//
// LED blink thread
//
void led_thread(uint32_t thread_arg)
{
	while (1)
	{
		led_on();
		thread_sleep_ms(100);
		led_off();
		thread_sleep_ms(100);
	}
}


static uint32_t thread_counter = 0;
static void thread_fun(uint32_t);


void create_thread()
{
	char buf[20];
	sprintf(buf, "Thread %u", thread_counter++);
	thread_create(4 * 1024, buf, thread_fun, thread_counter);
}



static void thread_fun(uint32_t thread_arg)
{
	for (int i = 0; i < 25; i++)
	{
		printf("Thread %u\n", thread_arg);
		thread_sleep_ms((thread_arg % 4) * 100);
	}

	create_thread();
}



static void uart_thread(uint32_t thread_arg)
{
	uint8_t ch;

	while (1)
	{
		while (!uart_trygetc(&ch))
			thread_yield();

		while (!uart_tryputc(ch))
			thread_yield();

		if (ch == '\r')
			while (!uart_tryputc('\n'))
				thread_yield();
	}
}


static void time_thread(uint32_t thread_arg)
{
	while (1)
	{
		TRACE("Timestamp");
		thread_sleep_ms(100);
	}
}



//
// Application entry point
//
extern "C" void rpi_main(uint32_t thread_arg)
{
	// Create a led blink timer
	thread_create(4 * 1024, "LED thread", &led_thread, 0);
	
	// Create a thread that monitors the UART for incoming data
	thread_create(4 * 1024, "UART thread", &uart_thread, 0);

	// Create a time traces
	thread_create(4 * 1024, "Time thread", &time_thread, 0);

	// Create some worker threads
	for (int i = 0; i < 5; i++)
		create_thread();

	// Suspend the main thread, the created threads will do their work
	thread_suspend();
}
