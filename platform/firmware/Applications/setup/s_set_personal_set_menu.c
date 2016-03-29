
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Lib_state.h"
#include "Fwl_Initialize.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
} T_PERSET_PARM;

static T_PERSET_PARM *pPerSetParm;
//extern T_VOID MenuPicSetBuffer(T_VOID);

#ifdef UI_USE_ICONMENU
extern T_VOID StdbPicSetBuffer(T_VOID);
#endif

static T_U8 fontsize;
static T_VOID Setup_ResumeFunc(T_VOID)
{
    if (fontsize != gs.FontSize)
    {
        IconExplorer_Free(&pPerSetParm->IconExplorer);

        MenuStructInit(&pPerSetParm->IconExplorer);
    	GetMenuStructContent(&pPerSetParm->IconExplorer, mnPERSONAL_MENU);
        fontsize = gs.FontSize;
    }
   
}
#endif
/*---------------------- BEGIN OF STATE s_set_personal_set_menu ------------------------*/
void initset_personal_set_menu(void)
{
#ifdef SUPPORT_SYS_SET

	fontsize = gs.FontSize;
    pPerSetParm = (T_PERSET_PARM *)Fwl_Malloc(sizeof(T_PERSET_PARM));
    AK_ASSERT_PTR_VOID(pPerSetParm, "initstdb_standby(): malloc error");

    MenuStructInit(&pPerSetParm->IconExplorer);
    GetMenuStructContent(&pPerSetParm->IconExplorer, mnPERSONAL_MENU);
	m_regResumeFunc(Setup_ResumeFunc);
#endif
}

void exitset_personal_set_menu(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Free(&pPerSetParm->IconExplorer);
    pPerSetParm = Fwl_Free(pPerSetParm);
#endif
}

void paintset_personal_set_menu(void)
{
#ifdef SUPPORT_SYS_SET
    IconExplorer_Show(&pPerSetParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_personal_set_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pPerSetParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pPerSetParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
        case eNext:
            switch ( IconExplorer_GetItemFocusId(&pPerSetParm->IconExplorer) )
            {
                case 10:
                    m_triggerEvent(M_EVT_1, pEventParm);
                    break;
                case 20:
                    m_triggerEvent(M_EVT_2, pEventParm);
                    break;
                case 50:
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_4, pEventParm);
                    break;
                case 60:
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_5, pEventParm);
                    break;
                case 70:
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_6, pEventParm);
                    break;
                case 100:
                    m_triggerEvent(M_EVT_8, pEventParm);
                    break;
                // the animated setting    
                case 110:
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_9, pEventParm);
                    break;
                case 120:
                    m_triggerEvent(M_EVT_10, pEventParm);
                    break;		
                default:
                    break;
            }
            break;

        case eOption:
            switch (IconExplorer_GetItemFocusId(&pPerSetParm->IconExplorer))
            {
                case 30:
#ifdef UI_USE_ICONMENU
                    StdbPicSetBuffer();
#endif
                    break;
                case 40:
                    Menu_LoadRes();
                    //specify item's background image over again after image data is changed
					SlipMgr_LoadItemBgImg(pPerSetParm->IconExplorer.pSlipMgr);

                    IconExplorer_SetRefresh(&pPerSetParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
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

