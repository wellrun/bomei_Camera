/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKAudioListBGApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKAUDIOLISTBGAPP_H__
#define __AKAUDIOLISTBGAPP_H__
#include "AKThread.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCAudioListBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...

}CAudioListBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CAudioListBGApp_New(IThread **ppi);

#endif //__AKAUDIOLISTBGAPP_H__


