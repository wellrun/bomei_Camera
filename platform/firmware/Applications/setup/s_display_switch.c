/**
 * @file: s_display_switch.c
 * @brief: 
 * 
 * @author: hoube
 * @date:  2012-3-15
 */

#include "fwl_public.h"

#ifdef SUPPORT_TVOUT

#include "Ctl_IconExplorer.h"
#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "gbl_macrodef.h"
#include "Fwl_osCom.h"
#include "Eng_ScreenSave.h"


typedef struct {
	T_ICONEXPLORER	iconExplorer;
	
} T_DISPLAY_SWITCH;

static T_DISPLAY_SWITCH *pDisplaySwitch = AK_NULL;

T_VOID DisplaySwitch(DISPLAY_TYPE_DEV dstDisplayType)
{
	if (dstDisplayType == Fwl_GetDispalyType())
	{
		return;
	}
	
	if(Fwl_GetDispalyType() < DISPLAY_TVOUT_PAL)//LCD-->TVOUT
	{
		Fwl_SetDisplayType(dstDisplayType);	
		Fwl_RefreshDisplay();
		Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
		
		ScreenSaverDisable();
	}
	else
	{
		if (dstDisplayType < DISPLAY_TVOUT_PAL)	//TVOUT-->LCD
		{
			Fwl_SetDisplayType(DISPLAY_LCD_0);				
			Fwl_RefreshDisplay();
			/* this delay resolve: top left corner and bottom right corner of display will appear jib-headed bright flash, 
									when switch to LCD mode from TVOUT mode that MPU panel */
			Fwl_MiniDelay(30);
			Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
			
			ScreenSaverEnable();
		}
		else									//PAL<-->NTSC
		{
            Fwl_SetDisplayType(dstDisplayType);
            Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
            Fwl_RefreshDisplay();
		}
	}
	
}

#endif

void initdisplay_switch(void)
{
#ifdef SUPPORT_TVOUT

    pDisplaySwitch = (T_DISPLAY_SWITCH *)Fwl_Malloc(sizeof(T_DISPLAY_SWITCH));
    AK_ASSERT_PTR_VOID(pDisplaySwitch, "pDisplaySwitch: malloc error");
	memset(pDisplaySwitch, 0, sizeof(T_DISPLAY_SWITCH));

    MenuStructInit(&pDisplaySwitch->iconExplorer);
    GetMenuStructContent(&pDisplaySwitch->iconExplorer, mnDISPLAY_SWITCH);
	
	IconExplorer_SetFocus(&pDisplaySwitch->iconExplorer, Fwl_GetDispalyType());

#endif
}

void exitdisplay_switch(void)
{
#ifdef SUPPORT_TVOUT

    IconExplorer_Free(&pDisplaySwitch->iconExplorer);

	Fwl_Free(pDisplaySwitch);
	pDisplaySwitch = AK_NULL;
	
#endif
}

void paintdisplay_switch(void)
{
#ifdef SUPPORT_TVOUT

    IconExplorer_Show(&pDisplaySwitch->iconExplorer);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handledisplay_switch(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_TVOUT

    T_eBACK_STATE   IconExplorerRet;
	
    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pDisplaySwitch->iconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pDisplaySwitch->iconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
   	case eNext:
   		DisplaySwitch(IconExplorer_GetItemFocusId(&pDisplaySwitch->iconExplorer));
		break;
		   
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }

#endif

	return 0;
}

