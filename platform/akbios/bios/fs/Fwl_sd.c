/*
 * @(#)sd.c
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
#include <stdlib.h>
#include "drv_api.h"
#include "fs.h"
#include "Fwl_sd.h"
#include "Eng_Debug.h"
#include "fwl_osmalloc.h"
#include "Gbl_Global.h"
#include "fs.h"
#include "Mount.h"

#define SD_INIT_MAX_TRY_TIMES      5

typedef struct
{
    T_pCARD_HANDLE     gCardHwHandle;
    T_PARTITION_INFO gMntInfo;
    T_BOOL           bIsInit;
    T_BOOL           bIsMount;
}T_SD_PARM,*T_pSD_PARM;

static T_SD_PARM           m_SdMmcParm;
static T_SD_PARM        m_SdioParm;

static T_pSD_PARM Sd_GetCardParm(T_eINTERFACE_TYPE type);
static T_BOOL Sd_GetCardInfo(T_eINTERFACE_TYPE type, T_U32 *disk_BlkNum,T_U32 *disk_BlkSize);
static T_BOOL Sd_HwDestory(T_eINTERFACE_TYPE type);
static T_BOOL Sd_HwInit(T_eINTERFACE_TYPE type);
static T_U32 Sd_SdioRead(T_PMEDIUM medium, T_U8 *buf, T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdioWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdMmcRead(T_PMEDIUM medium, T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt);
static T_U32 Sd_SdMmcWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt);

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
        printf("Sd_GetCardInfo fail interface type :%d",type);
        return AK_FALSE;
    }
}

static T_BOOL Sd_HwDestory(T_eINTERFACE_TYPE type)
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
            mini_delay(10);
            ret = AK_TRUE;
        }
    }
#endif    
    return ret;
}

/*
 * this function should only be called once, maybe at the startup
 */
static T_BOOL Sd_HwInit(T_eINTERFACE_TYPE type)
{
    T_pCARD_HANDLE sdHandle = AK_NULL;
    T_pSD_PARM CardParm = AK_NULL;
    T_eCARD_INTERFACE CardInterface;
    T_eBUS_MODE BusMode;
    T_U32 maxTryTimes = 0;

    if (eSD_INTERFACE_COUNT <= type)
    {
        printf("Sd_HwInit fail,IF type is %d",type);
        return AK_FALSE;
    }

    CardParm = Sd_GetCardParm(type);

    if (AK_NULL == CardParm)
    {
        printf("Sd_HwInit:get card parm fail");
        return AK_FALSE;
    }

    if (CardParm->bIsMount) //has initaled ,no need to initial
    {
        printf("SD card has initial,return true!\n");
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
        sdHandle = sd_initial(CardInterface, BusMode);        

        if (AK_NULL == sdHandle)
        {
            printf("Sd_HwInit maxTryTimes: %d, delaytime: %d\n", maxTryTimes, 100*maxTryTimes);
            mini_delay(100*maxTryTimes);
        }
        else // initial succeed
        {
            break;
        }
    }

    CardParm->gCardHwHandle = sdHandle;
    CardParm->bIsInit = AK_TRUE;

      if (AK_NULL == sdHandle)
      {
        printf("sd card initial error!!!\n");
        return AK_FALSE;
      }
    else
    {
        return AK_TRUE;
    }
}
//read sector function for sd.
static T_U32 Sd_SdioRead(T_PMEDIUM medium, T_U8 *buf, T_U32 BlkAddr, T_U32 BlkCnt)
{
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
            printf("R.Er,card Interface is SDIO");
        }
    }
    else
    {
        printf("Read fail:SDIO card handle is null");
    }

    return ret;
}

//write sector function for sd.
static T_U32 Sd_SdioWrite(T_PMEDIUM medium, const T_U8* buf, T_U32 BlkAddr, T_U32 BlkCnt)
{    
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
            printf("W.Er.card Interface is SDIO");
        }
    }
    else
    {
        printf("Write fail.SDIO card handle is null");
    }

    return ret;

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
            printf("R.Er,card Interface is SDMMC");
        }
    }
    else
    {
        printf("Read fail:SDMMC card handle is null");
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
            printf("W.Er.card Interface is SDMMC");
        }
    }
    else
    {
        printf("Write fail.SDMMC card handle is null");
    }
#endif
    return ret;
}

T_BOOL Fwl_Sd_MountSdBootCard(T_VOID)
{
    T_pSD_PARM CardParm = AK_NULL;
    T_DRIVER_INFO driver;
    T_eINTERFACE_TYPE InterfaceType;
    T_U32 capacity = 0;
    T_U32 BytsPerSec = 0;
    T_U8 firstNo = 0;
    T_U8 drvCnt = 0;
    T_BOOL ret = AK_FALSE;
    
#if (defined (SDIOBOOT))
    InterfaceType = eSD_INTERFACE_SDIO;
#elif (defined (SDMMCBOOT))
    InterfaceType = eSD_INTERFACE_SDMMC;
#endif
        
    if (!Sd_HwInit(InterfaceType))
    {
        printf("Sd_HwInit fail\n");
        return AK_FALSE;
    }

    ret = Sd_GetCardInfo(InterfaceType, &capacity, &BytsPerSec);
            
    printf("Sd_GetCardInfo [%s]: capacity = %d BytsPerSec = %d\n", ret?"Ok.":"Failed!", capacity, BytsPerSec);
        
    if (ret)
    {
        driver.nBlkCnt     = capacity;
        driver.nBlkSize  = BytsPerSec;
        driver.nMainType = MEDIUM_SD;
        driver.nSubType  = USER_PARTITION;
        if (eSD_INTERFACE_SDMMC == InterfaceType)
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
    }
            
    printf("[MntSd]:FirstDrvNo = %d Count = %d",firstNo,drvCnt);
    if (0 == drvCnt)
    {
        Sd_HwDestory(InterfaceType);
        CardParm->bIsInit = AK_FALSE;
        return AK_FALSE;
    }
    
    CardParm = Sd_GetCardParm(InterfaceType);
    if (AK_NULL != CardParm)
    {
        CardParm->bIsMount = AK_TRUE;
        CardParm->gMntInfo.firstDrvNo = firstNo;
        CardParm->gMntInfo.driverCnt  = drvCnt;
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_BOOL Fwl_Sd_GetInitState(T_eINTERFACE_TYPE type)
{
    T_pSD_PARM CardParm;
    T_BOOL ret = AK_FALSE;
    
    if (eSD_INTERFACE_COUNT <= type)
    {
        printf("Fwl_Sd_GetInitState fail,IF type is %d\n",type);
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
                    if (CardParm->bIsMount)
                    {
                        return i;
                    }
                }
            }
        }
    }
    return eSD_INTERFACE_COUNT;
}

#endif //end of #ifndef NNAD_BOOT


