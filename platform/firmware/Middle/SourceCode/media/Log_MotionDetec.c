/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name:Log_MotionDetec.c
 * Function: video MotionDetec logic
 * Author:  wangxi
 * Date:  2010-12-11
 * Version: 1.0
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include "Fwl_public.h"
#include "fwl_osmalloc.h"
#include "Log_MotionDetec.h"



#define MDETECT_PRINT      MDetect_Print
#define MDETECT_INFO(tips) MDETECT_PRINT("%s\n",tips)
#define MDETECT_ERROR(tips) MDETECT_PRINT("Er:%s @%d\n",tips,__LINE__)


#define THRESHOLD_MIN   10
#define THRESHOLD_MAX   40

typedef struct tag_MOTION_DETECTOR 
{
	T_pVOID        hDetctor;//motion detect handle of lib
	T_U16          m_uThreshold;// motion detect threshold val
	T_U16          m_uWidth;//motion detect width
	T_U16          m_uHeight;// motion detect height
	T_U8          *m_pData;// motion detect current data
	T_U8          *m_pOldData;// motiondetect predata
	T_BOOL         m_isDetectRun;// is motion detect enable
} T_MOTION_DETECTOR;


static T_BOOL MDetect_SetWin(T_HANDLE hdl,T_U16 width,T_U16 height);


static T_S32 MDetect_Print(T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    va_start(args, s);
    len = Fwl_VPrint(C3, M_MDETECT, s, args);
    va_end(args); 
    
    return len;
}

static T_pVOID MDetect_Malloc(T_U32 size)
{
	return (T_pVOID)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID MDetect_Free(T_pVOID var)
{
   return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}


T_HANDLE MDetect_Open(T_U32 width,T_U32 height,T_U32 Threshold)
{
	T_MOTION_DETECTOR *pMotionDetector;
	T_MOTION_DETECTOR_OPEN_PARA openParam;
	T_MOTION_DETECTOR_RATIO ratioParam; 
	T_pVOID hDetector = AK_NULL;

	pMotionDetector = (T_MOTION_DETECTOR*)Fwl_Malloc(sizeof(T_MOTION_DETECTOR));
	
	if (AK_NULL == pMotionDetector)
	{
        MDETECT_ERROR("Param Er");
		return 0;
	}

	memset(pMotionDetector, 0, sizeof(T_MOTION_DETECTOR));
	
	openParam.m_uWidth     = (T_U16)width;
	openParam.m_uHeight    = (T_U16)height;
	openParam.m_uThreshold = (T_U16)Threshold;
	if (openParam.m_uThreshold < THRESHOLD_MIN)
	{
	    openParam.m_uThreshold = THRESHOLD_MIN;
	}
	
	if (openParam.m_uThreshold > THRESHOLD_MAX)
	{
	    openParam.m_uThreshold = THRESHOLD_MAX;
	}

    openParam.m_uThreshold = 30;

	openParam.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)MDetect_Print;
	openParam.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)MDetect_Malloc;
	openParam.m_CBFunc.m_FunFree   = (MEDIALIB_CALLBACK_FUN_FREE)MDetect_Free;

	//value smaller, precision bigger
	ratioParam.m_uFullRatio = 125;
	ratioParam.m_uAreaRatio1 = 160;
	ratioParam.m_uAreaRatio2 = 160;
	ratioParam.m_uAreaRatio3 = 160;
	ratioParam.m_uAreaRatio4 = 160;
	ratioParam.m_uAreaRatio5 = 125;
	ratioParam.m_uAreaRatio6 = 160;
	ratioParam.m_uAreaRatio7 = 160;
	ratioParam.m_uAreaRatio8 = 160;
	ratioParam.m_uAreaRatio9 = 160;
	
#ifdef OS_ANYKA	
	hDetector = Motion_Detector_Open(&openParam);
#endif
	if (AK_NULL == hDetector)
	{
       MDETECT_ERROR("Detector_Open Fail");
	   MDetect_Close((T_HANDLE)pMotionDetector);
	   pMotionDetector = AK_NULL;
	   return 0;
	}
	
#ifdef OS_ANYKA
	Motion_Detector_SetRatio(hDetector, &ratioParam);
#endif

	pMotionDetector->hDetctor      = hDetector;
	pMotionDetector->m_uWidth      = openParam.m_uWidth;
	pMotionDetector->m_uHeight     = openParam.m_uHeight;
	pMotionDetector->m_uThreshold  = openParam.m_uThreshold;
	pMotionDetector->m_pData       = AK_NULL;
	pMotionDetector->m_pOldData    = AK_NULL;
	pMotionDetector->m_isDetectRun = AK_FALSE;

	MDETECT_PRINT("Open Ok [%dX%d,%d].\n",width,height,Threshold);

	return (T_HANDLE)pMotionDetector;
}

static T_BOOL MDetect_SetWin(T_HANDLE hdl,T_U16 width,T_U16 height)
{
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;

	if (AK_NULL == pMotionDetector)
	{
		MDETECT_ERROR("Param Fail");
		return AK_FALSE;
	}
	
	if ((pMotionDetector->m_uWidth == width) && (pMotionDetector->m_uHeight == height))
	{
		MDETECT_PRINT("NeedNot SetDimension %d X %d.\n",width,height);
		return AK_TRUE;
	}
#ifdef OS_ANYKA
	if (Motion_Detector_SetDimension(pMotionDetector->hDetctor,width,height))
	{
        pMotionDetector->m_uWidth  = width;
        pMotionDetector->m_uHeight = height;
		return AK_TRUE;
	}
	else
	{
		MDETECT_PRINT("SetDimension %d X %d Fail\n",width,height);
		return AK_FALSE;
	}
#else
	return AK_FALSE;
#endif
}

T_VOID MDetect_Close(T_HANDLE hdl)
{
   T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;
   	
	if (AK_NULL != pMotionDetector)
	{
		if (AK_NULL != pMotionDetector->hDetctor)
		{
		#ifdef OS_ANYKA
			Motion_Detector_Close(pMotionDetector->hDetctor);
		#endif
		}
		MDetect_Stop((T_HANDLE)pMotionDetector);
		memset(pMotionDetector,0,sizeof(T_MOTION_DETECTOR));
		Fwl_Free(pMotionDetector);
		MDETECT_INFO("Close Ok");
	}
}

T_BOOL MDetect_Start(T_HANDLE hdl, T_U32 width, T_U32 height)
{
	T_BOOL ret = AK_FALSE;
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;
	
	if (AK_NULL != pMotionDetector)
	{
		if (!pMotionDetector->m_isDetectRun)
		{
			MDetect_SetWin(hdl,(T_U16)width,(T_U16)height);
			pMotionDetector->m_pData	= AK_NULL;
			pMotionDetector->m_pOldData    = Fwl_Malloc(width * height);//必须分配空间，用于保存旧数据
			if (AK_NULL == pMotionDetector->m_pOldData )
			{
				MDETECT_ERROR("malloc space for old data failed!");
			}else
			{			
				pMotionDetector->m_isDetectRun = AK_TRUE;
				ret = AK_TRUE;
			}
		}
	}

	if (AK_TRUE ==ret)
	{
		MDETECT_PRINT("MDetect_Start ok, win width=%d,height=%d.\n",width,height);
	}else
	{
		MDETECT_ERROR("MDetect_Start failed!");
	}
	
	return ret;
}


T_VOID MDetect_Stop(T_HANDLE hdl)
{
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;

	if (AK_NULL != pMotionDetector)
	{
	    if (pMotionDetector->m_isDetectRun)
	    {
			pMotionDetector->m_pData	   = AK_NULL;
			if (pMotionDetector->m_pOldData !=AK_NULL)
			{
				Fwl_Free(pMotionDetector->m_pOldData );
				pMotionDetector->m_pOldData  = AK_NULL;
			}

			pMotionDetector->m_isDetectRun = AK_FALSE;
		}
	}
	MDETECT_INFO("MDetect_Stop ok!");
}



T_BOOL MDetect_IsRun(T_HANDLE hdl)
{
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;

	if (AK_NULL != pMotionDetector)
	{
		return pMotionDetector->m_isDetectRun;
	}

	return AK_FALSE;
}


T_VOID MDetect_UpdateData(T_HANDLE hdl, T_U8 *pData)
{
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;

	if (AK_NULL != pMotionDetector)
	{
		
		if (pMotionDetector->m_pOldData !=AK_NULL)
		{
			if (pMotionDetector->m_pData !=AK_NULL)
				memcpy(pMotionDetector->m_pOldData  , pMotionDetector->m_pData, pMotionDetector->m_uWidth * pMotionDetector->m_uHeight); 		
		}
					
		pMotionDetector->m_pData	= pData;
	}
}


T_BOOL MDetect_IsMoving(T_HANDLE hdl)
{
	T_BOOL ret = AK_FALSE;
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;
	
	if ((AK_NULL != pMotionDetector) \
		&& (AK_NULL != pMotionDetector->hDetctor) \
		&& (AK_NULL != pMotionDetector->m_pData) \
		&& (AK_NULL != pMotionDetector->m_pOldData))
	{
	#ifdef OS_ANYKA
		ret = Motion_Detector_Handle(pMotionDetector->hDetctor,
			pMotionDetector->m_pData, pMotionDetector->m_pOldData);
	#endif	
		MDETECT_PRINT("\n\n\t%s Moving\n",ret?"Yes":"NO");
	}
	else
	{
		MDETECT_ERROR("Detect_IsMoving Param Er");
	}
	
	return ret;
}

/**
 * @brief get detect windows size 
 * @param[in]     hdl   :handle for detect
 * @param[out] pWidth:width of  detect windows
 * @param[out] pHeight:width of  detect windows
 * @return AK_TRUE for success, AK_FALSE  for failure
 */
T_BOOL MDetect_GetWin(T_HANDLE hdl,T_U16 * pWidth,T_U16 * pHeight)
{
	T_MOTION_DETECTOR *pMotionDetector = (T_MOTION_DETECTOR *)hdl;
	
	if (AK_NULL == pMotionDetector)
	{
		return AK_FALSE;
	}
	
	*pWidth  = 	pMotionDetector->m_uWidth;
	*pHeight = 	pMotionDetector->m_uHeight;

	return AK_TRUE;
}



