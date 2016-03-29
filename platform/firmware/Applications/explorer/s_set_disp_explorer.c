#include "Fwl_public.h"
#include "Ctl_DisplayList.h"
#include "Ctl_AudioPlayer.h"
#ifdef SUPPORT_EXPLORER
#include <time.h>
#include <string.h>
#include "Fwl_pfKeypad.h"
#include "Ctl_Msgbox.h"
#include "Ctl_ImgBrowse.h"
#include "Ctl_Ebook.h"
#include "Ctl_AVIPlayer.h"
#include "Ctl_Fm.h"
#include "Fwl_Initialize.h"
#include "Eng_Alarm.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "Lib_state_api.h"
#include "Fwl_pfdisplay.h"
#include "fwl_display.h"
#include "Fwl_pfAudio.h"

typedef struct {
    T_DISPLAYLIST           displayList;
    T_MSGBOX                msgbox;
    T_USTR_FILE     filename;
	T_U32			ItemShowId;
} T_EXPLORER_PARM;

static T_EXPLORER_PARM *pExplorerParm = AK_NULL;

static T_BOOL   bNeedChgFrq = AK_FALSE;

T_VOID explorer_send2audio(T_DISPLAYLIST *pDisplayList);
T_VOID explorer_send2video(T_DISPLAYLIST *pDisplayList);
static T_VOID explorer_resume(T_VOID);

/*---------------------- BEGIN OF STATE s_set_disp_explorer ------------------------*/

static T_VOID Explr_PlayMp4(T_USTR_FILE FilePath, T_EVT_PARAM *pEventParm)
{
	T_AVIPLAY_ERR aviPlayChk;
	aviPlayChk = AVIPlayer_CheckVideoFile(FilePath);
	if (AVIPLAY_OK == aviPlayChk)
	{
		 // mp4 file
		AudioPlayer_Stop();
		explorer_send2video(&pExplorerParm->displayList);
		m_triggerEvent(M_EVT_2, pEventParm);
	}
	else if (AVIPLAY_OPENERR == aviPlayChk)
	{
		// audio file						 
		if(AudioPlayer_IsSupportFile(FilePath))
		{
#ifdef SUPPORT_AUDIOPLAYER
			AudioPlayer_Stop();
			explorer_send2audio(&pExplorerParm->displayList);
			m_triggerEvent(M_EVT_1, pEventParm);
#endif
		}
		else
		{
			MsgBox_InitAfx(&pExplorerParm->msgbox, 1, ctFAILURE, csFILE_INVALID, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&pExplorerParm->msgbox, MSGBOX_DELAY_1);
			m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerParm->msgbox);
		}
	}
}

static T_VOID Explr_OpenFile(T_USTR_FILE FilePath, T_EVT_PARAM *pEventParm, T_FILE_INFO *pFileInfo, T_U8 FileType)
{
	if (AudioPlayer_IsSupportFile(FilePath))    //audio
    {
#ifdef SUPPORT_AUDIOPLAYER
        AudioPlayer_Stop();
        explorer_send2audio(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_1, pEventParm);
#endif
    }
#ifdef SUPPORT_VIDEOPLAYER
    else if (AVIPlayer_IsSupportFileType(FilePath))//movie
    {
        AudioPlayer_Stop();
        explorer_send2video(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_2, pEventParm);
    }
#endif

#ifdef SUPPORT_IMG_BROWSE
    else if (ImgBrowse_IsBrowseSupportFileType(pFileInfo->name))//image
    {
        bNeedChgFrq = AK_TRUE;

        pEventParm = (T_EVT_PARAM *)(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_3, pEventParm);
    }
#endif

#ifdef SUPPORT_EBK
    else if (EBCtl_IsSupportFileType(pFileInfo->name))//ebook
    {
    	pExplorerParm->ItemShowId = IconExplorer_GetItemShowId(&pExplorerParm->displayList.IconExplorer);
        pEventParm = (T_EVT_PARAM *)(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_4, pEventParm);
    }
#endif

#ifdef SUPPORT_EMAP
    else if (ImgBrowse_IsEmapSupportFileType(pFileInfo->name))
    {
        bNeedChgFrq = AK_TRUE;
        pEventParm = (T_EVT_PARAM *)(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_5, pEventParm);
    }
#endif

#ifdef INSTALL_GAME_NES
    else if (FILE_TYPE_NES == FileType)
    {
         //pEventParm = (T_EVT_PARAM *)(&pExplorerParm->displayList);
        Utl_UStrCpy(pExplorerParm->filename, DisplayList_GetCurFilePath(&pExplorerParm->displayList));
        Utl_UStrCat(pExplorerParm->filename, pFileInfo->name);
        pEventParm->p.pParam1 = pExplorerParm->filename;
        pEventParm->p.pParam2 = DisplayList_GetCurFilePath(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_6, pEventParm);
    }
#endif

#if (SDRAM_MODE >= 16)
#ifdef INSTALL_GAME_SFC
    else if (FILE_TYPE_SNES == FileType)
    {
        Utl_UStrCpy(pExplorerParm->filename, DisplayList_GetCurFilePath(&pExplorerParm->displayList));
        Utl_UStrCat(pExplorerParm->filename, pFileInfo->name);
        pEventParm->p.pParam1 = pExplorerParm->filename;
        pEventParm->p.pParam2 = DisplayList_GetCurFilePath(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_7, pEventParm);
    }
#endif

#ifdef INSTALL_GAME_GBA
    else if (FILE_TYPE_GBA == FileType)
    {
        Utl_UStrCpy(pExplorerParm->filename, DisplayList_GetCurFilePath(&pExplorerParm->displayList));
        Utl_UStrCat(pExplorerParm->filename, pFileInfo->name);
        pEventParm->p.pParam1 = pExplorerParm->filename;
        pEventParm->p.pParam2 = DisplayList_GetCurFilePath(&pExplorerParm->displayList);
        m_triggerEvent(M_EVT_8, pEventParm);
    }
#endif
#endif

#ifdef INSTALL_GAME_MD
    else if (FILE_TYPE_MD == FileType)
    {
		Utl_UStrCpy(pExplorerParm->filename, DisplayList_GetCurFilePath(&pExplorerParm->displayList));
		Utl_UStrCat(pExplorerParm->filename, pFileInfo->name);
		pEventParm->p.pParam1 = pExplorerParm->filename;
		pEventParm->p.pParam2 = DisplayList_GetCurFilePath(&pExplorerParm->displayList);
		m_triggerEvent(M_EVT_9, pEventParm);
    }
#endif

#ifdef SUPPORT_FLASH
		else if (FILE_TYPE_FLASH == FileType)
		{
			m_triggerEvent(M_EVT_10, (T_EVT_PARAM *)&pExplorerParm->displayList);
		}
#endif

	// Unsupport File Type Or File Error
	else
    {
    	MsgBox_InitAfx(&pExplorerParm->msgbox, 1, ctFAILURE, csFILE_INVALID, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pExplorerParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerParm->msgbox);
    }

}
#endif
void initset_disp_explorer(void)
{
#ifdef SUPPORT_EXPLORER

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_ALL,
        FILE_TYPE_NONE
    };

    //gs.ImgSlideInterval = (T_U8)gb.nExploreSlideMode;

    bNeedChgFrq = AK_FALSE;    
//  FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_HIGH);
    
    pExplorerParm = (T_EXPLORER_PARM *)Fwl_Malloc(sizeof(T_EXPLORER_PARM));
    AK_ASSERT_PTR_VOID(pExplorerParm, "initset_disp_explorer error");

    DisplayList_init(&pExplorerParm->displayList, AK_NULL, \
            GetCustomString(csEXPLORER_TITLE), FileType);

    gb.bInExplorer = AK_TRUE;

	pExplorerParm->ItemShowId = 0;

    m_regResumeFunc(explorer_resume);
#endif
}

void exitset_disp_explorer(void)
{
#ifdef SUPPORT_EXPLORER

    //gb.nExploreSlideMode = gs.ImgSlideInterval;
    
    gb.bInExplorer = AK_FALSE;

    TopBar_DisableMenuButton();

    DisplayList_Free(&pExplorerParm->displayList);
    pExplorerParm = Fwl_Free(pExplorerParm);

    if(bNeedChgFrq)
    {
        bNeedChgFrq = AK_FALSE;
    }

    if ((AudioPlayer_GetCurState()!=AUDIOPLAYER_STATE_BACKGROUNDPLAY))
    {
       Fwl_AudioDisableDA();
    }
#endif
}

void paintset_disp_explorer(void)
{
#ifdef SUPPORT_EXPLORER

    //AK_DEBUG_OUTPUT("paintset_disp_explorer1 , itemQty =%d ,pIconExplorer->ItemListFlag=%d" , pExplorerParm->displayList.IconExplorer.ItemQty,pExplorerParm->displayList.IconExplorer.ItemListFlag);
    DisplayList_SetTopBarMenuIconState(&pExplorerParm->displayList);
    DisplayList_Show(&pExplorerParm->displayList);

    GE_StartShade();
    Fwl_RefreshDisplay();
    //AK_DEBUG_OUTPUT("paintset_disp_explorer2 , itemQty =%d ,pIconExplorer->ItemListFlag=%d" , pExplorerParm->displayList.IconExplorer.ItemQty,pExplorerParm->displayList.IconExplorer.ItemListFlag);
#endif
}

unsigned char handleset_disp_explorer(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_EXPLORER

    T_eBACK_STATE DisplayListRet;
  //  T_MMI_KEYPAD phyKey;
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_U8 FileType = _SD_MEDIA_TYPE_UNKNOWN;
    T_USTR_FILE     FilePath;
    //T_AVIPLAY_ERR aviPlayChk=0;
 //   T_ALARM_TYPE *pAlarmClock = &(gs.AlarmClock);
#ifdef SUPPORT_EBK
    T_ICONEXPLORER_ITEM *pTmpItem = AK_NULL;
    T_U16  *pTmpName  = AK_NULL;
    T_U32   i = 0;
    T_S32  nCmpRet = 0;
	T_U32 Id = 0;
#endif
    //T_S8    uzStr1[512], uzStr2[512];

    //AK_DEBUG_OUTPUT("handleset_disp_explorer event=%d, M_EVT_EXIT=%d",event,M_EVT_EXIT);

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pExplorerParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

    //check the txt name from s_ebk_view sm
    if(M_EVT_EXIT == event)
    {
        if(AK_NULL != pEventParm)
        {   
        #ifdef SUPPORT_EBK
            if(EBCtl_IsSupportFileType((T_U16 *)pEventParm))
            {
                pTmpItem = pExplorerParm->displayList.IconExplorer.pItemHead;
                // AK_DEBUG_OUTPUT("file type is txt3");
                for (i=0; i < pExplorerParm->displayList.IconExplorer.ItemQty; i++) 
                {
                    //AK_DEBUG_OUTPUT("file type is txt4");
                    pTmpName = ((T_FILE_INFO *)pTmpItem->pContent)->name;
                    //AK_DEBUG_OUTPUT("file type is txt5");
                    nCmpRet = Utl_UStrCmp(pTmpName, (T_U16 *)pEventParm); 


					Id = pTmpItem->Id;

					if (Id == pExplorerParm->ItemShowId)
					{
						pExplorerParm->displayList.IconExplorer.pItemShow = pTmpItem; 
					}
                    
                    if(0 == nCmpRet)
                    {       
                        pExplorerParm->displayList.IconExplorer.pItemOldFocus = pExplorerParm->displayList.IconExplorer.pItemFocus;
                        pExplorerParm->displayList.IconExplorer.pItemFocus = pTmpItem; 
                        IconExplorer_SetRefresh(&(pExplorerParm->displayList.IconExplorer),DISPLAYLIST_REFRESH_ALL);
                        break;
                    }  

                    if(AK_NULL != pTmpItem->pNext)
                    {
                        pTmpItem = pTmpItem->pNext;
                    }
                    else
                    {
                        break;
                    }
                    
                }
                Fwl_Free(pEventParm);
                pEventParm = AK_NULL;                
            }
		#endif
        }             

    }
    

    DisplayListRet = DisplayList_Handler(&pExplorerParm->displayList, event, pEventParm);
    switch (DisplayListRet)
    {
    case eNext:
        DisplayList_SetRefresh(&pExplorerParm->displayList, DISPLAYLIST_REFRESH_ALL);
        pFileInfo = DisplayList_Operate(&pExplorerParm->displayList);
        if (pFileInfo != AK_NULL)
        {
            if (AK_FALSE == Utl_IsLegalFname(pFileInfo->name))
            {
                MsgBox_InitAfx(&pExplorerParm->msgbox, 1, ctFAILURE, csFILE_INVALID, MSGBOX_INFORMATION);
                MsgBox_SetDelay(&pExplorerParm->msgbox, MSGBOX_DELAY_1);
                m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerParm->msgbox);
                break;
            }

            // special deal with mp4 postfix, this file will be audio or movie file
            FileType = Utl_GetFileType(pFileInfo->name);
            Fwl_Print(C3, M_EXPLORER, "FileType: %d", FileType);
			Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(&pExplorerParm->displayList));
            Utl_UStrCat(FilePath, pFileInfo->name);
			
            if (FILE_TYPE_MP4 == FileType)
            {
				Explr_PlayMp4(FilePath, pEventParm);
            }
            else
            {
				Explr_OpenFile(FilePath, pEventParm, pFileInfo, FileType);
            }
        }

        break;
        
    case eMenu:
        pFileInfo = DisplayList_GetItemContentFocus(&pExplorerParm->displayList);
        if ((DisplayList_GetSubLevel(&pExplorerParm->displayList) > 0) && pFileInfo)
        {
            if (!(((pFileInfo->attrib&0x10) == 0x10) \
                && (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)))
            {
                TopBar_DisableMenuButton();
                GE_ShadeInit();
                m_triggerEvent(M_EVT_NEXT, (T_EVT_PARAM *)&pExplorerParm->displayList);
                DisplayList_SetRefresh(&pExplorerParm->displayList, DISPLAYLIST_REFRESH_ALL);
            }
        }
        break;
/*
        case eOption:
            pFileInfo = DisplayList_GetItemContentFocus(&pExplorerParm->displayList);
            if ((DisplayList_GetSubLevel(&pExplorerParm->displayList) > 0) && pFileInfo)
            {
                if (((pFileInfo->attrib&0x10) == 0x10) \
                    && (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0))
                {
                    TopBar_DisableMenuButton();
                    DisplayList_SetRefresh(&pExplorerParm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
                else
                {
                    TopBar_EnableMenuButton();
                    DisplayList_SetRefresh(&pExplorerParm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
            }
            break;
   */
    default:
        ReturnDefauleProc(DisplayListRet, pEventParm);
        break;
    }
#endif
    return 0;
}

T_VOID explorer_send2audio(T_DISPLAYLIST *pDisplayList)
{
    T_USTR_FILE ImportPath, Ustrtmp;
    T_FILE_INFO *pFileInfo;
    T_FILELIST FileList;

    pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
    Utl_UStrCpy(ImportPath, DisplayList_GetCurFilePath(pDisplayList));
    Utl_UStrCat(ImportPath, pFileInfo->name);

    FileList_Init(&FileList, AUDIOPLAYER_MAX_ITEM_QTY, FILELIST_SORT_NONE, AudioPlayer_IsSupportFile);
    FileList_Add(&FileList, ImportPath, FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER);
    Eng_StrMbcs2Ucs(AUDIOLIST_TMP_FILE, Ustrtmp);
    FileList_SaveFileList(&FileList, Ustrtmp);
    FileList_Free(&FileList);
}

#ifdef SUPPORT_EXPLORER

T_VOID explorer_send2video(T_DISPLAYLIST *pDisplayList)
{
    T_USTR_FILE ImportPath, Ustrtmp;
    T_FILE_INFO *pFileInfo;
    T_FILELIST FileList;

    pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
    Utl_UStrCpy(ImportPath, DisplayList_GetCurFilePath(pDisplayList));
    Utl_UStrCat(ImportPath, pFileInfo->name);

    FileList_Init(&FileList, VIDEOLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE, AVIPlayer_IsSupportFileType);
    FileList_Add(&FileList, ImportPath, FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER);
    Eng_StrMbcs2Ucs(VIDEOLIST_TMP_FILE, Ustrtmp);
    FileList_SaveFileList(&FileList, Ustrtmp);
    FileList_Free(&FileList);
}

static T_VOID explorer_resume(T_VOID)
{
    T_ICONEXPLORER *pIconExplorer = AK_NULL;
    T_FILE_INFO *pFileInfo = AK_NULL;

    if(bNeedChgFrq )
    {
        bNeedChgFrq = AK_FALSE;
    }
    
    pIconExplorer = &pExplorerParm->displayList.IconExplorer;
    if (AK_TRUE == pIconExplorer->ScrollBarFlag)
    {
        ScBar_LoadImageData(&pIconExplorer->ScrollBar);
    }
    
    if(AK_NULL != pIconExplorer)
    {
//        AK_DEBUG_OUTPUT("explorer_resume1 , itemQty =%d ,pIconExplorer->ItemListFlag=%d" , pIconExplorer->ItemQty,pIconExplorer->ItemListFlag);
        IconExplorer_CheckItemList(pIconExplorer);
//        AK_DEBUG_OUTPUT("explorer_resume2 , itemQty =%d ,pIconExplorer->ItemListFlag=%d" , pIconExplorer->ItemQty,pIconExplorer->ItemListFlag);
    }
   
    pFileInfo = DisplayList_GetItemContentFocus(&pExplorerParm->displayList);
    if ((DisplayList_GetSubLevel(&pExplorerParm->displayList) > 0) && pFileInfo)
    {
        if (!(((pFileInfo->attrib&0x10) == 0x10) \
        && (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)))
        {
            TopBar_EnableMenuButton();
        }
    }
    else
    {
        TopBar_DisableMenuButton();
    }
}
#endif
