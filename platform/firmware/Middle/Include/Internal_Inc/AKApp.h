/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKAPP_H__
#define __AKAPP_H__

#include "AKComponent.h"
#include "AKInterface.h"
#include "AKThread.h"
#include "AKWnd.h"

////////////////////////////////////////////////////////////////////////////////////////////
#define AKAPP_PROP_BASE    (THREAD_PROP_BASE + AKAPP_PROP_OFFSET)

/*应用属性枚举定义*/
typedef enum
{
  AKAPP_PROP_NONE = AKAPP_PROP_BASE,
  AKAPP_PROP_INTER_WND
  
}T_eAKAPP_PROPERTY;

/*应用初始化信息定义*/
typedef struct 
{
  T_THREAD_INITPARAM  base;  /*线程初始化信息*/
  T_WND_INITPARAM     wnd;   /*窗体初始化信息*/
}T_APP_INITPARAM;


//==============================================================================
typedef struct IApp IApp;

#define AK_INHERIT_IAPP(name) \
        AK_INHERIT_ITHREAD(name)


AK_INTERFACE(IApp)
{
    AK_INHERIT_IAPP(IApp);
};

//==============================================================================
struct IApp
{
    AK_DEFINEVT(IApp);
    T_VOID *pData;
};

/*************************************************************************************************************/
/**
###############################################################################################################
##                                     组件描述
##   本组件为应用组件，是多实例组件，继承了IThread接口。主要负责窗体的创建和销毁等，而任务的创建、销毁和
## 消息队列的创建、消息循环和销毁以及掩码设置都是委托给它的父类线程处理的。
###############################################################################################################
===============================================================================================================
Function:      T_U32 IAkApp_AddRef(IAkApp *pIAkApp);
Description:   引用计数加1。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_U32 IAkApp_Release(IAkApp *pIAkApp);
Description:   引用计数减1，如果引用计数为0，则释放该组件实例。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_Run(IAkApp *pIAkApp);
Description:   运行指定应用，如果指定应用处于Terminate状态，则先会重置应用，然后重新运行。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_Suspend(IAkApp *pIAkApp);
Description:   挂起指定应用。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_Terminate(IAkApp *pIAkApp);
Description:   终止指定应用。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
   wAppCls   [in] 应用程序ID。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    不能在当前应用里终止自己。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_Exit(IAkApp *pIAkApp);
Description:   退出指定应用，会先终止该应用、删除该应用，然后释放该应用程序实例。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
    不能在当前应用里退出自己。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_Register(IAkApp *pIAkApp, ICBThread *pIListener);
Description:   向指定应用注册一个应用程序回调接口。
Parameters:
   pIAkApp    [in] 指向该组件实例接口的指针。
   pIListener [in] 指向应用程序回调接口的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   如果应用程序执行构造函数失败，则需要取消注册，即注册一个空指针AK_NULL。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_SetProperty(IAkApp *pIAkApp, T_U16 wPropID, T_VOID*  pPropData);
Description:   设置指定应用属性。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
   wPropID   [in] 属性类别ID。
   pPropData [in] 指向属性内存块的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   T_S32 IAkApp_SetEvtMsk(IAkApp *pIAkApp, T_U32 ulEvtMsk);
   DES:设置指定应用的事件掩码。
===============================================================================================================
===============================================================================================================
Function:      T_S32 IAkApp_GetProperty(IAkApp *pIAkApp, T_U16 wPropID, T_VOID*  pPropData);
Description:   获取指定应用属性。
Parameters:
   pIAkApp   [in] 指向该组件实例接口的指针。
   wPropID   [in] 属性类别ID。
   pPropData [in/out] 指向属性内存块的指针。
Return:  返回操作是否成功，如果为AK_SUCCESS，则表示操作成功，其它值表示操作失败，并
         对应于操作失败的原因值。
Remark:
   T_U32 IAkApp_GetEvtMsk(IAkApp *pIAkApp);
   DES:获取指定应用的事件掩码。
   T_hTask IAkApp_GetTask(IAkApp *pIAkApp);
   DES:获取指定应用的内部任务。
   T_hQueue IAkApp_GetQueue(IAkApp *pIAkApp);
   DES:获取指定应用的内部队列。
   T_S32 IAkApp_GetState(IAkApp *pIAkApp);
   DES:获取指定应用的当前状态。
   IWnd * IAkApp_GetIWnd(IApp *pIApp);
   DES:获取指定应用的内部窗体指针。
===============================================================================================================
*/
/*************************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

T_S32     CApp_New(IApp **ppi, T_APP_INITPARAM *param);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define IAkApp_AddRef(p)                     IThread_AddRef((IThread*)p)
#define IAkApp_Release(p)                    IThread_Release((IThread*)p)
#define IAkApp_Run(p)                        IThread_Run((IThread*)p)
#define IAkApp_Suspend(p)                    IThread_Suspend((IThread*)p)
#define IAkApp_Terminate(p)                  IThread_Terminate((IThread*)p)
#define IAkApp_Exit(p)                       IThread_Exit((IThread*)p)
#define IAkApp_Register(p,l)                 IThread_Register((IThread*)p,l)
#define IAkApp_SetProperty(p,i,v)            IThread_SetProperty((IThread*)p,i,v)
#define IAkApp_GetProperty(p,i,pp)           IThread_GetProperty((IThread*)p,i,pp)
#define IAkApp_SetEvtMsk(p,m)                IThread_SetEvtMsk((IThread*)p,m)
#define IAkApp_GetEvtMsk(p)                  IThread_GetEvtMsk((IThread*)p)
#define IAkApp_GetTask(p)                    IThread_GetTask((IThread*)p)
#define IAkApp_GetQueue(p)                   IThread_GetQueue((IThread*)p)

__inline IWnd * IAkApp_GetIWnd(IApp *pIApp)
{
    IWnd * pIWnd = AK_NULL;

    IAkApp_GetProperty(pIApp, AKAPP_PROP_INTER_WND, (T_VOID*)&pIWnd); 

    return pIWnd;
}

#endif //__AKAPP_H__

