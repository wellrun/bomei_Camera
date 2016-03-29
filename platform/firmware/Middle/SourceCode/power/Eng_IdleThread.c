/**
 * @file Eng_IdleThread.c
 * @brief Idle Thread Implementation for Running A Idle Thread Read Someone Register to Save Power
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Wang_GuoTian, Xie_Wenzhong
 * @date 2011-11-3
 * @version 1.0
 */

#include "akdefine.h"
#include "Hal_timer.h"
#include "Fwl_osMalloc.h"
#include "AkSubThread.h"
#include "AkAppMgr.h"
#include "AKError.h"
#include "Eng_Debug.h"
#include "Eng_IdleThread.h"

#ifdef OS_ANYKA

#define IDLE_THREAD_NAME        "IdleThread"
#define IDLE_THREAD_PRIORITY    254
#define IDLE_THREAD_TIME_SLICE    1
#define IDLE_THREAD_STACK_SIZE  512

#define IDLE_SLICE              (100) //ms

#define CPU_HALT
//#undef CPU_HALT

typedef struct tagIdleThread{
    ISubThread    *pSubThread;
    T_BOOL        bRun;

}T_IDLE_THREAD;

T_IDLE_THREAD idleThread = {0};

static volatile T_U32 m_idle = 0;
static volatile T_BOOL m_idle_update = AK_FALSE;

extern T_U32 cpu_halt(T_VOID);
    
static T_VOID IdleThread_Entry(T_pVOID pData)
{
    T_U32 idle     = 0;
    
#if (defined CPU_HALT)
    T_U32 tick;    
    T_U32 sec     = 0;     
    
    sec = get_tick_count();
#endif

    while(idleThread.bRun)
    {
#if (defined CPU_HALT)

        AK_Drv_Protect();    
        tick = get_tick_count();
        
        cpu_halt();
        
        idle += get_tick_count()-tick;    
        AK_Drv_Unprotect();
        
        if (get_tick_count()- sec > IDLE_SLICE)    
        {    
            idle = idle * 100 / (get_tick_count()-sec);
        
            m_idle = idle;
            m_idle_update = AK_TRUE;
                
            sec = get_tick_count();
        
            idle = 0;    
        }
#else
        //read register to reduce power
        idle = *(volatile T_U32 *)0x08000000;
#endif
    }
    
    Fwl_Print(C2, M_POWER, "Exit Idle sub thread");
}

T_U32 Idle_GetCpuIdle(T_VOID)
{
    return m_idle;
}

T_U32 Idle_GetCpuUsage(T_VOID)
{     
    if (m_idle_update)
    {
        m_idle_update = AK_FALSE; 
        return (100 - m_idle);
    }
    else
    {
        return 100;
    }
}

T_BOOL IdleThread_Create(T_VOID)
{
    if (!idleThread.bRun)
    {
        T_SUBTHREAD_INITPARAM   param;

        Fwl_Print(C3, M_POWER, "Enter Create Idle sub thread");
        
        param.pcszName        = IDLE_THREAD_NAME;
        param.byPriority      = IDLE_THREAD_PRIORITY;
        param.ulTimeSlice     = IDLE_THREAD_TIME_SLICE;
        param.ulStackSize     = IDLE_THREAD_STACK_SIZE;
        param.wMainThreadCls  = AKAPP_CLSID_MMI ;
        param.pUserData       = &idleThread;
        param.fnEntry         = IdleThread_Entry;
        param.fnAbort         = AK_NULL; 

        idleThread.bRun = AK_TRUE;
        
        if(AK_SUCCESS != CSubThread_New(&idleThread.pSubThread, &param, AK_TRUE))
        {
           Fwl_Print(C2, M_POWER, "Create Idle sub thread fail");
           idleThread.bRun = AK_FALSE;

           return AK_FALSE;
        }
    }

    return AK_TRUE;
}

T_BOOL IdleThread_Destroy(T_VOID)
{
    if(!idleThread.bRun)
    {
        return AK_TRUE;
    }

    idleThread.bRun = AK_FALSE;    
    AK_Sleep(2);
    
    ISubThread_Terminate(idleThread.pSubThread);    
    idleThread.pSubThread = AK_NULL;

    return AK_TRUE;
}

T_BOOL IdleThread_IsCreate(T_VOID)
{
    if (!idleThread.bRun)
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}

#endif    // OS_ANYKA
