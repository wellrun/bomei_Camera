/**
 * @filename usb_slave_std.h
 * @brief: AK3223M standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-13
 * @version 1.0
 */

#ifndef __USB_SLAVE_STD_H__
#define __USB_SLAVE_STD_H__

#include "anyka_types.h"
#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB_S_STD Framework USB_S STD Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************
#define DEF_DESC_LEN              (INTERFACE_DESC_LEN + CONFIGURE_DESC_LEN + ENDPOINT_DESC_LEN * ENDPOINT_NUMBER)               //9 + 9 + 3 * 7   : 3 - 3 end point
//********************************************************************
/** @brief Struct of Descriptor
 *
*/

struct S_USB_CONFIG_ALL
{
    T_USB_CONFIGURATION_DESCRIPTOR  cf_desc;    //Configure Descriptor
    T_USB_INTERFACE_DESCRIPTOR      if_desc;    //Interface Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep1;        //ENDPOINT Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep2;        //ENDPOINT Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep3;        //ENDPOINT Descriptor
};

typedef struct S_USB_CONFIG_ALL T_USB_CONFIG_ALL;

struct S_USB_CONFIG
{
    T_U8    *cf_desc;       //point of Configure Descriptor
    T_U8    *if_desc;       //point of Interface Descriptor
    T_U8    *ep1;           //point of ENDPOINT Descriptor
    T_U8    *ep2;           //point of ENDPOINT Descriptor
    T_U8    *ep3;           //point of ENDPOINT Descriptor
};

typedef struct S_USB_CONFIG T_USB_CONFIG;
typedef T_VOID (*T_fUSB_RESET)(T_U32 mode);
typedef T_VOID (*T_fUSB_SUSPEND)(T_VOID);                   
typedef T_VOID (*T_fUSB_RESUME)(T_VOID);                    
typedef T_VOID (*T_fUSB_STANDARD_REQ)(T_UsbDevReq dev_req); 
typedef T_VOID (*T_fUSB_VENDOR_REQ)(T_UsbDevReq dev_req);   
typedef T_VOID (*T_fUSB_CLASS_REQ)(T_UsbDevReq dev_req);


struct S_USB_SLAVE_STANDARD{
    T_U32   Usb_Device_Type;                            //usb device type
    T_U8    *Device_Descriptor;                         //device descriptor
    T_USB_CONFIG    Device_Config;                      //device config descriptor
    T_U8    *Device_String;                             //device string descriptor
    T_U8    *Buffer;                                    //device data buffer
    T_U32    buf_len;                                    //device data buffer length
    volatile T_U8    Device_ConfigVal;                           //device configure value
    volatile T_U8    Device_Address;                             //device address
    T_VOID  (*usb_reset)(T_U32 mode);                       //unusual usb reset handle
    T_VOID  (*usb_suspend)(T_VOID);                     //unusual usb suspend handle
    T_VOID  (*usb_resume)(T_VOID);                      //unusual usb resume handle
    
    T_U8 *(*usb_get_device_descriptor)(T_U32 *count);               //get device descriptor
    T_U8 *(*usb_get_config_descriptor)(T_U32 *count);               //get config descriptor
    T_U8 *(*usb_get_string_descriptor)(T_U8 index, T_U32 *count);     //get string descriptor
    T_U8 *(*usb_get_device_qualifier_descriptor)(T_U32 *count);     //get device qualifier descriptor
    T_U8 *(*usb_get_other_speed_config_descriptor)(T_U32 *count);      //get other speed config
};

typedef struct S_USB_SLAVE_STANDARD T_USB_SLAVE_STANDARD;
typedef struct S_USB_SLAVE_STANDARD* TP_USB_SLAVE_STANDARD;

extern T_USB_SLAVE_STANDARD Usb_Slave_Standard;

/**
 * @brief   change the clear stall condition in clear feature
 *
 * @author  liao_zhijun
 * @date    2010-07027
 * @param bSet [in]: clear stall in clear feature or not 
 * @return  T_VOID
 */
T_VOID usb_slave_std_hard_stall(T_BOOL bSet);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
