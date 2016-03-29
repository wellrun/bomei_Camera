/**
 * @file hal_camera.h
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2010-12-07
 * @version 1.0
 * @note this an example to use the API of camera driver
 *
 *            cam_open();        //open it for init
 *
 *            
 *            //set camera mode parameter if needed
 *            cam_set_to_prev(srcWidth, srcHeight);
 *
 *            //set camera feature, such as AWB, effect, brightness.etc.
 *            cam_set_feature(CAM_FEATURE_EFFECT, CAMERA_EFFECT_NEGATIVE); 
 *            ......
 *
 *            //capture a photo in YUV mode
 *            cam_capture_YUV(dy, du, dv, dstw, dsth, timeout);
 *        
 *            //then display on LCD or save it
 *            ......
 */


#ifndef __HAL_CAMERA_H__
#define __HAL_CAMERA_H__

/** @defgroup Hal_camera Hardware Abstract Layer of camera
 *    @ingroup Camera
 */
/*@{*/

#include "anyka_types.h"

/******************************************************************************************
 *    the following define the camera device register interface *      
******************************************************************************************/

/** @brief Camera Parameter Night Mode definition
 *
 *  This structure define the value of parameter Night Mode
 */
typedef enum
{
    CAMERA_DAY_MODE,
    CAMERA_NIGHT_MODE,
    CAMERA_NIGHT_NUM
}T_NIGHT_MODE;


/** @brief Camera Parameter CCIR601/656 protocol
 *
 *    This structure define the CMOS sensor compatible with CCIR601 or CCIR656 protocol
 */
typedef enum
{
    CAMERA_CCIR_601,
    CAMERA_CCIR_656,
    CAMERA_CCIR_NUM
}T_CAMERA_INTERFACE;

/** @brief Camera Parameter Exposure definition
 *
 *  This structure define the value of parameter Exposure
 */
typedef enum
{
    EXPOSURE_WHOLE = 0,
    EXPOSURE_CENTER,
    EXPOSURE_MIDDLE,
    CAMERA_EXPOSURE_NUM
}T_CAMERA_EXPOSURE;

/** @brief Camera Parameter AWB definition
 *
 *  This structure define the value of parameter AWB
 */
typedef enum
{
    AWB_AUTO = 0,
    AWB_SUNNY,
    AWB_CLOUDY,
    AWB_OFFICE,
    AWB_HOME,
    AWB_NIGHT,
    AWB_NUM
}T_CAMERA_AWB;


/** @brief Camera Parameter Brightness definition
 *
 *  This structure define the value of parameter Brightness
 */
typedef enum
{
    CAMERA_BRIGHTNESS_0 = 0,
    CAMERA_BRIGHTNESS_1,
    CAMERA_BRIGHTNESS_2,
    CAMERA_BRIGHTNESS_3,
    CAMERA_BRIGHTNESS_4,
    CAMERA_BRIGHTNESS_5,
    CAMERA_BRIGHTNESS_6,
    CAMERA_BRIGHTNESS_NUM
}T_CAMERA_BRIGHTNESS;


/** @brief Camera Parameter Contrast definition
 *
 *  This structure define the value of parameter Contrast
 */
typedef enum 
{
    CAMERA_CONTRAST_1 = 0,
    CAMERA_CONTRAST_2,
    CAMERA_CONTRAST_3,
    CAMERA_CONTRAST_4,
    CAMERA_CONTRAST_5,
    CAMERA_CONTRAST_6,
    CAMERA_CONTRAST_7,
    CAMERA_CONTRAST_NUM
}T_CAMERA_CONTRAST;

/** @brief Camera Parameter Saturation definition
 *
 *  This structure define the value of parameter Saturation
 */
typedef enum
{
    CAMERA_SATURATION_1 = 0,
    CAMERA_SATURATION_2,
    CAMERA_SATURATION_3,
    CAMERA_SATURATION_4,
    CAMERA_SATURATION_5,
    CAMERA_SATURATION_NUM
}T_CAMERA_SATURATION;

/** @brief Camera Parameter Sharpness definition
 *
 *  This structure define the value of parameter Sharpness
 */
typedef enum
{
    CAMERA_SHARPNESS_0 = 0,
    CAMERA_SHARPNESS_1,
    CAMERA_SHARPNESS_2,
    CAMERA_SHARPNESS_3,
    CAMERA_SHARPNESS_4,
    CAMERA_SHARPNESS_5,
    CAMERA_SHARPNESS_6,
    CAMERA_SHARPNESS_NUM
}T_CAMERA_SHARPNESS;

/** @brief Camera Parameter Mirror definition
 *
 *  This structure define the value of parameter Mirror
 */
typedef enum
{
    CAMERA_MIRROR_V = 0,
    CAMERA_MIRROR_H,
    CAMERA_MIRROR_NORMAL,
    CAMERA_MIRROR_FLIP,
    CAMERA_MIRROR_NUM
}T_CAMERA_MIRROR;

/** @brief Camera Parameter Effect definition
 *
 *  This structure define the value of parameter Effect
 */
typedef enum
{
    CAMERA_EFFECT_NORMAL = 0,
    CAMERA_EFFECT_SEPIA,
    CAMERA_EFFECT_ANTIQUE,
    CAMERA_EFFECT_BLUE,
    CAMERA_EFFECT_GREEN,
    CAMERA_EFFECT_RED,
    CAMERA_EFFECT_NEGATIVE,
    CAMERA_EFFECT_BW,
    CAMERA_EFFECT_BWN,    
    CAMERA_EFFECT_AQUA,    // PO1200 additional mode add by Liub 20060918
    CAMERA_EFFECT_COOL,
    CAMERA_EFFECT_WARM,
    CAMERA_EFFECT_NUM
}T_CAMERA_EFFECT;

/** @brief Camera type definition
 *
 *  This structure define the type of camera
 */
typedef enum
{
    CAMERA_P3M  = 0x00000001,
    CAMERA_1P3M = 0x00000002,
    CAMERA_2M   = 0x00000004,
    CAMERA_3M   = 0x00000008,
    CAMERA_4M   = 0x00000010,
    CAMERA_5M   = 0x00000020,
    CAMERA_ZOOM = 0x00000040
}T_CAMERA_TYPE;

/** @brief Camera feature definition
 *
 *  This structure define the feature list of camera
 */
typedef enum {
    CAM_FEATURE_NIGHT_MODE = 0,
    CAM_FEATURE_EXPOSURE,
    CAM_FEATURE_AWB,
    CAM_FEATURE_BRIGHTNESS,
    CAM_FEATURE_CONTRAST,
    CAM_FEATURE_SATURATION,
    CAM_FEATURE_SHARPNESS,
    CAM_FEATURE_MIRROR,
    CAM_FEATURE_EFFECT,
    CAM_FEATURE_NUM
}T_CAMERA_FEATURE;

typedef struct
{
    T_U32   width;
    T_U32   height;
    T_U8    *dY;
    T_U8    *dU;
    T_U8    *dV;
    volatile T_U8    status;
}T_CAMERA_BUFFER;

/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_open(T_VOID);

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID cam_close(T_VOID);

/**
 * @brief capture an image in YUV420 format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstY     Y buffer to save the image data 
 * @param[out] dstU     U buffer to save the image data
 * @param[out] dstV     V buffer to save the image data 
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture 
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL cam_capture_YUV(T_U8 *dstY, T_U8 *dstU, T_U8 *dstV,
                    T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout);

/**
 * @brief capture an image in RGB format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dst      buffer to save the image data 
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture 
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL cam_capture_RGB(T_U8 *dst, T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout);

/**
 * @brief capture an image in JPEG format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstJPEG     buffer to save the image data 
 * @param[out] JPEGlength  jpeg line length
 * @param[in] timeout time out value for capture 
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL cam_capture_JPEG(T_U8 *dstJPEG, T_U32 *JPEGlength, T_U32 timeout);

/**
 * @brief Set camera controller CCIR601/656 protocol 
 * @author xia_wenting  
 * @date 2010-12-07
 * @param[in] interface camera controller ccir601 or ccir656
 * @return T_VOID
 */
T_VOID cam_set_interface(T_CAMERA_INTERFACE camera_interface);

/**
 * @brief set camera feature 
 * @author xia_wenting 
 * @date 2010-12-01
 * @param[in] feature_type camera feature type,such as night mode,AWB,contrast.etc.
 * @param[in] feature_setting feature setting
 * @return T_VOID
 */
T_VOID cam_set_feature(T_CAMERA_FEATURE feature_type, T_U8 feature_setting);

/**
 * @brief set camera window
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
T_S32 cam_set_window(T_U32 srcWidth, T_U32 srcHeight);

/**
 * @brief Set camera frame rate manually
 * @author xia_wenting  
 * @date 2011-01-05
 * @param[in] framerate camera output framerate
 * @return T_U32
 * @retval 0 if error mode 
 * @retval 1 if success
 */
T_U32 cam_set_framerate(float framerate);

/**
 * @brief switch camera reg to preview mode
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_prev(T_U32 srcWidth, T_U32 srcHeight);

/**
 * @brief switch from preview mode to capture mode
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_cap(T_U32 srcWidth, T_U32 srcHeight);

/**
 * @brief switch camera to record mode
 * @author xia_wenting  
 * @date 2010-12-01
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_record(T_U32 srcWidth, T_U32 srcHeight);

/**
 * @brief get camera type
 * @author xia_wenting 
 * @date 2010-12-07
 * @return T_CAMERA_TYPE
 * @retval camera type defined by mega pixels
 */
T_CAMERA_TYPE cam_get_type(T_VOID);

/******************************************************************************************
 *    the following define the camera stream recording interface *      
******************************************************************************************/

/** 
 * @brief init camera interrupt mode
 * @author xia_wenting
 * @date 2010-12-07
 * @param[in] dstWidth camera dest width
 * @param[in] dstHeight camera dest height
 * @param[in] YUV1 store a frame YUV buffer, can be NULL
 * @param[in] YUV2 store a frame YUV buffer, can be NULL
 * @param[in] YUV3 store a frame YUV buffer, can be NULL
 * @return T_BOOL
 * @retval AK_TRUE init successfully
 * @retval AK_FALSE init unsuccessfully
 */
T_BOOL camstream_init(T_U32 dstWidth, T_U32 dstHeight,
                    T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3);

/** 
 * @brief notify app when data ready
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @param[in] T_VOID 
 * @return T_VOID
 */
typedef T_VOID (*T_fCAMSTREAMCALLBACK)(T_VOID);

/** 
 * @brief set notify callback function
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @param[in] callback_func callback function
 * @return T_VOID
*/
T_VOID camstream_set_callback(T_fCAMSTREAMCALLBACK callback_func);

/** 
 * @brief change camera configure
 * @author xia_wenting
 * @date 2010-12-07
 * @param[in] dstWidth  camera dest width
 * @param[in] dstHeight camera dest height
 * @param[in] YUV1     store a frame YUV buffer, can be NULL
 * @param[in] YUV2     store a frame YUV buffer, can be NULL
 * @param[in] YUV3     store a frame YUV buffer, can be NULL
 * @return T_BOOL
 * @retval AK_TRUE change successfully
 * @retval AK_FALSE change unsuccessfully
 */
T_BOOL camstream_change(T_U32 dstWidth, T_U32 dstHeight,
                       T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3);

/** 
 * @brief stop camera(interrupt mode)
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_VOID
 */
T_VOID camstream_stop(T_VOID);

/** 
 * @brief a frame is ready
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_BOOL
 * @retval AK_FALSE: no data
 * @retval AK_TRUE: a frame is ready
 */
T_BOOL camstream_ready(T_VOID);

/** 
 * @brief get a frame data
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_CAMERA_BUFFER*
 * @retval pointer of current frame data
 */
T_CAMERA_BUFFER *camstream_get(T_VOID);

/** 
 * @brief suspend camera interface, only accept current frame, other not accept
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_BOOL
 * @retval AK_FALSE: suspend failed
 * @retval AK_TRUE: suspend successful
 */
T_BOOL camstream_suspend(T_VOID);

/** 
 * @brief resume camera interface, start accept frame
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_VOID
 */
T_VOID camstream_resume(T_VOID);

/**
 * @brief get camera Id, Must be keep camera is open can be read
 * @author LHS
 * @date 2012-08-07
 * @return T_U32 camera id
 * @retval if successed,return driver id
 * @retval if failed,return 0
 */
T_U32 cam_get_id(T_VOID);
/*@}*/
#endif
