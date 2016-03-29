/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef   OS_WIN32
#include <stdlib.h>
#include <assert.h>

#include "vme_interface.h"
#include "drv_api.h"
#include "vme_engine.h"
#include "vme_event.h"
#include "Lib_state.h"

T_BOOL  vSleepModeConfig;

/*****************************************************************************
 * power interface
 *****************************************************************************
*/

T_BOOL VME_GetSleepModeConfig( T_VOID )
{

    return vSleepModeConfig;
}

T_VOID VME_SetSleepModeConfig(T_BOOL SleepModeConfig)
{
 

    vSleepModeConfig    = SleepModeConfig;
    return;
}

T_BOOL is_wakeup_by_keypad(T_U32 reason)
{
   return AK_FALSE;
}

T_BOOL is_wakeup_by_touchscreen(T_U32 reason)
{
   return AK_FALSE;
}

void change_amp_on_earphone_in()
{

}

void change_amp_on_earphone_out()
{

}

T_VOID open_call_amp()
{
}

T_VOID close_call_amp()
{
}

T_VOID open_back_light()
{

}

T_VOID recover_back_light()
{

}

T_VOID close_back_light()
{

}

T_VOID open_lcd()
{

}

T_VOID close_lcd()
{
}

T_VOID VME_SetPowerSavingMode()
{

}

T_VOID VME_ExitPowerSavingMode()
{

}

T_VOID switch_on_module()
{

}

T_VOID switch_off_module()
{

}


T_VOID open_keypadlight()
{

}

T_VOID close_keypadlight()
{

}

T_VOID mute_midi()
{
}

T_VOID open_midi()
{
}

T_BOOL enter_boot_mode()
{
    return AK_TRUE;
}

E_SYS_MODE sys_get_mode()
{
    return 0;
}
T_BOOL enter_idle_mode()
{
    return AK_TRUE;
}
T_BOOL enter_phone_mode()
{
    return AK_TRUE;
}

T_BOOL enter_full_mode()
{
    return AK_TRUE;
}
T_BOOL enter_sleep_mode()
{
    return AK_TRUE;
}

T_BOOL exit_sleep_mode()
{
    return AK_TRUE;    
}

T_VOID low_battery_process(T_U32 processFlag)
{

}

T_BOOL IsLowBatteryForbiddenSM(M_STATES curStateMachine)
{
    return AK_FALSE;
}

T_VOID Java_LowBattery_Process(T_VOID)
{

}

T_VOID MP3_LowBattery_Process(T_VOID)
{

}

T_BOOL adaptor_is_in(T_VOID)
{
    return AK_FALSE;
}

T_BOOL Fwl_UseExternCharge(T_VOID)
{
    return AK_FALSE;
}

T_U32 Fwl_GetBatteryVoltage(T_VOID)
{
    return BATTERY_VALUE_MAX;
}

T_BOOL Fwl_ChargeVoltageFull(T_VOID)
{
    return AK_FALSE;
}

T_VOID MonitorBatteryVoltage(T_U32 millisecond)
{

}
#endif