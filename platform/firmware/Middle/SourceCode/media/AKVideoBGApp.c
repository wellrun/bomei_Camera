/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKVideoBGApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKVideoBGApp.h"
#include "Eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"

#include "Log_MediaVideo.h"


//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CVideoBGApp);

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
static T_S32 CVideoBGApp_Constructor(CVideoBGApp *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    //Add your init code here...
	Fwl_Print(C4, M_VDEC, "Video BG Ctor().\n");
	
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
static T_S32 CVideoBGApp_Destructor(CVideoBGApp *pMe)
{ 
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
#ifdef MEDIA_FAST_SWITCH
	pMe->m_hPlayer->pVideo = Vs_Close(pMe->m_hPlayer->pVideo);
#endif

#if CI37XX_PLATFORM	
	MpuRefr_Free();		
#endif	
	
	Fwl_Print(C4, M_VDEC, "Video Thread END.\n");
	
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
static T_S32 CVideoBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    CVideoBGApp *pMe = (CVideoBGApp *)pICBThread->pData;
    //Add your Prepare code here...
    Fwl_Print(C4, M_VDEC, "Video BG Prepare().\n");
	pMe->m_hPlayer = Vs_PrepareDec();
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
static T_S32 CVideoBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    CVideoBGApp *pMe = (CVideoBGApp *)pICBThread->pData;
    //Add your Handle code here...
    
    switch(eEvent)
	{
	case EVT_VIDEO_STOP:
		VideoStream_Reset(pMe->m_hPlayer->pVideo->hVS);
		break;

	case EVT_VIDEO_CLOSE:
		pMe->m_hPlayer->pVideo = Vs_Close(pMe->m_hPlayer->pVideo);
		Fwl_Print(C3, M_VDEC, "Close Video Decoder.\n");
		break;

	case EVT_VIDEO_EXIT:
		IAppMgr_DeleteEntry(AK_GetAppMgr(), AKAPP_CLSID_VIDEO);
		Fwl_Print(C3, M_VDEC, "Delete Video Thread.\n");
		break;

	case EVT_VIDEO_SCAN:
		Vs_HandleDecode(pMe->m_hPlayer);
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
T_S32 	CVideoBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    Fwl_Print(C3, M_VDEC, "Video BG New().\n");
    AKAPP_BG_NEW(CVideoBGApp, AKAPP_CLSID_VIDEO, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////



