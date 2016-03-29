/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAppMgr.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKAppMgr.h"
#include "fwl_evtmailbox.h"
#include "fwl_vme.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKMsgDispatch.h"
#include "Gbl_Global.h"
#include "Akos_api.h"



//#################################################################################
#define AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS

#ifdef  AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
#define AKAPPMGR_HISR_STACK_SIZE        (5*1024)
#endif
/*================================================================================*/
typedef struct tagCAppMgr
{
    IThread   *m_pIBase;      //point to the base class instance.
    T_U32      m_Ref;
    IAppMgr    m_myIAppMgr;
    ICBThread  m_ICBThread;
    //Add your data here...
    IVector   *m_pAppList;    //All app list.
    T_U16      m_uBGAppNum;   //Backgruond app number.
#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    T_hHisr    m_hHisr;       //High ISR service.
    T_VOID    *m_HisrStack;   //Hisr stack base.
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
}CAppMgr;

/*=================================================================================*/
static IAppMgr *g_pIAppMgr = AK_NULL;

/*=================================================================================*/
static const T_AppInfo g_sDefaultAppInfo[] =
{
  /*{App_ClsID,					{Name(maxlen is 8), 	Priority,	Time_Slice,	Stack_Size, 	Queue_Size}},*/
    {AKAPP_CLSID_APPMGR,		{"AppMgr",			30,		2,			4*1024,     20*sizeof(T_SYS_MAILBOX)}},
#if (SDRAM_MODE == 8)
    {AKAPP_CLSID_MMI,			{"MMI",				100,	2,			64*1024,   128*sizeof(T_SYS_MAILBOX)}},
#else
	{AKAPP_CLSID_MMI,			{"MMI", 			100,	2,			192*1024,   128*sizeof(T_SYS_MAILBOX)}},
#endif
    {AKAPP_CLSID_MEDIA,			{"Media",			100,	2,			16*1024,   20*sizeof(T_SYS_MAILBOX)}},
    {AKAPP_CLSID_PUBLIC,		{"Public",			50, 	2,			10*1024,   20*sizeof(T_SYS_MAILBOX)}},
    {AKAPP_CLSID_VIDEO,			{"Video",			100,	2,			10*1024,   16*sizeof(T_SYS_MAILBOX)}},
	{AKAPP_CLSID_AUDIO,			{"Audio",			50,		1,			16*1024,   	16*sizeof(T_SYS_MAILBOX)}},
	{AKAPP_CLSID_POWERON,		{"PowerOn",			200,	2,			10*1024,    sizeof(T_SYS_MAILBOX)}},
	

#if (SDRAM_MODE == 8)
	{AKAPP_CLSID_AUDIOLIST,		{"AudioList",		110,		1,			64*1024,   	16*sizeof(T_SYS_MAILBOX)}},
	{AKAPP_CLSID_VIDEOLIST,		{"VideoList",		110,		1,			64*1024,   	16*sizeof(T_SYS_MAILBOX)}},
#else
	{AKAPP_CLSID_AUDIOLIST,		{"AudioList",		110,		1,			192*1024,   	16*sizeof(T_SYS_MAILBOX)}},
	{AKAPP_CLSID_VIDEOLIST,		{"VideoList",		110,		1,			192*1024,   	16*sizeof(T_SYS_MAILBOX)}},
#endif
    {AKAPP_CLSID_NONE,			{"",				0,  	0,			0,          0}}
}; 

//=================================================================================================================
//                                <---- Function Declaration ---->
//=================================================================================================================
#ifdef  AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
extern T_S32 AK_Task_Information(T_hTask task, T_CHR *name, T_OPTION *task_status, T_U32 *scheduled_count,
                                 T_OPTION *priority, T_OPTION *preempt, T_U32 *time_slice,
                                 T_VOID **stack_base, T_U32 *stack_size, T_U32 *minimum_stack);
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
/*================================================================================================================*/
static T_U32    CAppMgr_AddRef(IAppMgr *pIAppMgr);
static T_U32    CAppMgr_Release(IAppMgr *pIAppMgr);
static T_S32    CAppMgr_PostEventEx(IAppMgr *pIAppMgr,T_U16 wAppCls,T_SYS_MAILBOX *pMailBox,
                                            T_pfnEvtCmp pfnCmp,T_BOOL bIsUnique,T_BOOL bIsHead, T_BOOL bIsSuspend);
static IThread* CAppMgr_GetApp(IAppMgr *pIAppMgr,T_U16 wAppCls);
static T_S32    CAppMgr_GetAppCount(IAppMgr *pIAppMgr, T_EAppType eAppType);
static T_S32    CAppMgr_ReportAppsStatus(IAppMgr *pIAppMgr);
#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
static T_VOID   CAppMgr_HisrEntry(T_VOID);
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
#ifdef WIN32
extern void ResetEventxx(void);
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IAppMgr) g_IAppMgrFuncs =
{
    CAppMgr_AddRef,
    CAppMgr_Release,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    CAppMgr_PostEventEx,
    CAppMgr_GetApp,
    CAppMgr_GetAppCount,
    CAppMgr_ReportAppsStatus
};

////////////////////////////////////////////////////////////////////////////////////////////////////
IAppMgr* AK_GetAppMgr(T_VOID)
{
    return g_pIAppMgr;
}

T_AppInfo* AK_GetDefaultAppInfo(T_U16 wAppCls)
{ 
    T_U16 i = 0;
    T_AppInfo *pAppInfo = AK_NULL;

    while (g_sDefaultAppInfo[i].wAppCls != AKAPP_CLSID_NONE)
    {
        if (g_sDefaultAppInfo[i].wAppCls == wAppCls)
        {
            pAppInfo = (T_AppInfo*)&g_sDefaultAppInfo[i];
        }
        i++;
    }

    return pAppInfo;
}
//====================================================================================//

static T_S32 CAppMgr_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);

static const AK_VTABLE(ICBThread) g_ICBAppMgrFuncs =
{
    AK_NULL,
    CAppMgr_ICBThread_Handle,
    AK_NULL
};

//====================================================================================//
static T_S32 CAppMgr_Constructor(CAppMgr *pMe)
{
    //Simple Constructor, Add your code here...
    T_S32 lRet = AK_SUCCESS;
    T_AppInfo *pAppInfo = AK_NULL;
    
    pMe->m_uBGAppNum = 0;
#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    pMe->m_hHisr     = AK_INVALID_HISR;
    pMe->m_HisrStack = AK_NULL;
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    pAppInfo = AK_GetDefaultAppInfo(AKAPP_CLSID_APPMGR);
    if (AK_NULL == pAppInfo)
    {
        return AK_EUNSUPPORT;
    }
    lRet = CThread_New(&pMe->m_pIBase, &pAppInfo->sInitparam);
    lRet |= IThread_Register(pMe->m_pIBase,
                             (ICBThread*)&pMe->m_ICBThread);
    
    lRet |= CVector_New(&pMe->m_pAppList);

#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS  
    pMe->m_HisrStack = Fwl_Malloc(AKAPPMGR_HISR_STACK_SIZE);        
    if (AK_NULL != pMe->m_HisrStack)
    {
        memset(pMe->m_HisrStack, 0, AKAPPMGR_HISR_STACK_SIZE);
        pMe->m_hHisr = AK_Create_HISR(CAppMgr_HisrEntry, "AMHisr", 2, 
                                      pMe->m_HisrStack, AKAPPMGR_HISR_STACK_SIZE);
        if (AK_IS_INVALIDHANDLE(pMe->m_hHisr))
        {
            lRet = AK_EFAILED;
            if (!gs.emtest)
            {
                Fwl_Print(C1, M_AKFRAME, "----AKAppMgr:Create Hisr failed, retuen value=%d.",pMe->m_hHisr);
            }
        }
    }
    else
    {
        lRet = AK_ENOMEMORY;
    }
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    
    return lRet;
}

static T_S32 CAppMgr_Destructor(CAppMgr *pMe)
{
    //Add your code here...
    T_U16 i, wItemNum;
    T_AppEntry *pAppEntry = AK_NULL;
    
    //Free all items first...
    wItemNum = IVector_Size(pMe->m_pAppList);
    for (i = 0; i < wItemNum; i++)
    {
        pAppEntry = (T_AppEntry *)IVector_ElementAt(pMe->m_pAppList, i);
        if (AK_NULL != pAppEntry)
        {
            AK_RELEASEIF(pAppEntry->pIThread);
            pAppEntry = Fwl_Free(pAppEntry);
        }
    }
    
    //Release Vectors...
    AK_RELEASEIF(pMe->m_pAppList);

#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    if (AK_IS_VALIDHANDLE(pMe->m_hHisr))
    {
        AK_Delete_HISR(pMe->m_hHisr);
        pMe->m_hHisr = AK_INVALID_HISR;
    }

    if (AK_NULL != pMe->m_HisrStack)
    {
        pMe->m_HisrStack = Fwl_Free(pMe->m_HisrStack);
    }
#endif // AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS  
    return AK_SUCCESS;
}

static T_U32 CAppMgr_AddRef(IAppMgr *pIAppMgr)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    
    return ++pMe->m_Ref;
}

static T_U32 CAppMgr_Release(IAppMgr *pIAppMgr)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    
    if (--pMe->m_Ref == 0)
    {
        IThread_Register(pMe->m_pIBase,AK_NULL);
        (T_VOID)CAppMgr_Destructor(pMe);
        Fwl_Free(pMe);
        g_pIAppMgr = AK_NULL;
        
        return 0;
    }
    
    return pMe->m_Ref;
}


static T_AppEntry *CAppMgr_GetAppEntry(CAppMgr *pMe, T_U16 wAppCls)
{
    T_U16 i, wItemNum;
    T_AppEntry *pAppEntry = AK_NULL;
    
    wItemNum = IVector_Size(pMe->m_pAppList);
    
    for (i = 0; i < wItemNum; i++)
    {
        pAppEntry = IVector_ElementAt(pMe->m_pAppList, i);
        if ((AK_NULL != pAppEntry) && (pAppEntry->wAppCls == wAppCls))
        {
            break;
        }
        pAppEntry = AK_NULL;
    }
    
    return pAppEntry;
}


static T_AppEntry *CAppMgr_RemoveAppEntry(CAppMgr *pMe, T_U16 wAppCls)
{
    T_U16 i, wItemNum;
    T_AppEntry *pAppEntry = AK_NULL;
    
    wItemNum = IVector_Size(pMe->m_pAppList);
    
    for (i = 0; i < wItemNum; i++)
    {
        pAppEntry = IVector_ElementAt(pMe->m_pAppList, i);
        if ((AK_NULL != pAppEntry) && (pAppEntry->wAppCls == wAppCls))
        {
            pAppEntry = (T_AppEntry*)IVector_RemoveAt(pMe->m_pAppList, i);
            break;
        }
        pAppEntry = AK_NULL;
    }
    
    return pAppEntry;
}



static T_S32 CAppMgr_AddEntry(CAppMgr *pMe,T_AppEntry *pAppEntry, T_BOOL bIsActive)
{
    T_S32 lRet         = AK_SUCCESS;
    
    if (AK_NULL == pAppEntry)
    {
        return AK_EBADPARAM;
    }
    
    /*在应用列表查找是否已有该应用，如果存在，则返回错误*/
    if (AK_NULL != CAppMgr_GetAppEntry(pMe, pAppEntry->wAppCls))
    { 
        Fwl_Free(pAppEntry);
        return AK_EEXISTED; //Already exist.
    }
    
    /*判断是否是后台应用，如果是，则插入应用列表头并将后台应用计数加1*/
    if (IThread_IsBackground(pAppEntry->pIThread))
    {
        lRet = IVector_InsertAt(pMe->m_pAppList, 0, (T_VOID*)pAppEntry);
        pMe->m_uBGAppNum++;
    }
    else
    {
        /*如果是前台应用，则判断是否需要激活，如果要激活，则激活它*/
        if (bIsActive)
        {
            lRet = IVector_InsertAt(pMe->m_pAppList, pMe->m_uBGAppNum, (T_VOID*)pAppEntry);
        }
        else
        {
            lRet = IVector_AddElement(pMe->m_pAppList, (T_VOID*)pAppEntry);
        }      
    }
    
    /*如果添加应用成功，则运行该应用程序*/
    if (AK_IS_SUCCESS(lRet))
    { 
        lRet = IThread_Run(pAppEntry->pIThread);
    }
    else
    {
        Fwl_Free(pAppEntry);
    }

#ifdef WIN32
    ResetEventxx();
#endif
    return lRet;
}


static T_S32 CAppMgr_DeactiveApp(CAppMgr *pMe,T_U16 wAppCls)
{
    T_S32 lRet = AK_SUCCESS;
    T_AppEntry *pAppEntry = AK_NULL;
    
    /*获取当前激活的应用，如果指定的应用为当前激活应用，则去激活它*/
    pAppEntry = (T_AppEntry*)IVector_ElementAt(pMe->m_pAppList, pMe->m_uBGAppNum);
    if ((AK_NULL != pAppEntry) && (pAppEntry->wAppCls == wAppCls))
    {
        pAppEntry = (T_AppEntry*)IVector_RemoveAt(pMe->m_pAppList, pMe->m_uBGAppNum);
        lRet = IVector_AddElement(pMe->m_pAppList, (T_VOID*)pAppEntry);
    }
    
    return lRet;   
}

static T_S32 CAppMgr_ActiveApp(CAppMgr *pMe,T_U16 wAppCls)
{
    T_S32 lRet = AK_SUCCESS;
    T_U16 i, wItemNum;
    T_AppEntry *pAppEntry = AK_NULL;
    
    wItemNum = IVector_Size(pMe->m_pAppList);
    
    /*在前台应用里查找指定的应用*/
    for (i = pMe->m_uBGAppNum; i < wItemNum; i++)
    {
        pAppEntry = (T_AppEntry*)IVector_ElementAt(pMe->m_pAppList, i);
        if ((AK_NULL != pAppEntry) && (pAppEntry->wAppCls == wAppCls))
        {
            break;
        }
        pAppEntry = AK_NULL;
    }
    
    /*如果找到并且该应用不是激活应用，则激活该应用*/
    if (i > pMe->m_uBGAppNum && i < wItemNum)
    {
        pAppEntry = (T_AppEntry*)IVector_RemoveAt(pMe->m_pAppList, i);
        lRet = IVector_InsertAt(pMe->m_pAppList, pMe->m_uBGAppNum, (T_VOID*)pAppEntry);
    }
    
    return lRet;   
}


static T_S32 CAppMgr_DeleteEntry(CAppMgr *pMe, T_U16 wAppCls)
{
    T_S32 lRet            = AK_SUCCESS;
    T_AppEntry *pAppEntry = AK_NULL;
    
    /*在应用列表里查找指定的应用*/
    pAppEntry = CAppMgr_RemoveAppEntry(pMe, wAppCls);
    if (AK_NULL != pAppEntry)
    {
        //First Unregister the system event...
        IMsgDispatch_UnRegister(AK_GetIMsgDispatch(), pAppEntry->pIThread);
        if (IThread_IsBackground(pAppEntry->pIThread))
        {
            pMe->m_uBGAppNum--;
        }
        
        lRet = IThread_Exit(pAppEntry->pIThread);
        pAppEntry = Fwl_Free(pAppEntry);
    }
    
    return lRet;   
}

static T_BOOL CAppMgr_DefaultEventCmpCB(T_SYS_MAILBOX *pMailBox1, T_SYS_MAILBOX *pMailBox2)
{
    if (pMailBox1->event == pMailBox2->event)
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

static T_S32 CAppMgr_PostEventEx(IAppMgr *pIAppMgr,T_U16 wAppCls,T_SYS_MAILBOX *pMailBox,
                                 T_pfnEvtCmp pfnCmp,T_BOOL bIsUnique,T_BOOL bIsHead, T_BOOL bIsSuspend)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    T_AppEntry *pAppEntry = AK_NULL;
    T_U32 ulSuspend       = AK_NO_SUSPEND;
    T_S32 lRet            = AK_SUCCESS;

    if (AK_NULL == pMailBox)
    {
        return AK_EBADPARAM;
    }
    
    pAppEntry = CAppMgr_GetAppEntry(pMe, wAppCls);
    if (AK_NULL != pAppEntry)
    {
        if (bIsSuspend)
        {
            ulSuspend = AK_SUSPEND;
        }
        
        if ((AK_TRUE == bIsUnique) && (AK_NULL == pfnCmp))
        {
            pfnCmp = (T_pfnEvtCmp)CAppMgr_DefaultEventCmpCB;
        }

        if (bIsUnique && bIsHead)
        {
            lRet = AK_Send_Unique_To_Front_of_Queue(IThread_GetQueue(pAppEntry->pIThread), 
                                                    pMailBox, sizeof(T_SYS_MAILBOX), 
                                                    ulSuspend, pfnCmp);
        }
        else if (bIsUnique)
        {
            lRet = AK_Send_Unique_To_Queue(IThread_GetQueue(pAppEntry->pIThread), 
                                           pMailBox, sizeof(T_SYS_MAILBOX),
                                           ulSuspend, pfnCmp);
        }
        else if (bIsHead)
        {
            lRet = AK_Send_To_Front_of_Queue(IThread_GetQueue(pAppEntry->pIThread),
                                             pMailBox, sizeof(T_SYS_MAILBOX),
                                             ulSuspend);
        }
        else
        {
            lRet = AK_Send_To_Queue(IThread_GetQueue(pAppEntry->pIThread), 
                                    pMailBox, sizeof(T_SYS_MAILBOX), 
                                    ulSuspend);
        }
    }
    
    if (AK_IS_FAILURE(lRet) && !gs.emtest)
    {
        //Fwl_Print(C2, M_AKFRAME, "--CAppMgr_PostEventEx Failed,Task is:%s, event=0x%x, lRet=%d.", 
        //            IThread_GetName(pAppEntry->pIThread), pMailBox->event, lRet);
        
        //CAppMgr_HisrEntry();
    }
    
    return lRet;   
}


static IThread* CAppMgr_GetApp(IAppMgr *pIAppMgr,T_U16 wAppCls)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    T_AppEntry *pAppEntry = AK_NULL;
    
    if (AKAPP_CLSID_ACTIVE == wAppCls)
    {
        pAppEntry = (T_AppEntry*)IVector_ElementAt(pMe->m_pAppList, pMe->m_uBGAppNum);
    }
    else
    {
        pAppEntry = CAppMgr_GetAppEntry(pMe, wAppCls);
    }
    
    if (AK_NULL != pAppEntry)
    {
        return pAppEntry->pIThread; //Don't add its refrence count...
    }
    
    return AK_NULL;   
}

static T_S32 CAppMgr_GetAppCount(IAppMgr *pIAppMgr, T_EAppType eAppType)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    T_S32 lAppCount = 0;

    if (AKAPP_FG_APP == eAppType)
    {
        lAppCount = (T_S32)(IVector_Size(pMe->m_pAppList) - pMe->m_uBGAppNum);
    }
    else if (AKAPP_BG_APP == eAppType)
    {
        lAppCount = (T_S32)pMe->m_uBGAppNum;
    }
    else
    {
        lAppCount = (T_S32)IVector_Size(pMe->m_pAppList);
    }

    return lAppCount;
}


static T_S32 CAppMgr_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    CAppMgr *pMe = (CAppMgr *)pICBThread->pData;
    T_S32 lRet = AK_SUCCESS;
    
//    Fwl_Print("---I'm in CAppMgr_ICBThread_Handle.eEvent=%d.\r\n", eEvent);
    switch (eEvent)
    {
        case APPMGR_EVT_ADDENTRY:
            {
                lRet = CAppMgr_AddEntry(pMe, (T_AppEntry*)pEvtParam->w.Param1, 
                                       (T_BOOL)pEvtParam->w.Param2);
            }
            break;
        case APPMGR_EVT_ACTIVEAPP:
            {
                lRet = CAppMgr_ActiveApp(pMe, (T_U16)pEvtParam->w.Param1);
            }
            break;
        case APPMGR_EVT_DEACTIVEAPP:
            {
                lRet = CAppMgr_DeactiveApp(pMe, (T_U16)pEvtParam->w.Param1);
            }
            break;
        case APPMGR_EVT_DELETEENTRY:
            {
                lRet = CAppMgr_DeleteEntry(pMe, (T_U16)pEvtParam->w.Param1);
            }
            break;
        default:
            Fwl_Print(C2, M_AKFRAME, "Unhandled in AppMgr!");
            break;
    }
    
    return lRet;
}

#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
static T_VOID CAppMgr_HisrEntry(T_VOID)
{
    CAppMgr *pMe = (CAppMgr *)AK_GetAppMgr()->pData;
    T_U16 i, wAppNum;
    T_AppEntry *pAppEntry  = AK_NULL;
    T_CHR   szTaskName[16] = {0};
    T_OPTION byTaskStatus  = 0;
    T_U32 dwScheduledCount = 0;
    T_OPTION byPriority    = 0;
    T_OPTION byPreempt     = 0;
    T_U32    dwTimeSlice   = 0;
    T_VOID   *pStackBase   = AK_NULL;
    T_U32    dwStackSize   = 0;
    T_U32 dwStackNotUsed   = 0;
    
    if (!gs.emtest)
    {
        wAppNum = IVector_Size(pMe->m_pAppList);
        Fwl_Print(C3, M_AKFRAME, "===========Report Apps Status Begin:App Num=%d.===========",wAppNum);
        
        for (i=0; i<wAppNum; i++)
        {
            pAppEntry = (T_AppEntry*)IVector_ElementAt(pMe->m_pAppList, i);
            if (AK_NULL != pAppEntry)
            {
                AK_Task_Information(IThread_GetTask(pAppEntry->pIThread), szTaskName, &byTaskStatus, 
                                    &dwScheduledCount, &byPriority, &byPreempt, &dwTimeSlice,
                                    &pStackBase, &dwStackSize, &dwStackNotUsed);
                Fwl_Print(C3, M_AKFRAME, "App Index:%d, Name is:%s, Priority=%d, Time_Slice=%d, Status=%d.",i, IThread_GetName(pAppEntry->pIThread),
                           byPriority, dwTimeSlice, byTaskStatus);
                Fwl_Print(C3, M_AKFRAME, "    Stack Address=0x%x, Stack size=%d, Stack not used size=%d.",
                           pStackBase, dwStackSize, dwStackNotUsed);
            }
        }
        Fwl_Print(C3, M_AKFRAME, "=============Report Apps Status End=============");
    }    
}
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS

static T_S32 CAppMgr_ReportAppsStatus(IAppMgr *pIAppMgr)
{
    CAppMgr *pMe = (CAppMgr *)pIAppMgr->pData;
    T_U16 i, wAppNum;
    T_AppEntry *pAppEntry  = AK_NULL;
  
#ifdef AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    if (AK_IS_VALIDHANDLE(pMe->m_hHisr))
    {
        AK_Activate_HISR(pMe->m_hHisr);
    }
    else
#endif //AKAPPMGR_ENABLE_REPORT_DETAIL_APPS_STATUS
    {        
        if (!gs.emtest)
        {
            wAppNum = IVector_Size(pMe->m_pAppList);
            Fwl_Print(C3, M_AKFRAME, "===========Report Apps Status Begin:App Num=%d.===========",wAppNum);
            
            for (i=0; i<wAppNum; i++)
            {
                pAppEntry = (T_AppEntry*)IVector_ElementAt(pMe->m_pAppList, i);
                if (AK_NULL != pAppEntry)
                {
                    Fwl_Print(C3, M_AKFRAME, "App Index:%d, Name is:%s, Status=%d.",i, IThread_GetName(pAppEntry->pIThread),
                               IThread_GetState(pAppEntry->pIThread));
                }
            }
            Fwl_Print(C3, M_AKFRAME, "=============Report Apps Status End=============");
        }
    }
    
    return AK_SUCCESS;
}


T_S32 CAppMgr_New(IAppMgr **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    CAppMgr *pNew = AK_NULL;
    
    if (AK_NULL == ppi)
    {
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;
    if (AK_NULL != g_pIAppMgr)
    {
        *ppi =   g_pIAppMgr;
        AK_ADDREFIF(g_pIAppMgr);
        
        return AK_SUCCESS;
    }
    
    do 
    {
        pNew = AK_MALLOCRECORD(CAppMgr);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        memset(pNew, 0, sizeof(CAppMgr));
        
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_ICBThread), &g_ICBAppMgrFuncs);
        pNew->m_ICBThread.pData = (T_VOID*)pNew;
        AK_SETVT(&(pNew->m_myIAppMgr), &g_IAppMgrFuncs);
        pNew->m_myIAppMgr.pData = (T_VOID*)pNew;
        
        nErr = CAppMgr_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CAppMgr_Release(&pNew->m_myIAppMgr);
            pNew = AK_NULL;
            break;
        }
        
        *ppi = (IAppMgr *)&pNew->m_myIAppMgr;
        g_pIAppMgr = *ppi;
    } while(AK_FALSE);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

