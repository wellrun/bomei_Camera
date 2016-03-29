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

#ifndef __FWL_NANDFLASH_H__
#define __FWL_NANDFLASH_H__

#include "anyka_types.h"
#include "nandflash.h"
#include "nand_list.h"

#define BAD_BOLCK_SCAN

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Fwl_NandFlash Framework NandFlash Interface
 *  @ingroup Framework
 */
//********************************************************************
//********************************************************************

/**
 * @brief   initialization of nandflash frameworks.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_PNANDFLASH
 */
T_PNANDFLASH NandFlash_Init(T_VOID);


/**
 * @brief   reset all nandflash hardware.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @return  T_VOID
 */
T_VOID NandFlash_ResetAll(T_PNANDFLASH pNF_Info);  

/**
* @brief restore default read retrial scale value of nand
*
* @author  \b yangyiming
* @date    2011-9-21
* @param  [in]chip the chip to be operated
* @retrun T_VOID
*/
T_VOID Nand_Restore_Default_Scale(T_U32 chip);



T_BOOL Nand_IsBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block);
#ifdef BAD_BOLCK_SCAN
T_U32 FHA_Nand_EraseBlock(T_U32 nChip,  T_U32 nPage);
T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);
T_U32 FHA_Nand_ReadPage(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_BOOL ASA_ReadBytes(T_U32 chip, T_U32 rowAddr, T_U32 columnAddr, T_U8 data[], T_U32 len);
T_U32 FHA_GetNandChipCnt(T_VOID);
#endif
//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
