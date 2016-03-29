/**
 * @FILENAME: fwl_power.h
 * @BRIEF power driver head file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-27
 * @VERSION 1.0
 * @REF
 */
#ifndef __POWER_H__
#define __POWER_H__

#include "anyka_types.h"

#define IN_CHARGING                        1                            // it is in charging status
#define NOT_CHARGING                    0                            // not charging now

#define BATTERY_KILL_WAVE               3           // used for battery detection, avoid voltage wave
#define BATTERY_VALUE_AVDD              1500
#define BATTERY_VOL_FORMULA(vol_ad)     ((vol_ad * 3 * BATTERY_VALUE_AVDD) >> 10)

#define BATTERY_VALUE_CHANGE        4100

#ifdef USB_HOST
#define BATTERY_VALUE_WARN_USBHOST  3650
#endif

#define BATTERY_VALUE_WARN          3550
#define BATTERY_VALUE_MIN           3450
#define BATTERY_VALUE_MAX           4180
#define BATTERY_VALUE_TEST          3700
#define BATTERY_VALUE_INVALID       0xFFFFFFFF



#ifdef WM_ADI
    //for ADI module, the battery control is not same as simcom
    //and the low battery voltage is also not same
    #define NEED_BAT_VOLATGE                3450                        //the lowest volatge of battery we need
    #define BAT_FULL_VOLTAGE                4300
    #define BAT_LV0_LOW                        3630
    #define BAT_LV0_HIGH                    3730
    #define BAT_LV1_LOW                        3680
    #define BAT_LV1_HIGH                    3790
    #define BAT_LV2_LOW                        3750
    #define BAT_LV2_HIGH                    3900
    #define BAT_LV3_LOW                        3850
    #define BAT_LV3_HIGH                    3950
    #define BAT_LV4_LOW                        3920
    #define BAT_LV4_HIGH                    4200
    #define BAT_START_LOW_POWEROFF            3400
    #define BAT_START_LOW_WARNING            3600
#endif

#ifdef WM_SKYWORKS    //by ouyang 12.4jh
    #define NEED_BAT_VOLATGE                3400                        //the lowest volatge of battery we need
    #define BAT_FULL_VOLTAGE                4300
    #define BAT_LV0_LOW                        3630
    #define BAT_LV0_HIGH                    3730
    #define BAT_LV1_LOW                        3680
    #define BAT_LV1_HIGH                    3790
    #define BAT_LV2_LOW                        3750
    #define BAT_LV2_HIGH                    3900
    #define BAT_LV3_LOW                        3850
    #define BAT_LV3_HIGH                    3950
    #define BAT_LV4_LOW                        3920
    #define BAT_LV4_HIGH                    4300
    #define BAT_START_LOW_POWEROFF            3400
    #define BAT_START_LOW_WARNING            3600
#endif

#ifdef WM_SIMCOM
    #define NEED_BAT_VOLATGE                3500                        //the lowest volatge of battery we need
    #define BAT_FULL_VOLTAGE                4050
    #define BAT_LV0_LOW                        3630
    #define BAT_LV0_HIGH                    3730
    #define BAT_LV1_LOW                        3680
    #define BAT_LV1_HIGH                    3790
    #define BAT_LV2_LOW                        3750
    #define BAT_LV2_HIGH                    3850
    #define BAT_LV3_LOW                        3820
    #define BAT_LV3_HIGH                    3930
    #define BAT_LV4_LOW                        3900
    #define BAT_LV4_HIGH                    4050
    #define BAT_START_LOW_POWEROFF            3400
    #define BAT_START_LOW_WARNING            3600
#endif

#ifdef WM_INFINEON
    #define NEED_BAT_VOLATGE                3480                        //the lowest volatge of battery we need
    #define BAT_FULL_VOLTAGE                4200
    #define BAT_LV0_LOW                        3480
    #define BAT_LV0_HIGH                    3600
    #define BAT_LV1_LOW                        3600
    #define BAT_LV1_HIGH                    3710
    #define BAT_LV2_LOW                        3710
    #define BAT_LV2_HIGH                    3780
    #define BAT_LV3_LOW                        3780
    #define BAT_LV3_HIGH                    3900
    #define BAT_LV4_LOW                        3900
    #define BAT_LV4_HIGH                    4200
    #define BAT_START_LOW_POWEROFF            3400
    #define BAT_START_LOW_WARNING            3600
#endif
//low battery hint style bit
#define BAT_LOW_PROCESS_ALL                 0xffff
#define BAT_LOW_PROCESS_IDLE_MODE           0x0001
#define BAT_LOW_PROCESS_HINT_MSGBOX         0x0002
#define BAT_LOW_PROCESS_HINT_TONE           0x0004

/** @{@name System Clock divider
 *    define the system running clock divider
 */
#define SYSTEM_FULL_CLOCK_DIV    2
#define SYSTEM_IDLE_CLOCK_DIV    2
#define SYSTEM_SLEEP_CLOCK_DIV    2

#ifdef WM_INFINEON
    #define SYSTEM_PHONE_CLOCK_DIV    4
    #define SYSTEM_CHARGE_CLOCK_DIV    2
#else
    #define SYSTEM_PHONE_CLOCK_DIV    16
    #define SYSTEM_CHARGE_CLOCK_DIV    16
#endif

typedef enum {
    eSYS_BOOT = 0,
    eSYS_IDLE,
    eSYS_FULL,
    eSYS_PHONE,
    eSYS_SLEEP,
    eSYS_STANDBY
}E_SYS_MODE;

typedef enum {
    eRESET_ONLY_MODE = 0,
    eRESET_BEFORE_PIN_MODE,
    eRESET_SET_PIN_MODE,
    eRESET_FULL_MODE
}E_RESET_MODE;

/**
 * @BRIEF get system current mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN E_SYS_MODE
 * @RETVAL eSYS_BOOT,eSYS_IDLE,eSYS_FULL,eSYS_PHONE,eSYS_SLEEP,eSYS_STANDBY
 */
E_SYS_MODE sys_get_mode(T_VOID);

/**
 * @BRIEF enter boot mode
 *        it is the first mode of system
 *        while enter boot mode, the system clock will be set to default
 *        the system frequency is same as full mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL enter_boot_mode(T_VOID);
/**
 * @BRIEF enter idle mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL enter_idle_mode(T_VOID);

/**
 * @BRIEF enter full mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL enter_full_mode(T_VOID);

/**
 * @BRIEF enter phone mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL enter_phone_mode(T_VOID);

/**
 * @BRIEF enter sleep mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL enter_sleep_mode(T_VOID);

/**
 * @BRIEF exit sleep mode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-25
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL: if failed, return AK_FALSE
 */
T_BOOL exit_sleep_mode(T_VOID);

/**
 * @BRIEF Monitor battery voltage
 *      called in ddpubtimerhandler
 * @AUTHOR 
 * @DATE 2008-04-24
 * @PARAM[in] T_U32 millisecond
 * @RETURN T_VOID
 * @RETVAL: 
 */
T_VOID MonitorBatteryVoltage(T_U32 millisecond);

/**
 * @BRIEF Get battery voltage
 *      its return value is a average value
 * @AUTHOR 
 * @DATE 2008-04-24
 * @PARAM[in] T_VOID
 * @RETURN T_U32
 * @RETVAL: battery voltage
 */
T_U32 Fwl_GetBatteryVoltage(T_VOID);

/**
 * @brief Judge whether use extern charge
 * 
 * @author Guohui
 * @date 2008-4-24
 * @param T_VOID
 * @return T_BOOL
 * @retval 1: charge is in   0:charge is out
 */
T_BOOL Fwl_UseExternCharge(T_VOID);

/**
 * @brief Judge whether charging is finished
 * 
 * @author Guohui
 * @date 2008-4-24
 * @param T_VOID
 * @return T_BOOL
 * @retval 1: charge finish   0: charge not finish
 */
T_BOOL Fwl_ChargeVoltageFull(T_VOID);

T_VOID VME_Terminate(T_VOID);

T_U32 Fwl_SetChipStandby(T_VOID);

/*@} */
#endif


