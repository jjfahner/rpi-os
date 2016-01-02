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
#include "rpi-armtimer.h"
#include "rpi-gpio.h"
#include "rpi-interrupts.h"
#include "rpi-uart.h"
#include "asm-functions.h"


//
// Enable the ARM timer
//
void arm_timer_enable(uint32_t interval)
{
	// Disable interrupts
	_disable_interrupts();

	// Clear all pending interrupts
	rpi_arm_timer->irq_clear = 1;

	// Set timer interval
	rpi_arm_timer->load = 0x400;

	// Setup control flags and enable
	rpi_arm_timer->control =
		RPI_ARMTIMER_CTRL_32BIT |
		RPI_ARMTIMER_CTRL_ENABLE |
		RPI_ARMTIMER_CTRL_INT_ENABLE |
		RPI_ARMTIMER_CTRL_PRESCALE_1;

	// Enable previous interrupt mode
	_enable_interrupts();
}



//
// Disable the ARM timer
//
void arm_timer_disable(void)
{
	// Disable interrupts
	_disable_interrupts();

	// Clear all pending interrupts
	rpi_arm_timer->irq_clear = 1;

	// Clear control flags to disable timer
	rpi_arm_timer->control = 0;

	// Clear timer interval
	rpi_arm_timer->load = 0;

	// Disable the timer interrupt
	rpi_irq_controller->disable_basic_irqs = RPI_BASIC_ARM_TIMER_IRQ;

	// Enable previous interrupt mode
	_enable_interrupts();
}



//
// Invoked from the IRQ handler when the arm timer elapses
//
void arm_timer_interrupt(void)
{
	// Clear the pending interrupt flag
	rpi_arm_timer->irq_clear = 1;
}
