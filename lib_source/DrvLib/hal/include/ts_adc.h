/**
 * @file ts_adc.h
 * @brief touch panel ADC driver header
 *
 * This file provides touch panel and ADC APIs: touch panel initialization,  ADC initialization , Get handwriting point list , 
 *  Get current ADC sample point.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LuoXiaoqing
 * @date 2010-08-25
 * @version 1.0
 */

#ifndef _TS_ADC_H_
#define _TS_ADC_H_

/**
 * @brief touch panel  callback define
 * define  touch panel callback type
 */
typedef T_VOID (*T_f_H_TS_ADC_CALLBACK)(const T_TSPOINT *pt); 

/**
 * @brief Initialize touch panel and ADC
 *
 * this function only need call one times when the system power on !
 * But it must  be called after gpio and timer init
 * @author LianGenhui
 * @date 2010-08-25
 * @param[in] callback function
 * @return done
 * @remark  This callback function is very important in this driver! It will be called when the driver
 * get a ADC sample point. These are two special point  (0xff,0x00) means pen up (0xff,0xff) means fond end.
 */
T_BOOL ts_adc_init(T_f_H_TS_ADC_CALLBACK callback, T_U32 gpio, T_U8 active_level);

/**
 * @brief Get the current ADC point's coordinate data
 *
 * @author LianGenhui
 * @date 2010-08-25
 * @param[out] ADpt pointer of the current ADC point's coordinate data
 * @return T_BOOL
 * @retval AK_FALSE means failed
 * @retval AK_TRUE means successful
 * @remark  because the ADC is a 10 bit one, the ADC point value(x and y) is from 0 to 1023.
 *          This function is use for calibrating touch screen .
 */
T_BOOL ts_adc_get_cur_point(T_pTSPOINT ADpt);


#endif  // end _TS_ADC_H_
/*@}*/

