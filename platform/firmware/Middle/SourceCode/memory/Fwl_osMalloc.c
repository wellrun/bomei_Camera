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
#include "eng_string.h"
#include "anyka_types.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "akos_api.h"
#include "lib_res_api.h"
#include "raminit.h"


#ifdef OS_WIN32
#include <malloc.h>
#include <windows.h>
    #define Dbg_Malloc(_size) malloc(_size)
    #define Dbg_Free(_var) (free(_var),AK_NULL)
#else
    #define Dbg_Malloc(_size) Fwl_Malloc(_size)
    #define Dbg_Free(_var) Fwl_Free(_var)
#endif

//#define USE_TWO_BLOCK_MEMORY_BUFFER

#ifndef ENABLE_MEMORY_DEBUG
    #include "eng_debug.h"
    #define ENABLE_MEMORY_DEBUG    //内存越界自动监测器
#ifndef OS_WIN32
    #define DEBUG_TRACE_MEMORY_LEAK
#endif
    #define USE_MEMORY_MULTITHREAD
#endif

#ifdef AUTO_MEMORY_LEAK_MONITOR
static Ram_EnterStateMachine    gb_enterSectionHook = AK_NULL;
static Ram_LeaveStateMachine    gb_leaveSectionHook = AK_NULL;
#endif


#define SUPER_LARGE_SIZE        (190*1024)
#ifdef USE_TWO_BLOCK_MEMORY_BUFFER 
static T_GLOBALMEMHandle        gb_hLargeMemory = AK_NULL;
static T_pDATA                    gb_lpLargebuf = AK_NULL;
#endif

enum 
{
	MALLOC_NOT_INIT,
	MALLOC_INITING,
	MALLOC_INIT_OK
	
};

static int m_iInitFlag=MALLOC_NOT_INIT;


/***********************static function************************/

/*
 * @ 判断文件名地址合法性
 */
static T_S8* GetTraceFileNameValidate(T_pVOID ptr)
{
    const T_S8 *st_trace_name[2] = {"Null Filename, maybe not input!", "Corrupt Filename, Maybe beyond is happend!"};
#ifdef OS_WIN32
    const T_U32  st_min_ram_addr = 0x00001000;
    const T_U32  st_max_ram_addr = 0x2fffffff;
#else
    T_U32  st_min_ram_addr = 0;//RAM_BASE_ADDR;
    T_U32  st_max_ram_addr = 0;//RAM_BASE_ADDR + SDRAM_SIZE;
    Ram_getaddr(&st_min_ram_addr, &st_max_ram_addr);
#endif
    
    if(AK_NULL == ptr)
        return (T_S8*)st_trace_name[0];

    if ((((T_U32)ptr >= st_min_ram_addr) && ((T_U32)ptr <= st_max_ram_addr)))
        return (T_S8*)ptr;
    else
        return (T_S8*)st_trace_name[1];    
}


/*
 * @ 打印内存枚举等通用内存信息钩子回调函数
 */
static T_VOID PrintOneCommonTraceInfo(T_GLOBALMEMHandle hMemory, const T_MEMORY_TRACE *map, T_pVOID attach)
{
    if(Ram_GetGlobalMem() == hMemory)
    {
        if(map->addr==0 && map->size==0 && map->line==0)
        {
            Fwl_Print(C3, M_RAM_MLC, "%s\r\n", map->filename);
            return;
        }

        if(0 == map->old)
        {
        	T_S8 *fmt;
			T_U32 line  = map->line;
            T_S8 *strFn = GetTraceFileNameValidate(map->filename);
			fmt = "FileName:%s,\t\tLine:%ld,\t\tAddr:%lx,\t\tSize:%lu,\t\treqSize:%lu\r\n";

			if(map->line == 0xEFFC)
			{
				strFn = "SwordII_elfd.txt";
				line = (T_U32)map->filename;
				fmt = "FileName:%s,\t\tLine:%lx,\t\tAddr:%lx,\t\tSize:%lu,\t\treqSize:%lu\r\n";
			}

            Fwl_Print(C3, M_RAM_MLC, fmt,
                    strFn,
                    line,
                    (T_U32)map->addr,
                    map->size,
					map->reqSize);
        }
    }
}

/*
 * @ 专用于打印内存泄漏枚举信息钩子回调函数
 */
static T_VOID PrintOneMemoryLeakTraceInfo(T_GLOBALMEMHandle hMemory, const T_MEMORY_TRACE *map, T_pVOID attach)
{
    if(Ram_GetGlobalMem() == hMemory)
    {
        if(map->addr==0 && map->size==0 && map->line==0)
        {
            Fwl_Print(C3, M_RAM_MLC, "%s\r\n", map->filename);//Auto Leak check enter/leave stack machine
            return;
        }

        if(0 == map->old)
        {
       		T_S8 *fmt;
			T_U32 line  = map->line;
            T_S8 *strFn = GetTraceFileNameValidate(map->filename);
			fmt = "FileName:%s,\t\tLine:%ld,\t\tAddr:%lx,\t\tSize:%lu,\t\treqSize:%lu\r\n";

			Fwl_Print(C3, M_RAM_MLC, "******************* Memor Leak ******************* \r\n");

			if(map->line == 0xEFFC)
			{
				strFn = "SwordII_elfd.txt";
				line = (T_U32)map->filename;
				fmt = "FileName:%s,\t\tLine:%lx,\t\tAddr:%lx,\t\tSize:%lu,\t\treqSize:%lu\r\n";
			}

            Fwl_Print(C3, M_RAM_MLC, fmt,
                    strFn,
                    line,
                    (T_U32)map->addr,
                    map->size,
					map->reqSize);

        }
    }
}



T_VOID Fwl_MallocInit(T_VOID)
{
	
	if (MALLOC_NOT_INIT != m_iInitFlag)  //already ini
		return ;

    m_iInitFlag =MALLOC_INITING;//WHEN IS INITING , CAN NOT DO MALLOC OR INIT .
    Ram_MallocInit();
    m_iInitFlag =MALLOC_INIT_OK;


}

T_VOID Fwl_MallocSupportMultithread(T_VOID)
{
	Ram_MallocSupportMultithread();
}

T_pVOID Fwl_MallocAndTrace(T_U32 size, T_pSTR filename, T_U32 line)
{
    T_pVOID ptr;

    if ( MALLOC_NOT_INIT == m_iInitFlag)
    {
    	Fwl_MallocInit();
    	m_iInitFlag = MALLOC_INIT_OK;
    }else if (MALLOC_INITING == m_iInitFlag)
    	return AK_NULL;
    
    
#ifdef USE_TWO_BLOCK_MEMORY_BUFFER
    if((size > SUPER_LARGE_SIZE) && gb_hLargeMemory)
    {
        Fwl_Print(C3, M_RAM_MLC, "alloc super %ld \r\n", size);
        ptr = QQ_MallocAndTrace(gb_hLargeMemory, size, filename, line);
        if(ptr != AK_NULL)
            return ptr;
    }
#endif

    if (0 == size)
    {
        return AK_NULL;
    }
    
    ptr = Ram_Alloc(Ram_GetGlobalMem(), size, filename, line);

    if(AK_NULL == ptr)
    {
        Fwl_ReleaseMemory();
        ptr = Ram_Alloc(Ram_GetGlobalMem(), size, filename, line);
		if (AK_NULL == ptr)
			Fwl_Print(C3, M_RAM_MLC, "malloc %d failed,filename=%s,line=%d\n", size, filename, line);
    }

    return ptr;
}

T_pVOID    Fwl_ReMallocAndTrace(T_pVOID var, T_U32 size, T_pSTR filename, T_U32 line)
{
    T_pVOID ptr;

    if ( MALLOC_NOT_INIT == m_iInitFlag)
    {
    	Fwl_MallocInit();
    	m_iInitFlag = MALLOC_INIT_OK;
    }else if (MALLOC_INITING == m_iInitFlag)
    	return AK_NULL;
    	
#ifdef USE_TWO_BLOCK_MEMORY_BUFFER    
	{
	    T_WILD_TYPE type;

	    if((size > SUPER_LARGE_SIZE) && gb_hLargeMemory)
	    {
	        //Fwl_Print(C3, M_RAM_MLC, "alloc super %d \r\n", size);
	        if (WILD_OK == type)
	        {    
	                ptr = QQ_ReMallocAndTrace(gb_hLargeMemory, var, size, filename, line);
	            return ptr;
	        }
	        else
	        {
	            ptr = Fwl_Malloc(size);
	            if(ptr != AK_NULL)
	                memcpy(ptr, var, size);
	            Ram_Free(Ram_GetGlobalMem(), var, filename, line);
	            return ptr;
	        }            
	    }
	    else if (WILD_OK == type)
	    {
	        ptr = Fwl_Malloc(size);
	        if(ptr != AK_NULL)
	            memcpy(ptr, var, size);
	        Ram_Free(gb_hLargeMemory, var, filename, line);
	        return ptr;
	    }
    }
#endif

    if (0 == size)
    {
        return AK_NULL;
    }
    
    ptr = Ram_Realloc(Ram_GetGlobalMem(), var, size, filename, line);

    if(AK_NULL == ptr)
    {
        Fwl_ReleaseMemory();
        ptr = Ram_Realloc(Ram_GetGlobalMem(), var, size, filename, line);
    }


    return ptr;
}

T_pVOID Fwl_FreeAndTrace(T_pVOID var, T_pSTR filename, T_U32 line) 
{
    if ( MALLOC_INIT_OK != m_iInitFlag) //not init , return directly
    {
    	return AK_NULL; 
    }

#ifdef USE_TWO_BLOCK_MEMORY_BUFFER 
    if(gb_hLargeMemory && ((T_pDATA)var>gb_lpLargebuf))
    {
        Fwl_Print(C3, M_RAM_MLC, "free super \r\n");
        return QQ_FreeAndTrace(gb_hLargeMemory, var, filename, line);
    }
#endif

    return Ram_Free(Ram_GetGlobalMem(), var, filename, line); //一些野指针释放时需要调试信息
}  

/* debug function : should merge into one */
T_U32 Fwl_GetTotalRamSize(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

    return info.szTotal;
}

T_U32 Fwl_RamUsedBlock(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

    return info.blkUsed;
}

T_U32 Fwl_GetUsedRamSize(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

    return info.szUsed;
}

T_U32 Fwl_RamGetBlockNum(T_VOID)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

    return info.szBlocks-info.blkUsed;
}

T_U32 Fwl_RamGetBlockLen()
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

    return info.align;
}

T_U32 Fwl_GetRemainRamSize(T_VOID)
{
     GLOBALMEMInfo info;
     Ram_GetRamInfo(Ram_GetGlobalMem(), &info);

     return info.szSpare;
}

T_VOID    Fwl_RamLeakMonitorHooktoSM(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR
    if(gb_enterSectionHook && gb_leaveSectionHook)
    {
        m_addPushFunc(GRam_EnterStateMachine, 0);
        m_addPopFunc(GRam_LeaveStateMachine, 1);

    }
#endif
}

T_VOID    Fwl_RamLeakMonitorPointBeg(T_VOID)
{
#ifdef DEBUG_TRACE_MEMORY_LEAK   
    Ram_EnableAutoLeakTrace(Ram_GetGlobalMem(), PrintOneMemoryLeakTraceInfo, 4000, AK_NULL, AK_NULL);
    RAM_ENTER_LEAKCHK_SECTION(Ram_GetGlobalMem());
#endif
}

T_VOID    Fwl_RamLeakMonitorPointEnd(T_VOID)
{
#ifdef DEBUG_TRACE_MEMORY_LEAK    
    RAM_LEAVE_LEAKCHK_SECTION(Ram_GetGlobalMem());
    Ram_DisableAutoLeakTrace(Ram_GetGlobalMem());
#endif
}

T_VOID  Fwl_RamBeyondMonitorGetbyTimer(T_U32 LLD)
{
#ifdef USE_MEMORY_MULTITHREAD 
    T_MEMORY_BEYOND_TRACE beyondInfo;
    if(AK_FALSE == Ram_CheckBeyond(Ram_GetGlobalMem(), &beyondInfo))
    {
       /*
            if(beyondInfo.reqSize + beyondInfo.loc + beyondInfo.cnt < beyondInfo.size)
            {
                printf("Warning: beyond your request malloc!");
            }
            */

        Fwl_Print(C3, M_RAM_MLC, "******************* Memory Exception: Beyond Monitor ******************* \r\n");
        Fwl_Print(C3, M_RAM_MLC, "LLD=%lu ptr=0x%x size=%ld reqsize=%ld, fileline=%lu, type=%d ",
             LLD,
             beyondInfo.addr,
             beyondInfo.size,
             beyondInfo.reqSize,                              
             beyondInfo.line,
             beyondInfo.type);

        Fwl_Print(C3, M_RAM_MLC, "filename=%s \r\n", beyondInfo.filename);
        while(1);   
     }
#endif
}

T_VOID Fwl_RamBeyondMonitor(T_U32 LLD)
{
	Fwl_RamBeyondMonitorGetbyTimer(LLD);
}

T_VOID  Fwl_RamWilderMonitorGetbyTimer(T_U32 LLD)
{
#ifdef USE_MEMORY_MULTITHREAD
    T_MEMORY_WILDER_TRACE wilderInfo;

    if(AK_FALSE == Ram_CheckWilder(Ram_GetGlobalMem(), &wilderInfo))
    {
        Fwl_Print(C3, M_RAM_MLC, "******************* Memory Exception: Wilder Monitor ******************* \r\n");
        Fwl_Print(C3, M_RAM_MLC, "LLD=%lu ptr=0x%p size=%ld reqsize=%ld, fileline=%lu, type=%d ",
                LLD,
                wilderInfo.addr,
                wilderInfo.size,
                wilderInfo.reqSize,                                
                wilderInfo.line,
                wilderInfo.type);

        Fwl_Print(C3, M_RAM_MLC, "filename=%s \r\n", wilderInfo.filename);
        while(1);
    }
#endif

}

T_VOID  Fwl_RamWilderMonitor(T_U32 LLD)
{
	Fwl_RamWilderMonitorGetbyTimer(LLD);
}

T_VOID  Ram_Lock(T_U32 dwLock)
{
#ifdef USE_MEMORY_MULTITHREAD
#ifdef OS_WIN32
    LPCRITICAL_SECTION m_lpCritSect = (LPCRITICAL_SECTION)dwLock;
        if(0 == dwLock)
        return;
    EnterCriticalSection(m_lpCritSect);
#endif

#ifdef OS_ANYKA
        if(0 == dwLock)
        return;
    AK_Obtain_Semaphore((T_hSemaphore)dwLock, AK_SUSPEND);
#endif
#else

        if(0 == dwLock)
        return;
    *(T_U32*)dwLock = *(T_U32*)dwLock+1;
    if(*(T_U32*)dwLock>1)
    {
        Fwl_Print(C1, M_RAM_MLC, "Error: Reentry in IRQ\r\n");
        while(1);
    }        
#endif
}

T_VOID  Ram_Unlock(T_U32 dwLock)
{

    
#ifdef USE_MEMORY_MULTITHREAD
#ifdef OS_WIN32
    LPCRITICAL_SECTION m_lpCritSect = (LPCRITICAL_SECTION)dwLock;
        if(0 == dwLock)
        return;
    LeaveCriticalSection(m_lpCritSect);
#endif

#ifdef OS_ANYKA
        if(0 == dwLock)
        return;
    AK_Release_Semaphore((T_hSemaphore)dwLock);
#endif
#else
        if(0 == dwLock)
        return;
    *(T_U32*)dwLock = *(T_U32*)dwLock-1;
#endif
}

T_VOID  Fwl_RamEnumerateEachSeg(T_VOID)
{
    Ram_EnumMemTraceInfo(Ram_GetGlobalMem(), PrintOneCommonTraceInfo, AK_NULL);
}


T_BOOL Fwl_CheckPtr(T_pVOID var)
{
	T_MEMORY_TRACE map;
	T_WILD_TYPE wildType;

	if(AK_FALSE == Ram_GetPtrInfo(Ram_GetGlobalMem(), var, &map, &wildType))
		{
			return AK_FALSE;
		}

	return AK_TRUE;
}

T_U32  Fwl_ReleaseMemory(T_VOID)
{
	//NOTE: maybe current malloc is invoked from Res_DynamicLoad
	if (Res_Nested())
	{
		return 0;
	}
	
	return Res_DynamicRelease(Ram_GetRamBufferSize());
}

T_U32 Fwl_GetLargestSize_Allocable(T_VOID)
{
	return Ram_GetLargestSize_Allocable(Ram_GetGlobalMem());
}

T_pVOID  malloc (T_U32 size)
{
	return Fwl_MallocAndTrace(size, __FILE__,__LINE__);
}

T_VOID free(T_pVOID mem)
{
	Fwl_FreeAndTrace(mem, __FILE__ , __LINE__);
}

T_pVOID  realloc (T_pVOID mem , T_U32 size)
{
	return Fwl_ReMallocAndTrace(mem,size, __FILE__,__LINE__);
}
