
/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd. 
* All rights reserved.	
*  
* File Name£ºtscr_input.c
* Function£ºTouch screen Driver . But this file is windows version . 
*
* Author£ºLuoXiaoQing
* Date£º2005-10-10
* Version£º1.0		  
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/

#ifdef OS_WIN32
#include <windows.h>

#include "tscr_input.h"
#include "anyka_types.h"
#include "tscr_command.h"
/*
 * packet context
 */
typedef struct{
	T_U8	pckt[MAX_TPCKT_LEN];	//current packet content
	T_U8	offset;					//the offset of packet data
	T_POINT	pt;						//the inking point
	T_U8	words[STUFF_LEN-2];		//the recognized result, max length is 20
	T_U8	wordCnt;				//the words count
	T_S8	currentCmd;				//current command
	T_S8	currentRespone;			//current response
	T_S8	recogModeOff;			//recognise mode off
}TSCR_PCKT_CONTEXT;

TSCR_PCKT_CONTEXT pckt_context;

T_U8        *hDict = AK_NULL;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

T_BOOL tscr_HW_init()
{
	return AK_TRUE;
}
T_VOID tscr_HW_free()
{

}

T_BOOL tscr_SW_init( T_VOID )
{
//	gb_HandSignCheck = (T_U8*)GetBinData((T_pVOID *)&gb_HandSignCheck, AK_TRUE, eBINDATA_WT_UNI_LIB, AK_NULL);
	return AK_TRUE;

}

T_BOOL tscr_SW_free()
{	
	return AK_TRUE;
}


T_BOOL tscr_set_event_callback(T_fTSCRGET_CALLBACK callback_func)
{
	return AK_TRUE;
}



T_BOOL tscr_SetRecogMode(T_U32 rangeInput)
{

	return AK_TRUE;
}


T_BOOL tscr_SetMode(T_U8 mode)
{

	return AK_TRUE;
}



T_BOOL tscr_SetWriteArea(const T_RECT *rect)
 {

	return AK_TRUE;
}


 T_BOOL tscr_SetTextRect(const T_RECT *rect)
 {

	return AK_TRUE;
 	 
 }

T_U8   tscr_getCurADPt(T_pTSPOINT   ADpt)
{
	return AK_TRUE;
}

T_U8 tscr_getLastDownADPt(T_pTSPOINT   ADpt)
{
	return AK_TRUE;
}


T_BOOL tscr_GetPoint(T_POINT *pt)
{

	return AK_TRUE;
}

T_BOOL tscr_GETWORDS(T_pTSTR dest)
{

	return AK_TRUE;
}
T_U8 tscr_getUserMode()
{
	return 0;
}

// Load handwrite dictionary
T_S16 HWRE_LoadDict(T_VOID)
{
    return 0;
}

// Free handwrite dictionary
T_VOID HWRE_FreeDict(T_VOID)
{
	return;    
}


#endif

