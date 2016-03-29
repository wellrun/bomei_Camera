/**
 * @FILENAME: init.c
 * @BRIEF init module
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR xuchang
 * @DATE 2007.10.23
 * @VERSION 1.0
 * @REF
 */
#include "arch_init.h"
#include "l2.h"
#include "interrupt.h"
#include "drv_api.h"

#ifdef __GNUC__
extern int __initcall_start, __initcall_end;
static int *initcall_start=&__initcall_start, *initcall_end=&__initcall_end;
#endif
#ifdef __CC_ARM
extern int Image$$init$$Base, Image$$init$$Limit;
static int *initcall_start=&Image$$init$$Base, *initcall_end=&Image$$init$$Limit;
#endif

#define CHIP_SE 0x08000174
#define CHIP_ID 0x08000000

static T_DRIVE_INITINFO g_drv_info = {0};

void do_initcalls(void)
{
    initcall_t *call;
    int *addr;

    akprintf(C3, M_DRVSYS, "initcall(): start=0x%x, end=0x%x\n", initcall_start, initcall_end); 
    for (addr = initcall_start; addr < initcall_end; addr++) 
    {
        call = (initcall_t *)addr;
        (*call)();    
    }
}

void drv_init(T_PDRIVE_INITINFO drv_info)
{
    T_U32 reg_value;
    
    memcpy(&g_drv_info, drv_info, sizeof(g_drv_info));
    
    //release reset all module
    REG32(RESET_CTRL_REG) &= 0x00000000;

    /*L2 init*/
    l2_init();

    /* share pin init */
	gpio_share_pin_init();

    /* interrupt init */
    interrupt_init();
    /* device module registeration init */
    do_initcalls();
    /* set default arm dma priority, arm can not break dma */
    dma_init();

    //close voice wakeup
    REG32(ANALOG_CTRL_REG4) |= (1 << 26);      //power off
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 5);   //disable voice wakeup

    REG8(ANALOG_CTRL_REG1) |= (1 << 8);      //VREF pin output  high impedance

    DrvModule_Init();
}

/**
 * @brief memory alloc
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param size T_U32: size of memory to alloc
 * @return void *
 */
T_pVOID drv_malloc(T_U32 size)
{
    if(AK_NULL == g_drv_info.fRamAlloc)
        return AK_NULL;

    return g_drv_info.fRamAlloc((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

/**
 * @brief memory free
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @param var T_pVOID: address of memory to free
 * @return void *
 */
T_pVOID drv_free(T_pVOID var) 
{
    if(AK_NULL == g_drv_info.fRamFree)
        return AK_NULL;

    return g_drv_info.fRamFree(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}  

/**
 * @brief get chip type
 *
 * @author liao_zhijun
 * @date 2010-07-15
 * @return T_VOID
 */
E_AKCHIP_TYPE drv_get_chip_type()
{
    return g_drv_info.chip;
}

/**
 * @brief check current chip is the same series or not
 * @autor xuchang
 * @date 2010-12-14
 * @param[in] chip type
 * @return T_BOOL
 * @retval if same series, return AK_TRUE
 */
T_BOOL drv_check_series_chip(E_AKCHIP_TYPE chip_type)
{
    if ((chip_type>>8) == (g_drv_info.chip>>8))
        return AK_TRUE;
    else
        return AK_FALSE;
}

/**
 * @brief get dram type
 *
 * @author liao_zhijun
 * @date 2010-11-08
 * @return T_U32 ram size, uint (byte)
 */
T_U32 drv_get_ram_size()
{
    T_U32 reg, size;

    reg = REG32(SDRAM_CFG_REG2);

    size = (1 << ((reg & 0x7) + 1));

    return (size << 20);
}

/**
 * @brief drv_get_chip
 *
 * @author LHD
 * @date 2012-03-06
 * @return E_AKCHIP_INFO
 */
E_AKCHIP_INFO drv_get_chip_version()
{
    T_U32 reg,temp;
    E_AKCHIP_INFO type = CHIP_3771_UNKNOWN;

    reg = REG32(CHIP_ID);
    if(0x20130100 == reg)
    {
        return CHIP_3771_L;
    }
    else
    {
        reg = ReadRaml(CHIP_SE);

        temp = (reg>>24)&0xff;
        if (0x3e == temp)
             type = CHIP_3771_04SH;
        else if (0x00 == temp)    
            type =CHIP_3771_02SH;
        else
            type =CHIP_3771_UNKNOWN;
    }
    
    return type;
}


