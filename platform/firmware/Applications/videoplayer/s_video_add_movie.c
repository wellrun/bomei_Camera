#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Initialize.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_AVIPlayer.h"
#include "fwl_keyhandler.h"
#include "Eng_TopBar.h"
#include "Ctl_DisplayList.h"
#include "Ctl_Msgbox.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "svc_medialist.h"


typedef struct {
    T_DISPLAYLIST           displayList;
    T_ICONEXPLORER          *pIconExplorer;
    T_MSGBOX                msgbox;
    T_BOOL                  MsgFlag;
    T_USTR_FILE             CurFilePath;
    T_BOOL					fCurPathChange;
} T_VIDEO_ADD_MOVIE_PARM;

static T_VIDEO_ADD_MOVIE_PARM *pVideoAddMovieParm = AK_NULL;

static T_VOID VideoAddMovie_Add(T_BOOL SearchSub);

extern T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName);
extern T_VOID DisplayList_SetTopBarMenuIconStateInAVAddList(T_DISPLAYLIST *pDisplayList);

#endif
/*---------------------- BEGIN OF STATE s_audio_add_song ------------------------*/
void initvideo_add_movie(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_MP4,
        FILE_TYPE_3GP,
        FILE_TYPE_AKV,
        FILE_TYPE_AVI,
        FILE_TYPE_FLV,
        /*FILE_TYPE_RMVB,
        FILE_TYPE_RM,*/
        // FILE_TYPE_MKV,
        FILE_TYPE_VLT,
        FILE_TYPE_NONE
    };

    pVideoAddMovieParm = (T_VIDEO_ADD_MOVIE_PARM *)Fwl_Malloc(sizeof(T_VIDEO_ADD_MOVIE_PARM));
    AK_ASSERT_PTR_VOID(pVideoAddMovieParm, "initaudio_add_song error");

    DisplayList_init(&pVideoAddMovieParm->displayList, Fwl_GetDefPath(eVIDEO_PATH), \
            Res_GetStringByID(eRES_STR_VIDEOPLAYER_ADD_AUDIO), FileType);
    
    pVideoAddMovieParm->CurFilePath[0] = 0;
    pVideoAddMovieParm->fCurPathChange = AK_FALSE;

    MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    pVideoAddMovieParm->MsgFlag = AK_FALSE;

    TopBar_MenuIconShowSwitch(AK_TRUE);
#endif
}

void exitvideo_add_movie(void)
{
#ifdef SUPPORT_VIDEOPLAYER

	if (pVideoAddMovieParm->fCurPathChange == AK_TRUE)
	{
    	Fwl_SetDefPath(eVIDEO_PATH, pVideoAddMovieParm->CurFilePath);
    }
    
    DisplayList_Free(&pVideoAddMovieParm->displayList);
    pVideoAddMovieParm = Fwl_Free(pVideoAddMovieParm);

	TopBar_DisableMenuButton();
    TopBar_MenuIconShowSwitch(AK_FALSE);
#endif
}

void paintvideo_add_movie(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    if (pVideoAddMovieParm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pVideoAddMovieParm->msgbox);
    }
    else
    {
        if (1 == pVideoAddMovieParm->displayList.subLevel)
        {
            DisplayList_SetTopBarMenuIconStateInAVAddList(&pVideoAddMovieParm->displayList);
        }
        else
        {
            DisplayList_SetTopBarMenuIconState(&pVideoAddMovieParm->displayList);
        }
        DisplayList_Show(&pVideoAddMovieParm->displayList);
    }
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_add_movie(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_FILE_INFO     *FileInfo;
    T_eBACK_STATE DisplayListRet, msgRet;
    T_BOOL SearchSub = AK_FALSE;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pVideoAddMovieParm->displayList, DISPLAYLIST_REFRESH_ALL);
        MsgBox_SetRefresh(&pVideoAddMovieParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_1 == event)
        pVideoAddMovieParm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

    if (pVideoAddMovieParm->MsgFlag)
    {
        msgRet = MsgBox_Handler(&pVideoAddMovieParm->msgbox, event, pEventParm);
        if (eNext == msgRet)
        {
            SearchSub = AK_TRUE;
        }
        if (eNext == msgRet || eReturn == msgRet)
        {
            pVideoAddMovieParm->MsgFlag = AK_FALSE;
            if ((T_eKEY_ID)pEventParm->c.Param1 == kbCLEAR)
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                return 0;
            }
            VideoAddMovie_Add(SearchSub);
        }
    }
    else
    {
        DisplayListRet = DisplayList_Handler(&pVideoAddMovieParm->displayList, event, pEventParm);
        if (eNext == DisplayListRet)
        {
            FileInfo = DisplayList_Operate(&pVideoAddMovieParm->displayList);
            if (FileInfo != AK_NULL)
            {
                if (AK_FALSE == Utl_IsLegalFname(FileInfo->name))
                {
                    MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pVideoAddMovieParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoAddMovieParm->msgbox);
                    DisplayList_SetRefresh(&pVideoAddMovieParm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
                else
                {
                    VideoAddMovie_Add(SearchSub);
                }
            }
        }
        else if (eMenu == DisplayListRet)
        {
            FileInfo = DisplayList_GetItemContentFocus(&pVideoAddMovieParm->displayList);

            if ((AK_NULL == FileInfo) || (AK_FALSE == Utl_IsLegalFname(FileInfo->name))) // can't recognise file or folder
            {
                if (DisplayList_GetSubLevel(&pVideoAddMovieParm->displayList) == 0)
                    return 0;

                Fwl_Print(C3, M_FS, "Error file name = ");
                Printf_UC(FileInfo->name);
                
                MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                MsgBox_SetDelay(&pVideoAddMovieParm->msgbox, MSGBOX_DELAY_1);
                m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoAddMovieParm->msgbox);
                DisplayList_SetRefresh(&pVideoAddMovieParm->displayList, DISPLAYLIST_REFRESH_ALL);
            }
            else if (FileInfo != AK_NULL && (FileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER)
            {
                VideoAddMovie_Add(SearchSub);
            }
            else if (FileInfo != AK_NULL \
                && Utl_UStrCmp(FileInfo->name, _T("..")) != 0 \
                && (DisplayList_GetSubLevel(&pVideoAddMovieParm->displayList) != 1 \
                || Utl_UStrCmp(FileInfo->name, _T(".")) != 0))
            {
                pVideoAddMovieParm->MsgFlag = AK_TRUE;
                MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
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

#ifdef SUPPORT_VIDEOPLAYER

static T_VOID VideoAddMovie_Add(T_BOOL SearchSub)
{
    T_U16               *pFilePath;
    T_USTR_FILE         FilePath;
    T_FILE_INFO         *pFileInfo;
    T_BOOL  			AddRet = AK_FALSE;
    T_RECT              msgRect;
    T_BOOL              bCurPath = AK_FALSE;
	

    MsgBox_InitStr(&pVideoAddMovieParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
    MsgBox_Show(&pVideoAddMovieParm->msgbox);
    MsgBox_GetRect(&pVideoAddMovieParm->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

    pFilePath = DisplayList_GetCurFilePath(&pVideoAddMovieParm->displayList);
    pFileInfo = DisplayList_GetItemContentFocus(&pVideoAddMovieParm->displayList);
    if ((AK_NULL != pFilePath) && (AK_NULL != pFileInfo) && \
            (Utl_UStrLen(pFilePath) + Utl_UStrLen(pFileInfo->name) <= MAX_FILENM_LEN) && \
            (pVideoAddMovieParm->pIconExplorer != AK_NULL))
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

        AddRet = MList_AddItem(FilePath, SearchSub, AK_TRUE, eMEDIA_LIST_VIDEO);
    }

    if (AddRet)
    {
        Utl_UStrCpy(pVideoAddMovieParm->CurFilePath, FilePath);

        if (AK_FALSE == bCurPath)
        {
        	// pVideoAddMovieParm->fCurPathChange = AK_TRUE;
        	
            if(Fwl_FsIsDir(FilePath))
            {
                Utl_UStrCat(pVideoAddMovieParm->CurFilePath, _T("/"));
            }
            else
            {
                T_USTR_FILE tmppath, tmpname;
                Utl_USplitFilePath(FilePath, tmppath, tmpname);
                if(Fwl_IsRootDir(tmppath))
                {
                    Utl_UStrCpy(pVideoAddMovieParm->CurFilePath, (T_pCWSTR)Fwl_GetDefPath(eVIDEO_PATH));
                }
                else
                {
                    Utl_UStrCpy(pVideoAddMovieParm->CurFilePath, tmppath);
                }
            }
        }
        else
        {
            bCurPath = AK_FALSE;
        }
		
		if (!Fwl_FsIsDir(FilePath) && !AVIPlayer_IsSupportFileType(FilePath))
		{
			MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 1, ctFAILURE, csFILE_INVALID, MSGBOX_INFORMATION);
		}
		else
		{
			MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
		}
    }
    else
    {
        MsgBox_InitAfx(&pVideoAddMovieParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
    }
    MsgBox_SetDelay(&pVideoAddMovieParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoAddMovieParm->msgbox);
    DisplayList_SetRefresh(&pVideoAddMovieParm->displayList, DISPLAYLIST_REFRESH_ALL);

}

#endif
