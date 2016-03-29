/**
*@Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
*@date    2006-10-16
*@author zhaojiahuan
**/
#include "mount.h"
#include "nandflash.h"
#include "mtdlib.h"
#include "Hal_usb_s_disk.h"
#include "fwl_nandflash.h"
#include "fha.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"

#ifdef OS_WIN32
#include "nand_win32.h"
#include "gbl_global.h"
#endif


#define MNT_INFO  printf

#ifdef OS_ANYKA
//***********************************************************************************

T_PNANDFLASH  gNand_base = AK_NULL;
extern T_NAND_PHY_INFO *g_pNandInfo;
T_NAND_PHY_INFO g_FhaNandInfo;

//extern struct_Medium_OptCnt Sd_OptCnt;
//extern struct_Medium_OptCnt mmc_OptCnt;
extern struct_Medium_OptCnt nand_OptCnt;
extern struct_Medium_OptCnt resNd_OptCnt ;


//**************************************************************************//

//***********************************************************************************

T_VOID Nand_MountInit(T_VOID)
{
	T_U8 firstNo = 0;
	T_U8 drvCnt = 0;

#ifdef BAD_BOLCK_SCAN	
	if (!FHA_asa_scan(AK_TRUE))
	{
		MNT_INFO("NAND: FHA_asa_scan Failed\n");
		while(1);
	}
#endif
#if 1   //list all bad blocks
	{
		T_U32 temp_blk, total_blk, temp_total_bad = 0;
	
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
	}
#endif

	firstNo = FS_MountNandFlash(gNand_base, FHA_get_last_pos(), AK_NULL, &drvCnt);

	MNT_INFO("MountNand:FirstDrvNo = %d Count = %d\n",firstNo,drvCnt);

}

T_BOOL Fwl_FhaInit(T_VOID)
{
  T_U32   nFlag;
	T_FHA_INIT_INFO pInit = {0}; 
	T_FHA_LIB_CALLBACK pCB = {0}; 

	if (AK_NULL != gNand_base)
	{
	    printf("NAND: has been inited\n");
		return AK_TRUE;
	}
	
	gNand_base = NandFlash_Init();
	if(AK_NULL == gNand_base)
	{
	    printf("NAND:		init error gNand_base\n");
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

    pCB.RamAlloc = Fwl_Malloc; 
    pCB.RamFree  = Fwl_Free;
    pCB.MemSet   = memset;
    pCB.MemCpy   = memcpy;
    pCB.MemCmp   = memcmp;
    pCB.MemMov   = memmove;
    pCB.Printf   = printf;	
	
    //add g_FhaNandInfo for slc nand flash pagesize / 2
    nFlag = g_pNandInfo->flag;
    memcpy(&g_FhaNandInfo, g_pNandInfo, sizeof(g_FhaNandInfo));
    if (0 != (nFlag & FLAG_ENHANCE_SLC))
    {
         g_FhaNandInfo.page_per_blk >>= 1;
         MNT_INFO("MountNand:bios open slc, page_per_blk = %d\n",g_FhaNandInfo.page_per_blk);
    }
    return (FHA_SUCCESS == FHA_mount(&pInit, &pCB, &g_FhaNandInfo));
}

//********************************************************************
#endif // End of #ifdef OS_ANYKA

T_U8 Nand_GetZoneIdByType(T_U8 zonetype)
{
	T_DRIVER_INFO DriverInfo;
	T_BOOL Ret;

	Ret = FS_GetFirstDriver(&DriverInfo);
	while( AK_TRUE == Ret)
	{
		if (zonetype == DriverInfo.nSubType) //DriverInfo.type//luheshan
		{
			return DriverInfo.DriverID;
		}
		Ret = FS_GetNextDriver(&DriverInfo);
	}

	return 0;
}


T_VOID Nand_DestoryFs()
{
#ifdef BAD_BOLCK_SCAN	
	//Asa_DeInit();
#endif
    Fwl_Free(gNand_base);
	gNand_base = AK_NULL;
}


T_BOOL Nand_UsbRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
	T_PMEDIUM medium = (T_PMEDIUM)NandAddInfo;

    //printf("rn %x %x %x ", medium, BlkAddr, BlkCnt);
    
    if((AK_NULL != medium) \
		&& (NF_SUCCESS == medium->read(medium, buf, BlkAddr, BlkCnt)))
    {
       // printf("s %x\n", buf[0]);
        return AK_TRUE;
    }
    else
    {
      //  putch('F');
        return AK_FALSE;
    }

}

T_BOOL Nand_UsbWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
	T_PMEDIUM medium = (T_PMEDIUM)NandAddInfo;

	//printf("wn %x %x %x ", medium, BlkAddr, BlkCnt);
	
	if((AK_NULL !=medium) \
		&& (NF_SUCCESS == medium->write(medium, buf, BlkAddr, BlkCnt)))
	{
		//printf("s %x\n", buf[0]);
		return AK_TRUE;
	}
	else
	{
		//putch('F');
		return AK_FALSE;
	}

}

T_BOOL Nand_UsbNandRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    nand_OptCnt.read++;
	return Nand_UsbRead(buf, BlkAddr, BlkCnt, NandAddInfo);
}

T_BOOL Nand_UsbNandWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
    nand_OptCnt.write++;
	return Nand_UsbWrite(buf, BlkAddr, BlkCnt, NandAddInfo);
}

T_BOOL Nand_ResUsbNbRead(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
	resNd_OptCnt.read++;
	return Nand_UsbRead(buf, BlkAddr, BlkCnt, NandAddInfo);
}

T_BOOL Nand_ResUsbNdWrite(T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt, T_U32 NandAddInfo)
{
	resNd_OptCnt.write++;
	return Nand_UsbWrite(buf, BlkAddr, BlkCnt, NandAddInfo);
}



// End of File
