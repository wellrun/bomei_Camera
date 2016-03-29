/**
 * @file gpio.c
 * @brief gpio function file
 * This file provides gpio APIs: initialization, set gpio, get gpio,
 * gpio interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author tangjianlong
 * @date 2008-01-10
 * @version 1.0
 * @ref anyka technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h" 
#include "drv_api.h"
#include "interrupt.h"
#include "gpio.h"
#include "drv_gpio.h"

static T_BOOL gpio_interrupt_handler(T_VOID);

static T_fGPIO_CALLBACK m_fGPIOCallback[GPIO_NUMBER];

static T_U32 gpio_pin_dir_reg[] = {GPIO_DIR_REG1,       GPIO_DIR_REG2,      GPIO_DIR_REG3};
static T_U32 gpio_pin_in_reg[]  = {GPIO_IN_REG1,        GPIO_IN_REG2,       GPIO_IN_REG3};
static T_U32 gpio_pin_out_reg[] = {GPIO_OUT_REG1,       GPIO_OUT_REG2,      GPIO_OUT_REG3};
static T_U32 gpio_pin_inte_reg[]= {GPIO_INT_EN1,        GPIO_INT_EN2,       GPIO_INT_EN3};
static T_U32 gpio_pin_intp_reg[]= {GPIO_INT_LEVEL_REG1, GPIO_INT_LEVEL_REG2, GPIO_INT_LEVEL_REG3};
static T_U32 gpio_pin_intm_reg[]= {GPIO_INT_MODE_REG1, GPIO_INT_MODE_REG2, GPIO_INT_MODE_REG3};

static volatile T_U32 gpio_pin_inte[4] = {0};
static volatile T_U32 gpio_pin_intp[4] = {0};
static volatile T_U8  usb_vbus_level = GPIO_LEVEL_LOW;  //special gpio 51

#define USB_DETECT_GPIO     89

/* GPIO int before call back register, must be fatal error*/
static T_VOID DummyGPIOCallback(T_U32 pin, T_U8 polarity)
{
    gpio_int_control(pin, 0);
}

T_BOOL gpio_assert_legal(T_U32 pin)
{
    if((pin == INVALID_GPIO) || (pin >= GPIO_NUMBER))
    {
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }
}

/**
 * @brief: Init gpio.
 * @author tangjianlong
 * @date 2008-01-10
 * @return T_VOID
 * @retval
 */
T_VOID gpio_init( T_VOID )
{
    T_U32 pin;
    
    //clean gpio callback function array.
    for( pin=0; pin<GPIO_NUMBER; pin++ )
    {
        m_fGPIOCallback[ pin ] = DummyGPIOCallback;
    }

    //disable gpio int before enable its interrupt
    gpio_int_disableall();
    
    //enable GPIO interrupt here.
    int_register_irq(INT_VECTOR_GPIO, gpio_interrupt_handler);
    //enable level2 interrupt
    INTR_ENABLE_L2(IRQ_MASK_GPIO_BIT);
    
    INTR_DISABLE_L2(IRQ_MASK_WGPIO_BIT);    

}

/**
 * @brief: Set gpio output level
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: level: 0 or 1.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level )
{
    T_U32 index, residual;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }

    index = pin / 32;
    residual = pin % 32;
    
    irq_mask();
    if(level)
        *(volatile T_U32*)gpio_pin_out_reg[index] |= (1 << residual);
    else
        *(volatile T_U32*)gpio_pin_out_reg[index] &= ~(1 << residual);
    irq_unmask();
}


static T_U8 gpio_get_vbus_level()
{
    T_U32 reg;
    
    reg = REG32(MUL_FUN_CTL_REG3);
    if(reg & (1U << 31))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief: Get gpio input level
 * @author: tangjianlong
 * @param: pin: gpio pin ID.
 * @date 2008-01-10
 * @return T_U8
 * @retval: pin level; 1: high; 0: low;
 */
T_U8 gpio_get_pin_level( T_U32 pin )
{
    T_U32 index, level = 0, residual;   

    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return 0xff;
    }

    if (pin == USB_DETECT_GPIO)
    {
        return gpio_get_vbus_level();
    }

    index = pin / 32;
    residual = pin % 32;

    irq_mask();
    
    if(REG32(gpio_pin_in_reg[index]) & (1 << residual))
        level = 1;
    else
        level = 0;
        
    irq_unmask();
    return level;
}

/**
 * @brief: Set gpio direction
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: dir: 0 means input; 1 means output;
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir )
{
    T_U32 index, residual, i;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }

    index = pin / 32;
    residual = pin % 32;
    
    if(dir == 0)//input mode
    {
        //when set as input mode, check if it's necessary to enable IE attr
        if(0)//(i = gpio_pin_check(pin)) != INVALID_GPIO)
        {
            gpio_set_pin_attribute(pin, GPIO_ATTR_IE, AK_TRUE);
        }
        *(volatile T_U32*)gpio_pin_dir_reg[index] |= (1 << residual);
    }
    else{
        *(volatile T_U32*)gpio_pin_dir_reg[index] &= ~(1 << residual);
    }   
}

/**
 * @brief: gpio interrupt control
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: enable: 1 means enable interrupt. 0 means disable interrupt.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_int_control( T_U32 pin, T_U8 enable )
{
    T_U32 index, residual;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }

    index = pin / 32;
    residual = pin % 32;
    
    irq_mask();
    
    if(enable)
        gpio_pin_inte[index] |= 1 << residual;
    else
        gpio_pin_inte[index] &= ~(1 << residual);

    if(enable)
        *(volatile T_U32*)gpio_pin_inte_reg[index] |= (1 << residual);
    else
        *(volatile T_U32*)gpio_pin_inte_reg[index] &= ~(1 << residual);

    irq_unmask();
}


T_VOID gpio_int_disableall()
{
    *(volatile T_U32*)gpio_pin_inte_reg[0] = 0;
    *(volatile T_U32*)gpio_pin_inte_reg[1] = 0;
    *(volatile T_U32*)gpio_pin_inte_reg[2] = 0;
}

T_VOID gpio_int_restoreall()
{
    *(volatile T_U32*)gpio_pin_inte_reg[0] = gpio_pin_inte[0];
    *(volatile T_U32*)gpio_pin_inte_reg[1] = gpio_pin_inte[1];
    *(volatile T_U32*)gpio_pin_inte_reg[2] = gpio_pin_inte[2];
}

/*

*/
T_VOID gpio_set_int_mode( T_U32 pin, T_U8 mode)
{
    T_U32 index,  residual;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }
    
    irq_mask();
    index = pin / 32;
    residual = pin % 32;

    if (GPIO_LEVEL_INTERRUPT == mode)
        *(volatile T_U32*)gpio_pin_intm_reg[index] &= ~(1 << residual);
    else
        *(volatile T_U32*)gpio_pin_intm_reg[index] |= (1 << residual);
    irq_unmask();
}

/*

*/
T_VOID gpio_set_int_p( T_U32 pin, T_U8 polarity )
{
    T_U32 index,  residual;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }
    
    irq_mask();
    index = pin / 32;
    residual = pin % 32;

    if(polarity)//high level triggered
        gpio_pin_intp[index] |= (1 << residual); 
    else
        gpio_pin_intp[index] &= ~(1 << residual);

    if(polarity)
        *(volatile T_U32*)gpio_pin_intp_reg[index] &= ~(1 << residual);
    else
        *(volatile T_U32*)gpio_pin_intp_reg[index] |= (1 << residual);
    irq_unmask();
}
    

/**
 * @brief: Register one gpio interrupt callback function.
 * @author: tangjianlong
 * @date 2008-01-10
 * @param: pin: gpio pin ID.
 * @param: polarity: 1 means active high interrupt. 0 means active low interrupt.
 * @param: enable: Initial interrupt state--enable or disable.
 * @param: callback: gpio interrupt callback function.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_register_int_callback( T_U32 pin, T_U8 polarity, T_U8 enable, T_fGPIO_CALLBACK callback )
{
    if((AK_FALSE == gpio_assert_legal(pin)) || (AK_NULL == callback))
    {
        akprintf(C1, M_DRVSYS, "gpio_register_int_callback param error\n");
        return;
    }

    m_fGPIOCallback[pin] = callback;
//    gpio_set_int_mode(pin, polarity); //platform isn't need set
    gpio_set_int_p(pin, polarity);
    gpio_int_control( pin, enable);

}

static T_BOOL gpio_interrupt_handler(T_VOID)
{       
    T_U32 i, pin, inte, intp;
    T_U32 index, residual;   

    //check if gpio generates interrupt
    if (!(*(volatile T_U32 *)INT_SYS_MODULE_REG & INT_STATUS_GPIO_BIT))
        return AK_FALSE;

    for(i = 0; i <= USB_DETECT_GPIO; i++)
    {
        index = i / 32;
        residual = i % 32;

        inte = (gpio_pin_inte[index] >> residual) & 0x1;
        intp = (gpio_pin_intp[index] >> residual) & 0x1;

        if(inte)
        {            
            if(gpio_get_pin_level(i) == intp)
            {
                goto do_irq;
            }
        }
    }

    if (i > USB_DETECT_GPIO)
    {
        //akprintf(C3, M_DRVSYS, "gpio jitter!\n");
        return AK_TRUE;
    }

do_irq: 
    //pin number
    pin = i;       
    m_fGPIOCallback[pin](pin, intp);

    return AK_TRUE;
}


