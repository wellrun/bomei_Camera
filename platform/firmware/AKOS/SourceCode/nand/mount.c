/**
*@Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
*@date    2006-10-16
*@author zhaojiahuan
**/
#include <string.h>
#include "mount_pub_api.h"
#include "mount.h"
#include "nandflash.h"
#include "raminit.h"
#include "mtdlib.h"
#include "Hal_usb_s_disk.h"
#include "fwl_nandflash.h"
#include "fha.h"
#include "fha_asa.h"
#include "drv_api.h"

#ifdef OS_WIN32
#include "nand_win32.h"
#include "gbl_global.h"
#endif

extern T_VOID AkDebugOutput(const T_U8 *s, ...);



#define MNT_INFO  AkDebugOutput

#ifdef OS_ANYKA
//***********************************************************************************
#ifdef NANDBOOT
T_PNANDFLASH  gNand_base = AK_NULL;
extern T_NAND_PHY_INFO *g_pNandInfo;
T_NAND_PHY_INFO g_FhaNandInfo;
#endif

//**************************************************************************//
static T_pVOID Fha_Malloc(T_U32 size)
{
    return (void *)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID Fha_Free(T_pVOID var)
{
   return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

//***********************************************************************************
#ifdef NANDBOOT
T_BOOL Nand_MountInit(T_VOID)
{
    T_U8 firstNo = 0;
    T_U8 drvCnt = 0;
    T_U32 temp_blk, total_blk, temp_total_bad = 0;

    if (!FHA_asa_scan(AK_TRUE))
    {
        MNT_INFO("NAND: FHA_asa_scan Failed\n");
        while(1);
    }

    total_blk = gNand_base->BlockPerPlane * gNand_base->PlaneCnt;
    MNT_INFO("total blk is %d\n", total_blk);
    for (temp_blk = 0; temp_blk < total_blk; temp_blk++)
    {
        if (AK_TRUE == Nand_IsBadBlock(gNand_base, 0, temp_blk))
        {
            //MNT_INFO("BB%d ", temp_blk);
            temp_total_bad++;
        }
    }
    MNT_INFO("\nBB end,total=%d\n", temp_total_bad);

    firstNo = FS_MountNandFlash(gNand_base, FHA_get_last_pos(), AK_NULL, &drvCnt);

    MNT_INFO("MountNand:FirstDrvNo = %d Count = %d\n",firstNo,drvCnt);

    if (0 == drvCnt)
    {
        MNT_INFO("NAND: init error FS_MountNandFlash\n");
        return AK_FALSE;
    }
    return AK_TRUE;
}

T_BOOL Fwl_FhaInit(T_VOID)
{
    T_U32   nFlag;
    T_FHA_INIT_INFO pInit = {0}; 
    T_FHA_LIB_CALLBACK pCB = {0}; 

    if (AK_NULL != gNand_base)
    {
        MNT_INFO("NAND: has been inited\n");
        return AK_TRUE;
    }
    
    gNand_base = NandFlash_Init();
    if(AK_NULL == gNand_base)
    {
        MNT_INFO("NAND:        init error gNand_base\n");
        while(1);
    }

    //T_FSCallback    fsGlbCb = {0};
    pInit.eAKChip = FHA_CHIP_37XX;
    pInit.eMedium = MEDIUM_NAND;
    pInit.eMode   = MODE_UPDATE;
    pInit.ePlatform = PLAT_SWORD;
    pInit.nBlockStep = 1;
    pInit.nChipCnt   = FHA_GetNandChipCnt();

    //nand init
    pCB.Erase = FHA_Nand_EraseBlock;
    pCB.Write = FHA_Nand_WritePage;
    pCB.Read  = FHA_Nand_ReadPage;
    pCB.ReadNandBytes = ASA_ReadBytes;

    pCB.RamAlloc = Fha_Malloc; 
    pCB.RamFree  = Fha_Free;
    pCB.MemSet   = memset;
    pCB.MemCpy   = memcpy;
    pCB.MemCmp   = memcmp;
    pCB.MemMov   = memmove;
    pCB.Printf   = AkDebugOutput;    
    
    //add g_FhaNandInfo for slc nand flash pagesize / 2
    nFlag = g_pNandInfo->flag;
    memcpy(&g_FhaNandInfo, g_pNandInfo, sizeof(g_FhaNandInfo));
    if (0 != (nFlag & FLAG_ENHANCE_SLC))
    {
         g_FhaNandInfo.page_per_blk >>= 1;
         MNT_INFO("MountNand:open slc, akos page_per_blk = %d\n",g_FhaNandInfo.page_per_blk);
    }
    return (FHA_SUCCESS == FHA_mount(&pInit, &pCB, &g_FhaNandInfo));

}
T_VOID Nand_DestoryFs(T_VOID)
{
    //Asa_DeInit();
    Fha_Free(gNand_base);
    gNand_base = AK_NULL;
}
#endif //#NANDBOOT
//********************************************************************
#endif // End of #ifdef OS_ANYKA

T_BOOL Fwl_Fha_SD_Init(T_eINTERFACE_TYPE type)
{
#ifdef OS_ANYKA
    T_FHA_INIT_INFO pInit = {0}; 
    T_FHA_LIB_CALLBACK pCB = {0}; 
    T_U8 PhyInfo[512] = {0};

    if (!Fwl_Sd_GetInitState(type))
    {
        MNT_INFO("%s has no init\n",(eSD_INTERFACE_SDIO == type) ? "SDIO":"SDMMC");
        return FHA_FAIL;
    }

    //T_FSCallback    fsGlbCb = {0};
    pInit.eAKChip = FHA_CHIP_37XX;
    pInit.eMedium = MEDIUM_EMMC;
    pInit.eMode   = MODE_UPDATE;
    pInit.ePlatform = PLAT_SWORD;
    pInit.nBlockStep = 1;
    pInit.nChipCnt   = 1;
 
    //sd init
    pCB.Erase = FHA_SD_Erase;
    if (eSD_INTERFACE_SDIO == type)
    {
        pCB.Write = FHA_SDIO_Write;
        pCB.Read  = FHA_SDIO_Read;
    }
    else
    {
        pCB.Write = FHA_SDMMC_Write;
        pCB.Read  = FHA_SDMMC_Read;
    }
    pCB.ReadNandBytes = AK_NULL;
 
    pCB.RamAlloc = Fha_Malloc; 
    pCB.RamFree  = Fha_Free;
    pCB.MemSet   = memset;
    pCB.MemCpy   = memcpy;
    pCB.MemCmp   = memcmp;
    pCB.MemMov   = memmove;
    pCB.Printf   = AkDebugOutput;    
    
    return (FHA_SUCCESS == FHA_mount(&pInit, &pCB, PhyInfo));
#else
    return AK_FALSE;
#endif
}

//********************************************************************

T_BOOL Fwl_MountNand(T_VOID)
{
#ifdef OS_WIN32
    InitWin32Nand();
#endif

#ifdef NANDBOOT
#ifdef OS_ANYKA
    if(!Nand_MountInit())
    {
        MNT_INFO("NAND: disk init error!\n");
        return AK_FALSE;
    }
#endif
#endif
    MNT_INFO("\nNAND: mount nandflash success\n");     
    return AK_TRUE;
}

T_U8 Nand_GetMediumSymbol(T_PMEDIUM medium)
{
    T_DRIVER_INFO DriverInfo;
    T_BOOL Ret;
    
    Ret = FS_GetFirstDriver(&DriverInfo);
    while( AK_TRUE == Ret)
    {
        if (medium == DriverInfo.medium)
        {
            return DriverInfo.DriverID;
        }
        Ret = FS_GetNextDriver(&DriverInfo);
    }
    return 0;
}

#ifdef NANDBOOT

T_BOOL CheckLibVersions(T_VOID)
{    
#ifdef OS_ANYKA
    typedef struct
    {
        T_U8 *pLibName;
        T_U8 *(*pVerFun)(T_VOID);
    }T_VERSION_INFO;

    T_VERSION_INFO version_info[] = 
    {
        {VER_NAME_DRV,  drvlib_version},
        {VER_NAME_FS,   FSLib_GetVersion},
        {VER_NAME_MOUNT,FS_GetVersion},
        {VER_NAME_MTD,  MtdLib_GetVersion},
        {VER_NAME_FHA,  FHA_get_version}   
    }; 
    
    T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
    T_U32 i;
    T_U32 uRet;

    for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
    {
        strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
        sizeof(Lib_version[i].lib_name));
        strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
        sizeof(Lib_version[i].lib_version));
        uRet = FHA_check_lib_version(&(Lib_version[i]));
        if(FHA_FAIL == uRet)
        {
            MNT_INFO("%s no mathc\n", Lib_version[i].lib_name);
            MNT_INFO("%s->%s\n", Lib_version[i].lib_name, 
            Lib_version[i].lib_version);

            return AK_FALSE;
        }
    }
#endif
    return AK_TRUE;

}
#endif



// End of File
