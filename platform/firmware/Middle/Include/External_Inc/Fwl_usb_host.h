/**
 * @file Fwl_usb_host.h
 * @brief Mount USB DISK
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author 
 * @MODIFY  
 * @DATE    2010-6-5
 * @version 0.1.0
 * @
 */

#ifndef _USB_HOST_H_
#define _USB_HOST_H_

#include "anyka_types.h"

#ifdef OS_ANYKA
#ifdef USB_HOST

typedef enum {
    USB_HOST_PLUGOUT_NODEAL,
    USB_HOST_PLUGOUT_DEAL
} T_eUSB_HOST_PLUG_DEAL;

T_BOOL Fwl_MountUSBDisk(T_U8 DriverNo);
T_BOOL Fwl_UnMountUSBDisk(T_VOID);
T_BOOL UsbHost_DriveIsMnt(T_U8 drvId);
/**
 * @brief   Is use to get Usb Host Pulg Out,But no UnMount,return USB_HOST_PLUGOUT_DEAL
                If Had UnMounted will return USB_HOST_PLUGOUT_NODEAL
 * @author 
 * @date    
 * @param   [in] T_VOID
 * @return  T_eUSB_HOST_PLUG_DEAL
 */
T_eUSB_HOST_PLUG_DEAL Fwl_USbHost_GetPlugDeal(T_VOID);

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief   enable usb disk function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_VOID
 * @return  T_VOID
 */
T_BOOL Fwl_Usb_ConnectDisk(T_VOID);

/**
 * @brief   disnable usb disk function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_VOID
 * @return  T_VOID
 */
T_VOID Fwl_Usb_DisconnectDisk(T_VOID);
//********************************************************************

T_BOOL Fwl_UsbHostIsConnect(T_VOID);
T_VOID Fwl_UsbDisk_POP_UP(T_VOID);
T_VOID Fwl_UsbDisk_Unload(T_VOID);

#ifdef __cplusplus
}
#endif

#endif
#endif

T_BOOL UsbDiskIsMnt(T_VOID);

#endif
