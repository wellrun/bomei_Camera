/**
 * @file sys_ctl.c
 * @brief system function file, provide functions to control system work mode
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */

#include "akdefine.h"
#include "gbl_global.h"
#include "gpio.h"
#ifdef WM_SKYWORKS
#include "fwl_oscom.h"
#include "fwl_evtmailbox.h"
#endif

#ifdef SUPPORT_PANNIC_REBOOT
#include "Apl_Initialize.h"
#endif // end of #ifdef SUPPORT_PANNIC_REBOOT


/**
 * @BRIEF system reset
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_reset(T_VOID)
{
#if ((defined(NANDBOOT)) || (defined(SDIOBOOT)) || (defined(SDMMCBOOT)))
    T_U32 i, j;
    void (*f)(void) = 0;
    MMU_DisableDCache();
    MMU_DisableICache();
    MMU_DisableAlignFault();
    MMU_DisableWriteBuffer();
    
    //If write-back is used,the DCache should be cleared.
    for (i=0;i<64;i++)
        for (j=0;j<8;j++)
            MMU_CleanInvalidateDCacheIndex((i<<26)|(j<<5));
    MMU_InvalidateICache();

    MMU_DisableMMU();
    MMU_InvalidateTLB();

    mini_delay(100);
    store_all_int();
    gpio_int_disableall();
    vtimer_free();//vtimer_free();//timer_reset_reg
#else
    void (*f)(void) = 0;
    (void*)f = (void*)0x10010000;
#endif
    f();
}

/**
 * @BRIEF module reset detect handler
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] T_U32 pin: which pin use detect module reset
 * @PARAM[in] T_U8 polarity: polarity of pin
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID sys_module_reset_detect_handler(T_U32 pin, T_U8 polarity)
{
#ifdef WM_SKYWORKS
    if(1 == polarity)
    {
        mini_delay(20);
        if (gpio_get_pin_level(pin) != 1)
        {
            Fwl_Print(C2, M_DRVSYS, "drop a unuseful high reset signal ......\r\n");            
            return;
        }    
        Fwl_Print(C3, M_DRVSYS, "get reset signal......\r\n");
        gpio_set_int_p(pin, 0);        
        
        gb.bResetComplete = AK_FALSE;
        VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_Z99COM_MODULE_RESET, GetResetParam());
    }
    else
    {
        mini_delay(20);
        if (gpio_get_pin_level( pin ) != 0)
        {
            Fwl_Print(C2, M_DRVSYS, "drop a unuseful low reset signal......\r\n");            
            return;
        }    
        gpio_set_int_p(pin, 1);        
        Fwl_Print(C3, M_DRVSYS, "reset signal end......\r\n");
    }
#endif
}

/**
 * @BRIEF system init reset module
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_init_resetmodule(T_VOID)
{
#ifdef WM_SKYWORKS
    Fwl_Print(C3, M_DRVSYS, "register module reset handler\r\n");
    gpio_set_pin_dir(GPIO_MODULE_RESET_DETECT, 1);
    gpio_set_pin_level(GPIO_MODULE_RESET_DETECT, 0);   // init reset detect GPIO
    gpio_set_pin_dir(GPIO_MODULE_RESET_DETECT, 0);
    
    gpio_register_int_callback(GPIO_MODULE_RESET_DETECT, 1, 1, sys_module_reset_detect_handler);// GPIO 4 is connect to skyworks reset detect 
#endif    
}

/**
 * @BRIEF system usb pullup register
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_usb_pullup_R(T_VOID)
{
#ifdef CHIP_AK3223
#ifdef USE_GPIO_USB_PULL_UP
    gpio_set_pin_dir(GPIO_USB_PULL_UP, 1);
    gpio_set_pin_level(GPIO_USB_PULL_UP, 1);
#else
    *(volatile T_U32 *)0x20063000 |= 2;
#endif
#endif//CHIP_AK3223
}

/**
 * @BRIEF system usb pulldown register
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_usb_pulldown_R(T_VOID)
{
#ifdef CHIP_AK3223
#ifdef USE_GPIO_USB_PULL_UP
    gpio_set_pin_dir(GPIO_USB_PULL_UP, 1);
    gpio_set_pin_level(GPIO_USB_PULL_UP, 0);
#else
    *(volatile T_U32 *)0x20063000 &= 0xfffffffc;
#endif
#endif//CHIP_AK3223
}


/**
 * @BRIEF system initialize
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] T_U32 sysflag: system initialize data
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_initialize(T_U32 sysflag)
{
    gpio_pin_init();
    sys_usb_pulldown_R();
#ifdef SUPPORT_PANNIC_REBOOT
    if (SYS_PANNIC_RESET != SYS_STATE_FLAG)
#endif    // end of #ifdef SUPPORT_PANNIC_REBOOT
    {
        sys_init_resetmodule();
    }
}
