/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKXxxBGApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKXxxBGApp.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"


//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CXxxBGApp);

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
static T_S32 CXxxBGApp_Constructor(CXxxBGApp *pMe)
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
static T_S32 CXxxBGApp_Destructor(CXxxBGApp *pMe)
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
static T_S32 CXxxBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CXxxBGApp *pMe = (CXxxBGApp *)pICBThread->pData;
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
static T_S32 CXxxBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    //CXxxBGApp *pMe = (CXxxBGApp *)pICBThread->pData;
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
T_S32 	CXxxBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    
    AKAPP_BG_NEW(CXxxBGApp, AKAPP_CLSID_XXX, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

