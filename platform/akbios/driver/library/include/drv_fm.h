/**
 * @file drv_fm.h : fm hardware register level
 * @brief This header file is for fm function prototype
 *
 */
#ifndef __DRV_FM_H__
#define __DRV_FM_H__

#include "hal_fm.h"


typedef struct {
    T_U32   fm_id;
    T_U32   (*fm_read_id)(T_VOID);
    T_BOOL  (*fm_check)(T_VOID);
    T_BOOL  (*fm_init)(T_U32 freq);
    T_U32   (*fm_get_freq)(T_VOID);
    T_BOOL  (*fm_set_freq)(T_U32 freq);
    T_U32   (*fm_get_tune_time)(T_VOID);
    T_FM_SEARCH_RET (*fm_search)(T_U32 freq_s, T_FM_DIR dir, T_U32 *freq);
    T_BOOL  (*fm_find_station)(T_U32 *freq);
    T_BOOL  (*fm_set_range)(T_U32 freq_min, T_U32 freq_max);
    T_BOOL  (*fm_set_step)(T_U32 step);
    T_BOOL  (*fm_set_vol)(T_U16 vol);
    T_BOOL  (*fm_exit)(T_VOID);
    T_BOOL  (*fm_transmit_enable)(T_BOOL enable);
}T_FM_FUNCTION_HANDLER;


/**
 * @brief register device
 * @author zhengwenbo
 * @date 2008-04-17
 * @param[in] id T_U32  device id
 * @param[in] handler T_FM_FUNCTION_HANDLER  device function handler
 * @return AK_TRUE: register success
           AK_FALSE:register fail
 */    
T_BOOL fm_reg_dev(T_U32 id, T_FM_FUNCTION_HANDLER *handler);  

#endif  //__DRV_FM_H__

