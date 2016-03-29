/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAudioListBGApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKAudioListBGApp.h"
#include "Eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"
#include "svc_medialist.h"



//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CAudioListBGApp);

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
static T_S32 CAudioListBGApp_Constructor(CAudioListBGApp *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    //Add your init code here...
	Fwl_Print(C4, M_MLIST, "AudioList BG Ctor()");
	
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
static T_S32 CAudioListBGApp_Destructor(CAudioListBGApp *pMe)
{ 
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
	
	Fwl_Print(C4, M_MLIST, "AudioList Thread END.\n");
	
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
static T_S32 CAudioListBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CAudioListBGApp *pMe = (CAudioListBGApp *)pICBThread->pData;
    //Add your Prepare code here...
    Fwl_Print(C4, M_MLIST, "AudioListBG: Calling MList BG Prepare().\n");

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
static T_S32 CAudioListBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    //CAudioListBGApp *pMe = (CAudioListBGApp *)pICBThread->pData;
    //Add your Handle code here...
    
    switch(eEvent)
	{
	case eMEDIALIST_EVT_ADD:
		MList_BG_AddPath((T_pWSTR)pEvtParam->w.Param1, (T_BOOL)pEvtParam->s.Param3, (T_BOOL)pEvtParam->s.Param4, eMEDIA_LIST_AUDIO);
		break;

	case eMEDIALIST_EVT_DEL:
		MList_BG_DelPath((T_pWSTR)pEvtParam->w.Param1, (T_BOOL)pEvtParam->w.Param2, eMEDIA_LIST_AUDIO);
		break;

	case eMEDIALIST_EVT_DELALL:
		MList_BG_RemoveAll(eMEDIA_LIST_AUDIO);
		break;

	case eMEDIALIST_EVT_ID3_DELITEM:
		MList_BG_ID3_RemoveItem((T_pWSTR)pEvtParam->w.Param1, pEvtParam->w.Param2);
		break;

	case eMEDIALIST_EVT_ID3_DELCLASS:
		MList_BG_ID3_RemoveClass((T_pWSTR)pEvtParam->w.Param1, pEvtParam->w.Param2);
		break;
		
	case eMEDIALIST_EVT_CLOSE:
		MList_Close(eMEDIA_LIST_AUDIO);
		break;
	default:
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
T_S32 	CAudioListBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    Fwl_Print(C3, M_MLIST, "AudioList BG Thread New()");
    AKAPP_BG_NEW(CAudioListBGApp, AKAPP_CLSID_AUDIOLIST, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////



