;/**
; * @file boot.s
; * @brief boot code
; * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
; * @author Miaobaoli
; * @date 2005-07-13
; * @version 1.0
; * @note ref Janus II.pdf, AK3223M technical manual.
; */

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#include "anyka_bsp.h"
#include "boot.h"

#ifdef AKOS
	IMPORT	INC_Initialize
	IMPORT	TMT_Timer0_Interrupt

	IMPORT	 TCD_System_Stack		
	IMPORT	 TCT_System_Limit		
	IMPORT	 TMD_HISR_Stack_Ptr
	IMPORT	 TMD_HISR_Stack_Size	
	IMPORT	 TMD_HISR_Priority
	IMPORT	 TMD_Timer
#endif
  	
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
	b 	reset_handler		;Start point: Jump to first instruction: reset_handler
	b	undefined_handler	;Exception(Undefined Instruction): Dead Loop
	b 	swi_handler			;Software Interrupt: Jump to entry of SWI
	b	prefetch_handler	;Exception(Instruction Prefetch Failure): Dead Loop
	b	abort_handler		;Exception(Data Abort): Dead Loop
	nop						;Reserved
	b	irq_handler			;Interrupt Request: Jump to entry of IRQ
	b 	fiq_handler			;Fast Interrupt Request: Reserved

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
nandflash_information
     DCB  "NAND"
     DCD   0x5A5A5A5A             ;chip id
     DCW  2048                    ;page size
     DCW  64                      ;page of one blok
     DCW  1024                    ;total block number
     DCW  1024                    ;if size is 512M, A29 must the same when copy back, we thougt the A29=0 is a group,A29=1 is another group   
     DCW  1024                    
     DCB   64                      ;spare size
     DCB   2                       ;column address cycle
     DCB   0x0f                    ;last column  addrress cycle mask bit
     DCB  2                       ;row address cycle
     DCB   0xff                    ;last row address cycle mask bit
     DCB   0x0                     ;if is a custom nandflash(ohter not typicly nandflash),set bit,use this would fast
     DCD   0xC0000001              ;flag of flash character
     DCD   0xC3671                 ;nandflash command length
     DCD   0xF3637                 ;nandflash data length              ; read page command wait time 1mSecond. 0: use default value; other: new wait value;
     space   32
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

;===================================================================================
#ifdef AKOS
	
	;@ akos boot init, is called in main() function
	EXPORT	 AKOS_BOOT_INIT
AKOS_BOOT_INIT
	MOV	R0, #1					    ;@@ All vectors are assumed loaded
	LDR	R1, =INT_Loaded_Flag		;@@ Buildaddress of loaded flag
	STR	R0, [R1,#0]
	
	LDR	R0,=|Image$$ER_ZI$$ZI$$Limit|	   ;@@ Pick	up the .data start address
	MOV	R10,R0
	LDR	R3,=TCT_System_Limit	   ;@@ Pickup sys stack	limit addr
	STR	R10,[R3, #0]			   ;@@ Save	stack limit
	
	ldr r1, =SVC_MODE_STACK
	LDR	R3,=TCD_System_Stack		;@@ Pickup system stack address
	STR	r1,[R3,#0]				    ;@@ Save	stack pointer
	
	LDR	R3, =TMD_HISR_Stack_Ptr	 ;@@ Pickup the address of variable
	LDR R2, =UNDEF_MODE_STACK	
	;@ UNDEF_MODE_STACK not align by 8, so sub 4
	SUB	R2, R2,	#8				 ;@@ Increment to next available	word
	STR	R2, [R3, #0]			 ;@@ Setup timer	HISR stack pointer
    
;@@	MOV	R1,#TimerHISR		     ;@@ Pickup the timer HISR stack	size
	LDR	R1,=TimerHISR			 ;@@ Pickup the timer HISR stack	size
	BIC	R1,R1, #3				 ;@@ Insure word	alignment
	;ADD	R2,R2, R1				 ;@@ Allocate the timer HISR	stack
								 ;@@ from available memory
	LDR	R3,=TMD_HISR_Stack_Size	 ;@@ Pickup the address of variable
	STR	R1,[R3, #0]			          ;@@ Setup timer	HISR stack size

	MOV	R1,#Timer_Priority		;@@ Pickup timer HISR priority (0-2)
	LDR	R3,=TMD_HISR_Priority	;@@ Pickup the address of variable
	STR	R1,[R3, #0]			    ;@@ Setup timer	HISR priority
	
	MOV	PC, LR
	
#endif
;==================================================================================== 
	
	EXPORT __user_initial_stackheap

__user_initial_stackheap    
    LDR   r0,=|Image$$ER_ZI$$ZI$$Limit|
    MOV   pc,lr
	
	EXPORT main

main 
#ifdef AKOS
	IMPORT  INC_Initialize
    bl AKOS_BOOT_INIT
    ldr		pc, =INC_Initialize
#else
    ldr		pc,	=CMain
#endif
    MOV   pc,lr
	
	;end of user function, enter dead loop
reset_handler_loop
	b		reset_handler_loop	


;====================================================================================
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@	INT_Vectors_Loaded
;@@
;@@ This	function returns the flag that indicates whether or not all the
;@@ default vectors have	been loaded.  If it is false, each LISR register
;@@ also	loads the ISR shell	into the actual	vector table.
;@@
;@@	CALLED BY :	TCC_Register_LISR	Register LISR for vector
;@@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@ INT	  INT_Vectors_Loaded(void)

	EXPORT	INT_Vectors_Loaded

INT_Vectors_Loaded

;@@ Just	return the loaded vectors flag.	 
;@@ return(INT_Loaded_Flag);
	LDR	R1, =INT_Loaded_Flag
	LDR	R0,[R1, #0]

	BX	lr

;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@
;@@	INT_Setup_Vector
;@@		Thisfunction sets up the specified vector with the new vector
;@@ 		value. The previous	vector value is	returned to he caller.
;@@
;@@	Called By :	Application
;@@			TCC_Register_LISR
;@@
;@@	Input :
;@@	vector	- Vector number	to setup
;@@		- Pointer to new assembly language ISR
;@@
;@@	Ouput : old vector contents
;@@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@ VOID	 *INT_Setup_Vector(INT vector, VOID	*new)

	EXPORT	INT_Setup_Vector
INT_Setup_Vector

;@VOID	*old_Vector;				@@ Old interrupt vector
;@VOID	**vector_table				@@ Pointer to vector	table 

;@@ Calculate the starting address of the actual	vector table.  
;@@ vector_table	=  (VOID **)&INT_IRQ_Vectors;

;@@	Pickup the old interrupt vector.  
;@@	old_vector =  vector_table[vector];
   
;@@	Setup the new interrupt	vector.	 
;@@	vector_table[vector] =	new;
	
;@@	Return the old interrupt vector
;@@	return(old_vector);

	;LDR	R2, =INT_IRQ_Vectors		;@@ Load the vector table	address
	;MOV	R0, R0, LSL #2				;@@ Multiply vector by 4 to get offset into table
	;LDR	R3, [R2, R0]				;@@ Load the old pointer
	;STR	R1,[R2, R0]					;@@ Store	the	new	pointer	into the vector	table

	;MOV	R0, R3						;@@ Put the old pointer into the return register

	BX	LR


 ;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@	INT_Retrieve_Shell 
;@@ 
;@@	This function retrieves	the	pointer	to the shell interrupt service	
;@@	routine. The shell interrupt service routine calls the LISR	dispatch
;@@	routine.
;@@
;@@	Called by :	TCC_Register_LISR	- Register	LISR for vector
;@@
;@@	Input :	vector				- Vector number to setup
;@@
;@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
;@@ VOID	 *INT_Retrieve_Shell(INT vector)

	EXPORT INT_Retrieve_Shell

INT_Retrieve_Shell

	;LDR	R1, =INT_IRQ_Vectors
	;MOV	R0, R0, LSL #2			;@@ Multiply vector by 4 to get offset into table
	;STR	R0,[R1, R0]

	BX  LR

;====================================================================================

  	EXPORT	 INT_Loaded_Flag
INT_Loaded_Flag  DCD	0
;Loaded_Flag      EQU   INT_Loaded_Flag

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
