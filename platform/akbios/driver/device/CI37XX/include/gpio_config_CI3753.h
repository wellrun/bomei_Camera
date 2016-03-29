/**
 * @file gpio_config.h
 * @brief gpio function header file
/**
 * @FILENAME: gpio_config.h
 * @BRIEF config gpio
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @VERSION 1.0
 * @REF
 * @NOTE:
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
 
 * 2. 如果需要用到扩展io，只需要打开GPIO_MULTIPLE_USE宏，并设置对应的gpio
 *    GPIO_EXPAND_OUT1和GPIO_EXPAND_OUT2，如果只有一组扩展io,可以将GPIO_EXPAND_OUT2
 *	  设为INVALID_GPIO即可
 */
#ifndef __GPIO_CONFIG_CI3753_H__
#define __GPIO_CONFIG_CI3753_H__
#include "drv_gpio.h"


#ifdef CHIP_AK3753
        /** each GPIO pin use*/
#define GPIO_I2C_SCL                INVALID_GPIO	// DGPIO0
#define GPIO_I2C_SDA                INVALID_GPIO	// DGPIO1
#define GPIO_RING_DETECT            INVALID_GPIO
#define GPIO_LCDBL_CHIP_ENABLE      INVALID_GPIO               //set to1, lcd backlight chip enable
#define GPIO_KEYPAD_BL              INVALID_GPIO
#define GPIO_AUDIO_AMP              INVALID_GPIO              //28  //set to 1,enable amp   
#define GPIO_POWER_OFF              INVALID_GPIO          //set 1, system power on
#define GPIO_USB_BOOT               INVALID_GPIO  

#define GPIO_USB_DETECT             INVALID_GPIO           //9802=84 9805 = 16


#define GPIO_SWITCH_KEY             INVALID_GPIO
#define GPIO_NAND_WP                INVALID_GPIO
#define GPIO_DTR                    INVALID_GPIO
#define GPIO_FLASH_LIGHT1           INVALID_GPIO	// INVALID_GPIO  
#define GPIO_FLASH_LIGHT2           INVALID_GPIO	// INVALID_GPIO
#define GPIO_CAMERA_RESET           INVALID_GPIO	// 76
#define GPIO_CAMERA_CHIP_ENABLE     INVALID_GPIO // 77
#define GPIO_CAMERA_AVDD            INVALID_GPIO
#define GPIO_MODULE_RESET           INVALID_GPIO
#define GPIO_CHARGING               INVALID_GPIO
#define GPIO_DCIN_DETECT            INVALID_GPIO
#define GPIO_FM_RESET               INVALID_GPIO
#define GPIO_MOTOR                  INVALID_GPIO
#define GPIO_AVDD_EN                INVALID_GPIO
#define GPIO_MIC_SWITCH             INVALID_GPIO  
#define GPIO_AUDIO_SPK_CTL          INVALID_GPIO
#define GPIO_AUDIO_HP_CTL           INVALID_GPIO
#define GPIO_USB_PULL_UP            INVALID_GPIO
#define GPIO_RTS                    INVALID_GPIO
#define GPIO_MODULE_IGT             INVALID_GPIO
#define GPIO_HEADSET_DETECT         INVALID_GPIO 
#define GPIO_CHARGE_STATUS          INVALID_GPIO
#define GPIO_BT_PWREN               INVALID_GPIO
#define GPIO_BT_RST                 INVALID_GPIO
#define GPIO_LCD_SW                 INVALID_GPIO


#define GPIO_HEADSET_MUTE			INVALID_GPIO

#define GPIO_KEYAPD_ROW0            INVALID_GPIO // 40
#define GPIO_KEYAPD_ROW1            INVALID_GPIO // 47
#define GPIO_KEYAPD_COLUMN0         INVALID_GPIO // 50
#define GPIO_KEYAPD_COLUMN1         INVALID_GPIO // 5
#define GPIO_KEYAPD_COLUMN2         INVALID_GPIO // 48


#define GPIO_SD_DETECT              INVALID_GPIO
#define GPIO_TSCR_ADC               INVALID_GPIO
#define GPIO_BT_WAKEUP              INVALID_GPIO
#define GPIO_INNO_IRQ               INVALID_GPIO
#define GPIO_INNO_RESET             INVALID_GPIO
#define GPIO_INNO_POWER             INVALID_GPIO


#ifdef GPIO_MULTIPLE_USE
	#undef GPIO_MULTIPLE_USE
#endif


#endif //CI98XX 



#endif //#ifndef __GPIO_CONFIG_CI3753_H__

