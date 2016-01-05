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
#include "rpi-thread.h"
#include "rpi-systimer.h"
#include "rpi-uart.h"
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
#define THREAD_STATE_EVENT_WAIT		4
#define THREAD_STATE_MUTEX_WAIT		5
#define THREAD_STATE_SUSPENDED		6
#define THREAD_STATE_STOPPED		7



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

	// Scheduled time
	sys_time_t		sched_time;

	// Wait objects
	union {
		uint32_t	wait_object;
		event_t*	wait_event;
		mutex_t*	wait_mutex;
	};

	// Performance data
	uint32_t		run_count;
	uint32_t		run_cycles;
	
} thread_t;



//
// Setup the scheduler thread data
//
thread_t scheduler_thread = { THREAD_SCHEDULER_THREAD_ID, THREAD_STATE_RUNNING, "scheduler", NULL, 0, 0x4000, (void*)0x8000 };



//
// Thread id counter
//
static uint32_t thread_id_counter = THREAD_SCHEDULER_THREAD_ID + 1;



//
// Thread list
//
static thread_t* thread_list[THREAD_MAX_COUNT];



//
// Current thread
//
static thread_t* current_thread = &scheduler_thread;



//
// Tick counts
//
static sys_time_t perf_idle_ticks;
static sys_time_t perf_exec_ticks;



//
// Local functions
//
static uint32_t switch_to_scheduler();
static void switch_to_thread(thread_t* thread);
static void thread_stub();



//
// External functions
//
uint32_t event_acquire_scheduler(event_t* event, thread_id_t thread_id);
uint32_t mutex_acquire_scheduler(mutex_t* mutex, thread_id_t thread_id);


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
	thread->thread_id = thread_id_counter++;
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

	// Insert the thread in the thread list
	int insert_pos = 0;
	for (; insert_pos < THREAD_MAX_COUNT; insert_pos++)
	{
		if (thread_list[insert_pos] != NULL && thread_list[insert_pos]->thread_state == THREAD_STATE_STOPPED)
		{
			free(thread_list[insert_pos]->stack_base);
			free(thread_list[insert_pos]);
			thread_list[insert_pos] = NULL;
		}

		if (thread_list[insert_pos] == NULL)
		{
			thread_list[insert_pos] = thread;
			break;
		}
	}
	ASSERT(insert_pos < THREAD_MAX_COUNT);

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
	ASSERT(thread_get_id() != THREAD_SCHEDULER_THREAD_ID);

	// Mark the thread as exiting
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_STOPPED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Get the current thread id
//
thread_id_t thread_get_id(void)
{
	return current_thread->thread_id;
}



//
// Get the current thread name
//
const char* thread_name(thread_id_t thread_id)
{
	// Test current thread
	if (thread_id == current_thread->thread_id)
		return current_thread->thread_name;

	// Test scheduler thread (not in thread array)
	if (thread_id == scheduler_thread.thread_id)
		return scheduler_thread.thread_name;

	// Search for thread
	for (int i = 0; i < THREAD_MAX_COUNT; i++)
		if (thread_list[i]->thread_id == thread_id)
			return thread_list[i]->thread_name;

	// No thread found
	return "Invalid thread id";
}



//
// Sleep thread
//
void thread_sleep_usec(uint32_t microseconds)
{
	// The scheduler thread cannot yield
	// It can spinwait, but I'm not sure that's a good idea...
	if (thread_get_id() == THREAD_SCHEDULER_THREAD_ID)
		return;

	// Mark the thread as waiting
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_TIMED_WAIT;

	// Set scheduled time
	current_thread->sched_time = sys_timer_get_time() + microseconds;

	// Yield to the scheduler thread
	switch_to_scheduler();
}



//
// Sleep thread
//
void thread_sleep_msec(uint32_t milliseconds)
{
	thread_sleep_usec(milliseconds * 1000);
}



//
// Wait for an event
//
uint32_t thread_wait_event(event_t* event, sys_time_t timeout)
{
	ASSERT(thread_get_id() != THREAD_SCHEDULER_THREAD_ID);

	// Early out when the timeout is zero
	if (timeout == 0)
		return 0;

	// Mark the thread as waiting for event
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_EVENT_WAIT;

	// Store the mutex with the thread
	current_thread->wait_event = event;

	// Set the timeout
	if (timeout == TIMEOUT_INFINITE)
		current_thread->sched_time = TIMEOUT_INFINITE;
	else
		current_thread->sched_time = sys_timer_get_time() + timeout;

	// Yield to the scheduler thread
	return switch_to_scheduler();
}



//
// Thread wait for mutex
//
uint32_t thread_wait_mutex(mutex_t* mutex, sys_time_t timeout)
{
	ASSERT(thread_get_id() != THREAD_SCHEDULER_THREAD_ID);

	// Early out when the timeout is zero
	if (timeout == 0)
		return 0;

	// Mark the thread as waiting for mutex
	ASSERT(current_thread->thread_state == THREAD_STATE_RUNNING);
	current_thread->thread_state = THREAD_STATE_MUTEX_WAIT;

	// Store the mutex with the thread
	current_thread->wait_mutex = mutex;

	// Set the timeout
	if (timeout == TIMEOUT_INFINITE)
		current_thread->sched_time = TIMEOUT_INFINITE;
	else
		current_thread->sched_time = sys_timer_get_time() + timeout;

	// Yield to the scheduler thread
	return switch_to_scheduler();
}



//
// Yield thread time slice
//
void thread_yield()
{
	// The scheduler thread cannot yield
	if (thread_get_id() == THREAD_SCHEDULER_THREAD_ID)
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
	ASSERT(thread_get_id() != THREAD_SCHEDULER_THREAD_ID);

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
uint32_t switch_to_scheduler()
{
	ASSERT(current_thread != NULL);
	ASSERT(current_thread != &scheduler_thread);
	ASSERT(current_thread->thread_state != THREAD_STATE_RUNNING);

	// Set scheduler thread as current_thread
	thread_t* old_thread = current_thread;
	current_thread = &scheduler_thread;

	// Switch to the scheduler thread
	return _switch_to_thread(old_thread->registers.regs, scheduler_thread.registers.regs);
}



//
// Switch to a thread
//
void switch_to_thread(thread_t* thread)
{
	ASSERT(current_thread == &scheduler_thread);
	ASSERT(thread->thread_state != THREAD_STATE_RUNNING);

	// Set current thread
	current_thread = thread;
	current_thread->thread_state = THREAD_STATE_RUNNING;

	// Switch to the thread
	_switch_to_thread(scheduler_thread.registers.regs, current_thread->registers.regs);
}



//
// Dump the thread list
//
void thread_print_list()
{
	static char* buf = NULL;
	if (buf == NULL)
		buf = (char*)malloc(0x8000);

	sys_time_t time = sys_timer_get_time(&time);
	uint32_t tsec = time / 1000;
	uint32_t msec = tsec % 1000;
	tsec /= 1000;
	uint32_t sec = tsec % 60;
	tsec /= 60;
	uint32_t min = tsec % 60;
	tsec /= 60;
	uint32_t hrs = tsec % 24;
	tsec /= 24;
	uint32_t day = tsec;

	// Calculate performance
	static sys_time_t prev_exec_ticks = 0;
	static sys_time_t prev_idle_ticks = 0;
	sys_time_t cur_exec_ticks = perf_exec_ticks - prev_exec_ticks;
	sys_time_t cur_idle_ticks = perf_idle_ticks - prev_idle_ticks;
	prev_exec_ticks = perf_exec_ticks;
	prev_idle_ticks = perf_idle_ticks;	
	float busy = (float)cur_exec_ticks / (float)cur_idle_ticks * 100.0f;

	// Start at buffer start
	char* buf_ptr = buf;

	// Write time and performance
	buf_ptr += sprintf(buf_ptr, "%02u:%02u:%02u:%02u:%03u   Busy: %.2f                         \n", 
		day, hrs, min, sec, msec, busy);

	// Write header
	buf_ptr += sprintf(buf_ptr, "  Slot        ID    Name              Runcount        Time      State              Time    WaitObject\n");

	// Write threads
	for (int i = 0; i < THREAD_MAX_COUNT; i++)
	{
		// Get thread, skip empty slots
		thread_t* thread = thread_list[i];
		if (thread == NULL)
			continue;

		// Copy thread state
		uint32_t thread_state = thread->thread_state;

		// Calculate time remaining
		sys_time_t sched_time = thread->sched_time;
		if (sched_time <= time)
		{
			// No more time remaining, mark scheduled
			sched_time = 0;
			thread_state = THREAD_STATE_SCHEDULED;
		}
		else
		{
			// Still waiting, subtract current time
			sched_time = sched_time - time;
		}

		// Build state-specific info string
		char state_string[100];
		switch (thread_state)
		{
		case THREAD_STATE_STARTING:		sprintf(state_string, "Starting ");	break;
		case THREAD_STATE_SCHEDULED:	sprintf(state_string, "Scheduled"); break;
		case THREAD_STATE_RUNNING:		sprintf(state_string, "Running  ");	break;
		case THREAD_STATE_TIMED_WAIT:	sprintf(state_string, "TimedWait    %10u", (uint32_t)sched_time); break;
		case THREAD_STATE_EVENT_WAIT:	sprintf(state_string, "EventWait    %10u    %s", (uint32_t)sched_time, event_get_name(thread->wait_event)); break;
		case THREAD_STATE_MUTEX_WAIT:	sprintf(state_string, "MutexWait    %10u    %s", (uint32_t)sched_time, mutex_get_name(thread->wait_mutex)); break;
		case THREAD_STATE_SUSPENDED:	sprintf(state_string, "Suspended "); break;
		case THREAD_STATE_STOPPED:		sprintf(state_string, "Stopped   "); break;
		default:						sprintf(state_string, "Unknown   "); break;
		}

		// Write thread info string to buffer
		buf_ptr += sprintf(buf_ptr, "%6u    %6u    %-12.12s    %10u    %10u    %s", 
			i, thread->thread_id, thread->thread_name, thread->run_count, thread->run_cycles/1000, state_string);
		
		// Pad buffer
		while ((buf_ptr - buf) % 120 != 0)
			*buf_ptr++ = ' ';

		// Add line feed and terminate
		*buf_ptr++ = '\n';
		*buf_ptr = 0;
	}

	// Add a few lines for good measure
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 120; j++)
			*buf_ptr++ = ' ';
		*buf_ptr++ = '\n';
	}
	*buf_ptr = 0;


	// Reposition cursor and write buffer
	uart_putc('\x05');
	uart_puts(buf);
}



//
// This is the scheduler thread
//
void thread_scheduler()
{
	// Make sure the scheduler is not called by any thread other than the initial thread
	ASSERT(thread_get_id() == THREAD_SCHEDULER_THREAD_ID);
	
	TRACE("Scheduler started");

	// Start at last index so thread[0] will run first
	uint32_t cur_idx = THREAD_MAX_COUNT - 1;
	uint32_t prv_idx = cur_idx;

	// Scheduler main loop
	while (1)
	{
		// Calculate next slot, fold to zero at THREAD_MAX_COUNT
		cur_idx = ((cur_idx + 1) & (THREAD_MAX_COUNT - 1));

		// Get the thread in the slot, which may be NULL
		thread_t* thread = thread_list[cur_idx];
		if (thread == NULL)
			continue;

		// The scheduler thread should never appear in the thread list
		ASSERT(thread != &scheduler_thread);

		// If the current thread matches the last one that ran, all threads have
		// been checked and found not eligible to run, so wait for interrupts.
		// The system timer will resume the scheduler when its interrupt occurs.
		// This effectively keeps the CPU in low power mode unless there is work.
		if (cur_idx == prv_idx)
		{
			sys_time_t before = sys_timer_get_time();

			_wait_for_interrupt();

			sys_time_t after = sys_timer_get_time();
			perf_idle_ticks += (after - before);
		}

		// Get the current time
		sys_time_t time = sys_timer_get_time();

		// Handle thread states. Note that all states should be handled here!
		switch (thread->thread_state)
		{
		// Starting thread, cannot run yet
		case THREAD_STATE_STARTING:
			continue;

		// Next scheduled thread, run it
		case THREAD_STATE_SCHEDULED:
			break;

		// Running thread, should not occur!
		case THREAD_STATE_RUNNING:
			ASSERT(false);
			continue;

		// Thread waiting for timespan
		case THREAD_STATE_TIMED_WAIT:
			if (thread->sched_time <= time)
			{
				thread->registers.r0 = 1;
				break;
			}
			continue;

		// Thread waiting for event
		case THREAD_STATE_EVENT_WAIT:
			if (event_acquire_scheduler(thread->wait_event, thread->thread_id))
			{
				thread->registers.r0 = 1;
				break;
			}
			if (thread->sched_time <= time)
			{
				thread->registers.r0 = 0;
				break;
			}
			continue;

		// Thread waiting for mutex
		case THREAD_STATE_MUTEX_WAIT:
			if (mutex_acquire_scheduler(thread->wait_mutex, thread->thread_id))
			{
				thread->registers.r0 = 1;
				break;
			}
			if (thread->sched_time <= time)
			{
				thread->registers.r0 = 0;
				break;
			}
			continue;


		// Suspended thread, continue scanning
		case THREAD_STATE_SUSPENDED:
			continue;


		// Stopped thread, clear thread data and continue
		case THREAD_STATE_STOPPED:
			free(thread->stack_base);
			free(thread);
			thread_list[cur_idx] = NULL;
			continue;
		}

		// Mark this slot as the one to scan from. When the scan returns to this point
		// without finding an eligible thread, the list has been scanned completely.
		prv_idx = cur_idx;

		// Clear the wait object that the thread was waiting for
		thread->wait_object = 0;

		// Take start time
		sys_time_t before = sys_timer_get_time();

		// Switch to the thread that was found
		switch_to_thread(thread);

		// Take time elapsed
		sys_time_t after = sys_timer_get_time();
		sys_time_t elapsed = after - before;

		// Update thread performance data
		thread->run_count++;
		thread->run_cycles += elapsed;

		// Update scheduler performance data
		perf_exec_ticks += elapsed;
	}
}



//
// Thread stub, runs the thread function and cleans up after the thread
//
void thread_stub()
{
	ASSERT(thread_get_id() != THREAD_SCHEDULER_THREAD_ID);

	// Run the thread function
	current_thread->thread_fun(current_thread->thread_arg);

	// Mark the thread as stopped
	current_thread->thread_state = THREAD_STATE_STOPPED;

	// Yield to the scheduler thread
	switch_to_scheduler();
}
