
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_FileList.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER      *pIconExplorer;
    T_MSGBOX            msgbox;
} T_AUDIO_DELETE_LIST_PARM;

static T_AUDIO_DELETE_LIST_PARM *pAudio_Delete_List_Parm;
#endif

/*---------------------- BEGIN OF STATE s_audio_delete_cnfm ------------------------*/
void initaudio_delete_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_List_Parm = (T_AUDIO_DELETE_LIST_PARM *)Fwl_Malloc(sizeof(T_AUDIO_DELETE_LIST_PARM));
    AK_ASSERT_PTR_VOID(pAudio_Delete_List_Parm, "initaudio_delete_cnfm(): malloc error");

    MsgBox_InitAfx(&pAudio_Delete_List_Parm->msgbox, 0, ctHINT, csMP3_DELETE_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitaudio_delete_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_List_Parm = Fwl_Free(pAudio_Delete_List_Parm);
#endif
}

void paintaudio_delete_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    MsgBox_Show(&pAudio_Delete_List_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_delete_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   menuRet;
    T_pCWSTR        pFilePath;
    T_USTR_FILE     name, ext, ustr_1;
    T_BOOL          ret;

    ret = AK_TRUE;
    pFilePath = AK_NULL;
    
    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudio_Delete_List_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_1 == event)
    {
        pAudio_Delete_List_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
    }

    menuRet = MsgBox_Handler(&pAudio_Delete_List_Parm->msgbox, event, pEventParm);
    switch (menuRet)
    {
        case eNext:
            pFilePath = (T_pCWSTR)IconExplorer_GetItemContentFocus(pAudio_Delete_List_Parm->pIconExplorer);
            Utl_USplitFileName(pFilePath, name, ext);
            Utl_UStrLower(ext);

            Eng_StrMbcs2Ucs("alt", ustr_1);
            if (0 == Utl_UStrCmp(ext, ustr_1))
            {
                if (AK_TRUE != Fwl_FileDelete(pFilePath))
                {
                    ret  = AK_FALSE;
                }
            }

            if (AK_TRUE == ret)
            {
                if (AK_FALSE == IconExplorer_DelItemFocus(pAudio_Delete_List_Parm->pIconExplorer))
                {
                    ret  = AK_FALSE;
                }
            }
            
            if (AK_TRUE != ret)
            {
                MsgBox_InitAfx(&pAudio_Delete_List_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            }
            else
            {
                MsgBox_InitAfx(&pAudio_Delete_List_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            }
            MsgBox_SetDelay(&pAudio_Delete_List_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Delete_List_Parm->msgbox);

            break;
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
    }

#endif
    return 0;
}



