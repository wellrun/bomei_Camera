/*
 * @(#)Sd.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef		_SD_H_
#define		_SD_H_
typedef    struct tag_Medium *T_PMEDIUM;

#define SD_IS_USR(DriverInfo) ((MEDIUM_SD == (DriverInfo).nMainType) && (USER_PARTITION == (DriverInfo).nSubType))

typedef enum {
    eSD_INTERFACE_SDMMC = 0,
    eSD_INTERFACE_SDIO,
    eSD_INTERFACE_COUNT
} T_eINTERFACE_TYPE;

#ifdef OS_WIN32
T_VOID FS_Sd_SetEmulate(T_U8 DriverNo);
#endif
T_BOOL Fwl_Sd_HwInit(T_eINTERFACE_TYPE type);
T_BOOL Fwl_Sd_HwDestory(T_eINTERFACE_TYPE type);
T_BOOL Fwl_Sd_UnMount(T_eINTERFACE_TYPE type);
T_BOOL Fwl_Sd_Mount(T_eINTERFACE_TYPE type);
T_BOOL Fwl_Sd_GetInitState(T_eINTERFACE_TYPE type);
T_eINTERFACE_TYPE Fwl_Sd_GetInterfaceByID(T_U8 drvId);
T_BOOL Fwl_Sd_GetMountInfo(T_eINTERFACE_TYPE type, T_U8*FristId, T_U8*DrvCnt);
T_BOOL Fwl_Sd_DiskFormat(T_eINTERFACE_TYPE type);

T_U32 FHA_SD_Erase(T_U32 nChip,  T_U32 nPage);
T_U32 FHA_SDIO_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_U32 FHA_SDIO_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);
T_U32 FHA_SDMMC_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_U32 FHA_SDMMC_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);

//----------------------------------------------------------
#endif

