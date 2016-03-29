/**@file hal_usb_s_anyka.h
 * @brief provide operations of how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_S_ANYKA_H__
#define __HAL_USB_S_ANYKA_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup USB USB group
 *  @ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup USB_anyka USB_anyka group
 *  @ingroup USB
 */
/*@{*/

typedef T_VOID (*T_fUSBANYKA_RECEIVECALLBACK)(T_VOID);
typedef T_VOID (*T_fUSBANYKA_RECEIVEOKCALLBACK)(T_VOID);
typedef T_VOID (*T_fUSBANYKA_SENDFINISHCALLBACK)(T_VOID);


//********************************************************************

/**
 * @brief   usb anyka init.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  T_VOID
 */
T_VOID usbanyka_init(T_VOID); 

/**
 * @brief   enable anyka device in usb slave mode
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  T_BOOL
 * @retval  AK_TRUE means sucessful
 */
T_BOOL usbanyka_enable(T_VOID);

/**
 * @brief   disable anyka device in usb slave mode
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  T_VOID
 */
T_VOID usbanyka_disable(T_VOID);

/**
 * @brief   usb slave anyka set callback function.
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param[in] receive_func T_fUSBCDC_RECEIVECALLBACK : notify to receive data.
 * @param[in] receiveok_func T_fUSBANYKA_RECEIVEOKCALLBACK : notify receive data ok.
 * @param[in] sendfinish_func T_fUSBCDC_SENDFINISHCALLBACK : notify send finish   
 * @return  T_VOID
 */
T_VOID usbanyka_set_callback(T_fUSBANYKA_RECEIVECALLBACK receive_func, T_fUSBANYKA_RECEIVEOKCALLBACK receiveok_func, T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func);

/**
 * @brief   write data to usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] data data to write
 * @param   [in] data_len data length.
 * @return  T_S32
 * @retval  -1 write error, try again
 * @retval  other actual data length to write
 */
T_S32 usbanyka_write(T_U8 *data, T_U32 data_len);

/**
 * @brief   read data from usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [out] *data temp read data
 * @param   [in] data_len data length.
 * @return  T_S32
 * @retval  -1 read error, try again
 * @retval  other actual data length from read
 */
T_S32 usbanyka_read(T_U8 *data, T_U32 data_len);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
