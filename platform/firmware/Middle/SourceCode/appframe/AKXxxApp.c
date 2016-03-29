/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKXxxApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKXxxApp.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"

//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================

AKAPP_CALLBACK_VTABLE_INIT(CXxxApp);


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
static T_S32 CXxxApp_Constructor(CXxxApp *pMe)
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
static T_S32 CXxxApp_Destructor(CXxxApp *pMe)
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
static T_S32 CXxxApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CXxxApp *pMe = (CXxxApp *)pICBThread->pData;
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
static T_S32 CXxxApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    //CXxxApp *pMe = (CXxxApp *)pICBThread->pData;
    //Add your Handle code here...
    
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
T_S32 	CXxxApp_New(IApp **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    T_WND_INITPARAM wnd;
    
    wnd.dispbuf = AK_NULL;
    wnd.height  = Fwl_GetLcdHeight();
    wnd.width   = Fwl_GetLcdWidth();
    wnd.type    = AK_DISP_RGB;

    AKAPP_FG_NEW(CXxxApp, AKAPP_CLSID_XXX, wnd, ppi, nErr);
    
    return nErr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

