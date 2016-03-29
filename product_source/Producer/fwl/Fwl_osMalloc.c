/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：Fwl_osMalloc.c
 * Function：This header file is API for Memory Library
 *
 * Author：ZhangMuJun
 * Date：
 * Version：2.0.1
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#include "Fwl_osMalloc.h"
#include "mem_api.h"

#define    MAX_RAMBUFFER_SIZE        (2048*1024)

static T_U8 gb_RAMBuffer[MAX_RAMBUFFER_SIZE];

static T_GLOBALMEMHandle        gb_hGlobalMemory = AK_NULL;

/***********************static function************************/

T_VOID Fwl_MallocInit(T_VOID)
{  
    gb_hGlobalMemory = Ram_Initial(gb_RAMBuffer, sizeof(gb_RAMBuffer));    
}

T_pVOID Fwl_MallocAndTrace(T_U32 size, T_pSTR filename, T_U32 line)
{
    T_pVOID ptr;
    
    ptr = Ram_Alloc(gb_hGlobalMemory, size, filename, line);

    return ptr;
}

T_pVOID    Fwl_ReMallocAndTrace(T_pVOID var, T_U32 size, T_pSTR filename, T_U32 line)
{
    T_pVOID ptr;

    ptr = Ram_Realloc(gb_hGlobalMemory, var, size, filename, line);

     return ptr;
}

T_pVOID Fwl_FreeAndTrace(T_pVOID var, T_pSTR filename, T_U32 line) 
{
    return Ram_Free(gb_hGlobalMemory, var, filename, line); //一些野指针释放时需要调试信息
}  

/* debug function : should merge into one */
T_U32 Fwl_GetTotalRamSize(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szTotal;
}

T_U32 Fwl_RamUsedBlock(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.blkUsed;
}

T_U32 Fwl_GetUsedRamSize(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szUsed;
}

T_U32 Fwl_RamGetBlockNum(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szBlocks-info.blkUsed;
}

T_U32 Fwl_RamGetBlockLen()
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.align;
}

T_U32 Fwl_GetRemainRamSize(T_VOID)
{
     GLOBALMEMInfo info;
     Ram_GetRamInfo(gb_hGlobalMemory, &info);

     return info.szSpare;
}

T_pVOID Fwl_Malloc(T_U32 size)
{
   return Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

T_pVOID Fwl_ReMalloc(T_pVOID var, T_U32 size)
{
    return Fwl_ReMallocAndTrace((var), (size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

T_pVOID Fwl_Free(T_pVOID var)
{
    return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}


T_VOID  Ram_Lock(T_U32 dwLock)
{

}

T_VOID  Ram_Unlock(T_U32 dwLock)
{
}


