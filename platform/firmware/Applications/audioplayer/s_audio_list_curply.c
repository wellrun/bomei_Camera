#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Eng_KeyMapping.h"
#include "Fwl_pfAudio.h"
#include "Eng_ImgConvert.h"
#include "Fwl_Initialize.h"
#include "Lib_state.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"
#include "ctl_medialist.h"


typedef struct {
    T_ICONEXPLORER  *pIconExplorer;
    T_MSGBOX        msgbox;
	T_U16			addId;
    T_S32			firstEmptyId;
} T_AUDIO_LIST_CURPLY_PARM;

static T_AUDIO_LIST_CURPLY_PARM  *pAudioListCurPly = AK_NULL;
static T_VOID refreshCurPlyList(T_VOID);

#endif
    


/*---------------------- BEGIN OF STATE s_audio_list_currentplay ------------------------*/
void initaudio_list_curply(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudioListCurPly = (T_AUDIO_LIST_CURPLY_PARM *)Fwl_Malloc(sizeof(T_AUDIO_LIST_CURPLY_PARM));
    AK_ASSERT_PTR_VOID(pAudioListCurPly, "initvideo_list(): pAudioListCurPly malloc error");
	memset(pAudioListCurPly, 0, sizeof(T_AUDIO_LIST_CURPLY_PARM));

    TopBar_EnableMenuButton();

    m_regResumeFunc(refreshCurPlyList);
#endif
}

void exitaudio_list_curply(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_FILELIST      FileList;
    T_RECT          msgRect;

//    AK_DEBUG_OUTPUT("exitaudio_list_curply1 gb.listchanged= %d", gb.listchanged);
    
    if (AK_TRUE == gb.listchanged)
    {
        FileList_Init(&FileList, AUDIOPLAYER_MAX_ITEM_QTY, FILELIST_SORT_NONE, AudioPlayer_IsSupportFile);
        FileList_FromIconExplorer(&FileList, pAudioListCurPly->pIconExplorer);
        if (FileList_SaveFileList(&FileList, _T(AUDIOLIST_CURTPLY_FILE)) != AK_TRUE)
        {
            MsgBox_InitStr(&pAudioListCurPly->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csCAMERA_FILE_SAVE_FAILED), MSGBOX_INFORMATION);
            MsgBox_Show(&pAudioListCurPly->msgbox);
            MsgBox_GetRect(&pAudioListCurPly->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
            Fwl_MiniDelay(2000);
        }
        FileList_Free(&FileList);
        gb.listchanged = AK_FALSE;
    }
/*
    if (AUDIOPLAYER_STATE_PLAY != AudioPlayer_GetCurState())
    {
        IconExplorer_Free(pAudioListCurPly->pIconExplorer);
    }
  */ 

    TopBar_DisableMenuButton();

    pAudioListCurPly->pIconExplorer = AK_NULL;
    pAudioListCurPly = Fwl_Free(pAudioListCurPly);
#endif
}

void paintaudio_list_curply(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(pAudioListCurPly->pIconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}


unsigned char handleaudio_list_curply(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;
    T_USTR_FILE	   pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
	T_ICONEXPLORER_ITEM *p;
    T_FILELIST      FileList;
	T_USTR_FILE     filename;
    T_USTR_FILE     path;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(pAudioListCurPly->pIconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_CURPLAY == event)
    {
        pAudioListCurPly->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
        
        if (AUDIOPLAYER_STATE_PLAY == AudioPlayer_GetCurState())
        {
            m_triggerEvent(M_EVT_NEXT, pEventParm);
        }
        else 
        {
            if (AK_FALSE != IconExplorer_DelAllItem(pAudioListCurPly->pIconExplorer))
            {
				if (!Fwl_FileExist(_T(AUDIOLIST_CURTPLY_FILE)))
				{
					Ctl_MList_ToIconExplorerComplete(pAudioListCurPly->pIconExplorer, &pAudioListCurPly->addId, &pAudioListCurPly->firstEmptyId, eMEDIA_LIST_AUDIO);

					if (pAudioListCurPly->addId > 0)
					{
						gb.listchanged = AK_TRUE;
					}
				}
                else
                {
                	if (!FileList_Init(&FileList, AUDIOPLAYER_MAX_ITEM_QTY, FILELIST_SORT_NONE,  AudioPlayer_IsSupportFile))
	                {
	                    AK_DEBUG_OUTPUT("\nhandleaudio_list_curply():FileList_Init() fail!!!\n");
	                    return 0;
	                }
					
                	if (FileList_Add(&FileList, _T(AUDIOLIST_CURTPLY_FILE), FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) == FILELIST_ADD_ERROR)
                	{
						AK_DEBUG_OUTPUT("\nread data fail\n");
                	}
					
					FileList_ToIconExplorer(&FileList, pAudioListCurPly->pIconExplorer, eINDEX_TYPE_AUDIO, AK_NULL);
					FileList_Free(&FileList);
                }
                
                refreshCurPlyList();
            }
            else
            {
                AK_DEBUG_OUTPUT("\nhandleaudio_list_curply():delete all item fail");
            }
        }
    }

    IconExplorerRet = IconExplorer_Handler(pAudioListCurPly->pIconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
        case eNext:
        	p = IconExplorer_GetItemFocus(pAudioListCurPly->pIconExplorer);
            if (AK_NULL != p)
            {
                pcontent = (T_INDEX_CONTENT *)p->pContent;
                if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				// get text pointer
				Utl_USplitFilePath(pFilePath, path, filename);
				if (0 != Utl_UStrCmpN(filename, (T_U16 *)AudioPlayer_GetFocusName(), (T_U16)Utl_UStrLen(AudioPlayer_GetFocusName())))
				{
					memset(&pFilePath[0], 0, sizeof(pFilePath));
				}

				if (0 != pFilePath[0])
				{
	                if ((AUDIOPLAYER_STATE_PLAY == AudioPlayer_GetCurState() \
	                    && !AudioPlayer_IsPlayingFile(pFilePath)))
	                {
	                	AK_DEBUG_OUTPUT("AudioPlayer Current State: %d.\n", AudioPlayer_GetCurState());
	                    AudioPlayer_Stop();
	                }

	                pEventParm->p.pParam1 = pAudioListCurPly->pIconExplorer;
	                m_triggerEvent(M_EVT_NEXT, pEventParm);
				}
				else
				{
					IconExplorer_DelItemFocus(pAudioListCurPly->pIconExplorer);
					MsgBox_InitAfx(&pAudioListCurPly->msgbox, 1, ctFAILURE, csNOT_IN_AUDIOLIST, MSGBOX_INFORMATION);
					MsgBox_SetDelay(&pAudioListCurPly->msgbox, MSGBOX_DELAY_1);
            		m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioListCurPly->msgbox);
					gb.listchanged = AK_TRUE;
				}
            }
            break;
        case eMenu:
                IconExplorer_SetRefresh(pAudioListCurPly->pIconExplorer, ICONEXPLORER_REFRESH_ALL);
                pEventParm->p.pParam1 = pAudioListCurPly->pIconExplorer;
                pEventParm->p.pParam3 = (T_pVOID)LTP_CURPLY;
                GE_ShadeInit();
                m_triggerEvent(M_EVT_MENU, pEventParm);
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

static T_VOID refreshCurPlyList(T_VOID)
{

    TopBar_EnableMenuButton();

    IconExplorer_SetTitleText(pAudioListCurPly->pIconExplorer, Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_CURRENTPLAY), ICONEXPLORER_TITLE_TEXTCOLOR);
    IconExplorer_SetRefresh(pAudioListCurPly->pIconExplorer, ICONEXPLORER_REFRESH_ALL);

       
}
#endif

