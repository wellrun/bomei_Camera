/** 
 * @file hal_camera.c
 * @brief provide interfaces for high layer operation of Camera
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting
 * @date 2011-03-30
 * @version 1.0
 */
#include <string.h>
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "camera.h"
#include "interrupt.h"
#include "sysctl.h"
#include "hal_probe.h"
#include "drv_module.h"

//define camera buffer count
#define CAMERABUFFERCOUNT           3

//define camera message
#define CAMERA_MESSAGE              3

//suspend flag
#define CAMERA_SUSPEND              1
#define CAMERA_NO_SUSPEND           0

typedef enum
{
    BUFFER_EMPTY = 0,
    BUFFER_READY,
    BUFFER_CAPTURE,
    BUFFER_USE
}T_BUFFER_STATUS;

typedef struct 
{
    T_CAMERA_BUFFER camera_buffer[CAMERABUFFERCOUNT]; 
    T_fCAMSTREAMCALLBACK DataNotifyCallbackFunc;    
    T_U32 CameraBufferCount; 
    T_U8 CaptureBufferIndex; 
    T_U8 ReadyBufferIndex;
    T_U8 UseBufferIndex;
    T_U8 suspend_flag;
    T_U8 interrupt_flag;
    T_CAMERA_FUNCTION_HANDLER *pCameraHandler; 
    T_U32 drv_output_width;    //output width of camera driver
    T_U32 drv_output_height;   //output height of camera driver
    T_U32 sensor_output_width; //output width of camera sensor, maybe include window function(done by sensor itself or AK chip)
                               //after camera mode setting. if (sensor_output_width < drv_output_width), window function is called
    T_U32 sensor_output_height;//output height of camera sensor, maybe include window function(done by sensor itself or AK chip)
                               //after camera mode setting. if (sensor_output_height < drv_output_height), window function is called
    T_BOOL bIsCCIR656;
    T_BOOL bIsStream_change;   //indicate stream has been changed, used for abandoning the first frame
}T_HAL_PARAM;

static volatile T_HAL_PARAM m_hal_param = {0};

static T_VOID cam_start(T_VOID);
static T_VOID cam_interrupt_callback(T_VOID);
static T_VOID cam_message_callback(T_U32 *param, T_U32 len);
static T_VOID cam_reset_ctl(T_VOID);


/**
 * @brief capture an image in YUV420 format
 * @author xia_wenting 
 * @date 2010-12-16
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
                    T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout)
{
    if ((m_hal_param.sensor_output_width > 4096) || (m_hal_param.sensor_output_height > 4096) 
        || (m_hal_param.sensor_output_width < 1) || (m_hal_param.sensor_output_height < 1)
        || (dstWidth > 4096) || (dstHeight > 4096) || (dstWidth < 1) || (dstHeight < 1)
        || ((dstWidth % 4) != 0) || ((dstHeight % 2) != 0)
        || ((dstWidth * dstHeight % 128) != 0) //chip constrain
        || (m_hal_param.sensor_output_width < dstWidth) || (m_hal_param.sensor_output_height < dstHeight))
    {
        akprintf(C1, M_DRVSYS, "cam_capture_YUV set param error\n");
        return AK_FALSE;
    }

    if ((dstY == AK_NULL) || (dstU == AK_NULL) || (dstV == AK_NULL))
    {
        return AK_FALSE;
    }   
    
    return camctrl_capture_YUV(dstY, dstU, dstV, m_hal_param.sensor_output_width, m_hal_param.sensor_output_height, 
        dstWidth, dstHeight, timeout, m_hal_param.bIsCCIR656);
}

/**
 * @brief capture an image in RGB format
 * @author xia_wenting 
 * @date 2010-12-16
 * @param[out] dst      buffer to save the image data 
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture 
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL cam_capture_RGB(T_U8 *dst, T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout)
{
    if ((m_hal_param.sensor_output_width > 4096) || (m_hal_param.sensor_output_height > 4096) 
        || (m_hal_param.sensor_output_width < 1) || (m_hal_param.sensor_output_height < 1)
        || (dstWidth > 4096) || (dstHeight > 4096) || (dstWidth < 1) || (dstHeight < 1)
        || ((dstWidth % 4) != 0) || ((dstHeight % 2) != 0)
        || ((dstWidth * dstHeight % 128) != 0) //chip constrain
        || (m_hal_param.sensor_output_width < dstWidth) || (m_hal_param.sensor_output_height < dstHeight))
    {
        akprintf(C1, M_DRVSYS, "cam_capture_RGB set param error\n");
        return AK_FALSE;
    }

    if (dst == AK_NULL)
    {
        return AK_FALSE;
    }
    
    return camctrl_capture_RGB(dst, m_hal_param.sensor_output_width, m_hal_param.sensor_output_height, 
        dstWidth, dstHeight, timeout, m_hal_param.bIsCCIR656); 
}

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
T_BOOL cam_capture_JPEG(T_U8 *dstJPEG, T_U32 *JPEGlength, T_U32 timeout)
{
    if (dstJPEG == AK_NULL)
    {
        return AK_FALSE;
    }
    
    return camctrl_capture_JPEG(dstJPEG, JPEGlength, timeout);
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting
 * @date 2010-12-06
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
static T_BOOL cam_init(T_VOID)
{
    int i = 0;
    T_BOOL camera_open_flag = AK_FALSE;
  
    m_hal_param.pCameraHandler = cam_probe();
    
    if (m_hal_param.pCameraHandler != AK_NULL)
    {        
        for (i = 0; i < 3; i++)
        {
            if (m_hal_param.pCameraHandler->cam_init_func() != AK_FALSE)
            {
                camera_open_flag = AK_TRUE;
                break;
            }
            mini_delay(100);
        }

        if (AK_FALSE == camera_open_flag)
        {
            m_hal_param.pCameraHandler->cam_close_func();
            m_hal_param.pCameraHandler = AK_NULL;
        }
    }
    
    return camera_open_flag;
}

/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */    
T_BOOL cam_open(T_VOID)
{    
    camctrl_enable();
    if (cam_init() != AK_FALSE)
    {
        return camctrl_open(m_hal_param.pCameraHandler->cam_mclk);      
    }
    else
    {
        camctrl_disable();
        akprintf(C2, M_DRVSYS, "open camera failed\n");
        return AK_FALSE;
    }
}

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID cam_close(T_VOID)
{    
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_close_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_close_func();
        m_hal_param.pCameraHandler = AK_NULL;
    }
    camctrl_disable();
}

/**
 * @brief Set camera controller CCIR601/656 protocol 
 * @author xia_wenting  
 * @date 2010-12-07
 * @param[in] interface camera controller ccir601 or ccir656
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_interface(T_CAMERA_INTERFACE camera_interface)
{
    if (m_hal_param.pCameraHandler != AK_NULL)
    {
        switch (camera_interface)
        {
            case CAMERA_CCIR_601:
                m_hal_param.bIsCCIR656 = AK_FALSE;
                //m_hal_param.pCameraHandler->cam_set_interface_func(interface);
                break;

            case CAMERA_CCIR_656:
                m_hal_param.bIsCCIR656 = AK_TRUE;
                //m_hal_param.pCameraHandler->cam_set_interface_func(interface);
                break;
        }
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_interface is not supported\r\n");
}

/**
 * @brief Set camera night mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] mode night mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_night_mode(T_NIGHT_MODE mode)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_night_mode_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_night_mode_func(mode);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_night_mode is not supported\r\n");
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_exposure(T_CAMERA_EXPOSURE exposure)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_exposure_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_exposure_func(exposure);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_exposure is not supported\r\n");
}

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_AWB(T_CAMERA_AWB awb)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_AWB_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_AWB_func(awb);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_AWB is not supported\r\n");
}

/**
 * @brief Set camera brightness level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_brightness_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_brightness_func(brightness);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_brightness is not supported\r\n");
}

/**
 * @brief Set camera contrast level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_contrast(T_CAMERA_CONTRAST contrast)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_contrast_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_contrast_func(contrast);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_contrast is not supported\r\n");
}

/**
 * @brief Set camera saturation level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_saturation(T_CAMERA_SATURATION saturation)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_saturation_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_saturation_func(saturation);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_saturation is not supported\r\n");
}

/**
 * @brief Set camera sharpness level 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_sharpness_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_sharpness_func(sharpness);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_sharpness is not supported\r\n");
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_mirror(T_CAMERA_MIRROR mirror)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_mirror_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_mirror_func(mirror);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_mirror is not supported\r\n");
}

/**
 * @brief Set camera effect mode 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
T_VOID cam_set_effect(T_CAMERA_EFFECT effect)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_effect_func != AK_NULL))
    {
        m_hal_param.pCameraHandler->cam_set_effect_func(effect);
        return;
    }
    akprintf(C2, M_DRVSYS, "cam_set_effect is not supported\r\n");
}

/**
 * @brief set camera feature 
 * @author xia_wenting
 * @date 2010-12-01
 * @param[in] feature_type camera feature type,such as night mode,AWB,contrast..etc.
 * @param[in] feature_setting feature setting
 * @return T_VOID
 */
T_VOID cam_set_feature(T_CAMERA_FEATURE feature_type, T_U8 feature_setting)
{
    switch (feature_type)
    {
        case CAM_FEATURE_NIGHT_MODE:
            cam_set_night_mode(feature_setting);
            break;

        case CAM_FEATURE_EXPOSURE:
            cam_set_exposure(feature_setting);
            break;

        case CAM_FEATURE_AWB:
            cam_set_AWB(feature_setting);
            break;

        case CAM_FEATURE_BRIGHTNESS:
            cam_set_brightness(feature_setting);
            break;

        case CAM_FEATURE_CONTRAST:
            cam_set_contrast(feature_setting);
            break;
            
        case CAM_FEATURE_SATURATION:
            cam_set_saturation(feature_setting);
            break;  
            
        case CAM_FEATURE_SHARPNESS:
            cam_set_sharpness(feature_setting);
            break;
            
        case CAM_FEATURE_MIRROR:
            cam_set_mirror(feature_setting);
            break;
            
        case CAM_FEATURE_EFFECT:
            cam_set_effect(feature_setting);
            break;
            
        default:
            akprintf(C1, M_DRVSYS,  "error camera feature\r\n");
            break;
    }
    
}

/**
 * @brief set camera window, but when chip is 3771, to set camera if clip func
 * @author  xia_wenting
 * @date 2011-03-30
 * @param[in] srcWidth window or clip width
 * @param[in] srcHeight window or clip height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
T_S32 cam_set_window(T_U32 srcWidth, T_U32 srcHeight)
{
    T_U32 clip_hoff = 0, clip_voff = 0;
    
    if (srcWidth > m_hal_param.sensor_output_width
        || srcHeight > m_hal_param.sensor_output_height)
    {
        akprintf(C1, M_DRVSYS, "cam_set_window set param error\n");
        return -1;
    }
    else
    {
        if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_window_func != AK_NULL))
        {
            m_hal_param.sensor_output_width = srcWidth;
            m_hal_param.sensor_output_height = srcHeight;
            
            return m_hal_param.pCameraHandler->cam_set_window_func(srcWidth, srcHeight);
        }
    }

    akprintf(C2, M_DRVSYS, "cam_set_window is not supported\r\n");
    return -1;
}

/**
 * @brief Set camera frame rate manually
 * @author xia_wenting  
 * @date 2011-01-05
 * @param[in] framerate camera output framerate
 * @return T_U32
 * @retval 0 if error mode 
 * @retval 1 if success
 */
T_U32 cam_set_framerate(float framerate)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_framerate_func != AK_NULL))
    {
        return m_hal_param.pCameraHandler->cam_set_framerate_func(framerate);      
    }
    
    akprintf(C2, M_DRVSYS, "cam_set_framerate is not supported\r\n");
    return 0;
}

/**
 * @brief switch camera reg to preview mode
 * @author  xia_wenting
 * @date 2011-03-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{
    if ((srcWidth > 4096) || (srcHeight > 4096) || (srcWidth < 1) || (srcHeight < 1))
    {
        return AK_FALSE;
    }
    else if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_to_prev_func != AK_NULL))
    {
        m_hal_param.drv_output_width = srcWidth;
        m_hal_param.drv_output_height = srcHeight;
        
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;
        
        if (AK_TRUE == m_hal_param.pCameraHandler->cam_set_to_prev_func(srcWidth, srcHeight))
        {
            cam_set_window(srcWidth, srcHeight);
            return AK_TRUE;
        }      
    }
    akprintf(C2, M_DRVSYS, "cam_set_to_prev is not supported\r\n");
    return AK_FALSE;    
}

/**
 * @brief switch from preview mode to capture mode
 * @author xia_wenting
 * @date 2011-03-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_to_cap_func != AK_NULL))
    {
        m_hal_param.drv_output_width = srcWidth;
        m_hal_param.drv_output_height = srcHeight;
        
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;
        if (AK_TRUE == m_hal_param.pCameraHandler->cam_set_to_cap_func(srcWidth, srcHeight))
        {
            cam_set_window(srcWidth, srcHeight);
            return AK_TRUE;
        } 
    }
    akprintf(C2, M_DRVSYS, "cam_set_to_cap is not supported\r\n");
    return AK_FALSE;
}

/**
 * @brief switch camera to record mode
 * @author xia_wenting 
 * @date 2011-03-30
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL cam_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_set_to_record_func != AK_NULL))
    {
        m_hal_param.drv_output_width = srcWidth;
        m_hal_param.drv_output_height = srcHeight;
        
        m_hal_param.sensor_output_width = srcWidth;
        m_hal_param.sensor_output_height = srcHeight;
        
        if (AK_TRUE == m_hal_param.pCameraHandler->cam_set_to_record_func(srcWidth, srcHeight))
        {
            cam_set_window(srcWidth, srcHeight);
            return AK_TRUE;
        }
    }
    akprintf(C2, M_DRVSYS, "cam_set_to_record is not supported\r\n");
    return AK_FALSE;    
}

/**
 * @brief get camera type
 * @author xia_wenting
 * @date 2010-12-01
 * @return T_CAMERA_TYPE
 * @retval camera type defined by mega pixels
 */
T_CAMERA_TYPE cam_get_type(T_VOID)
{
    if ((m_hal_param.pCameraHandler != AK_NULL) && (m_hal_param.pCameraHandler->cam_get_type != AK_NULL))
    {
        return(m_hal_param.pCameraHandler->cam_get_type());
    }

    akprintf(C2, M_DRVSYS, "cam_get_type is not supported\r\n");
    
    return CAMERA_2M;    
}

/** 
 * @brief set notify callback function
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @param[in] callback_func callback function
 * @return T_VOID
*/
T_VOID camstream_set_callback(T_fCAMSTREAMCALLBACK callback_func)
{
    m_hal_param.DataNotifyCallbackFunc = callback_func;
}

/** 
 * @brief change camera configure
 * @author xia_wenting
 * @date 2011-03-30
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
                       T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3)
{
    T_U32 clip_hoff = 0, clip_voff = 0, clip_width = 0, clip_height = 0;

    if ((m_hal_param.sensor_output_width > 4096) || (m_hal_param.sensor_output_height > 4096) 
        || (m_hal_param.sensor_output_width < 1) || (m_hal_param.sensor_output_height < 1)
        || (dstWidth > 4096) || (dstHeight > 4096) || (dstWidth < 1) || (dstHeight < 1)
        || ((dstWidth % 4) != 0) || ((dstHeight % 2) != 0)
        || ((dstWidth * dstHeight % 128) != 0) //chip constrain
        || (m_hal_param.sensor_output_width < dstWidth) || (m_hal_param.sensor_output_height < dstHeight))
    {
        akprintf(C1, M_DRVSYS, "camstream_change set param error\n");
        return AK_FALSE;
    } 

    if(m_hal_param.bIsStream_change)
    {
        akprintf(C1, M_DRVSYS, "camstream_change called too frequently\n");
        return AK_FALSE;
    }

    akprintf(C3, M_DRVSYS, "camera change\n");

    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);//mask camera interrupt
       
    memset((T_U8 *)m_hal_param.camera_buffer, 0, sizeof(m_hal_param.camera_buffer));
    m_hal_param.CameraBufferCount = 0;
    
    if (YUV1 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV1->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV1->dU;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV1->dV;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (YUV2 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV2->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV2->dU  ;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV2->dV ;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (YUV3 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV3->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV3->dU ;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV3->dV  ;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (m_hal_param.CameraBufferCount < 2)
    {
        akprintf(C1, M_DRVSYS, "error, buffer count is %d, must > 2\n", m_hal_param.CameraBufferCount);
        return AK_FALSE;
    }

    m_hal_param.CaptureBufferIndex = 0;
    m_hal_param.ReadyBufferIndex = 0;
    m_hal_param.UseBufferIndex = 0;

    if ((m_hal_param.sensor_output_width != dstWidth) || (m_hal_param.sensor_output_height != dstHeight))
    {
        if (m_hal_param.sensor_output_width / dstWidth < 2)
        {
            clip_width = m_hal_param.sensor_output_width - m_hal_param.sensor_output_width % dstWidth;
        }
        else
        {
            clip_width = m_hal_param.sensor_output_width - m_hal_param.sensor_output_width % (2 * dstWidth);
        }

        if (m_hal_param.sensor_output_height / dstHeight < 2)
        {
            clip_height = m_hal_param.sensor_output_height - m_hal_param.sensor_output_height % dstHeight;
        }
        else
        {
            clip_height = m_hal_param.sensor_output_height - m_hal_param.sensor_output_height % (2 * dstHeight);
        }
       
        cam_set_window(clip_width, clip_height);
    }
    
    camctrl_set_info(m_hal_param.sensor_output_width, m_hal_param.sensor_output_height, 
                    dstWidth, dstHeight, m_hal_param.bIsCCIR656);

    cam_start();

    m_hal_param.bIsStream_change = AK_TRUE;

    INTR_ENABLE(IRQ_MASK_CAMERA_BIT);

    return AK_TRUE;
}

static T_VOID cam_start(T_VOID)
{
    if (CAMERA_NO_SUSPEND == m_hal_param.suspend_flag)
    {
        m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status = BUFFER_CAPTURE;

        camctrl_capture_frame(m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dY,
                          m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dU,
                          m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].dV);
    }
}

/** 
 * @brief stop camera(interrupt mode)
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @return T_VOID
 */
T_VOID camstream_stop(T_VOID)
{
    camstream_suspend();
    
    m_hal_param.DataNotifyCallbackFunc = AK_NULL;

    camctrl_set_interrupt_callback(AK_NULL);  

    DrvModule_Terminate_Task(DRV_MODULE_CAMERA);
}

/** 
 * @brief a frame is ready
 * @author yi_ruoxiang
 * @date 2010-12-01
 * @return T_BOOL
 * @retval AK_FALSE if no data
 * @retval AK_TRUE if a frame is ready
 */
T_BOOL camstream_ready(T_VOID)
{
    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);

    if(m_hal_param.camera_buffer[m_hal_param.ReadyBufferIndex].status == BUFFER_READY)
    {    
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        return AK_TRUE;
    }
    else
    {
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        
        return AK_FALSE;
    }
}

/** 
 * @brief get a frame data
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_CAMERA_BUFFER*
 * @retval pointer of current frame data
 */
T_CAMERA_BUFFER *camstream_get(T_VOID)
{
    T_CAMERA_BUFFER *ret = AK_NULL;

    MMU_InvalidateDCache();

    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);

    if (m_hal_param.camera_buffer[m_hal_param.ReadyBufferIndex].status != BUFFER_READY)
    {
        INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
        return AK_NULL;
    }
        
    ret = (T_CAMERA_BUFFER*)&m_hal_param.camera_buffer[m_hal_param.ReadyBufferIndex];
    
    m_hal_param.camera_buffer[m_hal_param.ReadyBufferIndex].status = BUFFER_USE;
    m_hal_param.camera_buffer[m_hal_param.UseBufferIndex].status = BUFFER_EMPTY;
    m_hal_param.UseBufferIndex = m_hal_param.ReadyBufferIndex;

    m_hal_param.ReadyBufferIndex = (m_hal_param.ReadyBufferIndex + 1) % m_hal_param.CameraBufferCount; // read poiniter to next buffer

    if ((m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status == BUFFER_READY) || 
        (m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status == BUFFER_USE))
    {
        m_hal_param.CaptureBufferIndex = (m_hal_param.CaptureBufferIndex + 1) % m_hal_param.CameraBufferCount;
        
        cam_start();
        akprintf(C3, M_DRVSYS, "[ACS]");        
    }
    
    INTR_ENABLE(IRQ_MASK_CAMERA_BIT);
    
    return ret;
}

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
                    T_CAMERA_BUFFER *YUV1, T_CAMERA_BUFFER *YUV2, T_CAMERA_BUFFER *YUV3)
{
    if ((m_hal_param.sensor_output_width > 4096) || (m_hal_param.sensor_output_height > 4096) 
        || (m_hal_param.sensor_output_width < 1) || (m_hal_param.sensor_output_height < 1)
        || (dstWidth > 4096) || (dstHeight > 4096) || (dstWidth < 1) || (dstHeight < 1)
        || ((dstWidth % 4) != 0 || (dstHeight % 2) != 0)
        || ((dstWidth * dstHeight % 128) != 0) //chip constrain
        || (m_hal_param.sensor_output_width < dstWidth) || (m_hal_param.sensor_output_height < dstHeight))
    {
        akprintf(C1, M_DRVSYS, "cam_set_info set param error\n");
        return AK_FALSE;
    } 

    if (!DrvModule_Create_Task(DRV_MODULE_CAMERA))
    {
        return AK_FALSE;
    }

    if (!DrvModule_Map_Message(DRV_MODULE_CAMERA, CAMERA_MESSAGE, cam_message_callback))
    {
        DrvModule_Terminate_Task(DRV_MODULE_CAMERA);
        return AK_FALSE;
    }

    memset((T_U8 *)m_hal_param.camera_buffer, 0, sizeof(m_hal_param.camera_buffer));
    m_hal_param.CameraBufferCount = 0;

    if (YUV1 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV1->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV1->dU;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV1->dV;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (YUV2 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV2->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV2->dU;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV2->dV;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (YUV3 != AK_NULL)
    {
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dY = YUV3->dY;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dU = YUV3->dU;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].dV = YUV3->dV;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].width = dstWidth;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].height = dstHeight;
        m_hal_param.camera_buffer[m_hal_param.CameraBufferCount].status = BUFFER_EMPTY;
        m_hal_param.CameraBufferCount++;
    }

    if (m_hal_param.CameraBufferCount < 2)
    {
        akprintf(C1, M_DRVSYS, "error, buffer count is %d, must > 2\n", m_hal_param.CameraBufferCount);
        return AK_FALSE;
    }

    m_hal_param.CaptureBufferIndex = 0;
    m_hal_param.ReadyBufferIndex = 0;
    m_hal_param.UseBufferIndex = 0;
    m_hal_param.suspend_flag = CAMERA_NO_SUSPEND;
    m_hal_param.interrupt_flag = 0;

    camctrl_set_interrupt_callback(cam_interrupt_callback);

    camctrl_set_info(m_hal_param.sensor_output_width, m_hal_param.sensor_output_height, 
                    dstWidth, dstHeight, m_hal_param.bIsCCIR656);

    cam_start();
  
    return AK_TRUE;    
}

//used to register callback function by DrvModule_Map_Message,
//AKOS used only
static T_VOID cam_message_callback(T_U32 *param, T_U32 len)
{
    if (m_hal_param.DataNotifyCallbackFunc != AK_NULL)
        m_hal_param.DataNotifyCallbackFunc();
}

//used to register callback function by camctrl_set_interrupt_callback
//Chip interrupts used only
static T_VOID cam_interrupt_callback(T_VOID)
{
    T_U8 temp = 0;
    
    m_hal_param.interrupt_flag = 1;
    
    if (camctrl_check_status())    //accept a good frame
    {

        if(m_hal_param.bIsStream_change)            
        {
            m_hal_param.bIsStream_change = AK_FALSE; //abandon the first frame
            cam_start();
            return;            
        }
        
        m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status = BUFFER_READY;
        
        temp = (m_hal_param.CaptureBufferIndex + 1) % m_hal_param.CameraBufferCount;
        if(BUFFER_USE == m_hal_param.camera_buffer[temp].status)
        {
            cam_start();
            akprintf(C2, M_DRVSYS, "[CE2]");
        }
        else
        {
            m_hal_param.camera_buffer[m_hal_param.CaptureBufferIndex].status = BUFFER_READY;
            m_hal_param.CaptureBufferIndex = temp;

            cam_start();

            DrvModule_Send_Message(DRV_MODULE_CAMERA, CAMERA_MESSAGE, AK_NULL);
        }
    }
    else                      //accept a error frame
    {
        //if CHIP_3771_L, don't reset!
        if(CHIP_3771_L != drv_get_chip_version())
        {
            cam_reset_ctl(); 
        } 
        cam_start();

        akprintf(C3, M_DRVSYS, "[CE1]");
    }
}

static T_VOID cam_reset_ctl(T_VOID)
{    
    sysctl_reset(RESET_CAMERA);       
    camctrl_open(m_hal_param.pCameraHandler->cam_mclk);   
    cam_set_window(m_hal_param.drv_output_width, m_hal_param.drv_output_height);
    
    if ((m_hal_param.sensor_output_width >= m_hal_param.drv_output_width) 
        && (m_hal_param.sensor_output_height >= m_hal_param.drv_output_height))
    {
        camctrl_set_info(m_hal_param.sensor_output_width, m_hal_param.sensor_output_height, 
                m_hal_param.drv_output_width, m_hal_param.drv_output_height, m_hal_param.bIsCCIR656);        
    }     
}

/** 
 * @brief suspend camera interface, only accept current frame, other not accept
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_BOOL
 * @retval AK_FALSE if suspend failed
 * @retval AK_TRUE if suspend successful
 */
T_BOOL camstream_suspend(T_VOID)
{
    T_U32 i = 0;
    T_BOOL ret = AK_FALSE;
    
    m_hal_param.suspend_flag = CAMERA_SUSPEND;
    m_hal_param.interrupt_flag = 0;

    while (1)
    {
        if (1 == m_hal_param.interrupt_flag)
        {
            m_hal_param.interrupt_flag = 0;

            ret = AK_TRUE;
            break;
        }

        i++;
        mini_delay(1);

        if (i > 500)
        {
            ret = AK_FALSE;
            break;
        }        
    }

    return ret;
}

/** 
 * @brief resume camera interface, start accept frame
 * @author yi_ruoxiang
 * @date 2010-09-01
 * @return T_VOID
 */
T_VOID camstream_resume(T_VOID)
{
    m_hal_param.suspend_flag = CAMERA_NO_SUSPEND;
    
    cam_start();
}

/**
 * @brief get camera Id, Must be keep camera is open can be read
 * @author LHS
 * @date 2012-08-07
 * @return T_U32 camera id
 * @retval if successed,return driver id
 * @retval if failed,return 0
 */
T_U32 cam_get_id(T_VOID)
{    
    T_U32 ID = 0;
    
    if (m_hal_param.pCameraHandler != AK_NULL)
    {
        ID = m_hal_param.pCameraHandler->cam_read_id_func();
    }
    else
    {
        akprintf(C2, M_DRVSYS, "warning:camera is not open\n");
    }

    return ID;
}


