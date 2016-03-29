/**
 * @FILENAME: ts_adc_list.h
 * @BRIEF Touch pannel list driver head file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-17
 * @VERSION 1.0
 * @REF
 */

#ifndef __TS_ADC_LIST_H__
#define __TS_ADC_LIST_H__

#include "drv_api.h"

/**
 * @BRIEF Calculate coordinate
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-17
 * @PARAM T_U32 *x, T_U32 *y: x,y coordinate pointer,it is output
 * @PARAM T_TSPOINT tsPoint: x,y coordinate struct,it is input
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID tsCalculateCoordinate(T_U32 *x, T_U32 *y, T_TSPOINT tsPoint);

#endif
