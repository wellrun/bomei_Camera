#ifdef OS_WIN32
#include "drv_fm.h"

T_BOOL fm_check(T_VOID)
{
    return AK_FALSE;
}

/**
 * @brief init Fm Device
 * @author GuoHui
 * @date 2008-03-28
 * @param freq:the last played freq
 * @return AK_TRUE: init ok
           AK_FALSE:init fail
 */
T_BOOL fm_init(T_U32 freq)
{
    return AK_TRUE;
}


/**
 * @brief get freq from fm
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_U32: the freq value
 */
T_U32 fm_get_freq(T_VOID)
{
    return 0;
}


/**
 * @brief write freq into fm
 * @author GuoHui
 * @date 2008-03-28
 * @param freq:the the freq to be write
 * @return AK_TRUE: write freq success
           AK_FALSE:write freq fail
 */
T_BOOL fm_set_freq(T_U32 freq)
{
    return AK_TRUE;
}


/**
 * @brief get the fm tune delay time
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return T_U32: the tune delay time
 */
T_U32 fm_get_tune_time(T_VOID)
{
    return 0;
}

/**
 * @brief fm search station function
 * @author GuoHui
 * @date 2008-03-28
 * @param freq_s:the the freq to be write
 *        dir:search direction
 *        *freq:if find station it return the station freq,
                it reserved for multi task os
 * @return T_FM_SEARCH_RET: search result,refer to define
 */
T_FM_SEARCH_RET fm_search(T_U32 freq_s, T_FM_DIR dir, T_U32 *freq)
{
    return FM_SEARCH_RET_NONE;
}


/**
 * @brief to judge whether search a station
 * @author GuoHui
 * @date 2008-03-28
 * @param freq: used to return the finded station freq
 * @return AK_TRUE: find a station
           AK_FALSE:not find station
 */
T_BOOL fm_find_station(T_U32 *freq)
{
    return AK_FALSE;
}

/**
 * @brief to set fm param accord the freq range
 * @author GuoHui
 * @date 2008-03-28
 * @param min: the min freq
          max: the max freq
 * @return AK_TRUE: set range ok
           AK_FALSE:set range fail
 */
T_BOOL fm_set_range(T_U32 min, T_U32 max)
{
    return AK_TRUE;
}
/**
 * @brief to set fm step
 * @author GuoHui
 * @date 2008-03-28
 * @param step: the size for freq change each time
 * @return AK_TRUE: set range ok
           AK_FALSE:set range fail
 */
T_BOOL fm_set_step(T_U32 step)
{
    return AK_TRUE;
}
/**
 * @brief to set fm volume
 * @author GuoHui
 * @date 2008-03-28
 * @param vol: the system volume(we shall adjust the val between sys and fm)
 * @return AK_TRUE: set vol ok
           AK_FALSE:set vol fail
 */
T_BOOL fm_set_vol(T_U16 vol)
{
    return AK_TRUE;
}

/**
 * @brief exit fm play
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return AK_TRUE: exit ok
           AK_FALSE:exit fail
 */
T_BOOL fm_exit(T_VOID)
{
    return AK_TRUE;
}
/**
 * @brief register device
 * @author zhengwenbo
 * @date 2008-04-17
 * @param[in] T_U32 id: device id
 * @param[in] T_FM_FUNCTION_HANDLER *handler: device function handler
 * @return AK_TRUE: register success
           AK_FALSE:register fail
 */    
T_BOOL fm_reg_dev(T_U32 id, T_FM_FUNCTION_HANDLER *handler)    
{
    return AK_TRUE;
}
#endif