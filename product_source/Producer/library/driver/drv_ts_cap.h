/**
 * @file drv_ts_cap.h
 * @brief ts cap driver.
 *
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2008-04-29
 * @version 1.0
 */
#ifndef __DRV_TS_CAP_H__
#define __DRV_TS_CAP_H__

#include "hal_ts.h"

#define TS_CAP_NOT_USED     0xfffff
#define TS_CAP_MAX_SUPPORT  20


typedef T_VOID (*T_TS_CAP_CALLBACK)(const T_TSPOINT *pt); 


typedef struct
{
    T_VOID (*ts_cap_open_func)(T_VOID);
    T_VOID (*ts_cap_close_func)(T_VOID);
    T_U32  (*ts_cap_read_id_func)(T_VOID);
    T_VOID (*ts_cap_init_func)(T_TS_CAP_CALLBACK callback, T_U32 gpio, T_U8 active_level);
    T_BOOL (*ts_cap_set_param_func)(E_TS_CAP_EVENT event, U_TS_CAP_PARAM param);
    T_BOOL (*ts_cap_get_info_func)(E_TS_CAP_EVENT event, U_TS_CAP_PARAM *param);
}T_TS_CAP_FUNCTION_HANDLER;


/**
 * @brief register ts cap device
 * @author guoshaofeng
 * @date 2008-04-29
 * @param [in] id ts cap id
 * @param [in] handler ts cap device pointer
 * @return T_BOOL
 * @retval AK_TURE register successful
 * @retval AK_FALSE register failed
 */
T_BOOL ts_cap_reg_dev(T_U32 id, T_TS_CAP_FUNCTION_HANDLER *handler);


#endif

