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
#include "rpi-assert.h"
#include "rpi-led.h"
#include "rpi-thread.h"

#include <stdio.h>


//
// Use the non-locking version of puts for assertions
//
void uart_puts_nolock(const char *str);



//
// Assertion handler
//
void assert_fail_handler(const char* exp, const char* file, int line)
{
	// Blink up front, just in case the UART is broken
	led_blink(3, 500000);

	// Format assertion
	char buf[1024];
	sprintf(buf, "%s(%d): (%s) Assertion failed: '%s'\n", file, line, thread_name(thread_get_id()), exp);

	// Write to non-locking uart
	uart_puts_nolock(buf);

	// Continue error pulse
	led_error_pulse(3);
}
