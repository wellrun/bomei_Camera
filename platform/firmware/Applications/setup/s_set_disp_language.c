

#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Eng_DataConvert.h"
#include "Fwl_osFS.h"
#include "ctl_ebook.h"
#include "Ctl_FileList.h"
#include "Ctl_APlayerList.h"
#include "Ctl_AVIPlayer.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
    T_BOOL          MsgFlag;
    T_S16           focusID;
} T_LANG_PARM;

static T_LANG_PARM *pLangParm = AK_NULL;
#endif
/*---------------------- BEGIN OF STATE s_set_disp_language ------------------------*/
void initset_disp_language(void)
{
#ifdef SUPPORT_SYS_SET

    pLangParm = (T_LANG_PARM *)Fwl_Malloc(sizeof(T_LANG_PARM));
    AK_ASSERT_PTR_VOID(pLangParm, "initlanguage error");

    MenuStructInit(&pLangParm->IconExplorer);
    GetMenuStructContent(&pLangParm->IconExplorer, mnLANGUAGE_MENU);
    IconExplorer_SetFocus(&pLangParm->IconExplorer, gs.Lang);

#if (SDRAM_MODE == 8)    
    MsgBox_InitAfx(&pLangParm->msgbox, 0, ctHINT, csEBK_DELETE_FOLDER, MSGBOX_QUESTION | MSGBOX_YESNO);
    pLangParm->MsgFlag = AK_FALSE;
#endif
#endif
}

void exitset_disp_language(void)
{
#ifdef SUPPORT_SYS_SET

    IconExplorer_Free(&pLangParm->IconExplorer);
    pLangParm = Fwl_Free(pLangParm);
#endif
}

void paintset_disp_language(void)
{
#ifdef SUPPORT_SYS_SET

#if (SDRAM_MODE == 8)
    if (pLangParm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pLangParm->msgbox);
    }
    else
    {
        IconExplorer_Show(&pLangParm->IconExplorer);
    }
#else
    IconExplorer_Show(&pLangParm->IconExplorer);
#endif    

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_language(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   IconExplorerRet;
#if (SDRAM_MODE == 8)
    T_eBACK_STATE   msgRet = eStay;
#endif

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pLangParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

#if (SDRAM_MODE == 8)
    if (AK_TRUE == pLangParm->MsgFlag)
    {
        msgRet = MsgBox_Handler(&pLangParm->msgbox, event, pEventParm);
        switch (msgRet)
        {
            case eNext:
                gs.Lang =(T_RES_LANGUAGE)pLangParm->focusID;
                
                /*power off system*/
                VME_ReTriggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
                gb.ResetAfterPowerOff = AK_TRUE;

                pLangParm->MsgFlag = AK_FALSE;
                break;
            case eReturn:
                pLangParm->MsgFlag = AK_FALSE;
                break;
            default:
                break;
        }

    }
    else
#endif
    {
        IconExplorerRet = IconExplorer_Handler(&pLangParm->IconExplorer, event, pEventParm);

        switch (IconExplorerRet)
        {
        case eNext:
            pLangParm->focusID = (T_U16)IconExplorer_GetItemFocusId(&pLangParm->IconExplorer);
            if (pLangParm->focusID != gs.Lang)
            {
                //EBCtl_CleanBookmark(gb.ebkBookmark);                

#if (SDRAM_MODE == 8)      
                MsgBox_InitAfx(&pLangParm->msgbox, 0, ctHINT, csCHANGE_LANG_POWER_OFF, MSGBOX_QUESTION | MSGBOX_YESNO);
                pLangParm->MsgFlag = AK_TRUE;
#else
                gs.Lang =(T_RES_LANGUAGE)pLangParm->focusID;

                Eng_SetLanguage(gs.Lang);

                //Fwl_FileDelete(_T(AUDIO_META_INFO_FILE));
                //Fwl_FileDelete(_T(AUDIOLIST_DEF_FILE));

                //AVIPlayer_UpdateVideoList();  //SW10A00002689
				//APList_Update();

                gb.MainMenuRefresh = AK_TRUE;
#endif                
            }
            
#if (SDRAM_MODE >= 16)
            GE_ShadeInit();
            m_triggerEvent(M_EVT_EXIT, pEventParm);
#endif
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
        }
    }
#endif
    return 0;
}

