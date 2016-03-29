/**
 * @filename fwl_nandflash.h
 * @brief: AK3224M frameworks of nandflash driver.
 *
 * This file describe frameworks of nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-03
 * @version 1.0
 * @ref
 */

#ifndef __FWL_NANDFLASH_L_H__
#define __FWL_NANDFLASH_L_H__

#include "anyka_types.h" 
#include "nandflash.h"
#include "fha.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef REMAP_SUCCESS
#define REMAP_SUCCESS 0
#endif

#ifndef REMAP_FAIL
#define REMAP_FAIL 1
#endif

#ifndef Fwl_nand_eraseblock
#define Fwl_nand_eraseblock nand_eraseblock
#endif



typedef enum 
{
    NANDTEST_ERASEBLOCK = 1,
    NANDTEST_WRITESECTOR,
    NANDTEST_READSECTOR ,    
}NandOperate;

//********************************************************************


/**
 * @brief   initialization of nandflash frameworks.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_PNANDFLASH
 */
T_PNANDFLASH  FWL_Nand_Init_MTD(T_VOID);


/**
 * @brief   check bad blocks of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  T_BOOL
 */
T_BOOL Nand_IsBadBlock_L(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block );

/**
 * @brief   Set bad block of nandflash.
 *
 * @author  LiaoZhijun
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  T_VOID
 */
T_VOID Nand_SetBadBlock_L(T_PNANDFLASH hNF_Info, T_U32 chip, T_U32 block );

/**
 * @brief   read page for nand flash hidden area.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] nChip the chip selective.
 * @param   [in] nAbsPage the abslute pageno.
 * @param   [in] pMain the buffer which sectors are read into.
 * @param   [in] nMainLen the main buffer len.
 * @param   [in] pAdd buffer for read oob data.
 * @param   [in] nAddLen oob buffer len.
 * @param   [in] eDataType FHA data type. 
 * @return  E_NANDERRORCODE
 */  
T_U32 FHA_Nand_ReadPage_L(T_U32 nChip, T_U32 nAbsPage, T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType);
T_U32 FHA_Nand_WritePage_L(T_U32 nChip, T_U32 nAbsPage, const T_U8 *pMain, T_U32 nMainLen, T_U8 *pAdd, T_U32 nAddLen, E_FHA_DATA_TYPE eDataType);
T_U32 FHA_Nand_EraseBlock_L(T_U32 chip, T_U32 nAbsPage);
T_BOOL FHA_Nand_ReadBytes_L(T_U32 nChip, T_U32 nAbsPage, T_U32 nColumn, T_U8 *pBuf, T_U32 nBufLen);
/**
 * @brief   init nand flash according given nand param.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNandParam the struct of nandflash.
 * @return  NONE
 */
T_VOID FWL_Nand_Init(T_VOID * pNandParam);

#ifdef MTD_STRESS
T_VOID save_stress_info(NandOperate opr, T_U32 blk, T_U32 sector, T_U32 failFlag);
#endif


#ifdef __cplusplus
}
#endif

#endif


