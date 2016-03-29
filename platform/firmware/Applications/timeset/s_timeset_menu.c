
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_TIMESET_PARM;

static T_TIMESET_PARM *pTimeSetParm;
#endif
/*---------------------- BEGIN OF STATE s_timeset_menu ------------------------*/
void inittimeset_menu(void)
{
#ifdef SUPPORT_SYS_SET
    pTimeSetParm = (T_TIMESET_PARM *)Fwl_Malloc(sizeof(T_TIMESET_PARM));
    AK_ASSERT_PTR_VOID(pTimeSetParm, "inittimeset_menu(): malloc error");

    MenuStructInit(&pTimeSetParm->IconExplorer);
    GetMenuStructContent(&pTimeSetParm->IconExplorer, mnTIMESET_MENU);
#endif
}

void exittimeset_menu(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pTimeSetParm->IconExplorer);
    pTimeSetParm = Fwl_Free(pTimeSetParm);
#endif
}

void painttimeset_menu(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pTimeSetParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handletimeset_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pTimeSetParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pTimeSetParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pTimeSetParm->IconExplorer))
        {
        case 10: // Calendar
            GE_ShadeInit();
            m_triggerEvent(M_EVT_1, pEventParm);
            break;
        case 20: // System Time
            GE_ShadeInit();
            m_triggerEvent(M_EVT_2, pEventParm);
            break;
        case 30: // World Map
            GE_ShadeInit();
            m_triggerEvent(M_EVT_3, pEventParm);
            break;
        case 40: // Alarm Setup
            GE_ShadeInit();
            m_triggerEvent(M_EVT_4, pEventParm);
            break;
        default:
            break;
        }
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
