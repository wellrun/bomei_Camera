/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKThread.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKThread.h"
#include "fwl_evtmailbox.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKMsgDispatch.h"
#include "AKVector.h"
#include "Akos_api.h"

//#################################################################################
#define THREAD_DO_NOTHING
/*================================================================================*/
typedef struct tagCThread
{
   IThread   *m_pIBase;       //Base class, must contain this variable.
   T_U32      m_Ref;
   IThread    m_myIThread;
   ICBThread *m_pIListener;   //a pointer to the listener instance, no need to release it.
   T_U32	  m_ulEvtMsk;
   T_hTask	  m_hTask;
   T_pVOID	  m_pStackAddr;
   T_hQueue   m_hQueue;
   T_pVOID    m_pQueueAddr;
   T_pCSTR    m_pcszName;
   IVector   *m_pISubThreads; //Contains the interface pointers of subthreads . 
}CThread;

/*==============================================================================*/
/*##############################################################################*/
/*  =======================>        Call back function declare begin       <=========================*/
#define AK_CHECKVTFUN(p,fn)  ((AK_NULL != p)&&(AK_NULL != p->pvt->fn)) ? AK_TRUE : AK_FALSE


__inline T_S32 ICBThread_Prepare(ICBThread *pICBThread)
{ 
    if (AK_CHECKVTFUN(pICBThread, Prepare))
    {
         return  AK_GETVT(ICBThread, pICBThread)->Prepare(pICBThread);
    }

    return AK_EFAILED;
}

__inline T_S32 ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, 
                                T_SYS_PARAM *pEvtParam)
{ 
    if (AK_CHECKVTFUN(pICBThread, Handle))
    {
         return  AK_GETVT(ICBThread, pICBThread)->Handle(pICBThread,eEvent,pEvtParam);
    }

    return AK_EFAILED;
}

__inline T_S32 ICBThread_Free(ICBThread *pICBThread)
{ 
    if (AK_CHECKVTFUN(pICBThread, Free))
    {
         return  AK_GETVT(ICBThread, pICBThread)->Free(pICBThread);
    }

    return AK_EFAILED;
}
/*  =======================>         Call back function declare end        <=========================*/
/*##############################################################################*/
/*==============================================================================*/

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================
static T_VOID CThread_Task(T_U32 argc, T_VOID *argv);
static T_U32  CThread_AddRef(IThread *pIThread);
static T_U32  CThread_Release(IThread *pIThread);
static T_S32  CThread_Run(IThread *pIThread);
static T_S32  CThread_Suspend(IThread *pIThread);
static T_S32  CThread_Terminate(IThread *pIThread);
static T_S32  CThread_DeleteTask(IThread *pIThread);
static T_S32  CThread_Exit(IThread *pIThread);
static T_S32  CThread_Register(IThread *pIThread, ICBThread *pIListener);
static T_S32  CThread_SetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData);
static T_S32  CThread_GetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData);
static T_S32  CThread_AttachSubThread(CThread *pMe, IThread* pISubThread);
static T_S32  CThread_DetachSubThread(CThread *pMe, IThread* pISubThread);
static T_S32  CThread_FreeSubThreadsVector(CThread *pMe);
//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IThread) g_IThreadFuncs =
{
    CThread_AddRef,
    CThread_Release,
    CThread_Run,
    CThread_Suspend,
    CThread_Terminate,
    CThread_Exit,
    CThread_Register,
    CThread_SetProperty,
    CThread_GetProperty
};

static T_S32 CThread_Constructor(CThread *pMe, T_THREAD_INITPARAM *pParam)
{
	if (AK_NULL == pParam)
	{
	    return AK_EBADPARAM;
	}

	pMe->m_pIBase       = AK_NULL;
	pMe->m_pIListener   = AK_NULL;
	pMe->m_ulEvtMsk     = 0x00000000;

    pMe->m_pStackAddr   = AK_NULL;
    pMe->m_pQueueAddr   = AK_NULL;
    pMe->m_hTask        = AK_INVALID_TASK;
	pMe->m_hQueue       = AK_INVALID_QUEUE;
    pMe->m_pcszName     = (T_pCSTR)pParam->pcszName;
    pMe->m_pISubThreads = AK_NULL;

	pMe->m_pStackAddr = Fwl_Malloc(pParam->ulStackSize);
    Fwl_Print(C3, M_AKFRAME, "---I'm in CThread_Constructor pMe->m_pStackAddr=0x%x,stacksize=%d.",
        pMe->m_pStackAddr,pParam->ulStackSize);
	if (pMe->m_pStackAddr == AK_NULL)
	{
		return AK_ENOMEMORY;
	}
	
	memset(pMe->m_pStackAddr, 0, pParam->ulStackSize);
    
	pMe->m_hTask  = AK_Create_Task((T_VOID*)CThread_Task,pParam->pcszName,
                                   1 ,pMe, pMe->m_pStackAddr,
                                   pParam->ulStackSize,
                                   pParam->byPriority,
                                   pParam->ulTimeSlice,
                                   AK_PREEMPT,AK_NO_START);

    if (AK_IS_INVALIDHANDLE(pMe->m_hTask))
    {
		return AK_EFAILED;
    }

    //Check whether need Queue.
    if (pParam->ulQueueSize > 0)
    {
        pMe->m_pQueueAddr = Fwl_Malloc(pParam->ulQueueSize);
        if (pMe->m_pQueueAddr == AK_NULL)
        {
            return AK_ENOMEMORY;
        }
        
        memset(pMe->m_pQueueAddr, 0, pParam->ulQueueSize);

        
	    pMe->m_hQueue = AK_Create_Queue(pMe->m_pQueueAddr,
                                        pParam->ulQueueSize, AK_FIXED_SIZE,
                                        sizeof(T_SYS_MAILBOX), AK_FIFO);
        
        if (AK_IS_INVALIDHANDLE(pMe->m_hQueue))
        {
    	    return AK_EFAILED;
        }
    }
    
    return AK_SUCCESS;
}

static T_S32 CThread_Destructor(CThread *pMe)
{
    //Add your code here...
    AK_RELEASEIF(pMe->m_pIBase);

    //Give a chance to free the child class instence.
    ICBThread_Free(pMe->m_pIListener);
    
    CThread_DeleteTask(&pMe->m_myIThread);
    
    return AK_SUCCESS;
}

static T_U32 CThread_AddRef(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    return ++pMe->m_Ref;
}

static T_U32 CThread_Release(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CThread_Destructor(pMe);
        Fwl_Free(pMe);
        
        return 0;
    }
    
    return pMe->m_Ref;
}



static T_VOID CThread_Task(T_U32 argc, T_VOID *argv)
{
#ifdef OS_ANYKA
    CThread *pMe = (CThread *)argv;
#else
    CThread *pMe = (CThread *)argc;
#endif
    T_S32 lRet = AK_SUCCESS;
    T_U32 ulActQuSize;
    T_U32 ulOption = AK_SUSPEND;
    T_SYS_MAILBOX mailbox;
    
#ifndef WIN32
    Fwl_Print(C3, M_AKFRAME, "################CThread CThread_Task:");
#endif
    
    if (AK_IS_VALIDHANDLE(pMe->m_hQueue))
    {        
        //Supply a chance for init the sub instance...
        ICBThread_Prepare(pMe->m_pIListener);
        
        //Enter the message loop...
        while (1)
        {

            AK_Feed_Watchdog(4);
            
            //Fwl_Print("CThread Waiting\r\n");
            lRet = AK_Receive_From_Queue(pMe->m_hQueue,&mailbox,
                                         sizeof(T_SYS_MAILBOX),
                                         &ulActQuSize,ulOption);
            if (AK_IS_SUCCESS(lRet))
            {
                if (AK_SUSPEND == ulOption)
                {
                    ulOption = AK_NO_SUSPEND;
                }
                ICBThread_Handle(pMe->m_pIListener,mailbox.event,&mailbox.param);

                /*子线程运行结束，删除该子线程.*/
                if (SYS_EVT_SUBTHREAD_FINISH == mailbox.event)
                {
                     CThread_DetachSubThread(pMe, (IThread*)mailbox.param.lParam);
                }
            }
            else
            {
                AK_DispatchEvent();
                ulOption = AK_SUSPEND;
            }
        }
    }
    else
    {
        ICBThread_Handle(pMe->m_pIListener, 0, AK_NULL);
    }
}


static T_S32 CThread_Run(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
#ifndef WIN32     
    Fwl_Print(C3, M_AKFRAME, "CThread CThread_Run .");
#endif
    if (IThread_GetState(pIThread) == AK_TERMINATED)
    {
        AK_Reset_Task(pMe->m_hTask, 1, pMe);
    }
    
    return AK_Resume_Task(pMe->m_hTask);
}


static T_S32 CThread_Suspend(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    return AK_Suspend_Task(pMe->m_hTask);
}


static T_S32 CThread_Terminate(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    return AK_Terminate_Task(pMe->m_hTask);
}


static T_S32 CThread_DeleteTask(IThread *pIThread)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    /* 关闭所有子线程，并释放存放它们的动态数组.*/
    CThread_FreeSubThreadsVector(pMe);

    /* 关闭当前线程，并删除.*/
    if (AK_IS_VALIDHANDLE(pMe->m_hTask))
    {
        AK_Terminate_Task(pMe->m_hTask);
        AK_Delete_Task(pMe->m_hTask);
        pMe->m_hTask = AK_INVALID_TASK;
    }

    /* 删除所用队列.*/
    if (AK_IS_VALIDHANDLE(pMe->m_hQueue))
    {
        AK_Delete_Queue(pMe->m_hQueue);
        pMe->m_hQueue = AK_INVALID_QUEUE;
    }
    
    /* 释放队列所用空间.*/
    if (AK_NULL != pMe->m_pQueueAddr)
    {
        pMe->m_pQueueAddr = Fwl_Free(pMe->m_pQueueAddr);
    }
    
    /* 释放堆栈所用空间.*/
    if (AK_NULL != pMe->m_pStackAddr)
    {
        pMe->m_pStackAddr = Fwl_Free(pMe->m_pStackAddr);
    }    
    
    return AK_SUCCESS;
}

static T_S32  CThread_Exit(IThread *pIThread)
{
    while (IThread_Release(pIThread) > 0)
    {
        THREAD_DO_NOTHING;//Don't remove this ; symbol.
    }
    
    return AK_SUCCESS;
}


static T_S32 CThread_Register(IThread *pIThread, ICBThread *pIListener)
{
    CThread *pMe = (CThread *)pIThread->pData;
    
    pMe->m_pIListener = pIListener;
    
    return AK_SUCCESS;
}

static T_S32 CThread_SetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData)
{
    CThread *pMe = (CThread *)pIThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if ((AK_NULL == pIThread) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }
    
    switch(wPropID)
    {
    case THREAD_PROP_EVENT_MSK:
        {
            T_U32 *pulEvtMsk = (T_U32 *)pPropData;
            
            pMe->m_ulEvtMsk  = *pulEvtMsk;
        }
        break;
    case THREAD_PROP_ATTACH_SUBTHREAD:
        {
            return CThread_AttachSubThread(pMe, (IThread*)pPropData);
        }
        break;
    case THREAD_PROP_DETACH_SUBTHREAD:
        {
            return CThread_DetachSubThread(pMe, (IThread*)pPropData);
        }
        break;
    default:
        break;
    }
    
    return lRet;
}


static T_S32 CThread_GetProperty(IThread *pIThread, T_U16 wPropID, T_VOID*  pPropData)
{
    CThread *pMe = (CThread *)pIThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if ((AK_NULL == pIThread) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }
    
    switch(wPropID)
    {
    case THREAD_PROP_EVENT_MSK:
        {
            *((T_U32*)pPropData)   = pMe->m_ulEvtMsk;
        }
        break;
    case THREAD_PROP_INTER_TASK:
        {
            *((T_hTask*)pPropData) = pMe->m_hTask;
        }
        break;
    case THREAD_PROP_INTER_QUEUE:
        {
            *((T_hQueue*)pPropData) = pMe->m_hQueue;
        }
        break;
    case THREAD_PROP_TASK_NAME:
        {           
            *((T_CHR**)pPropData) = (T_CHR*)pMe->m_pcszName; 
        }
        break;
    case THREAD_PROP_TASK_STATE:
        {           
            *((T_S32*)pPropData) = AK_Task_Status(pMe->m_hTask); 
        }
        break;
    case THREAD_PROP_BACKGROUND:
        {
            *((T_BOOL*)pPropData) = AK_TRUE;
        }
        break;
    default:
        break;
    }
    
    return lRet;
}

static T_S32 CThread_AttachSubThread(CThread *pMe, IThread* pISubThread)
{
    T_S32 lRet = AK_SUCCESS;
    T_U16 i, wNum;
    IThread* pISub = AK_NULL;
    
    if (AK_NULL == pISubThread)
    {
        return AK_EBADPARAM;
    }
    
    if (AK_NULL == pMe->m_pISubThreads)
    {
        lRet = CVector_New(&pMe->m_pISubThreads);
        if (AK_IS_FAILURE(lRet))
        {
            return lRet;
        }
    }

    wNum = IVector_Size(pMe->m_pISubThreads);
    
    for (i=0; i<wNum; i++)
    {
        pISub = (IThread*)IVector_ElementAt(pMe->m_pISubThreads, i);
        if (pISub == pISubThread)
        {            
            return AK_EEXISTED;
        }
    }
    
    IThread_AddRef(pISubThread);
    lRet = IVector_AddElement(pMe->m_pISubThreads, (T_VOID*)pISubThread);
    
    return lRet;
}

static T_S32 CThread_DetachSubThread(CThread *pMe, IThread* pISubThread)
{
    T_U16 i, wNum;
    IThread* pISub = AK_NULL;

    if (AK_NULL == pISubThread)
    {
        return AK_EBADPARAM;
    }

    if (AK_NULL != pMe->m_pISubThreads)
    {
        wNum = IVector_Size(pMe->m_pISubThreads);
        
        for (i=0; i<wNum; i++)
        {
            pISub = (IThread*)IVector_ElementAt(pMe->m_pISubThreads, i);
            if (pISub == pISubThread)
            {
                IVector_RemoveAt(pMe->m_pISubThreads, i);
                IThread_Release(pISub);
                
                return AK_SUCCESS;
            }
        }
    }
    
    return AK_ENOTFOUND;
}

static T_S32 CThread_FreeSubThreadsVector(CThread *pMe)
{
    T_U16 i, wNum;
    IThread* pISubThread = AK_NULL;

    if (AK_NULL != pMe->m_pISubThreads)
    {
        wNum = IVector_Size(pMe->m_pISubThreads);

        for (i=0; i<wNum; i++)
        {
            pISubThread = (IThread*)IVector_ElementAt(pMe->m_pISubThreads, i);
            if (AK_NULL != pISubThread)
            {
                 while (IThread_Release(pISubThread) > 0)
                 {
                     THREAD_DO_NOTHING; //Don't remove this ; symbol.
                 }
                 pISubThread = AK_NULL;               
            }
        }
        
        IVector_Release(pMe->m_pISubThreads);
        pMe->m_pISubThreads = AK_NULL;
    }

    return AK_SUCCESS;
}


T_S32 CThread_New(IThread **ppi, T_THREAD_INITPARAM *pParam)
{
    T_S32 nErr = AK_SUCCESS;
    CThread *pNew = AK_NULL;
    
    if (AK_NULL == ppi)
    {
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CThread);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_myIThread), &g_IThreadFuncs);
        pNew->m_myIThread.pData = (T_VOID*)pNew;
        
        nErr = CThread_Constructor(pNew, pParam);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CThread_Release(&pNew->m_myIThread);
            pNew = AK_NULL;
            break;
        }
        
        *ppi = (IThread *)&pNew->m_myIThread;
        
    } while(AK_FALSE);
    
    return nErr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////
