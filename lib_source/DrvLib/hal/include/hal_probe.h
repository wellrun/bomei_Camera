 /**
 * @file hal_probe.h
 * @brief device probe head file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author guoshaofeng 
 * @date 2010-12-07
 * @version 1.0
 * @ref
 */
#ifndef __HAL_PROBE_H__
#define __HAL_PROBE_H__

#include "drv_fm.h"
#include "drv_keypad.h"
#include "drv_camera.h"
#include "drv_lcd.h"
#include "drv_ts_cap.h"

typedef struct {
    T_U32 DeviceID;
    T_FM_FUNCTION_HANDLER *handler; 
} T_FM_INFO;

typedef struct {
    T_U32  index;
    T_KEYPAD_HANDLE   *handler; 
}T_KEYPAD_TYPE_INFO;

/**
 * @BRIEF lcd probe pointer
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-24
 * @PARAM 
 * @RETURN T_LCD_FUNCTION_HANDLER: lcd device pointer
 * @RETVAL
 */
T_LCD_FUNCTION_HANDLER *lcd_probe(T_eLCD lcd);

/**
 * @brief camera probe pointer
 * @author xia_wenting
 * @date 2010-12-07
 * @param
 * @return T_CAMERA_FUNCTION_HANDLER camera device pointer
 * @retval
 */
T_CAMERA_FUNCTION_HANDLER *cam_probe(T_VOID);


/**
 * @brief probe fm
 * @author zhengwenbo
 * @date 2008-04-17
 * @param T_VOID
 * @return T_FM_FUNCTION_HANDLER: fm function handler
 */
T_FM_FUNCTION_HANDLER *fm_probe(T_VOID);

/**
 * @BRIEF keypad probe pointer
 * @AUTHOR dengjian
 * @DATE 2008-6-2
 * @PARAM 
 * @RETURN T_LCD_FUNCTION_HANDLER: lcd device pointer
 * @RETVAL
 */
T_KEYPAD_HANDLE *keypad_type_probe(T_U32 type);

/**
 * @BRIEF ts cap probe pointer
 * @AUTHOR guoshaofeng
 * @DATE 2008-04-29
 * @PARAM 
 * @RETURN T_TS_CAP_FUNCTION_HANDLER: ts cap device pointer
 * @RETVAL
 */
T_TS_CAP_FUNCTION_HANDLER *ts_cap_probe(T_VOID);

#endif  //__HAL_PROBE_H__

