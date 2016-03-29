#ifndef __FWL_EMMC_H__
#define __FWL_EMMC_H__

#include "anyka_types.h"
#include "mtdlib.h"

T_PMEDIUM SDDisk_Initial(T_VOID);

T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage);
T_U32 FHA_SD_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_U32 FHA_SD_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);

#endif
