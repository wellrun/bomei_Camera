/**
 * @file Ctl_SlipCalc.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPCALC_H__
#define __CTL_SLIPCALC_H__

#include "gbl_macrodef.h"
#include "Lib_event.h"
#include "akdefine.h"
#include "ctl_SlipMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	TOTAL_SLIP_TIME		(2000)
#define	SLIP_REFRESH_TIME	(50)


/**
* @brief Creat a slip calc control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIP_CALC * the calc handle
* @retval
*/
T_SLIP_CALC *SlipCalc_Creat(E_MOVETYPE movetype);

/**
* @brief Destroy a slip calc control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipCalc_Destroy(T_SLIP_CALC *pSlipCalc);


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
T_S32 SlipCalc_Handle(T_SLIP_CALC *pSlipCalc, T_EVT_CODE event, T_EVT_PARAM *pEventParm, T_TIMER timerId, T_BOOL bSlip, T_U8* SlipOK, T_S32 overLen, T_RECT rect);

/**
* @brief rebound,change move direction
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipCalc_Rebound(T_SLIP_CALC *pSlipCalc);

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
T_BOOL SlipCalc_SetV0AndAByLen(T_SLIP_CALC *pSlipCalc, T_S32 Length);

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
T_BOOL SlipCalc_SetMoveDirectionByKey(T_SLIP_CALC *pSlipCalc, T_eKEY_ID keyid);

/**
* @brief get move direction
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return E_MOVE_DIRECTION move direction
* @retval
*/
E_MOVE_DIRECTION SlipCalc_GetMoveDirection(T_SLIP_CALC *pSlipCalc);


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
T_BOOL SlipCalc_SetRemainLen(T_SLIP_CALC *pSlipCalc, T_S32 nextRemainLen, T_S32 preRemainLen);

/**
* @brief 获取实际的剩余长度
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_CALC *pSlipCalc : the calc handle
* @return T_S32 actualLength
* @retval
*/
T_S32 SlipCalc_GetRemainLen(T_SLIP_CALC *pSlipCalc);


#ifdef __cplusplus
}
#endif

#endif
