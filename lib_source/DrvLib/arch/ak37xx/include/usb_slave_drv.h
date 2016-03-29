/**
 * @file usb_slave_drv.h
 * @brief: frameworks of usb driver.
 *
 * This file describe driver of usb in slave mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __USB_SLAVE_DRV_H__
#define __USB_SLAVE_DRV_H__

#include "anyka_types.h"
#include "anyka_cpu.h"

#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_EP_IN_TYPE            0x1
#define USB_EP_OUT_TYPE           0x0

#define EP0_INTR            0x000000001
#define EP1_INTR            0x000000002
#define EP2_INTR            0x000000004
#define EP3_INTR            0x000000008
#define EP4_INTR            0x000000010
#define EP1_DMA_INTR        0x000000020
#define EP2_DMA_INTR        0x000000040
#define EP3_DMA_INTR        0x000000080
#define EP4_DMA_INTR        0x000000100
#define USB_INTR            0x000000200
#define EP_UNKNOWN          0x0

#define USB_DMA_SUPPORT           0x01
#define USB_DMA_UNSUPPORT         0x0

/**
 * @brief EP Index
 
 *   define ep index
 */
typedef enum
{
    EP0_INDEX = 0,  ///< EP0
    EP1_INDEX = 1,  ///< EP1
    EP2_INDEX = 2,  ///< EP2
    EP3_INDEX = 3,   ///< EP3
    EP4_INDEX = 4   ///< EP4
}EP_INDEX;

/**
 * @brief Control Transfer Stage
 
 *   define control transfer stage
 */
typedef enum
{
    CTRL_STAGE_IDLE = 0,    ///< idle stage
    CTRL_STAGE_SETUP,       ///< setup stage
    CTRL_STAGE_DATA_IN,     ///< data in stage
    CTRL_STAGE_DATA_OUT,    ///< data out stage
    CTRL_STAGE_STATUS       ///< status stage
}
E_CTRL_TRANS_STAGE;

/**
 * @brief Request Type
 
 *   define control transfer request type
 */
#define REQUEST_STANDARD    0   ///< standard request
#define REQUEST_CLASS       1   ///< class request
#define REQUEST_VENDOR      2   ///< vendor request


/**
 * @brief Control Tranfer Struct
 
 *   define control transfer struct
 */
typedef struct tagCONTROL_TRANS
{
    E_CTRL_TRANS_STAGE stage;   ///< stage
    T_UsbDevReq dev_req;        ///< request
    T_U8 *buffer;               ///< buffer
    T_U32 buf_len;              ///< buffer length
    T_U32 data_len;             ///< data length
}
T_CONTROL_TRANS;

/**
 *      rx notify callback handler
 */
typedef T_VOID (*T_fUSB_NOTIFY_RX_CALLBACK)(T_VOID);

/**
 *      rx finish callback handler
 */
typedef T_VOID (*T_fUSB_RX_FINISH_CALLBACK)(T_VOID);

/**
 *      tx finish callback handler
 */
typedef T_VOID (*T_fUSB_TX_FINISH_CALLBACK)(T_VOID);

/**
 *      reset event callback handler
 */
typedef T_VOID (*T_fUSB_RESET_CALLBACK)(T_U32 mode);

/**
 *      suspend event callback handler
 */
typedef T_VOID (*T_fUSB_SUSPEND_CALLBACK)(T_VOID);

/**
 *      resume event callback handler
 */
typedef T_VOID (*T_fUSB_RESUME_CALLBACK)(T_VOID);

/**
 *      config ok event callback handler
 */
typedef T_VOID (*T_fUSB_CONFIGOK_CALLBACK)(T_VOID);

/**
 *      control tranfer callback handler
 */
typedef T_BOOL (*T_fUSB_CONTROL_CALLBACK)(T_CONTROL_TRANS *pTrans);

//********************************************************************
/**
 * @brief   enable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] T_U32 usb mode
 * @return  T_VOID
 */
T_VOID usb_slave_device_enable(T_U32 mode);

/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_slave_device_disable(T_VOID);

/**
 * @brief   initialize usb slave global variables, and set buffer for control tranfer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param buffer [IN] buffer to be set for control transfer
 * @param buf_len [IN] buffer length
 * @return  T_BOOL
 * @retval AK_TRUE init successfully
 * @retval AK_FALSE init fail
 */
T_BOOL usb_slave_init(T_U8 *buffer, T_U32 buf_len);

/**
 * @brief  free usb slave global variables,L2 buffer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_slave_free(T_VOID);

/**
 * @brief   set control transfer call back function
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param type [IN] request type, must be one of (REQUEST_STANDARD, REQUEST_CLASS, REQUEST_VENDOR)
 * @param callback [In] callback function
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_ctrl_callback(T_U8 type, T_fUSB_CONTROL_CALLBACK callback);

/**
 * @brief   initialize usb slave register.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param reset_callback [IN] callback function for reset interrupt
 * @param suspend_callback [IN] callback function for suspend interrupt
 * @param resume_callback [IN] callback function for resume interrupt
 * @param configok_callback [IN] callback function for config ok event
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_callback(T_fUSB_RESET_CALLBACK reset_callback, T_fUSB_SUSPEND_CALLBACK suspend_callback, T_fUSB_RESUME_CALLBACK resume_callback, T_fUSB_CONFIGOK_CALLBACK configok_callback);


/**
 * @brief   Register a callback function to notify tx send data finish.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  callback_func [in]  T_fUSB_TX_FINISH_CALLBACK can be null
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_tx_callback(EP_INDEX EP_index, T_fUSB_TX_FINISH_CALLBACK callback_func);


/**
 * @brief   Register a callback function to notify rx receive data finish and rx have data.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  notify_rx [in] rx notify callbakc function, can be null
 * @param  rx_finish [in] rx finish callbakc function, can be null
 * @return  T_BOOL
 * @retval AK_TRUE callback function set successfully
 * @retval AK_FALSE fail to set callback function
 */
T_BOOL usb_slave_set_rx_callback(EP_INDEX EP_index, T_fUSB_NOTIFY_RX_CALLBACK notify_rx, T_fUSB_RX_FINISH_CALLBACK rx_finish);

/**
 * @brief   write usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  data [in] usb data buffer.
 * @param  count [in] count to be send.
 * @return  T_U32 data in count
 */
T_U32 usb_slave_data_in(EP_INDEX EP_index, T_U8 *data, T_U32 count);

/**
 * @brief   read usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  pBuf [out] usb data buffer.
 * @param  count [in] count to be read
 * @return T_U32 data out count
 */
T_U32  usb_slave_data_out(EP_INDEX EP_index, T_VOID *pBuf, T_U32 count);

/**
 * @brief   write usb address.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  address [in]  usb device address.
 * @return  T_VOID
 */
T_VOID usb_slave_set_address(T_U8 address);

/**
 * @brief  stall ep
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_VOID usb_slave_ep_stall(T_U8 EP_index);

/**
 * @brief  clear stall
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_VOID usb_slave_ep_clr_stall(T_U8 EP_index);


/**
 * @brief   read data count of usb end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index [in] usb end point.
 * @param cnt  [out] cnt data count.
 * @return  T_VOID
 */
T_VOID usb_slave_read_ep_cnt(T_U8 EP_index, T_U32 *cnt);


/**
 * @brief   set usb controller to enter test mode
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  testmode [in] T_U8 test mode, it can be one of the following value: 
 *
 *        Test_J                 0x1
 *
 *        Test_K                 0x2
 *
 *        Test_SE0_NAK       0x3
 *
 *        Test_Packet          0x4
 *
 *        Test_Force_Enable  0x5
 *
 * @return  T_VOID
 */
T_VOID usb_slave_enter_testmode(T_U8 testmode);

/**
 * @brief   get ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  T_VOID
 */
T_U16 usb_slave_get_ep_status(T_U8 EP_Index);

/**
 * @brief   set ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @param bStall  [in]  stall or not.
 * @return  T_VOID
 */
T_VOID usb_slave_set_ep_status(T_U8 EP_Index, T_BOOL bStall);

//********************************************************************

#ifdef __cplusplus
}
#endif

#endif
