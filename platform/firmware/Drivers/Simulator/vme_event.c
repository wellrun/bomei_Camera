/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef OS_WIN32

#include <stdlib.h>
#include <assert.h>
#include "vme_interface.h"
#include "vme_util.h"
#include "Fwl_tscrcom.h"
#include "winvme.h"
#include "Eng_Debug.h"


/*****************************************************************************
 * functions
 *****************************************************************************
*/

static void SendVMEEvent (vT_EvtCode EvtCode, T_SYS_PARAM* pEvtParam)
{
    //vme_util_PutInVMEEventQueue (EvtCode, pEvtParam);
	T_SYS_MAILBOX mailbox;

	mailbox.event = EvtCode;
	mailbox.param = *pEvtParam;

	AK_PostTerminalInputEvent(&mailbox);
  //  winvme_ScheduleVMEEngine ();

    return;
}

static vBOOL SendUniqueVMEEvent (T_SYS_EVTID EvtCode, T_SYS_PARAM* pEvtParam)
{
	vBOOL ret;

    ret=vme_util_PutUniqueVMEEventQueue (EvtCode, pEvtParam);

    //winvme_ScheduleVMEEngine ();

    return ret;
}
/*****************************************************************************
 * interface
 *****************************************************************************
*/

/** 
* vme system events
*/
void vme_event_Send_VME_EVT_SYSSTART (void)
{
//	vT_EvtParam EvtParam;
    T_SYS_PARAM EvtParam;
    SendVMEEvent (VME_EVT_SYSSTART, &EvtParam);

    return;
}

void vme_event_Send_VME_EVT_RETRIGGER (vT_EvtSubCode event, vUINT32 param)
{
    vT_EvtParam EvtParam;

    EvtParam.lParam = param;

	//update by ljh
    //SendVMEEvent ((VME_EVT_RETRIGGER | event), &EvtParam);
    SendVMEEvent (event, (T_SYS_PARAM*)&EvtParam);
    return;
}

/** 
* keypad events
*/

/*
* touch screen events
*/
void vme_event_Send_VME_EVT_TOUCHSCR_ACTION (T_TOUCHSCR_ACTION action, vUINT16 x, vUINT16 y, vUINT32 TimeStamp)
{
    vT_EvtParam EvtParam;
	static	vBOOL vme_lastHasDownEvt;		
    EvtParam.s.Param1   =action ; 
    EvtParam.s.Param2   = x;
	EvtParam.s.Param3	= y;
	EvtParam.s.Param4	= (vUINT16)TimeStamp;
	switch(action)
	{
	case eTOUCHSCR_UP:
		if(vme_lastHasDownEvt)
		{
			SendUniqueVMEEvent (SYS_EVT_TSCR, (T_SYS_PARAM*)&EvtParam);
		}
		break;
	case eTOUCHSCR_DOWN:		
		if(vTRUE==SendUniqueVMEEvent(SYS_EVT_TSCR, (T_SYS_PARAM*)&EvtParam))
		{
			vme_lastHasDownEvt=vTRUE;
		}
		else
		{
			vme_lastHasDownEvt=vFALSE;
		}
		break;
	case eTOUCHSCR_MOVE:
		if(vme_lastHasDownEvt)
		{
			if(SendUniqueVMEEvent(SYS_EVT_TSCR, (T_SYS_PARAM*)&EvtParam))
			{
			}
		}
		break;
	}
//     AK_DEBUG_OUTPUT("evtcodevme_lastHasDownEvt:%d,action:%d,x:%d,y:%d\n",VME_EVT_TOUCHSCR_ACTION,vme_lastHasDownEvt,action,x,y);
	return;
}

/** 
* V24 events
*/

// java adds this funtion for java user key process. 07.9.21
void vme_event_Send_VME_EVT_KEY( int KeyMes , int Key )
{
	vT_EvtParam EvtParam;
	EvtParam.c.Param1   = Key;
	SendVMEEvent ((vT_EvtCode)KeyMes , (T_SYS_PARAM*)&EvtParam);
    return;
}

void vme_event_Send_VME_EVT_V24_WRITE_CALLBACK (vUINT8 DeviceHandle)
{
    vT_EvtParam EvtParam;

    EvtParam.c.Param1   = DeviceHandle;

    SendVMEEvent (VME_EVT_V24_WRITE_CALLBACK, (T_SYS_PARAM*)&EvtParam);

    return;
}

void vme_event_Send_VME_EVT_V24_RECEIVED_DATA_AVAIL (vUINT8 DeviceHandle)
{
    vT_EvtParam EvtParam;

    EvtParam.c.Param1   = DeviceHandle;

    SendVMEEvent (VME_EVT_V24_RECEIVED_DATA_AVAIL, (T_SYS_PARAM*)&EvtParam);

    return;
}

void vme_event_Send_VME_EVT_V24_STATUS_LINES_CHANGED (vUINT8 DeviceHandle, vUINT8 Dtr, vUINT8 Rts)
{
    vT_EvtParam EvtParam;

    EvtParam.c.Param1   = DeviceHandle;
    EvtParam.c.Param2   = Dtr;              // DTR (PC Sim = DSR)
    EvtParam.c.Param3   = Rts;              // RTS (PC Sim = CTS) 

    SendVMEEvent (VME_EVT_V24_STATUS_LINES_CHANGED, (T_SYS_PARAM*)&EvtParam);

    return;
}

/** 
* vatc events
*/
// void vme_event_Send_VME_EVT_VATC_WRITE_CALLBACK (vUINT8 DeviceHandle)
// {
//     vT_EvtParam EvtParam;
// 
//     EvtParam.c.Param1   = DeviceHandle;
// 
//     SendVMEEvent (VME_EVT_VATC_WRITE_CALLBACK, (T_SYS_PARAM*)&EvtParam);
// 
//     return;
// }

// void vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL (vUINT8 DeviceHandle)
// {
//     vT_EvtParam EvtParam;
// 
//     EvtParam.c.Param1   = DeviceHandle;
// 
//     SendVMEEvent (VME_EVT_VATC_RECEIVED_DATA_AVAIL, (T_SYS_PARAM*)&EvtParam);
// 
//     return;
// }

// void vme_event_Send_VME_EVT_VATC_STATUS_LINES_CHANGED (vUINT8 DeviceHandle, vUINT8 Ring, vUINT8 Dcd)
// {
//     vT_EvtParam EvtParam;
// 
//     EvtParam.c.Param1   = DeviceHandle;
//     EvtParam.c.Param2   = Ring;             
//     EvtParam.c.Param3   = Dcd;    
// 
//     SendVMEEvent (VME_EVT_VATC_STATUS_LINES_CHANGED, (T_SYS_PARAM*)&EvtParam);
// 
//     return;
// }
// void vme_event_Send_VME_EVT_VATC_RECEIVED_MUX_DATA (vUINT8 DeviceHandle)
// {
//     vT_EvtParam EvtParam;
// 
//     EvtParam.c.Param1   = DeviceHandle;
// 
//     SendVMEEvent (VME_EVT_VATC_RECEIVED_MUX_DATA, (T_SYS_PARAM*)&EvtParam);
//     //SendUniqueVMEEvent (VME_EVT_VATC_RECEIVED_MUX_DATA, (T_SYS_PARAM*)&EvtParam); 
// 
//     return;
// }

/** 
* fsfflash events
*/
/*
void vme_event_Send_VME_EVT_FSFLASH_FINISH (vFS_RESULT_STRUCT *pRes)
{
    vT_EvtParam EvtParam;

    EvtParam.lpParam    =   pRes;

    SendVMEEvent (VME_EVT_FSFLASH_FINISH, &EvtParam);

    return;
}
*/
#endif
