
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Ctl_DisplayList.h"
#include "Fwl_Initialize.h"
#include "Eng_ImgConvert.h"
#include "Ctl_Msgbox.h"
#include "Eng_FileManage.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfkeypad.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Ctl_APlayerList.h"



typedef struct {
    T_DISPLAYLIST   *pDisplayList;
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
    T_BOOL          MsgFlag;
} T_AUDIO_RECORD_LIST_MENU_PARM;

static T_AUDIO_RECORD_LIST_MENU_PARM *pAudioRecordListMenuParm;
#endif
/*---------------------- BEGIN OF STATE s_audio_record_list_menu ------------------------*/
void initaudio_recorder_list_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRecordListMenuParm = (T_AUDIO_RECORD_LIST_MENU_PARM *)Fwl_Malloc(sizeof(T_AUDIO_RECORD_LIST_MENU_PARM));
    AK_ASSERT_PTR_VOID(pAudioRecordListMenuParm, "initaudio_recorder_list_menu(): malloc error");

    MenuStructInit(&pAudioRecordListMenuParm->IconExplorer);
    GetMenuStructContent(&pAudioRecordListMenuParm->IconExplorer, mnAUDIORECORDER_LIST_MENU);

    MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    pAudioRecordListMenuParm->MsgFlag = AK_FALSE;

    TopBar_DisableMenuButton();
#endif
}

void exitaudio_recorder_list_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Free(&pAudioRecordListMenuParm->IconExplorer);
    pAudioRecordListMenuParm = Fwl_Free(pAudioRecordListMenuParm);
#endif
}

void paintaudio_recorder_list_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    if (pAudioRecordListMenuParm->MsgFlag == AK_TRUE)
        MsgBox_Show(&pAudioRecordListMenuParm->msgbox);
    else
        IconExplorer_Show(&pAudioRecordListMenuParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_recorder_list_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE IconExplorerRet, msgRet;
    T_FILE_INFO *FileInfo;
    T_BOOL SubFolder = AK_FALSE;
    T_USTR_FILE     FilePath;
    T_BOOL addRet = AK_FALSE;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioRecordListMenuParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_MENU)
    {
        pAudioRecordListMenuParm->pDisplayList = (T_DISPLAYLIST *)pEventParm;
        FileInfo = DisplayList_GetItemContentFocus(pAudioRecordListMenuParm->pDisplayList);
    }

    if (pAudioRecordListMenuParm->MsgFlag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pAudioRecordListMenuParm->msgbox, event, pEventParm);
        switch (msgRet)
        {
        case eNext:
            SubFolder = AK_TRUE;
        case eReturn:
                pAudioRecordListMenuParm->MsgFlag = AK_FALSE;

                if ((M_EVT_USER_KEY == event) && ((T_eKEY_ID)pEventParm->c.Param1 == kbCLEAR))
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                break;
            }

                MsgBox_InitStr(&pAudioRecordListMenuParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
                MsgBox_Show(&pAudioRecordListMenuParm->msgbox);
                Fwl_RefreshDisplay();

                FileInfo = DisplayList_GetItemContentFocus(pAudioRecordListMenuParm->pDisplayList);
                if (FileInfo != AK_NULL)
                {
                   Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pAudioRecordListMenuParm->pDisplayList));
                   Utl_UStrCat(FilePath, FileInfo->name);
                   addRet = AudioPlayer_Add(FilePath, SubFolder);
                }

                if (addRet)
                {
                    MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pAudioRecordListMenuParm->msgbox, MSGBOX_DELAY_0);
                }
                else
                {
                    MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pAudioRecordListMenuParm->msgbox, MSGBOX_DELAY_1);
                }

                 m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRecordListMenuParm->msgbox);


                 break;
        default:
            break;
        }
    }
    else
    {
        IconExplorerRet = IconExplorer_Handler(&pAudioRecordListMenuParm->IconExplorer, event, pEventParm);
        switch (IconExplorerRet)
        {
        case eNext:
            switch (IconExplorer_GetItemFocusId(&pAudioRecordListMenuParm->IconExplorer))
            {
            case 10:
                FileInfo = DisplayList_GetItemContentFocus(pAudioRecordListMenuParm->pDisplayList);
                if (FileInfo == AK_NULL)
                {
                        MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pAudioRecordListMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRecordListMenuParm->msgbox);
                        break;
                }
                if ((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER)
                {
                    pAudioRecordListMenuParm->MsgFlag = AK_TRUE;
                    MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
                }
                else
                {
                    MsgBox_InitStr(&pAudioRecordListMenuParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
                    MsgBox_Show(&pAudioRecordListMenuParm->msgbox);
                    Fwl_RefreshDisplay();
                    Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pAudioRecordListMenuParm->pDisplayList));
                    Utl_UStrCat(FilePath, FileInfo->name);
                    addRet = AudioPlayer_Add(FilePath, AK_FALSE);
                    if (addRet)
                    {
                        MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pAudioRecordListMenuParm->msgbox, MSGBOX_DELAY_0);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pAudioRecordListMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pAudioRecordListMenuParm->msgbox, MSGBOX_DELAY_1);
                    }
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRecordListMenuParm->msgbox);
                }
                break;
            case 20:
                pEventParm->p.pParam1 = (T_pVOID)pAudioRecordListMenuParm->pDisplayList;
                m_triggerEvent(M_EVT_1, (T_EVT_PARAM *)pEventParm);
                break;
            case 30:
                pEventParm->p.pParam1 = (T_pVOID)pAudioRecordListMenuParm->pDisplayList;
                m_triggerEvent(M_EVT_2, pEventParm);
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
