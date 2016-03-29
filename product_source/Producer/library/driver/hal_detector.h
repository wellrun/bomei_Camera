/**
 * @file    hal_detector.c
 * @brief   detector module, for detecting device connected or disconnected
 *          by check gpio or voltage of ADC.
 *          The detected event of gpio can be indicated by interrupt of the 
 *          gpio,or by a timer.
 *          The detected event of ADC is indicated by a timer.
 * 支持防抖处理: 
 *     断开会马上响应;
 *     接合会有防抖处理，防抖时间为1秒钟: 当接合时，若1秒内没有出现断开，即为
 *     有效接合。
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.03.09
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

 
#ifndef _HAL_DETECTOR_H_
#define _HAL_DETECTOR_H_

#include "anyka_types.h"

/**
 * @brief Voltage table structure for ADC checking.
 */
typedef struct _VOLTAGE_TABEL
{
    T_U32   min_voltage;        ///< The minimum of the voltage range
    T_U32   max_voltage;        ///< The maximum of the voltage range
    T_U32   dev_connect_state;  ///< The connecting state of the devices 
}T_VOLTAGE_TABLE ;

/**
 * @brief detector call back function prototype;
 */
typedef T_VOID (*T_fDETECTOR_CALLBACK)(T_BOOL connect_state);


/**
 * @brief       Init detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID 
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_init(T_VOID);


/**
 * @brief       Free detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   T_VOID
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_free(T_VOID);


/**
 * @brief       Set the call back function of device named by devname,the call 
 *              back function will be call when the device inserted or removed.
 *              After seting the call back function, the dettector of the 
 *              device will start automatically.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pcallbackfunc
 *                  Call back function of the device.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_set_callback(T_pCSTR devname,
            T_fDETECTOR_CALLBACK pcallbackfunc);


/**
 * @brief       Enable or disable the detector.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   benable
 *                  AK_TRUE：Enable the detector;
 *                  AK_FALSE：Disable the detector;
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 *
 * @remark      It's not suggested to disable the detector,if the detector
 *              is disable, the connecting state of the device can't be 
 *              informed the user.
 *
 */ 
T_BOOL detector_enable(T_pCSTR devname, T_BOOL benable);

/**
 * @brief       Determine whether the specified window is enabled.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pbenable
 *                  Pointer to a T_BOOL type variable for fetching the 
 *                  detector state.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_is_enabled(T_pCSTR devname, T_BOOL *pbenable);

/**
 * @brief       Get the connecting state of the device named by devname.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a T_BOOL type variable for fetching the 
 *                  connecting state.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_get_state(T_pCSTR devname, T_BOOL *pState);



/**
 * @brief       Register the detector of GPIO type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 *                  devname must point to a const string, because the detect 
 *                  module won't hold a copy of the device name. 
 * @param[in]   gpio_num
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @param[in]   interrupt_mode
 *                  Detect type, AK_TRUE: interrupt, AK_FALSE: time.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL detector_register_gpio(T_pCSTR devname, T_U32 gpio_num, 
            T_BOOL active_level, T_BOOL interrupt_mode, T_U32 interval_ms);


/**
 * @brief       Register the detector of ADC type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname_list
 *                  Name list of the devices to be detected.
 *                  Each pointer in the name list must point to a const 
 *                  string, because the detect module won't hold a copy 
 *                  of the device name. 
 * @param[in]   devnum
 *                  Number of devices to be detected.
 * @param[in]   pvoltage_table
 *                  Voltage table .
 *                  pvoltage_table must point to a const memory, because
 *                  the detect module won't hold a copy of the Voltage 
 *                  table.
 * @param[in]   voltageitem_num
 *                  Number of voltage item of voltage table.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 *
 * @remark      The member dev_connect_state of T_VOLTAGE_TABLE is a bitmap 
 *              of the devices' connecting state.1 means the device is 
 *              connected,and 0 means the device is disconnected.
 *              The bitmap order must correspond to the device's name in 
 *              devname_list, that is, bit 0 of dev_connect_state correspond
 *              to the first device named by devname_list[0], bit 1 of 
 *              dev_connect_state correspond to the second device named by 
 *              devname_list[1].
 *              All the devices specified by devname_list, must have different
 *              name, or else, the action of detector is not foreseeable.
 */ 
T_BOOL detector_register_adc(T_pCSTR *devname_list, T_U32 devnum, 
            const T_VOLTAGE_TABLE  *pvoltage_table, T_U32 voltageitem_num, 
            T_U32 interval_ms);        


#endif //_HAL_DETECTOR_H_

