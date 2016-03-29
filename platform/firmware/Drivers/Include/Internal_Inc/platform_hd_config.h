/**
 * @file
 * @brief ANYKA software
 * 
 */

#ifndef __PLATFORM_HD_CONFIG_H__
/**
 * @def __PLATFORM_HD_CONFIG_H__
 *
 */
#define __PLATFORM_HD_CONFIG_H__




#define UART_VATC_ID   uiUART1
#define UART_BT_ID     uiUART3
#define UART_GPS_ID    MAX_UART_NUM

#define COMPORT_DEFAULT_BAUDRATE    115200
#define COMPORT_V24                 2

#define COMPORT_VATC_1              1
#define COMPORT_VATC_2              11
#define COMPORT_VATC_3              12


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
#define CAMERA_H_SHOT               1   //横屏还是竖屏
/* camera config end */


/*-----------------------------------LCD CONFIG-----------------------------------*/

/* LCD Type, Will Be Set In File \platform\Build\BoardTarget\command_CIXXX.txt
 * LCD=MPU_SPRING_8907 / MPU_S6D04H0 / RGB_OTA5180A ETC.
 */
 
//#define USE_LCD_HD66781
//#define USE_LCD_HX8312
//#define USE_LCD_DC8312
//#define USE_LCD_TRULY8312
//#define USE_LCD_ILI9320
//#define USE_LCD_SSD1289
//#define USE_LCD_HX8347A
//#define USE_LCD_8907A
//#define USE_LCD_SPFD5408A
//#define USE_LCD_IS2102B
//#define USE_LCD_LQ043T3DX02

//#define USE_LCD_MPU_8907A  
//#define USE_LCD_MPU_SPRING_8907
//#define  USE_LCD_RGB_TM050RDZ00
//#define  USE_LCD_RGB_A050VM01
//#define  USE_LCD_RGB_LQ043T3DX02
//#define  USE_LCD_RGB_A050VM01
//#define USE_LCD_RGB_HLY070ML209
//#define USE_LCD_RGB_HSD0701DW1
//#define USE_LCD_RGB_OTA5180A
//#define USE_LCD_MPU_ST7781
//#define USE_LCD_MPU_S6D04H0



//#define USE_LCD_BACKLIGHT_AAT3140
//#define USE_LCD_BACKLIGHT_AAT3155
//#define USE_LCD_BACKLIGHT_AAT3157
//#define USE_LCD_BACKLIGHT_LM2704
//#define USE_LCD_BACKLIGHT_TPS61061
//#define USE_LCD_BACKLIGHT_FAN5607
#define USE_LCD_BACKLIGHT_PT4401

/*-----------------------------------LCD END-----------------------------------*/ 

#define USE_CAMERA_OV9650
//#define USE_CAMERA_OV9653
//#define USE_CAMERA_OV9655
#define USE_CAMERA_OV7670
#define USE_CAMERA_GC0308
//#define USE_CAMERA_PO1200
//#define USE_CAMERA_130PC11
//#define USE_CAMERA_OV2640
#define USE_CAMERA_OV2643
#define USE_CAMERA_OV7725

//#define USE_FM_TEA5767
//#define USE_FM_RDA5800
#define USE_FM_RDA5807



//keypad scan mode
//#define MOUNT_DIODE_SCAN_MODE 
//#define MOUNT_KEYPERGPIO_SCAN_MODE 
//#define MOUNT_MATRIX_SCAN_MODE
//#define MOUNT_ANALOG_SCAN_MODE
//#define MOUNT_MIXED_SCAN_MODE


#endif	// End of define __PLATFORM_HD_CONFIG_H__

