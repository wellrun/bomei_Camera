/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_videoZoom.h
 * Function:camera output asyn zoom logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/
#ifndef __LOG_VIDEOZOOM_H__
#define __LOG_VIDEOZOOM_H__

#include "Anyka_Types.h"
#include "AKError.h"
#include "Eng_Debug.h"
#include "Fwl_osMalloc.h"
#include "AKOS_Api.h"
#include "AKDefine.h"
#include "AKFrameStream.h"
#include "Fwl_pfCamera.h"

#define ZOOM_STRM_ORDER_FIRST    (0)
#define ZOOM_STRM_ORDER_LAST     (-1)


typedef T_S32  (*T_ZOOM_FRM_PROC)(T_FRM_DATA *pFrame, T_HANDLE hdl);
typedef T_S32  (*T_ZOOM_FRM_GET_ORDER)(T_HANDLE hdl);


/*
 * @brief   open video zoom handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in] focusLvl: default focuslvevel	
 * @param[in] width/height:video srouce size	
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE VideoZoom_Open(T_U8 focusLvl, T_U32 width, T_U32 height);


/*
 * @brief   start video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_Start(T_HANDLE hdl);

/*
 * @brief   close video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_Close(T_HANDLE hdl);


/*
 * @brief   pause video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_Pause(T_HANDLE hdl);


/*
 * @brief   restart video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] width/height:video srouce size	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_Restart(T_HANDLE hdl, T_U32 width, T_U32 height);


/*
 * @brief   open camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_OpenVideoSrc(T_HANDLE hdl, T_U32 width, T_U32 height);

/*
 * @brief   restart camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_RestartVideoSrc(T_HANDLE hdl, T_U32 width, T_U32 height);

/*
 * @brief   close camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_CloseVideoSrc(T_HANDLE hdl);

/*
 * @brief   set camera stream source focus level information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] focusLvl: focus level
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetFocusLvl(T_HANDLE hdl, T_U8 focusLvl, T_BOOL isOnVideo);

/*
 * @brief   get camera focus windows
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[out] rect: focus windows
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_GetFoucsWin(T_HANDLE hdl, T_RECT *rect);

/*
 * @brief   get camera focus is enable
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_TRUE--enable, else disable
 */
T_BOOL VideoZoom_IsEnableFocusWin(T_HANDLE hdl);

/*
 * @brief   open encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in/out] EncFrameInfo: stream information
 * @return  resulst AK_SUCCESS--success, else fail
 * @return  resulst AK_TRUE--enable, else disable
 */
T_S32  VideoZoom_OpenEncSrc(T_HANDLE hdl,T_FRM_INFO *EncFrameInfo, T_BOOL isEnableFocus);

/*
 * @brief   open display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in/out] EncFrameInfo: stream information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_OpenDispSrc(T_HANDLE hdl,T_FRM_INFO *DispFrameInfo);

/*
 * @brief   close encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_CloseEncSrc(T_HANDLE hdl);

/*
 * @brief   close display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_CloseDispSrc(T_HANDLE hdl);

/*
 * @brief   enable encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnable: is enable
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_EnableEncSrc(T_HANDLE hdl, T_BOOL isEnable);

/*
 * @brief   enable display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnable: is enable
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_EnableDispSrc(T_HANDLE hdl, T_BOOL isEnable);

/*
 * @brief   get  encode stream zoom process is enable
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_TRUE--enable, else disable
 */
T_BOOL VideoZoom_IsEnableEnc(T_HANDLE hdl);

/*
 * @brief   get  display stream zoom process is enable
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_TRUE--enable, else disable
 */
T_BOOL VideoZoom_IsEnableDisp(T_HANDLE hdl);

/*
 * @brief   set  encode stream zoom process information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pInfo: zoom information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetEncInfo(T_HANDLE hdl, T_FRM_INFO *pInfo);

/*
 * @brief   set  display stream zoom process information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pInfo: zoom information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetDispInfo(T_HANDLE hdl, T_FRM_INFO *pInfo);

/*
 * @brief   set  encode stream zoom process prepare callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom prepare callback
 * @param[in] param: oom prepare  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetEncPreProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param);


/*
 * @brief   set  encode stream zoom process post callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom post callback
 * @param[in] param: zoom post callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetEncPostProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param);

/*
 * @brief   set  display stream zoom process post callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom post callback
 * @param[in] param: zoom post callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetDispPostProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param);

/*
 * @brief   set  display stream zoom process prepare callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom prepare callback
 * @param[in] param: oom prepare  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetDispPreProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param);

/*
 * @brief   set  display stream zoom process older judgement  callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: older judgement callback
 * @param[in] param: older judgement  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_SetDispOlderProc(T_HANDLE hdl, T_ZOOM_FRM_GET_ORDER proc, T_HANDLE param);

/*
 * @brief   get  frame from encode stream by process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] procFun: get process
 * @param[in] param: get process callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_GetEncFrameByProc(T_HANDLE hdl, T_STEM_GET_PROC procFun,T_pVOID param);

/*
 * @brief   get  frame from display stream by process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] procFun: get process
 * @param[in] param: get process callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_GetDispFrameByProc(T_HANDLE hdl, T_STEM_GET_PROC procFun,T_pVOID param);

/*
 * @brief   touch  display stream status by frame buffer address
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] addr: frame buffer address
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_TouchDispFrameByAddr(T_HANDLE hdl, T_U8 *addr);

/*
 * @brief   touch  encode stream status by frame buffer address
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] addr: frame buffer address
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_TouchEncFrameByAddr(T_HANDLE hdl, T_U8 *addr);

/*
 * @brief   enable motion detect function
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnalbe: is open
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_DetectEnable(T_HANDLE hdl, T_BOOL isEnalbe);

/*
 * @brief   motion detect  current status is mvoing
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pData: current detect data
 * @return  resulst AK_TRUE--moving, else no moving
 */
T_BOOL VideoZoom_DetectIsMoving(T_HANDLE hdl, T_U8 *pData);


/*
 * @brief   motion detect  current status is same with the parmeter isMoving
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isMoving: detect status check
 * @return  resulst AK_TRUE--same, else not same
 */
T_BOOL VideoZoom_DetectStatusCheck(T_HANDLE hdl, T_BOOL isMoving);

/*
 * @brief   set  motion detect interval
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] intervalMs: detect interval time(millsecond)
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_DetectSetInterval(T_HANDLE hdl, T_U32 intervalMs);


#endif
