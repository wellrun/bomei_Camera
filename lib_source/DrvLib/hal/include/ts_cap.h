/**
 * @filename: ts_cap.h
 * @brief ts cap driver.
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2008-04-29
 * @version 1.0
 * @ref
 */
#ifndef __TS_CAP_H__
#define __TS_CAP_H__

/**
 * @brief init ts cap device
 * @author guoshaofeng
 * @date 2008-04-29
 * @param[in] callback ts cap callback function.
 * @param[in] gpio gpio pin ID.
 * @param[in] active_level 1 means active high level. 0 means active low level.
 * @return T_VOID
 */
T_VOID ts_cap_init(T_TS_CAP_CALLBACK callback, T_U32 gpio, T_U8 active_level);

/**
 * @brief close ts cap device
 * @author guoshaofeng
 * @date 2008-04-29
 * @param
 * @return T_VOID
 */
T_VOID ts_cap_close(T_VOID);

#endif

