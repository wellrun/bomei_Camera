/******************************************
 * Copyright (c) 2008, Anyka Co., Ltd. 
 * All rights reserved.
 * @author he_ying_gz
 * @date 2008-3-14
 ******************************************/

#ifndef __RES_PORT_H__
#define __RES_PORT_H__

#include "anyka_types.h"
#include "lib_res_api.h"
#include "Gbl_Resource.h"
#ifndef SPIBOOT
#define RES_FILE_PATH               DRI_A"AkResData.Bin"
#define RES_FILE_PATH_BAK           DRI_C"AkResData.Bin"
#else
#define RES_FILE_PATH               "RES"
#endif
#define MAX_DYNAMIC_CACHE_SIZE      (300*1024)  //(500 * 1024)    //(4*1024*1024)

#define Res_GetBinResByID(ptrAddr, bCanKill, ulResID, len)           Res_DynamicLoad((T_pDATA*)(ptrAddr), (bCanKill), (ulResID), (len))

typedef enum
{
        eRES_TYPE_STRING,           //string type
        eRES_TYPE_BINARY,           //binary(bmp,akbmp,sound) type
        eRES_TYPE_NUM
}T_RES_TYPE;

T_BOOL   Res_Init(T_VOID);
T_BOOL   Res_ChkOpenBackFile(T_VOID);

T_pCWSTR Res_GetStringByID(T_RES_STRING id);

T_BOOL Res_SetLanguage(T_RES_LANGUAGE language);

T_VOID Res_Free(T_VOID);

/////////////////////////////   res_lib    ///////////////////////////////////

extern void *          GetHdrResourceEleInfo(const T_WCHR* pthFile);

extern unsigned char   GetAnyResourceEleInfo(void* hdrInfo, unsigned long resID, unsigned long *startPos, unsigned long *szData);

extern unsigned long   GetAnyResourceEleData(void* hdrInfo, unsigned char* mem, unsigned long startPos, unsigned long szData);

#endif
