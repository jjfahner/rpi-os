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
