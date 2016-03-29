/**
 * @file    detect_CI37XX.c
 * @brief   detect config and register on sundance3
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.03.12
 * @version 1.0
 */
 
/***************************************************************************
 The following device name is frequently-used:
 DEVICE NAME            DESCRIPTION
 ===========================================================================
 UDISK                  USB device
 USBHOST                USB host
 SD                     SD card
 MMC                    MMC Card
 TF                     TF Card
 HP                     earphone
 WIFI                   wifi card
 DC                     DC adapter
 
 ***************************************************************************/ 
 
#ifdef OS_ANYKA
#ifdef CI37XX_PLATFORM

#include "akdefine.h"
#include "hal_detector.h"
#include "gpio_config.h"
#include "arch_init.h"

#if ((defined(CHIP_AK3750)) || (defined(CHIP_AK3771)) || (defined(CHIP_AK3760)) )

#define SD_CONNECTED    (1<<0)
#define MMC_CONNECTED   (1<<1)
#define HP_CONNECTED    (1<<2)


/*
static T_pCSTR devname_list[] = 
{
    "SD",
    "MMC",
    "HP"
};
*/

#define AD_SD_VAL            318
#define AD_SD_MMC_VAL        177
#define AD_SD_HP_VAL         265
#define AD_SD_MMC_HP_VAL     151
#define AD_MMC_VAL           459
#define AD_MMC_HP_VAL        348
#define AD_HP_VAL            656
#define AK_SOCKET_NULL__VAL  1020

#define AD_VAL_WINDAGE       20
/*
static T_VOLTAGE_TABLE AD5_detect[] = 
{
	{AK_SOCKET_NULL__VAL - AD_VAL_WINDAGE, 	AK_SOCKET_NULL__VAL + 4*AD_VAL_WINDAGE, 0},	
	{AD_SD_MMC_HP_VAL - AD_VAL_WINDAGE,		AD_SD_MMC_HP_VAL + AD_VAL_WINDAGE, 		SD_CONNECTED | MMC_CONNECTED | HP_CONNECTED},
	{AD_MMC_HP_VAL - AD_VAL_WINDAGE, 		AD_MMC_HP_VAL + AD_VAL_WINDAGE, 		MMC_CONNECTED | HP_CONNECTED},
	{AD_MMC_VAL - AD_VAL_WINDAGE, 			AD_MMC_VAL + AD_VAL_WINDAGE, 			MMC_CONNECTED },	
	{AD_SD_HP_VAL - AD_VAL_WINDAGE, 		AD_SD_HP_VAL + AD_VAL_WINDAGE, 			SD_CONNECTED | HP_CONNECTED },
	{AD_SD_VAL - AD_VAL_WINDAGE, 			AD_SD_VAL + AD_VAL_WINDAGE, 			SD_CONNECTED },	
	{AD_HP_VAL - AD_VAL_WINDAGE, 			AD_HP_VAL + AD_VAL_WINDAGE, 			HP_CONNECTED},	
	{AD_SD_MMC_VAL - AD_VAL_WINDAGE, 		AD_SD_MMC_VAL + AD_VAL_WINDAGE, 		MMC_CONNECTED | SD_CONNECTED},
    
};
*/
#endif

static T_S32 detector_reg(T_VOID)
{   
	
	detector_register_gpio("UDISK",  GPIO_USB_DETECT, AK_TRUE, AK_TRUE, 100);
#if (defined(CHIP_AK3750))
    detector_register_gpio("MMC",  GPIO_MMC_DETECT, AK_FALSE, AK_TRUE, 100);
#endif
	detector_register_gpio("POWERKEY",  GPIO_POWER_KEY, AK_TRUE, AK_TRUE, 100);
	
/*	
#if (defined(CHIP_AK3753))
	detector_register_gpio("SD",  GPIO_SD_DETECT, AK_FALSE, AK_TRUE, 100);
    detector_register_gpio("MMC",  GPIO_MMC_DETECT, AK_FALSE, AK_TRUE, 100);
	detector_register_gpio("HP",   GPIO_HEADSET_DETECT, AK_FALSE, AK_TRUE, 100);
#else //3750,3760,ad ¼ì²â
    detector_register_adc(devname_list, sizeof(devname_list)/sizeof(devname_list[0]), 
        AD5_detect, sizeof(AD5_detect)/sizeof(AD5_detect[0]), 100);
#endif
*/
    return 0;
}



#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(detector_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif  //CI37XX_PLATFORM
#endif  //OS_ANYKA
