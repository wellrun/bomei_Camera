/**
 * @file gpio_config_CI3771.h
 * @brief gpio function header file
 *
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
#ifndef __GPIO_CONFIG_CI3771_H__
#define __GPIO_CONFIG_CI3771_H__

#ifdef OS_ANYKA

#ifdef CHIP_AK3771

        /** each GPIO pin use*/
#define GPIO_RING_DETECT            INVALID_GPIO
#define GPIO_LCDBL_CHIP_ENABLE      0             //set to1, lcd backlight chip enable
#define GPIO_KEYPAD_BL              INVALID_GPIO
#define GPIO_AUDIO_AMP              INVALID_GPIO              //28  //set to 1,enable amp   
#define GPIO_POWER_OFF              INVALID_GPIO              //set 1, system power on
#define GPIO_USB_BOOT               104  

#define GPIO_USB_DETECT             51              //9802=84 9805 = 16
#define GPIO_DCIN_DETECT            INVALID_GPIO
#define GPIO_SD_DETECT              INVALID_GPIO
#define GPIO_MMC_DETECT             INVALID_GPIO//5


#define GPIO_NAND_WP                INVALID_GPIO
#define GPIO_DTR                    INVALID_GPIO
#define GPIO_FLASH_LIGHT1           INVALID_GPIO  
#define GPIO_FLASH_LIGHT2           INVALID_GPIO
#ifdef SPIBOOT
#define GPIO_CAMERA_RESET           INVALID_GPIO//13 //用硬件控制
#define GPIO_CAMERA_CHIP_ENABLE     12
#else
#define GPIO_CAMERA_RESET           INVALID_GPIO//16 //用硬件控制
#define GPIO_CAMERA_CHIP_ENABLE     40
#endif
#define GPIO_CAMERA_AVDD            INVALID_GPIO
#define GPIO_MODULE_RESET           INVALID_GPIO
#define GPIO_CHARGING               INVALID_GPIO
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

#define GPIO_KEYAPD_ROW0            49
#define GPIO_KEYAPD_ROW1            47
#define GPIO_KEYAPD_ROW2            INVALID_GPIO

#define GPIO_KEYAPD_COLUMN0         50
#define GPIO_KEYAPD_COLUMN1         48
#define GPIO_KEYAPD_COLUMN2         INVALID_GPIO //PU 3.3V
#define GPIO_KEYAPD_COLUMN3         INVALID_GPIO


#define GPIO_TSCR_ADC               INVALID_GPIO
#define GPIO_BT_WAKEUP              INVALID_GPIO
#define GPIO_INNO_IRQ               INVALID_GPIO
#define GPIO_INNO_RESET             INVALID_GPIO
#define GPIO_INNO_POWER             INVALID_GPIO
#define GPIO_INNO_CS				INVALID_GPIO
#define GPIO_SPEAKER_EN				INVALID_GPIO

#define GPIO_USB_DRV_BUS            2

#define GPIO_I2C_SCL                6
#define GPIO_I2C_SDA                5

#define GPIO_SWITCH_KEY             INVALID_GPIO


#ifdef GPIO_MULTIPLE_USE
            #undef GPIO_MULTIPLE_USE
#endif

#endif	// CHIP_AK3771

#endif  // OS_ANYKA

#endif //#ifndef __GPIO_CONFIG_CI3771_H__

