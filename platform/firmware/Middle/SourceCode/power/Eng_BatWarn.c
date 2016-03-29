/**
 * @file
 * @brief battery warning function
 *
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @version 1.0
 */

#include "Eng_BatWarn.h"
#include "Fwl_pfDisplay.h"
#include "Lib_event.h"
#include "Fwl_Initialize.h"
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Eng_MsgQueue.h"
#include "Fwl_sysevent.h"
#include "eng_debug.h"
#include "fwl_power.h"
#ifdef USB_HOST	
#include "fwl_usb_host.h"
#endif

static T_U32                    gb_BatWarnCount = 0xffffffff;
static T_BOOL                       gb_BatWarnEnable = AK_FALSE;

/**
 * @brief Set screen count down
 *
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID BatWarnCountSet(T_U32 count)
{
    gb_BatWarnCount = count;
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
T_BOOL BatWarnCountDecrease(T_U32 millisecond)
{
    T_U32 second = millisecond / 1000;
    
#ifdef OS_ANYKA
	if (!gb_BatWarnEnable 
		|| Fwl_UseExternCharge())
	{
		return AK_FALSE;
    }
#endif // 

    if (gs.LowBatTM == 0)   /* always off */
        return AK_FALSE;

    if (gb_BatWarnCount > second)
        gb_BatWarnCount -= second;
    else
        gb_BatWarnCount = 0;

       Fwl_Print(C3, M_POWER,"\ndebug token: BatWarnCountDecrease(): gb_BatWarnCount = %d\n", gb_BatWarnCount);
    if (gb_BatWarnCount == 0)   /* Start auto power off */
    {
    	  /**exit screen saver*/
          VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_NULL);
          
        /* call function to show auto power off */
        if (gb.PubMsgAllow)
        {
            MsgQu_Push(GetCustomTitle(ctWARNING), GetCustomString(csLOW_BATTERY), MSGBOX_INFORMATION, MSGBOX_DELAY_1);
            if (!gb.InPublicMessage)
            {
                VME_ReTriggerUniqueEvent((vT_EvtSubCode)M_EVT_Z05COM_MSG, (T_U32)AK_NULL);
            }
        }
        
        BatWarnCountSet(gs.LowBatTM*60);
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_VOID BatWarnDisable(T_VOID)
{
    gb_BatWarnEnable = AK_FALSE;
}

T_VOID BatWarnEnable(T_VOID)
{
    gb_BatWarnEnable = AK_TRUE;
}

#if 0
T_BOOL BatWarnOn(T_VOID)
{
    if (gb_BatWarnCount == BATWARN_DISABLE_ID || (Fwl_UseExternCharge() == AK_TRUE))     /* disable */
        return AK_FALSE;

    gb_BatWarnCount = BATWARN_ON_ID;
    return AK_FALSE;
}
#endif

/**
 * @brief Judge auto power off is ture on or not
 *
 * @author Guanghua Zhang
 * @date 2005-03-16
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_BOOL BatWarnIsEnable(T_VOID)
{
    return gb_BatWarnEnable;
}

/**
 * @brief Get battery warning count
 *
 * @author zhengwenbo
 * @date 2006-10-12
 * @param T_VOID
 * @return T_U32
 * @retval
 */
T_U32 BatWarnGetCount(T_VOID)
{
    return gb_BatWarnCount;
}


