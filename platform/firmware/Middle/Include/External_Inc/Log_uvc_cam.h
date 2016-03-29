/**
 * @author: Bennis Zhang
 * @date: 2009-06-08
 */

#ifndef _LOG_UVC_CAM_H_
#define _LOG_UVC_CAM_H_

typedef enum
{
	E_UVCCAM_YUVTYPE_422 = 0,
	E_UVCCAM_YUVTYPE_420
	
} T_E_UVCCAM_YUVTYPE;

typedef struct
{
	T_U32				videoWidth, videoHeight;
	T_U32				camWinWidth, camWinHeight;
	T_U32				zoom_level;
	T_E_UVCCAM_YUVTYPE	YUVType;
	T_U32				USBType;
	
} T_UVCCAM_INI;

T_S32 UVCCam_Init(T_VOID);
T_BOOL  UVCCam_InitStruct(T_UVCCAM_INI * args);
T_HANDLE UVCCam_Open(T_UVCCAM_INI * args);
T_S32 UVCCam_Close(T_HANDLE hdl);
T_VOID UVCCam_UserExit(T_BOOL bFlag);

#endif



