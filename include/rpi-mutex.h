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
#include "rpi-systimer.h"



//
// Maximum mutex name length
//
#define MUTEX_NAME_LEN	32



//
// Mutex type
//
typedef struct mutex_t mutex_t;



//
// Create a mutex
//
EXTERN_C mutex_t* mutex_create(const char* name);



//
// Destroy a mutex
//
EXTERN_C void mutex_destroy(mutex_t* mutex);



//
// Get mutex name
//
EXTERN_C const char* mutex_get_name(mutex_t* mutex);



//
// Lock a mutex
//
EXTERN_C uint32_t mutex_lock(mutex_t* mutex, sys_time_t timeout);



//
// Unlock a mutex
//
EXTERN_C void mutex_unlock(mutex_t* mutex);
