/**
* @FILENAME nandflash.c
* @BRIEF    并口NAND FLASH控制器驱
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR   dengjian
* @MODIFY   zhaojiahuan   chenyanyan
* @DATE     2007-1-10
* @VERSION  1.0
* @REF      Please refer to…
* @NOTE     1.只支持目前市面上大量生产的samsung 和hynix的驱动（这样运行速度会快一些
*        果要支持比较特殊 的nandflash驱动，需要调整程序和宏定义
*       2. 多片选择的相关细节目前没有在驱动中体现，大多数都是针对单片flash编写。
*/
#ifdef OS_ANYKA

#include "anyka_types.h"

#include "nandflash.h"
#include "fwl_nandflash.h"
#include "fwl_nandflash_L.h"
#include "drv_api.h"
#include "nand_list.h"
#include "fha_asa.h"
#include "fha.h"
#include "mem_api.h"
#include <string.h>

#define NAND_DATA_SIZE_P512B      512
#define NAND_DATA_SIZE_P1KB       1024

#define NAND_PARITY_SIZE_MODE5    56    //56 Bytes parity data under 32 bit ecc
#define NAND_PARITY_SIZE_MODE4    42    //42 Bytes parity data under 24 bit ecc
#define NAND_PARITY_SIZE_MODE3    28    //28 Bytes parity data under 16 bit ecc
#define NAND_PARITY_SIZE_MODE2    21    //21 Bytes parity data under 12 bit ecc
#define NAND_PARITY_SIZE_MODE1    14    //14 Bytes parity data under  8 bit ecc
#define NAND_PARITY_SIZE_MODE0    7     //7  Bytes parity data under  4 bit ecc

//#define NF_TEST_API
//#define NF_TEST_DBG

/** @brief force write and force read flag */
#define AVAILABLE_FLAG                      0xFF
#define AVAILABLE_FLAG_OFFSET               2
#define BLOCK_INDEX_OFFSET                  0
#define LAST_BLOCK_FLAG                     0xFFEE

extern T_VOID AkDebugOutput(const T_U8 *s, ...);
#define printf AkDebugOutput

#define NF_INFO  printf

//#define NF_SHOW_CFG(base,member,fmt)AkDebugOutput("<CFG>%s ="fmt"\n",#member,base->member)
#define NF_SHOW_CFG(base,member,fmt)

//#define NF_API_VAL(var,fmt)AkDebugOutput("[NF] %s ="fmt"\n",#var,var)
#define NF_API_VAL(var,fmt)

//#define NF_API_DBG(tips)AkDebugOutput("[NF] %s (Line= %d)\n",tips,__LINE__)
//#define NF_API_DBG(tips) AkDebugOutput("[NF] %s\n",tips)
#define NF_API_DBG(tips) 

//#define NF_API_DBG_END(tips) AkDebugOutput("[NF] %s End\n",tips)
#define NF_API_DBG_END(tips) 
T_NAND_PHY_INFO *g_pNandInfo = AK_NULL;

static T_U32 m_true_block_per_die = 2048;
static T_U32 m_fake_block_per_die = 4096;

extern T_VOID AkDebugOutput(const T_U8 *s, ...);
extern T_U32  Fwl_GetBatteryVoltage(T_VOID);

//***********************************************************************************
static E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8* oob,T_U32 oob_len);
static E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8* oob,T_U32 oob_len);
static E_NANDERRORCODE Nand_CopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);
static E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

static E_NANDERRORCODE Nand_MultiWriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiReadSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiCopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);
static E_NANDERRORCODE Nand_MultiEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block);

static E_NANDERRORCODE Nand_ReadFlag_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 *spare_tbl,T_U32 oob_len);
static E_NANDERRORCODE Nand_WriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_ReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiWriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);
static E_NANDERRORCODE Nand_MultiReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len);

static E_NANDERRORCODE Nand_ReadSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page, T_U8 data[], T_U8* spare_tbl, T_U32 oob_len, T_U32 page_num);
static E_NANDERRORCODE Nand_WriteSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page, const T_U8 data[], T_U8* spare_tbl, T_U32 oob_len, T_U32 page_num);
static E_NANDERRORCODE Nand_EraseBlock_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block);

static E_NANDERRORCODE Nand_WriteFlag(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
static E_NANDERRORCODE Nand_ReadFlag(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
static T_BOOL Nand_SetBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

static T_VOID Nand_Config_Data(T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8* data, T_U32 datalen, T_U8 *oob, T_U32 oob_len, T_U8 ecctype);
static T_BOOL Nand_IsLowVoltage(T_VOID);

//static T_VOID Asa_SetBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block );
//static T_BOOL Asa_IsBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block );


#ifdef NF_TEST_API
static T_VOID NandFlash_Test(const T_PNANDFLASH Nand_base);
#endif

//judge whether nandflash is under low power supply, when we read a error spare value
//retrun AK_FALSE means normal power supply
//retrun AK_TURE means low power supply
static T_BOOL Nand_IsLowVoltage(T_VOID)
{
	T_U32 Bat_Value;
	Bat_Value = Fwl_GetBatteryVoltage();
	if (Bat_Value <= 3420)
	{
		NF_INFO("ERR:LHS:Nand_IsLowVoltage:%d\r\n",Bat_Value);
		return AK_TRUE;
	}

	return AK_FALSE;
}    

T_PNANDFLASH NandFlash_Init(T_VOID)
{
    extern T_U32 Image$$ER_RO$$Base;
    T_U32 i, ChipCnt = 0, dat_len;
    T_U32 nand_id_readout;  //real nand_id read out from physical nandflash
    T_PNandflash_Add pNand_Add;
    T_PNANDFLASH pNand_Info;

    T_U8 *src_start = &Image$$ER_RO$$Base;
    if((drv_get_chip_version() == CHIP_3771_L))
    {
        g_pNandInfo = (T_NAND_PHY_INFO *)(src_start + 36);
        
        FWL_Nand_Init(g_pNandInfo);
        pNand_Info = FWL_Nand_Init_MTD();

        pNand_Add = (T_PNandflash_Add)(&pNand_Info[1]);
        printf("g_pNandInfo:%.8x,p:%d,%d,%.8x,%.8x",g_pNandInfo,g_pNandInfo->row_cycle,
            g_pNandInfo->col_cycle,g_pNandInfo->flag,pNand_Add);
        pNand_Add->RowCycle      = g_pNandInfo->row_cycle;
        pNand_Add->ChipPos[0]    = 0;
        pNand_Add->ColCycle      = g_pNandInfo->col_cycle;
        pNand_Add->EccType       = (g_pNandInfo->flag >> 4) & 0xF;
        pNand_Add->PageSize      = pNand_Info->BytesPerSector;
        pNand_Add->PagesPerBlock = pNand_Info->PagePerBlock;
        return pNand_Info;
    }
    	
    pNand_Info = (T_PNANDFLASH)Fwl_Malloc(sizeof(T_NANDFLASH) + sizeof(T_Nandflash_Add));
	
    if(AK_NULL == pNand_Info)
    {
        NF_INFO( "no more memory!\n");
        while(1);
    }
	memset(pNand_Info, 0, (sizeof(T_NANDFLASH) + sizeof(T_Nandflash_Add)));

    nand_HWinit();

    pNand_Add = (T_PNandflash_Add)(&pNand_Info[1]);

    for( i = 0; i < NFC_SUPPORT_CHIPNUM; i++ )
    {
        pNand_Add->ChipPos[i] = 0xFF; //init for no connect flash
	}

    g_pNandInfo = (T_NAND_PHY_INFO *)(src_start + 36);
    NF_INFO("nand_id of nandboot=0x%x\r\n", g_pNandInfo->chip_id);    

    nand_id_readout = nand_read_chipID(0);
    NF_INFO("chip 0 id=0x%x\r\n", nand_id_readout);

    if (ERROR_CHIP_ID != nand_id_readout)
    {
        if (nand_id_readout != g_pNandInfo->chip_id)
        {
            NF_INFO("id is wrong!!\r\n");
            while(1);
        } 
    }
    else
    {
        while(1);
    }
    
    pNand_Add->ChipPos[0] = 0;
    ChipCnt = 1;

    for( i = 1; i < NFC_SUPPORT_CHIPNUM; i++ )
    { 
        nand_id_readout = nand_read_chipID(i);
        NF_INFO("chip %d id=0x%x\r\n", i, nand_id_readout);

        if(g_pNandInfo->chip_id == nand_id_readout)
        {
            pNand_Add->ChipPos[ChipCnt] = i;        //logic CE to phy CE
            ChipCnt++;
        }
    }

    if( ChipCnt > 0 )
    {
		dat_len = g_pNandInfo->data_len;
        NF_INFO("cmd_len 0x%x, data_len 0x%x\r\n",g_pNandInfo->cmd_len,dat_len);
        nand_config_timeseq(g_pNandInfo->cmd_len, 
                                    g_pNandInfo->data_len);
        //-------------------------------------------------------------------
		//pNand_Info->NandType = NANDFLASH_TYPE_SAMSUNG;
        pNand_Info->ChipCharacterBits = (g_pNandInfo->flag) & 0xffff0fff ;  
		
        pNand_Info->BytesPerSector    = g_pNandInfo->page_size;                            
        pNand_Info->BlockPerPlane     = g_pNandInfo->plane_blk_num;
        pNand_Info->PlanePerChip      = g_pNandInfo->blk_num / g_pNandInfo->plane_blk_num;
        pNand_Info->PagePerBlock      = g_pNandInfo->page_per_blk;
        pNand_Info->SectorPerPage     = g_pNandInfo->page_size / pNand_Info->BytesPerSector;
        pNand_Info->PlaneCnt = pNand_Info->PlanePerChip * ChipCnt;

        m_true_block_per_die = g_pNandInfo->group_blk_num;

        //if fake_block_per_die should be upscaled, double it
        if (1 == ((pNand_Info->ChipCharacterBits >> 11) & 0x1))
        {
            m_fake_block_per_die = m_true_block_per_die * 2;
            NF_INFO("block_per_die %d upscale to %d!!\n", m_true_block_per_die, m_fake_block_per_die);
        }
        else
        {
            m_fake_block_per_die = m_true_block_per_die;
        }
		
		//-------------------------------------------------------------------
		pNand_Add->RowCycle      = g_pNandInfo->row_cycle;
        pNand_Add->ColCycle      = g_pNandInfo->col_cycle;
		pNand_Add->EccType       = (pNand_Info->ChipCharacterBits >> 4) & 0xF;
		pNand_Add->PageSize      = pNand_Info->BytesPerSector;
		pNand_Add->PagesPerBlock = pNand_Info->PagePerBlock;
		
		if(pNand_Info->BytesPerSector == 8192)
		{
			pNand_Add->ChipType = NAND_8K_PAGE;
		}
		else if(pNand_Info->BytesPerSector == 4096)
		{
			pNand_Add->ChipType = NAND_4K_PAGE;
		}
		else if(pNand_Info->BytesPerSector == 2048)
		{
			pNand_Add->ChipType = NAND_2K_PAGE;
		}
		else
		{
			pNand_Add->ChipType = NAND_512B_PAGE;			
		}
		
		pNand_Info->WriteSector = Nand_WriteSector;
		pNand_Info->ReadSector  = Nand_ReadSector;
		
		pNand_Info->MultiRead       = Nand_MultiReadSector;
		pNand_Info->MultiWrite      = Nand_MultiWriteSector;
		pNand_Info->MultiCopyBack   = Nand_MultiCopyBack;
		pNand_Info->MultiEraseBlock = Nand_MultiEraseBlock;
		
		pNand_Info->CopyBack = Nand_CopyBack;	//mlc have copyback
        pNand_Info->EraseBlock = Nand_EraseBlock;
		
        pNand_Info->ReadFlag= Nand_ReadFlag;
        pNand_Info->WriteFlag= Nand_WriteFlag;
        pNand_Info->IsBadBlock = Nand_IsBadBlock;
		pNand_Info->SetBadBlock= Nand_SetBadBlock; 

        pNand_Info->ExReadFlag = Nand_ReadFlag_Exnftl;
        pNand_Info->ExWrite = Nand_WriteSector_Exnftl;
        pNand_Info->ExRead = Nand_ReadSector_Exnftl;
        pNand_Info->ExEraseBlock = Nand_EraseBlock_Exnftl;
        pNand_Info->ExIsBadBlock = Nand_IsBadBlock;
        pNand_Info->ExSetBadBlock = Nand_SetBadBlock;
   }
   else
   {
        NF_INFO("Cannot find chip!\r\n");
        pNand_Info = Fwl_Free(pNand_Info);
   }
   
   if(1 == ((pNand_Info->ChipCharacterBits >> 17) & 0x1))
   {
       nand_enable_randomizer(pNand_Add);
   }
   	//just calculate data length of nand once
    nand_calctiming(dat_len);
    nand_changetiming(get_asic_freq());

#ifdef NF_TEST_API
   NandFlash_Test(pNand_Info);
#endif
    //clear multi-bits
    pNand_Info->ChipCharacterBits &= 0xFFFF0FFF;    
   return pNand_Info;
}

T_VOID Nand_Restore_Default_Scale(T_U32 chip)
{
    nand_restore_default_scale(chip);
}


/**
 * @brief   convert nand driver's return values to NTFL's
 *
 * @author  yiruoxiang
 * @date    2010-12-01
 * @param   [in] ChipCharacterBits the flag of nandlist
 * @param   [in] ret nand driver's return value
 * @return  E_NANDERRORCODE NFTL's nand error code
 * @retval  NF_FAIL operation fail
 * @retval  NF_SUCCESS operation ok
 * @retval  NF_STRONG_DANGER nand's data are very dangerous, the block is unstable
 * @retval  NF_WEAK_DANGER nand's data have a lot of flips, the block can be used again but need to be refresh
 */
E_NANDERRORCODE Nand_ChangeReturnValue(T_U32 ChipCharacterBits, T_U32 ret)
{
    T_U32 ECCType, WeakDangerBits, StrongDangerBits;
    
    if(ret & ((1UL << FINAL_FAIL_BIT) | (1UL << L2_ALLOC_FAIL_BIT) | (1UL << TIME_OUT_BIT)))
    {
        return NF_FAIL;
    }
    else if(ret & (1UL << ONCE_OPERATION_FAILED_BIT))
    {
        if (AK_TRUE == nand_use_read_retry())
        {
            return NF_WEAK_DANGER;
        }
        else
        {
            return NF_STRONG_DANGER;
        }
    }
    
    ECCType = (ChipCharacterBits >> 4) & 0xF;
    if(ECC_4BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE0;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE0;
    }
    else if(ECC_8BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE1;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE1;
    }
    else if(ECC_12BIT_P512B == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE2;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE2;
    }
    else if(ECC_24BIT_P1KB == ECCType)
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE4;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE4;
    }
    else    //default is ECC_32BIT_P1KB
    {
        WeakDangerBits = WEAK_DANGER_BIT_NUM_MODE5;
        StrongDangerBits = STRONG_DANGER_BIT_NUM_MODE5;
    }
    ret = ret & 0xFF;
    if(ret < WeakDangerBits)
    {
        return NF_SUCCESS;
    }
    else if (ret < StrongDangerBits)
    {
        return NF_WEAK_DANGER;
    }
    else
    {
        if (AK_TRUE == nand_use_read_retry())
        {
            return NF_WEAK_DANGER;
        }
        else
        {
            return NF_STRONG_DANGER;
        }

    }
}

//***********************************************************************************


#if  defined(NF_TEST_DBG) || defined(NF_TEST_API)
#define TEST_PRINT_COL   32
static T_VOID NandFlash_TestPrintf(T_U8* tips, T_U8*pagedata,T_U32 pageLen,T_U8 *oobbuf)
{
    T_U32 i= 0;
    NF_INFO("=============== %s =================\n",tips);
    if (AK_NULL != pagedata)
    {
        for (i=0;i<pageLen;i++)
        {
            NF_INFO(" %02X",pagedata[i]);
            if ((i+1)%TEST_PRINT_COL == 0)
            {
               NF_INFO("\n");
            }
        }
    }
    if (AK_NULL != oobbuf)
    {
        if (AK_NULL != pagedata)
        {
            NF_INFO("\n");
        }
        NF_INFO("=== OOB :");
        for (i=0;i<8;i++)
        {
        NF_INFO(" %02X",oobbuf[i]);
        }
    }
    NF_INFO("\n=============================================\n");
}

#endif

#if  defined(NF_TEST_API)
static T_VOID NandFlash_Test(const T_PNANDFLASH Nand_base)
{
    T_U8 *writeDataBuf = AK_NULL,*readDateBuf = AK_NULL;
    T_U8 oobbuf[8];
    T_U32 maxPage = 0;
    T_U32 testPageId = 0,testchipid = 0, testblockid = 0;
    T_U32 pageLen = 0;
    T_U32 i = 0;
    if (AK_NULL == Nand_base)
    {
        NF_INFO("NandFlash_Test error : Param!\n");
        return;
    }

    maxPage = Nand_base->PagePerBlock * Nand_base->SectorPerPage - 1;
    pageLen = Nand_base->BytesPerSector;

    writeDataBuf = (T_U8*)Ram_Malloc(pageLen);
    readDateBuf = (T_U8*)Ram_Malloc(pageLen);
    
    if((AK_NULL == writeDataBuf)|| (AK_NULL == readDateBuf))
    {
        NF_INFO("NAND:NandFlash_Test: Malloc fail\n");
        return;
    }
//===========Write Page===================
    for (i=0;i<pageLen;i++)
    {
        writeDataBuf[i] = rand() % 0xff;
    }
    
    for (i=0;i<8;i++)
    {
        oobbuf[i] = i+2;
    }
    //------------------
    NandFlash_TestPrintf("write page", writeDataBuf, pageLen,oobbuf);
    //------------------
    testchipid = 0;
    testblockid = 0;
    testPageId = 10;
    Nand_base->WriteSector(Nand_base, testchipid,
            testblockid, testPageId,writeDataBuf, oobbuf, 8);
    
//=============Read Page================
    for (i=0;i<pageLen;i++)
    {
        readDateBuf[i] = 0;
    }
    for (i=0;i<8;i++)
    {
        oobbuf[i] = 0;
    }
    Nand_base->ReadSector(Nand_base, testchipid,
            testblockid, testPageId,readDateBuf, oobbuf, 8);
    //------------------
    NandFlash_TestPrintf("Read page", readDateBuf, pageLen,oobbuf);
    //------------------
    
    NF_INFO("================Cmp data ==================\n");
    for (i=0;i<pageLen;i++)
    {
        if (writeDataBuf[i]!=readDateBuf[i])
        {
           NF_INFO("Addr 0x%X: %02X \n",i,writeDataBuf[i]);
        }
    }
    NF_INFO("\n================Cmp End=======================\n");

//============write oob================
    for (i=0;i<8;i++)
    {
        oobbuf[i] = rand() % 0xff;
    }
    NandFlash_TestPrintf("write oob", AK_NULL, 0,oobbuf);
    //------------------
    Nand_base->WriteFlag(Nand_base, testchipid,
            testblockid, testPageId, oobbuf, 8);
    
//============read oob================
    for (i=0;i<8;i++)
    {
        oobbuf[i] = 0;
    }
    Nand_base->ReadFlag(Nand_base, testchipid,
            testblockid, testPageId,oobbuf, 8);

    NandFlash_TestPrintf("Read oob", AK_NULL, 0,oobbuf);
    
//=============Erase Bolck================

    Nand_base->EraseBlock(Nand_base, testchipid,testblockid);
    for (i=0;i<pageLen;i++)
    {
        readDateBuf[i] = 0;
    }
    for (i=0;i<8;i++)
    {
        oobbuf[i] = 0;
    }
    Nand_base->ReadSector(Nand_base, testchipid,
            testblockid, testPageId,readDateBuf, oobbuf, 8);
    //------------------
    NandFlash_TestPrintf("Erase Read page", readDateBuf, pageLen,oobbuf);

    Ram_Release(writeDataBuf);
    Ram_Release(readDateBuf);
    writeDataBuf = AK_NULL;
    readDateBuf = AK_NULL;
    
}
#endif

//分离的data and oob
static T_VOID Nand_Config_Data(T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8* data, T_U32 datalen, T_U8 *oob, T_U32 oob_len, T_U8 ecctype)
{
    if (AK_NULL != data_ctrl)
    {
        data_ctrl->buf = data;
        data_ctrl->buf_len = datalen;
        data_ctrl->ecc_section_len = (ecctype > ECC_16BIT_P512B) ? NAND_DATA_SIZE_P1KB : NAND_DATA_SIZE_P512B;
        data_ctrl->ecc_type = ecctype;
    }

    if (AK_NULL != spare_ctrl)
    {
        spare_ctrl->buf = oob;
        spare_ctrl->buf_len = oob_len;
        spare_ctrl->ecc_section_len = oob_len;
        spare_ctrl->ecc_type = (ecctype > ECC_12BIT_P512B) ? ECC_12BIT_P512B : ecctype;
    }
} 

//不分离的data and oob
static T_VOID Nand_Config_Data_SmallPage(T_PNAND_ECC_STRU data_ctrl,  T_PNAND_ECC_STRU spare_ctrl, T_U8* data, T_U32 datalen, T_U8 *oob, T_U32 oob_len, T_U8 ecctype)
{
    data_ctrl->buf = data;
    data_ctrl->buf_len = datalen;
    data_ctrl->ecc_section_len = NAND_DATA_SIZE_P512B + oob_len;
    data_ctrl->ecc_type = ecctype;
 
    spare_ctrl->buf = oob;
    spare_ctrl->buf_len = oob_len;
    spare_ctrl->ecc_section_len = data_ctrl->ecc_section_len;
    spare_ctrl->ecc_type = ecctype;
} 

/**
 * @brief   check bad blocks of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  T_BOOL
 */
T_BOOL Nand_IsBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
    return FHA_check_bad_block(block);
}

static T_BOOL Nand_SetBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
    FHA_set_bad_block(block);
    
    return AK_TRUE;
}


T_VOID NandFlash_ResetAll(T_PNANDFLASH nand)
{
    T_U32 i;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    for( i = 0; i < NFC_SUPPORT_CHIPNUM; i++ )
    {
        if( 0xFF != pNand_Add->ChipPos[i] )//this logic CE is connect flash
        {
            nand_reset(pNand_Add->ChipPos[i]);
        }
    }
}


/**
 * @brief   read 1 sector data from nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] data buffer for read sector, should be 512 bytes.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
static E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH nand, T_U32 chip,T_U32 block, T_U32 page, T_U8 data[], T_U8* oob,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32   rowAddr = 0, ret;
    E_NANDERRORCODE result;
	T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
    T_U32 chip_rowaddr_value;  //record chip and rowaddr information, = (rowAddr | (chip << 24))

    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    if (NAND_512B_PAGE == pNand_Add->ChipType)
    { 
        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data, nand->BytesPerSector,
                         oob, oob_len, pNand_Add->EccType);
    }
    else
    {
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                         data, nand->BytesPerSector,
                         new_spare_buffer, oob_len + sizeof(rowAddr),pNand_Add->EccType);
    } 

    ret = nand_readpage_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
                               pNand_Add, &data_ctrl, &spare_ctrl);

    if (NAND_512B_PAGE != pNand_Add->ChipType)
    { 
        memcpy((T_U8 *)(&chip_rowaddr_value), (T_U8 *)new_spare_buffer + oob_len, sizeof(rowAddr));

    	if ((0xffffffff != chip_rowaddr_value) && ((chip_rowaddr_value % pNand_Add->PagesPerBlock) != page))
        {
            NF_INFO("C%d_B%d_P%d != 0x%x,RL\r\n", chip, block, page, chip_rowaddr_value);
            if (Nand_IsLowVoltage())
            {
                NF_INFO("low voltage\n");
                while(1);
            }   
        }
    	
        memcpy(oob, (T_U8 *)new_spare_buffer, oob_len);
    }    
    
    result = Nand_ChangeReturnValue(nand->ChipCharacterBits, ret);

    ret = ret & 0xff;

    if (NF_FAIL == result)
    {
        NF_INFO("C=%d,B=%d,P=%d,mflip=%d,spare_0x%x_0x%x,RL\n", chip, block, page, ret, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
    }
    else if (NF_WEAK_DANGER == result)
    {
        NF_INFO("C=%d,B=%d,P=%d,mflip=%d,wdanger!\n", chip, block, page, ret); //weak danger!
    }    
    else if (NF_STRONG_DANGER == result)
    {
        NF_INFO("C=%d,B=%d,P=%d,mflip=%d,sdanger!\n", chip, block, page, ret); //strong danger!!
    }

    return result;
        
}


//***********************************************************************************
/**
 * @brief   write 1 sector data to nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] sector which sector will be written.
 * @param   [in] data buffer for write sector, should be 512 bytes.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
static E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8* oob,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32    rowAddr = 0;
    T_U32 ret = 0;
	T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
    //T_U32 chip_rowaddr_value;  //record chip and rowaddr information, = (rowAddr | (chip << 24))

    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    if (NAND_512B_PAGE == pNand_Add->ChipType)
    { 
        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data, nand->BytesPerSector,
                         oob, oob_len, pNand_Add->EccType);
    }
    else
    {
        //1: form new spare buffer
        memcpy((T_U8 *)new_spare_buffer, oob, oob_len);
        memcpy((T_U8 *)new_spare_buffer + oob_len, (T_U8 *)(&rowAddr), sizeof(rowAddr) - 1);
        new_spare_buffer[oob_len + sizeof(rowAddr) - 1] = chip;

    	//2: change length of new spare buffer
        oob_len = oob_len + sizeof(rowAddr);
        
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     data, nand->BytesPerSector,
                     (T_U8 *)new_spare_buffer, oob_len, pNand_Add->EccType);
    } 

    ret = nand_writepage_ecc(pNand_Add->ChipPos[chip],rowAddr, 0, 
                             pNand_Add, &data_ctrl, &spare_ctrl);

    if (0 != ret)
    {
        NF_INFO("C=%d,B=%d,P=%d,spare_0x%x_0x%x,WL\n", chip, block, page, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

/**
 * @brief   read file system infomation from nandflash without ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] sector which sector will be read.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
static E_NANDERRORCODE Nand_ReadFlag(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32    rowAddr = 0, columnAddr = 0;
    T_U32 ret = 0;
	T_U8 new_spare_buffer[20]; //conjoin pSpareCtrl and RowAddr together to form a new spare
    T_U32 chip_rowaddr_value;  //record chip and rowaddr information, = (rowAddr | (chip << 24))
    
    pNand_Add = (T_PNandflash_Add)(nand->BufStart);

    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    rowAddr = block * pNand_Add->PagesPerBlock + page;

    if (NAND_512B_PAGE == pNand_Add->ChipType)
    {
        T_NAND_ECC_STRU data_ctrl;
        T_U8 data_tmp[NAND_DATA_SIZE_P512B];

        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data_tmp, nand->BytesPerSector,
                         oob, oob_len, pNand_Add->EccType);

        ret = nand_readpage_ecc(pNand_Add->ChipPos[chip],rowAddr, 0, 
                                 pNand_Add, &data_ctrl, &spare_ctrl);
    }
    else
    {
        Nand_Config_Data(AK_NULL, &spare_ctrl, 
                         AK_NULL, 0,
                         new_spare_buffer, oob_len + sizeof(rowAddr), pNand_Add->EccType);

        
        if(pNand_Add->EccType <= ECC_12BIT_P512B)
        {
            columnAddr = (NAND_DATA_SIZE_P512B + (pNand_Add->EccType + 1) * NAND_PARITY_SIZE_MODE0) * (nand->BytesPerSector / NAND_DATA_SIZE_P512B);
        }

        else
        {
            columnAddr = (NAND_DATA_SIZE_P1KB + (pNand_Add->EccType - 1) * NAND_PARITY_SIZE_MODE1) * (nand->BytesPerSector / NAND_DATA_SIZE_P1KB);
        } 

        ret = nand_readpage_ecc(pNand_Add->ChipPos[chip],rowAddr, columnAddr , 
                                 pNand_Add, AK_NULL, &spare_ctrl);
		
		memcpy((T_U8 *)(&chip_rowaddr_value), (T_U8 *)new_spare_buffer + oob_len, sizeof(rowAddr));

    	if ((0xffffffff != chip_rowaddr_value) && ((chip_rowaddr_value % pNand_Add->PagesPerBlock) != page))
        {
            NF_INFO("C%d_B%d_P%d != 0x%x,RS\r\n", chip, block, page, chip_rowaddr_value);

            if (Nand_IsLowVoltage())
            {
                NF_INFO("low voltage\n");
                while(1);
            }
        }
        
        memcpy(oob, (T_U8 *)new_spare_buffer, oob_len);
    }
  
    if(ret & (1UL << FINAL_FAIL_BIT))
    {
        NF_INFO("C=%d,B=%d,P=%d,spare_0x%x_0x%x,RS\n", chip, block, page, *((T_U32 *)oob), *((T_U32 *)(oob + 4)));
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}




/**
 * @brief   write file system infomation to nandflash without ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] sector which sector will be written.
 * @param   [in] spare buffer for file system infomation, should be 4 bytes.
 * @return  E_NANDERRORCODE
 */
static E_NANDERRORCODE Nand_WriteFlag(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len)
{
    T_U8 *data;
    E_NANDERRORCODE ret = 0;
    
#if 1
    data = (T_U8 *)Fwl_Malloc(nand->BytesPerSector);
    memset(data, 0x5A, nand->BytesPerSector);
    ret = Nand_WriteSector(nand, chip, block, page, data, oob, oob_len);
    Fwl_Free(data);
    return ret;
#else
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU spare_ctrl;
    T_NAND_ECC_STRU data_ctrl;
    T_U32    rowAddr = 0, columnAddr = 0;
    T_U32 ret = 0;
    T_U8 *databuf = AK_NULL;
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    databuf = (T_U8*)Fwl_Malloc(nand->BytesPerSector);
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     databuf, nand->BytesPerSector,
                     oob, oob_len,pNand_Add->EccType);

    rowAddr = block * pNand_Add->PagesPerBlock + page;
    if(pNand_Add->EccType <= ECC_12BIT_P512B)
    {
        columnAddr = (NAND_DATA_SIZE_P512B + (pNand_Add->EccType + 1) * NAND_PARITY_SIZE_MODE0) * (nand->BytesPerSector / NAND_DATA_SIZE_P512B);
    }

    else
    {
        columnAddr = (NAND_DATA_SIZE_P1KB + (pNand_Add->EccType - 1) * NAND_PARITY_SIZE_MODE1) * (nand->BytesPerSector / NAND_DATA_SIZE_P1KB);
    } 
    
    ret = nand_writepage_ecc(pNand_Add->ChipPos[chip],rowAddr, columnAddr , 
                                 pNand_Add, &data_ctrl, &spare_ctrl);

    Fwl_Free(databuf);
    databuf = AK_NULL;
    if (0 == ret)
    {
        return NF_SUCCESS;
    }
    else
    {
        return NF_FAIL;
    }
#endif
}




/** 
 * @brief   erase 1 block of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] block which block whill be erased.
 * @return  T_U32
 */
static E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
    T_U32 blk_start_page, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }

    blk_start_page = block * nand->PagePerBlock;

    ret = nand_eraseblock(pNand_Add->ChipPos[chip], blk_start_page, pNand_Add);

    if (0 != ret)
    {
        NF_INFO("C=%d,B=%d,EB\n", chip, block);
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

/**
 * @brief   copy one physical page to another one, with hardware.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNand_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] SourceBlock read the source block.
 * @param   [in] DestBlock write to destination block.
 * @param   [in] page the page of the block will be copy.
 * @return  E_NANDERRORCODE
 */
static E_NANDERRORCODE Nand_CopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    T_U32   SouPhyPage, DesPhyPage, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    if ((m_fake_block_per_die != m_true_block_per_die))
    {
        if (SourceBlock >= m_true_block_per_die)
        {
            SourceBlock = (SourceBlock % m_true_block_per_die) + m_fake_block_per_die;
        }

        if (DestBlock >= m_true_block_per_die)
        {
            DestBlock = (DestBlock % m_true_block_per_die) + m_fake_block_per_die;
        }    
    }

    SouPhyPage = page + (SourceBlock * nand->PagePerBlock);
    DesPhyPage = page + (DestBlock * nand->PagePerBlock);

    ret = nand_copyback(pNand_Add->ChipPos[chip], SouPhyPage, DesPhyPage, pNand_Add);

    if (0 != ret)
    {
        NF_INFO("C=%d,SB=%d,DB=%d,P=%d,CB\n", chip, SourceBlock, DestBlock, page);
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}


/*
static T_U32  Nand_Fake2Real(T_PNANDFLASH nand, T_U32 plane, T_U32 FakePhyAddr, T_U32 *chip)
{
    NF_API_DBG("Fake2Real");
    
    return NF_FAIL;
}
*/

static E_NANDERRORCODE Nand_MultiWriteSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    //T_U32 ecc_len = 0;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 plane1_rowaddr = 0;   //row address of the block in plane 1
    T_U32 ret = 0;
    T_U8 new_spare_len = 0;
    T_U8 new_spare_buffer[30]; //conjoin spare_ctrl and RowAddr together to form a new spare
                               //new spare =  oob(oob_len bytes)  |  rowaddr(3 bytes)  |  chip(1 byte)
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    //1: calculate row address of two planes, and new spare length
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    plane1_rowaddr = (block + 1) * pNand_Add->PagesPerBlock + page;
    new_spare_len = oob_len + sizeof(plane0_rowaddr);

    //2: form new spare buffer from row address and chip in plane 0
    memcpy((T_U8 *)new_spare_buffer, SpareTbl, oob_len);
    memcpy((T_U8 *)new_spare_buffer + oob_len, (T_U8 *)(&plane0_rowaddr), sizeof(plane0_rowaddr) - 1);
    new_spare_buffer[new_spare_len - 1] = chip;
    
    //3: form new spare buffer from row address and chip in plane 1
    memcpy((T_U8 *)new_spare_buffer + new_spare_len, SpareTbl + oob_len, oob_len);
    memcpy((T_U8 *)new_spare_buffer + new_spare_len + oob_len, (T_U8 *)(&plane1_rowaddr), sizeof(plane1_rowaddr) - 1);
    new_spare_buffer[(new_spare_len * 2) - 1] = chip;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 (T_U8 *)new_spare_buffer, new_spare_len, pNand_Add->EccType);

    ret = nand_writepage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                             pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }

}
static E_NANDERRORCODE Nand_MultiReadSector(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 plane1_rowaddr = 0;  //row address of the block in plane 1
    T_U8 new_spare_buffer[30]; //conjoin pSpareCtrl and RowAddr together to form a new spare
    T_U32 chip_rowaddr_value1;  //record chip and rowaddr information of plane0, = (plane0_rowaddr | (chip << 24))
    T_U32 chip_rowaddr_value2;  //record chip and rowaddr information of plane1, = (plane1_rowaddr | (chip << 24))
    T_U32 ret;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    plane1_rowaddr = (block + 1) * pNand_Add->PagesPerBlock + page;
 
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     data, nand->BytesPerSector,
                     (T_U8 *)new_spare_buffer, oob_len + sizeof(plane0_rowaddr), pNand_Add->EccType);

    ret = nand_readpage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                               pNand_Add, &data_ctrl, &spare_ctrl);

    memcpy((T_U8 *)(&chip_rowaddr_value1), (T_U8 *)new_spare_buffer + oob_len, sizeof(plane0_rowaddr));
    memcpy((T_U8 *)(&chip_rowaddr_value2), (T_U8 *)new_spare_buffer + 2 * oob_len + sizeof(plane0_rowaddr), sizeof(plane1_rowaddr));
    
    if (((0xffffffff != chip_rowaddr_value1) && ((chip_rowaddr_value1 % pNand_Add->PagesPerBlock) != page))
        || ((0xffffffff != chip_rowaddr_value2) && ((chip_rowaddr_value2 % pNand_Add->PagesPerBlock) != page)))
    {
        NF_INFO("C%d_B%d_P%d != 0x%x_0x%x,MRL\r\n", chip, block, page, chip_rowaddr_value1, chip_rowaddr_value2);
		if (Nand_IsLowVoltage())
        {
            NF_INFO("low voltage\n");
            while(1);
        }
	}
    
    memcpy(SpareTbl, (T_U8 *)new_spare_buffer, oob_len);
    memcpy(SpareTbl + oob_len, (T_U8 *)new_spare_buffer + oob_len + sizeof(plane0_rowaddr), oob_len);
  
    return Nand_ChangeReturnValue(nand->ChipCharacterBits, ret);

}

static E_NANDERRORCODE Nand_MultiCopyBack(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    T_U32   SouPhyPage, DesPhyPage, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    SouPhyPage = page + (SourceBlock * nand->PagePerBlock);
    DesPhyPage = page + (DestBlock * nand->PagePerBlock);

    ret = nand_copyback_2plane(pNand_Add->ChipPos[chip], SouPhyPage, DesPhyPage, pNand_Add);

    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_MultiEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block)
{
    T_U32 blk_start_page, ret;
    T_PNandflash_Add pNand_Add = (T_PNandflash_Add)(&nand[1]);

    blk_start_page = block * nand->PagePerBlock;

    ret = nand_eraseblock_2plane(pNand_Add->ChipPos[chip], blk_start_page, pNand_Add);
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}
static E_NANDERRORCODE Nand_WriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data[], T_U8 *spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0;
    T_U32 ret = 0;      

    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;
    if(NAND_512B_PAGE == pNand_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data, nand->BytesPerSector,
                         spare_tbl, oob_len - sizeof(page), pNand_Add->EccType);
    }
    else
    {   
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 spare_tbl, oob_len, pNand_Add->EccType);
        
    *(T_U32 *)(spare_tbl + oob_len - sizeof(page)) = page;
    }
    ret = nand_writepage_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
                             pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_ReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0, drv_ret;
    T_U32 page_offset = 0;  //record page offset in a block
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    if(NAND_512B_PAGE == pNand_Add->ChipType)
    {
        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data, nand->BytesPerSector,
                         spare_tbl, oob_len - sizeof(page), pNand_Add->EccType);

    }
    else
    {
        Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     data, nand->BytesPerSector,
                     spare_tbl, oob_len , pNand_Add->EccType);
    }
    
    drv_ret = nand_readpage_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
                               pNand_Add, &data_ctrl, &spare_ctrl);
    
    fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);

    if(NF_FAIL != fwl_ret)
    {
        page_offset = *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

        if ((0xffffffff != page_offset) && (page != page_offset))
        {
            NF_INFO("C%d_B%d_P%d != 0x%x,RL###\n", chip, block, page, page_offset);
			if (Nand_IsLowVoltage())
            {
                NF_INFO("low voltage\n");
                while(1);
            }
		}
    }
    
    return fwl_ret;
}

static E_NANDERRORCODE Nand_MultiWriteSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 ret = 0;   
    pNand_Add = (T_PNandflash_Add)(&nand[1]);

    //1: calculate row address of  plane0
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;

    //2: form new spare buffer from row address in plane 0
    *(T_U32 *)(spare_tbl + oob_len - sizeof(page)) = page;

    //3: form new spare buffer from row address  in plane 1
    *(T_U32 *)(spare_tbl + (oob_len<<1) - sizeof(page)) = page;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                 data, nand->BytesPerSector,
                 spare_tbl, oob_len, pNand_Add->EccType);
    
    ret = nand_writepage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                             pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

static E_NANDERRORCODE Nand_MultiReadSector_Ex(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 page_offset0;  //record page offset in a block 
    T_U32 page_offset1;  //record page offset in a block
    T_U32 drv_ret;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;
    
    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                      data, nand->BytesPerSector,
                       spare_tbl, oob_len, pNand_Add->EccType);

    drv_ret = nand_readpage_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                               pNand_Add, &data_ctrl, &spare_ctrl);

    fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
    
    if(NF_FAIL != fwl_ret)
	{

	    page_offset0 =  *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

	    page_offset1 =  *(T_U32 *)(spare_tbl + (oob_len<<1) - sizeof(page));

	    if (((0xffffffff != page_offset0) && (page != page_offset0))
	        || ((0xffffffff != page_offset1) && (page != page_offset1)))
	    {
	        NF_INFO("C%d_B%d_P%d != 0x%x_0x%x,MRL***\n", chip, block, page, page_offset0, page_offset1);
	        if (Nand_IsLowVoltage())
	        {
	            NF_INFO("low voltage\n");
	            while(1);
	        }
	    }
	 }

    return fwl_ret;
}

static E_NANDERRORCODE Nand_ReadSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0, drv_ret;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;

    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;
    

    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                    data, nand->BytesPerSector,
                    spare_tbl, oob_len, pNand_Add->EccType);

    drv_ret = nand_readpage_cache_ecc(pNand_Add->ChipPos[chip], rowAddr, 0,
                               page_num, pNand_Add, &data_ctrl, &spare_ctrl);
       
   fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
    
    if(NF_FAIL != fwl_ret)
   {
        spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
        for(i = 0; i < page_num; i++)
        {   
            spare_tbl_offset += oob_len;

            page_offset = *(T_U32 *)spare_tbl_offset;
            
            if((0xffffffff != page_offset) && ((page + i) != page_offset))
            {
                NF_INFO("C%d_B%d_P%d != 0x%x,CRL***\n", chip, block, page + i, page_offset);
	            if (Nand_IsLowVoltage())
	            {
	                NF_INFO("low voltage\n");
	                while(1);
	            }
            }
            
        }
    }
   
    return fwl_ret;
    
}
static E_NANDERRORCODE Nand_MultiReadSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page, T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    E_NANDERRORCODE fwl_ret;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;  //row address of the block in plane 0
    T_U32 drv_ret;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;
     
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;

    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                     data, nand->BytesPerSector,
                     spare_tbl, oob_len, pNand_Add->EccType);


    drv_ret = nand_readpage_cache_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                               (page_num >> 1), pNand_Add, &data_ctrl, &spare_ctrl);
    fwl_ret = Nand_ChangeReturnValue(nand->ChipCharacterBits, drv_ret);
     
    if(NF_FAIL != fwl_ret)
	{

	    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
	    for(i = 0; i < page_num; i++)
	    {
	        spare_tbl_offset += oob_len;
	        page_offset = *(T_U32 *)spare_tbl_offset;

	        if((0xffffffff != page_offset) && ((page + (i >> 1)) != page_offset))
	        {
            NF_INFO("C%d_B%d_P%d != 0x%x,MCRL***\n", chip, block + (i & 0x1), page + (i >> 1), page_offset);
				if (Nand_IsLowVoltage())
	            {
	                NF_INFO("low voltage\n");
	                while(1);
	            }
			}

	    }
	}

    return fwl_ret;    
}

static E_NANDERRORCODE Nand_WriteSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len , T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0;
    T_U32 ret = 0;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    
    rowAddr = block * pNand_Add->PagesPerBlock + page;

    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
                    data, nand->BytesPerSector,
                    spare_tbl, oob_len, pNand_Add->EccType);
    


    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
    page_offset = page;
    for(i = 0; i < page_num; i++)
    {
        spare_tbl_offset += oob_len;
        *(T_U32 *)(spare_tbl_offset) = page_offset;
        page_offset++;
    }


    ret = nand_writepage_cache_ecc(pNand_Add->ChipPos[chip], rowAddr, 0, 
                             page_num, pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }

}


static E_NANDERRORCODE Nand_MultiWriteSector_Cache(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page, const T_U8 data [ ], T_U8 * spare_tbl, T_U32 oob_len, T_U32 page_num)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU data_ctrl;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 plane0_rowaddr = 0;   //row address of the block in plane 0
    T_U32 ret = 0;
    T_U32 i; 
    T_U32 page_offset = 0;  //record page offset in a block  
    T_U32 spare_tbl_offset = 0;
     
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    plane0_rowaddr = block * pNand_Add->PagesPerBlock + page;


    Nand_Config_Data(&data_ctrl, &spare_ctrl, 
            data, nand->BytesPerSector,
            spare_tbl, oob_len, pNand_Add->EccType);

    spare_tbl_offset = (T_U32)spare_tbl - sizeof(page);
    page_offset = page;

    for(i = 0; i < page_num; i++)
    {   
        spare_tbl_offset += oob_len;
        *(T_U32 *)spare_tbl_offset = page_offset;
        page_offset += (i & 0x1);//modify page offset
    }

    ret = nand_writepage_cache_ecc_2plane(pNand_Add->ChipPos[chip], plane0_rowaddr, 0, 
                             (page_num >> 1), pNand_Add, &data_ctrl, &spare_ctrl);
    
    if (0 != ret)
    {
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }    
}
static E_NANDERRORCODE Nand_ReadFlag_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 block, T_U32 page, T_U8 *spare_tbl,T_U32 oob_len)
{
    T_PNandflash_Add pNand_Add = AK_NULL;
    T_NAND_ECC_STRU spare_ctrl;
    T_U32 rowAddr = 0, columnAddr = 0;
    T_U32 ret = 0;
    T_U32 page_offset;  //record chip and rowaddr information, = (rowAddr | (chip << 24))
    
    pNand_Add = (T_PNandflash_Add)(&nand[1]);
    


    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
    }
    rowAddr = block * pNand_Add->PagesPerBlock + page;


    if (NAND_512B_PAGE == pNand_Add->ChipType) 
    {
        T_NAND_ECC_STRU data_ctrl;
        T_U8 data_tmp[NAND_DATA_SIZE_P512B];

        Nand_Config_Data_SmallPage(&data_ctrl, &spare_ctrl, 
                         data_tmp, nand->BytesPerSector,
                         spare_tbl, oob_len - sizeof(page), pNand_Add->EccType);

        ret = nand_readpage_ecc(pNand_Add->ChipPos[chip],rowAddr, 0, 
                                 pNand_Add, &data_ctrl, &spare_ctrl);

    }
    else
    {
        
        Nand_Config_Data(AK_NULL, &spare_ctrl, 
                     AK_NULL, 0,
                     spare_tbl, oob_len, pNand_Add->EccType);
    
        if(pNand_Add->EccType <= ECC_12BIT_P512B)
        {
            columnAddr = (NAND_DATA_SIZE_P512B + (pNand_Add->EccType + 1) * NAND_PARITY_SIZE_MODE0) * (nand->BytesPerSector / NAND_DATA_SIZE_P512B);
        }

        else
        {
            columnAddr = (NAND_DATA_SIZE_P1KB + (pNand_Add->EccType - 1) * NAND_PARITY_SIZE_MODE1) * (nand->BytesPerSector / NAND_DATA_SIZE_P1KB);
        } 

        ret = nand_readpage_ecc(pNand_Add->ChipPos[chip], rowAddr, columnAddr, 
                                 pNand_Add, AK_NULL, &spare_ctrl);
        
        page_offset = *(T_U32 *)(spare_tbl + oob_len - sizeof(page));

        if ((0xffffffff != page_offset) && (page_offset != page))
        {
            NF_INFO("C%d_B%d_P%d != 0x%x,RS$$$\n", chip, block, page, page_offset);
			if (Nand_IsLowVoltage())
            {
                NF_INFO("low voltage\n");
                while(1);
            }
        }
    }

    if(ret & (1UL << FINAL_FAIL_BIT))
    {
        NF_INFO("C=%d,B=%d,P=%d,spare_0x%x_0x%x,RS\n", chip, block, page, *((T_U32 *)spare_tbl), page_offset);
        return NF_FAIL;
    }
    else
    {
        return NF_SUCCESS;
    }
}

/**
* @brief reading interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating the num of plane to be operated
* @param   [in] block   which block to be operated.
* @param   [in] page   which page to be operated.
* @param   [out] data   buffer for data read from nand
* @param   [out] spare_tbl   buffer for file system infomation
* @param   [in] page_num   the number of page to be read
* @return    E_NANDERRORCODE
*/

static E_NANDERRORCODE Nand_ReadSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)
{
    E_NANDERRORCODE ret;
    T_U32 i;
    T_U8 *data_shift, *oob_shift;

    if(1 == plane_num)
    {
        if((1 == page_num) || (0 == ((nand->ChipCharacterBits) & FLAG_READ_CACHE))) //when page_num = 1 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i++)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_ReadSector_Ex(nand, chip,  block, page + i, data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    NF_INFO("ExSingleReading failed at block %d, page %d\n", block, page + i);
                    goto EXITR;
                }
            }
        }
        else
        {
              ret = Nand_ReadSector_Cache(nand, chip, block, page, data, spare_tbl, oob_len, page_num);
        }
    }
    else
    {
        if((2 == page_num) || (0 == ((nand->ChipCharacterBits) & FLAG_READ_CACHE)))//when page_num = 2 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i += 2)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_MultiReadSector_Ex(nand, chip,  block, page + (i >> 1), data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    NF_INFO("ExMultiReading failed at block %d, page %d\n", block, page + (i >> 1));
                    goto EXITR;
                }
            }
            
        }
        else
        {
            ret = Nand_MultiReadSector_Cache(nand, chip, plane_num, block, page, data, spare_tbl, oob_len, page_num);
        }
    }

EXITR:
    return ret;

}

/**
* @brief writing interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating if the chip was construted of odd-even planes ,1 for not, 2 for be
* @param   [in] block   which block to be operated.
* @param   [in] page   which page to be operated.
* @param   [in] data   buffer stored with the data to be writen
* @param   [in] spare_tbl   buffer stored with the file system infomation to be writen 
* @param   [in] page_num   the number of page to be writen
* @return    E_NANDERRORCODE
*/
static E_NANDERRORCODE Nand_WriteSector_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)
{
    E_NANDERRORCODE ret;
    T_U32 i;
    T_U8 *data_shift, *oob_shift;

    if(1 == plane_num)
    {
        if((1 == page_num) || (0 == ((nand->ChipCharacterBits) & FLAG_PROG_CACHE))) //when page_num = 1 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i++)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_WriteSector_Ex(nand, chip,  block, page + i, data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    NF_INFO("ExSingleWriting failed at block %d, page %d\n", block, page + i);
                    goto EXITW;
                }
            }
        }
        else
        {
              ret = Nand_WriteSector_Cache(nand, chip, block, page, data, spare_tbl, oob_len, page_num);
        }
    }
    else
    {
        if((2 == page_num) || (0 == ((nand->ChipCharacterBits) & FLAG_PROG_CACHE)))//when page_num = 2 or the cache operation is not supported
        {
            for(i = 0; i < page_num; i += 2)
            {
                data_shift = &data[i * nand->BytesPerSector];
                oob_shift = (T_U32)spare_tbl + (i * oob_len);
                ret = Nand_MultiWriteSector_Ex(nand, chip,  block, page + (i >> 1), data_shift, oob_shift, oob_len);
                
                if(NF_FAIL == ret)
                {
                    NF_INFO("ExMultiWriting failed at block %d, page %d\n", block, page + (i >> 1));
                    goto EXITW;
                }
            }
            
        }
        else
        {
            ret = Nand_MultiWriteSector_Cache(nand, chip, plane_num, block, page, data, spare_tbl, oob_len, page_num);
        }
    }

EXITW:

    return ret;
 
}

/**
* @brief erasing interface for exNFTL
* 
* @author yang_yiming
* @date 2011.11.14
*
* @param   [in] pNand_Info    the struct of nandflash.
* @param   [in] chip   which chip to be operated.
* @param   [in] plane_num  indicating if the chip was construted of odd-even planes ,1 for not, 2 for be
* @param   [in] block   which block to be operated.
* @return    E_NANDERRORCODE
*/

static E_NANDERRORCODE Nand_EraseBlock_Exnftl(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block)
{
    E_NANDERRORCODE ret;
        
    if((1 == plane_num) || (0 == nand->ChipCharacterBits >> 13))
    {
        ret = Nand_EraseBlock(nand, chip, block);
    }
    else//only if the plane_num = 2 & multi-operation supported, we called Nand_MultiEraseBlock
    {
        ret = Nand_MultiEraseBlock(nand, chip, plane_num, block);
    }

    return ret;
}

#if  1
extern T_PNANDFLASH  gNand_base;

T_U32 FHA_Nand_EraseBlock(T_U32 nChip,  T_U32 nPage)
{
    T_U32 ret = FHA_SUCCESS;
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&gNand_base[1]);
    
    if((drv_get_chip_version() == CHIP_3771_L))
    {
        ret = FHA_Nand_EraseBlock_L(nChip,nPage);
        return ret;
    }     
    if(0 != nand_eraseblock(pNF_Add->ChipPos[nChip], nPage, pNF_Add))
    {
         ret = FHA_FAIL;
    }

    return ret;
}

T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{
    T_U32 ret = FHA_SUCCESS;
    T_U32 block, page;

    if((drv_get_chip_version() == CHIP_3771_L))
    {
        ret = FHA_Nand_WritePage_L(nChip,nPage,pData,nDataLen,pOob,nOobLen,eDataType);
        return ret;
    }  

    block = nPage / gNand_base->PagePerBlock;
    page = nPage % gNand_base->PagePerBlock;

    if (FHA_DATA_BOOT != eDataType)
    {
        if(NF_SUCCESS != gNand_base->WriteSector(gNand_base, nChip, block, page, pData, pOob, nOobLen))
        {
             ret = FHA_FAIL;
        }
    }
    else
    {
        //if (!Burn_WriteBootPage(nPage, pData))
        {
            ret = FHA_FAIL;
        }    
    }    

    return ret;
}

T_U32 FHA_Nand_ReadPage(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{
    T_U32 ret = FHA_SUCCESS;
    T_U32 block, page;

    if((drv_get_chip_version() == CHIP_3771_L))
    {
        ret = FHA_Nand_ReadPage_L(nChip,nPage,pData,nDataLen,pOob,nOobLen,eDataType);
        return ret;
    } 

    block = nPage / gNand_base->PagePerBlock;
    page = nPage % gNand_base->PagePerBlock;

    if (FHA_DATA_BOOT != eDataType)
    {
        if(NF_SUCCESS != gNand_base->ReadSector(gNand_base, nChip, block, page, pData, pOob, nOobLen))
        {
             ret = FHA_FAIL;
        }
    }
    else
    {
        //if (!Burn_ReadBootPage(nPage, pData))
        {
            ret = FHA_FAIL;
        }    
    }    

    return ret;
}


T_BOOL ASA_ReadBytes(T_U32 chip, T_U32 rowAddr, T_U32 columnAddr, T_U8 data[], T_U32 len)
{
    T_PNandflash_Add pNF_Add = (T_PNandflash_Add)(&gNand_base[1]);
    T_U32 block = rowAddr / pNF_Add->PagesPerBlock;

    if((drv_get_chip_version() == CHIP_3771_L))
    {
        return FHA_Nand_ReadBytes_L(chip,rowAddr,columnAddr,data,len);
    }    
    if ((m_fake_block_per_die != m_true_block_per_die) && (block >= m_true_block_per_die))
    {
        block = (block % m_true_block_per_die) + m_fake_block_per_die;
        rowAddr = (block * pNF_Add->PagesPerBlock) + (rowAddr % pNF_Add->PagesPerBlock);
    }

    if(0 == nand_readbytes(pNF_Add->ChipPos[chip], rowAddr, columnAddr, pNF_Add, data, len))
    {
        return FHA_SUCCESS;
    }
    else
    {
        return FHA_FAIL;
    }    
} 

T_U32 FHA_GetNandChipCnt(T_VOID)
{
	if (AK_NULL == gNand_base || 0 == gNand_base->PlanePerChip)
	{
		return 1;
	}

	return gNand_base->PlaneCnt / gNand_base->PlanePerChip;
}

#endif
#endif

