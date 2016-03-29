/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKApp.h"
#include "fwl_evtmailbox.h"
#include "eng_debug.h"

//#################################################################################

typedef struct tagCApp
{
    IThread  *m_pIBase; //point to the base class instance.
    T_U32     m_Ref;
    IApp      m_myIApp;
    IWnd     *m_pIWnd;
    
}CApp;

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================

static T_U32  CApp_AddRef(IApp *pIApp);
static T_U32  CApp_Release(IApp *pIApp);
static T_S32 CApp_SetProperty(IApp *pIApp, T_U16 wPropID, T_VOID*  pPropData);
static T_S32 CApp_GetProperty(IApp *pIApp, T_U16 wPropID, T_VOID*  pPropData);
//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IApp) g_IAppFuncs =
{
    CApp_AddRef,
    CApp_Release,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    AK_NULL,
    CApp_SetProperty,
    CApp_GetProperty
};

static T_S32 CApp_Constructor(CApp *pMe, T_APP_INITPARAM *param)
{
    //Simple Constructor, Add your code here...
    T_S32 lRet = AK_SUCCESS;
    
    lRet = CThread_New(&pMe->m_pIBase, &param->base);
    lRet |= CWnd_New(&pMe->m_pIWnd, &param->wnd);
    
    return lRet;
}

static T_S32 CApp_Destructor(CApp *pMe)
{
    //Add your code here...
    AK_RELEASEIF(pMe->m_pIBase);
    AK_RELEASEIF(pMe->m_pIWnd);
    
    return AK_SUCCESS;
}

static T_U32 CApp_AddRef(IApp *pIApp)
{
    CApp *pMe = (CApp *)pIApp->pData;
    
    return ++pMe->m_Ref;
}

static T_U32 CApp_Release(IApp *pIApp)
{
    CApp *pMe = (CApp *)pIApp->pData;
    
    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CApp_Destructor(pMe);
        Fwl_Free(pMe);
        return 0;
    }
    
    return pMe->m_Ref;
}

static T_S32 CApp_SetProperty(IApp *pIApp, T_U16 wPropID, T_VOID*  pPropData)
{
    CApp *pMe = (CApp *)pIApp->pData;
    T_S32 lRet = AK_SUCCESS;

    if ((AK_NULL == pIApp) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }

    switch(wPropID)
    {
       default:
           {
               lRet = IThread_SetProperty(pMe->m_pIBase, wPropID, pPropData);
           }
           break;
    }

    return lRet;
}

static T_S32 CApp_GetProperty(IApp *pIApp, T_U16 wPropID, T_VOID*  pPropData)
{
    CApp *pMe = (CApp *)pIApp->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if ((AK_NULL == pIApp) || (AK_NULL == pPropData))
    {
        return AK_EBADPARAM;
    }
  
    switch(wPropID)
    {
       case THREAD_PROP_BACKGROUND:
           {
               *((T_BOOL*)pPropData) = AK_FALSE;
           }
           break;
       case AKAPP_PROP_INTER_WND:
           {
               AK_ADDREFIF(pMe->m_pIWnd);
               *((IWnd**)pPropData) = pMe->m_pIWnd;
           }
           break;
       default:
           {
               lRet = IThread_GetProperty(pMe->m_pIBase, wPropID, pPropData);
           }
           break;
    }

    return lRet;
}


T_S32 CApp_New(IApp **ppi, T_APP_INITPARAM *param)
{
    T_S32 nErr = AK_SUCCESS;
    CApp *pNew = AK_NULL;
    
    if (AK_NULL == ppi)
    {
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CApp);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_myIApp), &g_IAppFuncs);
        pNew->m_myIApp.pData = (T_VOID*)pNew;
        
        nErr = CApp_Constructor(pNew, param);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CApp_Release(&pNew->m_myIApp);
            pNew = AK_NULL;
            break;
        }
        
        *ppi = (IApp *)&pNew->m_myIApp;
        
    } while(AK_FALSE);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

