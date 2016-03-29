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
#ifndef __AKVIDEOBGAPP_H__
#define __AKVIDEOBGAPP_H__
#include "AKThread.h"

#include "Log_MediaStruct.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCVideoBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...

	T_pMT_MPLAYER m_hPlayer;
}CVideoBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CVideoBGApp_New(IThread **ppi);

#endif //__AKVIDEOAPP_H__


