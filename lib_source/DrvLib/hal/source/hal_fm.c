/**
 * @file fm.c
 * @brief fm driver, define fm APIs.
 * This file provides fm APIs: fm initialization,
 * play fm, etc..
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guohui
 * @date 2008-03-019
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_fm.h"
#include "hal_probe.h"

#ifdef OS_ANYKA
static T_FM_FUNCTION_HANDLER *m_fm_handle = AK_NULL;
#endif

/**
 * @brief Check Fm Device
 * @author GuoHui
 * @date 2008-03-28
 * @param T_VOID
 * @return AK_TRUE: device is ok
           AK_FALSE:device is error
 */
T_BOOL fm_check()
{
    T_BOOL ret = AK_TRUE;

#ifdef OS_ANYKA
    T_U8 i = 0;
    T_U32 id;

    m_fm_handle = fm_probe();
    if (AK_NULL != m_fm_handle)
    {
        ret = m_fm_handle->fm_check();
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    T_U8   i = 0;
    T_U32  id;

    if (AK_NULL == m_fm_handle)
    {
        m_fm_handle = fm_probe();

        if (AK_NULL != m_fm_handle)
        {
            ret = m_fm_handle->fm_init(freq);
        }
        else
        {
            akprintf(C2, M_DRVSYS, "FM init fail\n");
            ret = AK_FALSE;
        }

    }
    else
    {
        ret = m_fm_handle->fm_init(freq);
    }    
    #endif
    
    return ret;
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
    T_U32 freq = 0;
    
    #ifdef OS_ANYKA 
    if (m_fm_handle)
    {
        freq = m_fm_handle->fm_get_freq();
    }
    else
    {
        freq = 0;
    }
#endif

    return freq;
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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_set_freq(freq);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_U32 ret = 0;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_get_tune_time();
    }
    else
    {
        ret = 0;
    }
#endif

    return ret;
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
    T_FM_SEARCH_RET ret = FM_SEARCH_RET_ERROR;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_search(freq_s, dir, freq);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;

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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_find_station(freq);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_set_range(min, max);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_BOOL ret = AK_TRUE;

    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_set_step(step);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_set_vol(vol);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
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
    T_BOOL ret = AK_TRUE;
    
    #ifdef OS_ANYKA
    if (m_fm_handle)
    {
        ret = m_fm_handle->fm_exit();
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
}

/**
 * @brief enable fm transmit function
 * @author guoshaofeng
 * @date 2008-11-05
 * @param enable: AK_TRUE, transmit enable, receive disable; 
                            AK_FALSE, transmit disable, receive enable
 * @return AK_TRUE: transmit enable success
           AK_FALSE:transmit enable fail
 */
T_BOOL fm_transmit_enable(T_BOOL enable)
{
    T_BOOL ret = AK_TRUE;

#ifdef OS_ANYKA
    if ((m_fm_handle != AK_NULL) && (m_fm_handle->fm_transmit_enable != AK_NULL))
    {
        ret = m_fm_handle->fm_transmit_enable(enable);
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
}

/* end of file */
