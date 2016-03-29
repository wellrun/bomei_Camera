
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_ScreenSave.h"
#include "Eng_DynamicFont.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_SS_PARM;

static T_SS_PARM *pSsParm;
#endif
/*---------------------- BEGIN OF STATE s_set_disp_ss_time ------------------------*/
void initset_disp_ss_time(void)
{
#ifdef SUPPORT_SYS_SET
    pSsParm = (T_SS_PARM *)Fwl_Malloc(sizeof(T_SS_PARM));
    AK_ASSERT_PTR_VOID(pSsParm, "initstdb_standby(): malloc error");

    MenuStructInit(&pSsParm->IconExplorer);
    GetMenuStructContent(&pSsParm->IconExplorer, mnSS_TIME);
    IconExplorer_SetFocus(&pSsParm->IconExplorer, gs.ScSaverTM);
#endif
}

void exitset_disp_ss_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pSsParm->IconExplorer);
    pSsParm = Fwl_Free(pSsParm);
#endif
}

void paintset_disp_ss_time(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pSsParm->IconExplorer);
    
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_ss_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   IconExplorerRet;
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pSsParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pSsParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pSsParm->IconExplorer);
        if (focusID != gs.ScSaverTM)
        {
            gs.ScSaverTM = focusID;
            gs.KeyLightTM = focusID;
            UserCountDownReset();
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
