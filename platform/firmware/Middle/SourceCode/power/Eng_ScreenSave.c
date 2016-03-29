/**
 * @file 
 * @brief Screen saver function
 * 
 * @author ZouMai
 * @date 2002-10-21
 * @version 1.0
 */

#include "Eng_ScreenSave.h"
#include "Fwl_pfDisplay.h"
#include "Lib_event.h"
#include "Fwl_Initialize.h"
#include "Fwl_public.h"
#include "Eng_AutoPowerOff.h"
#include "fwl_sysevent.h"

#define SCREENSAVER_DISABLE_ID	0xFFFF
#define SCREENSAVER_ON_ID		0xFFFE


/*
    发屏保事件的条件之一为gb_ScSaverCount==0，因此初值不能为0，
    否则可能在重设gb_ScSaverCount前就发了M_EVT_Z06COM_SCREEN_SAVER .
*/
static volatile T_U32           gb_ScSaverCount = 60;
static T_BOOL                   gb_ScSaverFlag = AK_FALSE;
static T_U16                        nCalledCount = 0;
 

/**
 * @brief Reset screen saver and background light count down
 * 
 * @author @b ZouMai
 * @date 2002-10-31
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID UserCountDownReset(T_VOID)
{
    ScreenSaverCountSet(gs.ScSaverTM);
    AutoPowerOffCountSet(gs.PoffTM*60);
    if (gb.KeyLocked==AK_FALSE)
        SetKeyLightCount(gs.KeyLightTM);

    return;
}

/**
 * @brief Set screen count down
 * 
 * @author ZouMai
 * @date 2002-10-31
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_VOID ScreenSaverCountSet(T_U32 count)
{
    if (gb_ScSaverCount == SCREENSAVER_DISABLE_ID)     /* disable */
        return;

    gb_ScSaverCount = count;
    gb_ScSaverFlag = AK_FALSE;

    return;
}

/**
 * @brief Decrease screen saver count
 * 
 * @author ZouMai
 * @date 2002-10-31
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL ScreenSaverCountDecrease(T_U32 millisecond)
{
    T_U32 second = 0;
	static T_U32 num = 0;

    second = millisecond / 1000;
    
    if (gb_ScSaverCount == SCREENSAVER_DISABLE_ID)     /* disable */
        return AK_FALSE;

    if (gs.ScSaverTM == 0)   /* always off */
        return AK_FALSE;


    if (gb_ScSaverCount > second)
        gb_ScSaverCount -= second;
    else
        gb_ScSaverCount = 0;
        
	if (++num >= PRINT_TIMES_SEC)
	{
    	Fwl_Print(C3, M_PUBLIC, "second = %d, count = %d, SaverFlag = %d", second, gb_ScSaverCount, gb_ScSaverFlag);
		num = 0;
	}
	
    if (!gb_ScSaverFlag && gb_ScSaverCount == 0)    /* Start screen saver */
    {
        /* call function to show screen saver */
        VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_Z06COM_SCREEN_SAVER, (T_U32)AK_NULL);
        gb_ScSaverFlag = AK_TRUE;
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_VOID ScreenSaverDisable(T_VOID)
{
    gb_ScSaverCount = SCREENSAVER_DISABLE_ID;
    nCalledCount++;
}

T_VOID ScreenSaverEnable(T_VOID)
{
	if (nCalledCount>0)
	{
        nCalledCount--;
	}
	else
	{
		return;
	}
	
	if (nCalledCount == 0)
        gb_ScSaverCount = gs.ScSaverTM;

}

/**
 * @brief Judge screen saver is ture on or not
 * 
 * @author ZouMai
 * @date 2002-10-31
 * @param T_VOID
 * @return T_VOID
 * @retval 
 */
T_BOOL ScreenSaverIsOn(T_VOID)
{
    if (gb_ScSaverCount == SCREENSAVER_DISABLE_ID)
        return AK_FALSE;

    return AK_TRUE;
}


