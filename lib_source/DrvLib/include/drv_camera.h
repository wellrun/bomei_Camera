/**
 * @file drv_camera.h
 * @brief provide interfaces for low layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2010-12-07
 * @version 1.0
 */

#ifndef __DRV_CAMERA_H__
#define __DRV_CAMERA_H__

#include "hal_camera.h"

/** @defgroup Drv_camera Hardware Abstract Layer of camera
 *  @ingroup Camera
 */
/*@{*/


typedef struct
{
    T_U32            cam_mclk;
    T_VOID           (*cam_open_func)(T_VOID);
    T_BOOL           (*cam_close_func)(T_VOID);
    T_U32            (*cam_read_id_func)(T_VOID);
    T_BOOL           (*cam_init_func)(T_VOID);
    T_VOID           (*cam_set_mode_func)(T_U8 mode);
    T_VOID           (*cam_set_exposure_func)(T_CAMERA_EXPOSURE exposure);
    T_VOID           (*cam_set_brightness_func)(T_CAMERA_BRIGHTNESS brightness);
    T_VOID           (*cam_set_contrast_func)(T_CAMERA_CONTRAST contrast);
    T_VOID           (*cam_set_saturation_func)(T_CAMERA_SATURATION saturation);
    T_VOID           (*cam_set_sharpness_func)(T_CAMERA_SHARPNESS sharpness);
    T_VOID           (*cam_set_AWB_func)(T_CAMERA_AWB awb);
    T_VOID           (*cam_set_mirror_func)(T_CAMERA_MIRROR mirror);
    T_VOID           (*cam_set_effect_func)(T_CAMERA_EFFECT effect);
    T_S32            (*cam_set_window_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_VOID           (*cam_set_night_mode_func)(T_NIGHT_MODE mode);
    T_U32            (*cam_set_framerate_func)(float framerate);
    T_BOOL           (*cam_set_to_cap_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_BOOL           (*cam_set_to_prev_func)(T_U32 srcWidth, T_U32 srcHeight);    
    T_BOOL           (*cam_set_to_record_func)(T_U32 srcWidth, T_U32 srcHeight);
    T_CAMERA_TYPE    (*cam_get_type)(T_VOID);
}T_CAMERA_FUNCTION_HANDLER;

typedef struct
{
    T_U32 DeviceID;
    T_CAMERA_FUNCTION_HANDLER *handler;
}T_CAMERA_INFO;

#define CAMERA_MAX_SUPPORT        20


/** 
 * @brief register a camera device, which will be probed at camera open
 * @author xia_wenting
 * @date 2010-12-07
 * @param[in] id  the camera chip id
 * @param[in] handler camera interface
 * @return T_BOOL
 * @retval AK_TRUE register successfully
 * @retval AK_FALSE register unsuccessfully
 */
T_BOOL camera_reg_dev(T_U32 id, T_CAMERA_FUNCTION_HANDLER *handler);


/*@}*/
#endif //__DRV_CAMERA_H__
