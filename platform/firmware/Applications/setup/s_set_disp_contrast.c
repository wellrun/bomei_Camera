
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Ctl_Dialog.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_DIALOG        dialog;
    T_MSGBOX        msgbox;
} T_LCD_CONTRAST_PARM;

static T_LCD_CONTRAST_PARM *pLcd_ContrastParm;
#endif
/*---------------------- BEGIN OF STATE s_set_disp_contrast ------------------------*/
void initset_disp_contrast(void)
{
#ifdef SUPPORT_SYS_SET

    pLcd_ContrastParm = (T_LCD_CONTRAST_PARM *)Fwl_Malloc(sizeof(T_LCD_CONTRAST_PARM));
    AK_ASSERT_PTR_VOID(pLcd_ContrastParm, "initset_disp_contrast(): malloc error");

    Dialog_Init(&pLcd_ContrastParm->dialog, gs.LcdContrast, 0, 20, 1);
    Dialog_SetTitle(&pLcd_ContrastParm->dialog, GetCustomTitle(ctLCDCONTRAST));
#endif
}

void exitset_disp_contrast(void)
{
#ifdef SUPPORT_SYS_SET

    Dialog_Free(&pLcd_ContrastParm->dialog);
    pLcd_ContrastParm = Fwl_Free(pLcd_ContrastParm);
#endif
}

void paintset_disp_contrast(void)
{
#ifdef SUPPORT_SYS_SET
    Dialog_Show(&pLcd_ContrastParm->dialog, g_Graph.TtlFrCL);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_contrast(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   dialogRet;
    T_S16           CurValue;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    dialogRet = Dialog_Handler(&pLcd_ContrastParm->dialog, event, pEventParm);

    switch (dialogRet)
    {
    case eNext:
        CurValue = Dialog_GetCurValue(&pLcd_ContrastParm->dialog);
        if (CurValue != gs.LcdContrast)
        {
            gs.LcdContrast = (T_U8)(CurValue);
        }
        MsgBox_InitAfx(&pLcd_ContrastParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pLcd_ContrastParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pLcd_ContrastParm->msgbox);
        break;
    default:
        ReturnDefauleProc(dialogRet, pEventParm);
        break;
    }
#endif
    return 0;
}
