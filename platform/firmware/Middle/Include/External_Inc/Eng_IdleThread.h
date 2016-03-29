/**
 * @file Eng_IdleThread.h
 * @brief Idle Thread Interface for Running A Idle Thread Read Someone Register to Save Power
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Wang_GouTian, Xie_Wenzhong
 * @date 2011-11-3
 * @version 1.0
 * @note The following is an example to use playing APIs
 * @code

 IdleThread_Create();

 ... ...

 IdleThread_IsCreate();

 ... ...

 IdleThread_Destroy();
 */

#ifndef _ENG_IDLETHREAD_H_
#define _ENG_IDLETHREAD_H_

#include "anyka_types.h"
#ifdef OS_ANYKA
/**
 * @brief 	Create A Idle Thread
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL 
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */ 
T_BOOL IdleThread_Create(T_VOID);

/**
 * @brief 	Destroy The Idle Thread
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL 
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */ 
T_BOOL IdleThread_Destroy(T_VOID);

/**
 * @brief 	Query The Idle Thread Is Running
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL 
 * @retval	AK_FALSE	NO A Idle Thead Is running
 * @retval	AK_TRUE	The Idle Thead Is running
 */ 
T_BOOL IdleThread_IsCreate(T_VOID);

/**
 * @brief 	get cpu idle percent
 * @author 
 * @return 	T_U32  
 * @retval	T_U32 cpu idle percent
 */ 
T_U32 Idle_GetCpuIdle(T_VOID);

T_U32 Idle_GetCpuUsage(T_VOID);


#endif	// OS_ANYKA
#endif	// _ENG_IDLETHREAD_H_
