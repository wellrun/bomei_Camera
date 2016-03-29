/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name:Log_MotionDetec.h
 * Function: video MotionDetec logic
 * Author:  wangxi
 * Date:  2010-12-11
 * Version: 1.0
 *
 ***********************************************************************/

#ifndef __LOG_MOTIONDETEC_H__
#define __LOG_MOTIONDETEC_H__


#include "motion_detector_lib.h"

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

T_HANDLE MDetect_Open(T_U32 width,T_U32 height,T_U32 Threshold);
T_VOID MDetect_Close(T_HANDLE hdl);
T_BOOL MDetect_Start(T_HANDLE hdl, T_U32 width, T_U32 height);
T_VOID MDetect_Stop(T_HANDLE hdl);
T_VOID MDetect_UpdateData(T_HANDLE hdl, T_U8 *pData);
T_BOOL MDetect_IsMoving(T_HANDLE hdl);
T_BOOL MDetect_IsRun(T_HANDLE hdl);

/**
 * @brief get detect windows size 
 * @param[in]     hdl   :handle for detect
 * @param[out] pWidth:width of  detect windows
 * @param[out] pHeight:width of  detect windows
 * @return AK_TRUE for success, AK_FALSE  for failure
 */
T_BOOL MDetect_GetWin(T_HANDLE hdl,T_U16 * pWidth,T_U16 * pHeight);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif

