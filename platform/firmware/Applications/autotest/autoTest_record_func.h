
/** 
 * @file autotest_record_func.h
 * @brief rtc module control
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author li_xingjian
 * @date 2012.02.28
 * @version 1.0
 */
#ifndef __AUTOTEST_RECORD_H__
#define __AUTOTEST_RECORD_H__

#ifdef SUPPORT_AUTOTEST


#include "anyka_types.h"
#include "Fwl_osFS.h"
#include "fwl_tscrcom.h"



/**
 * @brief  creat and  open  the file for script file
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_BOOL
 * @retval   successful
 * @retval   error
 */
T_BOOL autotest_record_openfile(T_VOID);




/**
 * @brief   close and save or del the file for script file
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_BOOL
 * @retval   successful
 * @retval   error
 */

T_VOID autotest_record_closefile(T_BOOL safeflag);



/**
 * @brief   register which key press 
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_BOOL
 * @retval   successful
 * @retval   error
 */
T_BOOL autotest_record_statement(T_U8 keyID, T_U8 presstype);


/**
 * @brief   register which tscr press  info
 *
 * @author  Lixingjian
 * @date    2012-04-10
 * @param   [in] autotest
 * @return  T_BOOL
 * @retval   successful
 * @retval   error
 */
T_BOOL autotest_record_Tscr(T_TOUCHSCR_ACTION act, T_S16 pointx, T_S16 pointy);



#endif

#endif


