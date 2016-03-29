/**
 * @file usb_host_drv.h
 * @brief  frameworks of usb driver.
 *
 * This file describe driver of usb in host mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __USB_HOST_DRV_H__
#define __USB_HOST_DRV_H__

#include "anyka_types.h"
#include "anyka_cpu.h"

#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Usb_Host Usb Host group
 *	@ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup Host_Driver USB Host Driver
 *  @ingroup Usb_Host
 */
/*@{*/

#define USB_HOST_OUT_INDEX EP2_INDEX
#define USB_HOST_IN_INDEX EP1_INDEX

typedef enum
{
    EP0_INDEX = 0,  ///< EP0
    EP1_INDEX = 1,  ///< EP1
    EP2_INDEX = 2,  ///< EP2
    EP3_INDEX = 3,   ///< EP3
    EP4_INDEX = 4   ///< EP4
}EP_INDEX;

typedef enum
{
    USB_HOST_CONNECT = 0,
    USB_HOST_DISCONNECT,
    USB_HOST_UNDEFINE
}
E_USB_HOST_COMMON_INTR;

#define USB_HOST_TRANS_COMPLETE 0
#define USB_HOST_TRANS_ERROR    1


extern T_U8 g_UsbBulkinIndex;
extern T_U8 g_UsbBulkoutIndex;


/**
 *	usb host common usb interrupt call back
 */
typedef T_VOID (*T_fUHOST_COMMON_INTR_CALLBACK)(T_VOID);

/**
 *	usb host transfer end call back
 */
typedef T_VOID (*T_fUHOST_TRANS_CALLBACK)(T_U8 trans_state, T_U32 trans_len);


/**
 * @brief   enable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  mode [in] T_U32  full speed or high speed
 * @return  T_VOID
 */
T_VOID usb_host_device_enable(T_U32 mode);

/**
 * @brief   disable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_device_disable(T_VOID);

/**
 * @brief   set callback func for common interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param intr_type interrupt type
 * @param callback callback function
 * @return  T_VOID
 */
T_VOID usb_host_set_common_intr_callback(E_USB_HOST_COMMON_INTR intr_type, T_fUHOST_COMMON_INTR_CALLBACK callback);

/**
 * @brief   set callback func for transfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param ctrl_cbk callback function for control transfer
 * @param trans_cbk callback function for other transfer
 * @return  T_VOID
 */
T_VOID usb_host_set_trans_callback(T_fUHOST_TRANS_CALLBACK ctrl_cbk, T_fUHOST_TRANS_CALLBACK trans_cbk);

/**
 * @brief   set faddr to the new address send to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param address  [in]  usb device address.
 * @return  T_VOID
 */
T_VOID usb_host_set_address(T_U8 address);


/**
 * @brief   config usb host endpoint through device ep descriptor.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  ep [in]  ep description.
 * @return  T_VOID
 */
T_VOID usb_host_set_ep(T_USB_ENDPOINT_DESCRIPTOR ep);

/**
 * @brief   open or close sof interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  enable [in]  open sof interrupt or not.
 * @return  T_VOID
 */
T_VOID usb_host_sof_intr(T_BOOL enable);


/**
 * @brief   send reset signal to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_reset(T_VOID);


/**
   @brief   sent suspend signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_suspend(T_VOID);

/**
   @brief   sent resume signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_resume(T_VOID);


/**
   @brief   start control tranfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param dev_req [in] device request
 * @param data [in/out] data buffer
 * @param len [in] buffer length
 * @return  T_BOOL
 */
T_BOOL usb_host_ctrl_tranfer(T_UsbDevReq dev_req, T_U8 *data, T_U32 len);


/**
   * @brief   start bulk in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [out]  usb data buffer.
   * @param  len [in]  length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_bulk_in(T_U8 *data, T_U32 len);

/**
   * @brief   start bulk out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [in]  usb data buffer.
   * @param  len [in] len length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_bulk_out(T_U8 *data, T_U32 len);

/**
   * @brief   start iso in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data  [out] usb data buffer.
   * @param  packet_num [in]  number of packet to receive
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_iso_in(T_U8 *data, T_U32 packet_num);

/**
   * @brief   start iso out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data  [in] usb data buffer.
   * @param  len [in]  length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_iso_out(T_U8 *data, T_U32 len);

/**
   * @brief   start interrupt in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [out]  usb data buffer.
   * @param  len [in]  length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_intr_in(T_U8 *data, T_U32 len);

/**
   * @brief   start interrupt out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [in]  usb data buffer.
   * @param  len [in]  length
   * @return  T_U32 acctual read bytes
 */
T_BOOL usb_host_intr_out(T_U8 *data, T_U32 len);

/**
 * @brief   reset data toggle.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_clear_data_toggle(T_U8 EP_index);
/**
 * @brief   flush usb fifo.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_host_flush_fifo(T_U8 EP_index);



//********************************************************************
/*@}*/

#ifdef __cplusplus
}
#endif

#endif
