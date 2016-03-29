/**@file Arch_init.h
 * @brief list driver library initialize operations
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2008-02-1
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */
#ifndef __ARCH_INIT_H__
#define __ARCH_INIT_H__

#include "anyka_types.h"

/** @defgroup Init Init group
 *  @ingroup Drv_Lib
 */
/*@{*/

/*
 * Used for initialization calls..
 */
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);


#ifdef __GNUC__
#define __initcall(fn) \
initcall_t __initcall_##fn \
__attribute__((__section__(".initcall"))) = fn;
#endif

#ifdef __CC_ARM
#define __initcall(fn) \
initcall_t __initcall_##fn  = fn;
#endif

/**
 *  module init
 */
#define module_init(x)  __initcall(x)

/**
 *  memory alloc callback handler
 */
typedef T_pVOID (*T_RamAlloc)(T_U32 size, T_S8 *filename, T_U32 fileline);

/**
 *  memory free callback handler
 */
typedef T_pVOID (*T_RamFree)(T_pVOID var, T_S8 *filename, T_U32 fileline);

/** @brief chip name
 * define all chip supported
 */
typedef enum
{
    CHIP_8801 = 0x8801,    ///< AK8801, Aspen3 series
    CHIP_8802 = 0x8802,        ///< AK8802, Aspen3 series

    CHIP_9801 = 0x9801,        ///< AK9801, Aspen3s series
    CHIP_9802 = 0x9802,        ///< AK9802, Aspen3s series
    CHIP_9805 = 0x9805,        ///< AK9805, Aspen3s series

    CHIP_3771 = 0x3771,        ///<AK3771, Sundance3 series
    CHIP_3751 = 0x3751,        ///<AK3751, Sundance3 series
    CHIP_3751B = 0x3752,        ///<AK3751B, Sundance3 series
    CHIP_3760 = 0x3760,        ///<AK3760, Sundance3 series
    CHIP_3750 = 0x3750,        ///<AK3750, Sundance3 series
    CHIP_3753 = 0x3753,        ///<AK3753, Sundance3 series

    CHIP_RESERVE = 0xffff       ///< reserve
}
E_AKCHIP_TYPE;

typedef enum
{
    CHIP_3771_02SH,
    CHIP_3771_04SH,
    CHIP_3771_L,
    CHIP_3771_UNKNOWN
}
E_AKCHIP_INFO;

/** @brief driver init info
 * define chip type and memory callback 
 */
typedef struct tag_DRIVE_INITINFO
{
    E_AKCHIP_TYPE chip;     ///< chip type
    
    T_RamAlloc fRamAlloc;   ///< memory alloc callback function
    T_RamFree  fRamFree;     ///<memory free callback function
}
T_DRIVE_INITINFO, *T_PDRIVE_INITINFO;

/**
 * @brief driver library initialization
 *
 * should be called on start-up step, to initial interrupt module and register hardware as camera, lcd...etc.
 * @author xuchang
 * @date 2008-01-21
 * @return void
 */
void drv_init(T_PDRIVE_INITINFO drv_info);

/**
 * @brief memory alloc
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param size T_U32: size of memory to alloc
 * @return void *
 */
T_pVOID drv_malloc(T_U32 size);

/**
 * @brief drv_get_chip
 *
 * @author LHD
 * @date 2012-03-06
 * @return E_AKCHIP_INFO
 */
E_AKCHIP_INFO drv_get_chip_version(T_VOID);

/**
 * @brief memory free
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param var T_pVOID: address of memory to free
 * @return void *
 */
T_pVOID drv_free(T_pVOID var);

/**
 * @brief get chip type
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @return T_VOID
 */
E_AKCHIP_TYPE drv_get_chip_type(T_VOID);

/**
 * @brief check current chip is the same series or not
 * @author xuchang
 * @date 2010-12-14
 * @param[in] chip_type chip type
 * @return T_BOOL
 * @retval if same series, return AK_TRUE
 */
T_BOOL drv_check_series_chip(E_AKCHIP_TYPE chip_type);
 
/**
 * @brief get dram capacity
 *
 * @author liao_zhijun
 * @date 2010-11-08
 * @return T_U32 ram size, uint (byte)
 */
T_U32 drv_get_ram_size(T_VOID);


/*@}*/
#endif //__ARCH_INIT_H__

