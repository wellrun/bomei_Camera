/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKMsgDispatch.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKMSGDISPATCH_H__
#define __AKMSGDISPATCH_H__

#include "AKComponent.h"
#include "AKInterface.h"
#include "AKThread.h"
#include "Fwl_sysevent.h"


/**************************************************************************************************************/
/*######################################################################################*/
/*Msg register type defination...*/
typedef enum
{
   MSG_REG_QUEUE,
   MSG_REG_THREAD
}T_eMsgRegType;

/*######################################################################################*/
/**************************************************************************************************************/

typedef struct IMsgDispatch IMsgDispatch;

/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_IMSGDISPATCH(name) \
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
#define AK_INHERIT_IMSGDISPATCH(name) \
      AK_INHERIT_IUNKNOWN(name); \
	  T_S32 (*Activate)(name* p##name); \
	  T_S32 (*Register)(name* p##name, T_HANDLE hRegister, T_eMsgRegType eRegType, T_U32 dwEvtMsk); \
	  T_S32 (*UnRegister)(name* p##name, T_HANDLE hRegister)
	  

AK_INTERFACE(IMsgDispatch)
{
    AK_INHERIT_IMSGDISPATCH(IMsgDispatch);
};

struct IMsgDispatch
{
    AK_DEFINEVT(IMsgDispatch);
    T_VOID *pData;
};

/**************************************************************************************************************/
/**
###############################################################################################################
##                                     组件描述
##   本组件为消息分发器组件，是一个单实例组件。主要用于系统消息的分发，它含有一个高级中断服务，负责分发消息。
##   它同时给应用程序提供注册和取消注册系统事件的接口，可以随时随地的注册和取消注册系统事件。
###############################################################################################################
===============================================================================================================
Function:      IMsgDispatch *AK_GetIMsgDispatch(T_VOID);
Description:   获取消息分发实例接口指针。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
    本静态组件为单实例组件，不论创建多少次，返回的都是同一个接口指针。
    注: 没有增加该实例的引用计数，故无需释放它。
===============================================================================================================
===============================================================================================================
Function:      T_S32 AK_DispatchEvent(T_VOID);
Description:   进行一次消息分发，执行该函数将激活消息分发高级中断服务程序。
Parameters:    None
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_U32 IMsgDispatch_AddRef(IMsgDispatch *pIMsgDispatch);
Description:   引用计数加1。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_U32 IMsgDispatch_Release(IMsgDispatch *pIMsgDispatch);
Description:   引用计数减1，如果引用计数为0，则释放该组件实例。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
Return:  返回当前引用计数值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IMsgDispatch_Activate(IMsgDispatch *pIMsgDispatch);
Description:   激活高级中断服务程序来派发系统事件至应用队列。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IMsgDispatch_Register(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister, T_eMsgRegType eRegType, T_U32 dwEvtMsk);
Description:   将所要注册的对象句柄注册进消息分发器中，以接收系统消息。
               注册的应用需要设置要接收系统事件的掩码，否则将不会收到任何系统事件。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
   hRegister           [in] 指向要注册的对象句柄。
   eRegType           [in] 指向要注册的对象类型，目前支持线程和队列两种。
   dwEvtMsk           [in] 指向要注册的事件掩码，只有类型为队列时有效。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
===============================================================================================================
===============================================================================================================
Function:      T_S32 IMsgDispatch_UnRegister(IMsgDispatch *pIMsgDispatch, T_HANDLE hRegister);
Description:   将所要注册的对象句柄从消息分发器中取消注册，该应用将不再收到系统事件，除非
               它再次注册。
Parameters:
   pIMsgDispatch   [in] 指向该组件实例接口的指针。
   hRegister           [in] 指向已注册的对象句柄。
Return:  返回发送操作是否成功，如果为AK_SUCCESS，则表示发送成功，其它值表示发送失败，并
         对应于发送失败的原因值。
Remark:
===============================================================================================================
*/
/**************************************************************************************************************/

/*============================================================================================================*/
IMsgDispatch *AK_GetIMsgDispatch(T_VOID);
T_S32 AK_DispatchEvent(T_VOID);

//============================================================================================================//
T_S32 CMsgDispatch_New(IMsgDispatch **pp);

#define IMsgDispatch_AddRef(p)                  AK_GETVT(IMsgDispatch, p)->AddRef(p)
#define IMsgDispatch_Release(p)                 AK_GETVT(IMsgDispatch, p)->Release(p)
#define IMsgDispatch_Activate(p)                AK_GETVT(IMsgDispatch, p)->Activate((p))
#define IMsgDispatch_Register(p, h, t, m)       AK_GETVT(IMsgDispatch, p)->Register((p), (h), (t), (m))
#define IMsgDispatch_RegisterThread(p, h)       AK_GETVT(IMsgDispatch, p)->Register((p), ((T_HANDLE)h), (MSG_REG_THREAD), (0))
#define IMsgDispatch_RegisterQueue(p, h, m)     AK_GETVT(IMsgDispatch, p)->Register((p), ((T_HANDLE)h), (MSG_REG_QUEUE), (m))
#define IMsgDispatch_UnRegister(p, h)           AK_GETVT(IMsgDispatch, p)->UnRegister((p), ((T_HANDLE)h))

#endif //__AKMSGDISPATCH_H__
