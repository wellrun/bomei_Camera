/**
 * @filename usb_slave_std.h
 * @brief: AK3223M standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-13
 * @version 1.0
 * @ref
 */

#ifndef __USB_HOST_STD_H__
#define __USB_HOST_STD_H__

#include "anyka_types.h"
#include "hal_usb_std.h"
#include "hal_usb_h_interrupt.h"
#include "hal_usb_h_disk.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB_H_STD Framework USB_H Std Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************

#define USB_MODE_20             (1<<8)   ///<usb high speed
#define USB_MODE_11             (1<<9)   ///<usb full speed



#define MAX_ENDPOINT_NUM            5

struct S_USB_CONNECT_DEVICE
{
    T_BOOL                          dev_config;
    T_USB_HOST_INTERRUPT            host_int;
    //T_U32                           *dev_function;
    //T_USB_HOST_DISK_DES             dev_function[MAX_LUN_NUM];
    T_USB_DEVICE_DESCRIPTOR         dev_devdescritor;
    T_USB_CONFIGURATION_DESCRIPTOR  dev_cfgdescritor;
    T_USB_INTERFACE_DESCRIPTOR      dev_ifadescritor;
    T_USB_ENDPOINT_DESCRIPTOR       dev_epdescritor[MAX_ENDPOINT_NUM];
    volatile T_BOOL                 usb_connect_flag;
    volatile T_BOOL                 usb_disconnect_flag;
   // F_USBHostDisConnect             usb_disconnect_callback;
    
   // F_USBHostEPCallBack				usb_in_callback;
   // F_USBHostEPCallBack				usb_in_dma_callback;
   // F_USBHostEPCallBack				usb_out_callback;
   // F_USBHostEPCallBack				usb_out_dma_callback;
};

typedef struct S_USB_CONNECT_DEVICE T_USB_CONNECT_DEVICE;

extern T_USB_CONNECT_DEVICE *Usb_Host_Standard;

T_BOOL usb_host_std_get_status(T_VOID);
T_BOOL usb_host_std_clear_feature(T_U8 EP_Index);
T_BOOL usb_host_std_set_feature(T_VOID);
T_BOOL usb_host_std_set_address(T_U32 dev_addr);
T_S32 usb_host_std_get_descriptor(T_U8 desc_type, T_U8 desc_index, T_U32 lang_id, T_U8 data[], T_U32 len);
T_BOOL usb_host_std_set_descriptor(T_VOID);
T_BOOL usb_host_std_get_configuration(T_VOID);
T_BOOL usb_host_std_set_configuration(T_U8 conf_val);
T_BOOL usb_host_std_get_interface(T_VOID);
T_BOOL usb_host_std_set_interface(T_VOID);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
