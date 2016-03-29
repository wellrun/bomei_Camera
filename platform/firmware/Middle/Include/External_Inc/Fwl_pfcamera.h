/**
 * @file Eng_pfCamera.h
 * @brief This header file is for camera function prototype
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __FWL_PF_CAMERA_H__
#define __FWL_PF_CAMERA_H__

#include "akdefine.h"


/** camera run time out */
#define CAMERA_TIMEOUT                      4000    


/**
 * clip win mode
 */
typedef enum
{
    eCAMCLIP_AUTO = 0,// auto clip win
    eCAMCLIP_FULL,    // not clip
} T_CAM_CLIP_MODE;



T_BOOL Fwl_CamerIsNightMode(T_U16 FlashMode, T_U16 ColorEffect);
T_VOID Fwl_CameraSetEffect(T_U8 effect);
T_VOID Fwl_CameraSetNightMode(T_U8 mode);
T_VOID Fwl_CamerChangeNightMode(T_U16 *pVal);

/** @defgroup FWL_CAMR Camera interface
    @ingroup FWL
 */
/*@{*/

/**
 * @brief Initialize camera related parameters and open the camera
 * @param[in] T_BOOL : AK_TRUE to init rec audio
 * @return success or not
 */
T_BOOL Fwl_CameraInit(T_VOID);

/**
 * @brief Initialize camera in record mode
 * @param[in] srcwidth   window width
 * @param[in] srcheight  window height
 * @param[in] deswidth   width of capture size
 * @param[in] desheight  height of capture size
 * @param[in] pYUV1     camera buffer 1
 * @param[in] pYUV2     camera buffer 2
 * @param[in] pYUV3     camera buffer 3
 * @return success or not
 */
T_BOOL Fwl_CameraRecInit(T_U32 deswidth, T_U32 desheight,
                    T_U8 *pYUV1, T_U8 *pYUV2, T_U8 *pYUV3);

/**
 * @brief Free the camera
 * @return void
 */
T_VOID Fwl_CameraFree(T_VOID);

/**
 * @brief Free the camera,enable camera to standby mode
 * @return void
 */

T_VOID Fwl_CameraPowerDown(T_VOID);


/**
 * @brief stop camera stream(stop camera task)
 * @return void
 */
T_VOID Fwl_CamStreamStop(T_VOID);

/**
 * @brief Set camera saturation level 
 * @author 
 * @date 2004-09-22
 * @param[in] saturation   saturation value
 * @return success or not
 */
T_BOOL Fwl_CameraChangeSaturation(T_U8 saturation);
T_BOOL Fwl_CameraSaturationCanDec(T_U8 saturation);
T_BOOL Fwl_CameraSaturationCanInc(T_U8 saturation);

/**
 * @brief Set camera contrast level 
 * @author 
 * @date 2004-09-22
 * @param[in] contrast   contrast value
 * @return success or not
 */
T_BOOL Fwl_CameraChangeContrast(T_U8 contrast);
T_BOOL Fwl_CameraContrastCanDec(T_U8 contrast);
T_BOOL Fwl_CameraContrastCanInc(T_U8 contrast);

/**
 * @brief Set camera brightness level 
 * @author
 * @date 2004-09-22
 * @param[in] brightness   brightness value
 * @return success or not
 */
T_BOOL Fwl_CameraChangeBrightness(T_U8 brightness);
T_BOOL Fwl_CameraBrightnessCanDec(T_U8 brightness);
T_BOOL Fwl_CameraBrightnessCanInc(T_U8 brightness);

/**
 * @brief Set the  capture window
 * @param[in] width   capture width
 * @param[in] height   capture height
 * @return int
 * @retval 0    error mode
 * @retval 1    success
 * @retval -1    fail
 */
int Fwl_CameraSetWindows(int width, int height);

/**
 * @brief Set camera mirror mode 
 * @author Anyka
 * @date 2004-09-22
 * @param[in] mirror   mirror mode
 * @return void
 */
T_VOID Fwl_CameraSetMirror(T_U8 mirror);

/**
 * @brief capture an image in RGB format
 * @author Anyka
 * @date 2004-09-22
 * @param[out] RGB 	buffer to save the image data 
 * @param[in] srcW  source width 
 * @param[in] srcH  source height 
 * @param[in] dstW   desination width 
 * @param[in] dstH   desination height 
 * @param[in] timeout   time out value for capture 
 * @return success or not
 * @retval -1   time out
 * @retval 0   success
 */
T_BOOL FWl_CameraCaptureRGB(T_U8 *RGB, T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH, T_U32 timeout);

/**
 * @brief capture an image in YUV format
 * @author Anyka 
 * @date 2004-09-22
 * @param[out] iY  Y buffer to save the image data 
 * @param[out] iU 	U buffer to save the image data
 * @param[out] iV 	V buffer to save the image data 
 * @param[in] dstWidth   destination width 
 * @param[in] dstHeight   destination height 
 * @param[in] timeout   time out value for capture 
 * @return T_BOOL
 * @retval FAIL   time out
 * @retval TRUE   success
 */
T_BOOL Fwl_CameraCaptureYUV(T_U8 *iY, T_U8 *iU, T_U8 *iV, int dstWidth, int dstHeight, T_U32 timeout);

T_BOOL Fwl_CameraCaptureYUVNoWaitDMA(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, int dstWidth, int dstHeight, T_U32 timeout);
	
/**
 * @brief Change YUV to RGB
 * @author Anyka 
 * @date 2004-09-22
 * @param[in] srcY  	Y buffer to save the original image data 
 * @param[in] srcU 		U buffer to save the original image data
 * @param[in] srcV 		V buffer to save the original image data 
 * @param[out] dstRGB   buffer to save the destination RGB data
 * @param[in] srcW   source weight
 * @param[in] srcH   source height 
 * @param[in] dstW   destination width 
 * @param[in] dstH   destination height 
 * @param[in] timeout   time out value for capture 
 * @return int
 * @retval -1   time out
 * @retval 0   success
 */
T_BOOL Fwl_CameraYUV2RGB(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstRGB, \
						 T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH, T_U32 timeout);

/**
 * @brief Get the preview buffer of camera
 * @param[in] yPos   position to preview
 * @return buffer pointer to preview
 */
T_U8*  Fwl_GetCamPreviewBuf(T_S32 yPos);

T_U8*  Fwl_CameraRecordGetData(T_VOID);

T_BOOL Fwl_CameraCapComplete(T_VOID);

T_BOOL Fwl_CameraFlashInit(T_VOID);

T_VOID Fwl_CameraFlashOn(T_VOID);

T_VOID Fwl_CameraFlashOff(T_VOID);

T_VOID Fwl_CameraFlashClose(T_VOID);

T_BOOL	Fwl_CameraSetToCap(T_U32 width, T_U32 height);

T_BOOL	Fwl_CameraSetToPrev(T_U32 width, T_U32 height); 

T_BOOL  Fwl_CameraSetToRec(T_U32 width, T_U32 height);

T_BOOL	Fwl_CameraGetMaxSize(T_U32 *pCamMaxW, T_U32 *pCamMaxH);
T_BOOL  Fwl_GetRecFrameSize(T_CAMERA_MODE mode,T_U32 *wi,T_U32 *hi);
T_BOOL  Fwl_CameraGetScale(T_U32 *pScaleW, T_U32 *pScaleH, T_U32 destW, T_U32 destH);
T_BOOL  Fwl_CameraGetMinSize(T_U32 *pCamWinW, T_U32 *pCamWinH, T_U32 destW, T_U32 destH);
T_BOOL  Fwl_CameraGetWinSize(T_U32 *pCamWinW, T_U32 *pCamWinH, T_U32 destW, T_U32 destH);
T_BOOL  Fwl_CameraGetFocusWin(T_RECT *pFocusWin, T_U32 focusLvl, T_U32 maxLvl,T_U32 orignW, T_U32 orignH);
T_BOOL  Fwl_CameraGetClipWin(T_RECT *pClipWin, T_U32 srcW, T_U32 srcH,
                            T_U32 destW, T_U32 destH, T_CAM_CLIP_MODE clipMode);
T_VOID  Fwl_CameraGetFocusTips(T_U32 focusLvl, T_U8 *tips);
T_BOOL  Fwl_CameraCheckSize(T_U32 *pWidth, T_U32 *pHeight);
T_BOOL  Fwl_CameraGetRecSize(T_U32 *pWidth, T_U32 *pHeight);

T_U8 Fwl_CameraGetType(T_VOID);
T_BOOL Fwl_CameraIsSupportHd(T_VOID);

/*@}*/

#endif


