
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_ScreenSave.h"
#include "Eng_DynamicFont.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_POFF_PARM;

static T_POFF_PARM *pPoffParm;
#endif
/*---------------------- BEGIN OF STATE s_set_poff_time ------------------------*/
void initset_poff_time(void)
{
#ifdef SUPPORT_SYS_SET
    pPoffParm = (T_POFF_PARM *)Fwl_Malloc(sizeof(T_POFF_PARM));
    AK_ASSERT_PTR_VOID(pPoffParm, "initstdb_standby(): malloc error");

    MenuStructInit(&pPoffParm->IconExplorer);
    GetMenuStructContent(&pPoffParm->IconExplorer, mnPOFF_TIME);
    IconExplorer_SetFocus(&pPoffParm->IconExplorer, gs.PoffTM);
#endif
}

void exitset_poff_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pPoffParm->IconExplorer);
    pPoffParm = Fwl_Free(pPoffParm);
#endif
}

void paintset_poff_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pPoffParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_poff_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   IconExplorerRet; 
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pPoffParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pPoffParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pPoffParm->IconExplorer);
        if (focusID != gs.PoffTM)
        {
            gs.PoffTM = focusID;
            UserCountDownReset();
        }
#if 0
        MsgBox_InitAfx(&pPoffParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pPoffParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPoffParm->msgbox); 
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
