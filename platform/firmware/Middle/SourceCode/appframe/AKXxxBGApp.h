/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKXxxBGApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKXXXBGAPP_H__
#define __AKXXXBGAPP_H__
#include "AKThread.h"


//////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tagCXxxBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...
       
}CXxxBGApp;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CXxxBGApp_New(IThread **ppi);

#endif //__AKXXXAPP_H__

