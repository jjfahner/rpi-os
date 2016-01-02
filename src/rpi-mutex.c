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
#include "rpi-mutex.h"
#include "rpi-thread.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>



//
// Mutex definition
//
struct mutex_t
{
	thread_id_t		owner;					// Owning thread
	uint32_t		count;					// Owning thread recursive lock count
	uint32_t		waits;					// Number of threads waiting for the mutex
	char			name[MUTEX_NAME_LEN];	// Mutex name
};



//
// Create a mutex
//
mutex_t* mutex_create(const char* name)
{
	// Create and initialize mutex
	mutex_t* mutex = (mutex_t*)malloc(sizeof(mutex_t));
	memset(mutex, 0, sizeof(mutex_t));

	// Copy thread name
	strncpy(mutex->name, name, MUTEX_NAME_LEN);
	mutex->name[MUTEX_NAME_LEN - 1] = '\x0';

	return mutex;
}



//
// Destroy a mutex
//
void mutex_destroy(mutex_t* mutex)
{
	ASSERT(mutex->owner == 0);
	ASSERT(mutex->count == 0);
	ASSERT(mutex->waits == 0);

	free(mutex);
}



//
// Try to acquire a mutex. Implementation, not declared in header. Also used by the thread scheduler.
//
uint32_t mutex_acquire_thread(mutex_t* mutex, thread_id_t thread_id)
{
	ASSERT(thread_id != THREAD_INVALID_ID);
	ASSERT(mutex->waits > 0);

	if (mutex->owner == 0)
	{
		ASSERT(mutex->count == 0);

		// The thread is now owner of the mutex
		mutex->owner = thread_id;

		// Update counters
		mutex->count++;
		mutex->waits--;

		// Mutex locked successfully
		return 1;
	}
	else if (mutex->owner == thread_id)
	{
		ASSERT(mutex->count > 0);

		// Update counters
		mutex->count++;
		mutex->waits--;

		// Mutex locked successfully
		return 1;
	}
	else
	{
		// Failed waits on the scheduler thread are a problem
		ASSERT(thread_id != THREAD_SCHEDULER_THREAD_ID);
		return 0;
	}
}



//
// Try to lock a mutex
//
uint32_t mutex_trylock(mutex_t* mutex)
{
	// Add to wait count
	mutex->waits++;

	// If the lock succeeds, the wait count will have been removed
	if (mutex_acquire_thread(mutex, thread_get_id()))
		return 1;

	// Remove from wait count
	mutex->waits--;

	// Try lock failed
	return 0;
}



//
// Lock a mutex
//
void mutex_lock(mutex_t* mutex)
{
	// Add to wait count
	mutex->waits++;

	// Try to lock the mutex directly
	if (mutex_acquire_thread(mutex, thread_get_id()))
		return;

	// Yield the thread until the mutex is free
	thread_wait_mutex(mutex);
}



//
// Unlock a mutex
//
void mutex_unlock(mutex_t* mutex)
{
	ASSERT(mutex->count > 0);
	ASSERT(mutex->owner == thread_get_id());

	// Update count
	if (--mutex->count == 0)
		mutex->owner = 0;
}
