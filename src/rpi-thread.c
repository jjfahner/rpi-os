#include "rpi-thread.h"
#include "asm-functions.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>



//
// Thread states
//
#define THREAD_STATE_STARTING		0
#define THREAD_STATE_SCHEDULED		1
#define THREAD_STATE_RUNNING		2
#define THREAD_STATE_TIMED_WAIT		3
#define THREAD_STATE_MUTEX_WAIT		4
#define THREAD_STATE_SUSPENDED		5
#define THREAD_STATE_STOPPED		6



//
// Register struct
//
typedef union
{
	struct {
		uint32_t	r0;
		uint32_t	r1;
		uint32_t	r2;
		uint32_t	r3;
		uint32_t	r4;
		uint32_t	r5;
		uint32_t	r6;
		uint32_t	r7;
		uint32_t	r8;
		uint32_t	r9;
		uint32_t	r10;
		uint32_t	r11;
		uint32_t	r12;
		uint32_t	sp;
		uint32_t	lr;
	};
	uint32_t		regs[15];
} registers_t;



//
// Thread structure
//
typedef struct
{
	// Thread info
	thread_id_t		thread_id;
	uint32_t		thread_state;
	char			thread_name[THREAD_NAME_LEN];

	// Thread function and argument
	thread_fun_t	thread_fun;
	uint32_t		thread_arg;

	// Stack
	uint32_t		stack_size;
	void*			stack_base;

	// Registers
	registers_t		registers;

	// Wait timer
	uint32_t		wait_lo;
	uint32_t		wait_hi;

	// Wait mutex
	mutex_t*		wait_mutex;
} thread_t;




//
// Thread id counter
//
static uint32_t thread_id_counter = 0;



//
// Thread pointers
//
thread_t* thread_list[THREAD_MAX_COUNT];
thread_t* current_thread;



//
// Registers of the scheduler thread
//
registers_t scheduler_regs;



//
// Local functions
//
static void thread_stub();
static void switch_to_scheduler();
static void switch_to_thread();


//
// External functions
//
uint32_t mutex_trylock_thread(mutex_t* mutex, thread_id_t thread_id);



//
// Create a thread
//
thread_id_t thread_create(uint32_t stack_size, char const* name, thread_fun_t thread_fun, uint32_t thread_arg)
{
	// Allocate and clear thread data
	thread_t* thread = (thread_t*)malloc(sizeof(thread_t));
	ASSERT(thread != NULL);
	memset(thread, 0, sizeof(thread_t));

	// Set thread id and state
	thread->thread_id = ++thread_id_counter;
	thread->thread_state = THREAD_STATE_STARTING;

	// Copy thread name
	strncpy(thread->thread_name, name, THREAD_NAME_LEN);
	thread->thread_name[THREAD_NAME_LEN - 1] = '\x0';

	// Set thread function and argument, this will be invoked from the stub
	thread->thread_fun = thread_fun;
	thread->thread_arg = thread_arg;

	// Allocate stack
	thread->stack_size = stack_size;
	thread->stack_base = malloc(thread->stack_size);
	ASSERT(thread->stack_base != NULL);

	// Set registers
	thread->registers.sp = (uint32_t)((char*)thread->stack_base + thread->stack_size);
	thread->registers.lr = (uint32_t)&thread_stub; 

	// Add the thread to the thread list
	for (int i = 0; i < THREAD_MAX_COUNT; i++)
		if (thread_list[i] == NULL)
		{
			thread_list[i] = thread;
			break;
		}

	// Now that the thread is ready to run, mark it as scheduled
	thread->thread_state = THREAD_STATE_SCHEDULED;

	// Return the id of the new thread
	return thread->thread_id;
}



//
// Exit the current thread
//
void thread_exit()
{
	// Mark the thread as exiting
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_STOPPED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Get the current thread
//
thread_id_t thread_current(void)
{
	if (current_thread != NULL)
		return current_thread->thread_id;
	return 0;
}



//
// Sleep thread
//
void thread_sleep_us(uint32_t microseconds)
{
	// Mark the thread as waiting
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_TIMED_WAIT;

	// Read system timer
	uint32_t time_lo = rpi_sys_timer->clo;
	uint32_t time_hi = rpi_sys_timer->chi;

	// Calculate time to wake up again
	uint32_t lo_remaining = UINT32_MAX - time_lo;
	if (microseconds < lo_remaining)
	{
		current_thread->wait_lo = time_lo + microseconds;
		current_thread->wait_hi = time_hi;
	}
	else
	{
		current_thread->wait_lo = microseconds - lo_remaining;
		current_thread->wait_hi = time_hi + 1;
	}

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Sleep thread
//
void thread_sleep_ms(uint32_t milliseconds)
{
	thread_sleep_us(milliseconds * 1000);
}



//
// Thread wait for mutex
//
void thread_wait_mutex(mutex_t* mutex)
{
	// Mark the thread as waiting for mutex
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_MUTEX_WAIT;

	// Store the mutex with the thread
	current_thread->wait_mutex = mutex;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Yield thread time slice
//
void thread_yield()
{
	// Cannot yield when there is no current thread
	if (current_thread == NULL)
		return;

	// Mark the current thread as scheduled
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_SCHEDULED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Suspend current thread
//
void thread_suspend()
{
	// Mark the current thread as suspended
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_SUSPENDED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//////////////////////////////////////////////////////////////////////////
//
// Implementation
//
//////////////////////////////////////////////////////////////////////////



//
// Switch to the thread scheduler
//
void switch_to_scheduler()
{
	ASSERT(current_thread != NULL);
	ASSERT(current_thread->thread_state != THREAD_STATE_RUNNING);

	// Clear current_thread
	thread_t* old_thread = current_thread;
	current_thread = NULL;

	// Switch to the scheduler thread
	_switch_to_thread(old_thread->registers.regs, scheduler_regs.regs);
}



//
// Switch to a thread
//
void switch_to_thread(thread_t* thread)
{
	ASSERT(current_thread == NULL);
	ASSERT(thread->thread_state != THREAD_STATE_RUNNING);

	// Set current thread
	current_thread = thread;
	current_thread->thread_state = THREAD_STATE_RUNNING;

	// Switch to the thread
	_switch_to_thread(scheduler_regs.regs, current_thread->registers.regs);
}



//
// This is the scheduler thread
//
void thread_scheduler()
{
	TRACE("Scheduler started");

	// Start at last index so thread[0] will run first
	uint32_t cur_idx = THREAD_MAX_COUNT - 1;
	uint32_t prv_idx = cur_idx;

	// Scheduler main loop
	while (1)
	{
		// Advance to the next slot
		cur_idx = (cur_idx + 1) & (THREAD_MAX_COUNT - 1);
		thread_t* next_thread = thread_list[cur_idx];

		// If the current thread matches the last one that ran, all threads have
		// been checked and found not eligible to run, so wait for interrupts.
		// The system timer will resume the scheduler when its interrupt occurs.
		// This effectively keeps the cpu in low power mode unless there is work.
		if (cur_idx == prv_idx)
			_wait_for_interrupt();

		// If there's no thread entry in this slot, continue
		if (next_thread == NULL)
			continue;

		// Handle thread states. Note that all states should be handled here!
		switch (next_thread->thread_state)
		{
		// Starting thread, cannot run yet
		case THREAD_STATE_STARTING:
			continue;

		// Next scheduled thread, switch to it and continue when it yields
		case THREAD_STATE_SCHEDULED:
			prv_idx = cur_idx;
			switch_to_thread(next_thread);
			continue;

		// Running thread, should not occur!
		case THREAD_STATE_RUNNING:
			ASSERT(false);
			continue;

		// Thread waiting for timespan
		case THREAD_STATE_TIMED_WAIT:
			if (!(next_thread->wait_lo <= rpi_sys_timer->clo && next_thread->wait_hi <= rpi_sys_timer->chi))
				continue;
			prv_idx = cur_idx;
			switch_to_thread(next_thread);
			continue;

		// Thread waiting for mutex
		case THREAD_STATE_MUTEX_WAIT:
			if (!mutex_trylock_thread(next_thread->wait_mutex, next_thread->thread_id))
				continue;
			prv_idx = cur_idx;
			switch_to_thread(next_thread);
			continue;

		// Suspended thread, continue scanning
		case THREAD_STATE_SUSPENDED:
			continue;

		// Stopped thread, clear thread data and continue
		case THREAD_STATE_STOPPED:
			free(next_thread->stack_base);
			free(next_thread);
			thread_list[cur_idx] = NULL;
			continue;
		}

		// Error, should never get here
		ASSERT(false);
		printf("A thread state handler is missing!\n");
		break;
	}
}



//
// Thread stub, runs the thread function and cleans up after the thread
//
void thread_stub()
{
	// Run the thread function
	current_thread->thread_fun(current_thread->thread_arg);

	// Mark the thread as stopped
	current_thread->thread_state = THREAD_STATE_STOPPED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}
