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
#include "rpi-uart.h"
#include "rpi-led.h"
#include "rpi-mutex.h"
#include "rpi-systimer.h"
#include "rpi-mailbox-interface.h"
#include "rpi-thread.h"
#include "asm-functions.h"

#include <stdio.h>



//////////////////////////////////////////////////////////////////////////


//
// A thread that toggles the led on/off every 100 msecs
//
void led_thread(uint32_t thread_arg)
{
	while (1)
	{
		led_on();
		thread_sleep_msec(100);
		led_off();
		thread_sleep_msec(100);
	}
}



//////////////////////////////////////////////////////////////////////////



//
// A thread that monitors the uart for input, then echos it back
//
static void uart_thread(uint32_t thread_arg)
{
	uint8_t ch;

	while (1)
	{
		while (!uart_trygetc(&ch))
			thread_sleep_msec(1);

		while (!uart_tryputc(ch))
			thread_yield();

		if (ch == '\r')
			while (!uart_tryputc('\n'))
				thread_yield();
	}
}



//////////////////////////////////////////////////////////////////////////



//
// A timer thread, prints the thread list every second
//
static void time_thread(uint32_t thread_arg)
{
	uint32_t time_us = rpi_sys_timer->clo;
	while (1)
	{
		thread_print_list();
		thread_sleep_usec(1000000 - (rpi_sys_timer->clo - time_us));
		time_us += 1000000;
	}
}



//////////////////////////////////////////////////////////////////////////



static uint32_t thread_counter = 0;
static void worker_thread(uint32_t);

static mutex_t* test_mutex;

void create_worker()
{
	char buf[20];
	sprintf(buf, "Worker %u", thread_counter);
	thread_create(4 * 1024, buf, worker_thread, thread_counter++);
}



static void worker_thread(uint32_t thread_arg)
{
	uint32_t locked = mutex_lock(test_mutex, 10000000);

	int result = 0;
	
	for (int l = 0; l < 2500; l++)
	{
		for (int i = 1; i < 2500000; i++)
		{
			result += i;
			result /= i;
			result %= 1;
		}
		thread_sleep_usec(750);
	}

	if (locked)
		mutex_unlock(test_mutex);

	create_worker();
}



//////////////////////////////////////////////////////////////////////////



static event_t* test_event;


static void producer_thread(uint32_t thread_arg)
{
	while (1)
	{
		for (int i = 0; i < 6; i++)
		{
			event_signal(test_event);
			thread_sleep_msec(i * 1000);
		}
	}
}



static void consumer_thread(uint32_t thread_arg)
{
	while (1)
	{
		event_wait(test_event, TIMEOUT_INFINITE);
		thread_sleep_msec(5000);
	}
}


//////////////////////////////////////////////////////////////////////////


//
// Application entry point
//
extern "C" void rpi_main(uint32_t thread_arg)
{
	test_mutex = mutex_create("test_mutex");

	// Create a led blink timer
	thread_create(4 * 1024, "LED thread", &led_thread, 0);
	
	// Create a thread that monitors the UART for incoming data
	thread_create(4 * 1024, "UART thread", &uart_thread, 0);

	// Create a time trace thread
	thread_create(4 * 1024, "Time thread", &time_thread, 0);

	// Create an event, a producer and some consumer threads
	test_event = event_create("test_event", EVENT_TYPE_AUTO);
	thread_create(4 * 1024, "Producer", &producer_thread, 0);
	for (int i = 0; i < 5; i++)
	{
		thread_create(4 * 1024, "Consumer", &consumer_thread, i);
		thread_sleep_usec(5000);
	}

	// Create some worker threads
	for (int i = 0; i < 15; i++)
	{
		create_worker();
		thread_sleep_usec(5000);
	}

	// Suspend the main thread
	thread_suspend();

	// Main loop
	while (1)
		;
}
