/**
 * @file interrupt.h
 * @brief: This file describe how to control the AK3223M interrupt issues.
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
 */
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


/** @defgroup Interrupt  
 *  @ingroup M3PLATFORM
 */
/*@{*/
#include "anyka_cpu.h"
#include "anyka_types.h"


/** @{@name Interrupt Operator Define
 *  Define the macros to operate interrupt register, to enable/disable interrupt
 */
 /**IRQ mode*/
#define INTR_ENABLE(int_bits)   \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)IRQINT_MASK_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)

#define INTR_DISABLE(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)IRQINT_MASK_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)

 /**IRQ Level2 mode*/
#define INTR_ENABLE_L2(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)INT_SYS_MODULE_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)


#define INTR_DISABLE_L2(int_bits) \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)INT_SYS_MODULE_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)


/** FIQ mode*/
#define FIQ_INTR_ENABLE(int_bits)   \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)FRQINT_MASK_REG |= (int_bits); \
        irq_unmask(); \
    }while(0)

#define FIQ_INTR_DISABLE(int_bits)  \
    do{ \
        irq_mask(); \
        *(volatile unsigned long *)FRQINT_MASK_REG &= ~(int_bits); \
        irq_unmask(); \
    }while(0)



/** @} */

typedef T_BOOL (*T_INTR_HANDLER)(T_VOID);

typedef enum 
{
    INT_VECTOR_MIN=100,
    INT_VECTOR_LCD, 
    INT_VECTOR_CAMERA,
    INT_VECTOR_MOTION,
    INT_VECTOR_JPEG,
    INT_VECTOR_DAC,
    INT_VECTOR_ADC,
    INT_VECTOR_L2,
    INT_VECTOR_UART3,
    INT_VECTOR_UART2,
    INT_VECTOR_UART1,
    INT_VECTOR_SPI,    
    INT_VECTOR_MAC,    
    INT_VECTOR_IRDA,    
    INT_VECTOR_MCI2,
    INT_VECTOR_MCI1,    
    INT_VECTOR_USB,
    INT_VECTOR_USB_DMA,
    INT_VECTOR_GPIO,
    INT_VECTOR_TIMER,
    INT_VECTOR_RTC,
    INT_VECTOR_WGPIO,
    INT_VECTOR_MAX
}INT_VECTOR;

/**
 * @brief: interrupt init, called before int_register_irq()
 */
T_VOID interrupt_init(T_VOID);

/**
 * @brief: register irq interrupt
 */
T_BOOL int_register_irq(INT_VECTOR v, T_INTR_HANDLER handler);


#endif

