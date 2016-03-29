/**
 * @file s_watchdog_test.c
 * @brief ANYKA software
 * hardware watchdog test
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

 

#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Eng_KeyMapping.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "eng_font.h"
#include "eng_screensave.h"
#include "eng_string_uc.h"
#include "eng_akbmp.h"
#include "Fwl_oscom.h"
#include "Fwl_display.h"


#define WATCHDOG_FEED_TIMER				10000
#define WATCHDOG_AUTO_DEFAULT_INTVL		8
#define WATCHDOG_AUTO_MAX_INTVL			15
#define WATCHDOG_AUTO_MIN_INTVL			0

#define WATCHDOG_MAIN_STR_TOP			50
#define WATCHDOG_WARNING_STR_TOP		80
#define WATCHDOG_MODE_STR_TOP			130
#define WATCHDOG_MODEKEY_STR_TOP		150
#define WATCHDOG_OTHERKEY_STR_TOP		190




typedef struct {
	T_BOOL		bAutoMode;
	T_BOOL		bRefresh;
	T_WSTR_20	mainstr;
	T_pCDATA    pBkImg;      
	T_TIMER		timer;
	T_U8		interval;
} T_WATCHDOG_TEST_PARM;

static T_WATCHDOG_TEST_PARM *pWatchdog_test = AK_NULL;


static T_VOID WatchDog_Test_Start_Timer(T_VOID)
{
	T_U32 interval = 0;

	if (AK_NULL == pWatchdog_test)
	{
		return;
	}
	
	interval = pWatchdog_test->interval * 1000;

	if (ERROR_TIMER != pWatchdog_test->timer)
	{
		Fwl_StopTimer(pWatchdog_test->timer);
		pWatchdog_test->timer = ERROR_TIMER;
	}
	
	pWatchdog_test->timer = Fwl_SetTimerMilliSecond(interval, AK_TRUE);
}

static T_VOID WatchDog_Test_Stop_Timer(T_VOID)
{
	if (AK_NULL == pWatchdog_test)
	{
		return;
	}
	
	if (ERROR_TIMER != pWatchdog_test->timer)
	{
		Fwl_StopTimer(pWatchdog_test->timer);
		pWatchdog_test->timer = ERROR_TIMER;
	}
}



static T_VOID WatchDog_Test_ShowStr(T_VOID)
{
	T_U32 width = 0;
	T_POS left = 0;
	T_WSTR_50 display_str = {0};
	T_WSTR_20 interval_str = {0};


	if (!pWatchdog_test->bRefresh)
	{
		return;
	}

	Fwl_AkBmpDrawFromString(HRGB_LAYER, 0, TOP_BAR_HEIGHT, \
                    pWatchdog_test->pBkImg, &g_Graph.TransColor, AK_FALSE);

	width = UGetSpeciStringWidth(pWatchdog_test->mainstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWatchdog_test->mainstr));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, WATCHDOG_MAIN_STR_TOP, pWatchdog_test->mainstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pWatchdog_test->mainstr));


	Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_WATCHDOG_WARNING), 50);
	width = UGetSpeciStringWidth(display_str, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, WATCHDOG_WARNING_STR_TOP, display_str,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));

	memset(display_str, 0, 50);

	if (pWatchdog_test->bAutoMode)
	{
		Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_AUTO_MODE), 50);
		Utl_UItoa(pWatchdog_test->interval, interval_str, 10);
		Utl_UStrCat(display_str, interval_str);
		
		if (pWatchdog_test->interval <= 1)
		{
			Utl_UStrCat(display_str, Res_GetStringByID(eRES_STR_SECOND));
		}
		else
		{
			Utl_UStrCat(display_str, Res_GetStringByID(eRES_STR_SECONDS));
		}
	}
	else
	{
		Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_MANUAL_MODE), 50);
	}
	
	width = UGetSpeciStringWidth(display_str, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, WATCHDOG_MODE_STR_TOP, display_str,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));

	memset(display_str, 0, 50);
	Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_MODE_KEY), 50);
	width = UGetSpeciStringWidth(display_str, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, WATCHDOG_MODEKEY_STR_TOP, display_str,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));

	memset(display_str, 0, 50);

	if (pWatchdog_test->bAutoMode)
	{
		Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_INTVL_KEY), 50);
	}
	else
	{
		Utl_UStrCpyN(display_str, Res_GetStringByID(eRES_STR_FEED_KEY), 50);
	}
	
	width = UGetSpeciStringWidth(display_str, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, WATCHDOG_OTHERKEY_STR_TOP, display_str,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(display_str));

	pWatchdog_test->bRefresh = AK_FALSE;
}




/*---------------------- BEGIN OF STATE s_watchdog_test ------------------------*/


#endif


void initwatchdog_test(void)
{
#ifdef SUPPORT_SYS_SET

    pWatchdog_test = (T_WATCHDOG_TEST_PARM *)Fwl_Malloc(sizeof(T_WATCHDOG_TEST_PARM));
    AK_ASSERT_PTR_VOID(pWatchdog_test, "initwatchdog_test(): malloc error");
	memset(pWatchdog_test, 0, sizeof(T_WATCHDOG_TEST_PARM));

	pWatchdog_test->bRefresh = AK_TRUE;
	pWatchdog_test->timer = ERROR_TIMER;
	pWatchdog_test->interval = WATCHDOG_AUTO_DEFAULT_INTVL;

	Utl_UStrCpyN(pWatchdog_test->mainstr, Res_GetStringByID(eRES_STR_WATCHDOG_TEST), 20);
	pWatchdog_test->pBkImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MAIN_BACKGROUND, AK_NULL);
	TopBar_SetTitle(pWatchdog_test->mainstr);
	TopBar_Show(TB_REFRESH_ALL);

	Fwl_Watchdog_Timer_Start(WATCHDOG_FEED_TIMER);
	ScreenSaverDisable();
#endif
}

void exitwatchdog_test(void)
{
#ifdef SUPPORT_SYS_SET

	WatchDog_Test_Stop_Timer();
	
    pWatchdog_test = Fwl_Free(pWatchdog_test);
	Fwl_Watchdog_Timer_Stop();
	ScreenSaverEnable();
#endif
}

void paintwatchdog_test(void)
{
#ifdef SUPPORT_SYS_SET

	WatchDog_Test_ShowStr();

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlewatchdog_test(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

	T_MMI_KEYPAD    phyKey;
	

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

	if (VME_EVT_TIMER == event)
	{
		if (pEventParm->w.Param1 == (T_U32)pWatchdog_test->timer)
		{
			Fwl_Watchdog_Timer_Feed();
			AK_DEBUG_OUTPUT("auto feed\n");
		}
	}
	else if (M_EVT_USER_KEY == event)
	{
		phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

		switch(phyKey.keyID)
		{
		case kbCLEAR:
			if (PRESS_SHORT == phyKey.pressType)
			{
				m_triggerEvent(M_EVT_EXIT, pEventParm);
			}
			break;
		case kbOK:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (!pWatchdog_test->bAutoMode)
				{
					Fwl_Watchdog_Timer_Feed();
					AK_DEBUG_OUTPUT("manual feed\n");
				}
			}
			break;
		case kbLEFT:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (pWatchdog_test->bAutoMode)
				{
					pWatchdog_test->bAutoMode = AK_FALSE;
					WatchDog_Test_Stop_Timer();
				}
				else
				{
					pWatchdog_test->bAutoMode = AK_TRUE;
					WatchDog_Test_Start_Timer();
				}
				
				pWatchdog_test->bRefresh = AK_TRUE;
				Fwl_Watchdog_Timer_Feed();
			}
			break;
		case kbUP:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (pWatchdog_test->bAutoMode)
				{
					if (pWatchdog_test->interval < WATCHDOG_AUTO_MAX_INTVL)
					{
						pWatchdog_test->interval++;
						pWatchdog_test->bRefresh = AK_TRUE;

						WatchDog_Test_Stop_Timer();
						WatchDog_Test_Start_Timer();
						Fwl_Watchdog_Timer_Feed();
					}
				}
			}
			break;
		case kbDOWN:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (pWatchdog_test->bAutoMode)
				{
					if (pWatchdog_test->interval > WATCHDOG_AUTO_MIN_INTVL)
					{
						pWatchdog_test->interval--;
						pWatchdog_test->bRefresh = AK_TRUE;

						WatchDog_Test_Stop_Timer();
						WatchDog_Test_Start_Timer();
						Fwl_Watchdog_Timer_Feed();
					}
				}
			}
			break;
		default:
			break;
		}
		
	}
	

#endif

    return 0;
}
