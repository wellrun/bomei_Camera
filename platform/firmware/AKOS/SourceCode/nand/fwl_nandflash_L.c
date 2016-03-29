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
#include "mtdlib.h"
#include "fwl_nandflash_L.h"
#include "nandflash.h"
#include "nand_list.h"
#include "arch_nand_L.h"
#include "string.h"
#include "fha_asa.h"
#include "hal_print.h"

#define NAND_SUPPORT_LARGE_PAGE 1
#define NAND_SUPPORT_SMALL_PAGE 1

#ifdef NAND_SUPPORT_SMALL_PAGE
#define OOB_BUF_LEN NAND_MAX_ADD_LEN * 4
#else
#define OOB_BUF_LEN NAND_MAX_ADD_LEN 
#endif
#define ALL_SECTION 0xFF
#define ASA_ADD_LEN 4
#define SPOT_PAGE_LIMIT 4096
#define VPAGE_SIZE  4096

typedef enum
{
    AREA_P0 = 0,
    AREA_BOOT,
    AREA_FSA,
    AREA_ASA,
 //   AREA_BIN,
    AREA_CNT
}E_AREA;
#pragma arm section zidata = "_bootbss1_"
T_U32 g_nPageSize;
T_U32 g_nPagePerBlock;
T_U32 g_nPagePerBlock_asa;
T_U32 g_nBlockPerChip;
T_U8  g_nChipCnt;
#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
static T_BOOL bSmall;
static T_U8 nRwPage;
static T_U8 nEBlock;
#else
#ifdef NAND_SUPPORT_SMALL_PAGE
#define bSmall AK_TRUE
#define nRwPage 4
#define nEBlock 8
#endif

#ifdef NAND_SUPPORT_LARGE_PAGE
#define bSmall AK_FALSE
#define nRwPage 1
#define nEBlock 1
#endif
#endif
T_NAND_DEVICE_INFO *m_pDevice;
//static T_U8 *m_pBuf_BadBlk = AK_NULL;
//static T_U8 m_buf_stat = 0;
static T_BOOL m_bEnhanceSLC;
static T_NAND_ECC_CTRL *m_apEcc[AREA_CNT];
static T_NAND_ECC_CTRL  m_ASAEcc;
#pragma arm section zidata
#pragma arm section rodata = "_drvbootconst_"
static const T_U8 m_aEraseErr[] = "NF_Erf ";
static const T_U8 m_aChipID[] = "NF";
static const T_U8 m_aSLC[] = "SLC";
static const T_U8 m_aRWErr[]="NF_RWf ";
#pragma arm section rodata

static T_NANDFLASH m_NandMTD[2];
#define NF_MSG "nand"
#pragma arm section code = "_drvbootcode_"

T_U8 bitnum(T_U32 i)
{
    T_U8 ret = 0;
    for(; (T_U32)(1<<ret) < i; ret++);

    return ret;
}

static T_VOID  config_device_data(T_NAND_DATA *pData, T_U8 *pMain, T_U8 *pAdd,T_U8 nPageCnt, T_U8 nSectCnt, E_AREA eArea)
{
    T_U32 nMaxSectCnt;
    
    pData->pEccCtrl = m_pDevice->ppEccCtrl[eArea];
    nMaxSectCnt = pData->pEccCtrl->nMainSectCnt;
    pData->nSectCnt = nMaxSectCnt < nSectCnt ? nMaxSectCnt : nSectCnt;
    pData->nPageCnt = nPageCnt;
    pData->pMain  = pMain;
    pData->pAdd  = pAdd;
}

static T_VOID  copy_add(T_U8 *pDest, T_U8 *pSrc, T_U8 nAddLen)
{
    if ((AK_NULL != pSrc) && (AK_NULL != pDest))
    {
//        while (nAddLen--)
        {
        memcpy(pDest, pSrc, nAddLen);
  //          *pDest++ = *pSrc++;
        }
    }
}

T_U32 nand_eraseblock_L(T_U32 nChip, T_U32 nAbsPage)
{
    T_U32 nRet = REMAP_SUCCESS;
    T_U32 nDevRet;
    T_NAND_ADDR Addr;
 
#ifdef NAND_SUPPORT_SMALL_PAGE
 if (bSmall)
 {
     nAbsPage <<= 2;
 }
#endif
    if (m_bEnhanceSLC)
    {
      nAbsPage = ((nAbsPage / g_nPagePerBlock_asa )* g_nPagePerBlock);
    }

    Addr.nTargetAddr = 0;
    Addr.nSectAddr = 0;
    Addr.nLogAbsPageAddr = nAbsPage;

    nDevRet = m_pDevice->FunSet.erase(m_pDevice, &Addr, nEBlock);

    if (NAND_FAIL_MASK & nDevRet)
    {
        akprintf(C2,NF_MSG,"erase failed:%d\n", nAbsPage);
        nRet = REMAP_FAIL;    
    }

    return nRet;
}

#pragma arm section code

#pragma arm section code = "_nand_read_"

static T_VOID  config_device_addr(T_NAND_ADDR *pAddr, T_U16 nTarget, T_U32 nBlock, T_U32 nPage, T_U16 nSect)
{
    pAddr->nTargetAddr = nTarget;
#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
            8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
            changed by Burntool, and we have to recovery it here for "nand_get_device".
            */
        nBlock <<= 3;
        nPage <<= 2;
    }
#endif
    pAddr->nLogAbsPageAddr = nPage + (nBlock << m_pDevice->LogInfo.nBlockShift);
    pAddr->nSectAddr = nSect;
}

static E_AREA  convert_area(T_U32 nAbsPage,T_U32 eDataType)
{
    E_AREA eArea;

    switch (eDataType)
    {
        case FHA_DATA_BOOT:
        {
           if (0 == nAbsPage)
            eArea = AREA_P0;
           else
            eArea = AREA_BOOT;
        }
        break;
        case FHA_DATA_FS:
        {
            eArea = AREA_FSA;
        }
        break;
        case FHA_DATA_ASA:
        case FHA_DATA_BIN:
        {
            eArea = AREA_ASA;
        }
        break;
    }

    return eArea;
}

E_NANDERRORCODE Erase_NBlock(T_U32 nChip, T_U32 nBlock, T_U32 nBlockCnt)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_ADDR Addr;

    config_device_addr(&Addr, nChip, nBlock, 0, 0);

    nDevRet = m_pDevice->FunSet.erase(m_pDevice, &Addr, nBlockCnt);
    
    if (NAND_FAIL_MASK & nDevRet)
    {
        akprintf(C2,NF_MSG,"erase failed B:%d\n", nBlock);

        eRet = NF_FAIL;    
    }
#ifdef MTD_STRESS
    save_stress_info(NANDTEST_ERASEBLOCK, nBlock, 0, eRet);
#endif
    return eRet;
}
#pragma arm section code

#pragma arm section code = "_nand_write_"

static E_NANDERRORCODE Write_Page(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{  
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[OOB_BUF_LEN];

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip)  \
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        akprintf(C1,NF_MSG,"Writing Error Arg: Chip %d, Block %d, Page %d Buff %x.\n", nChip, nBlock, nPage, pMain);
        return NF_FAIL;
    }
  

    config_device_addr(&Addr, nChip, nBlock, nPage, 0);

    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : aAdd, nRwPage, 
        ALL_SECTION, eArea);
    copy_add(aAdd, pAdd, nAddLen);
    nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);

    if (nDevRet & NAND_FAIL_MASK)
    {
       akprintf(C2,NF_MSG,"%s C%d B%d P%d,%.8x", __func__, nChip, nBlock, nPage,nDevRet);
        eRet = NF_FAIL;    
    }
#ifdef MTD_STRESS
    save_stress_info(NANDTEST_WRITESECTOR, nBlock, 0, eRet);
#endif
    return eRet;

}

static E_NANDERRORCODE Write_PageN(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U32 page_num, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{  
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    //T_U8 aAdd[OOB_BUF_LEN];
    T_U32 page_offset = 0;  //record page offset in a block
    T_U32 spare_tbl_offset = 0;
    T_U32 i;

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip)  \
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        akprintf(C1,NF_MSG,"Writing Error Arg: Chip %d, Block %d, Page %d Buff %x.\n", nChip, nBlock, nPage, pMain);
        return NF_FAIL;
    }

    spare_tbl_offset = (T_U32)pAdd - sizeof(nPage);
    page_offset = nPage;
    for(i = 0; i < page_num; i++)
    {
        spare_tbl_offset += nAddLen;
        *(T_U32 *)(spare_tbl_offset) = page_offset;
        page_offset++;
    }  

    config_device_addr(&Addr, nChip, nBlock, nPage, 0);

    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : pAdd, page_num, 
        ALL_SECTION, eArea);
    //copy_add(aAdd, pAdd, nAddLen);
    nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);

    if (nDevRet & NAND_FAIL_MASK)
    {
       akprintf(C2,NF_MSG,"%s C%d B%d P%d,%.8x", __func__, nChip, nBlock, nPage,nDevRet);
        eRet = NF_FAIL;    
    }

    return eRet;

}

static E_NANDERRORCODE Read_PageN(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U32 page_num, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    //T_U8 aAdd[OOB_BUF_LEN];

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip)\
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        akprintf(C1, NF_MSG,"Reading Error Arg: Chip %d, Block %d, Page %d Buff %x.\n", nChip, nBlock, nPage, pMain);
        return NF_FAIL;
    }
    
    config_device_addr(&Addr, nChip, nBlock, nPage, 0);
    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : pAdd, page_num, \
        ALL_SECTION, eArea);
   
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);

    //copy_add(pAdd, aAdd, nAddLen);
    
    if (nDevRet & NAND_FAIL_MASK)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d Fail!\n",nBlock, nPage);
        eRet = NF_FAIL;
    }
    else if (nDevRet & NAND_WARN_STRONG_DANGER)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d Strong Danger!\n",nBlock, nPage);
        eRet = NF_STRONG_DANGER;
    }
    else if (nDevRet & NAND_WARN_WEAK_DANGER)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d weak danger\n",nBlock, nPage);
        eRet = NF_WEAK_DANGER;
    }

    return eRet;
}


#pragma arm section code

#pragma arm section code = "_nand_read_"
static E_NANDERRORCODE Read_Page(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[OOB_BUF_LEN];

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip)\
        || (nPage >= g_nPagePerBlock) || (AK_NULL == pMain))
    {
        akprintf(C1, NF_MSG,"Reading Error Arg: Chip %d, Block %d, Page %d Buff %x.\n", nChip, nBlock, nPage, pMain);
        return NF_FAIL;
    }
    
    config_device_addr(&Addr, nChip, nBlock, nPage, 0);
    config_device_data(&Data, pMain, \
        AK_NULL == pAdd ? AK_NULL : aAdd, nRwPage, \
        ALL_SECTION, eArea);
   
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);

    copy_add(pAdd, aAdd, nAddLen);
    
    if (nDevRet & NAND_FAIL_MASK)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d Fail!\n",nBlock, nPage);
        eRet = NF_FAIL;
    }
    else if (nDevRet & NAND_WARN_STRONG_DANGER)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d Strong Danger!\n",nBlock, nPage);
        eRet = NF_STRONG_DANGER;
    }
    else if (nDevRet & NAND_WARN_WEAK_DANGER)
    {
        akprintf(C2, NF_MSG,"RS B %d P %d weak danger\n",nBlock, nPage);
        eRet = NF_WEAK_DANGER;
    }
#ifdef MTD_STRESS
        save_stress_info(NANDTEST_READSECTOR, nBlock, 0, eRet);
#endif
    return eRet;
}
#pragma arm section code

T_BOOL Nand_IsBadBlock_L(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block )
{
    T_U32 blk_per_chip, phyBlk;

    blk_per_chip = hNF_Info->BlockPerPlane * hNF_Info->PlanePerChip;
    phyBlk = chip * blk_per_chip + block;
    
    return FHA_check_bad_block(phyBlk); 

}

T_VOID Nand_SetBadBlock_L(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block )
{
    T_U32 blk_per_chip, phyBlk;

    blk_per_chip = hNF_Info->BlockPerPlane * hNF_Info->PlanePerChip;
    phyBlk = chip * blk_per_chip + block;
    
    FHA_set_bad_block(phyBlk);

}

E_NANDERRORCODE Nand_EraseBlock_L(T_PNANDFLASH hNF_Info, T_U32 nChip, T_U32 nBlock)
{
    return Erase_NBlock(nChip, nBlock, nEBlock);
}

E_NANDERRORCODE Nand_WriteSector_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd, T_U32 nAddLen)
{
    return Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
}

E_NANDERRORCODE Nand_ReadSector_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen)
{
    return Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
}

E_NANDERRORCODE Nand_ReadFlag_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pAdd, T_U32 nAddLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    T_U8 aAdd[NAND_MAX_ADD_LEN];

    if ((nChip >= g_nChipCnt) || (nBlock >= g_nBlockPerChip) \
       || (nPage >= g_nPagePerBlock) || (AK_NULL == pAdd))
    {
        akprintf(C1, NF_MSG,"Reading Error Arg: Chip %d, Block %d, Page %d Buff %x.\n", nChip, nBlock, nPage, pAdd);
        return NF_FAIL;
    }

    config_device_data(&Data, AK_NULL, aAdd, 1, 0, AREA_FSA);
    config_device_addr(&Addr, nChip, nBlock, nPage, bSmall ? 0 : Data.pEccCtrl->nMainSectCnt);

    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    copy_add(pAdd, aAdd, nAddLen);
    
    if (NAND_FAIL_MASK & nDevRet)
    {
        akprintf(C2, NF_MSG,"Read Flag Fail: Chip %d, Block %d Page %d\n", nChip, nBlock, nPage);
        eRet = NF_FAIL;    
    }
    
    return eRet;
}


E_NANDERRORCODE Nand_WriteFlag_L(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block, T_U32 sector, T_U8 *spare, T_U32 spare_len)
{
    return NF_SUCCESS;
}

E_NANDERRORCODE Nand_MutiWriteSector_L(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    return NF_FAIL;
}
E_NANDERRORCODE Nand_MutiReadSector_L(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len)
{
    return NF_FAIL;
}

E_NANDERRORCODE Nand_MultiCopyBack_L(T_PNANDFLASH nand, T_U32 chip, T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page)
{
    return NF_FAIL;
}

E_NANDERRORCODE Nand_MultiEraseBlock_L(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block)
{
    return NF_FAIL;
}
#pragma arm section code = "_nand_write_"
                              
E_NANDERRORCODE Nand_WriteSector_Exnftl_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nPlaneCnt, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, T_U32 nPageCnt)
{  
       E_NANDERRORCODE ret;
       //T_U32 i;

       //for(i = 0; i < nPageCnt; i++)
       {
           ret = Write_PageN(nChip, nBlock, nPage, nPageCnt, pMain, pAdd, nAddLen, AREA_FSA);
    
           if(NF_FAIL == ret)
           {
               goto L_EXIT;
           } 
           nPage++;
           pMain += g_nPageSize;
           pAdd += nAddLen;
       }
    
    L_EXIT:
       return  ret;
#if 0
    return Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
#endif
}
#pragma arm section code
#pragma arm section code = "_nand_read_"

E_NANDERRORCODE Nand_ReadSector_Exnftl_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nPlaneCnt, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, T_U32 nPageCnt)
{
    E_NANDERRORCODE ret;
    //T_U32 i;
    

    //for(i = 0; i < nPageCnt; i++)
    {
        ret = Read_PageN(nChip, nBlock, nPage, nPageCnt, pMain, pAdd, nAddLen, AREA_FSA);

        if(NF_FAIL == ret)
        {
           goto L_EXIT;
        }        
        nPage++;
        pMain += g_nPageSize;
        pAdd += nAddLen;
    }
    
L_EXIT:
    return  ret;
#if 0
    return Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, ASA_ADD_LEN ==  nAddLen ? AREA_ASA : AREA_FSA);
#endif
}
#pragma arm section code

E_NANDERRORCODE Nand_EraseBlock_Exnftl_L(T_PNANDFLASH nand, T_U32 nChip, T_U32 plane_num, T_U32 nBlock)
{
    return Erase_NBlock(nChip, nBlock, nEBlock);
}
#ifdef NAND_SUPPORT_SMALL_PAGE
typedef E_NANDERRORCODE (*pREAD_WRITE_PAGE)(T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U8 *pMain, T_U8 *pAdd , T_U32 nAddLen, E_AREA eArea);

static T_U32 Small_Page0(T_U8 *pMain, T_BOOL bWrite)
{    
    E_NANDERRORCODE nRet = NF_FAIL;
    pREAD_WRITE_PAGE pRwFun = bWrite?Write_Page:Read_Page;
    
    bSmall = AK_FALSE;
    
    nRwPage = 1;
    nRet = pRwFun(0, 0, 0, pMain, AK_NULL, 0, AREA_P0);
    
    nfc_config_randomize(0, 0, AK_FALSE, AK_TRUE);

    if (NF_FAIL != nRet)
    {
        nRwPage = 3;
        nRet = pRwFun(0, 0, 1, pMain + 512, AK_NULL, 0, AREA_BOOT);
    }

    nRwPage = 4;
    bSmall = AK_TRUE;
    
    return (NF_FAIL == nRet)?FHA_FAIL:FHA_SUCCESS;
}

#endif

E_NANDERRORCODE Nand_ReadBytes_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U16 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    
    config_device_addr(&Addr, nChip, nBlock, nPage, nColumn);
    Data.pEccCtrl = AK_NULL;
    Data.nSectCnt = nBufLen;
    Data.nPageCnt = 1;
    Data.pMain  = pBuf;
    Data.pAdd  = AK_NULL;
    
    nDevRet = m_pDevice->FunSet.read(m_pDevice, &Addr, &Data);
    
    if (NAND_FAIL_MASK & nDevRet)
    {
        akprintf(C2, NF_MSG, "fail!\n");
        eRet= NF_FAIL;
    }
    
     
    return eRet;
}

E_NANDERRORCODE Nand_WriteBytes_L(T_PNANDFLASH pNand, T_U32 nChip, T_U32 nBlock, T_U32 nPage, T_U16 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = NF_SUCCESS;
    T_U32 nDevRet;
    T_NAND_DATA Data;
    T_NAND_ADDR Addr;
    
    config_device_addr(&Addr, nChip, nBlock, nPage, nColumn);
    Data.pEccCtrl = AK_NULL;
    Data.nSectCnt = nBufLen;
    Data.nPageCnt = 1;
    Data.pMain  = pBuf;
    Data.pAdd  = AK_NULL;
    
    nDevRet = m_pDevice->FunSet.program(m_pDevice, &Addr, &Data);

    if (NAND_FAIL_MASK & nDevRet)
    {
        akprintf(C2, NF_MSG,"faile!\n");
        eRet = NF_FAIL;    
    }
    return eRet;

}


T_BOOL FHA_Nand_ReadBytes_L(T_U32 nChip, T_U32 nAbsPage, T_U32 nColumn, T_U8 *pBuf, T_U32 nBufLen)
{
    E_NANDERRORCODE eRet = FHA_SUCCESS;
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);

    if (NF_FAIL ==  Nand_ReadBytes_L(AK_NULL, nChip, nBlock, nPage, nColumn, pBuf, nBufLen))
        eRet= FHA_FAIL;
     
    return eRet;
}


T_U32 FHA_Nand_EraseBlock_L(T_U32 chip, T_U32 nAbsPage)
{
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
#ifdef NAND_SUPPORT_SMALL_PAGE
    
    if (bSmall)
    {
        nBlock >>= 1;
    }
#endif
    
    //add for ENHANCE SLC
    if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
    }

    
    if (NF_FAIL == Erase_NBlock(chip, nBlock, nEBlock))
    {
        akprintf(C2, NF_MSG,"FHA EB:%d,%d\n",nAbsPage,nBlock);
        return FHA_FAIL;
    }
    else
    {
        return FHA_SUCCESS;
    }
}

T_U32 FHA_Nand_WritePage_L(T_U32 nChip, T_U32 nAbsPage, const T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType)
{
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);
    E_AREA eArea = convert_area(nAbsPage, eDataType);
    E_NANDERRORCODE nRet;
    T_BOOL bElcMode;//add for ENHANCE SLC

    if (0 == nAbsPage)//rom read the page0 with Randomizer enable, so we should write the data with Randomizer enable too.
    {
        nfc_config_randomize(0, 0, AK_TRUE, AK_FALSE);
    }
#ifdef NAND_SUPPORT_SMALL_PAGE
    if (bSmall)
    {
        if (0 == nAbsPage)
        {
            return Small_Page0(pMain, AK_TRUE);
        }
        else
        {
            if (nBlock & 0x1)
                nPage += 32;

            nBlock >>= 1;
        }

    }
#endif

    //add for ENHANCE SLC
    if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
        m_pDevice->FunSet.ctrl(OP_GET_LOWER_PAGE, nAbsPage % g_nPagePerBlock_asa, &nPage);
        bElcMode = AK_TRUE;
        m_pDevice->FunSet.ctrl(OP_ENABLE_SLC, nChip, (T_VOID *)&bElcMode);
    }

    nRet = Write_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, eArea);
    
    //temp add for ENHANCE SLC
    if (AK_TRUE == m_bEnhanceSLC)
    {
        bElcMode = AK_FALSE;
        m_pDevice->FunSet.ctrl(OP_ENABLE_SLC, nChip, (T_VOID *)&bElcMode);
    }

    
    if (0 == nAbsPage)
    {
        nfc_config_randomize(0, 0, AK_FALSE, AK_FALSE);
    }

    //should we do some thing for page0?
    if (NF_FAIL == nRet)
        return FHA_FAIL;
    else
        return FHA_SUCCESS;

}

T_U32 FHA_Nand_ReadPage_L(T_U32 nChip, T_U32 nAbsPage, T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType)
{ 
    T_U32 nBlock = nAbsPage >> m_pDevice->LogInfo.nBlockShift;
    T_U32 nPage = nAbsPage & ((1 << m_pDevice->LogInfo.nBlockShift) - 1);
    E_AREA eArea = convert_area(nAbsPage, eDataType);
    E_NANDERRORCODE nRet;

    if (0 == nAbsPage)//rom read the page0 with Randomizer enable, so we should write the data with Randomizer enable too.
    {
        nfc_config_randomize(0, 0, AK_TRUE, AK_TRUE);
    }
#ifdef NAND_SUPPORT_SMALL_PAGE
    
    if (bSmall)
    {
        if (0 == nAbsPage)
        {
            return Small_Page0(pMain, AK_FALSE);
        }
        else
        {
            if (nBlock & 0x1)
                nPage += 32;

            nBlock >>= 1;
        }
    }
#endif

    //add for ENHANCE SLC
    if (AK_TRUE == m_bEnhanceSLC)
    {
        nBlock = nAbsPage / g_nPagePerBlock_asa;
        m_pDevice->FunSet.ctrl(OP_GET_LOWER_PAGE, nAbsPage % g_nPagePerBlock_asa, &nPage);
    }


    nRet = Read_Page(nChip, nBlock, nPage, pMain, pAdd, nAddLen, eArea);
    if (0 == nAbsPage)
    {
        nfc_config_randomize(0, 0, AK_FALSE, AK_TRUE);
    }
    if (NF_FAIL == nRet)
        return FHA_FAIL;
    else
        return FHA_SUCCESS;

}

T_PNANDFLASH FWL_Nand_Init_MTD(T_VOID)
{
    T_PNANDFLASH pNandMtdInfo = &m_NandMTD;

    if (AK_NULL == m_pDevice)
    {
        akprintf(C2, NF_MSG, "No Nand Device!\n");
        pNandMtdInfo = AK_NULL;
    }
    
    pNandMtdInfo->ChipCharacterBits = 0x1;//step = 1
    pNandMtdInfo->NandType = 0;//NANDFLASH_TYPE_SAMSUNG useless
    pNandMtdInfo->BytesPerSector = m_pDevice->LogInfo.nLogicBPP;
    pNandMtdInfo->BlockPerPlane = m_pDevice->LogInfo.nLogicBPC;
    #ifdef MTD_STRESS
    pNandMtdInfo->BlockPerPlane = m_pDevice->LogInfo.nLogicBPC - 100;
    #endif
    pNandMtdInfo->PlanePerChip = 1;
    pNandMtdInfo->PlaneCnt = g_nChipCnt;
    pNandMtdInfo->PagePerBlock =  m_pDevice->LogInfo.nLogicPPB;
    pNandMtdInfo->SectorPerPage = 1;   
    pNandMtdInfo->WriteSector = Nand_WriteSector_L;
    pNandMtdInfo->ReadSector = Nand_ReadSector_L;
    pNandMtdInfo->ReadFlag = Nand_ReadFlag_L;
    pNandMtdInfo->WriteFlag = Nand_WriteFlag_L;
    pNandMtdInfo->CopyBack = AK_NULL;

    pNandMtdInfo->EraseBlock = Nand_EraseBlock_L;
    pNandMtdInfo->IsBadBlock = Nand_IsBadBlock_L;
    pNandMtdInfo->SetBadBlock  = Nand_SetBadBlock_L;

    pNandMtdInfo->MultiEraseBlock = Nand_MultiEraseBlock_L;
    pNandMtdInfo->MultiCopyBack = Nand_MultiCopyBack_L;
    pNandMtdInfo->MultiWrite = Nand_MutiWriteSector_L;
    pNandMtdInfo->MultiRead = Nand_MutiReadSector_L;

    pNandMtdInfo->ExEraseBlock = Nand_EraseBlock_Exnftl_L;
    pNandMtdInfo->ExRead = Nand_ReadSector_Exnftl_L;
    pNandMtdInfo->ExReadFlag = Nand_ReadFlag_L;
    pNandMtdInfo->ExWrite = Nand_WriteSector_Exnftl_L;
    pNandMtdInfo->ExIsBadBlock = Nand_IsBadBlock_L;    
    pNandMtdInfo->ExSetBadBlock = Nand_SetBadBlock_L;

#ifdef NAND_SUPPORT_SMALL_PAGE   
    if (bSmall)
    {
        pNandMtdInfo->BytesPerSector = 2048;
        pNandMtdInfo->PagePerBlock = 64;
        pNandMtdInfo->BlockPerPlane >>= 3;
    }
#endif

    return pNandMtdInfo;

}

#pragma arm section code
#pragma arm section code = "_drvbootinit_"

T_VOID FWL_Nand_Init(T_VOID * pNandParam)
{
    T_U32 ChipID[2], ChipCnt;
    T_U32 nGpioPos;
    T_NAND_PARAM NandParam;
    
#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
    bSmall = AK_FALSE;
    nRwPage = 1;
    nEBlock = 1;
#endif

    //reg_nand();
    nGpioPos = 0xFFFFFFFF;
    nand_setCe_getID(&ChipID[0], &ChipCnt, &nGpioPos, NFC_SUPPORT_CHIPNUM);

    akprintf(C3,NF_MSG,"Chip ID:%.8x,Chip Count:%d\n", ChipID[0], ChipCnt);    
    memcpy(&NandParam, pNandParam, sizeof(T_NAND_PARAM));

#ifdef NAND_SUPPORT_SMALL_PAGE

    if (1 == NandParam.col_cycle)    
    {    
        /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
            8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
            changed by Burntool, and we have to recovery it here for "nand_get_device".
            */
        
#if defined(NAND_SUPPORT_SMALL_PAGE) && defined(NAND_SUPPORT_LARGE_PAGE)
        bSmall = AK_TRUE;
        nRwPage = 4;
        nEBlock = 8;
#endif
        NandParam.page_per_blk = 32;
        NandParam.page_size = 512;
        //NandParam.blk_num >>= 3;
        NandParam.plane_blk_num = NandParam.blk_num;
        NandParam.group_blk_num = NandParam.blk_num;        
    }
#endif
    
    m_pDevice = nand_get_device(&NandParam, ChipCnt);

    if (AK_NULL == m_pDevice)
    {
    }
    else
    {   
        m_pDevice->FunSet.ctrl(OP_GET_ECC, 0, m_apEcc);
        m_pDevice->ppEccCtrl = m_apEcc;
        m_apEcc[AREA_ASA]= &m_ASAEcc;
        //  m_apEcc[AREA_ASA] = m_apEcc[AREA_FSA];
        m_ASAEcc.bSeperated = m_apEcc[AREA_FSA]->bSeperated;
        m_ASAEcc.nAddEcc = m_apEcc[AREA_FSA]->nAddEcc;
        m_ASAEcc.nAddLen = m_apEcc[AREA_FSA]->nAddLen;
        m_ASAEcc.nMainEcc = m_apEcc[AREA_FSA]->nMainEcc;
        m_ASAEcc.nMainLen = m_apEcc[AREA_FSA]->nMainLen; //512;
        m_ASAEcc.nSectOffset = bSmall ? 0 : m_apEcc[AREA_FSA]->nSectOffset;;
       // m_ASAEcc.nMainSectCnt = (m_pDevice->LogInfo.nLogicBPP > SPOT_PAGE_LIMIT ? SPOT_PAGE_LIMIT : m_pDevice->LogInfo.nLogicBPP) / m_apEcc[AREA_FSA]->nMainLen;
        m_ASAEcc.nMainSectCnt = m_apEcc[AREA_FSA]->nMainSectCnt;
        g_nChipCnt = ChipCnt;
        g_nPageSize = m_pDevice->LogInfo.nLogicBPP;                            
        g_nBlockPerChip = m_pDevice->LogInfo.nLogicBPC;
        g_nPagePerBlock = m_pDevice->LogInfo.nLogicPPB;

#ifdef NAND_SUPPORT_SMALL_PAGE
       
        if (1 == NandParam.col_cycle)    
        {    
            /*Considering the lack of memory, we conbines 4 small(512B-size) pages into a 2KB-size page,
             *8 small(512B * 32) blocks into a 128K(2KB * 64 ) block. Plat can only get the param(a fake one) 
             *changed by Burntool, and we have to recovery it here for "nand_get_device".
             */
            g_nPageSize = 2048;                            
            g_nBlockPerChip = m_pDevice->LogInfo.nLogicBPC >> 3;

            g_nPagePerBlock = 64;
        }
#endif
        g_nPagePerBlock_asa = g_nPagePerBlock;
        
            //add for ENHANCE SLC
            /* when using slc mode, the upper page in every paired page will be skipped,
            so the page amount in fha area should be divided by 2.
            */
        if (0 != (FLAG_ENHANCE_SLC & NandParam.flag))
        {
            g_nPagePerBlock_asa >>= 1;
            m_bEnhanceSLC = AK_TRUE;
            akprintf(C3,NF_MSG,"SLC:PagePerBlockAsa = %d\n", g_nPagePerBlock_asa);
        }


    }
    
}
#pragma arm section code

#endif

