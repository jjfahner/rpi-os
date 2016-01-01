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
