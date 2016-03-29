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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef OS_WIN32
#include <malloc.h>
	#define Dbg_Malloc(_size) malloc(_size)
	#define Dbg_Free(_var) (free(_var),AK_NULL)
#else
	#define Dbg_Malloc(_size) Fwl_Malloc(_size)
	#define Dbg_Free(_var) Fwl_Free(_var)
#endif

#define	MAX_RAMBUFFER_SIZE		(2048*1024)



static T_U8 gb_RAMBuffer[MAX_RAMBUFFER_SIZE];


static T_GLOBALMEMHandle		gb_hGlobalMemory = AK_NULL;

#define SUPER_LARGE_SIZE		(200*1024)
static T_GLOBALMEMHandle        gb_hLargeMemory = AK_NULL;
static T_pDATA					gb_lpLargebuf = AK_NULL;


extern void *memcpy(void * /*s1*/, const void * /*s2*/, size_t /*n*/);



/***********************static function************************/

/*
 * @ 判断文件名地址合法性
 */
static T_S8* GetTraceFileNameValidate(T_pVOID ptr)
{
#ifdef OS_WIN32
	const T_U32  st_min_ram_addr = 0x00001000;
	const T_U32  st_max_ram_addr = 0x2fffffff;
#else
	const T_U32  st_min_ram_addr = 0x30000000;
	const T_U32  st_max_ram_addr = 0x30800000;
#endif
	const T_S8 *st_trace_name[2] = {"Null Filename, maybe not input!", "Corrupt Filename, Maybe beyond is happend!"};

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
	if(gb_hGlobalMemory == hMemory)
	{
		if(map->addr==0 && map->size==0 && map->line==0)
		{
    printf("%s\r\n", map->filename);
			return;
		}

		if(0 == map->old)
		{
			T_S8 *strFn = GetTraceFileNameValidate(map->filename);

    printf("FileName:%s,\t\tLine:%d,\t\tAddr:%x,\t\tSize:%d\r\n",
					strFn,
					map->line,
					map->addr,
					map->size);
		}
	}
}

/*
 * @ 专用于打印内存泄漏枚举信息钩子回调函数
 */
static T_VOID PrintOneMemoryLeakTraceInfo(T_GLOBALMEMHandle hMemory, const T_MEMORY_TRACE *map, T_pVOID attach)
{
	if(gb_hGlobalMemory == hMemory)
	{
		if(map->addr==0 && map->size==0 && map->line==0)
		{
    printf("%s\r\n", map->filename);//Auto Leak check enter/leave stack machine
			return;
		}

		if(0 == map->old)
		{
			T_S8 *strFn = GetTraceFileNameValidate(map->filename);

    printf("******************* Memor Leak ******************* \r\n");
    printf("FileName:%s,\t\tLine:%d,\t\tAddr:%x,\t\tSize:%d\r\n",
					strFn,
					map->line,
					map->addr,
					map->size);
		}
	}
}

T_BOOL  GetMemoryBufLayout(T_U32 memsize,/* T_U8 align, */T_U8 *sos, T_U32 *szLarge/*, T_U32 *szSecondary*/)
{
	/*
		  SYS          big memory      pre reserve (if has)
		***************************************************
		*   1     *        2       *          3           *
		***************************************************
        16B     100K          SUPER_LARGE_SIZE

        (只有当需要特大整块内存(>3.5M)时，才有第三段)
	*/

	T_U32 sz;

	//第二段 ：中内存区域 (150K --- SUPER_LARGE_SIZE)
	*szLarge = 1024*1024;  //if only two regn

	//第三段 ：大内存缓冲区域 (//ocr+200W bmp , ....  >SUPER_LARGE_SIZE )
	#ifdef SUPPORT_BCR		
		*szLarge = 5000*1024;
	#endif
	#ifdef INSTALL_GAME_SFC
	*szLarge = 5*1024*1024; 
	#endif

    #ifdef SDRAM_8M
    *szLarge = 1024*1024; 
    #endif
	//第一段 ：小内存区域 (<150K)
		//目前的MMI系统中小内存(<150K) 不超过1.8M
	sz = memsize;//-(*szLarge);
	*sos = (T_U8)(((/*sz-*/2*1024*1024)*100)/sz);

	//#ifdef SUPPORT_BCR
	//*szSecondary = (100*1024)/align; 
	//#else

	return AK_TRUE;
}

T_VOID Fwl_MallocInit(T_VOID)
{
	T_U32   largesz;
	T_U8    sos;
		
	GetMemoryBufLayout(sizeof(gb_RAMBuffer), &sos, &largesz);

#ifndef AUTO_MEMORY_BEYOND_MONITOR
	//gb_hGlobalMemory = Ram_Initial(gb_RAMBuffer, sizeof(gb_RAMBuffer));
	gb_hGlobalMemory = Ram_InitialEx(gb_RAMBuffer, MAX_RAMBUFFER_SIZE, 0, 0, 0x67, 0x93, 16, 6400, sos, 4, AK_NULL);		
#else	
	//for system debuger : can check memory beyond (检测内存越界)
	gb_hGlobalMemory = Ram_InitialEx(gb_RAMBuffer, MAX_RAMBUFFER_SIZE, 16, 16, 0x67, 0x93, 16, 6400, sos, 4, Fwl_Printf);
    printf("Fwl_MallocInit : initialize for memory beyond tracer \r\n");
#endif
	
	if(AK_NULL == gb_hGlobalMemory)
	{
    printf("Fwl_MallocInit : global memory allocator initialize error \r\n");
		return;
	}

	gb_lpLargebuf = Ram_Alloc(gb_hGlobalMemory, largesz, __FILE__, __LINE__);
	gb_hLargeMemory = Ram_InitialEx(gb_lpLargebuf, largesz, 0, 0, 0, 0, 16, 1, 100, 4, AK_NULL);
	if(AK_NULL == gb_hLargeMemory)
	{
    printf("Fwl_MallocInit : large memory allocator initialize error \r\n");
		//return;
	}

#ifdef AUTO_MEMORY_LEAK_MONITOR
	//Enable memory leak tracer (检测内存泄漏)
	{
		//set hook
		Ram_EnableAutoLeakTrace(gb_hGlobalMemory, PrintOneMemoryLeakTraceInfo, &gb_enterSectionHook, &gb_leaveSectionHook);
    printf("Fwl_MallocInit : enable for memory leak tracer \r\n");

		//eg 1
		//RAM_ENTER_LEAKCHK_SECTION(gb_hGlobalMemory);
		//.... // want to be checked codes
		//RAM_LEAVE_LEAKCHK_SECTION(gb_hGlobalMemory);

		//eg 2
		Fwl_RamLeakMonitorHooktoSM();

		//Ram_DisableAutoLeakTrace(gb_hGlobalMemory);
	}
#endif

}

T_pVOID Fwl_MallocAndTrace(T_U32 size, T_pSTR filename, T_U32 line)
{
	T_pVOID ptr;
	if((size > SUPER_LARGE_SIZE) && gb_hLargeMemory)
	{
    printf("alloc super %d \r\n", size);
		ptr = QQ_MallocAndTrace(gb_hLargeMemory, size, filename, line);
		if(ptr != AK_NULL)
			return ptr;
	}


	ptr = Ram_Alloc(gb_hGlobalMemory, size, filename, line);

#ifdef AUTO_MEMORY_LEAK_MONITOR
	if(AK_NULL == ptr)
    printf("alloctrace failure : size = %d \r\n", size);
	if(size >= 100*1024)
    printf("alloc largebuf : ptr=0x%x, size=%d \r\n", ptr, size);
#endif

	return ptr;
}

T_pVOID	Fwl_ReMallocAndTrace(T_pVOID var, T_U32 size, T_pSTR filename, T_U32 line)
{
	T_pVOID ptr;
	if((size > SUPER_LARGE_SIZE) && gb_hLargeMemory)
	{
    printf("alloc super %d \r\n", size);
		ptr = QQ_ReMallocAndTrace(gb_hLargeMemory, var, size, filename, line);
		if(ptr != AK_NULL)
			return ptr;
	}


	ptr = Ram_Realloc(gb_hGlobalMemory, var, size, filename, line);

#ifdef AUTO_MEMORY_LEAK_MONITOR
	if(AK_NULL == ptr)
    printf("realloctrace failure : size = %d \r\n", size);
	if(size >= 100*1024)
    printf("alloc largebuf : ptr=0x%x, size=%d \r\n", ptr, size);
#endif

	return ptr;
}

T_pVOID Fwl_FreeAndTrace(T_pVOID var, T_pSTR filename, T_U32 line) 
{
	if(gb_hLargeMemory && ((T_pDATA)var>gb_lpLargebuf))
	{
    printf("free super \r\n");
		return QQ_FreeAndTrace(gb_hLargeMemory, var, filename, line);
	}

#ifdef AUTO_MEMORY_LEAK_MONITOR
	{
		T_MEMORY_TRACE map;
		if((AK_TRUE == Ram_GetPtrInfo(gb_hGlobalMemory, var, &map))
			&& (map.size >= 100*1024))
    printf("free largebuf : ptr=0x%x, size=%d \r\n", var, map.size);
	}
#endif

	return Ram_Free(gb_hGlobalMemory, var, filename, line); //一些野指针释放时需要调试信息
}  

T_pVOID Fwl_Malloc(T_U32 size)
{
	T_pVOID ptr;
	if((size > SUPER_LARGE_SIZE) && gb_hLargeMemory)
	{
    printf("alloc super %d \r\n", size);
		ptr = QQ_Malloc(gb_hLargeMemory, size);
		if(ptr != AK_NULL)
			return ptr;
	}

	ptr = Ram_Alloc(gb_hGlobalMemory, size, AK_NULL, 0);

#ifdef AUTO_MEMORY_LEAK_MONITOR
	if(AK_NULL == ptr)
    printf("alloc failure : size = %d \r\n", size);
	if(size >= 100*1024)
    printf("alloc largebuf : ptr=0x%x, size=%d \r\n", ptr, size);
#endif

	return ptr;
}

T_pVOID Fwl_ReMalloc(T_pVOID var, T_U32 size)
{
	T_pVOID ptr;
	T_WILD_TYPE type;

	Ram_CheckPtr(gb_hLargeMemory, var, &type);
	
	if (size > SUPER_LARGE_SIZE)
	{
		if (WILD_OK == type)
		{	
			return ptr = QQ_ReMalloc(gb_hLargeMemory, var, size);
		}
		else
		{
			ptr = Fwl_Malloc(size);
			if(ptr != AK_NULL)
				memcpy(ptr, var, size);
			Ram_Free(gb_hGlobalMemory, var, __FILE__, __LINE__);
			return ptr;
		}			
	}
	else if (WILD_OK == type)
	{
		ptr = Fwl_Malloc(size);
		if(ptr != AK_NULL)
			memcpy(ptr, var, size);
		Ram_Free(gb_hLargeMemory, var, __FILE__, __LINE__);
		return ptr;
	}

	ptr = Ram_Realloc(gb_hGlobalMemory, var, size, AK_NULL, 0);

#ifdef AUTO_MEMORY_LEAK_MONITOR
	if(AK_NULL == ptr)
    printf("realloc failure : size = %d \r\n", size);
	if(size >= 100*1024)
    printf("re alloc largebuf : ptr=0x%x, size=%d \r\n", ptr, size);
#endif

	return ptr;
} 

T_pVOID Fwl_Free(T_pVOID var)
{
	if(gb_hLargeMemory && ((T_pDATA)var>gb_lpLargebuf))
	{
    printf("free super \r\n");
		return QQ_Free(gb_hLargeMemory, var);
	}


#ifdef AUTO_MEMORY_LEAK_MONITOR
	{
		T_MEMORY_TRACE map;
		if((AK_TRUE == Ram_GetPtrInfo(gb_hGlobalMemory, var, &map))
			&& (map.size >= 100*1024))
    printf("free largebuf : ptr=0x%x, size=%d \r\n", var, map.size);
	}
#endif

	return Ram_Free(gb_hGlobalMemory, var, AK_NULL, 0);
}

T_BOOL  Fwl_CheckPtr(T_pVOID var)
{
	T_WILD_TYPE type;

	if(gb_hLargeMemory && ((T_pDATA)var>gb_lpLargebuf))
	{
		return QQ_CheckPtr(gb_hLargeMemory,var);
    }

	if(AK_FALSE == Ram_CheckPtr(gb_hGlobalMemory, var, &type))
	{
    printf("Fwl_CheckPtr : var is wrong, type = %d \r\n", type);
		return AK_FALSE;
	}

	return AK_TRUE;
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


/* ********************* Memory Debuger *********************************************** */


T_VOID	Fwl_RamBeyondMonitorGetbyTimer(T_U16 LID)
{
#ifdef AUTO_MEMORY_BEYOND_MONITOR

	T_MEMORY_BEYOND_TRACE beyondInfo;

		Fwl_Printf("Memory alloc check %d@@@\r\n", LID);
	if(AK_FALSE == Ram_CheckBeyond(gb_hGlobalMemory, &beyondInfo))
	{
    printf("******************* Memor Exception ******************* \r\n");
    printf("LID=%d ptr=0x%x size=%d filename=%s, fileline=%d, type=%d\r\n",
			       LID,
				beyondInfo.addr,
				beyondInfo.size,
				beyondInfo.filename,
				beyondInfo.line,
				beyondInfo.type);
	}

#endif

}

static T_VOID GRam_EnterStateMachine(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR
	(*gb_enterSectionHook)(gb_hGlobalMemory);
#endif
}

static T_VOID GRam_LeaveStateMachine(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR
	(*gb_leaveSectionHook)(gb_hGlobalMemory);
#endif
}


T_VOID	Fwl_RamLeakMonitorHooktoSM(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR
	if(gb_enterSectionHook && gb_leaveSectionHook)
	{
		m_addPushFunc(GRam_EnterStateMachine, 0);
		m_addPopFunc(GRam_LeaveStateMachine, 1);

	}
#endif

}

T_VOID	Fwl_RamLeakMonitorPointBeg(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR	
	RAM_ENTER_LEAKCHK_SECTION(gb_hGlobalMemory);
#endif
}

T_VOID	Fwl_RamLeakMonitorPointEnd(T_VOID)
{
#ifdef AUTO_MEMORY_LEAK_MONITOR	
	RAM_LEAVE_LEAKCHK_SECTION(gb_hGlobalMemory);
#endif
}

T_VOID  Fwl_RamEnumerateEachSeg(T_VOID)
{
	Ram_EnumMemTraceInfo(gb_hGlobalMemory, PrintOneCommonTraceInfo, AK_NULL);
}

/* **************************** local share ************************************ */
T_pVOID QQ_CreateAllocator(T_pVOID mempool, T_U32 size)
{
	T_GLOBALMEMHandle hAllocator = AK_NULL;

#ifndef AUTO_MEMORY_BEYOND_MONITOR
	//gb_hGlobalMemory = Ram_Initial(gb_RAMBuffer, sizeof(gb_RAMBuffer));
	hAllocator = Ram_InitialEx(mempool, size, 0, 0, 0x67, 0x93, 16, 6400, 48, 4, AK_NULL);		
#else	
	//for system debuger : can check memory beyond (检测内存越界)
	hAllocator = Ram_InitialEx(mempool, size, 16, 16, 0x67, 0x93, 16, 6400, 48, 4, Fwl_Printf);
    printf("QQ_CreateAllocator : initialize for memory beyond tracer \r\n");
#endif
	
	if(AK_NULL == hAllocator)
	{
    printf("QQ_CreateAllocator : local memory allocator initialize error \r\n");
    printf("QQ_CreateAllocator : use global memory allocator \r\n");
		return gb_hGlobalMemory;
	}

    printf("QQ_CreateAllocator : local memory allocator initialize ok \r\n");
	return hAllocator;

}

T_pVOID QQ_DestroyAllocator(T_pVOID hAllocator)
{
	if(hAllocator == gb_hGlobalMemory)
		return AK_NULL;

    printf("QQ_DestroyAllocator : destroy local memory allocator \r\n");
	return Ram_Exit(hAllocator);
}

T_pVOID QQ_MallocAndTrace(T_pVOID hAllocator, T_U32 size, T_pSTR filename, T_U32 line)
{
	return (T_pVOID)Ram_Alloc(hAllocator, size, filename, line);
}

T_pVOID	QQ_ReMallocAndTrace(T_pVOID hAllocator, T_pVOID var, T_U32 size, T_pSTR filename, T_U32 line)
{
	return (T_pVOID)Ram_Realloc(hAllocator, var, size, filename, line);
}

T_pVOID QQ_FreeAndTrace(T_pVOID hAllocator, T_pVOID var, T_pSTR filename, T_U32 line)
{
	return Ram_Free(hAllocator, var, filename, line); //一些野指针释放时需要调试信息
}

T_pVOID QQ_Malloc(T_pVOID hAllocator, T_U32 size)
{
	return (T_pVOID)Ram_Alloc(hAllocator, size, AK_NULL, 0);
}

T_pVOID QQ_ReMalloc(T_pVOID hAllocator, T_pVOID var, T_U32 size)
{
	return (T_pVOID)Ram_Realloc(hAllocator, var, size, AK_NULL, 0);
}

T_pVOID QQ_Free(T_pVOID hAllocator, T_pVOID var)
{
	return Ram_Free(hAllocator, var, AK_NULL, 0);
}

T_BOOL  QQ_CheckPtr(T_pVOID hAllocator, T_pVOID var)
{
	T_WILD_TYPE type;

	if(AK_FALSE == Ram_CheckPtr(hAllocator, var, &type))
	{
    printf("QQ_CheckPtr : var is wrong, type = %d \r\n", type);
		return AK_FALSE;
	}

	return AK_TRUE;
}

T_VOID	QQ_RamBeyondMonitorGetbyTimer(T_pVOID hAllocator)
{
#ifdef AUTO_MEMORY_BEYOND_MONITOR

	T_MEMORY_BEYOND_TRACE beyondInfo;
	if(AK_FALSE == Ram_CheckBeyond(hAllocator, &beyondInfo))
	{
    printf("******************* Memor Exception ******************* \r\n");
    printf("ptr=0x%x size=%d filename=%s, fileline=%d, type=%d\r\n",
				beyondInfo.addr,
				beyondInfo.size,
				beyondInfo.filename,
				beyondInfo.line,
				beyondInfo.type);
	}

#endif

}
