/**
* @file lcd.h
* @brief lcd register bit define head file
* 
* Copyright (C) 2007 Anyka (Guang zhou) Software Technology Co., LTD
* 
* @date 2010-06-09
* @version 0.1
*/

#ifndef _LCD_H_
#define _LCD_H_

#include "anyka_types.h"


/** @defgroup LCD LCD group
 *      @ingroup Drv_Lib
 */
/*@{*/

/** @{@name LCD config define
 *      Define LCD controller config value and bit map
 *
 */
#define MAIN_LCD_MPU_CMD            (0 << 24)      //master LCD command
#define MAIN_LCD_MPU_DATA           (2 << 24)      //master LCD data
#define SUB_LCD_MPU_CMD             (1 << 24)      //slaver LCD command
#define SUB_LCD_MPU_DATA            (3 << 24)      //slaver LCD data
/** @} */


//TVOUT size definition
#define TV_WIDTH                    720
#define TV_PAL_HEIGHT               576
#define TV_NTSC_HEIGHT              480


#define OSD_CHANNEL_EN              (1 << 0)
#define YUV2_CHANNEL_EN             (1 << 1)
#define YUV1_CHANNEL_EN             (1 << 2)
#define RGB_CHANNEL_EN              (1 << 3)
#define RGB_VIR_EN                  (1 << 29)   //enable virtual page function of rgb channal
#define MPU_INTERFACE_MODE          (1 << 5)

#define MPU_REFLASH_START           (1 << 3)
#define MPU_REFLASH_DISABLE         (0 << 3)
#define RGB_REFLASH_START           (1 << 2)
#define RGB_REFLASH_DISABLE         (0 << 2)
#define TV_REFLASH_START            (1 << 1)
#define TV_REFLASH_DISABLE          (0 << 1)
#define RGB_SYSTEIM_DISABLE         (1)

//lcd status
#define FIFO_ALARM_STAT             (1 << 18)
#define ALERT_VALID_STAT            (1 << 17)
#define TV_REFLASH_START_STAT       (1 << 10)
#define TV_REFLASH_OK_STAT          (1 << 9)
#define RGB_REFLASH_START_STAT      (1 << 4)
#define RGB_REFLASH_OK_STAT         (1 << 3)
#define MPU_DISPLAY_OK_STAT         (1 << 2)
#define MPU_DISPLAY_START_STAT      (1 << 1)
#define RGB_SYS_ERROR_STAT          (1)

#define YUV1_V_SCALER_ENABLE        (1 << 22)
#define YUV1_H_SCALER_ENABLE        (1 << 23)

#define LCD_GO_SYS_STOP             (1 << 0)
#define LCD_GO_TV                   (1 << 1)
#define LCD_GO_RGB                  (1 << 2)
#define LCD_GO_MPU                  (1 << 3)


/*@}*/

#endif
