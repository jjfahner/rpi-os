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

//
// This header file contains the various types and constants required for Raspberry Pi programming
//



//
// Required include files
//
#include <stdint.h>



//
// Basic types
//
typedef volatile uint32_t rpi_reg_rw_t;
typedef volatile const uint32_t rpi_reg_ro_t;
typedef volatile uint32_t rpi_reg_wo_t;

typedef volatile uint64_t rpi_wreg_rw_t;
typedef volatile const uint64_t rpi_wreg_ro_t;


////////////////////////////////////////////////////////////////////////////////
//
// Memory offsets
//
////////////////////////////////////////////////////////////////////////////////



//#if defined(RPIPLUS)
#define PERIPHERAL_BASE     0x20000000UL
//#else
//#error TODO set peripheral base for other RPi versions
//#endif



////////////////////////////////////////////////////////////////////////////////
//
// Interrupts
//
////////////////////////////////////////////////////////////////////////////////



/** @brief See Section 7.5 of the BCM2836 ARM Peripherals documentation, the base
address of the controller is actually xxxxB000, but there is a 0x200 offset
to the first addressable register for the interrupt controller, so offset the
base to the first register */
#define RPI_INTERRUPT_CONTROLLER_BASE   ( PERIPHERAL_BASE + 0xB200 )



/** @brief Bits in the Enable_Basic_IRQs register to enable various interrupts.
See the BCM2835 ARM Peripherals manual, section 7.5 */
#define RPI_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define RPI_BASIC_ARM_MAILBOX_IRQ       (1 << 1)
#define RPI_BASIC_ARM_DOORBELL_0_IRQ    (1 << 2)
#define RPI_BASIC_ARM_DOORBELL_1_IRQ    (1 << 3)
#define RPI_BASIC_GPU_0_HALTED_IRQ      (1 << 4)
#define RPI_BASIC_GPU_1_HALTED_IRQ      (1 << 5)
#define RPI_BASIC_ACCESS_ERROR_1_IRQ    (1 << 6)
#define RPI_BASIC_ACCESS_ERROR_0_IRQ    (1 << 7)
#define RPI_BASIC_ARM_IRQ_1				(1 << 8)
#define RPI_BASIC_ARM_IRQ_2				(1 << 9)



/** @brief The interrupt controller memory mapped register set */
typedef struct {
	volatile uint32_t irq_basic_pending;
	volatile uint32_t irq_pending_1;
	volatile uint32_t irq_pending_2;
	volatile uint32_t fiq_control;
	volatile uint32_t enable_irqs_1;
	volatile uint32_t enable_irqs_2;
	volatile uint32_t enable_basic_irqs;
	volatile uint32_t disable_irqs_1;
	volatile uint32_t disable_irqs_2;
	volatile uint32_t disable_basic_irqs;
} rpi_irq_controller_t;



//
// Pointer to the IRQ controller
//
#define rpi_irq_controller ((rpi_irq_controller_t*)RPI_INTERRUPT_CONTROLLER_BASE)



////////////////////////////////////////////////////////////////////////////////
//
// ARM Timer
//
////////////////////////////////////////////////////////////////////////////////



/** @brief See the documentation for the ARM side timer (Section 14 of the
BCM2835 Peripherals PDF) */
#define RPI_ARMTIMER_BASE               ( PERIPHERAL_BASE + 0xB400 )

/** @brief 0 : 16-bit counters - 1 : 32-bit counter */
#define RPI_ARMTIMER_CTRL_32BIT         ( 1 << 1 )

#define RPI_ARMTIMER_CTRL_PRESCALE_1    ( 0 << 2 )
#define RPI_ARMTIMER_CTRL_PRESCALE_16   ( 1 << 2 )
#define RPI_ARMTIMER_CTRL_PRESCALE_256  ( 2 << 2 )

/** @brief 0 : Timer interrupt disabled - 1 : Timer interrupt enabled */
#define RPI_ARMTIMER_CTRL_INT_ENABLE    ( 1 << 5 )
#define RPI_ARMTIMER_CTRL_INT_DISABLE   ( 0 << 5 )

/** @brief 0 : Timer disabled - 1 : Timer enabled */
#define RPI_ARMTIMER_CTRL_ENABLE        ( 1 << 7 )
#define RPI_ARMTIMER_CTRL_DISABLE       ( 0 << 7 )


/** @brief Section 14.2 of the BCM2835 Peripherals documentation details
the register layout for the ARM side timer */
typedef struct {

	/** The timer load register sets the time for the timer to count down.
	This value is loaded into the timer value register after the load
	register has been written or if the timer-value register has counted
	down to 0. */
	volatile uint32_t load;

	/** This register holds the current timer value and is counted down when
	the counter is running. It is counted down each timer clock until the
	value 0 is reached. Then the value register is re-loaded from the
	timer load register and the interrupt pending bit is set. The timer
	count down speed is set by the timer pre-divide register. */
	volatile uint32_t value;

	/** The standard SP804 timer control register consist of 8 bits but in the
	BCM implementation there are more control bits for the extra features.
	Control bits 0-7 are identical to the SP804 bits, albeit some
	functionality of the SP804 is not implemented. All new control bits
	start from bit 8 upwards. */
	volatile uint32_t control;

	/** The timer IRQ clear register is write only. When writing this register
	the interrupt-pending bit is cleared. When reading this register it
	returns 0x544D5241 which is the ASCII reversed value for "ARMT". */
	volatile uint32_t irq_clear;

	/** The raw IRQ register is a read-only register. It shows the status of
	the interrupt pending bit. 0 : The interrupt pending bits is clear.
	1 : The interrupt pending bit is set.

	The interrupt pending bits is set each time the value register is
	counted down to zero. The interrupt pending bit can not by itself
	generates interrupts. Interrupts can only be generated if the
	interrupt enable bit is set. */
	volatile uint32_t raw_irq;

	/** The masked IRQ register is a read-only register. It shows the status
	of the interrupt signal. It is simply a logical AND of the interrupt
	pending bit and the interrupt enable bit. 0 : Interrupt line not
	asserted. 1 :Interrupt line is asserted, (the interrupt pending and
	the interrupt enable bit are set.)  */
	volatile uint32_t masked_irq;

	/** This register is a copy of the timer load register. The difference is
	that a write to this register does not trigger an immediate reload of
	the timer value register. Instead the timer load register value is
	only accessed if the value register has finished counting down to
	zero. */
	volatile uint32_t reload;

	/** The Pre-divider register is not present in the SP804. The pre-divider
	register is 10 bits wide and can be written or read from. This
	register has been added as the SP804 expects a 1MHz clock which we do
	not have. Instead the pre-divider takes the APB clock and divides it
	down according to:

	timer_clock = apb_clock/(pre_divider+1)

	The reset value of this register is 0x7D so gives a divide by 126. */
	volatile uint32_t pre_devider;

	/** The free running counter is not present in the SP804. The free running
	counter is a 32 bits wide read only register. The register is enabled
	by setting bit 9 of the Timer control register. The free running
	counter is incremented immediately after it is enabled. The timer can
	not be reset but when enabled, will always increment and roll-over.

	The free running counter is also running from the APB clock and has
	its own clock pre-divider controlled by bits 16-23 of the timer
	control register.

	This register will be halted too if bit 8 of the control register is
	set and the ARM is in Debug Halt mode. */
	volatile uint32_t free_running_counter;

} rpi_arm_timer_t;


//
// Pointer to the arm timer
//
#define rpi_arm_timer ((rpi_arm_timer_t*)RPI_ARMTIMER_BASE)



////////////////////////////////////////////////////////////////////////////////
//
// GPIO
//
////////////////////////////////////////////////////////////////////////////////



/** The base address of the GPIO peripheral (ARM Physical Address) */
#define RPI_GPIO_BASE       ( PERIPHERAL_BASE + 0x00200000UL )


typedef enum {
    FS_INPUT = 0,
    FS_OUTPUT,
    FS_ALT5,
    FS_ALT4,
    FS_ALT0,
    FS_ALT1,
    FS_ALT2,
    FS_ALT3,
    } rpi_gpio_alt_function_t;

/* A mask to be able to clear the bits in the register before setting the
   value we require */
#define FS_MASK     (7)

typedef enum {
    RPI_GPIO0 = 0,
    RPI_GPIO1,
    RPI_GPIO2,
    RPI_GPIO3,
    RPI_GPIO4,
    RPI_GPIO5,
    RPI_GPIO6,
    RPI_GPIO7,
    RPI_GPIO8,
    RPI_GPIO9,
    RPI_GPIO10 = 10,
    RPI_GPIO11,
    RPI_GPIO12,
    RPI_GPIO13,
    RPI_GPIO14,
    RPI_GPIO15,
    RPI_GPIO16,
    RPI_GPIO17,
    RPI_GPIO18,
    RPI_GPIO19,
    RPI_GPIO20 = 20,
    RPI_GPIO21,
    RPI_GPIO22,
    RPI_GPIO23,
    RPI_GPIO24,
    RPI_GPIO25,
    RPI_GPIO26,
    RPI_GPIO27,
    RPI_GPIO28,
    RPI_GPIO29,
    RPI_GPIO30 = 30,
    RPI_GPIO31,
    RPI_GPIO32,
    RPI_GPIO33,
    RPI_GPIO34,
    RPI_GPIO35,
    RPI_GPIO36,
    RPI_GPIO37,
    RPI_GPIO38,
    RPI_GPIO39,
    RPI_GPIO40 = 40,
    RPI_GPIO41,
    RPI_GPIO42,
    RPI_GPIO43,
    RPI_GPIO44,
    RPI_GPIO45,
    RPI_GPIO46,
    RPI_GPIO47,
    RPI_GPIO48,
    RPI_GPIO49,
    RPI_GPIO50 = 50,
    RPI_GPIO51,
    RPI_GPIO52,
    RPI_GPIO53,
    } rpi_gpio_pin_t;


/** The GPIO Peripheral is described in section 6 of the BCM2835 Peripherals
    documentation.

    There are 54 general-purpose I/O (GPIO) lines split into two banks. All
    GPIO pins have at least two alternative functions within BCM. The
    alternate functions are usually peripheral IO and a single peripheral
    may appear in each bank to allow flexibility on the choice of IO voltage.
    Details of alternative functions are given in section 6.2. Alternative
    Function Assignments.

    The GPIO peripheral has three dedicated interrupt lines. These lines are
    triggered by the setting of bits in the event detect status register. Each
    bank has its’ own interrupt line with the third line shared between all
    bits.

    The Alternate function table also has the pull state (pull-up/pull-down)
    which is applied after a power down. */
typedef struct {
    rpi_reg_rw_t    GPFSEL0;
    rpi_reg_rw_t    GPFSEL1;
    rpi_reg_rw_t    GPFSEL2;
    rpi_reg_rw_t    GPFSEL3;
    rpi_reg_rw_t    GPFSEL4;
    rpi_reg_rw_t    GPFSEL5;
    rpi_reg_ro_t    Reserved0;
    rpi_reg_wo_t    GPSET0;
    rpi_reg_wo_t    GPSET1;
    rpi_reg_ro_t    Reserved1;
    rpi_reg_wo_t    GPCLR0;
    rpi_reg_wo_t    GPCLR1;
    rpi_reg_ro_t    Reserved2;
    rpi_reg_wo_t    GPLEV0;
    rpi_reg_wo_t    GPLEV1;
    rpi_reg_ro_t    Reserved3;
    rpi_reg_wo_t    GPEDS0;
    rpi_reg_wo_t    GPEDS1;
    rpi_reg_ro_t    Reserved4;
    rpi_reg_wo_t    GPREN0;
    rpi_reg_wo_t    GPREN1;
    rpi_reg_ro_t    Reserved5;
    rpi_reg_wo_t    GPFEN0;
    rpi_reg_wo_t    GPFEN1;
    rpi_reg_ro_t    Reserved6;
    rpi_reg_wo_t    GPHEN0;
    rpi_reg_wo_t    GPHEN1;
    rpi_reg_ro_t    Reserved7;
    rpi_reg_wo_t    GPLEN0;
    rpi_reg_wo_t    GPLEN1;
    rpi_reg_ro_t    Reserved8;
    rpi_reg_wo_t    GPAREN0;
    rpi_reg_wo_t    GPAREN1;
    rpi_reg_ro_t    Reserved9;
    rpi_reg_wo_t    GPAFEN0;
    rpi_reg_wo_t    GPAFEN1;
    rpi_reg_ro_t    Reserved10;
    rpi_reg_wo_t    GPPUD;
    rpi_reg_wo_t    GPPUDCLK0;
    rpi_reg_wo_t    GPPUDCLK1;
    rpi_reg_ro_t    Reserved11;
    } rpi_gpio_t;


typedef enum {
    RPI_IO_LO = 0,
    RPI_IO_HI,
    RPI_IO_ON,
    RPI_IO_OFF,
    RPI_IO_UNKNOWN,
    } rpi_gpio_value_t;


//
// Pointer to the GPIO
//
#define rpi_gpio ((rpi_gpio_t*)RPI_GPIO_BASE)



////////////////////////////////////////////////////////////////////////////////
//
// UART
//
////////////////////////////////////////////////////////////////////////////////



// The GPIO registers base address
#define GPIO_OFFSET		(RPI_GPIO_BASE)

// The base address for UART
#define UART0_OFFSET	(GPIO_OFFSET + 0x00001000)

// The offsets for each register for the UART
#define UART0_DR		(UART0_OFFSET + 0x00)
#define UART0_RSRECR	(UART0_OFFSET + 0x04)
#define UART0_FR		(UART0_OFFSET + 0x18)
#define UART0_ILPR		(UART0_OFFSET + 0x20)
#define UART0_IBRD		(UART0_OFFSET + 0x24)
#define UART0_FBRD		(UART0_OFFSET + 0x28)
#define UART0_LCRH		(UART0_OFFSET + 0x2C)
#define UART0_CR		(UART0_OFFSET + 0x30)
#define UART0_IFLS		(UART0_OFFSET + 0x34)
#define UART0_IMSC		(UART0_OFFSET + 0x38)
#define UART0_RIS		(UART0_OFFSET + 0x3C)
#define UART0_MIS		(UART0_OFFSET + 0x40)
#define UART0_ICR		(UART0_OFFSET + 0x44)
#define UART0_DMACR		(UART0_OFFSET + 0x48)
#define UART0_ITCR		(UART0_OFFSET + 0x80)
#define UART0_ITIP		(UART0_OFFSET + 0x84)
#define UART0_ITOP		(UART0_OFFSET + 0x88)
#define UART0_TDR		(UART0_OFFSET + 0x8C)

// Interrupt mask bits
#define UART0_OEIM		(1 << 10)
#define UART0_BEIM		(1 <<  9)
#define UART0_PEIM		(1 <<  8)
#define UART0_FEIM		(1 <<  7)
#define UART0_RTIM		(1 <<  6)
#define UART0_TXIM		(1 <<  5)
#define UART0_RXIM		(1 <<  4)
#define UART0_CTSMIM	(1 <<  1)


typedef struct 
{
	volatile uint32_t	dr;				// 0x00
	volatile uint32_t	rsrecr;			// 0x04
	volatile uint32_t	padding0[4];	// 0x08 0x0c 0x10 0x14
	volatile uint32_t	fr;				// 0x18
	volatile uint32_t	padding1[1];	// 0x1c
	volatile uint32_t	ilpr;			// 0x20
	volatile uint32_t	ibrd;			// 0x24
	volatile uint32_t	fbrd;			// 0x28
	volatile uint32_t	lcrh;			// 0x2c
	volatile uint32_t	cr;				// 0x30
	volatile uint32_t	ifls;			// 0x34
	volatile uint32_t	imsc;			// 0x38
	volatile uint32_t	ris;			// 0x3c
	volatile uint32_t	mis;			// 0x40
	volatile uint32_t	icr;			// 0x44
	volatile uint32_t	dmacr;			// 0x48
} rpi_uart_t;


//
// Pointer to the uart
//
#define rpi_uart		((rpi_uart_t*)UART0_OFFSET)


////////////////////////////////////////////////////////////////////////////////
//
// LED
//
////////////////////////////////////////////////////////////////////////////////



#if defined( RPIBPLUS ) || defined( RPI2 )
#define LED_GPFSEL      GPFSEL4
#define LED_GPFBIT      21
#define LED_GPSET       GPSET1
#define LED_GPCLR       GPCLR1
#define LED_GPIO_BIT    15
#else
#define LED_GPFSEL      GPFSEL1
#define LED_GPFBIT      18
#define LED_GPSET       GPSET0
#define LED_GPCLR       GPCLR0
#define LED_GPIO_BIT    16
#endif



////////////////////////////////////////////////////////////////////////////////
//
// System timer
//
////////////////////////////////////////////////////////////////////////////////


//
// Base address of the rpi_sys_timer_t object
//
#define RPI_SYSTIMER_BASE       ( PERIPHERAL_BASE + 0x3000 )



//
// System timers. 1 and 3 are arm side, 0 and 2 are GPU side
//
#define SYS_TIMER_0				(1 << 0)
#define SYS_TIMER_1				(1 << 1)
#define SYS_TIMER_2				(1 << 2)
#define SYS_TIMER_3				(1 << 3)



//
// Timer structure
//
typedef struct {
	volatile uint32_t cs;		// Control/status
	volatile uint32_t clo;		// Counter lo
	volatile uint32_t chi;		// Counter hi
	volatile uint32_t c0;		// Compare 0
	volatile uint32_t c1;		// Compare 1
	volatile uint32_t c2;		// Compare 2
	volatile uint32_t c3;		// Compare 3
} rpi_sys_timer_t;



//
// Pointer to the system timer
//
#define rpi_sys_timer ((rpi_sys_timer_t*)RPI_SYSTIMER_BASE)



////////////////////////////////////////////////////////////////////////////////
//
// 
//
////////////////////////////////////////////////////////////////////////////////

