/**
 * @filename hal_usb_s_cdc.h
 * @brief: how to use usb cdc.
 *
 * This file describe frameworks of usb cdc driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 * @ref
 */

#ifndef __HAL_USB_S_CDC_H__
#define __HAL_USB_S_CDC_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"


#ifdef __cplusplus
extern "C" {
#endif


//********************************************************************

typedef T_VOID (*T_fUSBCDC_RECEIVECALLBACK)(T_VOID);
typedef T_VOID (*T_fUSBCDC_SENDFINISHCALLBACK)(T_VOID);



/**
 * @brief   usb cdc init.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_VOID.
 * @return  T_VOID
 */
T_VOID usbcdc_init(T_VOID); 

/**
 * @brief   usb slave enable cdc device function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_VOID.
 * @return  T_VOID
 */
T_BOOL usbcdc_enable(T_VOID);

/**
 * @brief   usb slave disnable cdc device function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_VOID.
 * @return  T_VOID
 */
T_VOID usbcdc_disable(T_VOID);


/**
 * @brief   usb slave cdc set callback function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_fUSBCDC_RECEIVECALLBACK : notify to receive data.
                 T_fUSBCDC_SENDFINISHCALLBACK : notify send finish   
 * @return  T_VOID
 */
T_VOID usbcdc_set_callback(T_fUSBCDC_RECEIVECALLBACK receive_func, T_fUSBCDC_SENDFINISHCALLBACK sendfinish_func);


/**
 * @brief set pool to receive data
 * 
 * @author Qinxiaojun
 * @date 2007-9-17
 * @param[in] uart_id: UART ID
              T_U8 *pool: buffer to recieve data,as big as you can
              T_U32 poollength : buffer length
 *
 * @return T_VOID
 * @remarks the data received from uart will be stored in the pool waiting to be fetched by
 *  uart_read().
 */
T_VOID usbcdc_set_datapool(T_U8 *pool, T_U32 poollength);

/**
 * @brief   write data to usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_U8 *data: data to write
                 T_U32 data_len: data length.
 * @return  -1: write error, try again
            other: actual data length to write
 */
T_S32 usbcdc_write(T_U8 *data, T_U32 data_len);

/**
 * @brief   read data from usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_U8 *data: temp read data
                 T_U32 data_len: data length.
 * @return  -1: read error, try again
            other: actual data length from read
 */

T_S32 usbcdc_read(T_U8 *data, T_U32 data_len);



//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
