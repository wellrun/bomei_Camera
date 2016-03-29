/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKSubThread.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKSUBTHREAD_H__
#define __AKSUBTHREAD_H__

#include "AKThread.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
/* =============================>        Macro define begin         <===========================*/
#define SUBTHREAD_PROP_BASE        (THREAD_PROP_BASE + SUBTHREAD_PROP_OFFSET)

/*===============================================================================*/
#define SUBTHREAD_MAINTHREAD_PRIORITY    0xFF
/* =============================>        Macro define end          <============================*/

typedef T_VOID (*T_pfnSubThreadEntry)(T_VOID *pData);
typedef T_VOID (*T_pfnSubThreadAbort)(T_VOID *pData);
////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
    SUBTHREAD_PROP_NONE = SUBTHREAD_PROP_BASE,
    SUBTHREAD_PROP_MAIN_THREAD
}T_eSUBTHREAD_PROPERTY;

typedef struct 
{
  T_CHR*  pcszName;               //SubThread name(Max len is 8).
  T_U8    byPriority;			  //SubThread priority.	
  T_U32   ulTimeSlice;			  //1 means 10ms per time slice.
  T_U32   ulStackSize;            //Stack size. 
  T_VOID* pUserData;              //SubThread argv param.
  T_U16   wMainThreadCls;         //Main thread class id.
  T_pfnSubThreadEntry   fnEntry;  //SubThread execute function.
  T_pfnSubThreadAbort   fnAbort;  //SubThread abort function.
}T_SUBTHREAD_INITPARAM;


//////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ISubThread ISubThread;

/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_ITHREAD(name) \ 
        AK_INHERIT_IUNKNOWN(name); \
        //Add your function pointer list
        //of virtual table here...
If your extention doest inherit from IUnknown, you can use the Inherit 
macro AK_INHERIT_IYyy instead of AK_INHERIT_IUNKNOWN. For ezample:

#define AK_INHERIT_ICAT(name) \
        AK_INHERIT_IANIMAL(name); \
        ...    ...

*/
/*************************************************************************/
#define AK_INHERIT_ISUBTHREAD(name) \
      AK_INHERIT_ITHREAD(name); \
      T_S32 (*Restart)(name* p##name, T_VOID* pUserData); \
      T_S32 (*SetNotifyTimer)(name* p##name, T_U32 dwMilliseconds); \
      T_S32 (*StopNotifyTimer)(name* p##name)


AK_INTERFACE(ISubThread)
{
    AK_INHERIT_ISUBTHREAD(ISubThread);
};

struct ISubThread
{
    AK_DEFINEVT(ISubThread);
    T_VOID *pData;
};


////////////////////////////////////////////////////////////////////////////////////////////
T_S32 CSubThread_New(ISubThread **ppi, T_SUBTHREAD_INITPARAM *pParam, T_BOOL bIsAutoRun);
//=====================================================================//
__inline T_S32 ISubThread_Resume(ISubThread *pISubThread)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, Run, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->Run(pISubThread); 
}

__inline T_S32 ISubThread_Suspend(ISubThread *pISubThread)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, Suspend, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->Suspend(pISubThread); 
}

__inline T_S32 ISubThread_ReStart(ISubThread *pISubThread, T_VOID* pUserData)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, Restart, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->Restart(pISubThread, pUserData); 
}

__inline T_S32 ISubThread_Terminate(ISubThread *pISubThread)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, Terminate, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->Terminate(pISubThread);
}

__inline T_S32 ISubThread_Exit(ISubThread *pISubThread)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, Exit, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->Exit(pISubThread);
}

__inline T_S32 ISubThread_SetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, SetProperty, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->SetProperty(pISubThread, wPropID, pPropData);
}

__inline T_S32 ISubThread_GetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, GetProperty, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->GetProperty(pISubThread, wPropID, pPropData);
}

__inline T_S32 ISubThread_SetNotifyTimer(ISubThread *pISubThread, T_U32 dwMilliseconds)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, SetNotifyTimer, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->SetNotifyTimer(pISubThread, dwMilliseconds); 
}

__inline T_S32 ISubThread_StopNotifyTimer(ISubThread *pISubThread)
{
    AK_VTFUNC_CHECK(ISubThread, pISubThread, StopNotifyTimer, AK_EUNSUPPORT);
    return AK_GETVT(ISubThread, pISubThread)->StopNotifyTimer(pISubThread);
}

__inline T_hTask ISubThread_GetTask(ISubThread *pISubThread)
{
    T_hTask hTask = 0;
    
    ISubThread_GetProperty(pISubThread, 
                           THREAD_PROP_INTER_TASK, 
                           (T_VOID*)&hTask);
    
    return hTask;
}

__inline T_S32 ISubThread_GetState(ISubThread *pISubThread)
{
    T_S32 lState = 0;
    
    ISubThread_GetProperty(pISubThread, 
                           THREAD_PROP_TASK_STATE, 
                           (T_VOID*)&lState);
    return lState;
}


#endif //__AKSUBTHREAD_H__

