/**
 * @file 
 * @brief auto power off function
 * 
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @version 1.0
 */

#include "Eng_AutoPowerOff.h"
#include "Fwl_pfDisplay.h"
#include "Lib_event.h"
#include "Fwl_Initialize.h"
#include "Fwl_public.h"
#include "Fwl_sysevent.h"
#include "fwl_power.h"
#include "misc.h"
#include "Lib_state_api.h"
#ifdef USB_HOST
#include "fwl_usb_host.h"
#endif

static volatile T_U32 gb_AutoOffCount;
static T_BOOL gb_AutoOffFlag = AK_FALSE;
static T_BOOL gb_AutoPwrOffEnable = AK_TRUE;
static T_U32 gb_AutoPwrOffDisFlag = 0;

/**
 * @brief Set screen count down
 * 
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID AutoPowerOffCountSet(T_U32 count)
{
    gb_AutoOffCount = count;
    
    if (gb_AutoPwrOffEnable)
    {   
        gb_AutoOffFlag = AK_FALSE;
    }
}

/**
 * @brief Decrease auto power off count
 * 
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL AutoPowerOffCountDecrease(T_U32 millisecond)
{
    T_U32 second = millisecond / 1000;
    
    if (!gb_AutoPwrOffEnable || (0 == gs.PoffTM))     /* disable */
    {   
        return AK_FALSE;
    }

    if (gb_AutoOffCount > second)
    {
        gb_AutoOffCount -= second;
    }
    else
    {
        gb_AutoOffCount = 0;
    }

#ifdef OS_ANYKA
    //Fwl_Print(C3, M_POWER,"debug token:AutoPowerOffCountDecrease():gb_AutoOffFlag = %d, gb_AutoOffCount = %d", gb_AutoOffFlag, gb_AutoOffCount);

/* Start auto power off */
	if (!gb_AutoOffFlag
		&& gb_AutoOffCount == 0
		&& !Fwl_UseExternCharge())
	{
        /* call function to show auto power off */
		Fwl_Print(C3, M_POWER,"Eng_AutoPowerOff.c:Auto PowerOff\n");
        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
		m_triggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
        gb_AutoOffFlag = AK_TRUE;

        Fwl_DisplayOn();
        open_keypadlight();
        Fwl_DisplayBacklightOn( AK_TRUE);
        return AK_TRUE;
    }
#endif // OS_ANYKA

    return AK_FALSE;
}

T_VOID AutoPowerOffDisable(T_AUTOPOWEROFF_FLAG flag)
{
    gb_AutoPwrOffEnable = AK_FALSE;
    gb_AutoPwrOffDisFlag |= flag;
}

T_VOID AutoPowerOffEnable(T_AUTOPOWEROFF_FLAG flag)
{
	if (gb_AutoPwrOffDisFlag & flag)
	{
        gb_AutoPwrOffDisFlag ^= flag;
	}
	else
	{
		return;
	}
	
	if (0 == gb_AutoPwrOffDisFlag)
	{
		gb_AutoPwrOffEnable = AK_TRUE;
		AutoPowerOffCountSet(gs.PoffTM*60);
	}
}

/**
 * @brief Judge auto power off is ture on or not
 * 
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL AutoPowerOffIsEnable(T_VOID)
{
    return gb_AutoPwrOffEnable;
}

/**
 * @brief Get gb_AutoOffCount
 * 
 * @author zhengwenbo
 * @date 2006-10-09
 * @param T_VOID
 * @return T_U32
 * @retval 
 */
 T_U32 AutoPowerOffGetCount(T_VOID)
{
    return gb_AutoOffCount;
}
