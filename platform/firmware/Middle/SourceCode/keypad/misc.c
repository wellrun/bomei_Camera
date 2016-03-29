/**
 * @FILENAME: misc.c
 * @BRIEF misc driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-20
 * @VERSION 1.0
 * @REF
 */

 
#include "anyka_types.h"
#include "gbl_global.h"
#include "misc.h"
#include "Eng_Debug.h"
#include "Fwl_Initialize.h"
#include "gpio_config.h"
#include "drv_gpio.h"

static T_BOOL m_bKeyLightLock = AK_FALSE;
//static T_BOOL m_bModuleWakeup = AK_FALSE;


/**
 * @brief: open keypad light
 * @author: guoshaofeng
 * @date 2007-04-20
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID open_keypadlight(T_VOID)
{	
	gpio_set_pin_level(GPIO_KEYPAD_BL, 1);
}

/**
 * @brief: close keypad light
 * @author: guoshaofeng
 * @date 2007-04-20
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID close_keypadlight(T_VOID)
{
	gpio_set_pin_level(GPIO_KEYPAD_BL, 0);
}

/**
 * @brief: recover_keypad_light
 * @author: guoshaofeng
 * @date 2007-04-20
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID recover_keypad_light(T_VOID)
{
	if (m_bKeyLightLock)
	{
		m_bKeyLightLock = AK_FALSE;
		open_keypadlight();
	}
}

/**
 * @brief: lock_keypad_light
 * @author: guoshaofeng
 * @date 2007-04-20
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID lock_keypad_light(T_VOID)
{
	m_bKeyLightLock = AK_TRUE;
}

