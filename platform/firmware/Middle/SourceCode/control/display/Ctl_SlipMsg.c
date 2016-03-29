/**
 * @file Ctl_SlipMsg.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#include "ctl_slipMsg.h"
#include "fwl_oscom.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "Fwl_tscrcom.h"


#define DIFF(a, b)	((a) > (b) ? (a) - (b) : (b) - (a))

#define MOVE_LEN_MIN	(12)
#define SLIPMSG_TIME	(10)
T_BOOL	gb_UserkeyValid = AK_TRUE;

static T_BOOL SlipMsg_StartTimer(T_SLIP_MSG *pSlipMsg);
static T_BOOL SlipMsg_StopTimer(T_SLIP_MSG *pSlipMsg);

/**
* @brief Creat a slip msg control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIP_MSG * the msg handle
* @retval
*/
T_SLIP_MSG *SlipMsg_Creat(E_MOVETYPE movetype)
{
	T_SLIP_MSG *pSlipMsg = AK_NULL;

	if (movetype > MOVETYPE_Y)
	{
		return AK_NULL;
	}

	pSlipMsg = (T_SLIP_MSG *)Fwl_Malloc(sizeof(T_SLIP_MSG));

	AK_ASSERT_PTR(pSlipMsg, "SlipMsg_Creat(): pSlipMsg malloc error", AK_NULL);
	memset(pSlipMsg, 0, sizeof(T_SLIP_MSG));

	pSlipMsg->lastSta = SLIPMSG_STA_STOP;
	pSlipMsg->curSta = SLIPMSG_STA_STOP;
	pSlipMsg->timer = ERROR_TIMER;
	pSlipMsg->moveType = movetype;

	gb_UserkeyValid = AK_TRUE;

	return pSlipMsg;
}

/**
* @brief Destroy a slip msg control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipMsg_Destroy(T_SLIP_MSG *pSlipMsg)
{
	if (AK_NULL == pSlipMsg)
	{
		return AK_NULL;
	}
	
	gb_UserkeyValid = AK_TRUE;

	SlipMsg_StopTimer(pSlipMsg);

	pSlipMsg = Fwl_Free(pSlipMsg);
	return AK_NULL;
}



/**
* @brief 发送消息给消息管理模块，根据消息进行状态转换
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @param in T_SLIPMSG_STRC msgstrc:the msg type and param
* @return T_BOOL
* @retval
*/
T_BOOL SlipMsg_Send(T_SLIP_MSG *pSlipMsg, T_SLIPMSG_STRC msgstrc)
{
	if (AK_NULL == pSlipMsg)
	{
		gb_UserkeyValid = AK_TRUE;
		return AK_FALSE;
	}

	switch (msgstrc.type)
	{
		case SLIPMSG_UP:
			pSlipMsg->downPoint.x = -1;
			pSlipMsg->downPoint.y = -1;

			if (SLIPMSG_STA_DOWN == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_DOWN;
				pSlipMsg->curSta = SLIPMSG_STA_STOP;
			}
			else if (SLIPMSG_STA_FOCUS == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_FOCUS;
				pSlipMsg->curSta = SLIPMSG_STA_STOP;
			}
			else if (SLIPMSG_STA_MOVE == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_MOVE;
				pSlipMsg->curSta = SLIPMSG_STA_SLIP;
			}
			break;
		case SLIPMSG_DOWN:
			pSlipMsg->downPoint.x = (T_POS)msgstrc.param1;
			pSlipMsg->downPoint.y = (T_POS)msgstrc.param2;
			
			if (SLIPMSG_STA_STOP == pSlipMsg->curSta)
			{
				if (SLIPMSG_FOCUS_ITEM == pSlipMsg->focusFlag)
				{
					pSlipMsg->lastSta = SLIPMSG_STA_STOP;
					pSlipMsg->curSta = SLIPMSG_STA_FOCUS;
				}
				else if (SLIPMSG_UNFOCUS_ITEM == pSlipMsg->focusFlag)
				{
					pSlipMsg->lastSta = SLIPMSG_STA_STOP;
					pSlipMsg->curSta = SLIPMSG_STA_DOWN;
					SlipMsg_StartTimer(pSlipMsg);
				}
				else
				{
					pSlipMsg->lastSta = SLIPMSG_STA_STOP;
					pSlipMsg->curSta = SLIPMSG_STA_STOP;
				}
			}
			else if (SLIPMSG_STA_SLIP == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_SLIP;
				pSlipMsg->curSta = SLIPMSG_STA_MOVE;
			}
			break;
		case SLIPMSG_MOVE:
			if (SLIPMSG_STA_MOVE == pSlipMsg->curSta
				||
				MOVETYPE_X == pSlipMsg->moveType && -1 != pSlipMsg->downPoint.x 
					&& DIFF(pSlipMsg->downPoint.x, (T_POS)msgstrc.param1) >= MOVE_LEN_MIN
				||
				MOVETYPE_Y == pSlipMsg->moveType && -1 != pSlipMsg->downPoint.y 
					&& DIFF(pSlipMsg->downPoint.y, (T_POS)msgstrc.param2) >= MOVE_LEN_MIN
			)
			{
				if (SLIPMSG_STA_DOWN == pSlipMsg->curSta)
				{
					pSlipMsg->lastSta = SLIPMSG_STA_DOWN;
					pSlipMsg->curSta = SLIPMSG_STA_MOVE;
				}
				else if (SLIPMSG_STA_FOCUS == pSlipMsg->curSta)
				{
					pSlipMsg->lastSta = SLIPMSG_STA_FOCUS;
					pSlipMsg->curSta = SLIPMSG_STA_MOVE;
				}
				else if (SLIPMSG_STA_MOVE == pSlipMsg->curSta)
				{
					pSlipMsg->lastSta = SLIPMSG_STA_MOVE;
					pSlipMsg->curSta = SLIPMSG_STA_MOVE;
				}
			}
			break;
		case SLIPMSG_DOWNTIMER:
			if (SLIPMSG_STA_DOWN == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_DOWN;
				pSlipMsg->curSta = SLIPMSG_STA_FOCUS;
				SlipMsg_StopTimer(pSlipMsg);
			}
			break;
		case SLIPMSG_REFRESHTIMER:
			if (SLIPMSG_STA_MOVE == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_MOVE;
				pSlipMsg->curSta = SLIPMSG_STA_MOVE;
			}
			else if (SLIPMSG_STA_SLIP == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_SLIP;
				pSlipMsg->curSta = SLIPMSG_STA_SLIP;
			}
			break;
		case SLIPMSG_SLIPOK:
			if (SLIPMSG_STA_SLIP == pSlipMsg->curSta)
			{
				pSlipMsg->lastSta = SLIPMSG_STA_SLIP;
				pSlipMsg->curSta = SLIPMSG_STA_STOP;
			}
			break;
		default:
			break;
		
	}

	if (SLIPMSG_STA_STOP == pSlipMsg->curSta)
	{
		gb_UserkeyValid = AK_TRUE;
	}
	else
	{
		gb_UserkeyValid = AK_FALSE;
	}

	return AK_TRUE;
}

/**
* @brief handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @param in T_EVT_CODE event:event
* @param in T_EVT_PARAM *pEventParm:pEventParm
* @param in T_TIMER timerId:refresh timer id
* @return T_eBACK_STATE
* @retval
*/
T_eBACK_STATE SlipMsg_Handle(T_SLIP_MSG *pSlipMsg, T_EVT_CODE event, T_EVT_PARAM *pEventParm, T_TIMER timerId)
{
	T_SLIPMSG_STRC msg;
	T_eBACK_STATE ret = eStay;
	
	if (AK_NULL == pSlipMsg)
	{
		return ret;
	}

	if (AK_NULL == pEventParm)
	{
		return ret;
	}

	msg.type = SLIPMSG_NONE;
	msg.param1 = 0;
	msg.param2 = 0;
	msg.param3 = 0;
	msg.param4 = 0;

	switch (event)
	{
		case M_EVT_TOUCH_SCREEN:
			if (eTOUCHSCR_UP == pEventParm->s.Param1)
			{
				msg.type = SLIPMSG_UP;
				msg.param1 = pEventParm->s.Param2;
				msg.param2 = pEventParm->s.Param3;
				
				SlipMsg_Send(pSlipMsg, msg);

				//由focus状态转为stop状态时，相当于确认选择了该项
				if ((SLIPMSG_STA_STOP == pSlipMsg->curSta) && (SLIPMSG_STA_FOCUS == pSlipMsg->lastSta))
				{
					pSlipMsg->lastSta = SLIPMSG_STA_STOP;
					ret = eNext;
				}
			}
			else if (eTOUCHSCR_DOWN == pEventParm->s.Param1)
			{
				msg.type = SLIPMSG_DOWN;
				msg.param1 = pEventParm->s.Param2;
				msg.param2 = pEventParm->s.Param3;
				
				SlipMsg_Send(pSlipMsg, msg);
			}
			else if (eTOUCHSCR_MOVE == pEventParm->s.Param1)
			{
				msg.type = SLIPMSG_MOVE;
				msg.param1 = pEventParm->s.Param2;
				msg.param2 = pEventParm->s.Param3;

				SlipMsg_Send(pSlipMsg, msg);
			}
			break;
		case VME_EVT_TIMER:
			if (pEventParm->w.Param1 == (T_U32)pSlipMsg->timer)
			{
				msg.type = SLIPMSG_DOWNTIMER;
				SlipMsg_Send(pSlipMsg, msg);
			}
			else if (pEventParm->w.Param1 == (T_U32)timerId)
			{
				msg.type = SLIPMSG_REFRESHTIMER;
				SlipMsg_Send(pSlipMsg, msg);
			}
			break;
		default:
			break;
	}

	return ret;
}


/**
* @brief Get Cur Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMsg_GetCurStatus(T_SLIP_MSG *pSlipMsg)
{
	if (AK_NULL == pSlipMsg)
	{
		return SLIPMSG_STA_STOP;
	}

	return pSlipMsg->curSta;
}

/**
* @brief Get Last Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMsg_GetLastStatus(T_SLIP_MSG *pSlipMsg)
{
	if (AK_NULL == pSlipMsg)
	{
		return SLIPMSG_STA_STOP;
	}

	return pSlipMsg->lastSta;
}


/**
* @brief Set focus Flag
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @param in E_FOCUS_FLAG flag:focus status
* @return T_BOOL
* @retval
*/
T_BOOL SlipMsg_SetFocusFlag(T_SLIP_MSG *pSlipMsg, E_FOCUS_FLAG flag)
{
	if (AK_NULL == pSlipMsg)
	{
		return AK_FALSE;
	}

	pSlipMsg->focusFlag = flag;

	return AK_TRUE;
}


/**
* @brief start timer
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipMsg_StartTimer(T_SLIP_MSG *pSlipMsg)
{
	if(AK_NULL == pSlipMsg)
    {
        return AK_FALSE;
    }
	
	if (ERROR_TIMER != pSlipMsg->timer)
	{
		Fwl_StopTimer(pSlipMsg->timer);
		pSlipMsg->timer = ERROR_TIMER;
	}
	
	pSlipMsg->timer = Fwl_SetTimerMilliSecond(SLIPMSG_TIME, AK_FALSE);
	return AK_TRUE;
}

/**
* @brief stop timer
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipMsg_StopTimer(T_SLIP_MSG *pSlipMsg)
{
	if(AK_NULL == pSlipMsg)
    {
        return AK_FALSE;
    }

	if (ERROR_TIMER != pSlipMsg->timer)
	{
		Fwl_StopTimer(pSlipMsg->timer);
		pSlipMsg->timer = ERROR_TIMER;
	}

	return AK_TRUE;
}

