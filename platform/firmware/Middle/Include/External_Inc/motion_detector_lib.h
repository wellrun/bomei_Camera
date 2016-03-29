/**
 * @file Motion_detector_lib.h
 * @brief This file provides 3GP/AVI Motion Detection recording functions
 *
 * Copyright (C) 2009 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Hu Jing 
 * @date 2010-5-12
 * @version 0.1.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use playing APIs
   @code

T_VOID main(int argc, char* argv[])
{
	T_MOTION_DETECTOR_OPEN_PARA 	detector_open_para;
	T_VOID *hMdetector;
	T_MOTION_DETECTOR_RATIO ratio;
	T_U8 *pData1;
	T_U8 *pData2;
	T_U32	tickcount;
	T_BOOL	bMotionIsDetected = 0;
	T_U32 	timeDiv = 0;
	init();	// initial file system, memory, camera, lcd and etc.

	memset(&detector_open_para, sizeof(T_MOTION_DETECTOR_OPEN_PARA));
	memset(&ratio, sizeof(T_MOTION_DETECTOR_RATIO));
	
	detector_open_para.m_uWidth = 352;
	detector_open_para.m_uHeight = 288;
	detector_open_para.m_uThreshold = 35;

	ratio.m_uFullRatio = 125;
	ratio.m_uAreaRatio1 = 250;
	ratio.m_uAreaRatio2 = 250;
	ratio.m_uAreaRatio3 = 250;
	ratio.m_uAreaRatio4 = 250;
	ratio.m_uAreaRatio5 = 250;
	ratio.m_uAreaRatio6 = 250;
	ratio.m_uAreaRatio7 = 250;
	ratio.m_uAreaRatio8 = 250;
	ratio.m_uAreaRatio9 = 250;

	hMdetector = Motion_Detector_Open(&detector_open_para);
	if (AK_NULL == hMdetector)
	{
		Motion_Detector_Close(hMdetector);
		return;
	}

	Motion_Detector_SetRatio(hMdetector, &ratio);

	while (bMotionIsDetected != 0)
	{
		pData1 = get_y_data();

		tickcount = get_system_time_ms();

		while(1)
		{
			if (get_system_time_ms() > (tickcount + 500))
			{
				break;
			}
		}

		pData2 = get_y_data();
		
		bMotionIsDetected = Motion_Detector_Handle(hMdetector, pData1, pData2);
		if (bMotionIsDetected)
		{
			start_record();//motion is detected, do something
			break;
		}
	
	}

	Motion_Detector_Close(hMdetector);

	return;
}

	@endcode

 ***************************************************/


#ifndef	_MOTION_DETECTOR_LIB_H_
#define _MOTION_DETECTOR_LIB_H_

#include "Lib_media_global.h"

#define MOTION_DETECTOR_LIB_VERSION		"Media Lib V0.1.0--svn3297"

typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF				m_FunPrintf;

	MEDIALIB_CALLBACK_FUN_MALLOC				m_FunMalloc;
	MEDIALIB_CALLBACK_FUN_FREE					m_FunFree;
}T_MOTION_DETECTOR_CB;

typedef struct _T_MOTION_DETECTOR_RATIO
{
	T_U16	m_uFullRatio;		//白色像素占整幅图像的比例，该比例，控制判断是否运动，以千分比的形式，范围[1, 300]
	T_U16	m_uAreaRatio1;		//白色像素占区域图像的比例
	T_U16	m_uAreaRatio2;
	T_U16	m_uAreaRatio3;
	T_U16	m_uAreaRatio4;
	T_U16	m_uAreaRatio5;
	T_U16	m_uAreaRatio6;
	T_U16	m_uAreaRatio7;
	T_U16	m_uAreaRatio8;
	T_U16	m_uAreaRatio9;
}T_MOTION_DETECTOR_RATIO;


typedef struct _T_MOTION_DETECTOR_OPEN_PARA
{
	T_U16	m_uWidth;			//图像的宽度
	T_U16	m_uHeight;			//图像的高度
	T_U16	m_uThreshold;		//判断该像素为运动前景的阈值，取值范围[10, 40]

	T_MOTION_DETECTOR_CB	m_CBFunc;	//callback functions
}T_MOTION_DETECTOR_OPEN_PARA;


/**
 * @brief Get Motion detector library version
 *
 * @author Su_Dan
 * @return const T_CHR *
 * @retval	version string
 */
const T_CHR *Motion_Detector_GetVersion(T_VOID);

/**
 * @brief Open a resource
 *
 * @author Hu_Jing
 * @param	motion_detector_open_para		[in]	pointer of T_MOTION_DETECTOR_OPEN_PARA struct
 * @return T_pVOID
 * @retval	AK_NULL		open failed
 * @retval	other		open ok
  */
T_pVOID Motion_Detector_Open(T_MOTION_DETECTOR_OPEN_PARA *pDetector_open_para);

/**
 * @brief Close recorder
 *
 * @author Hu_Jing
 * @param	pDetector		[in]	pointer which is returned by Motion_Detector_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FLASE	Close fail
 */
T_BOOL	Motion_Detector_Close(T_pVOID pDetector);

/**
 * @brief process motion detection
 *
 * @author Hu_Jing
 * @param	pDetector		[in]	pointer which is returned by Motion_Detector_Open function
 * @param	pData1			[in]	Y data of first picture
 * @param	pData2			[in]	Y data of second picture
 * @return T_BOOL
 * @retval	AK_TRUE		motion is detected
 * @retval	AK_FLASE	motion is not detected
 */
T_BOOL	Motion_Detector_Handle(T_pVOID pDetector, T_U8 *pData1, T_U8 *pData2);

/**
 * @brief set motion detection ratio para
 *
 * @author Hu_Jing
 * @param	pDetector	[in]	pointer which is returned by Motion_Detector_Open function
 * @param	pRatio		[in]	pointer of the struct T_MOTION_DETECTOR_RATIO_PARA
 * @return T_BOOL
 * @retval	AK_TRUE		set ok
 * @retval	AK_FLASE	set fail
 */
T_BOOL	Motion_Detector_SetRatio(T_pVOID pDetector, T_MOTION_DETECTOR_RATIO *pRatio);

/**
 * @brief set motion detection dimension para
 *
 * @author Hu_Jing
 * @param	pDetector	[in]	pointer which is returned by Motion_Detector_Open function
 * @param	uWidth		[in]	new picture width
 * @param	uHeight		[in]	new picture height
 * @return T_BOOL
 * @retval	AK_TRUE		set ok
 * @retval	AK_FLASE	set fail
 */
T_BOOL Motion_Detector_SetDimension(T_pVOID pDetector, T_U16 uWidth, T_U16 uHeight);
#endif//_MOTION_DETECTOR_LIB_H_
