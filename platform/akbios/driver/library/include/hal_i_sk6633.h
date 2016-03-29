/**@file hal_sd.h
 * @brief provide hal level operations of how to control sd.
 *
 * This file describe sd hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_I_SK6633_H
#define __HAL_I_SK6633_H

#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef T_U8 (*T_SK6633BIN)[512];


T_BOOL sk6633_mmc_init(T_SK6633BIN erase_bin,T_SK6633BIN llf1_bin,T_SK6633BIN llf2_bin,T_U8* llf_param,T_U8*  fdm_bin);
T_pCARD_HANDLE sk6633_mmc_open_card(T_eCARD_INTERFACE cif, T_U8 bus_mode);

#ifdef __cplusplus
}
#endif


#endif 
  
