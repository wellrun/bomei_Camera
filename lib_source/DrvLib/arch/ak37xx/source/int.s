#include "anyka_cpu.h"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Entry of IRQ   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global irq_handler
irq_handler:
    stmdb	sp!, {r0-r12, lr}				@backup the context

	@load the status register of interrupt
	ldr 	r0, =INT_STATUS_REG
	ldr		r9, [r0]
	ldr     r0, =IRQINT_MASK_REG
	ldr     r8, [r0]

	and    	r9, r9, r8
	
    mov     r0, #0
irq_handler_check_status:					@loop to check all bits of the status register
    mov     r1, #1
    mov     r1, r1, lsl r0
    tst     r9, r1
    bicne   r9, r9, r1
    stmdb	sp!, {r0}
    blne    irq_dispatch_handler
    ldmia	sp!, {r0}
    cmp     r9, #0
    beq     irq_handler_exit
    add     r0, r0, #1
    @cmp		r0, #INT_STATUS_NBITS     		@check valid status bits

    b     irq_handler_check_status

irq_handler_exit:							@Exit point of IRQ
	ldmia	sp!, {r0-r12, lr}				@restore the context
	subs	pc, lr, #4						@return to the interrupt point

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Entry of FIQ   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global fiq_handler
fiq_handler:

   stmdb	sp!, {r0-r7, lr}				@backup the context

	@load the status register of interrupt
	ldr 	r0, =INT_STATUS_REG
	ldr		r9, [r0]
	ldr     r0, =FRQINT_MASK_REG
	ldr     r8, [r0]

	AND    	r9, r9, r8
    mov     r0, #0
fiq_handler_check_status:					@loop to check all bits of the status register
    mov     r1, #1
    mov     r1, r1, lsl r0
    tst     r9, r1
    bicne   r9, r9, r1
    stmdb	sp!, {r0}
    blne    fiq_dispatch_handler
    ldmia	sp!, {r0}
    cmp     r9, #0
    beq     fiq_handler_exit
    add     r0, r0, #1
    cmp		r0, #INT_STATUS_NBITS     		@check valid status bits

    bne     fiq_handler_check_status

fiq_handler_exit:							@Exit point of FIQ
	ldmia	sp!, {r0-r7, lr}				@restore the context
	subs	pc, lr, #4						@return to the interrupt point


@@@@@@@@@@@@@@@@@@@@@   Entry of Undefined handler   @@@@@@@@@@@@@@@@@@@@@@@@@
.global undefined_handler
undefined_handler:
	@when undefined error is accured, CPU will arrive here 
	@save r0-r12,LR, SPSR,CPSR into the current stack ,and stack of undefined module
	stmdb	sp!, {r0-r12, lr}				@backup the context
	mrs		r0,	 spsr
	ldr		r1,  =UNDEF_ERROR
	stmdb	sp!, {r0-r1}

	@ save  current sp into R6,in order to get the saving in previou module
	mov		r6,	  sp

	@change CPU state into previous
	and		r0,	 r0,	 #0x0000000ff
	and		r1,  r1,	 #0x0ffffff00
	orr		r0,	 r0, r1
	msr		cpsr,	r0

	@ save previoue SP into stack of undefined module
	@ and us stack of undefined module as current stack
	str		sp,		[r6, #-4]!
	mov		sp,		r6

	@call c function handle with previous scene
	mov		r0,		sp
	ldr		pc,		=system_error_check
	
	ldmia	sp!, {r0-r12, lr}				@restore the context
	subs	pc, lr, #4						@return to the interrupt point

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@@@@@@@@@@@@@@@@@@@   Entry of Prefech handler  @@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global prefetch_handler
prefetch_handler:
	stmdb	sp!, {r0-r12, lr}				@backup the context
	mrs		r0,	 spsr
	ldr		r1,  =PREF_ERROR
	stmdb	sp!, {r0-r1}

	@ save  current sp into R6,in order to get the saving in previou module
	mov		r6,	  sp

	@change CPU state into previous
	and		r0,	 r0,	 #0x0000000ff
	and		r1,  r1,	 #0x0ffffff00
	orr		r0,	 r0, r1
	msr		cpsr,	r0

	@ save previoue SP into stack of undefined module
	@ and us stack of undefined module as current stack
	str		sp,		[r6, #-4]!
	mov		sp,		r6

	@call c function handle with previous scene
	mov		r0,		sp
	ldr		pc,		=system_error_check
	
	ldmia	sp!, {r0-r12, lr}				@restore the context
	subs	pc, lr, #4						@return to the interrupt point

	
@@@@@@@@@@@@@@@@@@@@@   Entry of Data abort handler  @@@@@@@@@@@@@@@@@@@@@@@@@
.global abort_handler
abort_handler:
	stmdb	sp!, {r0-r12, lr}	@backup the context 14 words
	mrs		r0,	 spsr
	ldr		r1,  =ABT_ERROR
	stmdb	sp!, {r0-r1}        @2words


	@ save  current sp into R6,in order to get the saving in previou module
	mov		r6,	  sp
	add     sp,   sp,   #64

	@change CPU state into previous
	and		r0,	 r0,	 #0x0000000ff
	and		r1,  r1,	 #0x0ffffff00
	orr		r0,	 r0, r1
	msr		cpsr,	r0

	@ save previoue SP into stack of undefined module
	@ and us stack of undefined module as current stack
	str		sp,		[r6, #-4]!
	mov		sp,		r6

	@call c function handle with previous scene
	mov		r0,		r6
	ldr		pc,		=system_error_check

	ldmia	sp!, {r0-r12, lr}				@restore the context
	subs	pc, lr, #4						@return to the interrupt point

@@@@@@@@@@@@@@@@@@@@@   Entry of intr_mask_all  @@@@@@@@@@@@@@@@@@@@@@@@@
.global intr_mask_all
intr_mask_all:
	stmdb	sp!, {r0}
	mrs		r0, cpsr
	orr		r0, r0, #0x00c0
	msr		cpsr, r0
	ldmia	sp!, {r0}
	mov		pc, lr
	
@@@@@@@@@@@@@@@@@@@@@   Entry of intr_unmask_all @@@@@@@@@@@@@@@@@@@@@@@@@
.global intr_unmask_all
intr_unmask_all:
	stmdb	sp!, {r0}
	mrs		r0, cpsr
	bic		r0, r0, #0x00c0
	msr		cpsr, r0
	ldmia	sp!, {r0}
	mov		pc, lr

@@@@@@@@@@@@@@@@@@@@@   Entry of intr_mask_irq_fiq @@@@@@@@@@@@@@@@@@@@@@@@@
.global intr_mask_irq_fiq
intr_mask_irq_fiq:
	stmdb	sp!, {r0,r1}
	
	ldr		r0, =IRQINT_MASK_REG	
	ldr		r1, =0x0
	str		r1, [r0]
	
	@disable all fiq interrupt
	ldr		r0, =FRQINT_MASK_REG
	ldr		r1, =0x0
	str		r1, [r0]

	ldmia	sp!, {r0,r1}
	mov		pc, lr	
	
