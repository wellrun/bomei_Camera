/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_videoDisp.h
 * Function:video asyn display logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/
#ifndef __LOG_VIDEODISP_H__
#define __LOG_VIDEODISP_H__

#include "Fwl_display.h"

/**
 * the callback type for paint icon on video
 */
typedef T_S32  (*VIDEO_DISP_VIDEO_UI_CBF)(T_FRM_DATA *pDispFrame);

/**
 * the callback type for paint pop menu on video
 */
typedef T_S32  (*VIDEO_DISP_MENU_UI_CBF)(T_VOID);

/**
 * the callback type for if need paint pop menu
 */
typedef T_BOOL (*VIDEO_DISP_PAINT_UI_CHECK_CBF)(T_VOID);


/*
 * @brief   open video display handle
 * @author WangXi
 * @date	2011-10-25
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE VideoDisp_Open(T_VOID);

/*
 * @brief   close video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Close(T_HANDLE hdl);

/*
 * @brief   start video display porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in/out] srcCtlHdl: zoom handle
 * @param[in] tvOutMode:video srouce size	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Start(T_HANDLE hdl,T_HANDLE srcCtlHdl, DISPLAY_TYPE_DEV tvOutMode);

/*
 * @brief   pause video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Pause(T_HANDLE hdl);

/*
 * @brief   restart video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Restart(T_HANDLE hdl);

/*
 * @brief   switch video refresh mode by  display mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] tvOutMode: display mode
 * @param[in] brightness: display brightness val
 * @return	resulst AK_SUCCESS--success, else fail
 */
//T_S32 VideoDisp_SwitchTvOut(T_HANDLE hdl, DISPLAY_TYPE_DEV tvOutMode, T_U8 brightness);

/*
 * @brief   close video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] videoUiCbf:paint icon on video frame  cbf	
 * @param[in] isPaintMenuCbf:is show pop menu cbf	
 * @param[in] menuPainCbf:paint pop menu cbf	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_SetPaintUiCbf(T_HANDLE hdl,VIDEO_DISP_VIDEO_UI_CBF videoUiCbf,
	VIDEO_DISP_PAINT_UI_CHECK_CBF isPaintMenuCbf,VIDEO_DISP_MENU_UI_CBF menuPainCbf);


#endif


