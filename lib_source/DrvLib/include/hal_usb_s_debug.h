/**@file hal_usb_s_debug.h
 * @brief provide operations of how to use usb device of debug.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_SLAVE_DEBUG_H__
#define __HAL_USB_SLAVE_DEBUG_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"


#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_debug USB_debug group
 *  @ingroup USB
 */
/*@{*/

//********************************************************************
/**
 * @brief   enable usb debug in usb slave mode
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL usbdebug_enable(T_VOID);

/**
 * @brief   disable usb debug in usb slave mode
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @return  T_VOID
 */
T_VOID usbdebug_disable(T_VOID);

/**
 * @brief print to usb
 *
 * @author zhaojiahuan
 * @date   2006-11-04
 * @param[in] str string to be print
 * @param[in] len string length
 * @return  T_VOID
 */
void usbdebug_printf(T_U8 *str, T_U32 len);


/**
 * @brief   get string from usb
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param[out] str buffer to store input string 
 * @param[in] len buffer size
 * @return  T_U32
 */
T_U32 usbdebug_getstring(T_U8 *str, T_U32 len);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
