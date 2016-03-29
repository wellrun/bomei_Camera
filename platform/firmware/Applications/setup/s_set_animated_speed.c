#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Eng_BatWarn.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_ANIMATED_PARAM;

T_ANIMATED_PARAM *pAnimatedParam;
#endif
/*---------------------- BEGIN OF STATE s_set_lowbat_time ------------------------*/
void initset_animated_speed(void)
{
#ifdef SUPPORT_SYS_SET
    pAnimatedParam = (T_ANIMATED_PARAM *)Fwl_Malloc(sizeof(T_ANIMATED_PARAM));
    AK_ASSERT_PTR_VOID(pAnimatedParam, "initstdb_standby(): malloc error");

    MenuStructInit(&pAnimatedParam->IconExplorer);
    GetMenuStructContent(&pAnimatedParam->IconExplorer, mnANIMATED_CONFIG);
    IconExplorer_SetFocus(&pAnimatedParam->IconExplorer, gs.AniSwitchLevel);
#endif
}

void exitset_animated_speed(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pAnimatedParam->IconExplorer);
    pAnimatedParam = Fwl_Free(pAnimatedParam);
#endif
}

void paintset_animated_speed(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pAnimatedParam->IconExplorer);
    
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_animated_speed(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   IconExplorerRet;
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAnimatedParam->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAnimatedParam->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
        case eNext:
            focusID = (T_eAniMenuLevelType)IconExplorer_GetItemFocusId(&pAnimatedParam->IconExplorer);
            if (focusID != gs.AniSwitchLevel)
            {
                gs.AniSwitchLevel = focusID;

                if (gs.AniSwitchLevel == eClose)
                {
                    GE_ShadeFree();
                }
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
