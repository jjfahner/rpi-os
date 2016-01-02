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
// Current system uptime in microseconds, wraps after ~1.1 hours
//
EXTERN_C uint32_t sys_timer_uptime_usec();



//
// Current system uptime in milliseconds, wraps after ~49.7 days
//
// Note: this timer is updated with the frequency of the system timer interrupt
//
EXTERN_C uint32_t sys_timer_uptime_msec();



//
// Current system uptime in seconds, wraps after ~196.1 years
//
// Note: this timer is updated with the frequency of the system timer interrupt
//
EXTERN_C uint32_t sys_timer_uptime_sec();



//
// Wait functions
//
EXTERN_C void sys_timer_wait_usec(uint32_t microseconds);
EXTERN_C void sys_timer_wait_msec(uint32_t milliseconds);



//
// Enable the system timer
//
EXTERN_C void sys_timer_enable();



//
// System timer callback
//
typedef void(*sys_timer_proc_t)(uint32_t timer_id);



//
// Set a timer
//
// Interval is in microseconds, restricted in precision by the arm system timer.
// Count is the number of times the timer will be invoked, pass 0 for infinite.
// Callback is the function invoked when the timer elapsed. Called from the interrupt handler,
// so code must limit register use and cycle count.
//
EXTERN_C uint32_t sys_timer_install(uint32_t interval, uint32_t count, sys_timer_proc_t callback);
