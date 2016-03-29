/*
 * @(#)Sd.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef        _SD_H_
#define        _SD_H_

#if (defined (SDIOBOOT) || (defined (SDMMCBOOT)))

typedef    struct tag_Medium *T_PMEDIUM;

#define SD_IS_USR(DriverInfo) ((MEDIUM_SD == (DriverInfo).nMainType) && (USER_PARTITION == (DriverInfo).nSubType))
#define SD_IS_SYS(DriverInfo) ((MEDIUM_SD == (DriverInfo).nMainType) && (SYSTEM_PARTITION == (DriverInfo).nSubType))

typedef enum {
    eSD_INTERFACE_SDMMC = 0,
    eSD_INTERFACE_SDIO,
    eSD_INTERFACE_COUNT
} T_eINTERFACE_TYPE;

typedef enum {
    eMedium_NULL = 0,
    eMedium_MountSuccess,
    eMedium_MountFail,
    eMedium_UnMountSuccess,
    eMedium_UnMountFail,
}e_MEDIUM_STATE;

typedef struct
{
    T_U8 driverCnt;
    T_U8 firstDrvNo;
    T_VOID *handle;
} T_PARTITION_INFO, *TP_PARTITION_INFO;

T_BOOL Fwl_Sd_MountSdBootCard(T_VOID);
T_eINTERFACE_TYPE Fwl_Sd_GetInterfaceByID(T_U8 drvId);
//----------------------------------------------------------
#endif
#endif

