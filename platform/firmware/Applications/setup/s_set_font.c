/************************************************************************
 * Copyright (c) 2011, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£∫s_set_font.c

 * Function£∫font set
 *
 * Author£∫deng zhou
 * Date£∫2011-01-25
 * Version£∫1.0		  
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "eng_font.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_FONTSET_PARM;

static T_FONTSET_PARM *pFontSetParm;
#endif
/*---------------------- BEGIN OF STATE s_set_lowbat_time ------------------------*/
void initset_font(void)
{
#ifdef SUPPORT_SYS_SET

	T_U16 focus;
    pFontSetParm = (T_FONTSET_PARM *)Fwl_Malloc(sizeof(T_FONTSET_PARM));
    AK_ASSERT_PTR_VOID(pFontSetParm, "initstdb_standby(): malloc error");

    MenuStructInit(&pFontSetParm->IconExplorer);
    GetMenuStructContent(&pFontSetParm->IconExplorer, mnFONT_SIZE_SET_MENU);
	switch (gs.FontSize)
	{
		case FONT_12:
			focus = 10;
			break;
		case FONT_16:
			focus = 20;
			break;
		default:
			focus = 10;
			break;
	}
    IconExplorer_SetFocus(&pFontSetParm->IconExplorer, focus);
#endif
}

void exitset_font(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pFontSetParm->IconExplorer);
    pFontSetParm = Fwl_Free(pFontSetParm);
#endif
}

void paintset_font(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pFontSetParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_font(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

	T_eBACK_STATE   IconExplorerRet;
	T_U16     focusID;
	T_U8	fontSize;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pFontSetParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pFontSetParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pFontSetParm->IconExplorer);
        
		switch (focusID)
		{
			case 10:
				fontSize = FONT_12;
				break;
			case 20:
				fontSize = FONT_16;
				break;
			default:
				fontSize = FONT_16;
				break;
		}
	
		if (fontSize != gs.FontSize)
        {
			gs.FontSize = fontSize;

			DynamicFont_Codepage_Free();
			DynamicFont_FontLib_Free();
			FontResize(gs.FontSize);
			if (!Eng_FontLib_Init())
			{
				Eng_FontLib_ChkOpenBackFile();	//¥”±∏∑›≈Ãª÷∏¥(NandBoot/SDBoot)
				Eng_FontLib_Init();				//init again
			}
		    Eng_Codepage_Init();
        }
        
        GE_ShadeInit();
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

