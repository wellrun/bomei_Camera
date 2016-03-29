/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKMsgDispatch.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKMsgDispatch.h"
#include "AKThread.h"
#include "fwl_osmalloc.h"
#include "fwl_pfdisplay.h"
#include "akos_api.h"
#include "AKList.h"
#include "string.h"
#include "fwl_evtmailbox.h"
#include "eng_debug.h"
#include "Lib_event.h"
#include "AKAppMgr.h"
#include "Gbl_Global.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define AK_DISPATCH_STACK_SIZE   (5*1024)
//###################################################################################
/*Msg register entry defination...*/
typedef struct tagMsgRegEntry
{
    T_eMsgRegType  eRegType;    /*Msg Register type*/
    T_HANDLE       hRegister;   /*Msg Register handle*/
    T_U32          dwEvtMsk;    /*Msg event mask, only the eRegType is MSG_REG_QUEUE, this member is valid.*/
}T_MsgRegEntry;


/*###################################################################################*/
/**********************************************************************************************************/


static IMsgDispatch *g_pIMsgDispatch = AK_NULL;


/*===================================================================================*/

typedef struct tagCMsgDispatch
{
    T_U32           m_Ref;
    IMsgDispatch    m_myIMsgDispatch;
    T_hHisr         m_hHISR;
    IList           *m_pIList;
    T_pVOID            m_pStackAddr;
}CMsgDispatch;

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================
static T_U32 CMsgDispatch_AddRef(IMsgDispatch *pi);
static T_U32 CMsgDispatch_Release(IMsgDispatch *pi);
static T_S32 CMsgDispatch_Activate(IMsgDispatch *pIMsgDispatch);
static T_S32 CMsgDispatch_Register(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister, T_eMsgRegType eRegType, T_U32 dwEvtMsk);
static T_S32 CMsgDispatch_UnRegister(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister);
static T_VOID CMsgDispatch_DispatchEvent(T_VOID);
//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IMsgDispatch) g_IMsgDispatchFuncs =
{
    CMsgDispatch_AddRef,
    CMsgDispatch_Release,
    CMsgDispatch_Activate,
    CMsgDispatch_Register,
    CMsgDispatch_UnRegister
};


/**************************************************************************
* @BRIEF    Get the interface pointer of MsgDispatch instance.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   None.
* @PARAM 
* @RETURN  IMsgDispatch*  The pointer of MsgDispatch interface.
***************************************************************************/
IMsgDispatch *AK_GetIMsgDispatch(T_VOID)
{
    return g_pIMsgDispatch;
}

/**************************************************************************
* @BRIEF    Active the MsgDispatch HISR to dispatch events.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
T_S32 AK_DispatchEvent(T_VOID)
{
    return IMsgDispatch_Activate(g_pIMsgDispatch);
}

/**************************************************************************
* @BRIEF     Default mailbox compare function.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMailBox1 [in]  The first mailbox item to be compared.
* @PARAM   pMailBox2 [in]  The second mailbox item to be compared.
* @RETURN  T_BOOL  The result of the comparison.
* @RETVAL  AK_TRUE       The two items are same.
* @RETVAL  AK_FALSE      The two items are diffrent.
***************************************************************************/
static T_BOOL AK_DefaultEventCmpCB(T_SYS_MAILBOX *pMailBox1, T_SYS_MAILBOX *pMailBox2)
{
    if (pMailBox1->event == pMailBox2->event)
    {
		if(SYS_EVT_USER_KEY==pMailBox1->event)
		{//xuyr Swd200001120
			if(
				pMailBox1->param.c.Param1 == pMailBox2->param.c.Param1
			&&	pMailBox1->param.c.Param2 == pMailBox2->param.c.Param2
			)
				return AK_TRUE;
			else
				return AK_FALSE;
		}
		
        return AK_TRUE;
    }

    return AK_FALSE;
}

/**************************************************************************
* @BRIEF  Post a system event to the system queue, then dispatch the event.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMailBox  [in]  Point to the mailbox whitch will be post to system queue.
* @PARAM   pfnCmp    [in]  Compare function, if the param bIsUnique is AK_TRUE, it must be set.
* @PARAM   bIsUnique [in]  Indicate whether this mailbox need to be unique in the system queue.
* @PARAM   bIsHead   [in]  Indicate whether this mailbox need to be put in the system queue head.
* @PARAM   bIsTerminalInput [in]  Indicate whether this mailbox is an input event. 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
T_S32 AK_PostEventEx(T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                         T_BOOL bIsUnique, T_BOOL bIsHead,
                         T_BOOL bIsTerminalInput)
{
     vBOOL bIsSuccess = vFALSE;
     
     if (AK_NULL == pMailBox)
     {
        return AK_EBADPARAM;
     }
     if ((AK_TRUE == bIsUnique) && (AK_NULL == pfnCmp))
     {
          pfnCmp = (T_pfnEvtCmp)AK_DefaultEventCmpCB;
     }
     pMailBox->bIsUnique = bIsUnique;
     pMailBox->bIsHead   = bIsHead;
     pMailBox->fnCmp     = pfnCmp;
    
   
     bIsSuccess = MB_PushEvent(pMailBox, pfnCmp, bIsUnique, bIsHead, bIsTerminalInput);
       
     if (bIsSuccess)
     {
         AK_DispatchEvent();
              
         return AK_SUCCESS;
     }
     AK_DEBUG_OUTPUT("MB_PushEvent fail!\n");
     return  AK_EFAILED;
}

/**************************************************************************
* @BRIEF     Get the system event from the system queue head, but not remove this event 
                   from the system queue.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMailBox              [in/out]  Point to the mailbox structure whitch will be set 
                                                          value after this funtion execution.
* @PARAM   bIsTerminalInput  [in]        Indicate whether this mailbox is an input event.  
* @RETURN  T_BOOL  The result of this function executing.
* @RETVAL  AK_TRUE       Peek the system event successfully.
* @RETVAL  AK_FALSE      The system queue is empty or peeking event failed.
***************************************************************************/
static T_BOOL AK_PeekEvent(T_SYS_MAILBOX *pMailBox, T_BOOL bIsTerminalInput)
{
     if (AK_NULL == pMailBox)
     {
        return AK_FALSE;
     }

     return MB_PeekEvent(pMailBox, bIsTerminalInput);
}

/**************************************************************************
* @BRIEF     Delete the system event from the system queue head.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   bIsTerminalInput  [in]        Indicate whether this mailbox is an input event.  
* @RETURN  T_BOOL  The result of this function executing.
* @RETVAL  AK_TRUE       Delete the system event successfully.
* @RETVAL  AK_FALSE      Delete event failed.
***************************************************************************/
static T_BOOL AK_DeleteEvent(T_BOOL bIsTerminalInput)
{
     return MB_DeleteEvent(bIsTerminalInput);
}

/**************************************************************************
* @BRIEF     Get the system event from the system queue head, and remove this event 
                   from the system queue.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMailBox              [in/out]  Point to the mailbox structure whitch will be set 
                                                          value after this funtion execution.
* @PARAM   bIsTerminalInput  [in]        Indicate whether this mailbox is an input event.  
* @RETURN  T_BOOL  The result of this function executing.
* @RETVAL  AK_TRUE       Get the system event successfully.
* @RETVAL  AK_FALSE      The system queue is empty or get event failed.
***************************************************************************/
static T_BOOL AK_GetEvent(T_SYS_MAILBOX *pMailBox, T_BOOL bIsTerminalInput)
{
     if (AK_NULL == pMailBox)
     {
        return AK_FALSE;
     }

     return MB_GetEvent(pMailBox, bIsTerminalInput);
}

/**************************************************************************
* @BRIEF    Get the event mask witch the event belong to.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM  eEvent [in]  Event type.
* @PARAM 
* @RETURN  T_S32  The event mask of the event belong to.
* @RETVAL 
***************************************************************************/
static __inline T_U32 AK_GetEventMsk(T_SYS_EVTID eEvent)
{
    if (eEvent > SYS_EVT_XXX_MSK_BEGIN)
    {
        return SYS_EVT_XXX_MSK;
    }
    else if (eEvent > SYS_EVT_BT_MSK_BEGIN)
    {
        return SYS_EVT_BT_MSK;
    }
    else if (eEvent > SYS_EVT_MMI_MSK_BEGIN)
    {
        return SYS_EVT_MMI_MSK;
    }
    else if (eEvent > SYS_EVT_AUDIO_MSK_BEGIN)
    {
        return SYS_EVT_AUDIO_MSK;
    }
    else if (eEvent > SYS_EVT_VATC_MSK_BEGIN)
    {
        return SYS_EVT_VATC_MSK;
    }
    else if (eEvent > SYS_EVT_INPUT_MSK_BEGIN)
    {
        return SYS_EVT_INPUT_MSK;
    }
    else if (eEvent > SYS_EVT_APP_MSK_BEGIN)
    {
        return SYS_EVT_APP_MSK;
    }
    else if (eEvent > SYS_EVT_COMM_MSK_BEGIN)
    {
        return SYS_EVT_COMM_MSK;
    }

    //Consider not all the mmi sys events were converted to akos sys events.
    if (eEvent < AK_SYS_EVT_BASE)
    {
        return SYS_EVT_MMI_MSK;
    }

    return SYS_EVT_MSK_NONE;
}

/**************************************************************************
* @BRIEF  Check whether this event is the one witch the thread concerned.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pThread [in]  A pointer to the Thread instanc.
* @PARAM   eEvent   [in]  The event type.
* @RETURN  T_BOOL .
* @RETVAL  AK_TRUE    This event is the thread needed.
* @RETVAL  AK_FALSE   The thread doesn't need this event.
***************************************************************************/
static __inline T_BOOL AK_CheckEvent(T_MsgRegEntry* pRegEntry, T_SYS_EVTID eEvent)
{
    T_U32 ulEvtMsk       = 0x0;

    if (MSG_REG_THREAD == pRegEntry->eRegType)
    {
        ulEvtMsk = IThread_GetEvtMsk((IThread*)pRegEntry->hRegister);
    }
    else
    {
        ulEvtMsk = pRegEntry->dwEvtMsk;
    }
    
    ulEvtMsk &= AK_GetEventMsk(eEvent); //get the actual enabled events
    if (ulEvtMsk > 0)
    {
         return AK_TRUE;
    }
    
    return AK_FALSE;
}

/**************************************************************************
* @BRIEF  Send the event to the thread's queue.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   queue      [in]  The thread queue handle.
* @PARAM   message  [in]  Point to the mailbox whitch will be sent to thread's queue.
* @PARAM   size         [in]  The message size.
* @PARAM   pfnEvtCmp    [in]  Compare function, if the param bIsUnique is AK_TRUE, it must be set.
* @PARAM   bIsUnique [in]  Indicate whether this mailbox need to be unique in the thread's queue.
* @PARAM   bIsHead   [in]  Indicate whether this mailbox need to be put in the thread's queue head.
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 AK_Send_To_Queue_Ex(T_hQueue queue, T_VOID *message,
                                      T_U32 size, T_pfnEvtCmp pfnEvtCmp, 
                                      T_BOOL bIsUnique, T_BOOL bIsHead)
{
    if (bIsUnique && bIsHead)
    {
        return AK_Send_Unique_To_Front_of_Queue(queue, message, size, AK_NO_SUSPEND, pfnEvtCmp);
    }
    else if (bIsUnique)
    {
        return AK_Send_Unique_To_Queue(queue, message, size, AK_NO_SUSPEND, pfnEvtCmp);
    }
    else if (bIsHead)
    {
         return AK_Send_To_Front_of_Queue(queue, message, size, AK_NO_SUSPEND);
    }

    return AK_Send_To_Queue(queue, message, size, AK_NO_SUSPEND);
}

/**************************************************************************
* @BRIEF    The constructor function of the MsgDispatch component.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMe [in]  The pointer to the CMsgDispatch structure.
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CMsgDispatch_Constructor(CMsgDispatch *pMe)
{
    T_S32 lRet = AK_SUCCESS;

    pMe->m_pStackAddr = Fwl_Malloc(AK_DISPATCH_STACK_SIZE);
    if (pMe->m_pStackAddr == AK_NULL)
    {
        return AK_ENOMEMORY;
    }   
    memset(pMe->m_pStackAddr, 0, AK_DISPATCH_STACK_SIZE);
    
    pMe->m_hHISR = AK_Create_HISR(CMsgDispatch_DispatchEvent, "MsgDis", 2, 
                                 pMe->m_pStackAddr, AK_DISPATCH_STACK_SIZE);
    if (AK_IS_INVALIDHANDLE(pMe->m_hHISR) )
    {
        return AK_EFAILED;
    }
    lRet = CList_New(&pMe->m_pIList, AK_TRUE);

    Fwl_Print(C3, M_AKFRAME, "Exit CMsgDispatch_Init lRet=%d.\n", lRet);
    
    return lRet;
}

/**************************************************************************
* @BRIEF    The Destructor function of the MsgDispatch component.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pMe [in]  The pointer to the CMsgDispatch structure.
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CMsgDispatch_Destructor(CMsgDispatch *pMe)
{
    //Add your code here...
    IList_Release(pMe->m_pIList);
    AK_Delete_HISR(pMe->m_hHISR);

    pMe->m_pStackAddr = Fwl_Free(pMe->m_pStackAddr);    

    return AK_SUCCESS;
}

/**************************************************************************
* @BRIEF    Add  the refrence conter.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM  pi [in]  The pointer to the IMsgDispatch interface.
* @PARAM 
* @RETURN  T_U32  The current refrence conter of this component.
* @RETVAL  
***************************************************************************/
static T_U32 CMsgDispatch_AddRef(IMsgDispatch *pi)
{
    CMsgDispatch *pMe = (CMsgDispatch *)pi->pData;

    return ++pMe->m_Ref;
}

/**************************************************************************
* @BRIEF    reduce the refrence conter, when the refrence conter is zero, it will destroy itself.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM  pi [in]  The pointer to the IMsgDispatch interface.
* @PARAM 
* @RETURN  T_U32  The current refrence conter of this component.
* @RETVAL  
***************************************************************************/
static T_U32 CMsgDispatch_Release(IMsgDispatch *pi)
{
    CMsgDispatch *pMe = (CMsgDispatch *)pi->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CMsgDispatch_Destructor(pMe);
        pMe = Fwl_Free(pMe);
        g_pIMsgDispatch = AK_NULL;
        
        return 0;
    }

    return pMe->m_Ref;
}

/**************************************************************************
* @BRIEF    The MsgDispatch routine.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_VOID.
* @RETVAL  
***************************************************************************/
#ifdef OS_ANYKA
//#pragma Otime
#endif
static T_VOID CMsgDispatch_DispatchEvent(T_VOID)
{
    CMsgDispatch *pMe    = AK_NULL;
    T_S32 lRet           = AK_SUCCESS;
    T_BOOL bIsConsumed   = AK_FALSE;
    T_BOOL bIsAppNeed    = AK_FALSE;
    IThread *pActThread  = AK_NULL;
    T_MsgRegEntry* pRegEntry = AK_NULL;
    T_SYS_MAILBOX mailbox; 

    if (AK_NULL == g_pIMsgDispatch)
        return;
    
    /*获取消息分发器实例指针*/
    pMe = (CMsgDispatch *)g_pIMsgDispatch->pData;

    do
    { 
        /*首先获取输入系统队列头的消息，并从队列头删除*/
        if (AK_GetEvent( &mailbox, AK_TRUE))
        {
            /*如果有输入事件，则发送给当前激活应用*/
            pActThread = IAppMgr_GetActiveApp(AK_GetAppMgr());
            if ((AK_NULL != pActThread))
            {
                lRet = AK_Send_To_Queue_Ex(IThread_GetQueue(pActThread),
                                          (T_VOID*)&mailbox, sizeof(T_SYS_MAILBOX),
                                           mailbox.fnCmp, mailbox.bIsUnique,
                                           mailbox.bIsHead);
                if (AK_IS_FAILURE(lRet) && (AK_EXIST_MESSAGE != lRet))
                {
                    if (!gs.emtest)
                    {
                       Fwl_Print(C2, M_AKFRAME, "--Send Input Evt failed: lRet=%d.\n", lRet);
                    }
                }
            }
            else
            {
                /*如果当前没有激活应用，则输出出错信息。此情况不应出现*/
                if (!gs.emtest)
                {
                   Fwl_Print(C2, M_AKFRAME, "--Send Input Evt failed, Current active app is NULL.\n");
                }
            }
        }
        
        /*获取非输入系统队列头的消息，且不从队列头删除*/
        if (AK_PeekEvent( &mailbox, AK_FALSE))
        {
            T_hQueue hQueue = AK_INVALID_QUEUE;
            
            /*如果有非输入事件，则分发该事件给所有关心的应用*/
            pRegEntry = (T_MsgRegEntry*)(IList_FindFirst(pMe->m_pIList));
            while (AK_NULL != pRegEntry)
            { 
                /*判断该事件是否是指定应用所需要的*/
                if (AK_CheckEvent(pRegEntry, mailbox.event))
                {
                    if (MSG_REG_THREAD == pRegEntry->eRegType)
                    {
                         hQueue = IThread_GetQueue((IThread*)pRegEntry->hRegister);
                    }
                    else
                    {
                         hQueue = (T_hQueue)pRegEntry->hRegister;
                    }
                    
                    lRet = AK_Send_To_Queue_Ex(hQueue, (T_VOID*)&mailbox, 
                                               sizeof(T_SYS_MAILBOX), mailbox.fnCmp, 
                                               mailbox.bIsUnique, mailbox.bIsHead);
                    if (AK_IS_SUCCESS(lRet) || (AK_EXIST_MESSAGE == lRet))
                    {
                        bIsConsumed = AK_TRUE;
                    }
                    else
                    {
                        bIsAppNeed = AK_TRUE;
                        if (!gs.emtest)
                        {
                           Fwl_Print(C2, M_AKFRAME, "--Send Sys Evt=0x%x failed: lRet=%d.\n", mailbox.event,lRet);
                           if (MSG_REG_THREAD == pRegEntry->eRegType)
                           {
                               Fwl_Print(C2, M_AKFRAME, "--Current Thread is:%s.\n", IThread_GetName((IThread*)pRegEntry->hRegister));
                           }
                           else
                           {
                               Fwl_Print(C2, M_AKFRAME, "--Current Thread is third part.\n");
                           }
                        }
                    }
                } 
              
                /*获取下一个注册的应用程序接口指针*/
                pRegEntry = (T_MsgRegEntry*)(IList_FindNext(pMe->m_pIList));
             }

             /*如果该事件被处理了或该事件没有应用关心，则从队列头删除该消息*/
             if (bIsConsumed || !bIsAppNeed)
             {
                 AK_DeleteEvent(AK_FALSE); //delete this event from sys queue.
             }
         }
    }while (AK_FALSE);
}
#ifdef OS_ANYKA
//#pragma no_Otime
#endif

/**************************************************************************
* @BRIEF     Active the MsgDispatch HISR routine.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM    pIMsgDispatch [in]  The interface pointer of IMsgDispatch.
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CMsgDispatch_Activate(IMsgDispatch *pIMsgDispatch)
{
    CMsgDispatch *pMe = (CMsgDispatch *)pIMsgDispatch->pData;
    
    return AK_Activate_HISR(pMe->m_hHISR);
}

/**************************************************************************
* @BRIEF    Find the MsgRegEntry through the handle .
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM    pIMsgDispatch [in]  The interface pointer of IMsgDispatch.
* @PARAM    pIThread         [in]  The thread pointer.
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_BOOL CMsgDispatch_IsContainCB(T_MsgRegEntry *pRegEntry, T_HANDLE hHandle)
{
    if (pRegEntry->hRegister == hHandle)
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}


/**************************************************************************
* @BRIEF    Find the MsgRegEntry through the handle .
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM    pIMsgDispatch [in]  The interface pointer of IMsgDispatch.
* @PARAM    pIThread         [in]  The thread pointer.
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static __inline T_MsgRegEntry* CMsgDispatch_FindByHandle(CMsgDispatch *pMe, T_HANDLE hRegister)
{
    T_MsgRegEntry* pRegEntry = AK_NULL;

    if (AK_IS_VALIDHANDLE(hRegister))
    {
         pRegEntry = (T_MsgRegEntry*)IList_Find(pMe->m_pIList, (T_VOID*)hRegister,
                                                (T_pfnContainCB)CMsgDispatch_IsContainCB);
    }
    
    return pRegEntry;
}

/**************************************************************************
* @BRIEF    Register the thread to the MsgDispatch to be enable to get system events.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM    pIMsgDispatch [in]  The interface pointer of IMsgDispatch.
* @PARAM    pIThread         [in]  The thread pointer.
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CMsgDispatch_Register(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister, T_eMsgRegType eRegType, T_U32 dwEvtMsk)
{
    CMsgDispatch *pMe = (CMsgDispatch *)pIMsgDispatch->pData;
    T_MsgRegEntry* pRegEntry = AK_NULL;
    
    if (!AK_IS_VALIDHANDLE(hRegister))
    {
        return AK_EBADPARAM;
    }
    
    //Avoid registering twice...
    if (AK_NULL == CMsgDispatch_FindByHandle(pMe, hRegister))
    {
        pRegEntry = (T_MsgRegEntry*)Fwl_Malloc(sizeof(T_MsgRegEntry));
        if (AK_NULL == pRegEntry)
        {
             return AK_ENOMEMORY;
        }

        pRegEntry->hRegister = hRegister;
        pRegEntry->eRegType  = eRegType;
        pRegEntry->dwEvtMsk  = dwEvtMsk;
        
        return IList_Add(pMe->m_pIList,(T_VOID*)pRegEntry);
    }
    else
    {
        pRegEntry->dwEvtMsk  = dwEvtMsk;
    }
    
    return AK_SUCCESS;
}

/**************************************************************************
* @BRIEF      Unregister the thread to the MsgDispatch, this thread will not receive system
                    event latter.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM    pIMsgDispatch [in]  The interface pointer of IMsgDispatch.
* @PARAM    pIThread         [in]  The thread pointer.
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CMsgDispatch_UnRegister(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister)
{
    CMsgDispatch *pMe = (CMsgDispatch *)pIMsgDispatch->pData;
    T_MsgRegEntry* pRegEntry = AK_NULL;
    
    if (!AK_IS_VALIDHANDLE(hRegister))
    {
        return AK_EBADPARAM;
    }
    
    pRegEntry = CMsgDispatch_FindByHandle(pMe, hRegister);
    if (AK_NULL == pRegEntry)
    {
         return AK_ENOTFOUND;
    }
    
    return IList_Delete(pMe->m_pIList,(T_VOID*)pRegEntry);
}

/**************************************************************************
* @BRIEF    Create the MsgDispatch instance.
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM   pp [in/out]  Return the IMsgDispatch pointer.
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
T_S32 CMsgDispatch_New(IMsgDispatch **pp)
{
    T_S32 nErr = AK_SUCCESS;
    CMsgDispatch *pNew = AK_NULL;

    if (AK_NULL == pp)
    {
        return AK_EBADPARAM;
    }
    
    *pp = AK_NULL;

    if (AK_NULL != g_pIMsgDispatch)
    {
        *pp = g_pIMsgDispatch;
        AK_ADDREFIF(g_pIMsgDispatch);

        return AK_SUCCESS;
    }
    
    do 
    {
        pNew = AK_MALLOCRECORD(CMsgDispatch);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);

        pNew->m_Ref     = 1;
        AK_SETVT(&(pNew->m_myIMsgDispatch), &g_IMsgDispatchFuncs);
        pNew->m_myIMsgDispatch.pData = (T_VOID*)pNew;

        nErr = CMsgDispatch_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CMsgDispatch_Release(&pNew->m_myIMsgDispatch);
            pNew = AK_NULL;
            break;
        }

        *pp = (T_VOID *)&pNew->m_myIMsgDispatch;
        g_pIMsgDispatch = *pp;

    } while(AK_FALSE);

    return nErr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

