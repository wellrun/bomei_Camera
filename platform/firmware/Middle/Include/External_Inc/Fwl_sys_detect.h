/**
 * @FILENAME: fwl_sys_detect.h
 * @BRIEF sys_delay head file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @VERSION 1.0
 * @REF
 */

#ifndef __SYS_DETECT_H__
#define __SYS_DETECT_H__

#include "anyka_types.h"

#define NONE_FLAG           0x0
#define GPIO_CHARGING_FLAG  0x1
#define GPIO_FULL_FLAG      0x2
#define GPIO_OVER_FLAG      0x4

#define PINIO_EARPHONE_EVENT        5


#ifdef OS_ANYKA
/**
 * @BRIEF system init charger detector callback function
 * @AUTHOR wangguotian
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_charger_detector_init(T_VOID);


/**
 * @BRIEF system init usb detector callback function
 * @AUTHOR wangguotian
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */    
T_VOID sys_usb_detector_init(T_VOID);

/**
 * @BRIEF system init gpio flip callback function
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */ 
T_VOID sys_init_gpio_flip(T_VOID);

/**
 * @BRIEF system init headset detector callback function
 * @AUTHOR wangguotian
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */ 
T_VOID sys_headset_detector_init(T_VOID);


/**
 * @BRIEF system init sd card detector callback function
 * @AUTHOR wangguotian
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */ 
T_VOID sys_sd_detector_init(T_VOID);

/**
 * @BRIEF system init mmc card detector callback function
 * @AUTHOR wangguotian
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */     
T_VOID sys_mmc_detector_init(T_VOID);

#endif


/**
 * @BRIEF system usb detect
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  1: usb disk 0:not usbdisk
 */ 
T_BOOL sys_usb_detect(T_VOID);

/**
 * @BRIEF detect usb disk is connected
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  AK_TRUE: connected  0:disconnected
 */ 
T_BOOL usb_is_connected(T_VOID);

/**
 * @BRIEF enable/disable usb disk detecting
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM  T_BOOL
 * @RETURN T_VOID
 * @RETVAL 
 */ 
T_VOID usb_enable_detect(T_BOOL benable);


/**
 * @BRIEF detect headset is connected
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  AK_TRUE: connected  0:disconnected
 */ 
T_BOOL headset_is_conneted(T_VOID);


/**
 * @BRIEF detect dc charger is connected
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  AK_TRUE: connected  0:disconnected
 */ 
T_BOOL charger_is_conneted(T_VOID);


/**
 * @BRIEF detect sd card is connected
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  AK_TRUE: connected  0:disconnected
 */ 
T_BOOL sd_is_connected(T_VOID);

/**
 * @BRIEF enable/disable sd card detecting
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM  T_BOOL
 * @RETURN T_VOID
 * @RETVAL 
 */ 
T_VOID sd_enable_detect(T_BOOL benable);


/**
 * @BRIEF detect mmc card is connected
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  AK_TRUE: connected  0:disconnected
 */ 
T_BOOL mmc_is_connected(T_VOID);

/**
 * @BRIEF enable/disable mmc card detecting
 * @AUTHOR wangguotian
 * @DATE 
 * @PARAM  T_BOOL
 * @RETURN T_VOID
 * @RETVAL 
 */ 
T_VOID mmc_enable_detect(T_BOOL benable);

T_VOID sys_init_app(T_VOID);

T_VOID sys_powerkey_detector_init(T_VOID);



#endif  //#ifndef __SYS_DETECT_H__

