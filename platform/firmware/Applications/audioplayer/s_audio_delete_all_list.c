
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Ctl_FileList.h"
#include "Fwl_osFS.h"
#include "Eng_DataConvert.h"
#include "Fwl_Initialize.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER		*pIconExplorer;
    T_MSGBOX			msgbox;
} T_AUDIO_DELETE_ALL_LIST;

static T_AUDIO_DELETE_ALL_LIST *pAudio_Delete_All_List;
#endif


/*---------------------- BEGIN OF STATE s_audio_delete_all_cnfm ------------------------*/
void initaudio_delete_all_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_All_List = (T_AUDIO_DELETE_ALL_LIST *)Fwl_Malloc(sizeof(T_AUDIO_DELETE_ALL_LIST));
    AK_ASSERT_PTR_VOID(pAudio_Delete_All_List, "initaudio_delete_all_cnfm(): malloc error");

    MsgBox_InitAfx(&pAudio_Delete_All_List->msgbox, 0, ctHINT, csMP3_DELETE_ALL_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitaudio_delete_all_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_All_List = Fwl_Free(pAudio_Delete_All_List);
#endif
}

void paintaudio_delete_all_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    MsgBox_Show(&pAudio_Delete_All_List->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_delete_all_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   menuRet;
    T_pCWSTR        pFilePath;
    T_hFILESTAT     find;
    T_FILE_INFO     FileInfo;
    T_USTR_FILE     name, ext;
    T_USTR_FILE     FindPath, FilePath;
    T_BOOL          ret;

    ret = AK_TRUE;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudio_Delete_All_List->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_2 == event)
    {
        pAudio_Delete_All_List->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
    }
    
    menuRet = MsgBox_Handler(&pAudio_Delete_All_List->msgbox, event, pEventParm);
    switch(menuRet)
    {
        case eNext:
            pFilePath = (T_pCWSTR)IconExplorer_GetItemContentFocus(pAudio_Delete_All_List->pIconExplorer);
            Utl_USplitFileName(pFilePath, name, ext);
            Utl_UStrLower(ext);

            if (0 == Utl_UStrCmp(ext, _T("alt")))
            {
                Eng_StrMbcs2Ucs("*.alt", ext);
                Utl_UStrCpy(FindPath, (T_pCWSTR)Fwl_GetDefPath(eAUDIOLIST_PATH));
                Utl_UStrCat(FindPath, ext);
               
                find = Fwl_FsFindFirst(FindPath);
                if (find != -1)
                {
                    do
                    {
                        Fwl_FsFindInfo(&FileInfo, find);
                        if (Utl_UStrCmp(FileInfo.name, _T(".")) == 0 \
                            || (Utl_UStrCmp(FileInfo.name, _T(".."))) == 0)
                        {
                            continue;
                        }
                        Utl_UStrCpy(FilePath, (T_pCWSTR)(Fwl_GetDefPath(eAUDIOLIST_PATH)));
                        Utl_UStrCat(FilePath, FileInfo.name);
                        if (AK_TRUE != Fwl_FileDelete((T_pWSTR)FilePath))
                        {
                            ret  = AK_FALSE;
                            break;
                        }

                        if (AK_FALSE == IconExplorer_DelItemFocus(pAudio_Delete_All_List->pIconExplorer))
                        {
                            ret  = AK_FALSE;
                            break;
                        }
                    } while (Fwl_FsFindNext(find));
					Fwl_FsFindClose(find);
					find =FS_INVALID_STATHANDLE;
                }
            }
            else
            {
                if (AK_FALSE == IconExplorer_DelAllItem(pAudio_Delete_All_List->pIconExplorer))
                {
                    ret  = AK_FALSE;
                }
            }

            if (AK_TRUE != ret)
            {
                MsgBox_InitAfx(&pAudio_Delete_All_List->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            }
            else
            {
                MsgBox_InitAfx(&pAudio_Delete_All_List->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            }
            MsgBox_SetDelay(&pAudio_Delete_All_List->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Delete_All_List->msgbox);
        
            break;
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
    }

#endif
    return 0;
}


