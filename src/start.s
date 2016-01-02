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
.section ".text.startup"
.global _start



//
// ARM Processor Modes (section A2.2)
//
.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F



//
// Program Status Registers (section A2.5)
//
.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20



//
// The address at which the kernel is loaded
// TODO move kernel load address to 0x0000?
//
.equ	KERNEL_LOAD_ADDRESS,		0x8000



//
// Stack addresses for the supervisor and interrupt handler
//
.equ	SUPERVISOR_STACK_ADDRESS,	0x8000
.equ	INTERRUPT_STACK_ADDRESS,	0x4000



//
// Entry point of rpi-os
//
_start:
	//
	// Interrupt jump table
	//
    ldr		pc, _reset_h
    ldr		pc, _undefined_instruction_vector_h
    ldr		pc, _software_interrupt_vector_h
    ldr		pc, _prefetch_abort_vector_h
    ldr		pc, _data_abort_vector_h
    ldr		pc, _unused_handler_h
    ldr		pc, _interrupt_vector_h
    ldr		pc, _fast_interrupt_vector_h


	//
	// Interrupt table, located directly after the jump table
	//
	_reset_h:                           .word   _reset_
	_undefined_instruction_vector_h:    .word   undefined_instruction_vector
	_software_interrupt_vector_h:       .word   software_interrupt_vector
	_prefetch_abort_vector_h:           .word   prefetch_abort_vector
	_data_abort_vector_h:               .word   data_abort_vector
	_unused_handler_h:                  .word   _reset_
	_interrupt_vector_h:                .word   interrupt_vector
	_fast_interrupt_vector_h:           .word   fast_interrupt_vector



//
// Program reset
//
// Note: ARM starts in supervisor mode (ARM Section A2.2)
//
_reset_:
	//
	// If the reset instruction is already at 0x0000, there's been a restart
	//
	ldr		r0, =_reset_h
	mov		r1, #0
	ldr		r1, [r1]
	cmp		r0, r1
	beq		_restart

	//
    // If the kernel is not loaded at 0x0000 the interrupt jump tables must be copied to 0x0000
	//
    mov     r0, #KERNEL_LOAD_ADDRESS
	cmp		r0, #0
	beq		_copy_done
    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
_copy_done:



	//
	// Setup the interrupt stack
	//
    mov		r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT)
    msr		cpsr_c, r0
    mov		sp, #INTERRUPT_STACK_ADDRESS



    //
	// Setup the supervisor stack
	//
    mov		r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr		cpsr_c, r0
    mov		sp, #SUPERVISOR_STACK_ADDRESS

    

	//
	// Setup the VFP coprocessor
	//
    mrc		p15, #0, r1, c1, c0, #2
    orr		r1, r1, #(0xf << 20)
    mcr		p15, #0, r1, c1, c0, #2
    mov		r1, #0
    mcr		p15, #0, r1, c7, c5, #4
    mov		r0, #0x40000000
    fmxr	fpexc, r0



	//
	// Zero the BSS section
	//
	movs	r0, #0
	ldr		r1, =__bss_start__
	ldr		r2, =__bss_end__
_bss_loop:
	cmp		r1, r2
	strlo	r0, [r1], #4
	blo		_bss_loop



	//
	// Call _cmain fuction, should not return
	//
    bl      _cmain

	

	//
    // Disable interrupts and spin
	//
	bl		_disable_interrupts
_idle_loop:
	bl		_wait_for_interrupt
	bl		_idle_loop



	//
	// A restart has been triggered
	//
_restart:
	bl		_led_blink
	bl		_restart
