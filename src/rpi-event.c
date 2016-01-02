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
#include "rpi-event.h"
#include "rpi-thread.h"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>



//
// Event structure
//
struct event_t
{
	event_type_t		type;					// Event type
	char				name[EVENT_NAME_LEN];	// Event name
	uint32_t			count;					// Event signal count
	uint32_t			waits;					// Number of waiting threads
};



//
// Create an event object
//
event_t* event_create(const char* name, event_type_t type)
{
	// Create and initialize event
	event_t* event = (event_t*)malloc(sizeof(event_t));

	// Set event properties
	event->type = type;
	event->count = 0;
	event->waits = 0;

	// Copy event name
	strncpy(event->name, name, EVENT_NAME_LEN);
	event->name[EVENT_NAME_LEN - 1] = '\x0';

	return event;
}



//
// Destroy an event object
//
void event_destroy(event_t* event)
{
	ASSERT(event->waits == 0);
	free(event);
}



//
// Signal an event
//
void event_signal(event_t* event)
{
	ASSERT(event->count < UINT32_MAX);
	event->count++;
}



//
// Reset an event
//
// TODO should this always reset to zero, even for AUTO?
//
void event_reset(event_t* event)
{
	if (event->type == EVENT_TYPE_AUTO)
	{
		ASSERT(event->count != 0);
		event->count--;
	}
	else
	{
		event->count = 0;
	}
}



//
// Is the event signaled?
//
uint32_t event_is_signaled(event_t* event)
{
	return event->count != 0;
}



//
// Wait for an event to be signaled
//
void event_wait(event_t* event)
{
	if (event->count == 0)
	{
		// The event is not signaled, yield thread
		event->waits++;
		thread_wait_event(event);
	}
	else
	{
		if (event->type == EVENT_TYPE_AUTO)
		{
			// Decrement the counter
			ASSERT(event->count != 0);
			event->count--;
		}
	}
}



//
// Used by thread scheduler: check whether an event is signaled
// If the event is signaled and auto reset, reset the event here
//
uint32_t event_acquire_scheduler(event_t* event, thread_id_t thread_id)
{
	// This should only ever be called by the scheduler
	ASSERT(thread_get_id() == THREAD_SCHEDULER_THREAD_ID);

	// If the thread isn't signaled, fail
	if (event->count == 0)
		return 0;

	// If the event is auto reset, update the count
	if (event->type == EVENT_TYPE_AUTO)
	{
		ASSERT(event->count != 0);
		event->count--;
	}
	else
	{
		TRACE("Event type: %u, name: %s, count: %u, waits: %u", event->type, event->name, event->count, event->waits);
		ASSERT(false);
	}

	// Event acquired
	event->waits--;
	return 1;
}
