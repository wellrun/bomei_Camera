/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAppMgr.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKAPPMGR_H__
#define __AKAPPMGR_H__

#include "AKComponent.h"
#include "AKInterface.h"
#include "anyka_types.h"
#include "Fwl_sysevent.h"
#include "AKThread.h"
#include "AKSubThread.h"
#include "AKVector.h"
#include "AKFrameStream.h"
#include "Fwl_pfDisplay.h"
#include "Akos_api.h"

///////////////////////////////////////////////////////////////////////////
/*应用程序ID枚举定义，值为16位的整型*/
enum
{
  AKAPP_CLSID_NONE    = 0x0000,
  AKAPP_CLSID_APPMGR  = 0x0001,    /*应用管理器*/
  AKAPP_CLSID_MMI,                 /*MMI应用*/
  AKAPP_CLSID_MEDIA,               /*流媒体任务*/
  AKAPP_CLSID_DECODEIMAGE,         /*图片解码任务*/
  AKAPP_CLSID_TIMERHANDLE,         /* 虚拟timer线程 */ 
  AKAPP_CLSID_GLONAV,              /* GLONAV GPS 线程 */ 
  AKAPP_CLSID_EMAP,                /* CARELAND EMAP线程 */
  AKAPP_CLSID_PUBLIC,              /* 公共后台线程*/
  AKAPP_CLSID_VIDEO,               /* Video Decoder */
  AKAPP_CLSID_AUDIO,               /* Audio Decoder */
  AKAPP_CLSID_POWERON,		   	   /* PowerOn Thread */
  AKAPP_CLSID_AUDIOLIST,           /* Audio List */
  AKAPP_CLSID_VIDEOLIST,           /* Video List */
  AKAPP_CLSID_XXX,                 /*待添加任务*/
  
  AKAPP_CLSID_ACTIVE  = 0xffff     /*当前激活的应用*/
};

/*应用管理器内部事件定义*/
enum 
{
    APPMGR_EVT_ADDENTRY = 1,
    APPMGR_EVT_ACTIVEAPP,
    APPMGR_EVT_DEACTIVEAPP,
    APPMGR_EVT_DELETEENTRY
};

/*应用基本信息定义*/
typedef struct 
{
    T_U16              wAppCls;     /*应用程序的ID*/
    T_THREAD_INITPARAM sInitparam;  /*应用程序初始化信息*/
}T_AppInfo;

/*应用程序记录定义*/
typedef struct tagAppEntry
{
    T_U16       wAppCls;    /*应用程序的ID*/
    IThread    *pIThread;   /*应用程序实例接口指针*/
}T_AppEntry;

/*应用程序类型定义*/
typedef enum
{
   AKAPP_FG_APP,
   AKAPP_BG_APP,
   AKAPP_ALL_APP
}T_EAppType;

///////////////////////////////////////////////////////////////////////////
typedef struct IAppMgr IAppMgr;

#define AK_INHERIT_IAPPMGR(name) \
      AK_INHERIT_ITHREAD(name); \
      T_S32    (*PostEventEx)(name* p##name, T_U16 wAppCls, T_SYS_MAILBOX *pMailBox,T_pfnEvtCmp pfnCmp,T_BOOL bIsUnique, T_BOOL bIsHead, T_BOOL bIsSuspend); \
      IThread* (*GetApp)(name* p##name, T_U16 wAppCls); \
      T_S32    (*GetAppCount)(name* p##name, T_EAppType eAppType); \
      T_S32    (*ReportAppsStatus)(name* p##name)


AK_INTERFACE(IAppMgr)
{
    AK_INHERIT_IAPPMGR(IAppMgr);
};

struct IAppMgr
{
    AK_DEFINEVT(IAppMgr);
    T_VOID *pData;
};

/*************************************************************************************************************/
/**
###############################################################################################################
##                                     组件描述
##   本组件为应用管理器组件，是一个单实例组件。主要用于应用程序的管理，它本身又是一个任务程序，负责应用程序的
## 添加运行、删除、前台应用的激活和去激活，以及任务间消息发送。
###############################################################################################################
===============================================================================================================
Function:      IAppMgr * AK_GetAppMgr(T_VOID);
Description:   获取应用管理器实例接口指针。
Parameters:    None
Return:  返回应用管理器实例接口指针。
Remark:
    注: 没有增加该实例的引用计数，故无需释放它。
===============================================================================================================
===============================================================================================================
Function:      T_AppInfo* AK_GetDefaultAppInfo(T_U16 wAppCls);
Description:   获取指定应用程序默认初始化信息。
Parameters:
   wAppCls   [in] 应用程序ID。
Return:  返回应用程序默认初始化信息，若指定的应用程序ID为非法的，则返回AK_NULL。
Remark:
   调用此接口前，请确保该应用程序默认初始化信息已添加进g_sDefaultAppInfo里。
===============================================================================================================
===============================================================================================================
Function:      T_U32 IAppMgr_AddRef(IAppMgr *pIAppMgr);
Description:   引用计数加1。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_U32 IAppMgr_Release(IAppMgr *pIAppMgr);
Description:   引用计数减1，如果引用计数为0，则释放该组件实例。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAppMgr_AddEntry(IAppMgr *pIAppMgr,T_AppEntry *pAppEntry, T_BOOL bIsActive);
Description:   向应用管理器添加一个应用记录，新建的一个应用必须要加入应用管理器，应用管理器会
               自动运行该应用程序。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   pAppEntry  [in] 指向要添加应用程序记录的指针。
   bIsActive  [in] 是否让当前添加的应用成为激活应用，此参数只对前台应用有效。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    添加一个应用流程记录：
    T_AppEntry AppEntry;
    IApp *pIApp = AK_NULL;
    T_S32 lRet  = AK_SUCCESS;

    lRet = CMMI_New(&pIApp);
    if (AK_IS_SUCCESS(lRet))
    {
        AppEntry.wAppCls  = AKAPP_CLSID_MMI;
        AppEntry.pIThread = (IThread*)pIApp;
        lRet = IAppMgr_AddEntry(pIAppMgr, &AppEntry, AK_TRUE);
    }    
===============================================================================================================
===============================================================================================================
Function:      T_S32  IAppMgr_DeleteEntry(IAppMgr *pIAppMgr,T_U16 wAppCls);
Description:   将指定的应用程序从任务列表中删除，并销毁该应用程序。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   删除一个应用程序:
   lRet = IAppMgr_DeleteEntry(pIAppMgr, AKAPP_CLSID_MMI);
===============================================================================================================
===============================================================================================================
Function:      T_S32  IAppMgr_ActiveApp(IAppMgr *pIAppMgr,T_U16 wAppCls);
Description:   激活指定的前台应用程序，如果指定的是一个后台应用，则不做任何操作。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   激活应用是一个可以接收输入事件(键盘和触摸屏等)和绘屏的前台应用。
   注:目前绘屏控制还没有做完善。
===============================================================================================================
===============================================================================================================
Function:      T_S32  IAppMgr_DeactiveApp(IAppMgr *pIAppMgr,T_U16 wAppCls);
Description:   去激活指定的前台应用程序，如果指定的是一个后台应用，则不做任何操作。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32  IAppMgr_PostEventEx(IAppMgr *pIAppMgr,T_U16 wAppCls,
                                          T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                                          T_BOOL bIsUnique, T_BOOL bIsHead,
                                          T_BOOL bIsSuspend);
Description:   向指定的应用队列发送一个消息，如果bIsHead为AK_TRUE，则将消息放在队列的头，否则放在队列
               尾，如果bIsSuspend为AK_TRUE，则指定应用队列满时，发送消息的任务将被挂起。
Parameters:
   pIAppMgr    [in] 指向该组件实例接口的指针。
   wAppCls     [in] 应用程序ID。
   pMailBox    [in] 指向消息结构体的指针。
   pfnCmp      [in] 消息比较函数，只有bIsUnique为AK_TRUE时有效。
   bIsUnique  [in] 标识是否要在队列里唯一。
   bIsHead     [in] 标识是否要放在队列头。
   bIsSuspend  [in] 对方应用队列满时，发送方是否要被挂起。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
   该接口函数只能在高级中断服务程序和应用程序里被调用，且当被高级中断服务程序调用时，bIsSuspend必须
   被设置为AK_FALSE，否则发送将不成功。
===============================================================================================================
===============================================================================================================
Function:      T_S32  IAppMgr_PostEvent(IAppMgr *pIAppMgr,T_U16 wAppCls, T_SYS_MAILBOX *pMailBox);
Description:   向指定的应用队列发送一个消息。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
   pMailBox   [in] 指向消息结构体的指针。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
   该接口函数只能在高级中断服务程序和应用程序里被调用，且消息被放在应用队列尾。
===============================================================================================================
===============================================================================================================
Function:      IThread*  IAppMgr_GetApp(IAppMgr *pIAppMgr,T_U16 wAppCls);
Description:   获取应用程序ID指定的应用程序接口指针。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   wAppCls    [in] 应用程序ID。
Return:  返回指定的应用程序接口指针，如果该应用不存在或没有被创建，则返回AK_NULL。
Remark:
   IThread* IAppMgr_GetActiveApp(IAppMgr *pIAppMgr);
   DES:返回当前激活的应用程序接口指针。
注: IThread为所有应用程序的父接口。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAppMgr_GetAppCount(IAppMgr *pIAppMgr, T_EAppType eAppType);
Description:   获取指定类型的应用程序个数。
Parameters:
   pIAppMgr   [in] 指向该组件实例接口的指针。
   eAppType    [in] 应用程序类型。
Return:  返回指定类型的应用程序个数，如果传入的参数错误，则返回0。
Remark:
===============================================================================================================

*/
/*************************************************************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
T_S32 CAppMgr_New(IAppMgr **ppi);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
IAppMgr * AK_GetAppMgr(T_VOID);
T_AppInfo* AK_GetDefaultAppInfo(T_U16 wAppCls);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Everry ext must realize these two function...
#define IAppMgr_AddRef(p)               AK_GETVT(IAppMgr, p)->AddRef(p)
#define IAppMgr_Release(p)              AK_GETVT(IAppMgr, p)->Release(p)
#define IAppMgr_PostEvent(p,c,e)        IAppMgr_PostEventEx(p,c,e,AK_NULL,AK_FALSE,AK_FALSE,AK_FALSE)
#define IAppMgr_GetActiveApp(p)         IAppMgr_GetApp(p,AKAPP_CLSID_ACTIVE)
#define IAppMgr_PostEvt2Head(p,c,e)     IAppMgr_PostEventEx(p,c,e,AK_NULL,AK_FALSE,AK_TRUE,AK_FALSE)
#define IAppMgr_PostUniqueEvt(p,c,e)    IAppMgr_PostEventEx(p,c,e,AK_NULL,AK_TRUE,AK_FALSE,AK_FALSE)
#define IAppMgr_PostUniqueEvt2Head(p,c,e) IAppMgr_PostEventEx(p,c,e,AK_NULL,AK_TRUE,AK_TRUE,AK_FALSE)




__inline IThread* IAppMgr_GetApp(IAppMgr *pIAppMgr,T_U16 wAppCls)
{
    if (AK_NULL == pIAppMgr)
    {
        return AK_NULL;
    }

    return AK_GETVT(IAppMgr, pIAppMgr)->GetApp(pIAppMgr,wAppCls);
}

__inline T_S32 IAppMgr_GetAppCount(IAppMgr *pIAppMgr, T_EAppType eAppType)
{
    if (AK_NULL == pIAppMgr)
    {
        Fwl_Print(C1, M_AKFRAME, "--IAppMgr_GetAppCount failed, AK_NULL == pIAppMgr.");
        return 0;
    }

    return AK_GETVT(IAppMgr, pIAppMgr)->GetAppCount(pIAppMgr,eAppType);
}

__inline T_S32 IAppMgr_PostEventEx(IAppMgr *pIAppMgr,T_U16 wAppCls, 
                                          T_SYS_MAILBOX *pMailBox, T_pfnEvtCmp pfnCmp, 
                                          T_BOOL bIsUnique,T_BOOL bIsHead,T_BOOL bIsSuspend)
{
    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }

    return AK_GETVT(IAppMgr, pIAppMgr)->PostEventEx(pIAppMgr,wAppCls,pMailBox,
                                                    pfnCmp,bIsUnique,bIsHead,bIsSuspend);
}

__inline T_S32 IAppMgr_AddEntry(IAppMgr *pIAppMgr,T_AppEntry *pAppEntry, T_BOOL bIsActive)
{
    T_AppEntry *pEntry = AK_NULL;
    T_SYS_MAILBOX  mailbox;

    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }

    /*在应用列表查找是否已有该应用，如果存在，则返回错误*/
    if (AK_NULL != IAppMgr_GetApp(pIAppMgr, pAppEntry->wAppCls))
    {
        return AK_EEXISTED;
    }

    /*为当前应用程序记录分配空间，并复制信息*/
    pEntry = (T_AppEntry*)Fwl_Malloc(sizeof(T_AppEntry));
    if (AK_NULL == pEntry)
    {
         return AK_ENOMEMORY;
    }

    pEntry->wAppCls  = pAppEntry->wAppCls;
    pEntry->pIThread = pAppEntry->pIThread; 

    mailbox.event = APPMGR_EVT_ADDENTRY;
    mailbox.param.w.Param1 = (vUINT32)pEntry;
    mailbox.param.w.Param2 = (vUINT32)bIsActive;

    return AK_Send_To_Queue(IThread_GetQueue((IThread*)pIAppMgr), &mailbox, 
                            sizeof(T_SYS_MAILBOX), AK_NO_SUSPEND);
}


__inline T_S32  IAppMgr_ActiveApp(IAppMgr *pIAppMgr,T_U16 wAppCls)
    
{
    IThread *pIThread = AK_NULL;
    T_SYS_MAILBOX  mailbox;

    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }
    
    pIThread = IAppMgr_GetApp(pIAppMgr, wAppCls);
    /*如果此应用不存在，则返回错误*/
    if (AK_NULL == pIThread)
    {
        return AK_ENOTFOUND;
    }
    /*如果此应用是后台应用或已经激活，则返回错误*/
    if (IThread_IsBackground(pIThread) || 
        (pIThread == IAppMgr_GetActiveApp(pIAppMgr)))
    {
        return AK_EINVALIDOPT;
    }
    
    mailbox.event = APPMGR_EVT_ACTIVEAPP;
    mailbox.param.w.Param1 = (vUINT32)wAppCls;

    return AK_Send_To_Queue(IThread_GetQueue((IThread*)pIAppMgr), &mailbox, 
                            sizeof(T_SYS_MAILBOX), AK_NO_SUSPEND);
}


__inline T_S32  IAppMgr_DeactiveApp(IAppMgr *pIAppMgr,T_U16 wAppCls)
    
{
    IThread *pIThread = AK_NULL;
    T_SYS_MAILBOX  mailbox;

    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }

    pIThread = IAppMgr_GetApp(pIAppMgr, wAppCls);
    /*如果此应用不存在，则返回错误*/
    if (AK_NULL == pIThread)
    {
        return AK_ENOTFOUND;
    }
    /*如果此应用是后台应用或非激活或没有其它前台应用，则返回错误*/
    if (IThread_IsBackground(pIThread) || 
        (pIThread != IAppMgr_GetActiveApp(pIAppMgr)) ||
        (IAppMgr_GetAppCount(pIAppMgr, AKAPP_FG_APP) <= 1))
    {
        return AK_EINVALIDOPT;
    }
    
    mailbox.event = APPMGR_EVT_DEACTIVEAPP;
    mailbox.param.w.Param1 = (vUINT32)wAppCls;

    return AK_Send_To_Queue(IThread_GetQueue((IThread*)pIAppMgr), &mailbox, 
                            sizeof(T_SYS_MAILBOX), AK_NO_SUSPEND);
}


__inline T_S32  IAppMgr_DeleteEntry(IAppMgr *pIAppMgr,T_U16 wAppCls)
    
{
    IThread *pIThread = AK_NULL;
    T_SYS_MAILBOX  mailbox;

    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }

    pIThread = IAppMgr_GetApp(pIAppMgr, wAppCls);
    /*如果此应用不存在，则返回错误*/
    if (AK_NULL == pIThread)
    {
        return AK_ENOTFOUND;
    }
    
    mailbox.event = APPMGR_EVT_DELETEENTRY;
    mailbox.param.w.Param1 = (vUINT32)wAppCls;

    return AK_Send_To_Queue(IThread_GetQueue((IThread*)pIAppMgr), &mailbox,
                            sizeof(T_SYS_MAILBOX), AK_NO_SUSPEND);
}


__inline T_S32  IAppMgr_ReportAppsStatus(IAppMgr *pIAppMgr)
{
    if (AK_NULL == pIAppMgr)
    {
        return AK_EBADPARAM;
    }

    return AK_GETVT(IAppMgr, pIAppMgr)->ReportAppsStatus(pIAppMgr);
}


#endif //__AKAPPMGR_H__

