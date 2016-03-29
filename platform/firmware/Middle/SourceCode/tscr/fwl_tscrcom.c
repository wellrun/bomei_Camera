/************************************************************************
* Copyright (c) 2001, Anyka Co., Ltd. 
* All rights reserved.	
*  
* File Name£º Fwl_tscrCom.c
* Function£ºGet Hand write char from ELAN by com port
 * this file will constraint the access to the bottom layer common OS fucntion, 
 * avoid resource competition. Also, this file os for porting to different OS
*
* Author£ºLuoXiaoQing
* Date£º2005-10-10
* Version£º1.0		  
*
* Reversion: 
* Author: 
* Date: 
**************************************************************************/
#include "fwl_evtmailbox.h"
#ifdef os_win32
	#include <windows.h>
#endif
#include "tscr_input.h"
#include "fwl_keyhandler.h"
#include "Lib_state.h"
#include "Eng_screensave.h"
#include "eng_debug.h"
#include "Fwl_sysevent.h"
#include "AKError.h"
#include "fwl_public.h"

#define EVTPARAM_TSCRACT(_param) ((T_TOUCHSCR_ACTION)((_param)->s.Param3))
#define MIN_MOVE_LEN	(3)

static T_VOID Fwl_TSCR_SendEvent_Callback(T_TOUCHSCR_ACTION act ,T_POINT pt);

static T_BOOL  tscr_touching = AK_FALSE;
static T_POINT point;


///for hard ware
/**
 * @brief Initial Touch Screen hard ware
 * 		Open the com port
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
T_BOOL Fwl_Tscr_HW_init(T_VOID)
{    
	tscr_set_event_callback( Fwl_TSCR_SendEvent_Callback);
    //tscr_SetMatrixToDef();
	return tscr_HW_init();
}
/**
 * @brief free l Touch Screen hard ware
 * 		close the com port
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
T_VOID Fwl_Tscr_HW_free(T_VOID)
{
	tscr_HW_free();
}

////end of hard ware

T_BOOL gb_lastHasDownEvt = AK_FALSE;

T_BOOL Fwl_TSCR_IsNeedRespond(T_VOID)
{
//	if(gb_stdb.bKeypadLock == AK_TRUE && CallGetCallQty() == 0 )
//	{
//		return AK_FALSE;
//	}
	if(  AK_TRUE == ScreenSaverIsOn())
	{
		return AK_FALSE;
	}
	return AK_TRUE;
}

static T_BOOL Fwl_TSCR_CompareEvtParam(const T_SYS_MAILBOX *pMailBox1, const T_SYS_MAILBOX *pMailBox2)
{
    if (pMailBox1->event == pMailBox2->event)
    {
    	if(EVTPARAM_TSCRACT(&pMailBox1->param) == EVTPARAM_TSCRACT(&pMailBox2->param))
    	{
    		return AK_TRUE;
    	}
    }
	return AK_FALSE;
}

#ifdef SUPPORT_AUTOTEST
extern T_BOOL autotest_testflag;

extern T_BOOL autotest_record_Tscr(T_TOUCHSCR_ACTION act, T_S16 pointx, T_S16 pointy);


static T_VOID  Autotest_TSCR_SendEvent(T_TOUCHSCR_ACTION act ,T_POINT pt)
{
	T_SYS_MAILBOX  mailbox;

    //added by ZhaoXing 20090516
    //if usb is used,don't send event
    if (gb.InUsbMessage)
    {
        //Fwl_Print(C3, M_FWL, "return\r\n");
        return;
    }

    //AK_DEBUG_OUTPUT("Fwl_TSCR_SendEvent_Callback, gb.bTscrIsValid" , gb.bTscrIsValid);
    if(!Fwl_IsTSCRValid())
    {
        return ;
    }

	mailbox.event = SYS_EVT_TSCR;//VME_EVT_TOUCHSCR_ACTION;	
	mailbox.param.s.Param1 = act;
	mailbox.param.s.Param2 = pt.x;
	mailbox.param.s.Param3 = pt.y;
	mailbox.param.s.Param4 = 0;
	
	
	switch(act)
	{
	case eTOUCHSCR_UP:
		if(gb_lastHasDownEvt)
		{
			gb_lastHasDownEvt = AK_FALSE;
			//AK_PostTerminalInputEvent(&mailbox);
			AK_PostTerminalInputEventEx(&mailbox, AK_NULL, AK_FALSE, AK_FALSE);
			Fwl_Print(C3, M_FWL, "TS UP(%d, %d)\r\n",pt.x,pt.y);
		}
		break;
	case eTOUCHSCR_DOWN:		
		if (AK_PostTerminalInputEventEx(&mailbox, Fwl_TSCR_CompareEvtParam, AK_TRUE, AK_FALSE) == AK_SUCCESS)
		{
			if (!AK_GetScreenSaverStatus())
			{
				gb_lastHasDownEvt = AK_TRUE;
			}
			else
			{
				gb_lastHasDownEvt = AK_FALSE;
			}
			
			Fwl_Print(C3, M_FWL, "TS DOWN(%d, %d)\r\n",pt.x,pt.y);
			point.x = pt.x;
			point.y = pt.y;
		}
		else
		{
			gb_lastHasDownEvt=AK_FALSE;
		}
		break;
	case eTOUCHSCR_MOVE:
		if(gb_lastHasDownEvt)
		{
			if ((pt.x - point.x)*(pt.x - point.x) + (pt.y - point.y)*(pt.y - point.y) < MIN_MOVE_LEN * MIN_MOVE_LEN)
			{
				break;
			}
			else
			{
				point.x = pt.x;
				point.y = pt.y;
			}
			
			if (AK_PostTerminalInputEventEx(&mailbox, Fwl_TSCR_CompareEvtParam, AK_FALSE, AK_FALSE) == AK_SUCCESS)
			{
				///Fwl_Print(C3, M_FWL, "TS MOVE(%d, %d)\r\n",pt.x,pt.y);
			}
		}
		break;
	case eTOUCHSCR_GETWORDS:
		AK_PostTerminalInputEvent(&mailbox); 
		Fwl_Print(C3, M_FWL, "TS GETWORDS\r\n");
		gb_lastHasDownEvt=AK_FALSE;
		break;
	case eTOUCHSCR_REFRESH_LINE:
#ifdef OS_ANYKA
		tscr_showCurLine(pt);
#endif
        if (AK_PostTerminalInputEventEx(&mailbox, AK_NULL, AK_TRUE, AK_FALSE) == AK_SUCCESS)
		{
			//Fwl_Print(C3, M_FWL, "Refresh Line \r\n");
		}
		
		break;
	}
    
	return;
}


/** 
 * @brief Send the Event to EEDIT that autotest happened
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */

//
T_VOID Autotest_TSCR_SendEvent_Callback(T_TOUCHSCR_ACTION act ,T_POS pointx, T_POS pointy)
{
	T_POINT pt;
	
	pt.x = pointx;
	pt.y = pointy;
	
	Autotest_TSCR_SendEvent( act, pt);
}

#endif
/** 
 * @brief Send the Event to EEDIT that GetWORDS happened
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
static T_VOID  Fwl_TSCR_SendEvent_Callback(T_TOUCHSCR_ACTION act ,T_POINT pt)
{
	T_SYS_MAILBOX  mailbox;

    //added by ZhaoXing 20090516
    //if usb is used,don't send event
    if (gb.InUsbMessage)
    {
        //Fwl_Print(C3, M_FWL, "return\r\n");
        return;
    }

    //AK_DEBUG_OUTPUT("Fwl_TSCR_SendEvent_Callback£¬ gb.bTscrIsValid" , gb.bTscrIsValid);
    if(!Fwl_IsTSCRValid())
    {
        return ;
    }

	mailbox.event = SYS_EVT_TSCR;//VME_EVT_TOUCHSCR_ACTION;	
	mailbox.param.s.Param1 = act;
	mailbox.param.s.Param2 = pt.x;
	mailbox.param.s.Param3 = pt.y;
	mailbox.param.s.Param4 = 0;
#ifdef SUPPORT_AUTOTEST
	//autotest record tscr info
	autotest_record_Tscr(act, pt.x, pt.y);

	//autotest testing not to press tscr info
	if(autotest_testflag == AK_TRUE)
	{
		return ;
	}
#endif
	
	switch(act)
	{
	case eTOUCHSCR_UP:
		if(gb_lastHasDownEvt)
		{
			gb_lastHasDownEvt = AK_FALSE;
			//AK_PostTerminalInputEvent(&mailbox);
			AK_PostTerminalInputEventEx(&mailbox, AK_NULL, AK_FALSE, AK_FALSE);
			Fwl_Print(C3, M_FWL, "TS UP(%d, %d)\r\n",pt.x,pt.y);
		}
		break;
	case eTOUCHSCR_DOWN:		
		if (AK_PostTerminalInputEventEx(&mailbox, Fwl_TSCR_CompareEvtParam, AK_TRUE, AK_FALSE) == AK_SUCCESS)
		{
			if (!AK_GetScreenSaverStatus())
			{
				gb_lastHasDownEvt = AK_TRUE;
			}
			else
			{
				gb_lastHasDownEvt = AK_FALSE;
			}
			
			Fwl_Print(C3, M_FWL, "TS DOWN(%d, %d)\r\n",pt.x,pt.y);
			point.x = pt.x;
			point.y = pt.y;
		}
		else
		{
			gb_lastHasDownEvt=AK_FALSE;
		}
		break;
	case eTOUCHSCR_MOVE:
		if(gb_lastHasDownEvt)
		{
			if ((pt.x - point.x)*(pt.x - point.x) + (pt.y - point.y)*(pt.y - point.y) < MIN_MOVE_LEN * MIN_MOVE_LEN)
			{
				break;
			}
			else
			{
				point.x = pt.x;
				point.y = pt.y;
			}
			
			if (AK_PostTerminalInputEventEx(&mailbox, Fwl_TSCR_CompareEvtParam, AK_FALSE, AK_FALSE) == AK_SUCCESS)
			{
				///Fwl_Print(C3, M_FWL, "TS MOVE(%d, %d)\r\n",pt.x,pt.y);
			}
		}
		break;
	case eTOUCHSCR_GETWORDS:
		AK_PostTerminalInputEvent(&mailbox); 
		Fwl_Print(C3, M_FWL, "TS GETWORDS\r\n");
		gb_lastHasDownEvt=AK_FALSE;
		break;
	case eTOUCHSCR_REFRESH_LINE:
#ifdef OS_ANYKA
		tscr_showCurLine(pt);
#endif
        if (AK_PostTerminalInputEventEx(&mailbox, AK_NULL, AK_TRUE, AK_FALSE) == AK_SUCCESS)
		{
			//Fwl_Print(C3, M_FWL, "Refresh Line \r\n");
		}
		
		break;
	}
    
	return;
}


T_BOOL Fwl_TSCR_GetTouchStatus(void)
{
	return tscr_touching;
}

T_VOID Fwl_TSCR_SetTouchStatus(T_BOOL status)
{
	tscr_touching = status;
}

/**
 * @brief Get words
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
T_BOOL Fwl_TSCR_GetWords(T_pTSTR dest)
{
	return tscr_GETWORDS(dest);
}

/**
 * @brief tscr_SetRecogMode
 * @author Luo XiaoQing
 * @date	2005-10-10
 * @version 1.0
 */
T_BOOL Fwl_tscr_SetRecogMode(T_U32 range)
{
	return tscr_SetRecogMode(range);
}

  

/**
 * @brief tscr_SetCodeTable
 * @author Luo XiaoQing
 * @date	2005-10-10
 *brief: set write area location
 *param: tx, top-left X-coordinate
 *param: ty, top-left Y-coordinate
 *param: bx, bottom-right X-coordinate
 *param: by, bottom-right Y-coordinate
 all x,y is from 0 to 0xFE
 * @version 1.0
 */
 T_BOOL Fwl_tscr_SetWriteArea(const T_RECT *rect)
 {

 	 return tscr_SetWriteArea(rect);
 }

/*
 *brief: Default is Recognition mode. Graphic mode transmission coordinates
		are similar to Recognition mode except recognition is not processed
		under Graphic mode.
 *param: mode, 0x00 for recogniton mode, 0x01 for graphic mode
 *command: 0x49
 */
T_BOOL Fwl_tscr_SetMode(T_U8 mode)
{
	return tscr_SetMode(mode);
}

/**
 * @brief tscr_SetTextRect
 * @author Luo XiaoQing
 * @date	2005-10-10
 *brief: set Text Show Rect
 * @version 1.0
 */
 T_BOOL Fwl_tscr_SetTextRect(const T_RECT *rect)
 {
	return tscr_SetTextRect(rect);
 	 
 }
  /** 
 * @brief Get the Current ADC point
 * @author \b LuoXiaoqing
 * @date 2006-04-26
  * @param[out] ADpt  the current ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
 */  
 T_U8   Fwl_tscr_getCurADPt(T_pTSPOINT   ADpt)
 {
	return tscr_getCurADPt( ADpt) ;  
 }

 
 T_U8   Fwl_tscr_getCurADPt_point(T_pPOINT  pPoint)
 {
 	T_TSPOINT  adpt = {-1, -2};
	T_U8        ret;

	ret = tscr_getCurADPt(&adpt);

	pPoint->x = (T_POS)adpt.x;
	pPoint->y = (T_POS)adpt.y;

	return ret;	
 }

   /** 
 * @brief  enable the touch screen
 * @author 
 * @date 
  * @param[out] none
 * @return  none
 */   
T_VOID Fwl_EnableTSCR(T_VOID)
{
#ifdef TOUCH_SCR
      gb.bTscrIsValid = AK_TRUE;
#endif    
}

  /** 
 * @brief  disable the touch screen
 * @author 
 * @date 
  * @param[out] 
 * @return 
 */   
T_VOID Fwl_DisableTSCR(T_VOID)
{
#ifdef TOUCH_SCR
      gb.bTscrIsValid = AK_FALSE;
#endif    
} 

  /** 
 * @brief   check that whether the touch screen is validate
 * @author 
 * @date 
  * @param[out] 
 * @return 
 */   
T_BOOL Fwl_IsTSCRValid(T_VOID)  
  {
#ifdef TOUCH_SCR
    return gb.bTscrIsValid;
#else
    return AK_FALSE;
#endif
  }

  /** 
 * @brief Get the Last down ADC point
  * @param[out] ADpt  the Last down ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
 */  
 T_U8   Fwl_tscr_getLastDownADPt(T_pTSPOINT   ADpt)
 {
	return tscr_getLastDownADPt( ADpt) ;  
 }

 T_U8	Fwl_tscr_getUserMode(T_VOID)
 {
	 return tscr_getUserMode();
 }



