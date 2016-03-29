/**
 * @file
 * @brief ANYKA software
 * 
 */

#ifndef __CI9802_HD_CONFIG_H__
/**
 * @def __CI9802_HD_CONFIG_H__
 *
 */
#define __CI9802_HD_CONFIG_H__

// #include "mmi_ui.h"

//keypad scan
#define MOUNT_DIODE_SCAN_MODE
#define MOUNT_KEYPERGPIO_SCAN_MODE
#define MOUNT_MATRIX_SCAN_MODE


#define UART_VATC_ID   uiUART1
#define UART_BT_ID     uiUART4
#define UART_GPS_ID    MAX_UART_NUM

#define COMPORT_DEFAULT_BAUDRATE    115200
#define COMPORT_V24                 2

#define COMPORT_VATC_1              1
#define COMPORT_VATC_2              11
#define COMPORT_VATC_3              12
#define MAX_LCD_NUM                 1

#ifndef USE_INBUILD_PA
    #define USE_INBUILD_PA
#endif


#ifndef SUPPORT_PHYSKEYPAD
    #define SUPPORT_PHYSKEYPAD
#endif

#ifndef WM_SUPPORT_GBK15FONTLIB
    #define WM_SUPPORT_GBK15FONTLIB
#endif


#define USE_GPIO_DETECT_HEADSET
    
#define USE_T_FLASH
//#define USE_SD_CARD

#ifndef SUPPORT_MODULE_RESET
    #define SUPPORT_MODULE_RESET 
#endif

/* camera config */
#define SIMULATOR_GAME_H							1	//使用横屏
#define CAMERA_H_SHOT							1	//横屏还是竖屏

//#define CAMERA_P3M_TO_1P3M                    1        //0.3mega camera to 1.3mega
#define CAMERA_1P3M_TO_2M                        1       //1.3mega camera to 2mega

/* camera config end */

/*must open the macro if use RGB lcd*/
#define USE_RGB_LCD

//#define USE_LCD_HD66781
//#define USE_LCD_HX8312
//#define USE_LCD_DC8312
//#define USE_LCD_TRULY8312
//#define USE_LCD_ILI9320
//#define USE_LCD_SSD1289
#define USE_LCD_HX8347A
#define USE_LCD_8907A
#define USE_LCD_SPFD5408A
#define USE_LCD_IS2102B

//#define USE_LCD_BACKLIGHT_AAT3140
//#define USE_LCD_BACKLIGHT_AAT3155
//#define USE_LCD_BACKLIGHT_AAT3157
//#define USE_LCD_BACKLIGHT_LM2704
//#define USE_LCD_BACKLIGHT_TPS61061
//#define USE_LCD_BACKLIGHT_FAN5607
//#define USE_LCD_BACKLIGHT_PT4401

#define USE_CAMERA_OV9650
#define USE_CAMERA_OV9653
#define USE_CAMERA_OV9655
#define USE_CAMERA_OV7670
#define USE_CAMERA_PO1200
#define USE_CAMERA_130PC11
#define USE_CAMERA_OV2640

//#define USE_FM_TEA5767
#define USE_FM_RDA5800




//keypad scan mode
#define MOUNT_DIODE_SCAN_MODE 
#define MOUNT_KEYPERGPIO_SCAN_MODE 
#define MOUNT_MATRIX_SCAN_MODE 
#define MOUNT_MIXED_SCAN_MODE


#endif

