/**
 * @file usb_bus_driver.h
 * @brief:  frameworks of usb bus driver.
 *
 * This file describe driver of usb in host mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __USB_BUS_DRV_H__
#define __USB_BUS_DRV_H__

#include "anyka_types.h"
#include "anyka_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Bus_Driver USB Bus Driver
 *  @ingroup Usb_Host
 */
/*@{*/

/** @name urb wait completion return value Define
 *	
 */
/*@{*/
#define URB_COMPLETE 0          /* 0: complete successfully */
#define URB_ERROR -1            /* -1: data transfer fail */
#define URB_TIMEOUT -2          /* -2: data transfer timeout */
/* @} */

#define URB_INVALIDATE_HANDLE AK_NULL

#define URB_MAX_WAIT_TIME   400 /*some udisk req sense proc exceed 1s,so wait 2s*/

#define MAX_REG_CLASS_NUM 8

#define USB_BUS_IDLE 0
#define USB_BUS_CTRL_TRANS 1
#define USB_BUS_BULK_TRANS 2

#define MESSAGE_CONNECT     0x1
#define MESSAGE_DISCONNECT  0x2

#define CONNECT_MSG_PARAM_ENUM      0xffff 

#define EVENT_TRANS_COMPLETE 0x1

#define MAX_ENDPOINT_NUM            5

/**
 *	disconnect callback handler
 */
typedef T_VOID (*T_fUSB_BUS_DISCONNECT_CALLBACK)(T_VOID);

/**
 *	enum ok callback handler
 */
typedef T_VOID (*T_fUSB_BUS_ENUMOK_CALLBACK)(T_VOID);

/**
 *	urb complete ok callback handler
 */
typedef T_VOID (*T_fUSB_BUS_URB_CALLBACK)(T_VOID);

/**
 *	urb handle
 */
typedef T_VOID* T_URB_HANDLE;

/**
 * @brief Transfer Type
 
 *   define Transfer Type
 */
typedef enum
{
    TRANS_CTRL = 0,     ///< control transfer
    TRANS_BULK,         ///< bulk tranfer
    TRANS_ISO,          ///< isochronous transfer
    TRANL_INTR          ///< interrupt transfer
}
E_TRANSFER_TYPE;

/**
 * @brief Transfer direction
 
 *   define Transfer direction
 */
typedef enum
{
    TRANS_DATA_OUT = 0,  ///< data out
    TRANS_DATA_IN      ///< data in
}
E_TRANSFER_DIR;

/**
 * @brief bus event handler
 
 *   define bus event handler, different class driver have different handler
 */
typedef struct tagUSB_BUS_HANDLER
{
    T_U8 class_code;                                    ///< class code 
    T_fUSB_BUS_ENUMOK_CALLBACK enumok_callback;         ///< callback function for enum ok event
    T_fUSB_BUS_DISCONNECT_CALLBACK discon_callback;     ///< callback function for disconnect
}
T_USB_BUS_HANDLER, *T_pUSB_BUS_HANDLER;

/**
 * @brief usb request block
 
 *   define usb request block
 */
typedef struct tagURB
{
    E_TRANSFER_TYPE trans_type;                 ///< transfer type
    E_TRANSFER_DIR trans_dir;                   ///< transfer direction

    T_UsbDevReq dev_req;                       ///< device request, used in control trans only

    T_U8 *data;                                 ///< data buffer
    T_U32 buffer_len;                           ///< buffer length
    T_U32 data_len;                             ///< actual data transfer length

    T_U32 packet_num;                           ///< number of packets in this trans, only used in iso trans
    T_U32 interval;                             ///< polling interval, used in interrupt trans only

    T_U32 timeout;                              ///< time out value, million seconds
    T_fUSB_BUS_URB_CALLBACK callback;           ///< callback function after compeltion

    T_U32 trans_len;
    T_U8 result;
}
T_URB;

/**
 * @brief enum device
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_bus_enum(T_VOID);

/**
 * @brief open usb controller and phy
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] T_U32 usb mode
 * @return  T_VOID
 */
T_VOID usb_bus_open(T_U32 mode);

/**
 * @brief   close usb controller and phy.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  T_VOID
 */
T_VOID usb_bus_close();

/**
 * @brief   register usb class to bus driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param bus_handler struction containing with class code and event handler
 * @return  T_VOID
 */
T_BOOL usb_bus_reg_class(T_pUSB_BUS_HANDLER bus_handler);

/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  urb usb request block
 * @return  T_URB_HANDLE urb handle
 * @retval AK_NULL commit fail
 */
T_URB_HANDLE usb_bus_commit_urb(T_URB *urb);

/**
 * @brief   wait urb to complete, return value indicates it finished success or fail
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  hUrb urb handle
 * @return  T_S32
 * @retval URB_COMPLETE urb successfully complete
 * @retval URB_ERROR error during data transfer
 * @retval URB_TIMEOUT data transfer timeout
 */
T_S32 usb_bus_wait_completion(T_URB_HANDLE hUrb);

/**
 * @brief   get descriptor for usb bus
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param desc_type descriptor type
 * @param data [out]: buffer
 * @param len [in]: buffer length
 * @return  T_U32 length
 * @retval 0 fail
 */
T_U32 usb_bus_get_decsriptor(T_U8 desc_type, T_U8 data[], T_U32 len);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif
