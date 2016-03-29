/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKApp_Def.h
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKAPP_DEF__
#define __AKAPP_DEF__


/*==========================================================================================*/
#define AK_INTERFACE_TO_ME(name, i) \
    name *pMe = (name *)i->pData


/*==========================================================================================*/
#define AKAPP_CALLBACK_VTABLE_INIT(name) \
/*===================================================================================*/\
/*                      <---- Function Declaration ---->                             */\
/*===================================================================================*/\
static T_S32 name##_ICBThread_Prepare(ICBThread *pICBThread);\
static T_S32 name##_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);\
/*===================================================================================*/\
static T_S32 name##_Destructor(name *pMe);\
/* <== Call back Free function ==>*/\
static T_S32 name##_ICBThread_Free(ICBThread *pICBThread)\
{\
    name *pMe = (name *)pICBThread->pData;\
\
    IThread_Register((IThread*)pMe->m_pIBase,AK_NULL);\
    AK_RELEASEIF(pMe->m_pIBase); \
    name##_Destructor(pMe);\
    Fwl_Free(pMe);\
\
    return AK_SUCCESS;\
}\
\
static const AK_VTABLE(ICBThread) g_ICB##name##Funcs = \
{ \
    name##_ICBThread_Prepare,\
    name##_ICBThread_Handle,\
    name##_ICBThread_Free\
}

/*==========================================================================================*/
#define AKAPP_CALLBACK_VTABLE_INIT_NO_PREPARE(name) \
/*===================================================================================*/\
/*                      <---- Function Declaration ---->                             */\
/*===================================================================================*/\
static T_S32 name##_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);\
/*===================================================================================*/\
static T_S32 name##_Destructor(name *pMe);\
/* <== Call back Free function ==>*/\
static T_S32 name##_ICBThread_Free(ICBThread *pICBThread)\
{\
    name *pMe = (name *)pICBThread->pData;\
\
    IThread_Register((IThread*)pMe->m_pIBase,AK_NULL);\
    AK_RELEASEIF(pMe->m_pIBase); \
    name##_Destructor(pMe);\
    Fwl_Free(pMe);\
\
    return AK_SUCCESS;\
}\
\
static const AK_VTABLE(ICBThread) g_ICB##name##Funcs = \
{ \
    AK_NULL,\
    name##_ICBThread_Handle,\
    name##_ICBThread_Free\
}


/*==========================================================================================*/
#define AKAPP_FG_AUTOMATIC_INIT(name) \
    typedef struct tag##name\
    {\
         AKAPP_FG_MEMBER_DEF;\
    }name;\
    /*###############*/\
    static T_S32 name##_Constructor(name *pMe)\
    {\
        return AK_SUCCESS;\
    }\
    static T_S32 name##_Destructor(name *pMe)\
    {\
        return AK_SUCCESS;\
    }\
    AKAPP_CALLBACK_VTABLE_INIT_NO_PREPARE(name)


/*==========================================================================================*/
#define AKAPP_BG_AUTOMATIC_INIT(name) \
    typedef struct tag##name\
    {\
         AKAPP_BG_MEMBER_DEF;\
    }name;\
    /*###############*/\
    static T_S32 name##_Constructor(name *pMe)\
    {\
        return AK_SUCCESS;\
    }\
    static T_S32 name##_Destructor(name *pMe)\
    {\
        return AK_SUCCESS;\
    }\
    AKAPP_CALLBACK_VTABLE_INIT_NO_PREPARE(name)
    

/*==========================================================================================*/
#define AKAPP_FG_MEMBER_DEF \
    IApp     *m_pIBase;  /*Base class instance...*/\
    ICBThread m_ICBThread

/*==========================================================================================*/
#define AKAPP_BG_MEMBER_DEF \
    IThread  *m_pIBase;  /*Base class instance...*/\
    ICBThread m_ICBThread

/*==========================================================================================*/
#define AKAPP_FG_INIT(cls, wnd, ret) \
  do\
  {\
    T_AppInfo *pAppInfo = AK_NULL;\
    T_APP_INITPARAM sInitParam;\
    \
    pAppInfo = AK_GetDefaultAppInfo(cls);\
    if (AK_NULL != pAppInfo)\
    {\
        sInitParam.base = pAppInfo->sInitparam;\
        sInitParam.wnd  = wnd;\
        ret = CApp_New(&pNew->m_pIBase, &sInitParam);\
        ret |= IAkApp_Register(pNew->m_pIBase,\
                               &pNew->m_ICBThread);\
    }\
    else\
    {\
        ret = AK_EUNSUPPORT;\
    }\
  }while(AK_FALSE)


/*==========================================================================================*/
#define AKAPP_BG_INIT(cls, ret) \
  do\
  {\
    T_AppInfo *pAppInfo = AK_NULL;\
    \
    pAppInfo = AK_GetDefaultAppInfo(cls);\
    if (AK_NULL != pAppInfo)\
    {\
        ret = CThread_New(&pNew->m_pIBase, &pAppInfo->sInitparam);\
        ret |= IThread_Register(pNew->m_pIBase,\
                               &pNew->m_ICBThread);\
    }\
    else\
    {\
        ret = AK_EUNSUPPORT;\
    }\
  }while(AK_FALSE)
  
  
/*==========================================================================================*/
#define AKAPP_FG_NEW(name, cls, wnd,  ppi, ret) \
    do \
    {\
        name *pNew = AK_NULL;\
     \
        if (AK_NULL != ppi)\
        {\
            *ppi = AK_NULL;\
        }\
     \
        pNew = AK_MALLOCRECORD(name);\
        AK_BREAKIF(AK_NULL == pNew, ret, AK_ENOMEMORY);\
     \
        AK_SETVT(&(pNew->m_ICBThread), &g_ICB##name##Funcs);\
        pNew->m_ICBThread.pData = (T_VOID*)pNew;\
     \
        AKAPP_FG_INIT(cls, wnd, ret); \
        if (AK_IS_SUCCESS(ret))\
        {\
            ret = name##_Constructor(pNew);\
        }\
        if (AK_IS_FAILURE(ret))\
        {\
            name##_ICBThread_Free(&(pNew->m_ICBThread));\
            pNew = AK_NULL;\
            break;\
        }\
        else\
        {\
            T_AppEntry AppEntry;\
     \
            AppEntry.wAppCls  = cls;\
            AppEntry.pIThread = (IThread*)pNew->m_pIBase;\
            ret = IAppMgr_AddEntry(AK_GetAppMgr(), &AppEntry, AK_FALSE);\
        }\
     \
        if (AK_NULL != ppi)\
        {\
            *ppi = (IApp*)pNew->m_pIBase;\
        }\
    } while(AK_FALSE)


/*==========================================================================================*/
#define AKAPP_BG_NEW(name, cls, ppi, ret) \
    do \
    {\
        name *pNew = AK_NULL;\
     \
        if (AK_NULL != ppi)\
        {\
            *ppi = AK_NULL;\
        }\
     \
        pNew = AK_MALLOCRECORD(name);\
        AK_BREAKIF(AK_NULL == pNew, ret, AK_ENOMEMORY);\
     \
        AK_SETVT(&(pNew->m_ICBThread), &g_ICB##name##Funcs);\
        pNew->m_ICBThread.pData = (T_VOID*)pNew;\
     \
        AKAPP_BG_INIT(cls, ret); \
        if (AK_IS_SUCCESS(ret))\
        {\
            ret = name##_Constructor(pNew);\
        }\
        if (AK_IS_FAILURE(ret))\
        {\
            name##_ICBThread_Free(&(pNew->m_ICBThread));\
            pNew = AK_NULL;\
            break;\
        }\
        else\
        {\
            T_AppEntry AppEntry;\
     \
            AppEntry.wAppCls  = cls;\
            AppEntry.pIThread = (IThread*)pNew->m_pIBase;\
            ret = IAppMgr_AddEntry(AK_GetAppMgr(), &AppEntry, AK_FALSE);\
        }\
     \
        if (AK_NULL != ppi)\
        {\
            *ppi = (IThread*)pNew->m_pIBase;\
        }\
    } while(AK_FALSE)






#endif //__AKAPP_DEF__