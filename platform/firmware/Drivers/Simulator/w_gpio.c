/**
 * @file w_gpio.c
 * @brief GPIO operation funcitons for PC version
 * This file provides GPIO operation functions for PC version
 * All of these operations are empty funcitons
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-09-22
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#ifdef OS_WIN32

#include "anyka_types.h"
#include "drv_gpio.h"
/**
 * @brief: init gpio 
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: T_VOID
 * @return T_VOID
 * @retval
 */


T_VOID gpio_init( T_VOID )
{
}

/**
 * @brief: Set gpio output level
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: level: 0 or 1.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level )
{
}

/**
 * @brief: Get gpio input level
 * @author: Miaobaoli
 * @param: pin: gpio pin ID.
 * @date 2005-09-22
 * @return T_U8
 * @retval: pin level; 1: high; 0: low;
 */
T_U8 gpio_get_pin_level( T_U32 pin )
{
	return 1;
}

/**
 * @brief: Set gpio direction
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: dir: 0 means input; 1 means output;
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir )
{
}

/**
 * @brief: gpio interrupt control
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: enable: 1 means enable interrupt. 0 means disable interrupt.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_int_control( T_U32 pin, T_U8 enable )
{
}

/**
 * @brief: set gpio interrupt polarity.
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: polarity: 1 means active high interrupt. 0 means active low interrupt.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_int_p( T_U32 pin, T_U8 polarity )
{
}
	
/**
 * @brief: set gpio interrupt mode (edge or level).
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: mode: 0 means level interrupt. 1 means edge interrupt.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_int_mode( T_U32 pin, T_U8 mode )
{
}

/**
 * @brief: Register one gpio interrupt callback function.
 * @author: Miaobaoli
 * @date 2005-09-22
 * @param: pin: gpio pin ID.
 * @param: polarity: 1 means active high interrupt. 0 means active low interrupt.
 * @param: enable: Initial interrupt state--enable or disable.
 * @param: callback: gpio interrupt callback function.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_register_int_callback( T_U32 pin, T_U8 polarity, T_U8 enable, T_fGPIO_CALLBACK callback )
{
}

T_U8 gpio_pin_get_ActiveLevel(T_U8 pin)
{
	return 0xff;
}

T_VOID get_wGpio_Mask(T_U32 *wgpio_mask)
{
    *wgpio_mask = 0xff;
}
#endif
