/**************************************************************************************
 * @file Eng_PowerOnThread.c
 * @brief PowerOn Thread Implementation for executing time-consuming operate. it will exit after run once.
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author HouBangen
 * @date 2011-12-21
 * @version 1.0
 **************************************************************************************/
 
#include "fwl_public.h"
#include "AKThread.h"
#include "Fwl_Initialize.h"
#include "AKError.h"
#include "Fwl_OsFS.h"
#include "akos_api.h"

#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKAppmgr.h"

typedef struct tagCPowerOnBGApp
{
    AKAPP_BG_MEMBER_DEF;
    //Add your member here...
       
} CPowerOnBGApp;


//============================================================
AKAPP_CALLBACK_VTABLE_INIT(CPowerOnBGApp);
//============================================================


static T_S32 CPowerOnBGApp_Constructor(CPowerOnBGApp *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    //Add your init code here...
    
    return lRet;
}

static T_S32 CPowerOnBGApp_Destructor(CPowerOnBGApp *pMe)
{ 
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
    
    return lRet;
}

static T_S32 CPowerOnBGApp_ICBThread_Prepare(ICBThread *pICBThread)
{
    T_S32 lRet = AK_SUCCESS;   
    //Add your code here...
    
    return lRet;
}


static T_S32 CPowerOnBGApp_ICBThread_Handle(ICBThread *pICBThread, 
						T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
	T_S32 lRet = AK_SUCCESS;
	
	if (!gs.sysbooterrstatus)
	{
		gs.sysbooterrstatus = AK_TRUE;	//开机后保存为异常关机状态
		SaveUserdata();
	}

	Fwl_CreateDefPath();


	/// exit PowerOn thread
	lRet = IAppMgr_DeleteEntry(AK_GetAppMgr(), AKAPP_CLSID_POWERON);

    return lRet;
}


T_VOID CPowerOnThread_New(T_VOID)
{
	IThread **ppi  = AK_NULL;
	T_S32     lRet = AK_SUCCESS;
	
	T_SYS_MAILBOX mailbox;
	
	mailbox.event 		 = 0;
	mailbox.param.lParam = 0;

/// create poweron thread
	AKAPP_BG_NEW(CPowerOnBGApp, AKAPP_CLSID_POWERON, ppi, lRet);

/// post event cause to call handle func
	if (AK_SUCCESS == lRet)
	{
    	IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_POWERON, &mailbox);
	}
	else
	{
		Fwl_Print(C1, M_INIT, "Create PowerOn background thread failed\n");
	}
	
}

