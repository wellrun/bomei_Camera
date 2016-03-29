/**
* @file ramtofla.h
* @brief Media access control
* This file provides all the APIs of accessing storage  media¡­
* 1.chage page/block address access to byte address access (ram flash disk)
* 2.change usb packet read/write(64 byte per packet) to page/block access
* 3.read the data from media to sdram buffer,or write the data from sdram buffer to meida
in buffer is
static T_U32 buffer_data_in[DEF_BUF32_IN_SIZE];
out buffer is
static T_U32 buffer_data_out1[DEF_BUF32_OUT_SIZE];
static T_U32 buffer_data_out2[DEF_BUF32_OUT_SIZE];

  * @Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
  * @author Deng Jian
  * @date 2005-08-05
  * @version 1.0
  * @ref Please refer to all the specification of FAT16
*/
#ifdef OS_ANYKA

#include "ramtofla.h"
#include "mount.h"
#include "hal_usb_s_disk.h"
#include "fwl_sd.h"

#define ERROR_PRINT printf
#define INFO_PRINT  printf




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
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
T_ST_INQUIRY  Sd_inquiry = 
{
    "Anyka",
    "SD card",
    "1.00"
};
T_ST_INQUIRY  MMC_inquiry = 
{
    "Anyka",
    "MMC card",
    "1.00"
};
struct_Medium_OptCnt Sd_OptCnt = {0};
struct_Medium_OptCnt mmc_OptCnt = {0};
#endif
struct_Medium_OptCnt nand_OptCnt = {0};
struct_Medium_OptCnt resNd_OptCnt = {0};

#define USB_MNT_MAX_NUM  8

typedef struct 
{
    T_ACCESS_MEDIUM type;
    T_PMEDIUM medium;
    T_U8 drvId;
    T_BOOL cachemode;
} T_USB_MEDIUM_INFO;


T_USB_MEDIUM_INFO UsbMediumInfo[USB_MNT_MAX_NUM];

T_VOID Usb_MediumInfoInit(T_VOID)
{
    T_U32 i = 0;
    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        UsbMediumInfo[i].medium = AK_NULL;
        UsbMediumInfo[i].type   = 0;
        UsbMediumInfo[i].drvId = 0;
        UsbMediumInfo[i].cachemode = AK_FALSE;
    }
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
    memset(&Sd_OptCnt, 0, sizeof(Sd_OptCnt));
    memset(&mmc_OptCnt, 0, sizeof(Sd_OptCnt));
#endif    
    memset(&nand_OptCnt, 0, sizeof(nand_OptCnt));
    memset(&resNd_OptCnt, 0, sizeof(resNd_OptCnt));
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
            return AK_TRUE;
        }
    }

    return AK_FALSE;
}


T_U8 Usb_FlushAllDisk(T_BOOL NeedFlush)
{
    T_U32 i = 0;
    T_PMEDIUM pMedium_tmp;
    T_BOOL isNeedFlush = AK_FALSE;
    T_U8 flshCnt = 0;
    T_ACCESS_MEDIUM type = 0;
    struct_Medium_OptCnt *pMediumOptCnt = AK_NULL;

    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if (AK_NULL != pMedium_tmp)
        {
            isNeedFlush = AK_FALSE;
            pMediumOptCnt = AK_NULL;
            type = UsbMediumInfo[i].type;
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))    
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
#endif
            if (NANDFLASH_DISK == type)//Nand Open
            {
                isNeedFlush = AK_TRUE;
                pMediumOptCnt = &nand_OptCnt;
            }

            else if (NANDRESERVE_ZONE == type)//Nand reserve
            {
                isNeedFlush = AK_TRUE;
                pMediumOptCnt = &resNd_OptCnt;
            }

            if ((isNeedFlush && (AK_NULL != pMediumOptCnt)) || NeedFlush)
            {
                if (((pMediumOptCnt->read + pMediumOptCnt->write) == 0) || NeedFlush)
                {
                    //printf("[%d]<",UsbMediumInfo[i].drvId);
                    pMedium_tmp->flush(pMedium_tmp);
                    flshCnt++;
                }
                pMediumOptCnt->read  = 0;//clear read count
                pMediumOptCnt->write = 0;//clear write count
            }
        }
    }
    return flshCnt;
}


static T_VOID Usb_DestroyCacheMedium(T_VOID)
{
    T_U32 i = 0;

    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        if ((AK_NULL != UsbMediumInfo[i].medium) && UsbMediumInfo[i].cachemode)
        {
            FS_DestroyCache(UsbMediumInfo[i].medium);
        }
    }
}

T_VOID Usb_reInitFs(T_BOOL isFlush)
{
#ifdef OS_ANYKA
   if (isFlush)
   {
       Usb_FlushAllDisk(AK_FALSE);
   }
   Usb_DestroyCacheMedium();
   Usb_InitFs();
   usbdisk_stop();
   usb_slave_set_state(USB_NOTUSE);
#endif
}

T_VOID Usb_InitFs(T_VOID)
{
#if 0
    FS_InstallDriver(0,26);
#else
    T_U32 i = 0;
    T_U8 drvId = 0;
    T_PMEDIUM pMedium_tmp;
    
    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if (AK_NULL != pMedium_tmp)
        {
            drvId = UsbMediumInfo[i].drvId;
            INFO_PRINT("[USB] InitFs Drv[%d]\n",drvId);
            FS_InstallDriver(drvId,1);
        }
    }
#endif
}

T_VOID Usb_DeInitFs(T_VOID)
{
    T_U32 i = 0;
    T_U8 drvId = 0;
    T_PMEDIUM pMedium_tmp;
    
    for (i=0;i<USB_MNT_MAX_NUM;i++)
    {
        pMedium_tmp = UsbMediumInfo[i].medium;
        if (AK_NULL != pMedium_tmp)
        {
            drvId = UsbMediumInfo[i].drvId;
            INFO_PRINT("[USB] DeInitFs Drv[%d]\n",drvId);
            FS_UnInstallDriver(drvId,1);
        }
    }
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
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
static T_BOOL Usb_sd_read(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    T_PMEDIUM medium;

    //printf("RA:%d, C:%d \r\n", BlkAddr, BlkCnt);
    medium = (T_PMEDIUM)LunAddInfo;
    if (AK_NULL != medium
        && medium->read((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt))
    //if((AK_NULL != (T_PMEDIUM)LunAddInfo) \
    //    && (Sd_Read((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt)))
    {
        Sd_OptCnt.read++;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_sd_write(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    T_PMEDIUM medium;

    //printf("WA:%d, C:%d \r\n", BlkAddr, BlkCnt);
    medium = (T_PMEDIUM)LunAddInfo;
    if (AK_NULL != medium
        && medium->write((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt))

    //if((AK_NULL != (T_PMEDIUM)LunAddInfo) \
    //    && (Sd_Write((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt)))
    {
        Sd_OptCnt.write++;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_MMC_read(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    T_PMEDIUM medium;

    medium = (T_PMEDIUM)LunAddInfo;
    if (AK_NULL != medium
        && medium->read((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt))
    {
        mmc_OptCnt.read++;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

static T_BOOL Usb_MMC_write(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 LunAddInfo)
{
    T_PMEDIUM medium;

    medium = (T_PMEDIUM)LunAddInfo;
    if (AK_NULL != medium
        && medium->write((T_PMEDIUM)LunAddInfo, buf, BlkAddr, BlkCnt))
    {
        mmc_OptCnt.write++;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

#endif

/**
* @brief: mount the access media with logic unit number,support 16 LUN
* Detail:here support 4 LUN,if need support more, chang define in usb_mass.c
* @author Deng Jian
* @date 2005-10-31
* @return  success=AK_TURE;
*/
T_BOOL usb_MountLUN (T_ACCESS_MEDIUM disk_type)
{
    T_LUN_INFO add_lun;
    T_DRIVER_INFO DriverInfo;
    T_BOOL Ret,addRet = AK_FALSE;
    T_U32 Dis_BlkCnt = 1;
    T_U32 Dis_BlkSize = 1;

    /* index of this LUN(logic unit number),it is must defferent for every LUN */
    add_lun.LunIdx = (T_U32)disk_type;
    
    /*sense and capacity information*/
    add_lun.sense = MEDIUM_NOSENSE;
    add_lun.BlkSize = 512;
    
    /* disk accses information */
    switch (disk_type)
    {
    case  SDCARD_DISK:
#if (defined (SDIOBOOT))
        printf("#########mount SDIOCAED_DISK######### \n");
                
        FillInquiry(&add_lun, &Sd_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_sd_read;
        add_lun.Write = Usb_sd_write;
        add_lun.sense = MEDIUM_NOSENSE;

        Ret = FS_GetFirstDriver(&DriverInfo);
        while(Ret)
        {
            addRet = AK_FALSE;
            if (SD_IS_USR(DriverInfo) && (eSD_INTERFACE_SDIO == Fwl_Sd_GetInterfaceByID(DriverInfo.DriverID)))
            {
                //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                
                INFO_PRINT("[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]\n",DriverInfo.DriverID,add_lun.LunAddInfo);
                INFO_PRINT("[UsbOpeNd]BlkCnt = %d BlkSize = %d\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                addRet = usbdisk_addLUN(&add_lun);
            }
            
    #ifdef _USB_DBG
            INFO_PRINT("[UsbOpnNd] <%s> Drv [%d] MainType[%x] SubType[%x]\n",\
                addRet?"Match":"Pass",DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
    #endif
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }
        
#elif (defined (SDMMCBOOT))
        printf("#########mount MMCCAED_DISK######### \n");
                
        FillInquiry(&add_lun, &MMC_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_MMC_read;
        add_lun.Write = Usb_MMC_write;
        add_lun.sense = MEDIUM_NOSENSE;

        Ret = FS_GetFirstDriver(&DriverInfo);
        while(Ret)
        {
            addRet = AK_FALSE;
            if (SD_IS_USR(DriverInfo) && ((eSD_INTERFACE_SDMMC == Fwl_Sd_GetInterfaceByID(DriverInfo.DriverID))))
            {
                //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                
                INFO_PRINT("[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]\n",DriverInfo.DriverID,add_lun.LunAddInfo);
                INFO_PRINT("[UsbOpeNd]BlkCnt = %d BlkSize = %d\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                addRet = usbdisk_addLUN(&add_lun);
            }
            
#ifdef _USB_DBG
            INFO_PRINT("[UsbOpnNd] <%s> Drv [%d] MainType[%x] SubType[%x]\n",\
                addRet?"Match":"Pass",DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
#endif
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }
            
#endif

        break;
#if (defined (SDIOBOOT))
    case  SDCAED_ZONE:
        printf("#########mount SDIOCAED_ZONE######### \n");
                
        FillInquiry(&add_lun, &Sd_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_sd_read;
        add_lun.Write = Usb_sd_write;
        add_lun.sense = MEDIUM_NOSENSE;

        Ret = FS_GetFirstDriver(&DriverInfo);
        /*
        if (!FS_GetDriver(&DriverInfo, 0))
        {
            printf("++++++++++++++++=Ret is turn\n");
            return AK_FALSE;
        }*/
        while(Ret)
        {
            printf("Ret is turn\n");
            addRet = AK_FALSE;
            if (SD_IS_SYS(DriverInfo) && (eSD_INTERFACE_SDIO == Fwl_Sd_GetInterfaceByID(DriverInfo.DriverID)))
            {
                add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                //add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                
                INFO_PRINT("[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]\n",DriverInfo.DriverID,add_lun.LunAddInfo);
                INFO_PRINT("[UsbOpeNd]BlkCnt = %d BlkSize = %d\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                addRet = usbdisk_addLUN(&add_lun);
            }
            
    #ifdef _USB_DBG
            INFO_PRINT("[UsbOpnNd] <%s> Drv [%d] MainType[%x] SubType[%x]\n",\
                addRet?"Match":"Pass",DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
    #endif
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }

        break;
        
#elif (defined (SDMMCBOOT))
    case  SDCAED_ZONE:
        printf("#########mount MMCCAED_ZONE######### \n");
                
        FillInquiry(&add_lun, &MMC_inquiry);
        add_lun.FastBlkCnt = 64;//64 block(32k is best for Sd accessing) 
        add_lun.Read = Usb_MMC_read;
        add_lun.Write = Usb_MMC_write;
        add_lun.sense = MEDIUM_NOSENSE;

        Ret = FS_GetFirstDriver(&DriverInfo);
        /*
        if (!FS_GetDriver(&DriverInfo, 0))
        {
            printf("++++++++++++++++=Ret is turn\n");
            return AK_FALSE;
        }*/
        while(Ret)
        {
            printf("Ret is turn\n");
            addRet = AK_FALSE;
            if (SD_IS_SYS(DriverInfo) && (eSD_INTERFACE_SDMMC == Fwl_Sd_GetInterfaceByID(DriverInfo.DriverID)))
            {
                add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                //add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                
                INFO_PRINT("[UsbOpeNd]Drv[%d] LunAddInfo[0x%x]\n",DriverInfo.DriverID,add_lun.LunAddInfo);
                INFO_PRINT("[UsbOpeNd]BlkCnt = %d BlkSize = %d\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                addRet = usbdisk_addLUN(&add_lun);
            }
            
#ifdef _USB_DBG
            INFO_PRINT("[UsbOpnNd] <%s> Drv [%d] MainType[%x] SubType[%x]\n",\
                addRet?"Match":"Pass",DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
#endif
            
            Ret = FS_GetNextDriver(&DriverInfo);
        }

        break;
#endif

    case  NANDFLASH_DISK:
        FillInquiry(&add_lun, &NdOpn_inquiry);
        add_lun.Read  = Nand_UsbNandRead;
        add_lun.Write = Nand_UsbNandWrite;
        add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
        add_lun.sense = MEDIUM_NOSENSE;
        
        Ret = FS_GetFirstDriver(&DriverInfo);
        INFO_PRINT("usbdisk_addLUN2 %s\n",Ret?"Begin..":"Error");
        while( AK_TRUE == Ret)
        {
            if (NAND_IS_USR(DriverInfo))
            {
                putch('&');
                
                //add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                INFO_PRINT("LunAddInfo= %d\r\n",add_lun.LunAddInfo);
                INFO_PRINT("BlkCnt= %d BlkSize=%x\r\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                INFO_PRINT("usbdisk_addLUN2\n");
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_TRUE);
                usbdisk_addLUN(&add_lun);
            }
            else
            {
                ERROR_PRINT("usbdisk_addLUN2 Pass Drv [%d] MainType[%x] SubType[%x]\n",\
                    DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
            }
            Ret = FS_GetNextDriver(&DriverInfo);
        }
        
        break;
    case  NANDRESERVE_ZONE:
        FillInquiry(&add_lun, &NdRes_inquiry);
        add_lun.Read = Nand_ResUsbNbRead;
        add_lun.Write = Nand_ResUsbNdWrite;
        add_lun.FastBlkCnt = 1;//(512 byte is best for Nandflash accessing)
        add_lun.sense = MEDIUM_NOSENSE;
        
        Ret = FS_GetFirstDriver(&DriverInfo);
        INFO_PRINT("usbdisk_addLUN3 %s\n",Ret?"Begin..":"Error");
        while( AK_TRUE == Ret)
        {
            if (NAND_IS_SYS(DriverInfo))
            {
                putch('#');
                
                add_lun.LunAddInfo = (T_U32)DriverInfo.medium;
                //add_lun.LunAddInfo = (T_U32)FS_CreateCache(DriverInfo.medium, 512*1024);//(T_U32)DriverInfo.medium;
                add_lun.BlkCnt       = DriverInfo.nBlkCnt;
                add_lun.BlkSize    = DriverInfo.nBlkSize;
                
                INFO_PRINT("LunAddInfo= %d\r\n",add_lun.LunAddInfo);
                INFO_PRINT("BlkCnt= %d BlkSize=%x\r\n",add_lun.BlkCnt, add_lun.BlkSize);
                    
                INFO_PRINT("usbdisk_addLUN3\n");
                Usb_AddMediumInfo((T_PMEDIUM)add_lun.LunAddInfo,DriverInfo.DriverID,disk_type,AK_FALSE);
                usbdisk_addLUN(&add_lun);
            }
            else
            {
                ERROR_PRINT("usbdisk_addLUN3 Pass Drv [%d] MainType[%x] SubType[%x]\n",\
                    DriverInfo.DriverID,DriverInfo.nMainType,DriverInfo.nSubType);
            }
            Ret = FS_GetNextDriver(&DriverInfo);
        }
        break;
    default:
        ERROR_PRINT("\r\n!!!!this medium not support,mount fail!!!!");
        return AK_FALSE;
        break;
    }
    return AK_TRUE;
}


#endif //OS_AKRTOS
