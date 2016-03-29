/**
 * @file hal_fm.h : fm hardware abstract level
 * @brief This header file is for fm function prototype
 *
 */
#ifndef __HAL_FM_H__
#define __HAL_FM_H__

#include  "akdefine.h"

#define FM_VOL_MAX_LEVLE    10


typedef enum {
	FM_DIR_FREQ_INC = 0,
	FM_DIR_FREQ_DEC
}T_FM_DIR;

typedef enum {
	FM_SEARCH_RET_ONE = 0,		//search one station
	FM_SEARCH_RET_NONE,			//no station is searched
	FM_SEARCH_RET_ERROR
}T_FM_SEARCH_RET;


/**
 * @brief Check Fm Device
 * @author GuoHui
 * @date 2008-03-28
 * @return AK_TRUE: device is ok
           AK_FALSE:device is error
 */
T_BOOL fm_check(T_VOID);

/**
 * @brief init Fm Device
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] freq the last played freq
 * @return AK_TRUE: init ok
           AK_FALSE:init fail
 */
T_BOOL fm_init(T_U32 freq);


/**
 * @brief get freq from fm
 * @author GuoHui
 * @date 2008-03-28
 * @return T_U32: the freq value
 */
T_U32 fm_get_freq(T_VOID);


/**
 * @brief write freq into fm
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] freq the freq to be write
 * @return AK_TRUE: write freq success
           AK_FALSE:write freq fail
 */
T_BOOL fm_set_freq(T_U32 freq);


/**
 * @brief get the fm tune delay time
 * @author GuoHui
 * @date 2008-03-28
 * @return T_U32: the tune delay time
 */
T_U32 fm_get_tune_time(T_VOID);


/**
 * @brief fm search station function
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] freq_s the the freq to be write
 * @param[in] dir search direction
 * @param[out] *freq if find station it return the station freq,
                it reserved for multi task os
 * @return T_FM_SEARCH_RET: search result,refer to define
 */
T_FM_SEARCH_RET fm_search(T_U32 freq_s, T_FM_DIR dir, T_U32 *freq);



/**
 * @brief to judge whether search a station
 * @author GuoHui
 * @date 2008-03-28
 * @param[out] freq used to return the finded station freq
 * @return AK_TRUE: find a station
           AK_FALSE:not find station
 */
T_BOOL fm_find_station(T_U32 *freq);

/**
 * @brief to set fm param accord the freq range
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] min the min freq
 * @param[in] max the max freq
 * @return AK_TRUE: set range ok
           AK_FALSE:set range fail
 */
T_BOOL fm_set_range(T_U32 min, T_U32 max);

/**
 * @brief to set fm step
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] step the size for freq change each time
 * @return AK_TRUE: set range ok
           AK_FALSE:set range fail
 */
T_BOOL fm_set_step(T_U32 step);

/**
 * @brief to set fm volume
 * @author GuoHui
 * @date 2008-03-28
 * @param[in] vol the system volume(we shall adjust the val between sys and fm)
 * @return AK_TRUE: set vol ok
           AK_FALSE:set vol fail
 */
T_BOOL fm_set_vol(T_U16 vol);


/**
 * @brief exit fm play
 * @author GuoHui
 * @date 2008-03-28
 * @return AK_TRUE: exit ok
           AK_FALSE:exit fail
 */
T_BOOL fm_exit(T_VOID);

/**
 * @brief enable fm transmit function
 * @author guoshaofeng
 * @date 2008-11-05
 * @param[in] enable AK_TRUE, transmit enable, receive disable; 
                            AK_FALSE, transmit disable, receive enable
 * @return AK_TRUE: transmit enable success
           AK_FALSE:transmit enable fail
 */
T_BOOL fm_transmit_enable(T_BOOL enable);
 

#endif  

