/**
 * @filename: ts_cap.c
 * @brief ts cap driver.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2008-04-29
 * @version 1.0
 * @ref
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_probe.h"
#include "ts_cap.h"

static T_TS_CAP_FUNCTION_HANDLER *pTsCapHandler = AK_NULL;


T_VOID ts_cap_init(T_TS_CAP_CALLBACK callback, T_U32 gpio, T_U8 active_level)
{
    pTsCapHandler = ts_cap_probe();
	
    if ((pTsCapHandler != AK_NULL) && (pTsCapHandler->ts_cap_init_func != AK_NULL))
    {
        pTsCapHandler->ts_cap_init_func(callback, gpio, active_level);
    }
    else
    {
        akprintf(C2, M_DRVSYS, "ts_cap_init is not supported\r\n");
    }
}

T_VOID ts_cap_close(T_VOID)
{	
    if ((pTsCapHandler != AK_NULL) && (pTsCapHandler->ts_cap_close_func != AK_NULL))
    {
        pTsCapHandler->ts_cap_close_func();
    }
    else
    {
        akprintf(C2, M_DRVSYS, "ts_cap_close is not supported\r\n");
    }
}

/**
 * @brief set ts cap param
 * @author guoshaofeng
 * @date 2008-04-29
 * @param[in] event refer to E_TS_CAP_EVENT definition
 * @param[in] param refer to U_TS_CAP_PARAM definition
 * @return T_BOOL
 * @retval AK_TURE set ts cap param successful
 * @retval AK_FALSE set ts cap param failed
 */
T_BOOL ts_cap_set_param(E_TS_CAP_EVENT event, U_TS_CAP_PARAM param)
{
    if ((pTsCapHandler != AK_NULL) && (pTsCapHandler->ts_cap_set_param_func != AK_NULL))
    {
        return pTsCapHandler->ts_cap_set_param_func(event, param);
    }
    
    akprintf(C2, M_DRVSYS, "ts_cap_set_param is not supported\r\n");
    return AK_FALSE;	
}

/**
 * @brief get ts cap info
 * @author guoshaofeng
 * @date 2008-04-29
 * @param[in] event refer to E_TS_CAP_EVENT definition
 * @param[in] param pointer, refer to U_TS_CAP_PARAM definition
 * @return T_BOOL
 * @retval AK_TURE get ts cap info successful
 * @retval AK_FALSE get ts cap info failed
 */
T_BOOL ts_cap_get_info(E_TS_CAP_EVENT event, U_TS_CAP_PARAM *param)
{
    if ((pTsCapHandler != AK_NULL) && (pTsCapHandler->ts_cap_get_info_func != AK_NULL))
    {
        return pTsCapHandler->ts_cap_get_info_func(event, param);
    }
    
    akprintf(C2, M_DRVSYS, "ts_cap_get_info is not supported\r\n");
    return AK_FALSE;	
}


