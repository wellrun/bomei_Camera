#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Initialize.h"
#include "Fwl_pfKeypad.h"
#include "Ctl_Msgbox.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_AVIPlayer.h"
#include "Ctl_DisplayList.h"
#include "Eng_TopBar.h"
#include "fwl_keyhandler.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST           displayList;
    T_ICONEXPLORER          *pIconExplorer;
    T_MSGBOX                msgbox;
} T_VIDEO_READ_LIST_PARM;

static T_VIDEO_READ_LIST_PARM *pvideoReadListParm = AK_NULL;

static T_VOID VideoReadList_Add(T_FILELIST_SEARCH_SUB_MODE SearchMode);

extern T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName);
#endif
/*---------------------- BEGIN OF STATE s_audio_add_song ------------------------*/
void initvideo_read_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_VLT,
        FILE_TYPE_NONE
    };
    pvideoReadListParm = (T_VIDEO_READ_LIST_PARM *)Fwl_Malloc(sizeof(T_VIDEO_READ_LIST_PARM));
    AK_ASSERT_PTR_VOID(pvideoReadListParm, "initaudio_read_list error");

    DisplayList_init(&pvideoReadListParm->displayList, Fwl_GetDefPath(eVIDEOLIST_PATH), \
            Res_GetStringByID(eRES_STR_AUDIOPLAYER_READ_LIST), FileType);
    
    TopBar_MenuIconShowSwitch(AK_FALSE);
#endif
}

void exitvideo_read_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    DisplayList_Free(&pvideoReadListParm->displayList);
    pvideoReadListParm = Fwl_Free(pvideoReadListParm);
    
    TopBar_MenuIconShowSwitch(AK_TRUE);
#endif
}

void paintvideo_read_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    DisplayList_Show(&pvideoReadListParm->displayList);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_read_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE DisplayListRet;
    T_FILELIST_SEARCH_SUB_MODE SearchMode = FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER;
    T_MMI_KEYPAD phyKey;
    T_FILE_INFO *FileInfo;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pvideoReadListParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_2 == event)
        pvideoReadListParm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

    DisplayListRet = DisplayList_Handler(&pvideoReadListParm->displayList, event, pEventParm);
    switch (DisplayListRet)
    {
        case eNext:
            FileInfo = DisplayList_Operate(&pvideoReadListParm->displayList);
            if (FileInfo != AK_NULL)
            {
                IconExplorer_DelAllItem(pvideoReadListParm->pIconExplorer);
                VideoReadList_Add(SearchMode);
            }
            break;
        case eStay:
            if (M_EVT_USER_KEY == event)
            {
                phyKey.keyID = pEventParm->c.Param1;
                phyKey.pressType = pEventParm->c.Param2;
                FileInfo = DisplayList_GetItemContentFocus(&pvideoReadListParm->displayList);
                if (phyKey.pressType == PRESS_SHORT && phyKey.keyID == kbMENU \
                    && FileInfo != AK_NULL \
                    && (FileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER)
                {
                    IconExplorer_DelAllItem(pvideoReadListParm->pIconExplorer);
                    VideoReadList_Add(SearchMode);
                }
            }
            break;
        default:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_VIDEOPLAYER

static T_VOID VideoReadList_Add(T_FILELIST_SEARCH_SUB_MODE SearchMode)
{
    T_U16               *pFilePath;
    T_USTR_FILE         FilePath;//, Ustrtmp;
    T_FILE_INFO         *pFileInfo;
    T_FILELIST_ADD_RET  FileListAddRet = FILELIST_ADD_ERROR;
    T_RECT              msgRect;
    T_FILELIST          FileList;

    MsgBox_InitStr(&pvideoReadListParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
    MsgBox_Show(&pvideoReadListParm->msgbox);
    MsgBox_GetRect(&pvideoReadListParm->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

    pFilePath = DisplayList_GetCurFilePath(&pvideoReadListParm->displayList);
    pFileInfo = DisplayList_GetItemContentFocus(&pvideoReadListParm->displayList);
    if ((AK_NULL != pFilePath) && (AK_NULL != pFileInfo) && \
            (Utl_UStrLen(pFilePath) + Utl_UStrLen(pFileInfo->name) <= MAX_FILENM_LEN) && \
            (pvideoReadListParm->pIconExplorer != AK_NULL))
    {
        Utl_UStrCpy(FilePath, pFilePath);
        Utl_UStrCat(FilePath, pFileInfo->name);

        FileList_Init(&FileList, VIDEOLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE, AK_NULL);
        FileListAddRet = FileList_Add(&FileList, FilePath, SearchMode);
        FileList_ToIconExplorer(&FileList, pvideoReadListParm->pIconExplorer, eINDEX_TYPE_VIDEO, AK_NULL);
        FileList_Free(&FileList);
    }

    if (FILELIST_ADD_SUCCESS == FileListAddRet)
        MsgBox_InitAfx(&pvideoReadListParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
    else if (FILELIST_ADD_NOSPACE == FileListAddRet)
        MsgBox_InitAfx(&pvideoReadListParm->msgbox, 1, ctFAILURE, csMP3_NOT_ENOUGH_SPACE, MSGBOX_INFORMATION);
    else if (FILELIST_ADD_OUTPATHDEEP== FileListAddRet)
        MsgBox_InitAfx(&pvideoReadListParm->msgbox, 1, ctFAILURE, csOUT_PATH_DEEP, MSGBOX_INFORMATION);
    else
        MsgBox_InitAfx(&pvideoReadListParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
    MsgBox_SetDelay(&pvideoReadListParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pvideoReadListParm->msgbox);
    DisplayList_SetRefresh(&pvideoReadListParm->displayList, DISPLAYLIST_REFRESH_ALL);

}




#endif

