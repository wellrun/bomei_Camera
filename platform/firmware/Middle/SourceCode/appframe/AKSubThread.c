/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKSubThread.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKSubThread.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppMgr.h"
#include "Akos_api.h"

#ifdef OS_WIN32
#include "string.h"
#endif

//#################################################################################
#define SUBTHREAD_DO_NOTHING
/*================================================================================*/
typedef struct tagCSubThread
{
    IThread     *m_pIBase;          //Base class, must contain this variable.
    T_U32        m_Ref;
    ISubThread   m_myISubThread;
    T_hTask	     m_hTask;
    T_pVOID	     m_pStackAddr;
    T_hTimer     m_hTimer;
    T_BOOL       m_bIsProcess;      //Whether SubThread routine is during process.
    T_U16        m_wMainThreadCls;
    T_VOID      *m_pUserData;       //Point to the user data which will be used as a param by m_fnEntry.
    T_pfnSubThreadEntry   m_fnEntry;
    T_pfnSubThreadAbort   m_fnAbort;
}CSubThread;

/*===========================================================================================*/
/*===========================================================================================*/

//============================================================================================//
//              <---- Function Declaration ---->
//============================================================================================//
static T_VOID CSubThread_Task(T_U32 argc, T_VOID *argv);
static T_U32  CSubThread_AddRef(ISubThread *pISubThread);
static T_U32  CSubThread_Release(ISubThread *pISubThread);
static T_S32  CSubThread_Resume(ISubThread *pISubThread);
static T_S32  CSubThread_Suspend(ISubThread *pISubThread);
static T_S32  CSubThread_Terminate(ISubThread *pISubThread);
static T_S32  CSubThread_DeleteTask(ISubThread *pISubThread);
static T_S32  CSubThread_Exit(ISubThread *pISubThread);
static T_S32  CSubThread_SetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData);
static T_S32  CSubThread_GetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData);
static T_S32  CSubThread_ReStart(ISubThread *pISubThread, T_VOID* pUserData);
static T_S32  CSubThread_SetNotifyTimer(ISubThread *pISubThread, T_U32 dwMilliseconds);
static T_S32  CSubThread_StopNotifyTimer(ISubThread *pISubThread);
//////////////////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(ISubThread) g_ISubThreadFuncs =
{
    CSubThread_AddRef,
    CSubThread_Release,
    CSubThread_Resume,
    CSubThread_Suspend,
    CSubThread_Terminate,
    CSubThread_Exit,
    AK_NULL,
    CSubThread_SetProperty,
    CSubThread_GetProperty,
    CSubThread_ReStart,
    CSubThread_SetNotifyTimer,
    CSubThread_StopNotifyTimer
};

static T_S32 CSubThread_Constructor(CSubThread *pMe, T_SUBTHREAD_INITPARAM *pParam)
{
	if (AK_NULL == pParam)
	{
	    return AK_EBADPARAM;
	}

    pMe->m_pIBase         = AK_NULL;
    
    pMe->m_hTimer         = AK_INVALID_TIMER;
    pMe->m_hTask          = AK_INVALID_TASK;
    pMe->m_pStackAddr     = AK_NULL;
    pMe->m_bIsProcess     = AK_FALSE; 
    pMe->m_pUserData      = pParam->pUserData;
    pMe->m_wMainThreadCls = pParam->wMainThreadCls;
    pMe->m_fnEntry        = pParam->fnEntry;
    pMe->m_fnAbort        = pParam->fnAbort;

	pMe->m_pStackAddr = Fwl_Malloc(pParam->ulStackSize);
	if (pMe->m_pStackAddr == AK_NULL)
	{
		return AK_ENOMEMORY;
	}
	
	memset(pMe->m_pStackAddr, 0, pParam->ulStackSize);
    
	pMe->m_hTask  = AK_Create_Task((T_VOID*)CSubThread_Task,pParam->pcszName,
                                   1 ,pMe, pMe->m_pStackAddr,
                                   pParam->ulStackSize,
                                   pParam->byPriority,
                                   pParam->ulTimeSlice,
                                   AK_PREEMPT,AK_NO_START);

    if (AK_IS_INVALIDHANDLE(pMe->m_hTask))
    {
		return AK_EFAILED;
    }
    
    return AK_SUCCESS;
}

static T_S32 CSubThread_Destructor(CSubThread *pMe)
{
    //Add your code here...
    AK_RELEASEIF(pMe->m_pIBase);
    
    CSubThread_DeleteTask(&pMe->m_myISubThread);
    
    return AK_SUCCESS;
}

static T_U32 CSubThread_AddRef(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;

    return ++pMe->m_Ref;
}

static T_U32 CSubThread_Release(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CSubThread_Destructor(pMe);
        Fwl_Free(pMe);
        
		return 0;
    }

    return pMe->m_Ref;
}



static T_VOID CSubThread_Task(T_U32 argc, T_VOID *argv)
{
#ifdef OS_ANYKA
    CSubThread *pMe = (CSubThread *)argv;
#else
    CSubThread *pMe = (CSubThread *)argc;
#endif
    T_SYS_MAILBOX mailbox;

#ifndef WIN32
    Fwl_Print(C3, M_AKFRAME, "################CSubThread CSubThread_Task:");
#endif

    pMe->m_bIsProcess = AK_TRUE;

    if (AK_NULL != pMe->m_fnEntry)
    {
        pMe->m_fnEntry(pMe->m_pUserData);
    }

    //Stop notify timer....
    if (AK_IS_VALIDHANDLE(pMe->m_hTimer))
    {
        CSubThread_StopNotifyTimer(&pMe->m_myISubThread);
    }
    
    if (pMe->m_bIsProcess)
    {        
        //Send subthread finished event to main thread...    
        mailbox.event = SYS_EVT_SUBTHREAD_FINISH;
        mailbox.param.lParam = (T_U32)&pMe->m_myISubThread;	
        IAppMgr_PostEvent(AK_GetAppMgr(), pMe->m_wMainThreadCls, &mailbox);

        pMe->m_bIsProcess = AK_FALSE;
    }
}


static T_S32 CSubThread_Resume(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
#ifndef WIN32     
    Fwl_Print(C3, M_AKFRAME, "CSubThread CSubThread_Resume .");
#endif
    if (ISubThread_GetState(pISubThread) == AK_TERMINATED)
    {
        AK_Reset_Task(pMe->m_hTask, 1, pMe);
    }
    
    return AK_Resume_Task(pMe->m_hTask);
}

static T_S32  CSubThread_ReStart(ISubThread *pISubThread, T_VOID* pUserData)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    IThread *pIMainThread = AK_NULL;

#ifndef WIN32     
    Fwl_Print(C3, M_AKFRAME, "CSubThread CSubThread_ReStart .");
#endif
    pMe->m_pUserData = pUserData;
    
    /*如果重新执行，则将该子线程再次添加到主线程*/
    pIMainThread = IAppMgr_GetApp(AK_GetAppMgr(), pMe->m_wMainThreadCls);
    if (AK_NULL != pIMainThread)
    {
        IThread_AttachSubThread(pIMainThread, (IThread*)pISubThread);
    }
    
    AK_Reset_Task(pMe->m_hTask, 1, pMe);
    
    return AK_Resume_Task(pMe->m_hTask);    
}

static T_S32 CSubThread_Suspend(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    
    return AK_Suspend_Task(pMe->m_hTask);
}


static T_S32 CSubThread_Terminate(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    T_S32 lRet = AK_SUCCESS;
    IThread* pIMainThread = AK_NULL;

    /*删除设置的Timer*/
    lRet = CSubThread_StopNotifyTimer(pISubThread);
    
    /*先终止子线程运行*/
    lRet += AK_Terminate_Task(pMe->m_hTask);

    /*再执行Abort函数，处理善后事情*/
    if (AK_TRUE == pMe->m_bIsProcess)
    {
        if (AK_NULL != pMe->m_fnAbort)
        {
            pMe->m_fnAbort(pMe->m_pUserData);
        }
        pMe->m_bIsProcess = AK_FALSE;
    }

    /*将该子线程从主线程中移除.*/
    pIMainThread = IAppMgr_GetApp(AK_GetAppMgr(), pMe->m_wMainThreadCls);
    if (AK_NULL != pIMainThread)
    {
        IThread_DetachSubThread(pIMainThread, (IThread*)pISubThread);
    }
    
    return lRet;
}

static T_S32 CSubThread_DeleteTask(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    
    /*终止和删除子线程*/
    if (AK_IS_VALIDHANDLE(pMe->m_hTask))
    {
        AK_Terminate_Task(pMe->m_hTask);
        AK_Delete_Task(pMe->m_hTask);
        pMe->m_hTask = AK_INVALID_TASK;
    }
    
    /*释放子线程堆栈空间*/
    if (AK_NULL != pMe->m_pStackAddr)
    {
        pMe->m_pStackAddr = Fwl_Free(pMe->m_pStackAddr);
    }
    
    return AK_SUCCESS;
}


static T_S32 CSubThread_Exit(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    IThread* pIMainThread = AK_NULL;
    
    /*将该子线程从主线程中移除.*/
    pIMainThread = IAppMgr_GetApp(AK_GetAppMgr(), pMe->m_wMainThreadCls);
    if (AK_NULL != pIMainThread)
    {
        IThread_DetachSubThread(pIMainThread, (IThread*)pISubThread);
    }
    
    /*释放该子线程，直到销毁.*/
    while (CSubThread_Release(pISubThread) > 0) 
    {
        SUBTHREAD_DO_NOTHING;
    }
    
    return AK_SUCCESS;
}

static T_S32 CSubThread_SetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData)
{
    //CSubThread *pMe = (CSubThread *)pISubThread->pData;
    T_S32 lRet = AK_SUCCESS;

    if ((AK_NULL == pISubThread) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }

    switch(wPropID)
    {
    default:
        {
            lRet = AK_EUNSUPPORT;
        }
        break;
    }

    return lRet;
}


static T_S32 CSubThread_GetProperty(ISubThread *pISubThread, T_U16 wPropID, T_VOID*  pPropData)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if ((AK_NULL == pISubThread) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }
  
    switch(wPropID)
    {
    case SUBTHREAD_PROP_MAIN_THREAD:
        {
            *((T_U16*)pPropData) = pMe->m_wMainThreadCls;
        }
        break;
    case THREAD_PROP_INTER_TASK:
        {
            *((T_hTask*)pPropData) = pMe->m_hTask;
        }
        break;
    case THREAD_PROP_TASK_STATE:
        {           
            *((T_S32*)pPropData) = AK_Task_Status(pMe->m_hTask); 
        }
        break;
    default:
        {
            lRet = AK_EUNSUPPORT;
        }
        break;
    }

    return lRet;
}

static T_VOID CSubThread_NotifyTimer(T_U32 dwData)
{
    CSubThread *pMe = (CSubThread *)((ISubThread*)dwData)->pData;
    T_SYS_MAILBOX mailbox;
    
    mailbox.event = SYS_EVT_SUBTHREAD_NOTIFY;
    mailbox.param.lParam = (T_U32)&pMe->m_myISubThread;	
    IAppMgr_PostEvent(AK_GetAppMgr(), pMe->m_wMainThreadCls, &mailbox);
}

static T_S32  CSubThread_SetNotifyTimer(ISubThread *pISubThread, T_U32 dwMilliseconds)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if (AK_IS_VALIDHANDLE(pMe->m_hTimer))
    {
        lRet = CSubThread_StopNotifyTimer(pISubThread);
    }

    if (AK_IS_SUCCESS(lRet))
    {
#ifdef OS_ANYKA
        pMe->m_hTimer = AK_Create_Timer(CSubThread_NotifyTimer,
                                        (T_U32)pISubThread,
                                        dwMilliseconds,
                                        dwMilliseconds,
                                        AK_ENABLE_TIMER);
    
        if (AK_IS_INVALIDHANDLE(pMe->m_hTimer))
        {
            return AK_EFAILED;
        }
#endif
    }
    
    return lRet;
}

static T_S32  CSubThread_StopNotifyTimer(ISubThread *pISubThread)
{
    CSubThread *pMe = (CSubThread *)pISubThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if (AK_IS_VALIDHANDLE(pMe->m_hTimer))
    {
#ifdef OS_ANYKA
        AK_Control_Timer(pMe->m_hTimer, AK_DISABLE_TIMER);
        lRet = AK_Delete_Timer(pMe->m_hTimer);
        if (AK_IS_SUCCESS(lRet))
        {
            pMe->m_hTimer = AK_INVALID_TIMER;
        }
#endif
    }
    
    return lRet;
}


static T_S32 CSubThread_Create(ISubThread **ppi, T_SUBTHREAD_INITPARAM *pParam)
{
    T_S32 nErr = AK_SUCCESS;
    CSubThread *pNew = AK_NULL;

    if (AK_NULL == ppi)
    {
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;

    do 
    {
        pNew = AK_MALLOCRECORD(CSubThread);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_myISubThread), &g_ISubThreadFuncs);
        pNew->m_myISubThread.pData = (T_VOID*)pNew;
        
        nErr = CSubThread_Constructor(pNew, pParam);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CSubThread_Release(&pNew->m_myISubThread);
            pNew = AK_NULL;
            break;
        }
        
        *ppi = (ISubThread *)&pNew->m_myISubThread;
        
    } while(AK_FALSE);

    return nErr;
}

T_S32 CSubThread_New(ISubThread **ppi, T_SUBTHREAD_INITPARAM *pParam, T_BOOL bIsAutoRun)
{
    T_S32 lRet = AK_SUCCESS;
    IThread* pIMainThread = AK_NULL;
    
    if ((AK_NULL == ppi) || (AK_NULL == pParam) || (0 == pParam->ulStackSize) ||
        (AKAPP_CLSID_NONE == pParam->wMainThreadCls) || (AK_NULL == pParam->fnEntry))
    {
        return AK_EBADPARAM;
    }
    
    /*如果优先级为0，则使用父线程的优先级.*/
    if (SUBTHREAD_MAINTHREAD_PRIORITY == pParam->byPriority)
    {
        T_AppInfo* pMainThreadInfo = AK_NULL;
        
        pMainThreadInfo = AK_GetDefaultAppInfo(pParam->wMainThreadCls);
        if (AK_NULL == pMainThreadInfo)
        {
            Fwl_Print(C2, M_AKFRAME, "---CSubThread_New:No This main thread.");
            return AK_EFAILED;
        }   
        pParam->byPriority = pMainThreadInfo->sInitparam.byPriority;
    }
    
    /*如果没有输入子线程的名字，则使用默认名字.*/
    if (AK_NULL == pParam->pcszName)
    {
        pParam->pcszName = "SubThrd";
    }
    
    /*创建子线程，默认不运行.*/
    lRet = CSubThread_Create(ppi, pParam);
    
    /*创建子线程成功后，将它添加到主线程.*/
    if (AK_IS_SUCCESS(lRet))
    {
        pIMainThread = IAppMgr_GetApp(AK_GetAppMgr(), pParam->wMainThreadCls);
        if (AK_NULL == pIMainThread)
        {
            CSubThread_Release(*ppi);
            *ppi = AK_NULL;
            Fwl_Print(C2, M_AKFRAME, "---CSubThread_New:The main thread don't exist.");
            
            return AK_EFAILED;
        }
        lRet = IThread_AttachSubThread(pIMainThread, (IThread*)*ppi);
    }
    
    /*如果需要自动运行子线程，则运行它.*/
    if (AK_IS_SUCCESS(lRet) && bIsAutoRun)
    {
        lRet = ISubThread_Resume(*ppi);
    }
    
    return lRet;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////
