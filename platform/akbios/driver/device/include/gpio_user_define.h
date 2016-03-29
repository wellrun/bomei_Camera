/**
 * @FILENAME: gpio_user_define.h
 * @BRIEF LCD backlight driver head file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-19
 * @VERSION 1.0
 * @REF
 */

#ifndef __GPIO_USER_DEFINE_H__
#define __GPIO_USER_DEFINE_H__

#ifdef CHIP_AK3771
#include "gpio_config_CI3771.h"
#elif CHIP_AK3753
#include "gpio_config_CI3753.h"
#elif CHIP_AK3750
#include "gpio_config_CI3750.h"
#endif

/** @{@name GPIO Event Define
 *	Define the event caused by GPIO
 */
#define VME_EVT_PINIO_FLIP              (VME_EVT_PINIO + 0) ///< digital input change detected on pin IRQ_FLIP
#define VME_EVT_PINIO_CHARGE_DETECT     (VME_EVT_PINIO + 1) ///< digital input change detected on pin IRQ_CHARGE_DETECT
/** @} */


/**
 * @brief: initialize system gpio pin
 * @note: this function should be called only once! so we usually place it in bios.
 *
 * @author YiRuoxiang
 * @date 2006-09-20
 * @return T_VOID
 * @retval
 */
T_VOID gpio_pin_init(T_VOID);

/**
 * @BRIEF get wakeup gpio mask of set
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM[out] T_U32 * wgpio_mask: pointer of data mask of get
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID get_wGpio_Mask(T_U32 *wgpio_mask);

/**
 * @brief: gpio set expand callback
 * @author: 
 * @date 2007-05-07
 * @param[in] pin: gpio pin ID.
 * @param[in] level: 1 means hight level. 0 means low level.
 * @return T_VOID
 * @retval
 */
T_VOID gpio_set_expand_callback(T_U32 pin, T_U8 level);

#endif  //#ifndef __GPIO_USER_DEFINE_H__
