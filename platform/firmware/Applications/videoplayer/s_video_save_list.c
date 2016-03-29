
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Ctl_AudioPlayer.h"
#include "Ctl_AVIPlayer.h"
#include "fwl_keyhandler.h"
#include "Ctl_MsgBox.h"
#include "Fwl_osFS.h"
#include "fwl_oscom.h"
#include "Fwl_Initialize.h"
#include "Eng_DataConvert.h"
#include "Eng_Math.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"

#define VIDEO_LIST_FILM_NAME                "FILM%06d.VLT"
#define VIDEO_LIST_ALBUM_NAME               "ALBUM%06d.VLT"
#define VIDEO_LIST_MOVIE_NAME               "MOVIE%06d.VLT"


typedef struct {
    T_ICONEXPLORER          *pIconExplorer;
    T_ICONEXPLORER          IconExplorer;
    T_MSGBOX                msgbox;
} T_VIDEO_SAVE_LIST_PARM;

static T_VIDEO_SAVE_LIST_PARM *pVideoSaveListParm = AK_NULL;

static T_BOOL VideoListMenu_SaveList(T_U8 name);

extern T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName);
#endif

void initvideo_save_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideoSaveListParm = (T_VIDEO_SAVE_LIST_PARM *)Fwl_Malloc(sizeof(T_VIDEO_SAVE_LIST_PARM));
    AK_ASSERT_PTR_VOID(pVideoSaveListParm, "initaudio_list_menu(): pVideoSaveListParm malloc error");

    MenuStructInit(&pVideoSaveListParm->IconExplorer);
    GetMenuStructContent(&pVideoSaveListParm->IconExplorer, mnMOVIE_SAVELIST);
#endif
}

void exitvideo_save_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Free(&pVideoSaveListParm->IconExplorer);
    pVideoSaveListParm = Fwl_Free(pVideoSaveListParm);
#endif
}

void paintvideo_save_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Show(&pVideoSaveListParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_save_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U8 name=0;
    T_U64_INT freeSize = {0};
    T_USTR_FILE     FilePath;
    T_U32   tickcount1, tickcount2;
    T_BOOL  MsgboxFlag = AK_FALSE;
    T_pRECT pMBoxRct = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pVideoSaveListParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_3)
        pVideoSaveListParm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

    IconExplorerRet = IconExplorer_Handler(&pVideoSaveListParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
        case eNext:

            if (MPLAYER_END < MPlayer_GetStatus())
            {
                tickcount1 = Fwl_GetTickCount();

                MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 0, ctHINT, csWAITING, MSGBOX_INFORMATION);
                MsgBox_Show(&pVideoSaveListParm->msgbox);
                pMBoxRct = &pVideoSaveListParm->msgbox.res.MsgBkImgRct;
                Fwl_InvalidateRect( pMBoxRct->left, pMBoxRct->top, pMBoxRct->width, pMBoxRct->height);
                MsgboxFlag = AK_TRUE;
            }

            Utl_UStrCpy(FilePath, (T_pCWSTR)Fwl_GetDefPath(eVIDEOLIST_PATH));
            Fwl_FsGetFreeSize(FilePath[0], &freeSize);
            
            switch (IconExplorer_GetItemFocusId(&pVideoSaveListParm->IconExplorer))
            {
                case 0:
                    name = 0;
                    if (VideoListMenu_SaveList(name))
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
                    }
                    else if (U64cmpU32(&freeSize, SAVELIST_MINIMAL_SPACE) < 0)
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csSAVESPACEFULL, MSGBOX_INFORMATION);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csEMPTY_DONE, MSGBOX_INFORMATION);
                    }
                    MsgBox_SetDelay(&pVideoSaveListParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoSaveListParm->msgbox);
                    break;
                case 1:
                    name = 1;
                    if (VideoListMenu_SaveList(name))
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
                    }
                    else if (U64cmpU32(&freeSize, SAVELIST_MINIMAL_SPACE) < 0)
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csSAVESPACEFULL, MSGBOX_INFORMATION);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csEMPTY_DONE, MSGBOX_INFORMATION);
                    }
                    MsgBox_SetDelay(&pVideoSaveListParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoSaveListParm->msgbox);
                    break;
                 case 2:
                    name = 2;
                    if (VideoListMenu_SaveList(name))
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
                    }
                    else if (U64cmpU32(&freeSize, SAVELIST_MINIMAL_SPACE) < 0)
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csSAVESPACEFULL, MSGBOX_INFORMATION);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pVideoSaveListParm->msgbox, 1, ctFAILURE, csEMPTY_DONE, MSGBOX_INFORMATION);
                    }
                    MsgBox_SetDelay(&pVideoSaveListParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoSaveListParm->msgbox);
                    break;
               default:
                    break;
            }

            if (AK_TRUE == MsgboxFlag)
            {
                tickcount2 = Fwl_GetTickCount();
                if ((tickcount2 - tickcount1) < 1500)
                    Fwl_MiniDelay(1500 - (tickcount2 - tickcount1));
            }
            
        break;

        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_VIDEOPLAYER

static T_U32 VideoListMenu_GetListNo(T_U8 name)
{
    T_pFILE fp;
    T_USTR_FILE FilePath, Ustrtmp;
    T_STR_FILE FileName;
    T_U32 i = 1;
    T_BOOL MkDirRet = AK_TRUE;

    if (Fwl_FsIsDir((T_pCWSTR)Fwl_GetDefPath(eVIDEOLIST_PATH)) == AK_FALSE)
        MkDirRet = Fwl_CreateDefPath();

    if (AK_FALSE == MkDirRet)
        return 0;

    while (1)
    {
        if (i > 999999)
            i = 1;
        if(name == 1)
            sprintf(FileName, VIDEO_LIST_ALBUM_NAME, (unsigned int)i);
        else if(name == 2)
            sprintf(FileName, VIDEO_LIST_MOVIE_NAME, (unsigned int)i);
        else
            sprintf(FileName, VIDEO_LIST_FILM_NAME, (unsigned int)i);

        if (i > 1024)
            return 0;

        Utl_UStrCpy(FilePath, (T_pCWSTR)Fwl_GetDefPath(eVIDEOLIST_PATH));
        Eng_StrMbcs2Ucs(FileName, Ustrtmp);
        Utl_UStrCat(FilePath, Ustrtmp);
        if ((fp=Fwl_FileOpen(FilePath, _FMODE_READ, _FMODE_READ)) != _FOPEN_FAIL)
        {
            Fwl_FileClose(fp);
        }
        else if (Fwl_FsIsDir(FilePath))
        {
            
        }
        else
            break;
        i++;
    }

    return i;
}

static T_BOOL VideoListMenu_SaveList(T_U8 name)
{
    T_U32 ListNo;
    T_USTR_FILE FilePath, Ustrtmp;
    T_STR_FILE FileName;
    T_FILELIST FileList;

    if (pVideoSaveListParm->pIconExplorer == AK_NULL)
        return AK_FALSE;

    FileList_Init(&FileList, VIDEOLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE, AK_NULL);
    FileList_FromIconExplorer(&FileList, pVideoSaveListParm->pIconExplorer);

    if(AK_NULL == FileList.pItemHead)
    {
        FileList_Free(&FileList);
        return AK_FALSE;
    }

    ListNo = VideoListMenu_GetListNo(name);
    if (0 == ListNo)
    {
        FileList_Free(&FileList);
        return AK_FALSE;
    }

    if(name == 1)
        sprintf(FileName, VIDEO_LIST_ALBUM_NAME, (unsigned int)ListNo);
    else if(name == 2)
        sprintf(FileName, VIDEO_LIST_MOVIE_NAME, (unsigned int)ListNo);
    else
        sprintf(FileName, VIDEO_LIST_FILM_NAME, (unsigned int)ListNo);

    Utl_UStrCpy(FilePath, (T_pCWSTR)Fwl_GetDefPath(eVIDEOLIST_PATH));
    Eng_StrMbcs2Ucs(FileName, Ustrtmp);
    Utl_UStrCat(FilePath, Ustrtmp);

    if (FileList_SaveFileList(&FileList, FilePath) == AK_FALSE)
    {
        AK_DEBUG_OUTPUT("VideoListMenu_SaveList creat error\n\r");
        FileList_Free(&FileList);
        return AK_FALSE;
    }

    FileList_Free(&FileList);
    return AK_TRUE;
}


#endif


