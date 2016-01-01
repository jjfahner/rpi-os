// Functions in the text segment
.section ".text.functions"



// Globally visible functions
.global _get_stack_pointer
.global _spin
.global _led_blink
.global _enable_interrupts
.global _disable_interrupts
.global _get_interrupts
.global _wait_for_interrupt
.global _switch_to_thread


// Return the stack pointer value
_get_stack_pointer:
    str     sp, [sp]
    ldr     r0, [sp]
    bx		lr



//
// Enable interrupts. Returns old mode.
//
// extern uint32_t _enable_interrupts(uint32_t mode);
//
_enable_interrupts:
    mrs     r0, cpsr			// Store current mode in r1
    bic     r0, r0, #0xC0		// Add new mode into r2
    msr     cpsr_c, r0			// Enable new mode from r2
    bx		lr



//
// Disable interrupts. Returns old mode.
//
// extern uint32_t _disable_interrupts(uint32_t mode);
//
_disable_interrupts:
    mrs     r0, cpsr			// Store current mode in r1
    orr     r0, r0, #0xC0		// Remove input flags
    msr     cpsr_c, r0			// Enable new mode
    bx		lr



//
// Get enabled interrupts
//
// uint32_t _get_interrupts();
//
_get_interrupts:
	mrs		r0, cpsr
	bx		lr



//
// Suspend cpu until interrupt occurs
//
// extern void _wait_for_interrupt(void);
//
_wait_for_interrupt:
	mcr		p15, 0, R0, c7, c0, 4
	bx		lr



//
// Instruction memory barrier
//
// extern void _isb();
//
_isb:
	mcr		p15, 0, R0, c7,  c5, 4
	bx		lr



//
// Data memory barrier
//
// extern void _dmb();
//
_dmb:
	mcr		p15, 0, R0, c7, c10, 5
	bx		lr



//
// Spin for the specified number of cycles
//
// extern void _spin(uint32_t cycles);
//
_spin: 
	subs	r0, #1
	bne		_spin
    bx		lr



//
// Blink the LED
//
// extern void _led_blink();
//
_led_blink:
	ldr		r0,=0x20200000
	mov		r1,#0x8000
	str		r1,[r0,#32]
	mov		r1, #0x100000
_wait_1:
	subs	r1, #1
	bne		_wait_1
	mov		r1,#0x8000
	str		r1,[r0,#44]
	mov		r1, #0x100000
_wait_2:
	subs	r1, #1
	bne		_wait_2
	bx		lr



//
// Switch from one thread to another
//
// extern void _switch_to_thread(uint32_t* cur_regs, uint32_t* new_regs);
//
_switch_to_thread:
	stmia	r0, {r0-r14}
	ldmia	r1, {r0-r14}
	bx	lr
