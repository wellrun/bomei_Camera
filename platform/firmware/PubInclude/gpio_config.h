/**
 * @file gpio_config.h
 * @brief gpio function header file
 *
 * @FILENAME: gpio_config.h
 * @BRIEF config gpio
 * Copyright (C) 2008 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2011-04-12
 * @VERSION 1.0
 * @REF
 * @NOTE:
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
 
 * 2. 如果需要用到扩展io，只需要打开GPIO_MULTIPLE_USE宏，并设置对应的gpio
 *    GPIO_EXPAND_OUT1和GPIO_EXPAND_OUT2，如果只有一组扩展io,可以将GPIO_EXPAND_OUT2
 *	  设为INVALID_GPIO即可
 */
#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__

#ifdef OS_ANYKA

#ifndef INVALID_GPIO
#define INVALID_GPIO                    0xfe
#endif

#if (defined (CHIP_AK3771))
#include "gpio_config_CI3771.h"
#elif (defined (CHIP_AK3753))
#include "gpio_config_CI3753.h"
#elif (defined (CHIP_AK3750))
#include "gpio_config_CI3750.h"
#elif (defined (CHIP_AK3760))
#include "gpio_config_CI3760.h"
#else
#error "MUST Include GPIO Config File!!!"
#endif

#endif  // OS_ANYKA

#endif //#ifndef __GPIO_CONFIG_H__

