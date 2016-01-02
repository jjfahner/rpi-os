#include "rpi-systimer.h"
#include "rpi-armtimer.h"
#include "rpi-led.h"
#include "rpi-interrupts.h"
#include "asm-functions.h"



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
// Wait for the system timer
//
void sys_timer_wait_us( uint32_t us )
{
    volatile uint32_t ts = rpi_sys_timer->clo;
    while( ( rpi_sys_timer->clo - ts ) < us )
		;
}



//
// Wait for the system timer
//
void sys_timer_wait_ms(uint32_t ms)
{
	sys_timer_wait_us(ms * 1000);
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
	rpi_sys_timer->c1 = (rpi_sys_timer->clo & ~(sys_timer_interval - 1)) + sys_timer_interval;
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
