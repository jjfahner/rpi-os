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
#include "rpi-systimer.h"
#include "rpi-armtimer.h"
#include "rpi-led.h"
#include "rpi-interrupts.h"
#include "asm-functions.h"

#include <time.h>

//
// Timer struct
//
typedef struct sys_timer_t
{
	sys_time_t			interval;
	sys_time_t			deadline;
	uint32_t			count;
	sys_timer_proc_t	callback;
} sys_timer_t;



//
// Timers
//
#define MAX_TIMERS 32
static sys_timer_t timers[MAX_TIMERS];
static uint32_t num_timers = 0;



//
// System timer interval
//
static uint32_t sys_timer_interval = 0x1000;



//
// Retrieve 64-bit system clock
//
// Note: since the clock is read in two consecutive cycles, it's not safe to read
// it without ensuring they are in sync. This function addresses that concern.
//
sys_time_t sys_timer_get_time()
{
	uint32_t hi, lo;
	
	do {
		hi = rpi_sys_timer->chi;
		lo = rpi_sys_timer->clo;
	} while (hi != rpi_sys_timer->chi);

	return ((uint64_t)hi << 32) | lo;
}



//
// Wait for the system timer
//
void sys_timer_wait_usec(uint32_t microseconds)
{
    volatile uint32_t ts = rpi_sys_timer->clo;
    while((rpi_sys_timer->clo - ts) < microseconds)
		;
}



//
// Wait for the system timer
//
void sys_timer_wait_msec(uint32_t milliseconds)
{
	sys_timer_wait_usec(milliseconds * 1000);
}



//
// Install a timer
//
uint32_t sys_timer_install(uint32_t interval, uint32_t count, sys_timer_proc_t callback)
{
	// Check timer count
	if (num_timers >= MAX_TIMERS)
		led_error_pulse(2);

	// Ensure we're not interrupted
	_disable_interrupts();

	// Clear pending interrupts
	rpi_sys_timer->cs = SYS_TIMER_1;

	// Find a timer slot
	int timer_id = 0;
	for (; timer_id < MAX_TIMERS; timer_id++)
	{
		if (timers[timer_id].callback == NULL)
		{
			sys_timer_t* timer = &timers[timer_id];
			timer->interval = interval;
			timer->count = count;
			timer->callback = callback;
			timer->deadline = sys_timer_get_time() + timer->interval;
			num_timers++;
			break;
		}
	}

	// Re-enable interrupts
	_enable_interrupts();

	// Return the timer id
	return timer_id;
}



//
// Invoke registered timers
//
void invoke_timers()
{
	// If there's no timers, early out
	if (num_timers == 0)
		return;

	// Take current time for all timers
	sys_time_t time = sys_timer_get_time();

	// Check timers
	for (int i = 0; i < MAX_TIMERS; i++)
	{
		// Check whether the timer is enabled and elapsed
	if (timers[i].callback != NULL)
		{
			if (timers[i].deadline < time)
			{
				// Determine new deadline before processing the timer
				timers[i].deadline = sys_timer_get_time() + timers[i].interval;

				// Invoke the callback
				timers[i].callback(i);

				// If there's a counter and the decrement goes to zero, uninstall it
				if (timers[i].count && --timers[i].count == 0)
				{
					timers[i].interval = 0;
					timers[i].count = 0;
					timers[i].deadline = 0;
					timers[i].callback = NULL;
					--num_timers;
				}
			}
		}
	}
}



//
// Set next compare value
//
static inline void sys_timer_set_compare()
{
	// Initialize previous interrupt time with current time
	static sys_time_t prev_time = 0;
	if (prev_time == 0)
		prev_time = rpi_sys_timer->clo;

	// Set next time as previous time plus interval
	sys_time_t next_time = prev_time + sys_timer_interval;
	rpi_sys_timer->c1 = next_time;

	// Store scheduled time as previous time
	prev_time = next_time;
}



//
// Interrupt called for the system timer
//
void sys_timer_interrupt()
{
	// Clear the interrupt bit
	rpi_sys_timer->cs = (1 << 1);

	// Invoke the timers
	invoke_timers();

	// Set next compare value
	sys_timer_set_compare();
}



//
// Enable the system timer
//
void sys_timer_enable()
{
	TRACE("Enabling system timer at %u us", sys_timer_interval);

	// Disable interrupts
	_disable_interrupts();

	// Set initial compare value
	sys_timer_set_compare();

	// Register handler for timer
	register_irq_handler(1, &sys_timer_interrupt);

	// Enable interrupts
	_enable_interrupts();
}
