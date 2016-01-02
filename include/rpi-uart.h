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
#include "rpi-interrupts.h"


//
// Init functions
//
EXTERN_C void uart_enable();
EXTERN_C void uart_term();
EXTERN_C void uart_enable_rx_interrupt(irq_handler_t handler);
EXTERN_C void uart_disable_rx_interrupt();



//
// Input/output functions
//
EXTERN_C uint8_t uart_trygetc(uint8_t* byte);
EXTERN_C uint8_t uart_getc(void);

EXTERN_C uint8_t uart_tryputc(uint8_t byte);
EXTERN_C void uart_putc(uint8_t byte);

EXTERN_C void uart_puts_len(const char *str, int len);
EXTERN_C void uart_puts(const char *str);
