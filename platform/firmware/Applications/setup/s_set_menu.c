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
    T_U8            bMMI;
    T_BOOL             bEnableMB;
} T_SETUP_PARM;

static T_SETUP_PARM *pSetupParm;
static T_RES_LANGUAGE SetupLang;

static T_VOID Setup_ResumeFunc(T_VOID);

static T_U8 fontsize;
#endif
/*---------------------- BEGIN OF STATE s_set_menu ------------------------*/
void initset_menu(void)
{
#ifdef SUPPORT_SYS_SET

    pSetupParm = (T_SETUP_PARM *)Fwl_Malloc(sizeof(T_SETUP_PARM));
    AK_ASSERT_PTR_VOID(pSetupParm, "initset_menu(): malloc error");
    pSetupParm->bMMI = 0;

    SetupLang = gs.Lang;
    fontsize = gs.FontSize;

    MenuStructInit(&pSetupParm->IconExplorer);
    GetMenuStructContent(&pSetupParm->IconExplorer, mnSETUP_MENU);

    pSetupParm->bEnableMB = TopBar_GetMenuButtonState();

    if (pSetupParm->bEnableMB)
    {
        TopBar_DisableMenuButton();
    }
    
    m_regResumeFunc(Setup_ResumeFunc);
#endif
}

void exitset_menu(void)
{
#ifdef SUPPORT_SYS_SET

    if (pSetupParm->bEnableMB)
    {
        TopBar_EnableMenuButton();
    }
    
    IconExplorer_Free(&pSetupParm->IconExplorer);
    pSetupParm = Fwl_Free(pSetupParm);
#endif
}

void paintset_menu(void)
{
#ifdef SUPPORT_SYS_SET
    if(0 != pSetupParm->bMMI)
    {
        MsgBox_Show(&pSetupParm->msgbox);
    }
    else
    {
        IconExplorer_Show(&pSetupParm->IconExplorer);
    }

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pSetupParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if(0 != pSetupParm->bMMI)
        IconExplorerRet = MsgBox_Handler(&pSetupParm->msgbox, event, pEventParm);
    else
        IconExplorerRet = IconExplorer_Handler(&pSetupParm->IconExplorer, event, pEventParm);

    if(0 != pSetupParm->bMMI)
    {
        switch(IconExplorerRet)
        {
        case eReturn:
        case eNext:
            IconExplorer_SetRefresh(&pSetupParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
            break;
        default:
            break;
        }

        return 0;
    }

    if(0 == pSetupParm->bMMI)
    {
        switch (IconExplorerRet)
        {
        case eNext:
            switch (IconExplorer_GetItemFocusId(&pSetupParm->IconExplorer))
            {
            case 10: // Language
                GE_ShadeInit();
                m_triggerEvent(M_EVT_1, pEventParm);
                break;
            case 20: // Memory Infomation
                m_triggerEvent(M_EVT_3, pEventParm);
                break;
            case 30: // Version Infomation
                m_triggerEvent(M_EVT_4, pEventParm);
                break;
            case 40: // Factory setting
                m_triggerEvent(M_EVT_5, pEventParm);
                break;
            case 50: // Personal setting
                GE_ShadeInit();
                m_triggerEvent(M_EVT_6, pEventParm);
                break;
            case 70: // Time setting
                GE_ShadeInit();
                m_triggerEvent(M_EVT_7, pEventParm);
                break;
            case 80: // lcd brightness setting
                GE_ShadeInit();
                m_triggerEvent(M_EVT_8, pEventParm);
                break;
    #ifdef SUPPORT_TVOUT
            case 130:
                GE_ShadeInit();
                m_triggerEvent(M_EVT_DISPLAY_SWITCH, pEventParm);
                break;
    #endif
    #ifdef TOUCH_SCR
            case 60: //calibrate touch screen
                pEventParm->c.Param8 = (T_U8)AK_FALSE;
                m_triggerEvent(M_EVT_TSCR_CALIBRATE, pEventParm);
                break;
    #endif  
    #ifdef SPIBOOT
            case 90://system update
                GE_ShadeInit();
                m_triggerEvent(M_EVT_9, pEventParm);
                break;
    #endif
            case 100://lib version
                GE_ShadeInit();
                m_triggerEvent(M_EVT_10, pEventParm);
                break;

#ifdef SUPPORT_VFONT                
            case 140: // VFont set
                GE_ShadeInit();
                m_triggerEvent(M_EVT_SET_VFONT, pEventParm);
                break;
#endif

			case 110: // watchdog test
                GE_ShadeInit();
                m_triggerEvent(M_EVT_WATCHDOG_TEST, pEventParm);
                break;

            default:
                break;
            }
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
        }
    }
#endif
    return 0;
}



#ifdef SUPPORT_SYS_SET

static T_VOID Setup_ResumeFunc(T_VOID)
{
    if ((SetupLang != gs.Lang) || (fontsize != gs.FontSize))
    {
        IconExplorer_Free(&pSetupParm->IconExplorer);

        MenuStructInit(&pSetupParm->IconExplorer);
        GetMenuStructContent(&pSetupParm->IconExplorer, mnSETUP_MENU);
        SetupLang = gs.Lang;
    fontsize = gs.FontSize;
    }
    
}
#endif
