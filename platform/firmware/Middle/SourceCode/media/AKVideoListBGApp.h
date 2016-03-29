/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKVideoListBGApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKVIDEOLISTBGAPP_H__
#define __AKVIDEOLISTBGAPP_H__
#include "AKThread.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCVideoListBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...

}CVideoListBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CVideoListBGApp_New(IThread **ppi);

#endif //__AKVIDEOLISTBGAPP_H__


