/**************************************************************************************
 * @file Eng_PowerOnThread.h
 * @brief Power Thread Implementation for executing time-consuming operate. it will exit after run once.
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author HouBangen
 * @date 2011-12-21
 * @version 1.0
 **************************************************************************************/

#ifndef __ENG_POWERONTHREAD_H__
#define __ENG_POWERONTHREAD_H__

#include "anyka_types.h"


/**
 * @brief 	create a poweron thread
 * @author 	HouBangen
 * @return 	T_BOOL 
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE		Success
 */
T_VOID CPowerOnThread_New(T_VOID);


#endif //__ENG_POWERONTHREAD_H__