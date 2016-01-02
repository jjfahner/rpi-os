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
#include "rpi-trace.h"
#include "rpi-systimer.h"


#include <stdio.h>
#include <stdarg.h>


//
// Use the non-locking version of puts for traces
//
void uart_puts_nolock(const char *str);



//
// Trace implementation
//
void trace(const char* fmt, ...)
{
	// Get system time
	uint32_t uptime_ms = sys_timer_uptime_msec();

	// Calculate time parts
	uint32_t time_ms = uptime_ms % 1000;
	uptime_ms /= 1000;
	uint32_t time_se = uptime_ms % 60;
	uptime_ms /= 60;
	uint32_t time_mi = uptime_ms % 60;
	uptime_ms /= 60;
	uint32_t time_hr = uptime_ms % 24;

	// Write format string into buffer
	// TODO add time string
	char format_buf[1024];
	sprintf(format_buf, "%02u:%02u:%02u:%03u: %s\n", time_hr, time_mi, time_se, time_ms, fmt);

	// Format arguments into buffer
	char trace_buf[2048];
	va_list args;
	va_start(args, fmt);
	vsprintf(trace_buf, format_buf, args);
	va_end(args);

	// Write buffer to the UART
	uart_puts_nolock(trace_buf);
}
