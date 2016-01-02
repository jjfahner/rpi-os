#pragma once

#include "rpi-base.h"



//
// Event type
//
typedef struct event_t event_t;



//
// Event object name
//
#define EVENT_NAME_LEN		32



//
// Event types
//
typedef enum event_type_t
{
	EVENT_TYPE_AUTO		= 0,		// Event is reset whenever a thread successfully waits on the event
	EVENT_TYPE_MANUAL	= 1,		// Event is reset by calling event_reset only
} event_type_t;



//
// Create an event object
//
EXTERN_C event_t* event_create(const char* name, event_type_t type);



//
// Destroy an event object
//
EXTERN_C void event_destroy(event_t* event);



//
// Signal an event
//
EXTERN_C void event_signal(event_t* event);



//
// Reset an event
//
EXTERN_C void event_reset(event_t* event);



//
// Is the event signaled?
//
EXTERN_C uint32_t event_is_signaled(event_t* event);



//
// Wait for an event to be signaled
//
EXTERN_C void event_wait(event_t* event);
