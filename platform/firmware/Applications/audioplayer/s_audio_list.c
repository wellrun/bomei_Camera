
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
#include "Eng_Math.h"
#include "Ctl_APlayerList.h"
#include "svc_medialist.h"
#include "ctl_medialist.h"
#include "ctl_slipmgr.h"


typedef struct
{
    T_ICONEXPLORER      IconExplorer;
    T_ICONEXPLORER      *pGIconExplorer;/* valid in audio player module	*/
    T_ICONEXPLORER      *pIconExplorer; /* for my favorite				*/
    T_LIST_TYPE         BaseListType;   /* the frist list type   		*/
    T_LIST_TYPE         CurListType;    /* the current list type 		*/ 
    T_LIST_CLASS_INFO   ClassInfo;		/* information for class  		*/
    T_MSGBOX            msgbox;			/* msgbox for prompt       		*/
    T_U32				ItemQty;        /* item quanitiy				*/
    T_FILELIST			mFileList;		/* list of audiolist.vlt    	*/
    T_U16				addId;
	T_U16				searchCnt[MAX_MEDIA_NUM];
    T_S32				firstHoleId;
} T_AUDIO_LIST_PARM;

#define UPDATE_TIMER_INTERVAL  		200 //200ms

static T_AUDIO_LIST_PARM *pAudioList = AK_NULL;



// flag of update audio lib, it will be seting if S_audio_update.c
T_BOOL fUpdateAudioLib = AK_FALSE;

static T_VOID AudioList_SetTitle(T_VOID);
static T_BOOL AudioList_LoadFavoriteList(T_ICONEXPLORER* pIconExplorer);
static T_VOID AudioList_SetClassInfo(T_U16 *pStr);
static T_VOID AudioList_refresh(T_VOID);

static T_VOID AudioList_ModifyCurrentListType(T_BOOL flag);
static T_VOID AudioList_ModifyClassInfo(T_VOID);
static T_VOID AudioList_FreeClassInfo(T_CLASSINFO *pClassInfo);
static T_VOID resume_audio_list(T_VOID);
static T_VOID suspend_audio_list(T_VOID);

static unsigned char AudioList_HandleInitEvt(T_EVT_PARAM* pEventParm)
{
	unsigned char ret = 0;

	pAudioList->pGIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
    pAudioList->BaseListType = (T_LIST_TYPE)(T_U32)pEventParm->p.pParam3;
    pAudioList->CurListType = pAudioList->BaseListType;

    if (LTP_TITLE 			== pAudioList->BaseListType
		|| LTP_OFTENPLAY	== pAudioList->BaseListType
        || LTP_RECEAPPEND	== pAudioList->BaseListType
        || LTP_RECEPLAY		== pAudioList->BaseListType)
    {
        TopBar_EnableMenuButton();
    }

	// ÊÕ²Ø¼Ð
    if (LTP_FAVORITE == pAudioList->BaseListType)
    {
        //load from d:/audio/alt/
        pAudioList->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam2;  
        AudioList_LoadFavoriteList(pAudioList->pIconExplorer);
        pAudioList->ItemQty = IconExplorer_GetItemQty(&pAudioList->IconExplorer);
        
        if (0 == pAudioList->ItemQty)
        {
            MsgBox_InitAfx(&pAudioList->msgbox, 2, ctHINT, csEMPTY_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudioList->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioList->msgbox);

            return 1;
        }
    }
    else
    {
		MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_NEW);
		AudioList_SetTitle();
    }
	return ret;
}

static T_VOID AudioList_SaveListFromExplorer(T_pCWSTR pFilePath)
{
	T_FILELIST      FileList;
	T_RECT          msgRect;
    T_U64_INT       freeSize;
   		
	Fwl_FsGetFreeSize(pFilePath[0], &freeSize);
    // check space 
    if (U64cmpU32(&freeSize, SAVELIST_MINIMAL_SPACE) < 0)
    {
        MsgBox_InitAfx(&pAudioList->msgbox, 0, ctFAILURE, csEXPLORER_FREE_SIZE, MSGBOX_INFORMATION);
		MsgBox_Show(&pAudioList->msgbox);
        MsgBox_GetRect(&pAudioList->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(2000);

		return;
    }
        	
    if (!FileList_Init(&FileList, MYLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE,  AudioPlayer_IsSupportFile))
    {
        AK_DEBUG_OUTPUT("exitaudio_list(): FileList_Init fail!!!\n");
		return;
    }
            
    // push item of explorer into FileList
    FileList_FromIconExplorer(&FileList, &pAudioList->IconExplorer);
    // save FileList
    if (!FileList_SaveFileList(&FileList, pFilePath))
    {
        MsgBox_InitStr(&pAudioList->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csCAMERA_FILE_SAVE_FAILED), MSGBOX_INFORMATION);
        MsgBox_Show(&pAudioList->msgbox);
        MsgBox_GetRect(&pAudioList->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(2000);
    }
    
    FileList_Free(&FileList);  
}

static T_VOID AudioList_update_list(T_VOID)
{
	if (AK_NULL == pAudioList)
	{
		return;
	}

	if (SLIPMSG_STA_STOP != SlipMgr_GetCurStatus(pAudioList->IconExplorer.pSlipMgr))
	{
		return;
	}

	AK_Feed_Watchdog(0);

	if (MList_IsAdding(eMEDIA_LIST_AUDIO))
	{
		if (MList_GetDelFlag(eMEDIA_LIST_AUDIO))
		{
			MList_SetDelFlag(eMEDIA_LIST_AUDIO, AK_FALSE);
			pAudioList->addId = 0;
			pAudioList->firstHoleId = MLIST_HOLE_NONE;
			memset(pAudioList->searchCnt, 0, MAX_MEDIA_NUM * sizeof(T_U16));
			IconExplorer_DelAllItem(&pAudioList->IconExplorer);
		}
		
		if (LTP_TITLE == pAudioList->BaseListType)
		{
			if (MLIST_HOLE_NONE != pAudioList->firstHoleId)
			{
				pAudioList->addId = 0;
				pAudioList->firstHoleId = MLIST_HOLE_NONE;
				IconExplorer_DelAllItem(&pAudioList->IconExplorer);
			}
			
			Ctl_MList_ToIconExplorerStep(&pAudioList->IconExplorer, &pAudioList->addId, eMEDIA_LIST_AUDIO);
		}
		else if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
               		&&(pAudioList->CurListType>=LTP_GENRE && pAudioList->CurListType<=LTP_COMPOSER))
		{
			if (MLIST_HOLE_NONE != pAudioList->firstHoleId)
			{
				pAudioList->addId = 0;
				pAudioList->firstHoleId = MLIST_HOLE_NONE;
				IconExplorer_DelAllItem(&pAudioList->IconExplorer);
			}
			
			Ctl_MList_ID3_ToIconExplorerStep(&pAudioList->IconExplorer, &pAudioList->addId, pAudioList->BaseListType-LTP_GENRE);
		}
		else if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
           		&&(LTP_TITLE == pAudioList->CurListType))
		{
			switch(pAudioList->BaseListType)
			{
			case LTP_GENRE:
				Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
					pAudioList->searchCnt, pAudioList->ClassInfo.ClassInfo.pGenre, eID3_TAGS_GENRE);
				break;
			case LTP_ARTIST:
				Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
					pAudioList->searchCnt, pAudioList->ClassInfo.ClassInfo.pArtist, eID3_TAGS_ARTIST);
				break;
			case LTP_ALBUM:
				Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
					&pAudioList->addId, pAudioList->ClassInfo.ClassInfo.pAlbum, eID3_TAGS_ALBUM);
				break;
			case LTP_COMPOSER:
				Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
					&pAudioList->addId, pAudioList->ClassInfo.ClassInfo.pComposer, eID3_TAGS_COMPOSER);
				break;
			default:
				break;
			}
		}	
		else if (LTP_OFTENPLAY == pAudioList->BaseListType 
			|| LTP_RECEPLAY == pAudioList->BaseListType)
		{
			Ctl_MList_ToIEByPlayInfoComplete(&pAudioList->IconExplorer, &pAudioList->addId, 
				&pAudioList->firstHoleId, pAudioList->BaseListType - LTP_OFTENPLAY, eMEDIA_LIST_AUDIO);
		}
		else if (LTP_RECEAPPEND == pAudioList->BaseListType)
		{
			if (MLIST_HOLE_NONE != pAudioList->firstHoleId)
			{
				pAudioList->addId = 0;
				pAudioList->firstHoleId = MLIST_HOLE_NONE;
				IconExplorer_DelAllItem(&pAudioList->IconExplorer);
			}
			
			Ctl_MList_ToIEByAppendTimeStep(&pAudioList->IconExplorer, &pAudioList->addId, eMEDIA_LIST_AUDIO);
		}
	}
	else
	{
		if (eADD_FLAG_NONE != MList_GetAddFlag(eMEDIA_LIST_AUDIO))
		{
			T_BOOL bNeedCheck = AK_FALSE;
			
			if (MList_GetAddFlag(eMEDIA_LIST_AUDIO) & eADD_FLAG_NEW)
			{
				bNeedCheck = AK_TRUE;
			}
			
			MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_NONE);

			if (MList_GetDelFlag(eMEDIA_LIST_AUDIO))
			{
				MList_SetDelFlag(eMEDIA_LIST_AUDIO, AK_FALSE);
				pAudioList->addId = 0;
				pAudioList->firstHoleId = MLIST_HOLE_NONE;
				memset(pAudioList->searchCnt, 0, MAX_MEDIA_NUM * sizeof(T_U16));
				IconExplorer_DelAllItem(&pAudioList->IconExplorer);
			}
			
			if (LTP_TITLE == pAudioList->BaseListType)
			{
				if (MLIST_HOLE_NONE != pAudioList->firstHoleId && bNeedCheck)
				{
					pAudioList->addId = 0;
					pAudioList->firstHoleId = MLIST_HOLE_NONE;
					IconExplorer_DelAllItem(&pAudioList->IconExplorer);
				}
				
				Ctl_MList_ToIconExplorerComplete(&pAudioList->IconExplorer, &pAudioList->addId, &pAudioList->firstHoleId, eMEDIA_LIST_AUDIO);
			}
			else if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
           		&&(pAudioList->CurListType>=LTP_GENRE && pAudioList->CurListType<=LTP_COMPOSER))
			{
				if (MLIST_HOLE_NONE != pAudioList->firstHoleId && bNeedCheck)
				{
					pAudioList->addId = 0;
					pAudioList->firstHoleId = MLIST_HOLE_NONE;
					IconExplorer_DelAllItem(&pAudioList->IconExplorer);
				}
				
				Ctl_MList_ID3_ToIconExplorerComplete(&pAudioList->IconExplorer, &pAudioList->addId, &pAudioList->firstHoleId, pAudioList->BaseListType-LTP_GENRE);
			}
			else if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
       			&&(LTP_TITLE == pAudioList->CurListType))
			{
				switch(pAudioList->BaseListType)
				{
				case LTP_GENRE:
					Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
						pAudioList->searchCnt, pAudioList->ClassInfo.ClassInfo.pGenre, eID3_TAGS_GENRE);
					break;
				case LTP_ARTIST:
					Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
						pAudioList->searchCnt, pAudioList->ClassInfo.ClassInfo.pArtist, eID3_TAGS_ARTIST);
					break;
				case LTP_ALBUM:
					Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
						&pAudioList->addId, pAudioList->ClassInfo.ClassInfo.pAlbum, eID3_TAGS_ALBUM);
					break;
				case LTP_COMPOSER:
					Ctl_MList_ID3_SongToIconExplorer(&pAudioList->IconExplorer, 
						&pAudioList->addId, pAudioList->ClassInfo.ClassInfo.pComposer, eID3_TAGS_COMPOSER);
					break;
				default:
					break;
				}
		
			}
			else if (pAudioList->BaseListType>=LTP_OFTENPLAY && pAudioList->BaseListType<=LTP_RECEAPPEND)
			{
				if (MLIST_HOLE_NONE != pAudioList->firstHoleId && bNeedCheck)
				{
					pAudioList->addId = 0;
					pAudioList->firstHoleId = MLIST_HOLE_NONE;
					IconExplorer_DelAllItem(&pAudioList->IconExplorer);
				}
				
				Ctl_MList_ToIEByPlayInfoComplete(&pAudioList->IconExplorer, &pAudioList->addId, 
					&pAudioList->firstHoleId, pAudioList->BaseListType - LTP_OFTENPLAY, eMEDIA_LIST_AUDIO);
			}
		}
	}
}

#endif

/*---------------------- BEGIN OF STATE s_audio_list_title ------------------------*/
void initaudio_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER
	pAudioList = (T_AUDIO_LIST_PARM *) Fwl_Malloc(sizeof(T_AUDIO_LIST_PARM));
    AK_ASSERT_PTR_VOID(pAudioList, "initaudio_list_genre(): malloc error");

    // initialize structure of pAudioList
    memset(pAudioList, 0, sizeof(T_AUDIO_LIST_PARM));
	memset(&pAudioList->mFileList, 0, sizeof(T_FILELIST));
	pAudioList->firstHoleId = MLIST_HOLE_NONE;
	
    MenuStructInit(&pAudioList->IconExplorer);
	IconExplorer_SetItemQtyMax(&pAudioList->IconExplorer, AUDIOPLAYER_MAX_ITEM_QTY);

    m_regResumeFunc(resume_audio_list);
    m_regSuspendFunc(suspend_audio_list);

#endif
}

void exitaudio_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER

	HJH_DEBUG("exitaudio_list: pAudioList->BaseListType = %d.\n", pAudioList->BaseListType);
	
    // save my favorite audio list
    // if item quantity is change, will be save change to list
    if (LTP_FAVORITE == pAudioList->BaseListType 
		&& AK_NULL != pAudioList->pIconExplorer
		&& pAudioList->ItemQty != IconExplorer_GetItemQty(&pAudioList->IconExplorer))
    {
		T_pCWSTR	pFilePath;
		
		pFilePath = (T_pCWSTR)IconExplorer_GetItemContentFocus(pAudioList->pIconExplorer);
	    if (pFilePath)
	    {
	    	AudioList_SaveListFromExplorer(pFilePath);
	    }
    }

    //check and free ClassInfo
    AudioList_FreeClassInfo(&pAudioList->ClassInfo.ClassInfo);
    IconExplorer_Free(&pAudioList->IconExplorer);    

    TopBar_DisableMenuButton();

	// free filelist
    FileList_Free(&pAudioList->mFileList);
    pAudioList = Fwl_Free(pAudioList);
    
#endif
}

void paintaudio_list(void)
{
#ifdef SUPPORT_AUDIOPLAYER
    IconExplorer_Show(&pAudioList->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;
    static T_U32    FocusID[5][5]={0};
	T_USTR_FILE	   pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
	T_ICONEXPLORER_ITEM *p;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioList->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }
    
    if (M_EVT_LIST == event)
    {
        AudioList_HandleInitEvt(pEventParm);
        
        if (pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_RECEAPPEND)
    	{
			WaitBox_Start(WAITBOX_CLOCK, (T_pWSTR)GetCustomString(csLOADING));
    		AudioList_update_list();
			WaitBox_Stop();
    	}
    }

    if (M_EVT_PUB_TIMER == event)
    {
    	if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_TITLE)
			|| (LTP_RECEAPPEND == pAudioList->BaseListType))
    	{
    		AudioList_update_list();
    	}
    }


    switch (IconExplorerRet = IconExplorer_Handler(&pAudioList->IconExplorer, event, pEventParm))
    {
    case eNext:
			//favorite list and audio list
            if ( LTP_FAVORITE 			== pAudioList->BaseListType 
                || LTP_TITLE 			== pAudioList->CurListType
                || LTP_COMPOSER_TITLE 	== pAudioList->CurListType
                || LTP_OFTENPLAY		== pAudioList->CurListType
                || LTP_RECEPLAY			== pAudioList->CurListType
                || LTP_RECEAPPEND		== pAudioList->CurListType )
            {
				pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(&pAudioList->IconExplorer);

	            if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 != pFilePath[0])
				{
	                AudioPlayer_Stop();
	                IconExplorer_DelAllItem(pAudioList ->pGIconExplorer);
	                IconExplorer_CopyItems(pAudioList ->pGIconExplorer, &pAudioList->IconExplorer);
	                
	                pEventParm->p.pParam1 = pAudioList ->pGIconExplorer;
	                // go to state of audio player, Play Media
	                m_triggerEvent(M_EVT_NEXT, pEventParm);
                }
				else if(0 != pAudioList->ItemQty)
				{
					IconExplorer_DelItemFocus(&pAudioList->IconExplorer);
					MsgBox_InitAfx(&pAudioList->msgbox, 1, ctFAILURE, csNOT_IN_AUDIOLIST, MSGBOX_INFORMATION);
					MsgBox_SetDelay(&pAudioList->msgbox, MSGBOX_DELAY_1);
            		m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioList->msgbox);
				}
            }
			// update list
            else                 
            {
            	p = IconExplorer_GetItemFocus(&pAudioList->IconExplorer);

            	if (AK_NULL != p && AK_NULL != p->pText)
            	{
	            	if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
	               		&&(pAudioList->CurListType>=LTP_GENRE && pAudioList->CurListType<=LTP_COMPOSER))
	               	{
	            		FocusID[pAudioList->BaseListType-LTP_GENRE][pAudioList->CurListType-LTP_GENRE]
	                		= IconExplorer_GetItemFocusId(&pAudioList->IconExplorer);
	               	}

	                // set class infomation
	                AudioList_SetClassInfo(p->pText);
	                // modify current list type
	                AudioList_ModifyCurrentListType(AK_TRUE);
	                // refresh explorer list
	                AudioList_refresh();
                }
            }
    
        break;
		
    case eMenu:
        if ( LTP_ALBUM 			!= pAudioList->CurListType
            && LTP_ARTIST 		!= pAudioList->CurListType
            && LTP_GENRE 		!= pAudioList->CurListType
            && LTP_COMPOSER		!= pAudioList->CurListType)
        {
            IconExplorer_SetRefresh(&pAudioList->IconExplorer, ICONEXPLORER_REFRESH_ALL);
            pEventParm->p.pParam1 = &pAudioList->IconExplorer;
            
            if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
           		&&(LTP_TITLE == pAudioList->CurListType))
            {
            	pEventParm->p.pParam2 = &pAudioList->ClassInfo.ClassInfo;
            }
            else
            {
            	
            }
            
            pEventParm->p.pParam3 = (T_pVOID)pAudioList->BaseListType;

            m_triggerEvent(M_EVT_MENU, pEventParm);
        }
        break;
        
    case eReturn:
        //from sub class to father class 
        if (pAudioList->BaseListType != pAudioList->CurListType)
        {
        	AudioList_ModifyCurrentListType(AK_FALSE);			
            AudioList_ModifyClassInfo();
            AudioList_refresh();
			
            if ((pAudioList->BaseListType>=LTP_GENRE && pAudioList->BaseListType<=LTP_COMPOSER)
              &&(pAudioList->CurListType>=LTP_GENRE && pAudioList->CurListType<=LTP_COMPOSER))
            {
            	IconExplorer_SetFocus(&pAudioList->IconExplorer, FocusID[pAudioList->BaseListType-LTP_GENRE][pAudioList->CurListType-LTP_GENRE]);
			}
            break;
        }
		
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

/**
 * @brief   get sub set and add class result to icon explorer
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_VOID 
 * @return  T_VOID
 */
static T_VOID AudioList_refresh(T_VOID) 
{
	 AK_DEBUG_OUTPUT("Refresh Audio List.\n");
	 
    if (LTP_FAVORITE == pAudioList->BaseListType)
    {
        AK_DEBUG_OUTPUT("AudioList_refresh(): BaseListType is LTP_FAVORITE, return\n");
        return;
    } 
  
    IconExplorer_DelAllItem(&pAudioList->IconExplorer);
    
    MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_NEW);
    pAudioList->addId = 0;
    pAudioList->firstHoleId = MLIST_HOLE_NONE;
	memset(pAudioList->searchCnt, 0, MAX_MEDIA_NUM * sizeof(T_U16));

	WaitBox_Start(WAITBOX_CLOCK, (T_pWSTR)GetCustomString(csLOADING));
    AudioList_update_list();
    WaitBox_Stop();
	
    AudioList_SetTitle();
}


/**
 * @brief   set title
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_VOID 
 * @return  T_VOID
 */
static T_VOID AudioList_SetTitle(T_VOID)
{
    T_CLASSINFO     *pClassInfo = AK_NULL;

    if (pAudioList->BaseListType == pAudioList->CurListType)
    {
        switch (pAudioList->BaseListType)
        {
        case LTP_GENRE:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_GENRE), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_ARTIST:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_ARTIST), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_ALBUM:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_ALBUM), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_TITLE:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_ALLAUDIO), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_COMPOSER:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_COMPOSER), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_OFTENPLAY:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_OPENPLAY), ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
		case LTP_RECEPLAY:
			IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_RECEPLAY), ICONEXPLORER_TITLE_TEXTCOLOR);
			break;
			
		case LTP_RECEAPPEND:
			IconExplorer_SetTitleText(&pAudioList->IconExplorer, \
						Res_GetStringByID(eRES_STR_AUDIOPLAYER_RECEADD), ICONEXPLORER_TITLE_TEXTCOLOR);
			break;
			
        default:
            break;
        }
    }
    else
    {
        pClassInfo = &pAudioList->ClassInfo.ClassInfo;
        
        switch (pAudioList->BaseListType)
        {
        case LTP_GENRE:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, pClassInfo->pGenre, ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_ARTIST:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, pClassInfo->pArtist, ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_ALBUM:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, pClassInfo->pAlbum, ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        case LTP_COMPOSER:
            IconExplorer_SetTitleText(&pAudioList->IconExplorer, pClassInfo->pComposer, ICONEXPLORER_TITLE_TEXTCOLOR);
            break;
			
        default:
            break;
        }
    }
}

/**
 * @brief   load favorite list.
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_ICONEXPLORER* pIconExplorer: icon explorer strcut pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE:fail
 */
static T_BOOL AudioList_LoadFavoriteList(T_ICONEXPLORER* pIconExplorer)
{
    T_FILELIST      FileList;
    T_ICONEXPLORER_ITEM * focusItem;
    
    focusItem = IconExplorer_GetItemFocus(pIconExplorer);
    if (FileList_Init(&FileList, MYLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE,  AudioPlayer_IsSupportFile) == AK_FALSE)
    {
        AK_DEBUG_OUTPUT("\ninit file list fail!!!\n");
        return AK_FALSE;
    }

    if (FileList_Add(&FileList, focusItem->pContent, FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) == FILELIST_ADD_ERROR)
    {
        AK_DEBUG_OUTPUT("\nload custom list fail!!!\n");
        return AK_FALSE;
    }

    MenuStructInit(&pAudioList->IconExplorer);
    IconExplorer_SetItemQtyMax(&pAudioList->IconExplorer, MYLIST_MAX_ITEM_QTY);
    IconExplorer_SetTitleText(&pAudioList->IconExplorer, focusItem->pText, ICONEXPLORER_TITLE_TEXTCOLOR);
    FileList_ToIconExplorer(&FileList, &pAudioList->IconExplorer, eINDEX_TYPE_AUDIO, AK_NULL);
    FileList_Free(&FileList);

    return AK_TRUE;
}


/**
 * @brief   set classify infomation.
 * should be call before enter sub class
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_U16 *pStr: classify keyword
 * @return  T_VOID
 */
static T_VOID AudioList_SetClassInfo(T_U16 *pStr)
{
    T_U16       *pTmp = AK_NULL;
    T_CLASSINFO *pClassInfo = AK_NULL;

    pTmp = (T_U16 *)Fwl_Malloc((T_U32)((Utl_UStrLen(pStr)+1)<<1));
    if (AK_NULL == pTmp)
    {
        AK_DEBUG_OUTPUT("AudioList_SetClassInfo(): Malloc Fail!\n");
        return;
    }
    
    Utl_UStrCpy(pTmp, pStr);
    pClassInfo = &pAudioList->ClassInfo.ClassInfo;
    
    if (LTP_GENRE == pAudioList->BaseListType)
    {
        if (pClassInfo->pGenre == AK_NULL)
        {
            pClassInfo->pGenre = pTmp;
        }
        else if (pClassInfo->pArtist == AK_NULL) 
        {
            pClassInfo->pArtist = pTmp;
        }
        else if (pClassInfo->pAlbum== AK_NULL)
        {
            pClassInfo->pAlbum = pTmp;
        }
    }
    else if (LTP_ARTIST == pAudioList->BaseListType)
    {
        if (pClassInfo->pArtist == AK_NULL) 
        {
            pClassInfo->pArtist = pTmp;
        }
        else if (pClassInfo->pAlbum== AK_NULL)
        {
            pClassInfo->pAlbum = pTmp;
        }
    }
    else if (LTP_ALBUM== pAudioList->BaseListType)
    {
        if (pClassInfo->pAlbum== AK_NULL)
        {
            pClassInfo->pAlbum = pTmp;
        }
    }
    else
    {
    	pClassInfo->pComposer= pTmp;
    }
}

/**
 * @brief   modify current list type.
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_BOOL flag:AK_TRUE : enter sub class
 * @param   T_BOOL flag:AK_FALSE : return father class
 * @return  T_VOID
 */
static T_VOID AudioList_ModifyCurrentListType(T_BOOL flag)
{
    if (AK_TRUE == flag)
    {
        switch(pAudioList->CurListType)
        {
        case LTP_GENRE:
            pAudioList->CurListType = LTP_TITLE;	// LTP_ARTIST;
            break;
			
        case LTP_ARTIST:
            pAudioList->CurListType = LTP_TITLE;	// LTP_ALBUM;
            break;
			
        case LTP_ALBUM:
            pAudioList->CurListType = LTP_TITLE;
            break;
			
        case LTP_COMPOSER:
            pAudioList->CurListType = LTP_TITLE;	// LTP_COMPOSER_TITLE;
            break;
			
        default:
            break;
        }
    }
    else
    {
        pAudioList->CurListType = pAudioList->BaseListType;
    }

    if (pAudioList->CurListType == LTP_TITLE || pAudioList->CurListType == LTP_COMPOSER_TITLE)
    {
        TopBar_EnableMenuButton();
    }
    else
    {
        TopBar_DisableMenuButton();
    }
}

/**
 * @brief   modify classify infomation.
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_VOID
 * @return  T_VOID
 */
static T_VOID AudioList_ModifyClassInfo(T_VOID)
{
    T_CLASSINFO     *pClassInfo = AK_NULL;

    pClassInfo = &pAudioList->ClassInfo.ClassInfo;
    
    switch(pAudioList->CurListType)
    {
    case LTP_ALBUM:
        pClassInfo->pAlbum = Fwl_Free(pClassInfo->pAlbum);
        break;
		
    case LTP_ARTIST:
        pClassInfo->pArtist = Fwl_Free(pClassInfo->pArtist);
        break;
		
    case LTP_GENRE:
        pClassInfo->pGenre = Fwl_Free(pClassInfo->pGenre);
        break;
		
    case LTP_COMPOSER:
        pClassInfo->pComposer= Fwl_Free(pClassInfo->pComposer);
        break;
		
    default:
        break;
    }
	
}

/**
 * @brief   check and free classify infomation. 
 * Should be called before exit s_audio_list state machine.
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_CLASSINFO *pClassInfo: classify infomation struct pointer
 * @return  T_VOID
 */
static T_VOID AudioList_FreeClassInfo(T_CLASSINFO *pClassInfo)
{
    if (AK_NULL != pClassInfo->pAlbum)
    {
        pClassInfo->pAlbum = Fwl_Free(pClassInfo->pAlbum);
    }

    if (AK_NULL != pClassInfo->pArtist)
    {
        pClassInfo->pArtist= Fwl_Free(pClassInfo->pArtist);
    }

    if (AK_NULL != pClassInfo->pGenre)
    {
        pClassInfo->pGenre = Fwl_Free(pClassInfo->pGenre);
    }

    if (AK_NULL != pClassInfo->pComposer)
    {
        pClassInfo->pComposer= Fwl_Free(pClassInfo->pComposer);
    }
}

/**
 * @brief   resume function 
 * system auto call
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_VOID
 * @return  T_VOID
 */
static T_VOID resume_audio_list(T_VOID)
{
    if (LTP_TITLE == pAudioList->BaseListType 
        || LTP_TITLE == pAudioList->CurListType
        || LTP_FAVORITE == pAudioList->BaseListType
        || LTP_OFTENPLAY	== pAudioList->BaseListType
        || LTP_RECEAPPEND	== pAudioList->BaseListType
        || LTP_RECEPLAY		== pAudioList->BaseListType)
    {
        TopBar_EnableMenuButton();
    }

}

/**
 * @brief   suspend function 
 * system auto call
 * @author  wangwei
 * @date    2008-05-06
 * @param   T_VOID
 * @return  T_VOID
 */
static T_VOID suspend_audio_list(T_VOID)
{
	return;
}
#endif

