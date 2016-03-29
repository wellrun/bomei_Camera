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
#include "AKAudioBGApp.h"
#include "Eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"
#include "Log_MediaPlayer.h"
#include "Log_MediaVideo.h"
#include "Log_MediaAudio.h"
#include "Log_Mp3Player.h"
#include "Fwl_waveout.h"


//###################################################################################

//===================================================================================
/**
##    初始化回调接口虚表，以使回调接口函数能在适当时候被调用。
*/
//===================================================================================
AKAPP_CALLBACK_VTABLE_INIT(CAudioBGApp);

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
static T_S32 CAudioBGApp_Constructor(CAudioBGApp *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    //Add your init code here...
	Fwl_Print(C4, M_ADEC, "Audio BG Ctor()");
	
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
static T_S32 CAudioBGApp_Destructor(CAudioBGApp *pMe)
{ 
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
    	
	// pMe->m_hPlayer->pAudio = Sd_Close(pMe->m_hPlayer->pAudio);
	
	Fwl_Print(C4, M_ADEC, "Audio Thread END.\n");
	
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
static T_S32 CAudioBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    CAudioBGApp *pMe = (CAudioBGApp *)pICBThread->pData;
    //Add your Prepare code here...
    AK_DEBUG_OUTPUT("AudioBG:	Calling Audio BG Prepare().\n");
	pMe->m_hPlayer = MPlayer_GetPlayer();
	pMe->m_mp3Player = MP3Player_GetPlayer();
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
static T_S32 CAudioBGApp_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    CAudioBGApp *pMe = (CAudioBGApp *)pICBThread->pData;
    //Add your Handle code here...
    
    switch(eEvent)
	{
	case EVT_AUDIO_START:
		Sd_Start(pMe->m_hPlayer, pEvtParam->w.Param1);
		break;
		
	case EVT_AUDIO_STOP:
		Sd_Stop(pMe->m_hPlayer->pAudio);
		break;

	case EVT_AUDIO_CLOSE:
		pMe->m_hPlayer->pAudio = Sd_Close(pMe->m_hPlayer->pAudio);
		break;

	case EVT_AUDIO_SCAN:
		Sd_HandleAudioDec(pMe->m_hPlayer);
		break;
			
	case EVT_AUDIO_MP3START:
		Sd_MP3Start(pMe->m_mp3Player, pEvtParam->w.Param1);
		break;	
	
	case EVT_AUDIO_MP3STOP:
		AK_DEBUG_OUTPUT("audio Bg app Sd_MP3Stop!!!\n");
		Sd_MP3Stop(pMe->m_mp3Player->pMp3Aud);
		break;
	
	case EVT_AUDIO_MP3CLOSE:
		pMe->m_mp3Player->pMp3Aud = Sd_Mp3Close(pMe->m_mp3Player->pMp3Aud);
		break;
		
	case EVT_AUDIO_MP3SCAN:
		Sd_Mp3HandleAudDec(pMe->m_mp3Player);
		break;

	case EVT_AUDIO_ENDA:
		WaveOut_EnDA();
		break;

	case EVT_AUDIO_DISDA:
		WaveOut_DisDA();
		break;

	case EVT_AUDIO_FILTER:
		Sd_SetFilter(pMe->m_hPlayer->pAudio, pEvtParam->w.Param1, (T_U8)pEvtParam->w.Param2);
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
T_S32 	CAudioBGApp_New(IThread **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    Fwl_Print(C3, M_ADEC, "Audio BG Thread New()");
    AKAPP_BG_NEW(CAudioBGApp, AKAPP_CLSID_AUDIO, ppi, nErr);
    
    return nErr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////



