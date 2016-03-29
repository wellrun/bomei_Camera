/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAudioBGApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKAudioBGAPP_H__
#define __AKAudioBGAPP_H__
#include "AKThread.h"

#include "Log_MediaStruct.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCAudioBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...

	T_pMT_MPLAYER m_hPlayer;
	T_pMT_MP3PLAYER m_mp3Player;
}CAudioBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CAudioBGApp_New(IThread **ppi);

#endif //__AKVIDEOAPP_H__


