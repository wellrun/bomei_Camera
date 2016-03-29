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

#ifdef __cplusplus
extern "C" {
#endif

T_BOOL Fwl_NandHWInit(T_U32 gpio_ce2, T_U32 gpio_ce3, T_U32* ChipID, T_U32* ChipCnt);

                                     
/**
 * @brief   write 1 page data to nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] data buffer for read page, should be a page size.
 * @param   [in] oob buffer for oob infomation, maybe 4 bytes or 8 bytes.
 * @param   [in] oob_len for length of oob infomation.

 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 *oob, T_U32 oob_len);

/**
 * @brief   read 1 page data from nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] page which page will be read.
 * @param   [in] data buffer for read page, should be a page size.
 * @param   [in] oob buffer for oob infomation, maybe 4 bytes or 8 bytes.
 * @param   [in] oob_len for length of oob infomation.
 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block, T_U32 page, T_U8 data[], T_U8 *oob, T_U32 oob_len);

/**
 * @brief   copy one physical page to another one, soft hardware.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] SourceBlock read the source block.
 * @param   [in] DestBlock write to destination block.
 * @param   [in] page the page of the block will be copy.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_CopyBack(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);

/** 
 * @brief   erase 1 block of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] block which block whill be erased.
 * @return  T_U32
 */
E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block);

/**
 * @brief   initialization of nandflash frameworks.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] T_VOID.
 * @return  T_PNANDFLASH
 */
//T_PNANDFLASH Nand_Init(T_NAND_PHY_INFO *nand_info);

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
T_BOOL Nand_IsBadBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block);

T_VOID Nand_SetBadBlock(T_PNANDFLASH pNF_Info, T_U32 chip, T_U32 block );

T_U32 FHA_Nand_EraseBlock(T_U32 nChip,  T_U32 nPage);

T_U32 FHA_Nand_WritePage(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);

T_U32 FHA_Nand_ReadPage(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);

T_BOOL ASA_ReadBytes(T_U32 chip, T_U32 rowAddr, T_U32 columnAddr, T_U8 data[], T_U32 len);


#ifdef __cplusplus
}
#endif

#endif
