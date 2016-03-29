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
#ifndef __AKAUDIOBGAPP_H__
#define __AKAUDIOBGAPP_H__
#include "AKThread.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCAudioBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...
       
}CMediaBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CMediaBGApp_New(IThread **ppi);

#endif //__AKAUDIOAPP_H__

