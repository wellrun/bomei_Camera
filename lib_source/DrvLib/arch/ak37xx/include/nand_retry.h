/** @file
 * @brief Define the nand retry infomation
 *
 * Define the the nand retry infomation
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2014-07-16
 * @version 1.0
 * @note
*/

#ifndef __READ_RETRY_H__
#define __READ_RETRY_H__


#include "anyka_types.h"
//#include "drv_cfg.h"


#if DRV_SUPPORT_NAND > 0
#if NAND_SUPPORT_RR > 0

#define IDL_Hynix_26nm_64gb   0xd294dead
#define IDH_Hynix_26nm_64gb   0x4304
#define IDL_Hynix_26nm_32gb   0xda94d7ad
#define IDH_Hynix_26nm_32gb   0xc374

#define IDL_Hynix_20nm_64gb_Adie   0xda94dead
#define IDH_Hynix_20nm_64gb_Adie   0xc474
#define IDL_Hynix_20nm_32gb_Cdie   0x9194d7ad
//#define IDH_Hynix_20nm_Adie   0x4304
//#define IDL_Hynix_20nm_64gb_Cdie   0xda94d7ad
//#define IDL_Hynix_20nm_64gb_Cdie_HI   0x21
//#define IDH_Hynix_20nm_Cdie   0xc374

#define IDL_Toshiba_24nm_32gb   0x3294d798
#define IDH_Toshiba_24nm_32gb   0x5676
#define IDL_Toshiba_24nm_64gb   0x8294de98
#define IDH_Toshiba_24nm_64gb   0x5676

#define IDL_Toshiba_19nm_64gb_DC   0x9384de98
#define IDH_Toshiba_19nm_64gb_DC   0x5772
#define IDL_Toshiba_19nm_128gb_DC  0x93853a98
#define IDH_Toshiba_19nm_128gb_DC  0x5776

#define IDL_Toshiba_19nm_64gb_DD   0x9394de98
#define IDH_Toshiba_19nm_64gb_DD   0x5776
#define IDL_Toshiba_19nm_128gb_DD  0x93953a98
#define IDH_Toshiba_19nm_128gb_DD  0x577a
#define IDL_Toshiba_1ynm_128gb_DD  0x9394de98
#define IDH_Toshiba_1ynm_128gb_DD  0x5076

#define IDL_Samsung_21nm           0x7e94d7ec
#define IDL_Micron_20nm_64gb       0x4b44642c
#define IDL_Micron_20nm_32gb       0x4b44442c

T_VOID modify_scales(T_U8 nTarget);
T_VOID revert_scales(T_U8 nTarget);
T_VOID retry_init(T_U8 nTarget,T_U32 nID[2]);


#define NAND_RR_H27UBG8T2B        1
#define NAND_RR_H27UBG8T2C        1
#define NAND_RR_H27UCG8T2M        1
#define NAND_RR_H27UCG8T2A        1
#define NAND_RR_H27UCG8T2C        1
#define NAND_RR_TOSHIBA_24NM      1
#define NAND_RR_TOSHIBA_19NM      1
#define NAND_RR_TOSHIBA_1YNM      1
#define NAND_RR_SAMSUNG_21NM      1  
#define NAND_RR_MICRON_20NM       1 


#define NAND_ENHANCED_SLC_H27UBG8T2B        1
#define NAND_ENHANCED_SLC_H27UBG8T2C        1
#define NAND_ENHANCED_SLC_H27UCG8T2M        1
#define NAND_ENHANCED_SLC_H27UCG8T2A        1
#define NAND_ENHANCED_SLC_H27UCG8T2C        1
#define NAND_ENHANCED_SLC_TOSHIBA_24NM      1
#define NAND_ENHANCED_SLC_TOSHIBA_19NM      1
#define NAND_ENHANCED_SLC_TOSHIBA_1YNM      1
#define NAND_ENHANCED_SLC_MICRON_20NM       1
#define NAND_ENHANCED_SLC_SAMSUNG_21NM      1
#define NAND_ENHANCED_SLC_MICRON_20NM       1 


#else

#define modify_scales(nTarget)      while(0)
#define revert_scales(nTarget)      while(0)
#define retry_init(nTarget, nID)    while(0)

#endif
#endif
#endif
