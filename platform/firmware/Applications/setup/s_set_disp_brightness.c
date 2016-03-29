
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Ctl_Dialog.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"

#include "Fwl_display.h"

typedef struct {
    T_DIALOG        dialog;
    T_MSGBOX        msgbox;
	T_U8			value;
} T_LCD_BRIGHTNESS_PARM;

static T_LCD_BRIGHTNESS_PARM *pLcd_BrightnessParm;
T_S16			CurValue;

#ifdef OS_WIN32
#ifndef LCD_BKL_BRIGHTNESS_MAX
#define LCD_BKL_BRIGHTNESS_MAX 7
#endif
#endif


/*---------------------- BEGIN OF STATE s_set_disp_brightness ------------------------*/

T_VOID resume_set_disp_brightness(T_VOID)
{ 			
	//if ( ! Fwl_TvoutIsOpen() )
    {
        Fwl_SetBrightness(DISPLAY_LCD_0, pLcd_BrightnessParm->value);
    }
}

T_VOID suspend_set_disp_brightness(T_VOID)
{    
	pLcd_BrightnessParm->value = (T_U8)Dialog_GetCurValue(&pLcd_BrightnessParm->dialog);
}
#endif
void initset_disp_brightness(void)
{
#ifdef SUPPORT_SYS_SET

    pLcd_BrightnessParm = (T_LCD_BRIGHTNESS_PARM *)Fwl_Malloc(sizeof(T_LCD_BRIGHTNESS_PARM));
    AK_ASSERT_PTR_VOID(pLcd_BrightnessParm, "initset_disp_brightness(): malloc error");

	pLcd_BrightnessParm->value = 0;
    Dialog_Init(&pLcd_BrightnessParm->dialog, gs.LcdBrightness, 1, LCD_BKL_BRIGHTNESS_MAX, 1);
    Dialog_SetTitle(&pLcd_BrightnessParm->dialog, GetCustomTitle(ctLCDBRIGHTNESS));

	m_regResumeFunc(resume_set_disp_brightness);
	m_regSuspendFunc(suspend_set_disp_brightness);

	CurValue = gs.LcdBrightness;
#endif
}

void exitset_disp_brightness(void)
{
#ifdef SUPPORT_SYS_SET
    Dialog_Free(&pLcd_BrightnessParm->dialog);
    pLcd_BrightnessParm = Fwl_Free(pLcd_BrightnessParm);
#endif
}

void paintset_disp_brightness(void)
{
#ifdef SUPPORT_SYS_SET

    Dialog_Show(&pLcd_BrightnessParm->dialog, g_Graph.TtlFrCL);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_brightness(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   dialogRet;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    dialogRet = Dialog_Handler(&pLcd_BrightnessParm->dialog, event, pEventParm);

	//if ( ! Fwl_TvoutIsOpen() )//SW10A00001637
    {
        Fwl_SetBrightness(DISPLAY_LCD_0, (T_U8)CurValue);
    }
	
    switch (dialogRet)
    {
    case eOption:
        CurValue = Dialog_GetCurValue(&pLcd_BrightnessParm->dialog);
			
		//if ( ! Fwl_TvoutIsOpen() )
        {
            Fwl_SetBrightness(DISPLAY_LCD_0, (T_U8)CurValue);
        }

        break;
    case eNext:
        CurValue = Dialog_GetCurValue(&pLcd_BrightnessParm->dialog);
        if (CurValue != gs.LcdBrightness)
        {
            gs.LcdBrightness = (T_U8)CurValue;

			//if ( ! Fwl_TvoutIsOpen() )
            {
                Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
            }
        }
        MsgBox_InitAfx(&pLcd_BrightnessParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pLcd_BrightnessParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pLcd_BrightnessParm->msgbox);
        break;
    case eReturn:   // cancel
        /*restore brightness*/     
		//if ( ! Fwl_TvoutIsOpen() )
        {
            Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
        }
        //no break;
    default:
        ReturnDefauleProc(dialogRet, pEventParm);
        break;
    }
#endif
    return 0;
}
