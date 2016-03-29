/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAudioBGApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "AKMediaBGApp.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"
#include "Fwl_pfAudio.h"
#include "Ctl_AudioPlayer.h"
#include "Log_MediaPlayer.h"
#include "Log_MediaDmx.h"
#include "Ctl_RecAudio.h"


//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CMediaBGApp);

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
static T_S32 CMediaBGApp_Constructor(CMediaBGApp *pMe)
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
static T_S32 CMediaBGApp_Destructor(CMediaBGApp *pMe)
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
static T_S32 CMediaBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CMediaBGApp *pMe = (CMediaBGApp *)pICBThread->pData;
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
static T_S32 CMediaBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    //CMediaBGApp *pMe = (CMediaBGApp *)pICBThread->pData;
    //Add your Handle code here...
    
#ifdef OS_ANYKA   
    switch (eEvent)
    {
	case EVT_DMX_SCAN:
	case EVT_DMX_CLOSE:
	case EVT_DMX_PAUSE:
	case EVT_DMX_RESUME:
	case EVT_DMX_STOP:
	case EVT_DMX_START:
	case EVT_DMX_EXIT:
	case EVT_DMX_FF:
	case EVT_DMX_FR:
		MPlayer_HandleEvent(eEvent, pEvtParam);
		break;
#if 0
    case MEDIA_EVT_SEEK:
        Fwl_AudioSeek(pEvtParam->lParam);
        break;
    
    case MEDIA_EVT_SWITCH:
        AudioPlayer_AutoSwitch();
        break;
	case MEDIA_EVT_RES_FREE:
        break;
	case MEDIA_EVT_AUTO_STOP:
    case MEDIA_EVT_USER_STOP:
        AudioPlayer_Stop();
        break;   
    case MEDIA_EVT_AB_PLAY:
        AudioPlayer_BSeekToA();
        break;			
#endif 

#if defined(SUPPORT_AUDIOREC) || defined(SUPPORT_FM)
	case EVT_RECAUDIO_START:
    case EVT_RECAUDIO_REC:
    case EVT_RECAUDIO_STOP:
        Ctl_RecAudio_HandleMsg(eEvent, pEvtParam);
        break;
#endif

    default:
        Fwl_Print(C2, M_ENGINE, "Warning:	Audio task get an unknown event: %d.", eEvent);
        break; 
    }
#endif
    
    
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
T_S32     CMediaBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    
    AKAPP_BG_NEW(CMediaBGApp, AKAPP_CLSID_MEDIA, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

