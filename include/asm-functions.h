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
// Enable interrupts
//
//EXTERN_C void _enable_interrupts();
//
EXTERN_C void _enable_interrupts(void);



//
// Disable interrupts
//
//EXTERN_C uint32_t _disable_interrupts(uint32_t mode);
//
EXTERN_C void _disable_interrupts(void);



//
// Get current interrupts
//
EXTERN_C uint32_t _get_interrupts();



//
// Wait for interrupts
//
EXTERN_C void _wait_for_interrupt(void);



//
// Instruction memory barrier
//
EXTERN_C void _isb();



//
// Data memory barrier
//
EXTERN_C void _dmb();



//
// Spin a number of cycles
//
EXTERN_C void _spin(uint32_t cycles);



//
// Blink the LED
//
EXTERN_C void _led_blink();



//
// Get the current stack pointer
//
EXTERN_C void* _get_stack_pointer(void);



//
// Switch from one thread to another
//
EXTERN_C uint32_t _switch_to_thread(uint32_t* cur_regs, uint32_t* new_regs);
