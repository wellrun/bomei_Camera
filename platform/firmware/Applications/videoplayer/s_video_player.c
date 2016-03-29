
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "akdefine.h"
#include "Gbl_Global.h"
#include "Gbl_Resource.h"
#include "Eng_String.h"
#include "Eng_GblString.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Ctl_AVIPlayer.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Fwl_pfKeypad.h"
#include "Eng_time.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_Math.h"
#include "Eng_FileManage.h"
#include "Ctl_Fm.h"
#include "fwl_keyhandler.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "Fwl_display.h"
#include "Log_MediaPlayer.h"
#include "svc_medialist.h"
#include "Fwl_DispOsd.h"


static T_BOOL bHandleKeyLock = AK_FALSE;

typedef struct {
    T_ICONEXPLORER	*pIconExplorer;
    T_MSGBOX		msgbox;
	T_BOOL			volumeStatus;
	T_USTR_FILE     pathExplorer;        	/**< path when play from explorer */
}T_VIDEO_PLAYER;

static T_VIDEO_PLAYER *pVideo_Player;

static T_VOID VideoPlayer_PlayNext(T_EVT_PARAM *pEventParm, T_U32 keyID);
static T_VOID player_PlayFocus(T_EVT_PARAM *pEventParm);
static T_VOID VideoPlayer_Suspend(T_VOID);
static T_VOID VideoPlayer_Resume(T_VOID);
static T_VOID VideoPlayer_EndCallBack(T_END_TYPE endtype, T_EVT_PARAM *pEventParm);


#endif
/*---------------------- BEGIN OF STATE s_video_player ------------------------*/

void initvideo_player(void)
{
#ifdef SUPPORT_VIDEOPLAYER

	//AK_DEBUG_OUTPUT("initvideo_player 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023

	Menu_FreeRes();
	Standby_FreeUserBkImg();

    pVideo_Player = (T_VIDEO_PLAYER *)Fwl_Malloc(sizeof(T_VIDEO_PLAYER));
    AK_ASSERT_PTR_VOID(pVideo_Player, "initvideo_player(): malloc error");

	pVideo_Player->volumeStatus = Fwl_GetAudioVolumeStatus();

    AudioPlayer_Stop();
    TopBar_DisableShow();

    m_regSuspendFunc(VideoPlayer_Suspend);
    m_regResumeFunc(VideoPlayer_Resume);

	Fwl_AudioEnableDA();

    bHandleKeyLock = AK_FALSE;
    gb.AlarmForbidFlag = 1;

	Fwl_SetMultiChannelDisp(AK_TRUE);
#endif
	//AK_DEBUG_OUTPUT("initvideo_player 2 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023
}

void exitvideo_player(void)
{
#ifdef SUPPORT_VIDEOPLAYER

	//AK_DEBUG_OUTPUT("exitvideo_player 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023

    //sometimes does not execute AVIPlayer_Stop because of openning file unsuccessfully
    //so these codes below have to be remain


	// save default movie path
    TopBar_EnableShow();

    // Fwl_AudioStop();
    AVIPlayer_Free();
    
    Fwl_AudioDisableDA();
    // Fwl_AudioDisableDAAndDsp();
	// DisableLayerYUV();
	
	if (pVideo_Player->volumeStatus && !Fwl_GetAudioVolumeStatus())
	{
		Fwl_AudioSetVolume(Fwl_GetAudioVolume());
		Fwl_SetAudioVolumeStatus(AK_TRUE);
	}

    pVideo_Player = Fwl_Free(pVideo_Player);
   // Menu_LoadRes();
    /**Enable power off*/
    AutoPowerOffEnable(FLAG_VIDEO);
    gb.AlarmForbidFlag = 0;

#ifdef OS_ANYKA
    GE_ShadeCancel();    
#endif

	Menu_LoadRes();
	Standby_LoadUserBkImg();
	
	AK_Sleep(20);// 加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
	Fwl_Osd_DisplayOff();

	Fwl_SetMultiChannelDisp(AK_FALSE);
	//AK_DEBUG_OUTPUT("exitvideo_player 2 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023
#endif
}

void paintvideo_player(void)
{
#ifdef SUPPORT_VIDEOPLAYER

	AVIPlayer_Paint();
#endif
}

unsigned char handlevideo_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE ret;
    T_MMI_KEYPAD phyKey;

    if (IsPostProcessEvent(event))
    {
        bHandleKeyLock = AK_TRUE;
        AVIPlayer_OperateSuspend();
        return 1;
    }

    if (bHandleKeyLock)
    {
        bHandleKeyLock = AK_FALSE;
        AVIPlayer_OperateResume();
        return 0;
    }

    if(event == M_EVT_NEXT)
    {
    	if (!AVIPlayer_Init())
		{
			MsgBox_InitAfx(&pVideo_Player->msgbox, 2, ctFAILURE, csMEM_NOT_ENOUGH, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&pVideo_Player->msgbox, MSGBOX_DELAY_0);
			m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_Player->msgbox);
		}
		else
		{        	
			pVideo_Player->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;	

			if (gb.bInExplorer)
			{
				Utl_UStrCpyN(pVideo_Player->pathExplorer, pEventParm->p.pParam2, sizeof(T_USTR_FILE)/2);
			}
			
           	player_PlayFocus(pEventParm);
		}				
        return 0;
    }

    if (event == VME_EVT_MEDIA)	// one media file end!
    {
        T_END_TYPE endtype = (T_END_TYPE)pEventParm->w.Param2;

        VideoPlayer_EndCallBack(endtype, pEventParm);  
    }

    ret = AVIPlayer_Handle(event, pEventParm);
    
    switch (ret)
    {
    case eNext:
        break;
            
    case eStay:
    	phyKey.keyID = pEventParm->c.Param1;
        phyKey.pressType = pEventParm->c.Param2;
        
        if ((event == M_EVT_USER_KEY && phyKey.pressType == PRESS_SHORT)
			|| (event == M_EVT_TOUCH_SCREEN && phyKey.pressType == PRESS_LONG))
        {
			pEventParm->w.Param2 = T_END_TYPE_USER;
			
        	VideoPlayer_PlayNext(pEventParm, phyKey.keyID);             
        }
        break;
         
    default:
        ReturnDefauleProc(ret, pEventParm);
        break;
       
    }
#endif
    return 0;
}

#ifdef SUPPORT_VIDEOPLAYER

static T_VOID player_PlayFocus(T_EVT_PARAM *pEventParm)
{
    T_INDEX_CONTENT *pcontent = AK_NULL;
    T_USTR_FILE		pFilePath = {0};
	T_END_TYPE endType;

	endType = (T_END_TYPE)(pEventParm->w.Param2);
	
    Fwl_KeyStop();
    if (!AVIPlayer_Stop(endType))
    {
		AK_DEBUG_OUTPUT("Flay Focus: AVIPlayer_Stop() FAILURE!\n");
		return;

    }

	Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
	

    if (pVideo_Player && pVideo_Player->pIconExplorer)
    {
        pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pVideo_Player->pIconExplorer);
        if (AK_NULL != pcontent)
		{
			MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_VIDEO);
		}

		if (0 == pFilePath[0] && gb.bInExplorer)
		{
			Utl_UStrCpyN(pFilePath, pVideo_Player->pathExplorer, sizeof(T_USTR_FILE)/2);
		}
		
		if (0 == pFilePath[0])        
        {
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            return;
        }
        
        if (!AVIPlayer_PlayFromFile(pFilePath))
        {
            if( !FileMgr_CheckFileIsExist(pFilePath) )
                MsgBox_InitAfx(&pVideo_Player->msgbox, 2, ctFAILURE, csFILE_NOT_EXIST, MSGBOX_INFORMATION);
            else
                MsgBox_InitAfx(&pVideo_Player->msgbox, 2, ctFAILURE, csVIDEO_FILE_ERROR, MSGBOX_INFORMATION);

        	MList_RemoveMediaItem(pFilePath, AK_FALSE, eMEDIA_LIST_VIDEO);
            IconExplorer_DelItemFocus(pVideo_Player->pIconExplorer);
            AK_DEBUG_OUTPUT("player_PlayFocus(): Play Failed.\n");
            AVIPlayer_Stop(T_END_TYPE_ERR);

            MsgBox_SetDelay(&pVideo_Player->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_Player->msgbox);
        }        
    }
}

static T_BOOL VideoPlayer_MoveFocus2Next(T_VOID)
{
	T_ICONEXPLORER_ITEM *pItem = AK_NULL;
	T_U32 i, count = 0, random;
	
	AK_DEBUG_OUTPUT("VPLAYER:	Move Focus to Next.\n");

	switch(gs.VideoRepMode)
	{
	case FILELIST_FETCH_RANDOM:
        pItem = IconExplorer_GetItemFocus(pVideo_Player->pIconExplorer);
		
        while (count++ < 3)
        {
            random = Fwl_GetRand(IconExplorer_GetItemQty(pVideo_Player->pIconExplorer));
            
            for (i=0; i<random; i++)
                IconExplorer_MoveFocus(pVideo_Player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);

            if (pItem != IconExplorer_GetItemFocus(pVideo_Player->pIconExplorer))
                break ;
        }
		break;
		
	case FILELIST_FETCH_SEQUENCE:
		pItem = IconExplorer_GetItemFocus(pVideo_Player->pIconExplorer);
		
        if (AK_NULL == pItem || AK_NULL == pItem->pNext)
        {
			m_triggerEvent(M_EVT_EXIT, AK_NULL);
            return AK_FALSE;            
        }        
		
	case FILELIST_FETCH_REPEAT:
		IconExplorer_MoveFocus(pVideo_Player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);
		break;
		
	case FILELIST_FETCH_REPEAT_SINGLE:
	default:
		break;		
	}	
    
    IconExplorer_SetRefresh(pVideo_Player->pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

	return AK_TRUE;
}

static T_VOID VideoPlayer_EndCallBack(T_END_TYPE endtype, T_EVT_PARAM *pEventParm)
{
	T_EVT_PARAM evtParm;

	evtParm.w.Param2 = (T_U32)endtype;
		
    AK_DEBUG_OUTPUT("VideoPlayer_EndCallBack(): End type = %d.\n", endtype);

    if (T_END_TYPE_NORMAL == endtype)
    {
        if (!VideoPlayer_MoveFocus2Next())
			return;
        

		player_PlayFocus(&evtParm);

    }
    
    else if (T_END_TYPE_ERR == endtype)
    {
        AVIPlayer_Stop(endtype);

        MsgBox_InitAfx(&pVideo_Player->msgbox, 2, ctFAILURE, csVIDEO_FILE_ERROR, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pVideo_Player->msgbox, MSGBOX_DELAY_0);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_Player->msgbox);
    }
}

static T_VOID VideoPlayer_Suspend(T_VOID)
{
	AK_Sleep(20);//加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
	Fwl_Osd_DisplayOff();
	Fwl_SetMultiChannelDisp(AK_FALSE);
}

static T_VOID VideoPlayer_Resume(T_VOID)
{
    // AVIPlayer_GetRes();
    TopBar_DisableShow();
	Fwl_SetMultiChannelDisp(AK_TRUE);
}

static T_VOID VideoPlayer_PlayNext(T_EVT_PARAM *pEventParm, T_U32 keyID)
{
	T_U8 direction = 0xFF;
	
	AK_ASSERT_PTR_VOID(pVideo_Player->pIconExplorer, "VideoPlayer_PlayNext(): pIconExplorer Is Invalid.\n");

	if (kbUP == keyID)
		direction = ICONEXPLORER_DIRECTION_UP;
	else if (kbDOWN == keyID)
		direction = ICONEXPLORER_DIRECTION_DOWN;
	else
		return;
		
	IconExplorer_MoveFocus(pVideo_Player->pIconExplorer, direction);
	
	player_PlayFocus(pEventParm);
}

#endif
