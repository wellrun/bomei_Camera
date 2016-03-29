/**
 * @FILENAME: raminit.c
 * @BRIEF config gpio
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @VERSION 1.0
 * @REF
 */
#include <stdio.h>
#include <stdarg.h>

#include "akdefine.h"
#include "mem_api.h"
#include "hal_print.h"
#include "akos_api.h"
#include "anyka_bsp.h"
#include "raminit.h"

#ifdef OS_WIN32
#include <malloc.h>
#include <windows.h>
#endif

#ifndef ENABLE_MEMORY_DEBUG
    #define ENABLE_MEMORY_DEBUG    //内存越界自动监测器
    #ifndef OS_WIN32
    #define DEBUG_TRACE_MEMORY_LEAK
#endif
    #define USE_MEMORY_MULTITHREAD
#endif
extern T_S32 Fwl_Print(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...);
extern T_S32 Fwl_VPrint(T_U8 level, T_pCSTR mStr, T_pCSTR s, va_list args);

#ifdef OS_ANYKA
extern T_U32 Image$$ER_ZI$$ZI$$Limit;
#define MALLOC_MEM_ENTRY ((T_U32)&Image$$ER_ZI$$ZI$$Limit + 4)
#define MALLOC_MEM_SIZE  (SVC_MODE_STACK - 8*1024 - (T_U32)MALLOC_MEM_ENTRY)
#else
#define MAX_RAMBUFFER_SIZE	((SDRAM_MODE << 20))
static T_U8 gb_RAMBuffer[MAX_RAMBUFFER_SIZE];
#define MALLOC_MEM_ENTRY  gb_RAMBuffer
#define MALLOC_MEM_SIZE   MAX_RAMBUFFER_SIZE
#endif



static T_GLOBALMEMHandle        gb_hGlobalMemory = AK_NULL;


T_S32 Ram_getaddr(T_U32 *minaddr, T_U32* maxaddr)
{
	*minaddr = RAM_BASE_ADDR;
	*maxaddr = RAM_BASE_ADDR + SDRAM_SIZE;
	return AK_TRUE;
}

T_U32 Ram_getMMUaddr(T_VOID)
{
	return _MMUTT_STARTADDRESS;
}


T_GLOBALMEMHandle Ram_GetGlobalMem(T_VOID)
{
	return gb_hGlobalMemory;
}

T_U32  Ram_GetRamBufferSize(T_VOID)
{
	return MALLOC_MEM_SIZE;
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
    *szLarge = 1024*1024; //if only two regn

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
    *sos = (T_U8)(((/*sz-*/3*1024*1024)*100)/sz);

    //#ifdef SUPPORT_BCR
    //*szSecondary = (100*1024)/align; 
    //#else

    return AK_TRUE;
}

static T_S32 Ram_Print(const T_U8 *s, ...)
{
	va_list 	args;
	T_S32		len;
	
	va_start(args, s);
	len = Fwl_VPrint(C3, M_RAM, s, args);
	va_end(args); 
	
	return len;
}


T_VOID Ram_MallocInit(T_VOID)
{
    T_U32   largesz;
    T_U8    sos;
//    T_hSemaphore     akos_mutex = 0;
        
    GetMemoryBufLayout(MALLOC_MEM_SIZE, &sos, &largesz);

#ifndef ENABLE_MEMORY_DEBUG    
    gb_hGlobalMemory = Ram_Initial(MALLOC_MEM_ENTRY, MALLOC_MEM_SIZE);    
#else

	/* 根据系统应用要求, 参数需要细微配置
	*  @para: lenfree, sos 成功率与速度的权衡参数, 这两个置越大, 成功率越高, 速度越低
	*/
    gb_hGlobalMemory = Ram_InitialEx(MALLOC_MEM_ENTRY, MALLOC_MEM_SIZE, 
                        0, 16/*0*/, 16, 0xFC, 0xCF, //内存越界检测参数指定
                       0x10/*0*/, 0xCC,         //野操作检测参数
                       16, 11200, 85, 2,   //高级分配器优化参数                   
                       (Ram_PrintfTraceFun)Ram_Print);

#endif
    
    if(AK_NULL == gb_hGlobalMemory)
    {
		Fwl_Print(C2, M_RAM, "Ram_MallocInit : global memory allocator initialize error \r\n");
        return;
    }
    
    

#ifdef AUTO_MEMORY_LEAK_MONITOR
    //Enable memory leak tracer (检测内存泄漏)
    {
        //set hook
        //Ram_EnableAutoLeakTrace(gb_hGlobalMemory, PrintOneMemoryLeakTraceInfo, 2000, &gb_enterSectionHook, &gb_leaveSectionHook);
        printf("Ram_MallocInit : enable for memory leak tracer \r\n");

        //eg 1
        //RAM_ENTER_LEAKCHK_SECTION(gb_hGlobalMemory);
        //.... // want to be checked codes
        //RAM_LEAVE_LEAKCHK_SECTION(gb_hGlobalMemory);

        //eg 2
        //Fwl_RamLeakMonitorHooktoSM();

        //Ram_DisableAutoLeakTrace(gb_hGlobalMemory);
    }
#endif


#ifdef USE_TWO_BLOCK_MEMORY_BUFFER
    gb_lpLargebuf = Ram_Alloc(gb_hGlobalMemory, largesz, __FILE__, __LINE__);
    gb_hLargeMemory = Ram_InitialEx(gb_lpLargebuf, largesz, 0, 0, 0, 0, 16, 1, 100, 4, AK_NULL);
    if(AK_NULL == gb_hLargeMemory)
    {
		Fwl_Print(C2, M_RAM, "Ram_MallocInit : large memory allocator initialize error \r\n");
        //return;
    }
#endif

    /*ram semaphore protection*/
//    akos_mutex = AK_Create_Semaphore(1, AK_PRIORITY);
//    Ram_SetLock(gb_hGlobalMemory, akos_mutex);

    #ifdef OS_WIN32
    {
        LPCRITICAL_SECTION lpc = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
        InitializeCriticalSection(lpc);
        Ram_SetLock(gb_hGlobalMemory, (unsigned long)lpc);
    }
    #endif


}

T_VOID Ram_MallocSupportMultithread(T_VOID)
{
    T_hSemaphore     akos_mutex = 0;

    akos_mutex = AK_Create_Semaphore(1, AK_PRIORITY);
    Ram_SetLock(gb_hGlobalMemory, akos_mutex);
	
}



T_BOOL Ram_AssertCheckPointer(T_pCVOID ptr)
{
	if ((((T_U32)ptr >= MIN_RAM_ADDR) && ((T_U32)ptr <= MAX_RAM_ADDR)) ||
		(((T_U32)ptr >= MIN_ROM_ADDR) && ((T_U32)ptr <= MAN_ROM_ADDR)))
		return AK_TRUE;
	else
	{
		Fwl_Print(C1, M_RAM, "Ram_AssertCheckPointer failed, ptr is: 0x%x ", (T_U32)ptr);
		return AK_FALSE;
	}
}



T_VOID Ram_AssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line)
{
	//T_U16		len = 0;
	//T_hMSGBOX	assertMsgbox = AK_NULL;

	Fwl_Print(C1, M_RAM, "Ram_AssertDispMsg %s, %s, %d\n", message, filename, line);

#ifdef ASSERT_DEATH_LOOP
	while(1);
#endif

#if defined(ASSERT_REBOOT) && defined(SUPPORT_PANNIC_REBOOT)
	System_Start(ASSERT_TYPE);
#endif
	return;		/* assert occur */

}

