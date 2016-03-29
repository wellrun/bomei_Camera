/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKPublicBGApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKPUBLICBGAPP_H__
#define __AKPUBLICBGAPP_H__
#include "AKThread.h"

//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCPublicBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...
       
}CPublicBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32     CPublicBGApp_New(IThread **ppi);

#endif //__AKPublicAPP_H__

