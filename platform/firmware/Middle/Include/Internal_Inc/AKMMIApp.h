/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKCustomApp.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKCUSTOMAPP_H__
#define __AKCUSTOMAPP_H__
#include "AKApp.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagCMMI
{
    IApp     *m_pIBase;  //Base class instance... 
    IApp      m_myIApp;
    ICBThread m_ICBThread;    
}CMMI;

//////////////////////////////////////////////////////////////////////////////////////////////////

T_S32 	CMMI_New(IApp **ppi);

IThread* CMMI_GetThread(T_VOID);

#endif //__AKMMIAPP_H__

