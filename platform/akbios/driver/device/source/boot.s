/**
 * @file boot.s
 * @brief boot code
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
 * @note ref Janus II.pdf, AK3223M technical manual.
 */

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "anyka_bsp.h"
#include "boot.h"

.global _start
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Exception vectors   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

_start:
					b 	reset_handler		@Start point: Jump to first instruction: reset_handler
					b	undefined_handler	@Exception(Undefined Instruction): Dead Loop
					b 	swi_handler			@Software Interrupt: Jump to entry of SWI
					b	prefetch_handler	@Exception(Instruction Prefetch Failure): Dead Loop
					b	abort_handler		@Exception(Data Abort): Dead Loop
					nop						@Reserved
					b	irq_handler			@Interrupt Request: Jump to entry of IRQ
					b 	fiq_handler			@Fast Interrupt Request: Reserved

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Entry of Reset   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

reset_handler:
	@disable all interrupt
    bl intr_mask_irq_fiq
    
	@if the program is in flash and need to be copied to RAM for running,
	@please define micro definition __COPY_DATA_TO_RAM__ to use the following program
#ifdef __COPY_DATA_TO_RAM__
	@copy .text, .data and .rodata to RAM
	ldr		r0, =0x10010000		@we suppose user's program is storaged at 0x10010000
   	ldr		r1, =_text
   	ldr		r3, =_edata
copy_data_loop:					@a loop to copy data to RAM
  	cmp		r1, r3
    ldrcc	r2, [r0], #4
    strcc	r2, [r1], #4
    bcc		copy_data_loop
#endif

#ifdef __COPY_DATA_TO_INNERRAM__
	@copy .inner to inner RAM
    	ldr		r0, =__rodata_end
   	ldr		r1, =0x40000000		@the start address of inner RAM
   	ldr		r3, =__bin_end
copy_inner_loop:				@a loop to copy data to inner RAM
  	cmp		r0, r3
    ldrcc	r2, [r0], #4
    strcc	r2, [r1], #4
    bcc		copy_inner_loop
#endif

    @clear bss
    ldr  r0, =__bss_start
    ldr  r2, =0x0
    ldr  r3, =__bss_end
clear_bss_loop:
    cmp  r0, r3
    strcc r2, [r0], #4
    bcc  clear_bss_loop
    
	@Change processor mode to IRQ Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_IRQ 
	msr		cpsr, r0
	ldr     r1, =IRQ_MODE_STACK
	mov		sp, r1

	@Change processor mode to fiq Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_FIQ 
	msr		cpsr, r0
	ldr     r1, =FIQ_MODE_STACK
	mov		sp, r1

	@Change processor mode to Abort Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_ABT
	msr		cpsr, r0
	ldr     r1, =ABORT_MODE_STACK
	mov		sp, r1

	@Change processor mode to Undefied Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_UNDEF
	msr		cpsr, r0
	ldr     r1, =UNDEF_MODE_STACK
	mov		sp, r1	
	
	@Change processor mode to User Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_SVC
	msr		cpsr, r0
	ldr     r1, =SVC_MODE_STACK
	mov		sp, r1

	@now start user function: CMain()
	ldr		pc,	=CMain
	
	@end of user function, enter dead loop
reset_handler_loop:
	b		reset_handler_loop	

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Entry of SWI   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

swi_handler:
	@check type number of SWI
	cmp		r0, #0
	beq		SWI_0
	
	@cmp	r0, #n
	@be		SWI_n
	
	b		swi_handler_exit

SWI_0:										@This is NO. 0 SWI, do flush cache
	mcr p15, 0, r0, c7, c1, 0	@Flush Cahce
	b		swi_handler_exit
	
@SWI_n:										@handle SWI n
	@b		swi_handler_exit
	
swi_handler_exit:							@exit point of SWI
	movs	r15, r14

