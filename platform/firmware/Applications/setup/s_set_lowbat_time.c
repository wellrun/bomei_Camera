
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Eng_BatWarn.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_LOWBAT_PARM;

static T_LOWBAT_PARM *pLowBatParm;
#endif
/*---------------------- BEGIN OF STATE s_set_lowbat_time ------------------------*/
void initset_lowbat_time(void)
{
#ifdef SUPPORT_SYS_SET

    pLowBatParm = (T_LOWBAT_PARM *)Fwl_Malloc(sizeof(T_LOWBAT_PARM));
    AK_ASSERT_PTR_VOID(pLowBatParm, "initstdb_standby(): malloc error");

    MenuStructInit(&pLowBatParm->IconExplorer);
    GetMenuStructContent(&pLowBatParm->IconExplorer, mnLOWBAT_TIME);
    IconExplorer_SetFocus(&pLowBatParm->IconExplorer, gs.LowBatTM);
#endif
}

void exitset_lowbat_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pLowBatParm->IconExplorer);
    pLowBatParm = Fwl_Free(pLowBatParm);
#endif
}

void paintset_lowbat_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pLowBatParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_lowbat_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   IconExplorerRet;
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pLowBatParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pLowBatParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pLowBatParm->IconExplorer);
        if (focusID != gs.LowBatTM)
        {
            gs.LowBatTM = focusID;
            if (AK_TRUE == BatWarnIsEnable())
                BatWarnCountSet(gs.LowBatTM * 60);
        }
#if 0
        MsgBox_InitAfx(&pLowBatParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pLowBatParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pLowBatParm->msgbox);
#endif
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

