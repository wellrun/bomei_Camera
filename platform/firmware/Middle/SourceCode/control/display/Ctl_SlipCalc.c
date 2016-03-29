/**
 * @file Ctl_SlipCalc.c
 * @brief ANYKA software
 * 这里的位移以象素为单位，时间以一个刷新timer的时长为1个时间单位。
 * 速度等以(象素/ 刷新timer时长)为单位
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "ctl_slipCalc.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "Fwl_tscrcom.h"


#define V0_MAX	(20)

static T_S32 SlipCalc_CalcSlip(T_SLIP_CALC *pSlipCalc, T_U8* SlipOK);
static T_S32 SlipCalc_CalcMove(T_SLIP_CALC *pSlipCalc, T_S32 overLen, T_RECT rect);
static T_BOOL SlipCalc_TouchScrHandle(T_SLIP_CALC *pSlipCalc, T_EVT_PARAM *pEventParm, T_RECT rect);
static T_S32 SlipCalc_TimerHandle(T_SLIP_CALC *pSlipCalc, T_EVT_PARAM *pEventParm, T_TIMER timerId, T_BOOL bSlip, T_U8* SlipOK, T_S32 overLen, T_RECT rect);


/**
* @brief Creat a slip calc control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIP_CALC * the calc handle
* @retval
*/
T_SLIP_CALC *SlipCalc_Creat(E_MOVETYPE movetype)
{
	T_SLIP_CALC *pSlipCalc = AK_NULL;

	if (movetype > MOVETYPE_Y)
	{
		return AK_NULL;
	}
	
	pSlipCalc = (T_SLIP_CALC *)Fwl_Malloc(sizeof(T_SLIP_CALC));

	AK_ASSERT_PTR(pSlipCalc, "SlipCalc_Init(): pSlipCalc malloc error", AK_NULL);
	memset(pSlipCalc, 0, sizeof(T_SLIP_CALC));

	pSlipCalc->moveType = movetype;

	return pSlipCalc;

}

/**
* @brief Destroy a slip calc control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipCalc_Destroy(T_SLIP_CALC *pSlipCalc)
{
	if (AK_NULL == pSlipCalc)
	{
		return AK_NULL;
	}

	pSlipCalc = Fwl_Free(pSlipCalc);
	return AK_NULL;
}



/**
* @brief handle function of the slip calc control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_EVT_CODE event:event
* @param in T_EVT_PARAM *pEventParm:pEventParm
* @param in T_TIMER timerId:refresh timer id
* @param in T_BOOL bSlip:slip or not
* @param out T_U8* SlipOK:slip complete or not
* @param in T_S32 overLen:cur over len
* @param in T_RECT rect:rect
* @return T_S32 offset(位移)
* @retval
*/
T_S32 SlipCalc_Handle(T_SLIP_CALC *pSlipCalc, T_EVT_CODE event, T_EVT_PARAM *pEventParm, T_TIMER timerId, T_BOOL bSlip, T_U8* SlipOK, T_S32 overLen, T_RECT rect)
{
	T_S32 ret = 0;
	
	if (AK_NULL == pSlipCalc)
	{
		return ret;
	}

	if ((AK_NULL == pEventParm) || (AK_NULL == SlipOK))
	{
		return ret;
	}

	switch (event)
	{
		case M_EVT_TOUCH_SCREEN:
			SlipCalc_TouchScrHandle(pSlipCalc, pEventParm, rect);
			break;
		case VME_EVT_TIMER:
			ret = SlipCalc_TimerHandle(pSlipCalc, pEventParm, timerId, bSlip, SlipOK, overLen, rect);
			break;
		default:
			break;
	}

	return ret;
}

/**
* @brief rebound,change move direction
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipCalc_Rebound(T_SLIP_CALC *pSlipCalc)
{
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	if (SLIPCALC_MOVE_DOWN == pSlipCalc->moveDirection)
	{
		pSlipCalc->moveDirection = SLIPCALC_MOVE_UP;
	}
	else if (SLIPCALC_MOVE_UP == pSlipCalc->moveDirection)
	{
		pSlipCalc->moveDirection = SLIPCALC_MOVE_DOWN;
	}
	else if (SLIPCALC_MOVE_RIGHT == pSlipCalc->moveDirection)
	{
		pSlipCalc->moveDirection = SLIPCALC_MOVE_LEFT;
	}
	else if (SLIPCALC_MOVE_LEFT == pSlipCalc->moveDirection)
	{
		pSlipCalc->moveDirection = SLIPCALC_MOVE_RIGHT;
	}
			
	return AK_TRUE;
}

/**
* @brief calc v0 and acceleration by length
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_S32 Length : length
* @return T_BOOL
* @retval
*/
T_BOOL SlipCalc_SetV0AndAByLen(T_SLIP_CALC *pSlipCalc, T_S32 Length)
{
	T_U32 len = 0;
	float time = TOTAL_SLIP_TIME / SLIP_REFRESH_TIME / 2;
		
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	len = Length >= 0 ? Length : 0 - Length;

	//vt = v0 + a * t，令vt = 0，得到a = -v0 / t， 带入len = v0*t + a * t * t / 2中得到v0 = 2 * len / t
	pSlipCalc->V0 = 2 * (float)len / time;
	pSlipCalc->Vt = pSlipCalc->V0;
	pSlipCalc->a = 0 - pSlipCalc->V0 / time;

	pSlipCalc->totalLen = (T_S32)(pSlipCalc->V0 * time + pSlipCalc->a * time * time / 2);
	pSlipCalc->S = 0;
	pSlipCalc->time = 0;	
	
	return AK_TRUE;
}


/**
* @brief set move direction by userkey id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_eKEY_ID keyid : userkey id
* @return T_BOOL
* @retval
*/
T_BOOL SlipCalc_SetMoveDirectionByKey(T_SLIP_CALC *pSlipCalc, T_eKEY_ID keyid)
{		
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	if (MOVETYPE_Y == pSlipCalc->moveType)
	{
		if ((kbUP == keyid) || (kbLEFT == keyid))
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_DOWN;
		}
		else if ((kbDOWN == keyid) || (kbRIGHT == keyid))
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_UP;
		}
	}
	else if (MOVETYPE_X == pSlipCalc->moveType)
	{
		if ((kbUP == keyid) || (kbLEFT == keyid))
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_RIGHT;
		}
		else if ((kbDOWN == keyid) || (kbRIGHT == keyid))
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_LEFT;
		}
	}
	
	return AK_TRUE;
}


/**
* @brief get move direction
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return E_MOVE_DIRECTION move direction
* @retval
*/
E_MOVE_DIRECTION SlipCalc_GetMoveDirection(T_SLIP_CALC *pSlipCalc)
{		
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	return pSlipCalc->moveDirection;
}


/**
* @brief 设置实际的剩余长度
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_S32 nextRemainLen : 后面的实际剩余长度
* @param in T_S32 preRemainLen : 前面的实际剩余长度
* @return T_BOOL
* @retval
*/
T_BOOL SlipCalc_SetRemainLen(T_SLIP_CALC *pSlipCalc, T_S32 nextRemainLen, T_S32 preRemainLen)
{		
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	if (MOVETYPE_Y == pSlipCalc->moveType)
	{
		if (SLIPCALC_MOVE_DOWN == pSlipCalc->moveDirection)
		{
			pSlipCalc->actualLen = preRemainLen;
		}
		else if (SLIPCALC_MOVE_UP == pSlipCalc->moveDirection)
		{
			pSlipCalc->actualLen = nextRemainLen;
		}
	}
	else if (MOVETYPE_X == pSlipCalc->moveType)
	{
		if (SLIPCALC_MOVE_RIGHT == pSlipCalc->moveDirection)
		{
			pSlipCalc->actualLen = preRemainLen;
		}
		else if (SLIPCALC_MOVE_LEFT == pSlipCalc->moveDirection)
		{
			pSlipCalc->actualLen = nextRemainLen;
		}
	}
	
	return AK_TRUE;
}

/**
* @brief 获取实际的剩余长度
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return T_S32 actualLen
* @retval
*/
T_S32 SlipCalc_GetRemainLen(T_SLIP_CALC *pSlipCalc)
{
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	return pSlipCalc->actualLen;
}


/**
* @brief 惯性滑动速度、位移计算
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param out T_U8* SlipOK : slip complete or not
* @return T_S32 滑动位移
* @retval
*/
static T_S32 SlipCalc_CalcSlip(T_SLIP_CALC *pSlipCalc, T_U8* SlipOK)
{
	T_S32 ret = 0;
	T_S32 length = 0;
	T_S32 reboundlen = 0;
	T_U32 time = 0;
	float length_tmp = 0;
	
	if (AK_NULL == pSlipCalc)
	{
		return 0;
	}

	if (AK_NULL == SlipOK)
	{
		return 0;
	}
	
	pSlipCalc->bStillFlag = AK_FALSE;
	pSlipCalc->time++;


	if (pSlipCalc->bReboundFlag)
	{
		AK_DEBUG_OUTPUT("rebound!\n");
		//回弹，方向取反
		if (SLIPCALC_MOVE_DOWN == pSlipCalc->moveDirection)
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_UP;
		}
		else if (SLIPCALC_MOVE_UP == pSlipCalc->moveDirection)
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_DOWN;
		}
		else if (SLIPCALC_MOVE_RIGHT == pSlipCalc->moveDirection)
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_LEFT;
		}
		else if (SLIPCALC_MOVE_LEFT == pSlipCalc->moveDirection)
		{
			pSlipCalc->moveDirection = SLIPCALC_MOVE_RIGHT;
		}
		
		pSlipCalc->time = 1;
		pSlipCalc->V0 = pSlipCalc->Vt;
		pSlipCalc->S = 0;
		pSlipCalc->bReboundFlag = AK_FALSE;
	}

	//vt = v0 + a * t
	pSlipCalc->Vt = pSlipCalc->V0 + pSlipCalc->a * pSlipCalc->time;
					
	//len = v0*t + a * t * t / 2
	length_tmp = pSlipCalc->V0 * pSlipCalc->time
			+ pSlipCalc->a * pSlipCalc->time * pSlipCalc->time / 2;

	length = (T_S32)length_tmp;

	//为减小误差，四舍五入
	if (length_tmp - (float)length > 0.5)
	{
		length += 1;
	}

	if (length > pSlipCalc->actualLen + (pSlipCalc->totalLen - pSlipCalc->actualLen) / 2)
	{
		//路程差＝totalLength -actualLength，当Slip到actualLength＋路程差/2的时候，剩余路程差/2，回弹。
		length = pSlipCalc->actualLen + (pSlipCalc->totalLen - pSlipCalc->actualLen) / 2;
		reboundlen = (pSlipCalc->totalLen - pSlipCalc->actualLen) / 2;
		pSlipCalc->V0 = pSlipCalc->Vt;

		if ((reboundlen > 1) && (pSlipCalc->V0 > 0))
		{
			pSlipCalc->bReboundFlag = AK_TRUE;
			//vt = v0 + a * t，令vt = 0，得到a = -v0 / t， 带入len = v0*t + a * t * t / 2中得到t = 2 * len / v0, v0 = 2 * len / t
			time = (T_U32)(2 * (float)reboundlen / pSlipCalc->V0);
			
			//为减小误差，修正v0
			pSlipCalc->V0 = 2 * (float)reboundlen / (float)time;
			pSlipCalc->a = 0 - pSlipCalc->V0 / time;
			pSlipCalc->Vt = pSlipCalc->V0;
		}
	}

	if (SLIPCALC_MOVE_DOWN == pSlipCalc->moveDirection)
	{
		ret = length - pSlipCalc->S;
	}
	else if (SLIPCALC_MOVE_UP == pSlipCalc->moveDirection)
	{
		ret = pSlipCalc->S - length;
	}
	else if (SLIPCALC_MOVE_RIGHT == pSlipCalc->moveDirection)
	{
		ret = length - pSlipCalc->S;
	}
	else if (SLIPCALC_MOVE_LEFT == pSlipCalc->moveDirection)
	{
		ret = pSlipCalc->S - length;
	}

	pSlipCalc->S = length;
	
	if (pSlipCalc->Vt <= 0)
	{
		//速度为0了，惯性滑动结束
		*SlipOK = 1;
		
		AK_DEBUG_OUTPUT("SlipOK!\n");
	}
	
	return ret;
}


/**
* @brief 移动速度、位移计算
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_S32 overLen:cur over len
* @param in T_RECT rect:rect
* @return T_S32 移动位移
* @retval
*/
static T_S32 SlipCalc_CalcMove(T_SLIP_CALC *pSlipCalc, T_S32 overLen, T_RECT rect)
{
	T_S32 ret = 0;
	float time = 0;

	if (AK_NULL == pSlipCalc)
	{
		return 0;
	}
	
	pSlipCalc->time = 0;
	pSlipCalc->S = 0;
	
	if (MOVETYPE_Y == pSlipCalc->moveType)
	{
		//move的位移
		ret = pSlipCalc->movePointNew.y - pSlipCalc->movePointOld.y;

		if (0 != overLen)
		{
			ret /= 2;
		}

		if (0 == ret)
		{
			if (!pSlipCalc->bStillFlag)
			{
				pSlipCalc->bStillFlag = AK_TRUE;
				return ret;
			}
		}

		if (ret >= 0)
		{
			if (ret >= rect.height / 2)
			{
				ret = rect.height / 2;
			}
			
			pSlipCalc->V0 = (float)ret;
			pSlipCalc->moveDirection = SLIPCALC_MOVE_DOWN;
		}
		else
		{
			if (ret <= 0 - rect.height / 2)
			{
				ret = 0 - rect.height / 2;
			}
			
			pSlipCalc->V0 = (float)(0 - ret);
			pSlipCalc->moveDirection = SLIPCALC_MOVE_UP;
		}

		pSlipCalc->bStillFlag = AK_FALSE;

		//速度限制最大值
		if (pSlipCalc->V0 > V0_MAX)
		{
			pSlipCalc->V0 = V0_MAX;
		}
		else if (pSlipCalc->V0 < 0 - V0_MAX)
		{
			pSlipCalc->V0 = 0 - V0_MAX;
		}
		
		time = TOTAL_SLIP_TIME / SLIP_REFRESH_TIME;
		pSlipCalc->Vt = pSlipCalc->V0;
		pSlipCalc->a = 0 - pSlipCalc->V0 / time;
		pSlipCalc->totalLen = (T_S32)(pSlipCalc->V0 * time + pSlipCalc->a * time * time / 2);
		
	}
	else if (MOVETYPE_X == pSlipCalc->moveType)
	{
		//move的位移
		ret = pSlipCalc->movePointNew.x - pSlipCalc->movePointOld.x;

		if (0 != overLen)
		{
			ret /= 2;
		}

		if (0 == ret)
		{
			if (!pSlipCalc->bStillFlag)
			{
				pSlipCalc->bStillFlag = AK_TRUE;
				return ret;
			}
		}
		
		if (ret >= 0)
		{
			if (ret >= rect.width / 2)
			{
				ret = rect.width / 2;
			}
			
			pSlipCalc->V0 = (float)ret;
			pSlipCalc->moveDirection = SLIPCALC_MOVE_RIGHT;
		}
		else
		{
			if (ret <= 0 - rect.width / 2)
			{
				ret = 0 - rect.width / 2;
			}
			
			pSlipCalc->V0 = (float)(0 - ret);
			pSlipCalc->moveDirection = SLIPCALC_MOVE_LEFT;
		}

		pSlipCalc->bStillFlag = AK_FALSE;

		//速度限制最大值
		if (pSlipCalc->V0 > V0_MAX)
		{
			pSlipCalc->V0 = V0_MAX;
		}
		else if (pSlipCalc->V0 < 0 - V0_MAX)
		{
			pSlipCalc->V0 = 0 - V0_MAX;
		}
		
		time = TOTAL_SLIP_TIME / SLIP_REFRESH_TIME;
		pSlipCalc->Vt = pSlipCalc->V0;
		pSlipCalc->a = 0 - pSlipCalc->V0 / time;
		pSlipCalc->totalLen = (T_S32)(pSlipCalc->V0 * time + pSlipCalc->a * time * time / 2);
	}
	
	pSlipCalc->movePointOld.x = pSlipCalc->movePointNew.x;
	pSlipCalc->movePointOld.y = pSlipCalc->movePointNew.y;

	return ret;
}


/**
* @brief touch screen handle
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_EVT_PARAM *pEventParm:pEventParm
* @param in T_RECT rect:rect
* @return T_BOOL
* @retval
*/
static T_BOOL SlipCalc_TouchScrHandle(T_SLIP_CALC *pSlipCalc, T_EVT_PARAM *pEventParm, T_RECT rect)
{
	if (AK_NULL == pSlipCalc)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pEventParm)
	{
		return AK_FALSE;
	}

	switch (pEventParm->s.Param1)
	{
		case eTOUCHSCR_UP:
			pSlipCalc->upPoint.x = pEventParm->s.Param2;
			pSlipCalc->upPoint.y = pEventParm->s.Param3;

			if (MOVETYPE_Y == pSlipCalc->moveType)
			{
				if (pSlipCalc->totalLen > pSlipCalc->actualLen + rect.height)
				{
					pSlipCalc->totalLen = pSlipCalc->actualLen + rect.height;
				}
			}
			else if (MOVETYPE_X == pSlipCalc->moveType)
			{
				if (pSlipCalc->totalLen > pSlipCalc->actualLen + rect.width)
				{
					pSlipCalc->totalLen = pSlipCalc->actualLen + rect.width;
				}
			}

			AK_DEBUG_OUTPUT("eTOUCHSCR_UP V0 = %f totallen = %d, actlen = %d\n", pSlipCalc->V0, pSlipCalc->totalLen, pSlipCalc->actualLen);
			break;
		case eTOUCHSCR_DOWN:
			pSlipCalc->downPoint.x = pEventParm->s.Param2;
			pSlipCalc->downPoint.y = pEventParm->s.Param3;
			pSlipCalc->movePointOld.x = pEventParm->s.Param2;
			pSlipCalc->movePointOld.y = pEventParm->s.Param3;
			pSlipCalc->movePointNew.x = pEventParm->s.Param2;
			pSlipCalc->movePointNew.y = pEventParm->s.Param3;

			pSlipCalc->bStillFlag = AK_FALSE;
			pSlipCalc->V0 = 0;
			pSlipCalc->Vt = 0;
			pSlipCalc->a = 0;
			pSlipCalc->totalLen = 0;
			pSlipCalc->actualLen = 0;
			pSlipCalc->S = 0;
			pSlipCalc->bReboundFlag = AK_FALSE;
			break;
		case eTOUCHSCR_MOVE:
			pSlipCalc->movePointNew.x = pEventParm->s.Param2;
			pSlipCalc->movePointNew.y = pEventParm->s.Param3;
			break;
		default:
			break;
	}

	return AK_TRUE;
}


/**
* @brief timer handle
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @param in T_EVT_PARAM *pEventParm:pEventParm
* @param in T_TIMER timerId:refresh timer id
* @param in T_BOOL bSlip:slip or not
* @param out T_U8* SlipOK:slip complete or not
* @param in T_S32 overLen:cur over len
* @param in T_RECT rect:rect
* @return T_S32 offset(位移)
* @retval
*/
static T_S32 SlipCalc_TimerHandle(T_SLIP_CALC *pSlipCalc, T_EVT_PARAM *pEventParm, T_TIMER timerId, T_BOOL bSlip, T_U8* SlipOK, T_S32 overLen, T_RECT rect)
{
	T_S32 ret = 0;
	
	if (AK_NULL == pSlipCalc)
	{
		return ret;
	}

	if ((AK_NULL == pEventParm) || (AK_NULL == SlipOK))
	{
		return ret;
	}

	if (timerId == (T_TIMER)pEventParm->w.Param1)
	{
		if (bSlip)
		{
			//slip
			ret = SlipCalc_CalcSlip(pSlipCalc, SlipOK);
		}
		else
		{
			//move
			ret = SlipCalc_CalcMove(pSlipCalc, overLen, rect);
		}
	}

	return ret;
}

