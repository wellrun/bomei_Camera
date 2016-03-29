
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Ctl_Audioplayer.h"
#include "Fwl_pfAudio.h"
#include "Ctl_Fm.h"
#include "Ctl_DisplayList.h"
#include "Ctl_FileList.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "fwl_oscom.h"
#include "fwl_pfdisplay.h"
#include "Eng_pwr_onoff_video.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "fwl_display.h"

#define PREVIEW_TIMER 5000          //5s for video preview

typedef struct {
    T_TIMER             timerId;
    T_ICONEXPLORER      *pIconExplorer;
    T_ICONEXPLORER      IconExplorer;
    T_MSGBOX            msgbox;
    T_BOOL              PreviewEndFlag;
} T_VIDEO_MENU_PARM;

static T_VIDEO_MENU_PARM *pVideoMenuParm;

extern T_U8  vitempaintflag;

T_BOOL video_set_preview(T_pCWSTR videopath);

static void VideoMenuSuspend(void)
{
	TopBar_MenuIconShowSwitch(AK_FALSE);
}

static T_VOID Video_SetPwrOnOffFile(T_USTR_FILE fileName)
{
	T_USTR_FILE	   pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
	
	pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pVideoMenuParm->pIconExplorer);
	if (AK_NULL != pcontent)
	{
		MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_VIDEO);
	}

	if (0 == pFilePath[0])
		return;
		
	if (!video_set_preview(pFilePath))
	{
		MsgBox_InitAfx(&pVideoMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
	}
	else
	{
		Utl_UStrCpy(fileName, pFilePath);
		MsgBox_InitAfx(&pVideoMenuParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
	}

	AK_Sleep(20);//加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
	
	Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
	MsgBox_SetDelay(&pVideoMenuParm->msgbox, MSGBOX_DELAY_1);
	m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideoMenuParm->msgbox);
}

#endif
/*---------------------- BEGIN OF STATE s_video_menu ------------------------*/
void initvideo_menu(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideoMenuParm = (T_VIDEO_MENU_PARM *)Fwl_Malloc(sizeof(T_VIDEO_MENU_PARM));
    AK_ASSERT_PTR_VOID(pVideoMenuParm, "initvideo_menu(): malloc error");

    MenuStructInit(&pVideoMenuParm->IconExplorer);
    GetMenuStructContent(&pVideoMenuParm->IconExplorer, mnVIDEO_MENU);
    m_regResumeFunc(TopBar_EnableShow);
	m_regSuspendFunc(VideoMenuSuspend);
    pVideoMenuParm->PreviewEndFlag = AK_FALSE;
    pVideoMenuParm->timerId = ERROR_TIMER;
    TopBar_DisableMenuButton();

	TopBar_MenuIconShowSwitch(AK_FALSE);
#endif
}

void exitvideo_menu(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Free(&pVideoMenuParm->IconExplorer);
    pVideoMenuParm = Fwl_Free(pVideoMenuParm);

	TopBar_MenuIconShowSwitch(AK_TRUE);
#endif
}

void paintvideo_menu(void)
{
#ifdef SUPPORT_VIDEOPLAYER
    IconExplorer_Show(&pVideoMenuParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_USTR_FILE	   pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pVideoMenuParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_MENU)
    {
        pVideoMenuParm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
        pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pVideoMenuParm->pIconExplorer);
        if (AK_NULL != pcontent)
		{
			MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_VIDEO);
		}
		
        if (0 == pFilePath[0])
        {
            IconExplorer_DelItem(&pVideoMenuParm->IconExplorer, 40);
            IconExplorer_DelItem(&pVideoMenuParm->IconExplorer, 70);
            IconExplorer_DelItem(&pVideoMenuParm->IconExplorer, 80);
        }
        if (IconExplorer_GetItemQty(pVideoMenuParm->pIconExplorer) == 0)
        {
            IconExplorer_DelItem(&pVideoMenuParm->IconExplorer, 50);
        }
    }

    IconExplorerRet = IconExplorer_Handler(&pVideoMenuParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
	{
    case eNext:
        pEventParm->p.pParam1 = pVideoMenuParm->pIconExplorer;
        switch (IconExplorer_GetItemFocusId(&pVideoMenuParm->IconExplorer))
        {
    	case 10:                // add
            m_triggerEvent(M_EVT_1, pEventParm);
            break;
			
        case 20:                // read
            m_triggerEvent(M_EVT_2, pEventParm);
            break;
		
        case 30:                // save
            m_triggerEvent(M_EVT_3, pEventParm);
            break;
        case 40:                // Delete
            m_triggerEvent(M_EVT_4, pEventParm);
            break;
					
        case 50:                // Delete All
            m_triggerEvent(M_EVT_5, pEventParm);
            break;
					
        case 60:                // set mode
            m_triggerEvent(M_EVT_6, pEventParm);
            break;
					
        case 70://power on
        	Video_SetPwrOnOffFile(gs.PathPonVideo);
            break;
					
        case 80://power off
            Video_SetPwrOnOffFile(gs.PathPoffVideo);
            break;       

		default:
            break;
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

T_VOID video_preview_callback(T_TIMER timer_id, T_U32 delay)
{
    if (ERROR_TIMER != pVideoMenuParm->timerId)
    {
        Fwl_StopTimer(pVideoMenuParm->timerId);
        pVideoMenuParm->timerId = ERROR_TIMER;
    }

    pVideoMenuParm->PreviewEndFlag = AK_TRUE;    
}

T_BOOL video_set_preview(T_pCWSTR videopath)
{
    T_BOOL ret = AK_FALSE;

    TopBar_DisableShow();

    AudioPlayer_Stop();
    pVideoMenuParm->PreviewEndFlag = AK_FALSE;
       
	if (!PowerOnOff_PlayVideo((T_pWSTR)videopath, AK_NULL))
    {
        AK_DEBUG_OUTPUT("video_set_preview(): PwrOnOff open video fail.\n");
        return AK_FALSE;
    }

    pVideoMenuParm->timerId = Fwl_SetMSTimerWithCallback(PREVIEW_TIMER, AK_FALSE, video_preview_callback);

    while(1)
    {
        AK_Sleep(50);
        if (pVideoMenuParm->PreviewEndFlag)
        {
			Eng_PwrOnOff_Video_Free();
			
            ret = AK_TRUE;
            break;
        }
    }

    return ret;
}

#endif
