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
#include "rpi-interrupts.h"
#include "rpi-armtimer.h"
#include "rpi-led.h"
#include "rpi-systimer.h"
#include "asm-functions.h"



//
// INTERRUPT attribute
//
#define INTERRUPT(type) __attribute__((interrupt(#type)))



//
// Registered IRQ handlers
//
static irq_handler_t irq_handlers[64];



//
// Enable IRQs
//
void enable_interrupts(void)
{
	TRACE("Enabling interrupts");

	// Enable irq flags
	rpi_irq_controller->enable_basic_irqs = RPI_BASIC_ARM_TIMER_IRQ | RPI_BASIC_ARM_IRQ_1 | RPI_BASIC_ARM_IRQ_2;

	// Enable interrupt signaling
	_enable_interrupts();
}



//
// Disable IRQs
//
void disable_interrupts(void)
{
	// Disable interrupt signalling
	_disable_interrupts();
}



//
// Register (and enable) an interrupt handler
//
void register_irq_handler(uint8_t irq, irq_handler_t handler)
{
	// Disable interrupts
	_disable_interrupts();

	// Check that the irq doesn't have a registered handler yet
	if (irq_handlers[irq] != NULL)
		led_error_pulse(2);

	// Register the handler
	irq_handlers[irq] = handler;

	// Enable the interrupt
	if (irq < 32)
		rpi_irq_controller->enable_irqs_1 = (1 << irq);
	else if (irq < 64)
		rpi_irq_controller->enable_irqs_2 = (1 << (irq - 32));
	else
		led_error_pulse(3);

	// Re-enable interrupts
	_enable_interrupts();
}



//
// Unregister (and disable) an interrupt handler
//
void unregister_irq_handler(uint8_t irq)
{
	// Disable interrupts
	//_disable_interrupts();

	// Check that the irq has a handler
	if (irq_handlers[irq] == NULL)
		led_error_pulse(2);

	// Disable the interrupt
	if (irq < 32)
		rpi_irq_controller->disable_irqs_1 = (1 << irq);
	else if (irq < 64)
		rpi_irq_controller->disable_irqs_2 = (1 << (irq - 32));
	else
		led_error_pulse(3);

	// Remove the handler
	irq_handlers[irq] = NULL;

	// Re-enable interrupts
	//_enable_interrupts();
}



/**
    @brief The Reset vector interrupt handler

    This can never be called, since an ARM core reset would also reset the
    GPU and therefore cause the GPU to start running code again until
    the ARM is handed control at the end of boot loading
*/
void INTERRUPT(ABORT) reset_vector(void)
{
    while( 1 )
    {
		led_error_pulse(3);
    }
}



/**
    @brief The undefined instruction interrupt handler

    If an undefined instruction is encountered, the CPU will start
    executing this function. Just trap here as a debug solution.
*/
void INTERRUPT(UNDEF) undefined_instruction_vector(void)
{
    while( 1 )
    {
		led_error_pulse(4);
	}
}



/**
    @brief The supervisor call interrupt handler

    The CPU will start executing this function. Just trap here as a debug
    solution.
*/
void INTERRUPT(SWI) software_interrupt_vector(void)
{
	led_pulse(100);
	//led_error_pulse(5);
}



/**
    @brief The prefetch abort interrupt handler

    The CPU will start executing this function. Just trap here as a debug
    solution.
*/
void INTERRUPT(ABORT) prefetch_abort_vector(void)
{
	led_error_pulse(6);
}



/**
    @brief The Data Abort interrupt handler

    The CPU will start executing this function. Just trap here as a debug
    solution.
*/
void INTERRUPT(ABORT) data_abort_vector(void)
{
	led_error_pulse(7);
}



/**
    @brief The IRQ Interrupt handler

    This handler is run every time an interrupt source is triggered. It's
    up to the handler to determine the source of the interrupt and most
    importantly clear the interrupt flag so that the interrupt won't
    immediately put us back into the start of the handler again.
*/
void INTERRUPT(IRQ) interrupt_vector(void)
{
	while (rpi_irq_controller->irq_basic_pending | rpi_irq_controller->irq_pending_1 | rpi_irq_controller->irq_pending_2)
	{
		// Timer interrupt
		if (rpi_irq_controller->irq_basic_pending & (1 << 0))
		{
			rpi_irq_controller->irq_basic_pending = (1 << 0);
			arm_timer_interrupt();
		}

		// Pending IRQ 0-31
		if (rpi_irq_controller->irq_pending_1)
			for (int i = 0; i < 32; i++)
				if (rpi_irq_controller->irq_pending_1 & (1 << i))
				{
					rpi_irq_controller->irq_pending_1 = (1 << i);
					irq_handlers[i]();
				}

		// Pending IRQ 32-63
		if (rpi_irq_controller->irq_pending_2)
			for (int i = 0; i < 32; i++)
				if (rpi_irq_controller->irq_pending_2 & (1 << i))
				{
					rpi_irq_controller->irq_pending_2 = (1 << i);
					irq_handlers[i + 32]();
				}
	}
}



/**
    @brief The FIQ Interrupt Handler

    The FIQ handler can only be allocated to one interrupt source. The FIQ has
    a full CPU shadow register set. Upon entry to this function the CPU
    switches to the shadow register set so that there is no need to save
    registers before using them in the interrupt.

    In C you can't see the difference between the IRQ and the FIQ interrupt
    handlers except for the FIQ knowing it's source of interrupt as there can
    only be one source, but the prologue and epilogue code is quite different.
    It's much faster on the FIQ interrupt handler.

    The prologue is the code that the compiler inserts at the start of the
    function, if you like, think of the opening curly brace of the function as
    being the prologue code. For the FIQ interrupt handler this is nearly
    empty because the CPU has switched to a fresh set of registers, there's
    nothing we need to save.

    The epilogue is the code that the compiler inserts at the end of the
    function, if you like, think of the closing curly brace of the function as
    being the epilogue code. For the FIQ interrupt handler this is nearly
    empty because the CPU has switched to a fresh set of registers and so has
    not altered the main set of registers.
*/
void INTERRUPT(FIQ) fast_interrupt_vector(void)
{
	return;
}
