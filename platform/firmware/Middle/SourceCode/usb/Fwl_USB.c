/**
@author liu zhenwu
@date   2006.1.24
@file       fwl_USB.c
@brief  control USB
*/
#include <string.h>
#include <stdlib.h>
#include "mount_pub_api.h"
#include "gbl_global.h"
#include "mount.h"
#include "fs.h"
#include "Fwl_vme.h"
#include "Fwl_osfs.h"
#include "Fwl_sd.h"
#include "fwl_usb.h"
#include "Lib_event.h"
#include "eng_debug.h"
#include "gpio_config.h"
#include "hal_usb_s_state.h"
#include "hal_usb_s_disk.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "akos_api.h"
#include "drv_api.h"
#include "fwl_sd.h"
#include "drv_gpio.h"
#include "fwl_osMalloc.h"
#include "Fwl_sys_detect.h"


#define  USB_FIXED_DESC_STR  "USB DEVICE"
#define USB_MNT_MAX_NUM  8

#if 0
static T_hSemaphore   gUSBSem = AK_INVALID_SEMAPHORE;

#define   USB_MUTEX_INIT()   TRD_MUTEX_INIT(gUSBSem)
#define   USB_MUTEX_DEINIT() TRD_MUTEX_DEINIT(gUSBSem)
#define   USB_LOCK()         TRD_LOCK(gUSBSem)
#define   USB_UNLOCK()       TRD_UNLOCK(gUSBSem)
#else
#define   USB_MUTEX_INIT()   
#define   USB_MUTEX_DEINIT() 
#define   USB_LOCK()         
#define   USB_UNLOCK()       
#endif

#define USB_CFG_TIMEOUT   (15000)
#define USB_FLUSH_INTERVAL (100)  //(5000)

typedef struct 
{
    T_ACCESS_MEDIUM type;
    T_PMEDIUM medium;
    T_U8 drvId;
    T_BOOL cachemode;
} T_USB_MEDIUM_INFO;

T_USB_MEDIUM_INFO UsbMediumInfo[USB_MNT_MAX_NUM];

typedef struct
{
    T_U16 read;
    T_U16 write;
}T_Medium_OptCnt;

typedef struct
{
    T_TIMER      Usb_TimerId;
    T_BOOL    Usb_NeedFlush;//need flush driver flg
    T_BOOL    Usb_FlushId[USB_MNT_MAX_NUM];
}T_USB_FLUSH;

typedef struct
{
    T_U8*  pVendor;        //string no longer than 8
    T_U8*  pProduct;       //string no longer than 16
    T_U8*  pRevision;       //string no longer than 4
}T_ST_INQUIRY;

T_ST_INQUIRY  NdRes_inquiry = 
{
    "Anyka",
    "USB disk 2.0",
    "1.00"
};

T_ST_INQUIRY  NdOpn_inquiry = 
{
    "Anyka",
    "USB disk 2.0",
    "1.00"
};


T_ST_INQUIRY  Sd_inquiry = 
{
    "Anyka",
    "SD card",
    "1.00"
};


T_ST_INQUIRY  Mmc_inquiry = 
{
    "Anyka",
    "MMC card",
    "1.00"
};


T_ST_INQUIRY  Default_inquiry = 
{
    "Anyka",
    "Other",
    "1.00"
};

static T_USB_FLUSH UsbFlush;

static T_Medium_OptCnt Sd_OptCnt = {0};
static T_Medium_OptCnt mmc_OptCnt = {0};
static T_Medium_OptCnt nand_OptCnt = {0};
static T_Medium_OptCnt resNd_OptCnt = {0};

static T_BOOL Usb_MountLUN(T_ACCESS_MEDIUM disk_type);
static T_BOOL Usb_ChangeLUN(T_ACCESS_MEDIUM disk_type,E_SENSESTATUS status);
static T_VOID Usb_UnInstallDriver(T_ACCESS_MEDIUM disk_type);
static T_VOID Usb_InstallDriver(T_ACCESS_MEDIUM disk_type);
static T_VOID Usb_MediumInfoInit(T_VOID);
static T_VOID Usb_reInitFs(T_BOOL isFlush);
static T_U8   Usb_FlushAllDisk(T_BOOL NeedFlush);
static T_BOOL Usb_AddMediumInfo(T_PMEDIUM medium,T_U8 drvId,T_ACCESS_MEDIUM type,T_BOOL cachemode);
static T_PMEDIUM Usb_GetDiskMedium(T_ACCESS_MEDIUM type);
T_VOID Usb_SdDectect(T_VOID);
static T_VOID Usb_MMCDectect(T_VOID);
static T_BOOL Usb_MediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_MediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_NandRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_NandWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_SdMediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_SdMediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_MMCMediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_MMCMediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_ResNdRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_BOOL Usb_ResNdWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo);
static T_VOID Usb_DestroyCacheMediumAll(T_VOID);
static T_VOID Usb_SetNeedFlushByType(T_ACCESS_MEDIUM type);
static T_VOID Usb_ClearFlushFlgById(T_U8 Id);
static T_VOID Usb_CheckStateTimer(T_TIMER timer_id, T_U32 delay);

#ifdef DUMMY_DISK
#define DUMMYDISKSIZE (5*1024*1024)
static T_U8 *dummyram = AK_NULL;

static T_BOOL DUMMY_Write(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
static T_BOOL DUMMY_Read(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo);
#endif

static T_BOOL Usb_SdMediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    if (sd_is_connected() && Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO))
    {
        Sd_OptCnt.read++;
        //UsbFlush.Usb_NeedFlush = AK_TRUE;
        //Usb_SetNeedFlushByType(SDCARD_DISK);
        return Usb_MediumRead(buf,BlkAddr,BlkCnt,LunAddInfo);
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_SdMediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    if (sd_is_connected() && Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO))
    {
        Sd_OptCnt.write++;
        UsbFlush.Usb_NeedFlush = AK_TRUE;
        Usb_SetNeedFlushByType(SDCARD_DISK);
        return Usb_MediumWrite(buf,BlkAddr,BlkCnt,LunAddInfo);
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_MMCMediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    if (mmc_is_connected() && Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC))
    {
        mmc_OptCnt.read++;
        //UsbFlush.Usb_NeedFlush = AK_TRUE;
        //Usb_SetNeedFlushByType(MMC_DISK);
        return Usb_MediumRead(buf,BlkAddr,BlkCnt,LunAddInfo);
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_MMCMediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    if (mmc_is_connected() && Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC))
    {
        mmc_OptCnt.write++;
        UsbFlush.Usb_NeedFlush = AK_TRUE;
        Usb_SetNeedFlushByType(MMC_DISK);
        return Usb_MediumWrite(buf,BlkAddr,BlkCnt,LunAddInfo);
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_NandRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    nand_OptCnt.read++;
    //UsbFlush.Usb_NeedFlush = AK_TRUE;
    //Usb_SetNeedFlushByType(NANDFLASH_DISK);
    return Usb_MediumRead(buf,BlkAddr,BlkCnt,LunAddInfo);
}

static T_BOOL Usb_NandWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    nand_OptCnt.write++;
    UsbFlush.Usb_NeedFlush = AK_TRUE;
    Usb_SetNeedFlushByType(NANDFLASH_DISK);
    return Usb_MediumWrite(buf,BlkAddr,BlkCnt,LunAddInfo);
}

static T_U32 Usb_GetPartitionSecCnt(T_PMEDIUM pMedium)
{
    T_U8 *TmpBuf  = AK_NULL;
    T_U32 TotSec = 0;
    
    TmpBuf = (T_U8*)Fwl_Malloc(8192);
    if (AK_NULL == TmpBuf)
    {
       return 0;
    }
    
    /* Read NandFlash to check the capacity. */
    if (pMedium->read(pMedium, (T_U8*)TmpBuf, 0, 1) != 0)
    {
       memcpy(&TotSec, TmpBuf + 19, sizeof(T_U16));
       if (0 == TotSec)
       {
           TotSec = *(T_U32*)(TmpBuf + 32);
       }
    }
    else
    {
       TotSec = pMedium->capacity;
    }

    if (TotSec < pMedium->capacity)
    {
       TotSec = pMedium->capacity;
    }

    Fwl_Free(TmpBuf);

    return TotSec;
}    

static T_BOOL Usb_ResNdRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    resNd_OptCnt.read++;
    //UsbFlush.Usb_NeedFlush = AK_TRUE;
    //Usb_SetNeedFlushByType(NANDRESERVE_ZONE);
    return Usb_MediumRead(buf,BlkAddr,BlkCnt,LunAddInfo);
}

static T_BOOL Usb_ResNdWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    resNd_OptCnt.write++;
    UsbFlush.Usb_NeedFlush = AK_TRUE;
    Usb_SetNeedFlushByType(NANDRESERVE_ZONE);
    return Usb_MediumWrite(buf,BlkAddr,BlkCnt,LunAddInfo);
}

static T_VOID Usb_MediumInfoInit(T_VOID)
{
    T_U32 i = 0;
    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        UsbMediumInfo[i].medium = AK_NULL;
        UsbMediumInfo[i].type   = 0;
        UsbMediumInfo[i].drvId = 0;
        UsbMediumInfo[i].cachemode = AK_FALSE;
    }
    memset(&Sd_OptCnt, 0, sizeof(Sd_OptCnt));
    memset(&mmc_OptCnt, 0, sizeof(mmc_OptCnt));
    memset(&nand_OptCnt, 0, sizeof(nand_OptCnt));
    memset(&resNd_OptCnt, 0, sizeof(resNd_OptCnt));
}


static T_VOID Usb_reInitFs(T_BOOL isFlush)
{
#ifdef OS_ANYKA    
       if (isFlush)
       {
        Usb_FlushAllDisk(AK_TRUE);//FLUSH 所有的盘,否则拷贝或删除一些东东没有修改
    }
    
       Usb_DestroyCacheMediumAll();
    
    Usb_InstallDriver(NANDFLASH_DISK);
       Usb_InstallDriver(SDCARD_DISK);
    Usb_InstallDriver(MMC_DISK);
    
       usbdisk_stop();
       usb_slave_set_state(USB_NOTUSE);
#endif
}
static T_BOOL Usb_AddMediumInfo(T_PMEDIUM medium,T_U8 drvId,T_ACCESS_MEDIUM type,T_BOOL cachemode)
{
    T_U32 i = 0;

    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        if (AK_NULL == UsbMediumInfo[i].medium)
        {
            UsbMediumInfo[i].medium    = medium;
            UsbMediumInfo[i].type       = type;
            UsbMediumInfo[i].drvId     = drvId;
            UsbMediumInfo[i].cachemode = cachemode;
            
            Fwl_Print(C3, M_USB, "USB Add Cache: DrvID=%d,CacheID=%d,Type=%d", drvId,i,type);

            return AK_TRUE;
        }
    }

    return AK_FALSE;
}

static T_VOID Usb_CleanMediumInfo(T_U8 CacheId)
{
    switch(UsbMediumInfo[CacheId].type)
    {
        case SDCARD_DISK:
            memset(&Sd_OptCnt, 0, sizeof(Sd_OptCnt));
            break;
        case MMC_DISK:
            memset(&mmc_OptCnt, 0, sizeof(mmc_OptCnt));
            break;
        case NANDFLASH_DISK:
            memset(&nand_OptCnt, 0, sizeof(nand_OptCnt));
            break;
        case NANDRESERVE_ZONE:
            memset(&resNd_OptCnt, 0, sizeof(resNd_OptCnt));
            break;    
            
        default:
            break;
    }
    
    UsbMediumInfo[CacheId].medium = AK_NULL;
    UsbMediumInfo[CacheId].type   = 0;
    UsbMediumInfo[CacheId].drvId = 0;
    UsbMediumInfo[CacheId].cachemode = AK_FALSE;
    
}

static T_VOID Usb_DestroyCacheMedium(T_ACCESS_MEDIUM Type)
{
    T_U32 i;
    
    for (i=0; i<USB_MNT_MAX_NUM; i++)
    {
        if (Type == UsbMediumInfo[i].type)
        {
            if ((AK_NULL != UsbMediumInfo[i].medium) && (AK_TRUE == UsbMediumInfo[i].cachemode))
            {
                Fwl_Print(C3, M_USB, "USB Destroy Cache: DrvID=%d,CacheID=%d,Type=%d", UsbMediumInfo[i].drvId,i,Type);
                FS_DestroyCache(UsbMediumInfo[i].medium);
                Usb_CleanMediumInfo((T_U8)i);
            }
        }
    }
}
    

static T_U8 Usb_FlushAllDisk(T_BOOL NeedFlush)
{
    T_U32 i = 0;
    T_PMEDIUM pMedium_tmp;
    T_BOOL isNeedFlush = AK_FALSE;
    T_U8 flshCnt = 0;
    T_ACCESS_MEDIUM type = 0;
    T_Medium_OptCnt *pMediumOptCnt = AK_NULL;

    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if (AK_NULL != pMedium_tmp)
        {
            isNeedFlush = AK_FALSE;
            pMediumOptCnt = AK_NULL;
            type = UsbMediumInfo[i].type;
            if (SDCARD_DISK == type)  //SDIO
            {
                isNeedFlush = Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO);
                pMediumOptCnt = &Sd_OptCnt;
            }
            
            else if (MMC_DISK == type) // SDMMC
            {
                isNeedFlush = Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC);
                pMediumOptCnt = &mmc_OptCnt;
            }

            else if (NANDFLASH_DISK == type)//Nand Open
            {
                isNeedFlush = AK_TRUE;
                pMediumOptCnt = &nand_OptCnt;
            }

            else if (NANDRESERVE_ZONE == type)//Nand reserve
            {
                isNeedFlush = AK_TRUE;
                pMediumOptCnt = &resNd_OptCnt;
            }

            if (isNeedFlush && (AK_NULL != pMediumOptCnt))
            {
                if (((pMediumOptCnt->write == 0) && (AK_TRUE == UsbFlush.Usb_FlushId[i])) \
                        || NeedFlush)
                {
                    pMedium_tmp->flush(pMedium_tmp);
                    Fwl_Print(C3, M_USB, "[%d]<",UsbMediumInfo[i].drvId);
                    Usb_ClearFlushFlgById((T_U8)i);
                    flshCnt++;
                }
                pMediumOptCnt->read  = 0;//clear read count
                pMediumOptCnt->write = 0;//clear write count
            }
        }
    }

    return flshCnt;
}

static T_VOID Usb_SetNeedFlushByType(T_ACCESS_MEDIUM type)
{
    T_U32 i;
    
    for (i=0; i<USB_MNT_MAX_NUM; i++)
    {          
        if (type == UsbMediumInfo[i].type)
        {   
            if (AK_NULL != UsbMediumInfo[i].medium)
            {   
                UsbFlush.Usb_FlushId[i] = AK_TRUE; //确定哪个盘需要FLUSH
            }
        }
    }
}
static T_VOID Usb_ClearFlushFlgById(T_U8 Id)
{
//    UsbFlush.Usb_NeedFlush = AK_FALSE;
    UsbFlush.Usb_FlushId[Id] = AK_FALSE; //Clear Flush Flg
}

static T_PMEDIUM Usb_GetDiskMedium(T_ACCESS_MEDIUM type)
{
    T_U32 i = 0;
    T_PMEDIUM pMedium_tmp = AK_NULL;
    
    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {  
        pMedium_tmp = UsbMediumInfo[i].medium;
        
        if (AK_NULL != pMedium_tmp)
        {   
            if (type == UsbMediumInfo[i].type)
            {   
                return pMedium_tmp;
            }
        }
    }
    return AK_NULL;
}

static T_VOID Usb_DestroyCacheMediumAll(T_VOID)
{
    T_U32 i = 0;

    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        if ((AK_NULL != UsbMediumInfo[i].medium) && UsbMediumInfo[i].cachemode)
        {
            
            FS_DestroyCache(UsbMediumInfo[i].medium);
            Fwl_Print(C3, M_USB, "Destroy Cache Id:%d",i);
        }
    }
}
static T_VOID Usb_UnInstallDriver(T_ACCESS_MEDIUM disk_type)
{
    T_U32 i = 0;
    T_PMEDIUM pMedium_tmp;
        
    for (i=0; i<USB_MNT_MAX_NUM; i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if ((AK_NULL != pMedium_tmp) && (disk_type == UsbMediumInfo[i].type))
        {
            FS_UnInstallDriver(UsbMediumInfo[i].drvId,1);
        }
    }
}

static T_VOID Usb_InstallDriver(T_ACCESS_MEDIUM disk_type) 
{
    T_U32 i = 0;
    T_PMEDIUM pMedium_tmp;
        
    for (i=0; i<USB_MNT_MAX_NUM; i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if ((AK_NULL != pMedium_tmp) && (disk_type == UsbMediumInfo[i].type))
        {
            FS_InstallDriver(UsbMediumInfo[i].drvId,1);
        }
    }
}

T_VOID USB_SendEvent(T_VOID)
{
    Fwl_Print(C3, M_USB, "USB_SendEvent");

#ifdef OS_ANYKA
#ifdef CAMERA_SUPPORT
    VME_ReTriggerEvent(M_EVT_VIDEO_RECORD_STOP, AK_NULL);
#endif
    VME_ReTriggerEvent(M_EVT_AUDIO_STOP, AK_NULL);
    VME_ReTriggerEvent(M_EVT_WAKE_SAVER, WAKE_GPIO);
#endif    
}


T_VOID Usb_SdDectect(T_VOID)
{
#ifdef OS_ANYKA
    /** plug in*/
    if((sd_is_connected()) && (!Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO)))
    {
        Fwl_Print(C3, M_USB, "\n[USB]Detect SDIO plug in");
        if (Fwl_Sd_HwInit(eSD_INTERFACE_SDIO) && Fwl_Sd_Mount(eSD_INTERFACE_SDIO))
        {  
            Usb_DestroyCacheMedium(SDCARD_DISK);
            Usb_ChangeLUN(SDCARD_DISK, MEDIUM_NOTREADY_TO_READY);
        }
        else
        {
            Fwl_Sd_HwDestory(eSD_INTERFACE_SDIO);
        }
    }
    /**plug out*/
    else if ((!sd_is_connected()) && Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO))
    {
        Fwl_Print(C3, M_USB, "\n[USB]Detect SDIO plug out");
        Usb_DestroyCacheMedium(SDCARD_DISK);
        Fwl_Sd_UnMount(eSD_INTERFACE_SDIO);
        Fwl_Sd_HwDestory(eSD_INTERFACE_SDIO);
        Usb_ChangeLUN(SDCARD_DISK, MEDIUM_NOTPRESENT);
    }
    else
    {}
#endif
}
static T_VOID Usb_MMCDectect(T_VOID)
{
#ifdef OS_ANYKA
    /** plug in*/
    if((mmc_is_connected()) && (!Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC)))
    {
        Fwl_Print(C3, M_USB, "\n[USB]Detect SDMMC plug in");
        if (Fwl_Sd_HwInit(eSD_INTERFACE_SDMMC) && Fwl_Sd_Mount(eSD_INTERFACE_SDMMC))
        {  
            Usb_DestroyCacheMedium(MMC_DISK);
            Usb_ChangeLUN(MMC_DISK, MEDIUM_NOTREADY_TO_READY);
        }
        else
        {
            Fwl_Sd_HwDestory(eSD_INTERFACE_SDMMC);
        }
    }
    /**plug out*/
    else if ((!mmc_is_connected()) && Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC))
    {
        Fwl_Print(C3, M_USB, "\n[USB]Detect SDMMC plug out");
        Usb_DestroyCacheMedium(MMC_DISK);
        Fwl_Sd_UnMount(eSD_INTERFACE_SDMMC);
        Fwl_Sd_HwDestory(eSD_INTERFACE_SDMMC);
        Usb_ChangeLUN(MMC_DISK, MEDIUM_NOTPRESENT);
    }
    else
    {}
#endif
}


#ifdef USB_FIXED_DESC_STR
static T_CHR * get_usb_desc_str(T_VOID)
{
    return USB_FIXED_DESC_STR;        
}
#else
/** 
 * @brief reverse str
 *
 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */
static T_VOID  reverse(T_CHR   *s)   
{ 
    T_CHR   *c; 
    T_U32   i; 

    c  =   s   +   strlen(s)   -   1; 
    while(s   <   c)  
    { 
        i = *s; 
        *s++ = *c; 
        *c-- = i; 
    } 
} 

/** 
 * @brief convert int to str
 *

 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */

static T_VOID Usb_itoa(T_U32   n,   T_CHR   *s) 
{ 
    T_CHR   *ptr; 
    ptr = s; 

    do   
    { 
        *ptr++   =   n   %   10   +   '0'; 
    } while   ((n   =   n   /   10)   >   0); 
    
    *ptr   =   '\0'; 
    
    reverse(s); 
}

/** 
 * @brief init serial number str in device desc
 *
 * the random str is truncated to 10  characters or less
 * @author Huang Xin
 * @date 2010-10-25
 * @return  T_VOID
 */
static T_VOID init_serial_number(T_VOID)
{

    T_U8 i = 0;
    T_U32 random = 0;
    T_CHR str[20] = {0};
    T_CHR *p = str;    

    srand(get_tick_count());
    random = rand();
    Usb_itoa(random,str);

    gs.UsbDescStr[MAX_NUM_USB_DESC_STR-1] = 0;
    
    for (i = 0; i<MAX_NUM_USB_DESC_STR-1; ++i)
    {
        if (*p)
        {
            gs.UsbDescStr[i] = *p++;
        }
        else
        {
            gs.UsbDescStr[i] = ' ';
        }
        gs.UsbDescStr[MAX_NUM_USB_DESC_STR-1] ^= gs.UsbDescStr[i];
        
    }
}

static T_CHR * get_usb_desc_str(T_VOID)
{
    T_U8         temp = 0;
    T_BOOL       bPass = AK_FALSE;
    T_U32        uCount;
    static T_CHR usb_desc_str[MAX_NUM_USB_DESC_STR] = {0};

#if(MAX_NUM_USB_DESC_STR <= 11)
#error "MAX_NUM_USB_DESC_STR <= 11"
#endif

    if(0 != gs.UsbDescStr[0])
    {
        for(uCount=0; uCount < MAX_NUM_USB_DESC_STR; uCount++)
        {
            temp ^= gs.UsbDescStr[uCount];
        }
        if(0 == temp)
        {
            bPass = AK_TRUE;
        }            
    }

    if(!bPass)
        init_serial_number();
        
    memcpy(usb_desc_str, gs.UsbDescStr,10);        
    usb_desc_str[10] = 0;
    Fwl_Print(C1, M_USB,"usb_desc_str = %s", usb_desc_str);
    return usb_desc_str;        
}

#endif

/**
 * @brief Mount U-Disk 
 * @author 
 * @date 2012-03-29
 * @param[in] T_VOID
 * @return T_BOOL
 * @retval AK_TRUE if Mount Disk success
 * @retval AK_FALSE if Mount Disk  fail
 */
T_BOOL Fwl_UsbMountDisk(T_VOID)
{
#ifdef OS_ANYKA
    T_U32 refTickCnt = 0,curTickCnt = 0;
    T_U32 mode = 0;
    T_U8 curStatus = 0;    

    mode = USB_MODE_20 | USB_MODE_DMA; // usb high speed 
    
    Usb_MediumInfoInit();
    
    mini_delay(250);//SW10A00001716
    UsbFlush.Usb_TimerId = ERROR_TIMER;
    
    if(usbdisk_init(mode))
    {
        usbdisk_set_str_desc(STR_SERIAL_NUM,get_usb_desc_str());

        //Must be Mount nand disk frist
        Usb_MountLUN(NANDFLASH_DISK);
        Usb_MountLUN(SDCARD_DISK);
        Usb_MountLUN(MMC_DISK);
        
        if(!usbdisk_start())
        {
            Usb_reInitFs(AK_FALSE);
            Fwl_Print(C1, M_USB, "start fail!");
            return AK_FALSE;                        
        }
        else
        {
            Fwl_Print(C1, M_USB, "Start Ok");
        }
        
        refTickCnt = get_tick_count();
        USB_MUTEX_INIT();
        
        AK_Feed_Watchdog(0);
        //usb config
        while (USB_OK != (curStatus = usb_slave_getstate()))
        {            
            if ((USB_ERROR == curStatus) || (USB_NOTUSE == curStatus) \
                || (USB_SUSPEND == curStatus) || (USB_START_STOP == curStatus) \
                || (USB_TEST_UNIT_STOP == curStatus) || (!usb_is_connected()))
            {
                Fwl_Print(C2, M_USB, "[UsbSlave]Status = %d",curStatus);

                Fwl_UsbDiskStop();

                return AK_FALSE;
            }

            curTickCnt = get_tick_count();
            if (USB_CONFIG == curStatus)
            {
                if ((curTickCnt > refTickCnt) \
                    && ((curTickCnt -refTickCnt) > USB_CFG_TIMEOUT))
                {
                    Fwl_Print(C1, M_USB, "config TimeOut!");

                    Fwl_UsbDiskStop();

                    return AK_FALSE;
                }
                else
                {
                    mini_delay(200);
                }
            }
        }
    }
    else
    {
        usbdisk_stop();
        Fwl_Print(C1, M_USB, "usbdisk init error!");
        return AK_FALSE;
    }
    
    UsbFlush.Usb_TimerId = vtimer_start(USB_FLUSH_INTERVAL, AK_TRUE, Usb_CheckStateTimer);
    if (ERROR_TIMER == UsbFlush.Usb_TimerId)
    {
        Fwl_Print(C1, M_USB, "usbdisk start check timer to flush fail");
        Fwl_UsbDiskStop();
        return AK_FALSE;
    }    
#endif

    return AK_TRUE;

}

/**
 * @brief To stop U-Disk Status,Stop Must Be Mount success.
 * @author 
 * @date 2012-03-29
 * @param[in] T_VOID
 * @return T_VOID
 */
T_VOID Fwl_UsbDiskStop(T_VOID)
{    
    if (UsbFlush.Usb_TimerId != ERROR_TIMER)
    {
        vtimer_stop(UsbFlush.Usb_TimerId);
        UsbFlush.Usb_TimerId = ERROR_TIMER;
    }
    
    AK_Feed_Watchdog(4);
    
    USB_MUTEX_DEINIT();
    Usb_reInitFs(AK_TRUE);
    
    Fwl_Print(C3, M_USB, "usb disk stop!");

#ifdef DEBUG_OUTPUT_USB
    //在退出U盘模式后初始化usbcdc
    mini_delay(200);//这个延迟是必要的
    console_init(uiUART0, CONSOLE_USB, UART_BAUD_115200);
    mini_delay(100);//这个延迟是必要的
#endif
}

static T_VOID Usb_CheckStateTimer(T_TIMER timer_id, T_U32 delay)
{
    T_SYS_MAILBOX mailbox;
    T_U8 curStatus = 0;    

    //check usb state,if usb is error,will stop usb disk.
    curStatus = usb_slave_getstate();
    if ((USB_ERROR == curStatus) || (USB_NOTUSE == curStatus) \
        || (USB_SUSPEND == curStatus) || (USB_START_STOP == curStatus) \
        || (USB_TEST_UNIT_STOP == curStatus) || (!usb_is_connected()))
    {
        Fwl_Print(C2, M_USB, "U-DISK Stop,Status = %d",curStatus);

        mailbox.event = SYS_EVT_USB_PLUG;
        mailbox.param.c.Param1 = EVT_USB_PLUG_OUT;
        AK_PostUniqueEvent( &mailbox, AK_NULL);
        return;
    }
/*
    //check sd state
    USB_LOCK();
    Usb_SdDectect();
    USB_UNLOCK();
*/    
    USB_LOCK();
    Usb_MMCDectect();
    USB_UNLOCK();

    //flush disk
    if (UsbFlush.Usb_NeedFlush)
    {
        USB_LOCK();
        Usb_FlushAllDisk(AK_FALSE);
        USB_UNLOCK();
    }
}


static T_BOOL FillInquiry(T_LUN_INFO * stpLunInfo, T_ST_INQUIRY * stpInquiry);

static T_BOOL Usb_ChangeLUN(T_ACCESS_MEDIUM disk_type,E_SENSESTATUS status)
{
    T_LUN_INFO    ChgLun;
    T_BOOL addRet = AK_FALSE;
    T_DRIVER_INFO DriverInfo;
    T_U8  DrvId;
    T_U8  SdFristId = 0;
    T_U8  SdDrvCnt = 0;
    
    if (NANDFLASH_DISK == disk_type)
       {
        ChgLun.LunAddInfo = (T_U32)Usb_GetDiskMedium(disk_type);
        
        if (ChgLun.LunAddInfo)
        {
            Fwl_Print(C3, M_USB, "[USB]ChLUN <type = %d status = %d>",disk_type,status);
        }
        else
        {
            Fwl_Print(C2, M_USB, "[USB]Need Not ChLUN!");
            return AK_FALSE;
        }
    }
    
    /* index of this LUN(logic unit number),it is must defferent for every LUN */
    ChgLun.LunIdx = (T_U32)disk_type;
    ChgLun.sense = status;
    ChgLun.reserved = 0;
    ChgLun.uAttribute = 0;
    /* disk accses information */
    switch (disk_type)
    {
    case  SDCARD_DISK:
        FillInquiry(&ChgLun, &Sd_inquiry);
        ChgLun.FastBlkCnt = 64;//8 block(4k is best for Sd accessing) 
        ChgLun.Read = Usb_SdMediumRead;
        ChgLun.Write = Usb_SdMediumWrite;
        addRet = AK_FALSE;
        
        Fwl_Print(C3, M_USB, "SDCARD_DISK");

        if (sd_is_connected() && Fwl_Sd_GetMountInfo(eSD_INTERFACE_SDIO, &SdFristId, &SdDrvCnt))
        {
            for (DrvId=SdFristId; DrvId<SdFristId+SdDrvCnt; DrvId++)
            {
                if (FS_GetDriver(&DriverInfo, DrvId))
                {
                    if (SD_IS_USR(DriverInfo))
                    {
                        //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                        ChgLun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                        ChgLun.BlkCnt       = DriverInfo.nBlkCnt;
                        ChgLun.BlkSize    = DriverInfo.nBlkSize;
                                                    
                        Usb_AddMediumInfo((T_PMEDIUM)ChgLun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                        Usb_UnInstallDriver(disk_type);
                        addRet = usbdisk_changeLUN(&ChgLun);

                        Fwl_Print(C3, M_USB, "[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,ChgLun.LunAddInfo);
                        Fwl_Print(C3, M_USB, "[UsbOpeNd]BlkCnt[%d] BlkSize[%d] addRet[%d]",ChgLun.BlkCnt, ChgLun.BlkSize, addRet);
                    }
                }
            }
        }

        if (!addRet)
        {
            addRet = usbdisk_changeLUN(&ChgLun);
        }
        break;
    case  MMC_DISK:
        FillInquiry(&ChgLun, &Mmc_inquiry);
        ChgLun.FastBlkCnt = 64;//8 block(4k is best for Sd accessing) 
        ChgLun.Read = Usb_MMCMediumRead;
        ChgLun.Write = Usb_MMCMediumWrite;
        addRet = AK_FALSE;

        Fwl_Print(C3, M_USB, "MMC_DISK");

        if (mmc_is_connected() && Fwl_Sd_GetMountInfo(eSD_INTERFACE_SDMMC, &SdFristId, &SdDrvCnt))
        {
            for (DrvId=SdFristId; DrvId<SdFristId+SdDrvCnt; DrvId++)
            {
                if (FS_GetDriver(&DriverInfo, DrvId))
                {
                    if (SD_IS_USR(DriverInfo))
                    {
                        //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                        ChgLun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                        ChgLun.BlkCnt       = DriverInfo.nBlkCnt;
                        ChgLun.BlkSize    = DriverInfo.nBlkSize;
                                                    
                        Usb_AddMediumInfo((T_PMEDIUM)ChgLun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                        Usb_UnInstallDriver(disk_type);
                        addRet = usbdisk_changeLUN(&ChgLun);

                        Fwl_Print(C3, M_USB, "[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,ChgLun.LunAddInfo);
                        Fwl_Print(C3, M_USB, "[UsbOpeNd]BlkCnt[%d] BlkSize[%d] addRet[%d]",ChgLun.BlkCnt, ChgLun.BlkSize, addRet);
                    }
                }
            }
        }

        if (!addRet)
        {
            addRet = usbdisk_changeLUN(&ChgLun);
        }
        break;

    case  NANDFLASH_DISK:
         break;
    case  NANDRESERVE_ZONE:
        break;
#ifdef DUMMY_DISK
        case DUMMYDISK:
            break;
#endif

    default:
        Fwl_Print(C2, M_FWL, "\r\n!!!!this medium not support,Usb_ChangeLUN fail!!!!");
        break;
    }

    return addRet;
}

static T_BOOL FillInquiry(T_LUN_INFO * stpLunInfo, T_ST_INQUIRY * stpInquiry)
{
    T_U32  uLen;

    if((!stpLunInfo) || (!stpInquiry))
    {
        return AK_FALSE;
    }

    uLen = strlen(stpInquiry->pVendor);    
    uLen = sizeof(stpLunInfo->Vendor) > uLen ? uLen : sizeof(stpLunInfo->Vendor);
    memset(stpLunInfo->Vendor,' ',sizeof(stpLunInfo->Vendor));
    memcpy(stpLunInfo->Vendor,stpInquiry->pVendor,uLen);


    uLen = strlen(stpInquiry->pProduct);    
    uLen = sizeof(stpLunInfo->Product) > uLen ? uLen : sizeof(stpLunInfo->Product);
    memset(stpLunInfo->Product,' ',sizeof(stpLunInfo->Product));
    memcpy(stpLunInfo->Product,stpInquiry->pProduct,uLen);


    uLen = strlen(stpInquiry->pRevision);    
    uLen = sizeof(stpLunInfo->Revision) > uLen ? uLen : sizeof(stpLunInfo->Revision);
    memset(stpLunInfo->Revision,' ',sizeof(stpLunInfo->Revision));
    memcpy(stpLunInfo->Revision,stpInquiry->pRevision,uLen);

    return AK_TRUE;
    
}
static T_BOOL Usb_MountLUN(T_ACCESS_MEDIUM disk_type)
{
    T_LUN_INFO    add_lun;
    T_DRIVER_INFO DriverInfo;
    T_U8  DrvId;
    T_U8  SdFristId = 0;
    T_U8  SdDrvCnt = 0;
    T_BOOL Ret,addRet = AK_FALSE;
    
    /* index of this LUN(logic unit number),it is must defferent for every LUN */
    add_lun.LunIdx = (T_U32)disk_type;
    add_lun.sense = MEDIUM_NOSENSE;
    add_lun.reserved = 0;
    add_lun.uAttribute = 0;
    /* disk accses information */
    switch (disk_type)
    {
    case  SDCARD_DISK:
        FillInquiry(&add_lun, &Sd_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_SdMediumRead;
        add_lun.Write = Usb_SdMediumWrite;
        add_lun.sense = MEDIUM_NOSENSE;
        addRet = AK_FALSE;

        if (sd_is_connected() && Fwl_Sd_GetMountInfo(eSD_INTERFACE_SDIO, &SdFristId, &SdDrvCnt))
        {
            for (DrvId=SdFristId; DrvId<SdFristId+SdDrvCnt; DrvId++)
            {
                if (FS_GetDriver(&DriverInfo, DrvId))
                {
                    if (SD_IS_USR(DriverInfo))
                    {
                        //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                        add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                        add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                        add_lun.BlkSize    = DriverInfo.nBlkSize;
                        
                        Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                        Usb_UnInstallDriver(disk_type);
                        addRet = usbdisk_addLUN(&add_lun);

                        Fwl_Print(C3, M_USB, "[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,add_lun.LunAddInfo);
                        Fwl_Print(C3, M_USB, "[UsbOpeNd]BlkCnt[%d] BlkSize[%d] addRet[%d]",add_lun.BlkCnt, add_lun.BlkSize, addRet);
                    }
                }
            }
        }

        if (!addRet) //挂空盘
        {
            add_lun.sense = MEDIUM_NOTPRESENT;
            addRet = usbdisk_addLUN(&add_lun);
        }
        break;
    case  MMC_DISK:
        FillInquiry(&add_lun, &Mmc_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_MMCMediumRead;
        add_lun.Write = Usb_MMCMediumWrite;
        add_lun.sense = MEDIUM_NOSENSE;
        addRet = AK_FALSE;

        if (mmc_is_connected() && Fwl_Sd_GetMountInfo(eSD_INTERFACE_SDMMC, &SdFristId, &SdDrvCnt))
        {
            for (DrvId=SdFristId; DrvId<SdFristId+SdDrvCnt; DrvId++)
            {
                if (FS_GetDriver(&DriverInfo, DrvId))
                {
                    if (SD_IS_USR(DriverInfo))
                    {
                        //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                        add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                        add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                        add_lun.BlkSize    = DriverInfo.nBlkSize;
                        
                        Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                        Usb_UnInstallDriver(disk_type);
                        addRet = usbdisk_addLUN(&add_lun);

                        Fwl_Print(C3, M_USB, "[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,add_lun.LunAddInfo);
                        Fwl_Print(C3, M_USB, "[UsbOpeNd]BlkCnt[%d] BlkSize[%d] addRet[%d]",add_lun.BlkCnt, add_lun.BlkSize, addRet);
                    }
                }
            }
        }

        if (!addRet) //挂空盘
        {
            add_lun.sense = MEDIUM_NOTPRESENT;
            addRet = usbdisk_addLUN(&add_lun);
        }
        break;
        
    case  NANDFLASH_DISK:
        FillInquiry(&add_lun, &NdOpn_inquiry);
        add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
        add_lun.Read = Usb_NandRead;
        add_lun.Write = Usb_NandWrite;
        
        Ret = FS_GetFirstDriver(&DriverInfo);
        while(Ret)
        {
            addRet = AK_FALSE;
            if (NAND_IS_USR(DriverInfo))
            {
                //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt     = Usb_GetPartitionSecCnt(DriverInfo.medium);
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                                
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                Usb_UnInstallDriver(disk_type);
                addRet = usbdisk_addLUN(&add_lun);

                Fwl_Print(C3, M_USB, "[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,add_lun.LunAddInfo);
                Fwl_Print(C3, M_USB, "[UsbOpeNd]BlkCnt = %d BlkSize = %d",add_lun.BlkCnt, add_lun.BlkSize);
            }    
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }
        break;

    case  NANDRESERVE_ZONE:
        FillInquiry(&add_lun, &NdRes_inquiry);
        add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
        add_lun.Read = Usb_ResNdRead;
        add_lun.Write = Usb_ResNdWrite;
        
        Ret = FS_GetFirstDriver(&DriverInfo);
        while(Ret)
        {
            addRet = AK_FALSE;
            if (NAND_IS_SYS(DriverInfo))
            { 
                //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                                    
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                Usb_UnInstallDriver(disk_type);
                addRet = usbdisk_addLUN(&add_lun);
                
                Fwl_Print(C3, M_USB, "[UsbResNd]Drv[%d] LunAddInfo[0x%x]",DriverInfo.DriverID,add_lun.LunAddInfo);
                Fwl_Print(C3, M_USB, "[UsbResNd]BlkCnt = %d BlkSize = %d",add_lun.BlkCnt, add_lun.BlkSize);
            }
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }
        break;
        
#ifdef DUMMY_DISK
        case DUMMYDISK:
            if(dummyram == AK_NULL)
                dummyram = (T_U32 *)Fwl_Malloc(DUMMYDISKSIZE);
            if(dummyram == AK_NULL)
                Fwl_Print(C2,M_FS, "+++++++++++++++++++malloc dummy ram error+++++++++++++++++");
          

            add_lun.Read = DUMMY_Read;
            add_lun.Write = DUMMY_Write;
            add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
            
            FillInquiry(&add_lun, &Default_inquiry);
            
            add_lun.LunAddInfo = 0;
            add_lun.BlkCnt = DUMMYDISKSIZE/512;
            add_lun.BlkSize = 512;
            usbdisk_addLUN(&add_lun);
            break;
#endif
        default:
            Fwl_Print(C2, M_FWL, "\r\n!!!!this medium not support,mount fail!!!!");
        return AK_FALSE;
        break;
    }
    
    return AK_TRUE;
}


//*************************For Usb *********************************
static T_BOOL Usb_MediumRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    //USB_INFO("R %x %x %x ", LunAddInfo, BlkAddr, BlkCnt);
    
    if((AK_NULL != (T_PMEDIUM)LunAddInfo) \
        && (NF_SUCCESS == ((T_PMEDIUM)LunAddInfo)->read((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt)))
    {
        //putch('S');
        return AK_TRUE;
    }
    else
    {
        //putch('F');
        return AK_FALSE;
    }
}



static T_BOOL Usb_MediumWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    //USB_INFO("W %x %x %x ", LunAddInfo, BlkAddr, BlkCnt);
    
    if((AK_NULL != (T_PMEDIUM)LunAddInfo) \
        && (NF_SUCCESS == ((T_PMEDIUM)LunAddInfo)->write((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt)))
    {
        //putch('S');
        return AK_TRUE;
    }
    else
    {
        //putch('F');
        return AK_FALSE;
    }
}

//********************************************************************
#ifdef DUMMY_DISK

static T_BOOL DUMMY_Write(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_PMEDIUM medium = AK_NULL;

    if(BlkCnt != 1)
    {
        Fwl_Print(C3, M_FWL, "nand write cnt != 1,continue");
    }

    memcpy((T_U8 *)dummyram + (BlkAddr * 512), buf, BlkCnt * 512);

    return AK_TRUE;
}
//********************************************************************
static T_BOOL DUMMY_Read(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    T_PMEDIUM medium = AK_NULL;

    if(BlkCnt != 1)
    {
        Fwl_Print(C3, M_FWL, "nand read cnt != 1,continue");
    }
    
    memcpy(buf, (T_U8 *)dummyram + (BlkAddr * 512), BlkCnt * 512);

    return AK_TRUE;
}

#endif

static T_eUSB_MODE m_usbMode = USB_NULL_MODE;
T_VOID Fwl_UsbSetMode(T_eUSB_MODE usbMode)
{
	m_usbMode = usbMode;
}

T_eUSB_MODE Fwl_UsbGetMode()
{
	return m_usbMode;
}

static T_eUSB_DEAl_PROC m_dealProc = USB_DEAL_END;

T_VOID Fwl_UsbSetDealProc(T_eUSB_DEAl_PROC proc)
{
	m_dealProc = proc;
}

T_eUSB_DEAl_PROC Fwl_UsbGetDealProc()
{
	return m_dealProc;
}

// end of file


