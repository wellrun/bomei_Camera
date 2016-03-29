
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER

#include "Fwl_Image.h"
#include "Eng_KeyMapping.h"
#include "Fwl_pfAudio.h"
#include "Eng_ImgConvert.h"
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "Eng_Math.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Ctl_APlayerList.h"




typedef struct {
    T_ICONEXPLORER      IconExplorer;
    T_ICONEXPLORER      *pIconExplorer;
    T_MSGBOX            msgbox;
    T_USTR_FILE         CurFilePath;
} T_AUDIO_ADDTO_MYLIST_PARM;

static T_AUDIO_ADDTO_MYLIST_PARM  *pAudio_AddTo_MyList_Parm = AK_NULL;

static T_VOID Audio_AddListToList(T_VOID);
#endif

void initaudio_add_list_to_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_FILELIST      FileList;
    T_USTR_FILE     Ustrtmp;
    T_FILELIST_ITEM *p;
    T_S32           id;

    pAudio_AddTo_MyList_Parm = (T_AUDIO_ADDTO_MYLIST_PARM *)Fwl_Malloc(sizeof(T_AUDIO_ADDTO_MYLIST_PARM));
    AK_ASSERT_PTR_VOID(pAudio_AddTo_MyList_Parm, "initvideo_list(): malloc error");

    if (FileList_Init(&FileList, MYLIST_MAX_ITEM_QTY, FILELIST_SORT_NAME,  AudioPlayer_IsSupportListFile) == AK_FALSE)
    {
        AK_DEBUG_OUTPUT("\nfile list init fail\n");
        return;
    }

    if (FileList_Add_Alt(&FileList, Fwl_GetDefPath(eAUDIOLIST_PATH), FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) == FILELIST_ADD_ERROR)
    {
        AK_DEBUG_OUTPUT("\nfile list is inexist or read fail\n");
    }

    MenuStructInit(&pAudio_AddTo_MyList_Parm->IconExplorer);
    IconExplorer_SetTitleText(&pAudio_AddTo_MyList_Parm->IconExplorer, Res_GetStringByID(eRES_STR_AUDIOPLAYER_ADD_LIST_TO_MYLIST), ICONEXPLORER_TITLE_TEXTCOLOR);
    //FileList_ToIconExplorer(&FileList, &pAudioListAll->IconExplorer);
    
    p = FileList.pItemHead;
    while (p != AK_NULL)
    {
        Utl_UStrCpy(Ustrtmp,Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_MYPLYLST));
        id = p->pText[7];
        Utl_UStrCatChr(Ustrtmp,(T_WCHR)id,(T_S16)1);     // because file name is "MyList_%d.ALT"
        IconExplorer_AddItemWithOption(&pAudio_AddTo_MyList_Parm->IconExplorer, id, p->pFilePath, (Utl_UStrLen(p->pFilePath)+1) << 1, \
            Ustrtmp, AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
        
        p = p->pNext;
    }
    IconExplorer_SetSortMode(&pAudio_AddTo_MyList_Parm->IconExplorer,ICONEXPLORER_SORT_ID);

    FileList_Free(&FileList);
#endif
}

void exitaudio_add_list_to_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudio_AddTo_MyList_Parm->IconExplorer);
    pAudio_AddTo_MyList_Parm = Fwl_Free(pAudio_AddTo_MyList_Parm);
#endif
}

void paintaudio_add_list_to_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudio_AddTo_MyList_Parm->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_add_list_to_mylist(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;
    T_USTR_FILE		pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudio_AddTo_MyList_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_4 == event)
    {
        if (0 == IconExplorer_GetItemQty(&pAudio_AddTo_MyList_Parm->IconExplorer))
        {
            MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 2, ctHINT, csEMPTY_FAVORITE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudio_AddTo_MyList_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_AddTo_MyList_Parm->msgbox);

            return 0;
        }
        else
        {
        	GE_ShadeInit();
	        pAudio_AddTo_MyList_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
            pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pAudio_AddTo_MyList_Parm->pIconExplorer);

            if (AK_NULL != pcontent)
			{
				MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
			}
			
            if (0 == pFilePath[0])
            {
                AK_DEBUG_OUTPUT("\npFilePath == AK_NULL\n");
            }
        }
    }

    IconExplorerRet = IconExplorer_Handler(&pAudio_AddTo_MyList_Parm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
        case eNext:
            if (IconExplorer_GetItemFocus(&pAudio_AddTo_MyList_Parm->IconExplorer) != AK_NULL)
            {
            	pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pAudio_AddTo_MyList_Parm->pIconExplorer);

	            if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}
				
	            if (0 == pFilePath[0])
	            {
	                AK_DEBUG_OUTPUT("\npFilePath == AK_NULL\n");
                    break;
	            }
               
               Audio_AddListToList();
            }
            break;

        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_AUDIOPLAYER

static T_VOID Audio_AddListToList(T_VOID)
{
    T_RECT              msgRect;
    T_FILELIST          FileList;
    T_USTR_FILE         FilePath;
    T_pWSTR             pListFilePath;
    T_FILELIST_ADD_RET  ret;
    T_BOOL              ret_sav;
    T_U64_INT           freeSize;

    pListFilePath   = AK_NULL;
    ret  = FILELIST_ADD_ERROR;
    ret_sav = AK_FALSE;

    MsgBox_InitStr(&pAudio_AddTo_MyList_Parm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
    MsgBox_Show(&pAudio_AddTo_MyList_Parm->msgbox);
    MsgBox_GetRect(&pAudio_AddTo_MyList_Parm->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

    Utl_UStrCpy(FilePath, Fwl_GetDefPath(eAUDIOLIST_PATH));
    Fwl_FsGetFreeSize(FilePath[0], &freeSize);
    if (U64cmpU32(&freeSize, 2048) < 0)
    {
        MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctFAILURE, csEXPLORER_FREE_SIZE, MSGBOX_INFORMATION);
    }
    else
    {
        FileList_Init(&FileList, MYLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE, AudioPlayer_IsSupportFile);
        pListFilePath = (T_pWSTR)IconExplorer_GetItemContentFocus(&pAudio_AddTo_MyList_Parm->IconExplorer);
        ret = FileList_Add(&FileList, pListFilePath, FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER);
		
        if (FILELIST_ADD_ERROR != ret 
			&& !(FILELIST_ADD_NOSPACE == ret && FileList.ItemQty <= FileList.ItemQtyMax))	//have no memory,  Filses add to list Not finish
        {
            ret = FileList_FromIconExplorer(&FileList, pAudio_AddTo_MyList_Parm->pIconExplorer);	//Add item to list from IconExplorer

			// FileList.ItemQty equals Max num 
			if ((FILELIST_ADD_NOSPACE == ret && FileList.ItemQty > FileList.ItemQtyMax)	
				|| FILELIST_ADD_SUCCESS == ret) 
			{
				//save list to mylist%.alt
                ret_sav = FileList_SaveFileList(&FileList, pListFilePath);	
            }
        }
        FileList_Free(&FileList);

        if (FILELIST_ADD_SUCCESS == ret)
        {
	        if (ret_sav)
     	    	MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
			else
				MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctSUCCESS, csSPACE_EMPTY, MSGBOX_INFORMATION);
        }
        else if (FILELIST_ADD_NOSPACE == ret )
        {	
        	if (ret_sav)
				MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctFAILURE, csMP3_NOT_ENOUGH_SPACE, MSGBOX_INFORMATION);
			else
				MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctFAILURE, csSPACE_EMPTY, MSGBOX_INFORMATION);
        }
        else if (FILELIST_ADD_OUTPATHDEEP == ret)
    	{
        	MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctFAILURE, csOUT_PATH_DEEP, MSGBOX_INFORMATION);
    	}
        else // (AK_FALSE == ret_sav) || (FILELIST_ADD_ERROR == ret)
        {
            MsgBox_InitAfx(&pAudio_AddTo_MyList_Parm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        }
    }
	
    MsgBox_SetDelay(&pAudio_AddTo_MyList_Parm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_AddTo_MyList_Parm->msgbox);
//     DisplayList_SetRefresh(&pAudio_AddTo_MyList_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);

}

#endif


