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
typedef struct
{
	uint32_t			interval;
	uint32_t			count;
	uint32_t			deadline_lo;
	uint32_t			deadline_hi;
	sys_timer_proc_t	callback;
} sys_timer_t;



//
// Current uptime
//
static uint32_t sys_uptime_msec;
static uint32_t sys_uptime_sec;



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
// Current system uptime in microseconds, wraps after ~1.1 hours
//
uint32_t sys_timer_uptime_usec()
{
	return rpi_sys_timer->clo;
}



//
// Current system uptime in milliseconds, wraps to zero after ~49.7 days
//
uint32_t sys_timer_uptime_msec()
{
	return sys_uptime_msec;
}



//
// Current system uptime in seconds, wraps after ~196.1 years
//
uint32_t sys_timer_uptime_sec()
{
	return sys_uptime_sec;
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
// Set the next deadline on a timer
//
static void set_deadline(sys_timer_t* timer)
{
	// Copy previous deadline
	uint32_t cur_lo = timer->deadline_lo;
	uint32_t cur_hi = timer->deadline_hi;

	// Calculate interval
	uint32_t remaining = UINT32_MAX - cur_lo;
	if (remaining >= timer->interval)
	{
		timer->deadline_hi = cur_hi;
		timer->deadline_lo = cur_lo + timer->interval;
	}
	else
	{
		timer->deadline_lo = timer->interval - remaining;
		timer->deadline_hi = cur_hi + (timer->interval / UINT32_MAX);
	}
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
			timer->deadline_hi = rpi_sys_timer->chi;
			timer->deadline_lo = rpi_sys_timer->clo;

			set_deadline(timer);

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
	// Time must be read in this order, the bcm2836 uses a timer-read-hold copy of the high word
	uint32_t time_lo = rpi_sys_timer->clo;
	uint32_t time_hi = rpi_sys_timer->chi;

	// Check timers
	for (int i = 0; i < MAX_TIMERS; i++)
	{
		// Check whether the timer is enabled and elapsed
	if (timers[i].callback != NULL)
		{
			if (timers[i].deadline_hi <= time_hi && timers[i].deadline_lo <= time_lo)
			{
				// Calculate new deadline before processing the timer
				set_deadline(&timers[i]);

				// Invoke the callback
				timers[i].callback(i);

				// If there's a counter and the decrement goes to zero, uninstall it
				if (timers[i].count && --timers[i].count == 0)
				{
					timers[i].interval = 0;
					timers[i].count = 0;
					timers[i].deadline_hi = 0;
					timers[i].deadline_lo = 0;
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
	static uint32_t prev_time = 0;
	if (prev_time == 0)
		prev_time = rpi_sys_timer->clo;

	// Set next time as previous time plus interval
	uint32_t next_time = (prev_time & ~(sys_timer_interval - 1)) + sys_timer_interval;
	rpi_sys_timer->c1 = next_time;

	// Set previous time as current time
	prev_time = next_time;
}



//
// Update system uptime
//
static inline void update_sys_uptime()
{
	// Previous time values
	static uint32_t prv_time_usec = 0;
	static uint32_t prv_time_msec = 0;

	////////// First update uptime in milliseconds

	// Get current system time in us
	uint32_t cur_time_usec = rpi_sys_timer->clo;

	// Calculate us difference with previous update
	uint32_t diff_time_usec = prv_time_usec < cur_time_usec ?
		cur_time_usec - prv_time_usec :				// Normal case
		UINT32_MAX - prv_time_usec + cur_time_usec;	// Counter wrapped

	// Early out if less than a millisecond elapsed
	if (diff_time_usec < 1000)
		return;

	// Calculate number of whole milliseconds elapsed
	uint32_t diff_time_ms = diff_time_usec / 1000;

	// Update previous update value with number of whole milliseconds elapsed
	prv_time_usec += diff_time_ms * 1000;

	// Update uptime_ms
	sys_uptime_msec += diff_time_ms;

	////////// Then update uptime in seconds

	// Get current uptime in ms
	uint32_t cur_time_msec = sys_uptime_msec;

	// Calculate ms difference with previous update
	uint32_t diff_time_msec = prv_time_msec < sys_uptime_msec ?
		cur_time_msec - prv_time_msec :				// Normal case
		UINT32_MAX - prv_time_msec + cur_time_msec;	// Counter wrapped

	// Early out if less than a second elapsed
	if (diff_time_msec < 1000)
		return;

	// Calculate number of whole seconds elapsed
	uint32_t diff_time_sec = diff_time_msec / 1000;

	// Update previous update value with number of whole seconds elapsed
	prv_time_msec += diff_time_sec * 1000;

	// Update uptime_ms
	sys_uptime_sec += diff_time_sec;
}



//
// Interrupt called for the system timer
//
void sys_timer_interrupt()
{
	// Clear the interrupt bit
	rpi_sys_timer->cs = (1 << 1);

	// Update uptime
	update_sys_uptime();

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
