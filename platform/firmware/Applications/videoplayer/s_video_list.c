
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Image.h"
#include "Eng_KeyMapping.h"
#include "Fwl_pfAudio.h"
#include "Eng_ImgConvert.h"
#include "Fwl_osFS.h"
#include "Ctl_AVIPlayer.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_Fm.h"
#include "Fwl_Initialize.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "Ctl_MsgBox.h"
#include "fwl_display.h"
#include "svc_medialist.h"
#include "ctl_medialist.h"

typedef struct {
    T_ICONEXPLORER      IconExplorer;
    T_MSGBOX			msgbox;
    T_U16				addId;
    T_S32				firstHoleId;
    T_USTR_FILE         pathExplorer;        	/**< path when play from explorer */
} T_VIDEO_LIST;

static T_VIDEO_LIST *pVideo_List = AK_NULL;
static T_VOID resume_video_list(T_VOID);
static T_VOID suspend_video_list(T_VOID);
static T_VOID update_video_list(T_VOID);

extern T_pDATA pGB_dispBuffBackup;
#endif
/*---------------------- BEGIN OF STATE s_video_list ------------------------*/
void initvideo_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_FILELIST FileList;
    T_USTR_FILE Ustrtmp = {0};

    pVideo_List = (T_VIDEO_LIST *)Fwl_Malloc(sizeof(T_VIDEO_LIST));
    AK_ASSERT_PTR_VOID(pVideo_List, "initvideo_list(): malloc error");
	memset(pVideo_List, 0, sizeof(T_VIDEO_LIST));

	pVideo_List->firstHoleId = MLIST_HOLE_NONE;
    
    if (gb.bInExplorer)
    {
        Eng_StrMbcs2Ucs(VIDEOLIST_TMP_FILE, Ustrtmp);

        // init filelist
	    if (!FileList_Init(&FileList, VIDEOLIST_MAX_ITEM_QTY, FILELIST_SORT_NONE, AK_NULL))
	        return;
        
        if (FileList_Add(&FileList, Ustrtmp, FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) != FILELIST_ADD_SUCCESS)
            AK_DEBUG_OUTPUT("initvideo_list enter a null file\r\n");

        // init icon explorer, add file list to icon explorer
	    MenuStructInit(&pVideo_List->IconExplorer);
	    IconExplorer_SetTitleText(&pVideo_List->IconExplorer, Res_GetStringByID(eRES_STR_VIDEO_LIST), ICONEXPLORER_TITLE_TEXTCOLOR);
	    FileList_ToIconExplorer(&FileList, &pVideo_List->IconExplorer, eINDEX_TYPE_VIDEO, pVideo_List->pathExplorer);
	    MList_SetAddFlag(eMEDIA_LIST_VIDEO, eADD_FLAG_NONE);
	    // free file list
	    FileList_Free(&FileList);
    }
    else
    {
    	
        Eng_StrMbcs2Ucs(VIDEOLIST_DEF_FILE, Ustrtmp);
        if (!Fwl_FileExist(Ustrtmp))
        {
            MList_AddItem(Fwl_GetDefPath(eVIDEO_PATH), AK_TRUE, AK_FALSE, eMEDIA_LIST_VIDEO);
        }

        MenuStructInit(&pVideo_List->IconExplorer);
	    IconExplorer_SetTitleText(&pVideo_List->IconExplorer, Res_GetStringByID(eRES_STR_VIDEO_LIST), ICONEXPLORER_TITLE_TEXTCOLOR);

		MList_SetAddFlag(eMEDIA_LIST_VIDEO, eADD_FLAG_NEW);
	    update_video_list();
    }

    TopBar_EnableMenuButton();

    m_regResumeFunc(resume_video_list);
    m_regSuspendFunc(suspend_video_list);

#endif
}

void exitvideo_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    TopBar_DisableMenuButton();

    MList_Close(eMEDIA_LIST_VIDEO);

    IconExplorer_Free(&pVideo_List->IconExplorer);
    pVideo_List = Fwl_Free(pVideo_List);
	pGB_dispBuffBackup = Fwl_Free(pGB_dispBuffBackup);
#endif
}

void paintvideo_list(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Show(&pVideo_List->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_USTR_FILE	   pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
    T_pFILE fp;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pVideo_List->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_2)
    {
        if (IconExplorer_GetItemFocus(&pVideo_List->IconExplorer) != AK_NULL)
        {
            pEventParm->p.pParam1 = &pVideo_List->IconExplorer;
            pEventParm->p.pParam2 = pVideo_List->pathExplorer;
            
            m_triggerEvent(M_EVT_NEXT, pEventParm);
        }
        
        else
        {
            m_triggerEvent(M_EVT_EXIT, pEventParm);
        }
        
        return 0;
    }

    if ( (event == M_EVT_EXIT || (event >= M_EVT_RETURN && event <= M_EVT_RETURN9))
    	&& gb.bInExplorer == AK_TRUE )
    {
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        return 0;
    }

    if (M_EVT_PUB_TIMER == event)
    {
    	update_video_list();
    }

    IconExplorerRet = IconExplorer_Handler(&pVideo_List->IconExplorer, event, pEventParm);
    
    switch (IconExplorerRet)
    {
    case eNext:
        if (IconExplorer_GetItemFocus(&pVideo_List->IconExplorer) != AK_NULL)
        {            
	        pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(&pVideo_List->IconExplorer);	
	        if (AK_NULL != pcontent)
			{
				MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_VIDEO);
			}

			if (0 == pFilePath[0])
			{
				IconExplorer_DelItemFocus(&pVideo_List->IconExplorer);
    				
				MsgBox_InitAfx(&pVideo_List->msgbox, 1, ctFAILURE, csNOT_IN_VIDEOLIST, MSGBOX_INFORMATION);
				MsgBox_SetDelay(&pVideo_List->msgbox, MSGBOX_DELAY_0);
				m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_List->msgbox);
					
				break;
			}
			
			fp = Fwl_FileOpen(pFilePath, _FMODE_READ, _FMODE_READ);
				
			// If Cann't Open This File, Delete it in Explorer List
    		if (FS_INVALID_HANDLE == fp)
    		{
    			MList_RemoveMediaItem(pFilePath, AK_FALSE, eMEDIA_LIST_VIDEO);
    			IconExplorer_DelItemFocus(&pVideo_List->IconExplorer);
    				
				MsgBox_InitAfx(&pVideo_List->msgbox, 1, ctFAILURE, csFILE_NOT_EXIST, MSGBOX_INFORMATION);
				MsgBox_SetDelay(&pVideo_List->msgbox, MSGBOX_DELAY_0);
				m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_List->msgbox);
					
				break;
			}
			else
			{
				Fwl_FileClose(fp);
			}
				
			pEventParm->p.pParam1 = &pVideo_List->IconExplorer;
	        m_triggerEvent(M_EVT_NEXT, pEventParm);
        }
        break;
        
    case eMenu:
        IconExplorer_SetRefresh(&pVideo_List->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        pEventParm->p.pParam1 = &pVideo_List->IconExplorer;
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


#ifdef SUPPORT_VIDEOPLAYER

T_BOOL AVIList_Add(T_pCWSTR pFilePath, T_BOOL SearchSub)
{	
    return MList_AddItem(pFilePath, SearchSub, AK_TRUE, eMEDIA_LIST_VIDEO);
}


static T_VOID resume_video_list(T_VOID)
{
	TopBar_EnableMenuButton();
	update_video_list();
}

static T_VOID suspend_video_list(T_VOID)
{
	return;
}

static T_VOID update_video_list(T_VOID)
{	
	if (AK_NULL == pVideo_List)
	{
		return;
	}

	if (MList_IsAdding(eMEDIA_LIST_VIDEO))
	{
		if (MLIST_HOLE_NONE != pVideo_List->firstHoleId
			|| MList_GetDelFlag(eMEDIA_LIST_VIDEO))
		{
			MList_SetDelFlag(eMEDIA_LIST_VIDEO, AK_FALSE);
			pVideo_List->addId = 0;
			pVideo_List->firstHoleId = MLIST_HOLE_NONE;
			IconExplorer_DelAllItem(&pVideo_List->IconExplorer);
		}
		
		Ctl_MList_ToIconExplorerStep(&pVideo_List->IconExplorer, &pVideo_List->addId, eMEDIA_LIST_VIDEO);
	}
	else
	{
		if (eADD_FLAG_NONE != MList_GetAddFlag(eMEDIA_LIST_VIDEO))
		{
			T_BOOL bNeedCheck = AK_FALSE;
			
			if (MList_GetAddFlag(eMEDIA_LIST_VIDEO) & eADD_FLAG_NEW)
			{
				bNeedCheck = AK_TRUE;
			}
			
			MList_SetAddFlag(eMEDIA_LIST_VIDEO, eADD_FLAG_NONE);

			if ((MLIST_HOLE_NONE != pVideo_List->firstHoleId && bNeedCheck)
				|| MList_GetDelFlag(eMEDIA_LIST_VIDEO))
			{
				MList_SetDelFlag(eMEDIA_LIST_VIDEO, AK_FALSE);
				pVideo_List->addId = 0;
				pVideo_List->firstHoleId = MLIST_HOLE_NONE;
				IconExplorer_DelAllItem(&pVideo_List->IconExplorer);
			}
			
			Ctl_MList_ToIconExplorerComplete(&pVideo_List->IconExplorer, &pVideo_List->addId, &pVideo_List->firstHoleId, eMEDIA_LIST_VIDEO);
		}
	}
}

#endif
/* end of files */
