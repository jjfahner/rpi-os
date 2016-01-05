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
.global _isb
.global _dsb



// Return the stack pointer value
_get_stack_pointer:
    str     sp, [sp]
    ldr     r0, [sp]
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
// Switch from one thread to another
//
// Note that the return value must be set through new_regs, it is not set by the code below
//
// extern uint32_t _switch_to_thread(uint32_t* cur_regs, uint32_t* new_regs);
//
_switch_to_thread:
	stmia	r0, {r0-r14}
	ldmia	r1, {r0-r14}
	bx	lr



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
// Add an unsigned 32-bit word to an unsigned 64-bit word
//
// extern void _uint64_add_32(uint32_t* lo, uint32_t* hi, uint32_t add);
//
_uint64_add_32:
	push	{r4,r5}
	ldr		r4, [r0]
	ldr		r5, [r1]
	adds	r4, r2
	adcs	r5, #0
	str		r4, [r0]
	str		r5, [r1]
	pop		{r4,r5}
	bx		lr



//
// Add an unsigned 64-bit word to an unsigned 64-bit word
//
// extern void _uint64_add_64(uint32_t* lo, uint32_t* hi, uint32_t add_lo, uint32_t add_hi);
//
_uint64_add_64:
	push	{r4,r5}
	ldr		r4, [r0]
	ldr		r5, [r1]
	adds	r4, r2
	adcs	r5, r3
	str		r4, [r0]
	str		r5, [r1]
	pop		{r4,r5}
	bx		lr



//
// Add an unsigned 32-bit word to a sys_time_t
//
// extern void _sys_time_t_add_32(sys_time_t* time, uint32_t add);
//
_sys_time_t_add_32:
	push	{r3, r4}
	ldr		r3, [r0]
	ldr		r4, [r0, #4]
	adds	r3, r1
	adcs	r4, #0
	str		r3, [r0]
	str		r4, [r0, #4]
	pop		{r3,r4}
	bx		lr



//
// Add an unsigned 64-bit word to a sys_time_t
//
// extern void _sys_time_t_add_64(sys_time_t* time, uint32_t add_lo, uint32_t add_hi);
//
_sys_time_t_add_64:
	push	{r3, r4}
	ldr		r3, [r0]
	ldr		r4, [r0, #4]
	adds	r3, r1
	adcs	r4, r2
	str		r3, [r0]
	str		r4, [r0, #4]
	pop		{r3,r4}
	bx		lr



//
// Subtract unsigned 64-bit word to a sys_time_t
//
// extern void _sys_time_t_sub_64(sys_time_t* time, uint32_t sub_lo, uint32_t sub_hi);
//
_sys_time_t_sub_64:
	push	{r3, r4}
	ldr		r3, [r0]
	ldr		r4, [r0, #4]
	subs	r3, r1
	subcs	r4, r2
	str		r3, [r0]
	str		r4, [r0, #4]
	pop		{r3,r4}
	bx		lr



//
// Compare two sys_time_t objects
//
// EXTERN_C int32_t sys_time_compare(const sys_time_t* first, const sys_time_t* second);
//
/*
sys_time_compare:
	push	{r2, r3}
	ldr		r2, [r0, #4]
	ldr		r3, [r1, #4]
	cmp		r2, r3
	bne		sys_time_compare_result
	ldr		r2, [r0]
	ldr		r3, [r1]
	cmp		r2, r3
sys_time_compare_result:
	moveq	r0, #0
	movhi	r0, #1
	movlo	r0, #0xFFFFFFFF
	pop		{r2, r3}
	bx		lr
*/