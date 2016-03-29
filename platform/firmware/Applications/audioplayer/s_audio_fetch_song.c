
#include "Fwl_public.h"


#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Initialize.h"
#include "Fwl_pfKeypad.h"
#include "Ctl_Msgbox.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_DisplayList.h"
#include "Eng_TopBar.h"
#include "fwl_pfkeypad.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "svc_medialist.h"


typedef struct {
    T_DISPLAYLIST           displayList;
    T_MSGBOX                msgbox;
    T_BOOL                  MsgFlag;
    T_USTR_FILE             CurFilePath;
} T_AUDIO_FETCH_SONG_PARM;

static T_AUDIO_FETCH_SONG_PARM *pAudioFetchSongParm = AK_NULL;
static T_BOOL fCurPathChange;
static T_VOID AudioFetchSong_Fetch(T_FILELIST_SEARCH_SUB_MODE SearchMode);
extern T_VOID DisplayList_SetTopBarMenuIconStateInAVAddList(T_DISPLAYLIST *pDisplayList);

extern T_FILE_TYPE AudioFileType[];

#endif

/*---------------------- BEGIN OF STATE s_audio_add_song ------------------------*/
void initaudio_fetch_song(void)
{
#ifdef SUPPORT_AUDIOPLAYER
/*
    T_FILE_TYPE FileType[] = {
		FILE_TYPE_RMVB, FILE_TYPE_RM,
        FILE_TYPE_MP1, FILE_TYPE_MP2, FILE_TYPE_MP3,
        FILE_TYPE_AAC, FILE_TYPE_AMR, FILE_TYPE_WMA,FILE_TYPE_ASF, FILE_TYPE_MID,
        FILE_TYPE_MIDI, FILE_TYPE_ADPCM, FILE_TYPE_WAV, FILE_TYPE_M4A, FILE_TYPE_MP4,        
        FILE_TYPE_FLAC_NATIVE, FILE_TYPE_FLAC_OGG, FILE_TYPE_FLAC_OGA, FILE_TYPE_APE,
        FILE_TYPE_ADIF, FILE_TYPE_ADTS, FILE_TYPE_ALT, FILE_TYPE_NONE
    };
*/
    pAudioFetchSongParm = (T_AUDIO_FETCH_SONG_PARM *)Fwl_Malloc(sizeof(T_AUDIO_FETCH_SONG_PARM));
    AK_ASSERT_PTR_VOID(pAudioFetchSongParm, "initaudio_add_song error");

    DisplayList_init(&pAudioFetchSongParm->displayList, Fwl_GetDefPath(eAUDIO_PATH), \
            Res_GetStringByID(eRES_STR_AUDIOPLAYER_FETCH_SONG), AudioFileType);

    pAudioFetchSongParm->CurFilePath[0] = 0;
    fCurPathChange = AK_FALSE;

    MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    pAudioFetchSongParm->MsgFlag = AK_FALSE;
#endif
}

void exitaudio_fetch_song(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    // save audio default path
    if (fCurPathChange == AK_TRUE)
    {
    	Fwl_SetDefPath(eAUDIO_PATH, pAudioFetchSongParm->CurFilePath);
    }
    
    DisplayList_Free(&pAudioFetchSongParm->displayList);
    pAudioFetchSongParm = Fwl_Free(pAudioFetchSongParm);
    
    TopBar_DisableMenuButton();
#endif
}

void paintaudio_fetch_song(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    if (pAudioFetchSongParm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pAudioFetchSongParm->msgbox);
    }
    else
    {
        if (1 == pAudioFetchSongParm->displayList.subLevel)
        {
            DisplayList_SetTopBarMenuIconStateInAVAddList(&pAudioFetchSongParm->displayList);
        }
        else
        {
            DisplayList_SetTopBarMenuIconState(&pAudioFetchSongParm->displayList);
        }
        DisplayList_Show(&pAudioFetchSongParm->displayList);
    }

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_fetch_song(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_FILE_INFO         *FileInfo;
    T_eBACK_STATE DisplayListRet, msgRet;
    T_FILELIST_SEARCH_SUB_MODE SearchMode = FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pAudioFetchSongParm->displayList, DISPLAYLIST_REFRESH_ALL);
        MsgBox_SetRefresh(&pAudioFetchSongParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (pAudioFetchSongParm->MsgFlag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pAudioFetchSongParm->msgbox, event, pEventParm);
        if (eNext == msgRet)
        {
            SearchMode = FILELIST_SEARCH_SUB_NO_RECODE_FOLDER;
        }
        if (eNext == msgRet || eReturn == msgRet)
        {
            pAudioFetchSongParm->MsgFlag = AK_FALSE;
            if ((T_eKEY_ID)pEventParm->c.Param1 == kbCLEAR)
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                return 0;
            }

            AudioFetchSong_Fetch(SearchMode);   // folder
        }
    }
    else
    {
        DisplayListRet = DisplayList_Handler(&pAudioFetchSongParm->displayList, event, pEventParm);
        if (eNext == DisplayListRet)
        {
            FileInfo = DisplayList_Operate(&pAudioFetchSongParm->displayList);
            if (FileInfo != AK_NULL)
            {
                if (!Utl_IsLegalFname(FileInfo->name))
                {
                    MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pAudioFetchSongParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioFetchSongParm->msgbox);
                    DisplayList_SetRefresh(&pAudioFetchSongParm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
                else
                {
                    AudioFetchSong_Fetch(SearchMode);   // add file
                }
            }
        }
        else if (eMenu == DisplayListRet) 
        {
            if (0 != pAudioFetchSongParm->displayList.subLevel)
            {
                FileInfo = DisplayList_GetItemContentFocus(&pAudioFetchSongParm->displayList);
                if ((AK_NULL == FileInfo) || (AK_FALSE == Utl_IsLegalFname(FileInfo->name))) // can't recognise file or folder
                {
                    Printf_UC(FileInfo->name);
                    MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pAudioFetchSongParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioFetchSongParm->msgbox);
                    DisplayList_SetRefresh(&pAudioFetchSongParm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
                else if (FileInfo != AK_NULL && (FileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER)
                {
                    AudioFetchSong_Fetch(SearchMode);
                }
                else if (FileInfo != AK_NULL \
                    && Utl_UStrCmp(FileInfo->name, _T("..")) != 0 \
                    && (DisplayList_GetSubLevel(&pAudioFetchSongParm->displayList) != 1 \
                    || Utl_UStrCmp(FileInfo->name, _T(".")) != 0))
                {
                    pAudioFetchSongParm->MsgFlag = AK_TRUE;
                    MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
                }
            }
        }
        else if (eReturn == DisplayListRet || eHome == DisplayListRet)
        {
            ReturnDefauleProc(DisplayListRet, pEventParm);
        }
    }

#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

static T_VOID AudioFetchSong_Fetch(T_FILELIST_SEARCH_SUB_MODE SearchMode)
{
    T_USTR_FILE         FilePath;
    T_RECT              msgRect;
    T_BOOL              bCurPath = AK_FALSE;
    T_FILE_INFO         *pFileInfo = AK_NULL;
    T_U16               *pFilePath = AK_NULL;
    T_BOOL              ret = AK_FALSE;
	T_BOOL              bSearchSub = AK_FALSE;

    MsgBox_InitStr(&pAudioFetchSongParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
    MsgBox_Show(&pAudioFetchSongParm->msgbox);
    MsgBox_GetRect(&pAudioFetchSongParm->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

    pFilePath = DisplayList_GetCurFilePath(&pAudioFetchSongParm->displayList);
    pFileInfo = DisplayList_GetItemContentFocus(&pAudioFetchSongParm->displayList);
    
    if (AK_NULL != pFilePath && AK_NULL != pFileInfo \
        && (Utl_UStrLen(pFilePath) + Utl_UStrLen(pFileInfo->name) <= MAX_FILENM_LEN))
    {
        Utl_UStrCpy(FilePath, pFilePath);
        if (0 != Utl_UStrCmp(pFileInfo->name, _T(".")))
        {
            Utl_UStrCat(FilePath, pFileInfo->name);
        }
        else
        {
            bCurPath = AK_TRUE;
        }

		if (FILELIST_SEARCH_SUB_NO_RECODE_FOLDER == SearchMode
			|| FILELIST_SEARCH_SUB_RECODE_FOLDER == SearchMode)
		{
			bSearchSub = AK_TRUE;
		}

        ret = MList_AddItem(FilePath, bSearchSub, AK_TRUE, eMEDIA_LIST_AUDIO);
    }
    
    if (ret)
    {
        Utl_UStrCpy(pAudioFetchSongParm->CurFilePath, FilePath);

        if (AK_FALSE == bCurPath)
        {
            // fCurPathChange = AK_TRUE;
          
            if(Fwl_FsIsDir(FilePath))
            {
                Utl_UStrCat(pAudioFetchSongParm->CurFilePath, _T("/"));
            }
            else
            {
                T_USTR_FILE tmppath, tmpname;
                Utl_USplitFilePath(FilePath, tmppath, tmpname);
                if(Fwl_IsRootDir(tmppath))
                {
                    Utl_UStrCpy(pAudioFetchSongParm->CurFilePath, Fwl_GetDefPath(eAUDIO_PATH));
                }
                else
                {
                    Utl_UStrCpy(pAudioFetchSongParm->CurFilePath, tmppath);
                }
            }
        }
        else
        {
            bCurPath = AK_FALSE;
        }

        MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
    }
    else
    {
        MsgBox_InitAfx(&pAudioFetchSongParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
    }
    MsgBox_SetDelay(&pAudioFetchSongParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioFetchSongParm->msgbox);
    DisplayList_SetRefresh(&pAudioFetchSongParm->displayList, DISPLAYLIST_REFRESH_ALL);
}


#endif
