/**
 * @file usb_host.c
 * @brief Mount USB DISK
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author 
 * @MODIFY  
 * @DATE    2010-6-5
 * @version 0.1.0
 * @
 */
#include "Akos_api.h"
#include "mount_pub_api.h"
#include "Fwl_usb_host.h"
#include "fwl_usb_host.h"
#include "drv_api.h"
#include "eng_debug.h"
#include "eng_screensave.h"
#include "fwl_vme.h"
#include "gbl_global.h"
#include "mount.h"
#include "lib_event.h"
#include "fs.h"
#include "fwl_osMalloc.h"
#include "gpio_config.h"
#include "Fwl_sys_detect.h"
#include "Lib_state.h"
#include "Fwl_power.h"
#include "Lib_state_api.h"

extern T_GLOBAL gb;

#ifdef USB_HOST
#define MAX_LUN_NUM        5

typedef struct
{
    PDRIVER_INFO  DevInfo;
    T_U8          StartID;
    T_U8          PartitionNum;
}T_ST_DISK_INFO;

static T_BOOL  UsbHostInit_OK =AK_FALSE ;
static T_BOOL  UsbHostIsConnect =AK_FALSE;

static T_eUSB_HOST_PLUG_DEAL UsbHostPlugDeal = USB_HOST_PLUGOUT_NODEAL;

static T_U32               usb_disk_num = 0;
static T_H_UDISK_LUN_INFO  host_dev_function[MAX_LUN_NUM] = {0};
static T_ST_DISK_INFO      usb_disk_info[MAX_LUN_NUM] = {{AK_NULL,0,0}};
static T_BOOL                usb_disk_is_mnt = AK_FALSE;


//static T_BOOL usb_host_Flush(T_U32 LUN);
//static T_VOID usb_host_Destroy(T_PMEDIUM obj);
static T_U32 usb_host_read(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size);
static T_U32 usb_host_write(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size);

T_BOOL UsbDiskIsMnt(T_VOID)
{
    return usb_disk_is_mnt;
}

T_BOOL Fwl_UsbHostIsConnect(T_VOID)
{
    return UsbHostIsConnect;
}

T_BOOL Fwl_MountUSBDisk(T_U8 DriverNo)
{
    T_H_UDISK_LUN_INFO *host_disk =AK_NULL;
    T_U32 disk_num; 
    T_U32 i,j;
    T_U32 disk_mounted = 0;
    DRIVER_INFO   stTemp_DrvInfo;
    T_U8  drvID, DriverCnt;
    
    Fwl_Print(C3, M_USB, "Fwl_MountUSBDisk");

    if (gb.driverUDISK != AK_NULL)
         return AK_TRUE;
    disk_num = udisk_host_get_lun_num();
    if(disk_num == 0)
           return AK_FALSE;

    if(disk_num > MAX_LUN_NUM)
        disk_num = MAX_LUN_NUM;
    usb_disk_num = disk_num;

    Fwl_Print(C3, M_USB, "disk_num = %lu", usb_disk_num);

    host_disk = (T_H_UDISK_LUN_INFO *)host_dev_function;
    
       for (i = 0; i < disk_num; i++)
    {
        usb_disk_info[i].DevInfo = (T_PDRIVER)Fwl_Malloc(sizeof(DRIVER_INFO));
        if(usb_disk_info[i].DevInfo == AK_NULL)
        {
            Fwl_Print(C2, M_USB, "usb host drive error");
            continue;
        }

        memset(usb_disk_info[i].DevInfo, 0, sizeof(DRIVER_INFO));
        
        udisk_host_get_lun_info(i, (host_disk+i));    

        if((0 == (host_disk+i)->ulCapacity) || (0 == (host_disk + i)->ulBytsPerSec))
        {
            Fwl_Free(usb_disk_info[i].DevInfo);
            usb_disk_info[i].DevInfo = AK_NULL;
            Fwl_Print(C2, M_USB, "\nulCapacity == 0 or ulBytsPerSec == 0 !!!!!!");
            continue;    
        }
        
        usb_disk_info[i].DevInfo->nBlkCnt = (host_disk+i)->ulCapacity;
        usb_disk_info[i].DevInfo->nBlkSize = (host_disk + i)->ulBytsPerSec;
        usb_disk_info[i].DevInfo->nMainType = MEDIUM_USBHOST;
        usb_disk_info[i].DevInfo->nSubType = USER_PARTITION;
        usb_disk_info[i].DevInfo->fRead= usb_host_read;
        usb_disk_info[i].DevInfo->fWrite= usb_host_write;
        
        usb_disk_info[i].StartID = FS_MountMemDev(usb_disk_info[i].DevInfo, 
                                                  &(usb_disk_info[i].PartitionNum),
                                                  (T_U8)-1);

        Fwl_Print(C3, M_USB, "drvID = %d,\ndrv_info->DriverID = %d\nDriverCnt=%d\n",
            drvID, usb_disk_info[i].DevInfo->DriverID, DriverCnt);

        if(T_U8_MAX == usb_disk_info[i].StartID)
        {
            Fwl_Free(usb_disk_info[i].DevInfo);
            usb_disk_info[i].DevInfo = AK_NULL;
            continue;            
        }

        if (!usb_disk_is_mnt)
        {
            usb_disk_is_mnt = AK_TRUE;
        }

        j = 0;
        while(!FS_GetDriver(&stTemp_DrvInfo, usb_disk_info[i].StartID))
        {
            AK_Sleep(1);
            
            if(400 == j++)
            {
                break;
            }
        }
		if (!Fwl_UsbHostIsConnect())
			break;

        if(j>400)
        {
            break;
        }
        
        gb.driverUDISK = &usb_disk_info[i];
        Fwl_Print(C3, M_USB, "mount ok");

        disk_mounted++;        
    }        
    
    if(disk_mounted > 0)
    {
        gb.bUDISKAvail = AK_TRUE;
        Fwl_Print(C3, M_USB, "mount usb host ok");
    }
    else
    {        
        gb.driverUDISK = AK_NULL;
        return AK_FALSE;
    }

    return AK_TRUE;
}



T_BOOL UsbHost_DriveIsMnt(T_U8 drvId)
{
    T_U8 i;

    for (i = 0; i < usb_disk_num; i++)
    {

        if((drvId >= usb_disk_info[i].StartID) && 
            (drvId - usb_disk_info[i].StartID < usb_disk_info[i].PartitionNum))
        {
            return AK_TRUE;
        }
    }

    return AK_FALSE;
}


T_BOOL Fwl_UnMountUSBDisk(T_VOID)
{
    T_U8 i = 0,j=0;
    T_BOOL ret = AK_FALSE;


    for (i = 0; i < usb_disk_num; i++)
    {    
        if(AK_NULL == usb_disk_info[i].DevInfo)
            continue;
        for(j = 0; j < usb_disk_info[i].PartitionNum; ++j)
        {
            ret = FS_UnMountMemDev(usb_disk_info[i].StartID + j);
            if(!ret)
            {
                Fwl_Print(C1, M_USB, "unmount usbdisk  error\n");
            }
        }

        Fwl_Free(usb_disk_info[i].DevInfo);
        memset(&usb_disk_info[i], 0, sizeof(usb_disk_info[0]));        
    } 

    gb.bUDISKAvail = AK_FALSE;
    gb.driverUDISK = AK_NULL;
    usb_disk_num = 0;

    if (usb_disk_is_mnt)
    {
        usb_disk_is_mnt =AK_FALSE;
    }

    Fwl_Print(C3, M_USB, "UnMount USBDisk Finish");
    return ret;
}

static T_VOID Fwl_UsbDisk_POP_IN(T_U16 lun_ready_flag)
{
	static T_U32 printTick = 0;

	if (get_tick_count() - printTick > 2000)
	{
		printTick = get_tick_count();
		AkDebugOutput("usbhost:in\n");
	}
	if(AK_FALSE == UsbHostIsConnect    )
    {    
	    T_EVT_PARAM  EventParm;
       
	    Fwl_Print(C3, M_USB, "Enter usbhost popin deal.Dealproc =%d !\n",
	    	Fwl_UsbGetDealProc());

        if (USB_DEAL_BEGIN == Fwl_UsbGetDealProc()) //正在处理中
        	return ;
        	
        UsbHostIsConnect = AK_TRUE;    

        Fwl_UsbSetDealProc(USB_DEAL_BEGIN);
        if (eM_s_stdb_standby != SM_GetCurrentSM())
	        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, &EventParm);

		m_triggerEvent(M_EVT_USBHOST,&EventParm);
    }
    
}

T_VOID Fwl_UsbDisk_POP_UP(T_VOID)
{
    T_EVT_PARAM  EventParm;

    Fwl_Print(C3, M_USB, "Fwl_UsbDisk_POP_UP is vorked!usb dealproc=%d\n"
    	,Fwl_UsbGetDealProc());
    UsbHostIsConnect = AK_FALSE;
    if (USB_DEAL_BEGIN == Fwl_UsbGetDealProc()) //正在处理中
    {
    	Fwl_UsbSetDealProc(USB_DEAL_ABORT);
    	return ;
    }

	if (eM_s_stdb_standby != SM_GetCurrentSM()) //使返回到standby状态机
		m_triggerEvent(M_EVT_Z09COM_SYS_RESET, &EventParm);


	m_triggerEvent(M_EVT_USBHOST,&EventParm);

}
T_VOID Fwl_UsbDisk_Unload(T_VOID)
{    
    
    Fwl_Print(C3, M_USB, "Fwl_UsbDisk_Unload is vorded!usb_disk_is_mnt = %d", usb_disk_is_mnt);
    if (usb_disk_is_mnt)
    {

#ifdef OS_ANYKA
        VME_ReTriggerEvent(M_EVT_WAKE_SAVER, WAKE_GPIO);        
#endif    

        //usb host bug:when screen save then pop up the usb ,after wakeup can't screen save!So callback
        UserCountDownReset();

		Fwl_UnMountUSBDisk();
	}
    Fwl_Usb_DisconnectDisk();
    ScreenSaverEnable();
	Fwl_Usb_ConnectDisk();//重新初始化，以便能再次侦测
    Fwl_UsbSetDealProc(USB_DEAL_END);
	
}


T_BOOL Fwl_Usb_ConnectDisk(T_VOID)
{
    T_BOOL  bRet;

    if (UsbHostInit_OK)
    {
    	return AK_TRUE;
    }

    Fwl_UsbSetMode(USB_HOST_MODE);
    bRet = udisk_host_init(USB_MODE_20); 
    if(bRet)
    {
        UsbHostInit_OK = AK_TRUE;
        udisk_host_set_callback(Fwl_UsbDisk_POP_IN,Fwl_UsbDisk_POP_UP);
        Fwl_Print(C3, M_USB, "usbhost init success!");
    }
    else
    {        
	    Fwl_UsbSetMode(USB_NULL_MODE);
        Fwl_Print(C3, M_USB, "usbhost init failed!");
    }
	
    return bRet;    
}
//********************************************************************
T_VOID Fwl_Usb_DisconnectDisk(T_VOID)
{
    if(UsbHostInit_OK)
    {
        UsbHostInit_OK = AK_FALSE;
        udisk_host_close();        
    }
	UsbHostIsConnect =AK_FALSE;

}

static T_U32 usb_host_read(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U32 ret;
    T_U32 Lun;

    //if usb host pop up,then return
	if (!Fwl_UsbHostIsConnect())
		return 0 ;
	
    for(Lun = 0; Lun < usb_disk_num; Lun++)
    {
        if((T_U32)medium  == 
            (T_U32)(usb_disk_info[Lun].DevInfo ? usb_disk_info[Lun].DevInfo->medium : AK_NULL))
            break;
    }

    if(Lun == usb_disk_num)
    {
        return 0;
    }
    
    ret = udisk_host_read(Lun, data, sector, size);

    return ret;
    
}

static T_U32 usb_host_write(T_PMEDIUM medium, T_U8 data[], T_U32 sector, T_U32 size)
{
    T_U32 ret;    
    T_U32 Lun;

    //if usb host pop up,then return
	if (!Fwl_UsbHostIsConnect())
		return 0;
    
    for(Lun = 0; Lun < usb_disk_num; Lun++)
    {
        if((T_U32)medium  == 
            (T_U32)(usb_disk_info[Lun].DevInfo ? usb_disk_info[Lun].DevInfo->medium : AK_NULL))
            break;
    }

    if(Lun == usb_disk_num)
    {
        return 0;
    }
    
    ret = udisk_host_write(Lun, data, sector, size);
    
    return ret;
}

T_eUSB_HOST_PLUG_DEAL Fwl_USbHost_GetPlugDeal(T_VOID)
{
    return UsbHostPlugDeal;
}

#else

T_BOOL UsbDiskIsMnt(T_VOID)
{
    return AK_FALSE;
}
#endif

//********************************mount USB host( mass storage class device)*************************//



