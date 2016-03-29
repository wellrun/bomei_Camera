/*
 * @(#)sd.c
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include <string.h>
#include "drv_api.h"
#include "mount_pub_api.h"
#include "mem_api.h"
#include "fs.h"
#include "Fwl_sd.h"
#include "fs.h"
#include "Mount.h"
#include "Akos_api.h"
#include "gpio_config.h"

#ifdef OS_WIN32
#include <windows.h>
#include "driver.h"
#endif

#define FHA_SUCCESS 1
#define FHA_FAIL    0

#define SD_INIT_MAX_TRY_TIMES      5

typedef struct
{
    T_pCARD_HANDLE     gCardHwHandle;
    T_PARTITION_INFO gMntInfo;
    T_BOOL           bIsInit;
    T_BOOL           bIsMount;
}T_SD_PARM,*T_pSD_PARM;

static T_SD_PARM           m_SdMmcParm = {0};
static T_SD_PARM        m_SdioParm = {0};

extern T_VOID AkDebugOutput(const T_U8 *s, ...);
extern T_BOOL mmc_is_connected(T_VOID);
extern T_BOOL sd_is_connected(T_VOID);

static T_pSD_PARM Sd_GetCardParm(T_eINTERFACE_TYPE type);
static T_BOOL Sd_GetCardInfo(T_eINTERFACE_TYPE type, T_U32 *disk_BlkNum,T_U32 *disk_BlkSize);
static T_U32 Sd_SdioRead(T_PMEDIUM medium, T_U8 *buf, T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdioWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdMmcRead(T_PMEDIUM medium, T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdMmcWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt);


#ifdef OS_WIN32
static T_U8 SDDisk_No = 'Y';
T_U8 SD_DriverNo = (T_U8)'S';

static T_VOID Sd_SetEmulate(T_U8 DriverNo);

T_BOOL Sd_Flush(T_PMEDIUM medium)
{
    return AK_TRUE;
}

static T_VOID  Sd_SetEmulate(T_U8 DriverNo)
{
    SDDisk_No = DriverNo;
}

T_VOID FS_Sd_SetEmulate(T_U8 DriverNo)
{
    if (DriverNo < 'S')
    {
        AkDebugOutput("set driverno can not less than 'S'\n");
        DriverNo = 'S';
    }
    else if(DriverNo >= 'Z')
    {
        AkDebugOutput("set driverno must use BIG char\n");
        while(1);
    }
    else
    {
        SD_DriverNo = DriverNo;
    }
}

T_PMEDIUM FS_Sd_Initial(T_VOID)
{
    T_U32 capacity, BytsPerSec, i;
    T_U32 driver;
    T_PMEDIUM medium;

    capacity = 200 * 1024;
    BytsPerSec = 512;

    medium = (T_PMEDIUM)Fwl_Malloc(sizeof(T_MEDIUM));
    if(medium == AK_NULL)
    {
        return AK_NULL;
    }

    i = 0;
    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }
    medium->SecBit = (T_U8) i;
    medium->PageBit = (T_U8) i;
    medium->SecPerPg = 0;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = Sd_SdioRead;
    medium->write = Sd_SdioWrite;
    medium->flush = Sd_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_SD;
    medium->msg = AK_NULL;

    driver = Driver_Initial(medium, DRIVER_BUFFER_LEN);
        
    if(0 != driver)
    {
        Global_MountDriver(driver, 4);
    }
    
    return medium;
}

#endif //end of #ifdef OS_WIN32

static T_pSD_PARM Sd_GetCardParm(T_eINTERFACE_TYPE type)
{
    T_pSD_PARM pParm = AK_NULL;

    if (eSD_INTERFACE_COUNT <= type)
    {
        return AK_NULL;
    }
    
    if (eSD_INTERFACE_SDMMC == type) //sdmmc interface
    {
        pParm = &m_SdMmcParm;
    }
    else //sdio interface
    {
        pParm = &m_SdioParm;
    }
    
    return pParm;
}

static T_BOOL Sd_GetSocketState(T_eINTERFACE_TYPE type)
{
    T_BOOL ret = AK_FALSE;
    
    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Sd_HwInit fail,IF type is %d\n",type);
        return AK_FALSE;
    }

    if (eSD_INTERFACE_SDMMC == type)
    {
        ret = mmc_is_connected();
    }
    else
    {
        ret = sd_is_connected();
    }

    return ret;
}

static T_BOOL Sd_GetCardInfo(T_eINTERFACE_TYPE type, T_U32 *disk_BlkNum, T_U32 *disk_BlkSize)
{
    T_pSD_PARM CardParm = AK_NULL;

    if ((eSD_INTERFACE_COUNT <= type) || (AK_NULL == disk_BlkNum) || (AK_NULL == disk_BlkSize))
    {
        return AK_FALSE;
    }
        
    CardParm = Sd_GetCardParm(type);

    if ((AK_NULL != CardParm) && (AK_NULL != CardParm->gCardHwHandle))
    {
        sd_get_info(CardParm->gCardHwHandle, disk_BlkNum, disk_BlkSize);
        return AK_TRUE;
    }
    else
    {
        AkDebugOutput("Sd_GetCardInfo fail interface type :%d\n",type);
        return AK_FALSE;
    }
}

//read sector function for sd.
static T_U32 Sd_SdioRead(T_PMEDIUM medium, T_U8 *buf, T_U32 BlkAddr, T_U32 BlkCnt)
{
#ifdef OS_WIN32    
    HANDLE hDev;
    T_U32 dwCB;
    T_S32 high;
    T_BOOL bRet;    
    char devName[10];    //"\\\\.\\g:";
    T_U8 *buf1;

    T_U32 ret = 0;
    
    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = SD_DriverNo;
    devName[5] = ':';
    devName[6] = 0;    
    hDev = CreateFile(devName, GENERIC_READ , FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    high = BlkAddr >> (32 - 9);
    SetFilePointer(hDev, BlkAddr << 9, &high, FILE_BEGIN);

    buf1 = (T_U8 *)malloc(512 * BlkCnt);
    bRet = ReadFile(hDev, buf1, 512 * BlkCnt, &dwCB, NULL);
    CloseHandle(hDev);
    if (bRet)
    {
        ret = BlkCnt;;
    }
    memcpy(buf, buf1, 512 * BlkCnt);
    free(buf1);

    return ret;
#else
    T_U32 ret=0;
    T_pSD_PARM CardParm;

    CardParm = Sd_GetCardParm(eSD_INTERFACE_SDIO);

    if ((AK_NULL != CardParm) && (AK_NULL != CardParm->gCardHwHandle))
    {
        if (sd_read_block(CardParm->gCardHwHandle, BlkAddr, buf, BlkCnt))
        {
            ret = BlkCnt;
        }
        else
        {
            if (!sd_is_connected())
            {
                AkDebugOutput("R.Er:SDIO is disconnected\n");
            }
            else
            {
                AkDebugOutput("R.Er,card Interface is SDIO\n");
            }
        }
    }
    else
    {
        AkDebugOutput("Read fail:SDIO card handle is null\n");
    }

    return ret;
#endif
}

//write sector function for sd.
static T_U32 Sd_SdioWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt)
{    
#ifdef OS_WIN32    
    DWORD dwCB;
    T_S32 high;
    HANDLE hDev;    
    char devName[10];    //"\\\\.\\g:";
    T_U8 *buf1;

    T_U32 ret = 0;
    
    devName[0] = '\\';
    devName[1] = '\\';
    devName[2] = '.';
    devName[3] = '\\';
    devName[4] = SD_DriverNo;
    devName[5] = ':';
    devName[6] = 0;    

    hDev = CreateFile(devName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDev == INVALID_HANDLE_VALUE) return 0;
    high = BlkAddr >> (32 - 9);
    SetFilePointer(hDev, BlkAddr << 9, &high, FILE_BEGIN);    

    buf1 = (T_U8 *)malloc(512 * BlkCnt);
    memcpy(buf1, buf, 512 * BlkCnt);
    if (WriteFile(hDev, buf1, 512 * BlkCnt, &dwCB, NULL))
    {
        ret = BlkCnt;
    }
    CloseHandle(hDev);        
    free(buf1);

    return 0;
#else
    T_U32 ret=0;
    T_pSD_PARM CardParm = AK_NULL;

    CardParm = Sd_GetCardParm(eSD_INTERFACE_SDIO);

    if ((AK_NULL != CardParm) && (AK_NULL != CardParm->gCardHwHandle))
    {
        if (sd_write_block(CardParm->gCardHwHandle, BlkAddr, buf, BlkCnt))
        {
            ret = BlkCnt;
        }
        else
        {
            if (!sd_is_connected())
            {
                AkDebugOutput("W.Er:SDIO is disconnected\n");
            }
            else
            {
                AkDebugOutput("W.Er.card Interface is SDIO\n");
            }
        }
    }
    else
    {
        AkDebugOutput("Write fail.SDIO card handle is null\n");
    }

    return ret;
#endif
}

static T_U32 Sd_SdMmcRead(T_PMEDIUM medium, T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt)
{
    T_U32 ret = 0;
#ifdef OS_ANYKA
    T_pSD_PARM CardParm;

    CardParm = Sd_GetCardParm(eSD_INTERFACE_SDMMC);

    if ((AK_NULL != CardParm) && (AK_NULL != CardParm->gCardHwHandle))
    {
        if (sd_read_block(CardParm->gCardHwHandle, BlkAddr, buf, BlkCnt))
        {
            ret = BlkCnt;
        }
        else
        {
            if (!mmc_is_connected())
            {
                AkDebugOutput("R.Er:SDMMC is disconnected\n");
            }
            else
            {
                AkDebugOutput("R.Er,card Interface is SDMMC\n");
            }
        }
    }
    else
    {
        AkDebugOutput("Read fail:SDMMC card handle is null\n");
    }
#endif
    return ret;
}

static T_U32 Sd_SdMmcWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt)
{
    T_U32 ret = 0;
#ifdef OS_ANYKA
    T_pSD_PARM CardParm = AK_NULL;

    CardParm = Sd_GetCardParm(eSD_INTERFACE_SDMMC);

    if ((AK_NULL != CardParm) && (AK_NULL != CardParm->gCardHwHandle))
    {
        if (sd_write_block(CardParm->gCardHwHandle, BlkAddr, buf, BlkCnt))
        {
            ret = BlkCnt;
        }
        else
        {
            if (!mmc_is_connected())
            {
                AkDebugOutput("W.Er:SDMMC is disconnected\n");
            }
            else
            {
                AkDebugOutput("W.Er.card Interface is SDMMC\n");
            }
        }
    }
    else
    {
        AkDebugOutput("Write fail.SDMMC card handle is null\n");
    }
#endif
    return ret;
}

T_BOOL Fwl_Sd_HwInit(T_eINTERFACE_TYPE type)
{
#ifdef OS_ANYKA
    T_pCARD_HANDLE sdHandle = AK_NULL;
    T_pSD_PARM CardParm = AK_NULL;
    T_eCARD_INTERFACE CardInterface;
    T_eBUS_MODE BusMode;
    T_U32 maxTryTimes = 0;
    T_BOOL CardSocketState = AK_FALSE;

    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Sd_HwInit fail,IF type is %d\n",type);
        return AK_FALSE;
    }
    CardParm = Sd_GetCardParm(type);
    if (AK_NULL == CardParm)
    {
        AkDebugOutput("Sd_HwInit:get card parm fail\n");
        return AK_FALSE;
    }
    
    if (CardParm->bIsInit)
    {
        AkDebugOutput("Sd_HwInit:had initial,no need ReInitial\n");
        return AK_TRUE;
    }

    if (eSD_INTERFACE_SDMMC == type)
    {
        CardInterface = INTERFACE_SDMMC8;
        BusMode = USE_FOUR_BUS;
    }
    else //interface type is sdio
    {
        CardInterface = INTERFACE_SDIO;
        BusMode = USE_FOUR_BUS;
    }
    
    while(maxTryTimes++ <= SD_INIT_MAX_TRY_TIMES)
    {        
        if (eSD_INTERFACE_SDMMC == type)
        {
            CardSocketState = mmc_is_connected();
        }
        else
        {
            CardSocketState = sd_is_connected();
        }
        
        if (!CardSocketState)
        {
            break;
        }
        
        sdHandle = sd_initial(CardInterface, BusMode);        
        if (AK_NULL == sdHandle)
        {
            AkDebugOutput("Sd_HwInit maxTryTimes: %d, delaytime: %d\n", maxTryTimes, 100*maxTryTimes);
            mini_delay(100*maxTryTimes);
        }
        else // initial succeed
        {
            break;
        }
    }

      if (AK_NULL == sdHandle)
      {
        AkDebugOutput("sd card initial error!!!\n");
        return AK_FALSE;
      }
    else
    {
        CardParm->gCardHwHandle = sdHandle;
        CardParm->bIsInit = AK_TRUE;
        return AK_TRUE;
    }
#endif
    return AK_FALSE;
}

T_BOOL Fwl_Sd_HwDestory(T_eINTERFACE_TYPE type)
{
    T_BOOL ret = 0;
#ifdef OS_ANYKA
    T_pSD_PARM CardParm = AK_NULL;

    if (eSD_INTERFACE_COUNT <= type)
    {
        return AK_FALSE;
    }

    CardParm = Sd_GetCardParm(type);
    
    if (AK_NULL != CardParm)
    {
        if (AK_NULL != CardParm->gCardHwHandle)
        {
            sd_free(CardParm->gCardHwHandle);
            CardParm->gCardHwHandle = AK_NULL;
            CardParm->bIsInit = AK_FALSE;
            mini_delay(10);
            ret = AK_TRUE;
        }
    }
#endif    
    return ret;
}

T_BOOL Fwl_Sd_Mount(T_eINTERFACE_TYPE type)
{
#ifdef OS_WIN32
    FS_Sd_Initial();
    return AK_TRUE;
#else
    T_pSD_PARM CardParm = AK_NULL;
    T_DRIVER_INFO driver;
    T_U32 capacity = 0;
    T_U32 BytsPerSec = 0;
    T_U8 firstNo = 0;
    T_U8 drvCnt = 0;    
    T_BOOL ret = AK_FALSE;

    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Fwl_Sd_Mount fail,IF type is %d\n",type);
        return AK_FALSE;
    }
    
    CardParm = Sd_GetCardParm(type);
    if (AK_NULL == CardParm)
    {
        AkDebugOutput("Sd type[%d] get parm fail\n",type);
        return AK_FALSE;
    }

    if (!CardParm->bIsInit)
    {
        AkDebugOutput("Sd type[%d] mount fail,sd is no initial\n",type);
        return AK_FALSE;
    }

    if (CardParm->bIsMount)
    {
        AkDebugOutput("Sd type[%d] is Mount,No need ReMount\n",type);
        return AK_TRUE;
    }

    ret = Sd_GetCardInfo(type, &capacity, &BytsPerSec);
    
    AkDebugOutput("Sd_GetCardInfo [%s]: capacity = %d BytsPerSec = %d\n", ret?"Ok.":"Failed!", capacity, BytsPerSec);

    if (ret)
    {
        driver.nBlkCnt     = capacity;
        driver.nBlkSize  = BytsPerSec;
        driver.nMainType = MEDIUM_SD;
        driver.nSubType  = USER_PARTITION;
        if (eSD_INTERFACE_SDMMC == type)
        {
            driver.fRead     = Sd_SdMmcRead;
            driver.fWrite     = Sd_SdMmcWrite;
        }
        else
        {
            driver.fRead     = Sd_SdioRead;
            driver.fWrite     = Sd_SdioWrite;
        }
         firstNo = FS_MountMemDev(&driver, &drvCnt, (T_U8)-1);
        mini_delay(20);
    }
    
    AkDebugOutput("[MntSd]:FirstDrvNo = %d Count = %d\n",firstNo,drvCnt);
    if (T_U8_MAX == firstNo)
    {        
        AkDebugOutput("SD[%d] Mount Fail\n", type);
        return AK_FALSE;
    }
    
    CardParm->bIsMount = AK_TRUE;
    CardParm->gMntInfo.firstDrvNo = firstNo;
    CardParm->gMntInfo.driverCnt  = drvCnt;
    return AK_TRUE;
#endif
}

T_BOOL Fwl_Sd_UnMount(T_eINTERFACE_TYPE type)
{
    T_pSD_PARM CardParm;
    T_U32 i;
    T_BOOL mntRet = AK_FALSE;

    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Fwl_Sd_UnMount fail,IF type is %d\n",type);
        return AK_FALSE;
    }

    CardParm = Sd_GetCardParm(type);
    if (AK_NULL == CardParm)
    {
        AkDebugOutput("Sd type[%d] get parm fail\n",type);
        return AK_FALSE;
    }

    if (!CardParm->bIsMount)
    {
        AkDebugOutput("Sd type[%d] not mnt, Need not UnMnt.\n",type);
        return AK_TRUE;
    }
    
    for (i = 0; i<CardParm->gMntInfo.driverCnt; i++)
    {
#ifdef OS_ANYKA
        mntRet = FS_UnMountMemDev(i + CardParm->gMntInfo.firstDrvNo);
#endif
        AkDebugOutput("SD UnMount:Card type:%d,driver ID:%d,UnMount State:%d\n",\
                                    type,i + CardParm->gMntInfo.firstDrvNo,mntRet);
    }
    
    CardParm->bIsMount = AK_FALSE;
    memset(&CardParm->gMntInfo,0,sizeof(T_PARTITION_INFO));
    
    AkDebugOutput("[SD]UnMnt Finish\n");
    return AK_TRUE;
}

T_BOOL Fwl_Sd_GetInitState(T_eINTERFACE_TYPE type)
{
    T_pSD_PARM CardParm;
    T_BOOL ret = AK_FALSE;
    
    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Fwl_Sd_GetInitState fail,IF type is %d\n",type);
        return AK_FALSE;
    }
    
    CardParm = Sd_GetCardParm(type);

    if (AK_NULL != CardParm)
    {
        ret = CardParm->bIsInit;
    }

    return ret;
}
T_eINTERFACE_TYPE Fwl_Sd_GetInterfaceByID(T_U8 drvId)
{    
    T_pSD_PARM CardParm = AK_NULL;
    T_U8 i,j;
    
    for(i=0; i<eSD_INTERFACE_COUNT; i++)
    {
        CardParm = Sd_GetCardParm(i);
        
        if (AK_NULL != CardParm)
        {
            for (j=0; j<CardParm->gMntInfo.driverCnt; j++)
            {
                if (drvId == (CardParm->gMntInfo.firstDrvNo + j))
                {
                    if (Sd_GetSocketState(i) && (CardParm->bIsMount))
                    {
                        return i;
                    }
                }
            }
        }
    }
    return eSD_INTERFACE_COUNT;
}

T_BOOL Fwl_Sd_GetMountInfo(T_eINTERFACE_TYPE type, T_U8*FristId, T_U8*DrvCnt)
{
    T_pSD_PARM CardParm;
    T_BOOL ret = AK_FALSE;
    
    if (eSD_INTERFACE_COUNT <= type)
    {
        AkDebugOutput("Fwl_Sd_GetInitState fail,IF type is %d\n",type);
        return AK_FALSE;
    }
    
    CardParm = Sd_GetCardParm(type);
    if (AK_NULL != CardParm)
    {
        if (CardParm->bIsMount)
        {
            *FristId = CardParm->gMntInfo.firstDrvNo;
            *DrvCnt = CardParm->gMntInfo.driverCnt;
            ret = AK_TRUE;
        }
    }

    return ret;
}

static T_BOOL Sd_Format(T_eINTERFACE_TYPE type)
{
    T_BOOL Ret = AK_FALSE;
    T_BOOL fmtRet = AK_FALSE;
#ifdef OS_ANYKA
    T_DRIVER_INFO DriverInfo;
    T_pSD_PARM  CardParm;
    T_U32 i;

    Ret = FS_GetFirstDriver(&DriverInfo);
    while(Ret)
    {
        if (SD_IS_USR(DriverInfo))
        {
            CardParm = Sd_GetCardParm(type);
            if (AK_NULL != CardParm)
            {
                if (DriverInfo.DriverID == CardParm->gMntInfo.firstDrvNo)
                {
                    for (i=0; i<CardParm->gMntInfo.driverCnt; i++)
                    {
                        fmtRet =  FS_FormatDriver(DriverInfo.DriverID+i, FAT_FS_ERROR);
                        AkDebugOutput("SD[%d] format %d\n",type,fmtRet);
                        if (!fmtRet)
                        {
                            break;
                        }
                    }
                }
            }
        }
        Ret = FS_GetNextDriver(&DriverInfo);
    }
#endif
    return fmtRet;
}

T_BOOL Fwl_Sd_DiskFormat(T_eINTERFACE_TYPE type)
{
    T_BOOL ret = AK_FALSE;
#ifdef OS_ANYKA
    switch(type)
    {
    case eSD_INTERFACE_SDMMC:
    case eSD_INTERFACE_SDIO:
        ret = Sd_Format(type);
        break;
    default:
        AkDebugOutput("Sd Format unknown type\n");
        break;
    }
    
    
#endif
    return ret;
}

T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage)
{
    return 0;  
}
 
T_U32 FHA_SDIO_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{  
    if(0 < Sd_SdioRead(AK_NULL, pData, nPage, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }  
}
 
T_U32 FHA_SDIO_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    if(0 < Sd_SdioWrite(AK_NULL, pData, nPage, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }    
}

T_U32 FHA_SDMMC_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{  
    if(0 < Sd_SdMmcRead(AK_NULL, pData, nPage, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }
}
 
T_U32 FHA_SDMMC_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    if(0 < Sd_SdMmcWrite(AK_NULL, pData, nPage, nDataLen))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }    
}

