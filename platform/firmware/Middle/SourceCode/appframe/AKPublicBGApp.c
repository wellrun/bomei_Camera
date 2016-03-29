/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKPublicBGApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKPublicBGApp.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"
#include "ctl_waitbox.h"

//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CPublicBGApp);

/*===================================================================================*/
/**************************************************************************
* @BRIEF  
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CPublicBGApp_Constructor(CPublicBGApp *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    //Add your init code here...
    
    return lRet;
}

/**************************************************************************
* @BRIEF  
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CPublicBGApp_Destructor(CPublicBGApp *pMe)
{ 
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
    
    return lRet;
}

/**************************************************************************
* @BRIEF  
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CPublicBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CPublicBGApp *pMe = (CPublicBGApp *)pICBThread->pData;
    //Add your Prepare code here...
    
    return AK_SUCCESS;
}

/**************************************************************************
* @BRIEF  
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
static T_S32 CPublicBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    //CPublicBGApp *pMe = (CPublicBGApp *)pICBThread->pData;
    //Add your Handle code here...
    
    switch (eEvent)
    {
        case PUBLIC_EVT_WAITBOX_SHOW:
            WaitBox_Show(pEvtParam->w.Param1);
            break;

        case PUBLIC_EVT_FREQMGR:            
            //FreqMgr_SetChipFreq(pEvtParam->w.Param1);
            break;
            
        default:
            Fwl_Print(C2, M_ENGINE, "Warning:Public task get an unknown event.");
            break; 
    }
    
    return AK_SUCCESS;
}


/**************************************************************************
* @BRIEF  
* @AUTHOR 
* @DATE     2007-09-05
* @PARAM 
* @PARAM 
* @RETURN  T_S32  The result of this function executing.
* @RETVAL  AK_SUCCESS        Execute successfully.
* @RETVAL  Not AK_SUCCESS  The error type value.
***************************************************************************/
T_S32   CPublicBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    
    AKAPP_BG_NEW(CPublicBGApp, AKAPP_CLSID_PUBLIC, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

