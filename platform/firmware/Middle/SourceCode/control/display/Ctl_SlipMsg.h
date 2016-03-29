/**
 * @file Ctl_SlipMsg.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPMSG_H__
#define __CTL_SLIPMSG_H__

#include "gbl_macrodef.h"
#include "Lib_event.h"
#include "akdefine.h"
#include "Ctl_SlipMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _T_SLIPMSG_STRC
{   
	E_SLIPMSG_TYPE type;	// 消息类型
	T_U32	param1;			// 消息参数
	T_U32	param2;
	T_U32	param3;
	T_U32	param4;
}T_SLIPMSG_STRC;


/**
* @brief Creat a slip msg control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIP_MSG * the msg handle
* @retval
*/
T_SLIP_MSG *SlipMsg_Creat(E_MOVETYPE movetype);

/**
* @brief Destroy a slip msg control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipMsg_Destroy(T_SLIP_MSG *pSlipMsg);

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
T_BOOL SlipMsg_Send(T_SLIP_MSG *pSlipMsg, T_SLIPMSG_STRC msgstrc);

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
T_eBACK_STATE SlipMsg_Handle(T_SLIP_MSG *pSlipMsg, T_EVT_CODE event, T_EVT_PARAM *pEventParm, T_TIMER timerId);

/**
* @brief Get Cur Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMsg_GetCurStatus(T_SLIP_MSG *pSlipMsg);

/**
* @brief Get Last Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_MSG *pSlipMsg:the msg handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMsg_GetLastStatus(T_SLIP_MSG *pSlipMsg);


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
T_BOOL SlipMsg_SetFocusFlag(T_SLIP_MSG *pSlipMsg, E_FOCUS_FLAG flag);


#ifdef __cplusplus
}
#endif

#endif
