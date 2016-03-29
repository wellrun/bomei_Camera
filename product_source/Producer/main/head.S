;/*
;*  head.S
; *
; * bios entry for M3 test boad
; */
#include "anyka_bsp.h"
#include "boot.h"
#include "version.h"

    IMPORT  CMain
    IMPORT  irq_handler
    IMPORT  fiq_handler
    IMPORT  undefined_handler
    IMPORT  prefetch_handler
    IMPORT  abort_handler
    IMPORT  swi_dispatch
    IMPORT  intr_mask_irq_fiq
    
    IMPORT |Image$$ER_RO$$Base|
	IMPORT |Image$$ER_RO$$Limit|
	IMPORT |Image$$ER_ZI$$ZI$$Base|	;; MUST bss$$ZI, not bss
	IMPORT |Image$$ER_ZI$$ZI$$Limit|  ;; MUST bss$$ZI, not bss
	
    AREA BOOT, CODE, READONLY
    CODE32
    ENTRY
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;   Exception vectors   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    EXPORT START
START
    b 	reset_handler				;Start point: Jump to first instruction: reset_handler
    b	undefined_handler	        ;Exception(Undefined Instruction): Dead Loop
    b 	swi_handler					;Software Interrupt: Jump to entry of SWI
    b	prefetch_handler		    ;Exception(Instruction Prefetch Failure): Dead Loop
    b	abort_handler				;Exception(Data Abort): Dead Loop
    nop								;Reserved
    b	irq_handler					;Interrupt Request: Jump to entry of IRQ
    nop							    ;Fast Interrupt Request: Reserved

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ver_check0      DCB VER_CHECK0
ver_check1      DCB VER_CHECK1
ver_check2      DCB VER_CHECK2
ver_check3      DCB VER_CHECK3
main_version    DCD MAIN_VERSION
sub1_version    DCD SUB1_VERSION
sub2_version    DCD SUB2_VERSION
reserve         DCD SUB3_VERSION

;;;;;;;;;;;;;;;;;;;;;;   Entry of Reset   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
reset_handler
	;Change processor mode to IRQ Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_IRQ 
	msr		CPSR_c, r0
	ldr     r1, =IRQ_MODE_STACK
	mov		sp, r1

	;Change processor mode to fiq Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_FIQ 
	msr		CPSR_c, r0
	ldr     r1, =FIQ_MODE_STACK
	mov		sp, r1

	;Change processor mode to Abort Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_ABT
	msr		CPSR_c, r0
	ldr     r1, =ABORT_MODE_STACK
	mov		sp, r1

	;Change processor mode to Undefied Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_UNDEF
	msr		CPSR_c, r0
	ldr     r1, =UNDEF_MODE_STACK
	mov		sp, r1	
	
	;Change processor mode to User Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_SVC
	msr		CPSR_c, r0
	ldr     r1, =SVC_MODE_STACK
	mov		sp, r1

	;disable all interrupt
    bl      intr_mask_irq_fiq
	
	;@ we will copy data , copy inner, and clear zi in __main
	;@ then go to main function
	IMPORT  __main
	
	B       __main

;==================================================================================== 
	
	EXPORT __user_initial_stackheap

__user_initial_stackheap    
    LDR   r0,=|Image$$ER_ZI$$ZI$$Limit|
    MOV   pc,lr
	
	EXPORT main

main 
    ldr		pc,	=CMain
    MOV   pc,lr
	
	;end of user function, enter dead loop
reset_handler_loop
	b		reset_handler_loop	

;====================================================================================

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;   Entry of SWI   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

swi_handler
    ;check SWI 0x18, ReportException SWI
    cmp		r0, #0x18
	bne	    swi_handler_exit
	
	;fetch number of SWI in ARM state
    ldr     r2, [lr, #-4]
    bic     r2, r2, #0xff000000

    bl      swi_dispatch
    
swi_handler_exit							;exit point of SWI
	movs	r15, r14

    END