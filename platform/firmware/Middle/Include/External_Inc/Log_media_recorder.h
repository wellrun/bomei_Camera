/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name:log_media_recorder.h
 * Function: video recorder logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/

#ifndef H_LOG_MEDIA_RECORDER_WX_2011_09
#define H_LOG_MEDIA_RECORDER_WX_2011_09
#include "log_MediaEncode.h"
#include "log_VideoZoom.h"
#include "log_videoDisp.h"

#ifdef CAMERA_SUPPORT

#define USER_EVT_STOP_REC     (0x11)

/*
 * @brief   open video recorder handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pAudioParm: default audio encode param	
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE MRec_Open(T_REC_AUDIO_INIT_PARAM *pAudioParm);

/*
 * @brief   start video record  porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in/out] srcCtlHdl: zoom handle
 * @param[in] pRecCtlParam: record control param	
 * @param[in] pAudioParm: audio encode param	
 * @param[in] pRecVideoParam: video encode param	
 * @return	resulst REC_ERROR_OK--success, else fail
 */
T_REC_ERROR_STATUS MRec_Start(T_HANDLE handle,T_HANDLE srcCtlHdl, T_REC_CTRL_INIT_PARAM   *pRecCtlParam, 
	T_REC_AUDIO_INIT_PARAM  *pRecAudioParam,T_REC_VIDEO_INIT_PARAM  *pRecVideoParam);

/*
 * @brief   stop video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSave:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Stop(T_HANDLE handle, T_BOOL isSave);

/*
 * @brief   pause video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Pause(T_HANDLE handle);

/*
 * @brief   close video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSave:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Close(T_HANDLE handle, T_BOOL isSave);

/*
 * @brief   get recorder information by ctrltype
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] ctlType: ctrl type
 * @param[out] arg:information buffer
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Ioctl(T_HANDLE handle, T_eMREC_IOCTL ctlType, T_VOID *arg);


/*
 * @brief   restart video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSavePre:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_REC_ERROR_STATUS MRec_Restart(T_HANDLE hdl, T_BOOL isSavePre);
#endif


#endif

