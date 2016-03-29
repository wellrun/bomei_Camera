/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKThread.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKTHREAD_H__
#define __AKTHREAD_H__

#include "akdefine.h"
#include "AKComponent.h"
#include "AKInterface.h"
#include "anyka_types.h"
#include "fwl_vme.h"
#include "Fwl_sysevent.h"
#include "eng_debug.h"
#include "AKApp_Def.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* ===================================>        Macro define begin         <==================================*/
#define THREAD_PROP_BASE        0x0000
#define AKAPP_PROP_OFFSET       0x0100
#define SUBTHREAD_PROP_OFFSET   0x0200


/*===========================================================================================================*/
//*************************************************************************************************************
#define AK_VTFUNC_CHECK(i,p,fn,err) \
    while (AK_NULL != p)  \
    { \
        if ((AK_NULL != (((i *)p)->pvt)) &&  \
           (AK_NULL != (((i *)p)->pvt->fn)))\
        { \
          break; \
        } \
        p = (i *)*((i **)p->pData);\
    } \
    if (AK_NULL == p) \
    { \
         return err; \
    }


/* ===================================>        Macro define end          <===================================*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*线程属性类别枚举定义*/
typedef enum
{
  THREAD_PROP_NONE = THREAD_PROP_BASE,
  THREAD_PROP_EVENT_MSK,               /*线程事件掩码*/
  THREAD_PROP_INTER_TASK,              /*线程内部任务*/
  THREAD_PROP_INTER_QUEUE,             /*线程内部队列*/ 
  THREAD_PROP_TASK_NAME,               /*线程当前任务名称*/
  THREAD_PROP_TASK_STATE,              /*线程当前任务状态*/
  THREAD_PROP_BACKGROUND,              /*线程类型信息*/
  THREAD_PROP_ATTACH_SUBTHREAD,        /*线程添加子线程*/
  THREAD_PROP_DETACH_SUBTHREAD         /*线程删除子线程*/
  
}T_eTHREAD_PROPERTY;

/*线程初始化信息定义*/
typedef struct 
{
  T_CHR *pcszName;            /*线程的名称*/	
  T_U8  byPriority;		      /*线程优先级*/		
  T_U32 ulTimeSlice;		  /*线程时间片*/
  T_U32 ulStackSize;          /*线程堆栈大小*/
  T_U32 ulQueueSize;		  /*线程队列所占空间大小，非消息个数*/
}T_THREAD_INITPARAM;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*==================================>      Define a call back  interface        <============================*/
typedef struct ICBThread ICBThread;

AK_INTERFACE(ICBThread)
{
    T_S32 (*Prepare)(ICBThread *pICBThread);
    T_S32 (*Handle)(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);
    T_S32 (*Free)(ICBThread *pICBThread);
};

struct ICBThread
{
    AK_DEFINEVT(ICBThread);
    T_VOID *pData;
};
/*###########################################################################################################*/
/**
===============================================================================================================
##                                       应用程序回调接口说明
##  应用程序回调接口是为了解决编写应用程序要继承所有IThread的接口函数而设计出来的一个简化接口。应用程序只要
##实现这三个接口函数就可以了。其中接口函数Prepare可以不用实现，直接在虚表里赋AK_NULL就可以了。在应用程序刚
##开始运行时会调用Prepare()函数，然后就进入了任务的消息循环等待消息，当有消息来时，就会调用Handle()将消息传
##给应用程序实现的Handle函数处理，当应用退出时，在释放之前会调用Free()，为应用程序提供一次释放资源的机会。
===============================================================================================================
===============================================================================================================
Function:     T_S32 ICBThread_Prepare(ICBThread *pICBThread);
Description:  应用程序运行前准备。
Parameters:
   pICBThread   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    应用程序刚运行时调用该接口函数。
===============================================================================================================
===============================================================================================================
Function:     T_S32 ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);
Description:  应用程序消息处理函数。
Parameters:
   pICBThread   [in] 指向该组件实例接口的指针。
   eEvent       [in] 事件类别。
   pEvtParam    [in] 事件参数。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   应用程序收到消息要处理时调用此函数。 
===============================================================================================================
===============================================================================================================
Function:     T_S32 ICBThread_Free(ICBThread *pICBThread);
Description:  应用程序释放函数。
Parameters:
   pICBThread   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    应用程序退出，实例被释放前调用此函数。
===============================================================================================================
  */
/*###########################################################################################################*/
/*========================================>    Define End     <==============================================*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct IThread IThread;

/*************************************************************************************************************/
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
/*************************************************************************************************************/
#define AK_INHERIT_ITHREAD(name) \
      AK_INHERIT_IUNKNOWN(name); \
      T_S32  (*Run)(name* p##name);  \
      T_S32  (*Suspend)(name* p##name); \
      T_S32  (*Terminate)(name* p##name); \
      T_S32  (*Exit)(name* p##name); \
      T_S32  (*Register)(name* p##name, ICBThread *pIListener); \
      T_S32  (*SetProperty)(name* p##name, T_U16 wPropID, T_VOID*  pPropData); \
      T_S32  (*GetProperty)(name* p##name, T_U16 wPropID, T_VOID*  pPropData)




AK_INTERFACE(IThread)
{
    AK_INHERIT_ITHREAD(IThread);
};

struct IThread
{
    AK_DEFINEVT(IThread);
    T_VOID *pData;
};

/*************************************************************************************************************/
/**
###############################################################################################################
##                                     组件描述
##   本组件为应用线程组件，是多实例组件，继承了IUnknown接口。主要负责任务的创建、销毁和消息队列的创建、消息
## 循环和销毁。本组件也提供了应用程序掩码的设置。
###############################################################################################################
===============================================================================================================
Function:      T_U32 IThread_AddRef(IThread *pIThread);
Description:   引用计数加1。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_U32 IThread_Release(IThread *pIThread);
Description:   引用计数减1，如果引用计数为0，则释放该组件实例。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_Run(IThread *pIThread);
Description:   运行指定线程，如果指定线程处于Terminate状态，则先会重置线程，然后重新运行。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_Suspend(IThread *pIThread);
Description:   挂起指定线程。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_Terminate(IThread *pIThread);
Description:   终止指定线程。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    不能在当前线程里终止自己。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_Exit(IThread *pIThread);
Description:   退出指定线程，会先终止该线程、删除该线程，然后释放该应用程序实例。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    不能在当前线程里退出自己。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_Register(IThread *pIThread, ICBThread *pIListener);
Description:   向指定线程注册一个应用程序回调接口。
Parameters:
   pIThread    [in] 指向该组件实例接口的指针。
   pIListener  [in] 指向应用程序回调接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   如果应用程序执行构造函数失败，则需要取消注册，即注册一个空指针AK_NULL。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_SetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData);
Description:   设置指定线程属性。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
   wPropID    [in] 属性类别ID。
   pPropData  [in] 指向属性内存块的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   T_S32 IThread_SetEvtMsk(IThread *pIThread, T_U32 ulEvtMsk);
   DES:设置指定线程的事件掩码。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IThread_GetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData);
Description:   获取指定线程属性。
Parameters:
   pIThread   [in] 指向该组件实例接口的指针。
   wPropID    [in] 属性类别ID。
   pPropData  [in/out] 指向属性内存块的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   T_BOOL IThread_IsBackground(IThread *pIThread);
   DES:判断指定线程所对应的应用是否是后台应用。
   T_U32 IThread_GetEvtMsk(IThread *pIThread);
   DES:获取指定线程的事件掩码。
   T_hTask IThread_GetTask(IThread *pIThread);
   DES:获取指定线程的内部任务。
   T_hQueue IThread_GetQueue(IThread *pIThread);
   DES:获取指定线程的内部队列。
   T_S32 IThread_GetState(IThread *pIThread);
   DES:获取指定线程的当前状态。
===============================================================================================================
*/
/*************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
T_S32 CThread_New(IThread **ppi, T_THREAD_INITPARAM *pParam);

//===========================================================================================================//
__inline T_U32 IThread_AddRef(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, AddRef, 0);
    return AK_GETVT(IThread, pIThread)->AddRef(pIThread); 
}

__inline T_U32 IThread_Release(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Release, 0);
    return AK_GETVT(IThread, pIThread)->Release(pIThread); 
}

__inline T_S32 IThread_Run(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Run, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->Run(pIThread); 
}

__inline T_S32 IThread_Suspend(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Suspend, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->Suspend(pIThread); 
}

__inline T_S32 IThread_Terminate(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Terminate, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->Terminate(pIThread);
}

__inline T_S32 IThread_Exit(IThread *pIThread)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Exit, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->Exit(pIThread);
}

__inline T_S32 IThread_Register(IThread *pIThread, ICBThread *pIListener)
{
    AK_VTFUNC_CHECK(IThread, pIThread, Register, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->Register(pIThread, pIListener);
}

__inline T_S32 IThread_SetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData)
{
    AK_VTFUNC_CHECK(IThread, pIThread, SetProperty, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->SetProperty(pIThread, wPropID, pPropData);
}

__inline T_S32 IThread_GetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData)
{
    AK_VTFUNC_CHECK(IThread, pIThread, GetProperty, AK_EUNSUPPORT);
    return AK_GETVT(IThread, pIThread)->GetProperty(pIThread, wPropID, pPropData);
}


__inline T_BOOL IThread_IsBackground(IThread *pIThread)
{
    T_BOOL bIsBackground = AK_FALSE;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_BACKGROUND,
                       (T_VOID*)&bIsBackground);
    
    return bIsBackground;
}

__inline T_S32 IThread_SetEvtMsk(IThread *pIThread, T_U32 ulEvtMsk)
{
    return IThread_SetProperty(pIThread, 
                             THREAD_PROP_EVENT_MSK, 
                             (T_VOID*)&ulEvtMsk);
}

__inline T_U32 IThread_GetEvtMsk(IThread *pIThread)
{
    T_U32 ulEvtMsk = 0;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_EVENT_MSK, 
                       (T_VOID*)&ulEvtMsk);
    
    return ulEvtMsk;
}

__inline T_hTask IThread_GetTask(IThread *pIThread)
{
    T_hTask hTask = 0;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_INTER_TASK, 
                       (T_VOID*)&hTask);
    
    return hTask;
}

__inline T_hQueue IThread_GetQueue(IThread *pIThread)
{
    T_hQueue hQueue = AK_INVALID_QUEUE;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_INTER_QUEUE, 
                       (T_VOID*)&hQueue);
    return hQueue;
}

__inline T_pCSTR IThread_GetName(IThread *pIThread)
{
    T_CHR *pszName = AK_NULL;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_TASK_NAME, 
                       (T_VOID*)&pszName);
    
    return (T_pCSTR)pszName;
}

__inline T_S32 IThread_GetState(IThread *pIThread)
{
    T_S32 lState = 0;
    
    IThread_GetProperty(pIThread, 
                       THREAD_PROP_TASK_STATE, 
                       (T_VOID*)&lState);
    return lState;
}

__inline T_S32 IThread_AttachSubThread(IThread *pIThread, IThread *pISubThread)
{
    return IThread_SetProperty(pIThread, 
                               THREAD_PROP_ATTACH_SUBTHREAD, 
                               (T_VOID*)pISubThread);
}

__inline T_S32 IThread_DetachSubThread(IThread *pIThread, IThread *pISubThread)
{
    return IThread_SetProperty(pIThread, 
                               THREAD_PROP_DETACH_SUBTHREAD, 
                               (T_VOID*)pISubThread);
}

#endif //__AKTHREAD_H__

