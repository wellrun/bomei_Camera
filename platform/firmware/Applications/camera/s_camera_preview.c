/*==============================================================
MODELNAME: camera preview
AUTHOR: zhengwenbo
CREATE DATE:   2006-7-4
LOG:
==============================================================*/
#include "Fwl_public.h"
#ifdef CAMERA_SUPPORT

#include "eng_string.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "ctl_preview.h"
#include "fwl_osmalloc.h"
#include "fwl_keyhandler.h"
#include "eng_keymapping.h"
#include "eng_screensave.h"
#include "Lib_state.h"
#include "Eng_topbar.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_AutoPowerOff.h"
#include "Ctl_Fm.h"
#include "Eng_font.h"
#include "Fwl_Image.h"
#include "fwl_oscom.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "Eng_dynamicfont.h"
#include "akos_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"
#include "Ctl_VideoRecord.h"
#include "log_videoZoom.h"
#include "Fwl_DispOsd.h"


static T_PREVIEW *pPreview = AK_NULL;

extern T_pDATA pGB_dispBuffBackup;
extern T_U16 * Eng_StrMbcs2Ucs_2(const T_S8 *src, T_U16 *ucBuf);
extern T_VOID ToolBar_DelButtonOption(T_pTOOLBAR pToolBar, T_U32 ButtonId);



static T_VOID CamPrev_resumeFunc(T_VOID);
static T_VOID CamPrev_suspendFunc(T_VOID);

static T_U8   CamPrev_HandleTB(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
static T_VOID CamPrev_HandleDC(T_EVT_PARAM *pEventParm);
static T_VOID CamPrev_HandleDV(T_EVT_PARAM *pEventParm);
static T_VOID CamPrev_SetCB(T_VOID);

static T_VOID CamPrev_CapRecSwitchCB(T_U32 CamMode);
static T_VOID CamPrev_ModeSelectCB(T_U32 ShotMode);
static T_VOID CamPrev_CamSizeCB(T_U32 CamSize);
static T_VOID CamPrev_Brightness_ClickCB(T_eKEY_ID KeyId);
static T_VOID CamPrev_Brightness_ShowCB(T_RECT EditItemRect);
static T_VOID CamPrev_Contrast_ClickCB(T_eKEY_ID KeyId);
static T_VOID CamPrev_Contrast_ShowCB(T_RECT EditItemRect);
static T_VOID CamPrev_Saturation_ClickCB(T_eKEY_ID KeyId);
static T_VOID CamPrev_Saturation_ShowCB(T_RECT EditItemRect);
static T_VOID CamPrev_NightModeCB(T_U32 NightModeFlag);
static T_VOID CamPrev_CapDelayCB(T_U32 DelayTime);
static T_VOID CamPrev_FlashLightCB(T_U32 OpenFlag);
static T_VOID CamPrev_CapQualityCB(T_U32 CapQuality);
static T_VOID CamPrev_RecFileTypeCB(T_U32 fileType);
static T_VOID CamPrev_DetectModeCB(T_U32 DetectMode);
static T_VOID CamPrev_CycleModeCB(T_U32 CycleMode);
#endif

//////////////////////////////////////////////////////////////////////
T_VOID initcamera_preview(T_VOID)
{
#ifdef CAMERA_SUPPORT
	
	T_USTR_FILE   videoRecPath;
	T_USTR_FILE   imageRecPath;

	//Fwl_Print(C3, M_CAMERA,  "initcamera_preview 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000, (*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023
	Fwl_SetMultiChannelDisp(AK_TRUE);
    TopBar_DisableShow();
    Standby_FreeUserBkImg();
    AudioPlayer_Stop();
	Fwl_AudioDisableDA();
    ScreenSaverDisable();
    Fwl_SetAudioVolumeStatus(AK_FALSE);
    
     /**disable auto power off*/
    AutoPowerOffDisable(FLAG_CAMERA);

    /*malloc preview*/
    pPreview = (T_PREVIEW*)Fwl_Malloc(sizeof(T_PREVIEW));
    AK_ASSERT_PTR_VOID(pPreview, "initcamera_preview(): pPreview malloc fail\n");

    memset(pPreview, 0, sizeof(T_PREVIEW));

	pPreview->AniLevel = gs.AniSwitchLevel;
	GE_SetAniSwitchLevel(eClose);

    /**display waitbox before sensor initial ok*/
    WaitBox_Start(WAITBOX_RAINBOW, (T_pWSTR)GetCustomString(csINITIALIZING));

    if (!Preview_Init(pPreview))
    {
        return;
	}
    if (!VideoRecord_IconPreLoad(&pPreview->recordPaintData))
    {
        return;
	}
    /**camera init*/
    pPreview->bCameraInit = Fwl_CameraInit();
    
    
    if (!pPreview->bCameraInit)
    {
        Fwl_Print(C3, M_CAMERA,  "initcamera_preview(): camera init fail\n");
        return;
    }

	gb.nZoomInMultiple = 0;
	gb.isDetectMode    = 0;// Detect
	
	pPreview->bCameraOpen = Preview_CamStrmOpen(pPreview);
    if (!pPreview->bCameraOpen)
    {
        Fwl_Print(C3, M_CAMERA,  "initcamera_preview(): camera open fail\n");
        return;
    }
	
    /**set camera feature*/
    if (!Preview_SetFeature(pPreview))
    {
        Fwl_Print(C3, M_CAMERA,  "initcamera_preview(): set camera feature fail\n");
        return;
    }
    

    m_regResumeFunc(CamPrev_resumeFunc);
    m_regSuspendFunc(CamPrev_suspendFunc);

    gb.AlarmForbidFlag = 1;
    pPreview->SuspendToCam = AK_FALSE;
	
	if (gb.ChangePath)
	{
		AK_DEBUG_OUTPUT("gb.ChangePath\n");
		gb.ChangePath = AK_FALSE;
		Eng_StrMbcs2Ucs((VIDEOREC_PATH), videoRecPath);
		Eng_StrMbcs2Ucs((IMAGEREC_PATH), imageRecPath);
		Fwl_SetDefPath(eVIDEOREC_PATH, (T_pWSTR)videoRecPath);
		Fwl_SetDefPath(eIMAGEREC_PATH, (T_pWSTR)imageRecPath);

		Printf_UC(Fwl_GetDefPath(eVIDEOREC_PATH));
		Printf_UC(Fwl_GetDefPath(eIMAGEREC_PATH));
	}
    
#endif
}

T_VOID exitcamera_preview(T_VOID)
{
	//Fwl_Print(C3, M_CAMERA,  "exitcamera_preview 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000, (*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023

#ifdef CAMERA_SUPPORT
    WaitBox_Stop();
    TopBar_EnableShow();

    Preview_Free(pPreview);

	GE_SetAniSwitchLevel(pPreview->AniLevel);

    if (AK_NULL != pPreview)
    {
        pPreview = Fwl_Free(pPreview);
        pPreview = AK_NULL;
    }

	pGB_dispBuffBackup = Fwl_Free(pGB_dispBuffBackup);
	
    ScreenSaverEnable();
    Fwl_SetAudioVolumeStatus(AK_TRUE);

    /**enable auto power off*/
    AutoPowerOffEnable(FLAG_CAMERA);
	
    gb.AlarmForbidFlag = 0;
    GE_ShadeCancel();
	gb.nZoomInMultiple = 0;
	gb.isDetectMode = 0;//stop Detect


	Fwl_AudioDisableDA();
	Standby_LoadUserBkImg();
	AK_Sleep(20);//加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
	Fwl_Osd_DisplayOff();
	Fwl_SetMultiChannelDisp(AK_FALSE);
	
#endif
}


T_VOID paintcamera_preview(T_VOID)
{
#ifdef CAMERA_SUPPORT
    T_U16 refresh;

    refresh = Preview_GetRefresh(pPreview);
    /**reverse for future*/

    if (refresh & CAMERA_PREVIEW_REFRESH_YUV)
    {
        WaitBox_Stop();
        Preview_Invalidate_YUV(pPreview);
    }

    Preview_SetRefresh(pPreview, CAMERA_PREVIEW_REFRESH_NONE);
#endif
}

unsigned char handlecamera_preview(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef CAMERA_SUPPORT
    T_eBACK_STATE retState = eStay;
    T_BOOL  isInitOk = AK_FALSE;
    
    if (IsPostProcessEvent(event))
    {
        return 1;
	}

	if (M_EVT_USER_KEY == event)
	{
		gbCameraPreviewOsdFreshFlag = AK_TRUE;
	}

    if (M_EVT_3 == event)
    {
		WaitBox_Stop();
        Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);

        isInitOk = (pPreview->bCameraInit && pPreview->bCameraOpen);
        if (isInitOk)
        {
			isInitOk = Preview_CameraStart(pPreview);
        }
		
        if (!isInitOk)
        {
            Preview_DispInfoMsgbox(pPreview, GetCustomTitle(ctERROR), GetCustomString(csCAMERA_INIT_FAILED), 0);
            AK_Sleep(200);
            /**exit if init fail*/
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            return 0;
		}
        Fwl_Print(C3, M_CAMERA,  "+++++++Preview_CameraStart\n");
        
        return 0;
    }


    // toolbar is opened! handle toolbar prior !
    if (VME_EVT_TIMER != event && pPreview->pToolBar)
    {
        return CamPrev_HandleTB(event, pEventParm);
    }

    retState = Preview_handle(pPreview, event, pEventParm);
    
    switch(retState)
    {
    case eReturn:
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
        
    case eNext:
        pEventParm->p.pParam1 = (T_pVOID*)pPreview;
        pPreview->SuspendToCam = AK_TRUE;

        switch(gs.CamMode)
        {
        case CAM_DC:
			VideoZoom_CloseVideoSrc(pPreview->hZoom);
            
			if (gb.PowerLowBattery)
			{
				MsgBox_InitAfx(&pPreview->msgbox, 2, ctWARNING, csLOW_BATTERY, MSGBOX_INFORMATION);
				MsgBox_SetDelay(&pPreview->msgbox, MSGBOX_DELAY_0);
				m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPreview->msgbox);
			}
			else
			{
            	CamPrev_HandleDC(pEventParm);
			}
			
            break;

        case CAM_DV:        	
			if (gb.PowerLowBattery)
			{
				MsgBox_InitAfx(&pPreview->msgbox, 2, ctWARNING, csLOW_BATTERY, MSGBOX_INFORMATION);
				MsgBox_SetDelay(&pPreview->msgbox, MSGBOX_DELAY_0);
				m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPreview->msgbox);
			}
			else
			{
            	CamPrev_HandleDV(pEventParm);
			}
			
            break;
            
        default:
            Fwl_Print(C3, M_CAMERA,  "handlecamera_preview(): Illegal Mode!\n");
            break;            
        }
        break;

    /* camera menu */
    case eMenu:        
        // 正常情况下， 此处应该为AK_NULL == pPreview->pToolBar的。否则说明程序处理有问题。
        AK_ASSERT_VAL(!pPreview->pToolBar, "You want to open the toolbar! But there may be some error! here!\n", 0);

		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
        Preview_ToolBarNew(pPreview);

        // DC_MULTI_SHOT Mode, Cancel CAPTURE SIZE Toolbar,  Default Size 640*480.
        if ((gs.CamMode == CAM_DC) && (gs.DCShotMode == DC_MULTI_SHOT))
            ToolBar_DisableButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id);

        // 设置Focus Button 为上一次最后设置的Focus Button。 
        // 如果已经退出过照相机模块，再进照相机模块，pPreview->OldFocuBtnId初始化为ERROR_BUTTON_ID了。
        if (pPreview->OldFocuBtnId != ERROR_BUTTON_ID
        	&& !ToolBar_SetFocusButtonById(pPreview->pToolBar, pPreview->OldFocuBtnId))
        {
            Fwl_Print(C3, M_CAMERA,  "ToolBar_SetFocusButton By Id: %d failed! so the focus button is the head button!",\
            	pPreview->OldFocuBtnId);
        }

		CamPrev_SetCB(); 

		if (gb.isDetectMode && (gs.CamMode == CAM_DV))	//Menu ToolBar is showing,Detect stop
	    {
			VideoZoom_DetectEnable(pPreview->hZoom,AK_FALSE);
		}
		
        break;
        
    default:
        break;
        
    }
#endif
    return 0;
}

#ifdef CAMERA_SUPPORT
static T_VOID CamPrev_resumeFunc(T_VOID)
{		
	Fwl_SetMultiChannelDisp(AK_TRUE);
    TopBar_DisableShow();

    // suspend to other state,not capture or record ,when resume back, 
    if (!pPreview->SuspendToCam)
    {
        Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    }
    
    Preview_UpdateVar(pPreview);
    if (ERROR_TIMER != pPreview->prev_timer_id)
    {
        pPreview->prev_timer_id = Fwl_StopTimer(pPreview->prev_timer_id);
        pPreview->prev_timer_id = ERROR_TIMER;
    }
	
	
	if (gs.CamMode == CAM_DC)
	{
		VideoZoom_OpenVideoSrc(pPreview->hZoom, pPreview->prev_window_width,
			                   pPreview->prev_window_height);
	}
	
	else if (gs.CamMode == CAM_DV)
	{
        VideoZoom_RestartVideoSrc(pPreview->hZoom, pPreview->prev_window_width,
			                      pPreview->prev_window_height);
	}
    	
	Preview_SetFeature(pPreview);//add by hxq 2011.8.30.replace above codes.
	
	if (gs.CamMode == CAM_DV)
	{
		VideoZoom_DetectEnable(pPreview->hZoom,gb.isDetectMode);
	}
	

	pPreview->prev_timer_id = Fwl_SetTimerMilliSecond(PREVIEW_INTERVAL, AK_TRUE);

    // drop bad frame
    Preview_CameraStart(pPreview);
	
}

static T_VOID CamPrev_suspendFunc(T_VOID)
{
    /**kill waitbox*/
    WaitBox_Stop();
    
    if (!pPreview->SuspendToCam) //非进入录像界面需要停止移动侦测
		VideoZoom_DetectEnable(pPreview->hZoom,AK_FALSE);
	
    if (ERROR_TIMER != pPreview->prev_timer_id)
    {
        pPreview->prev_timer_id = Fwl_StopTimer(pPreview->prev_timer_id);
        pPreview->prev_timer_id = ERROR_TIMER;
    }
    
    if (ERROR_TIMER != pPreview->zoomlevel_shown_timer_id)
    {
        Fwl_StopTimer(pPreview->zoomlevel_shown_timer_id);
        pPreview->zoomlevel_shown_timer_id = ERROR_TIMER;
    }

    if (!pPreview->SuspendToCam)
    {
        /**for background show black when display message box*/
        Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    }
    else
    {
        pPreview->SuspendToCam = AK_FALSE;
    }
	AK_Sleep(20);//加延迟保证刷新完毕
	Fwl_TurnOff_YUV();
	Fwl_Osd_DisplayOff();
	Fwl_SetMultiChannelDisp(AK_FALSE);

}


// Toolbar Handle
static T_U8 CamPrev_HandleTB(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	T_eBACK_STATE retState = eStay;	

	VideoZoom_DetectEnable(pPreview->hZoom,AK_FALSE);
	
	retState = ToolBar_Handler(pPreview->pToolBar, event, pEventParm);
	
	if (eNext == retState
		&& pPreview->pToolBar->pFocusBtn->Id == pPreview->ButtonIcon.SavePath.Id)
	{
		if (gs.CamMode == CAM_DV)
		{
			pEventParm->w.Param1 = eVIDEOREC_PATH;
		}
		else	//gs.CamMode == CAM_DC
		{
			pEventParm->w.Param1 = eIMAGEREC_PATH;
		}

		GE_ShadeCancel();
		m_triggerEvent(M_EVT_SETPATH, pEventParm);

		// Switch To RGB, Refresh Monitor
//		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
//		Fwl_RefreshDisplay();	
			
	}

	if (eReturn == retState)	//Cancel or OK from the toolbar.
	{
		pPreview->OldFocuBtnId = pPreview->pToolBar->pFocusBtn->Id;
		Preview_ToolBarFree(pPreview);

		gbCameraPreviewOsdFreshFlag = AK_TRUE;

		if (gb.isDetectMode && (gs.CamMode == CAM_DV))	//Menu ToolBar disappear,Detect go on
		{
			VideoZoom_DetectEnable(pPreview->hZoom,AK_TRUE);
		}
	}
	
	return 0;
}

static T_VOID CamPrev_HandleDC(T_EVT_PARAM *pEventParm)
{
	T_CAMERA_MODE photoMode = 0;
	
	switch(gs.DCShotMode)
	{
	case DC_MULTI_SHOT:
		photoMode = gs.camMultiShotMode;
		break;
					
	case DC_NORMAL_SHOT:
		photoMode = gs.camPhotoMode;
		break;
	
	default:
		photoMode = CAMERA_MODE_VGA;
		break;					
	}

    // 使用 预览的视窗
	pPreview->shotParm.width  = pPreview->prev_window_width;
	pPreview->shotParm.height = pPreview->prev_window_height;
	Fwl_GetRecFrameSize(photoMode, &pPreview->shotParm.c1, &pPreview->shotParm.c2);
	Fwl_Print(C3, M_CAMERA,  "CAM SHOT: shotParm.width = %d, shotParm.height = %d.\n", pPreview->shotParm.width, pPreview->shotParm.height);
	/* switch to capture state machine */
	m_triggerEvent(M_EVT_1, pEventParm);
	
}

static T_VOID CamPrev_HandleDV(T_EVT_PARAM *pEventParm)
{
    // 使用 预览的视窗
    pPreview->shotParm.width  = pPreview->prev_window_width;
	pPreview->shotParm.height = pPreview->prev_window_height;

	Fwl_GetRecFrameSize(gs.camRecMode[gs.CamRecFileType],&pPreview->shotParm.c1, 
	                                                     &pPreview->shotParm.c2);//视频大小

	Fwl_Print(C3, M_CAMERA,  "Set Win [%dx%d] To Rec[%dX%d]\n",
                     pPreview->shotParm.width, pPreview->shotParm.height,
                     pPreview->shotParm.c1,    pPreview->shotParm.c2);
	/* switch to recorder state machine */
	m_triggerEvent(M_EVT_2, pEventParm);

}

static T_VOID CamPrev_ShowEditStr(T_pCWSTR Ustr, T_RECT EditItemRect)
{
    //draw brightness on yuv
    T_U8    *pY = AK_NULL;
    T_U8    *pU = AK_NULL;
    T_U8    *pV = AK_NULL;
    T_U32   StrWidth;
    T_U16   strlen;
    T_RECT  strRect;

    if (pPreview->bufPtr == pPreview->pY2)
    {
        pY = pPreview->pY2;
        pU = pPreview->pU2;
        pV = pPreview->pV2;
    }
    else
    {
        pY = pPreview->pY;
        pU = pPreview->pU;
        pV = pPreview->pV;
    }

    strlen = (T_U16)Utl_UStrLen(Ustr);
    StrWidth = UGetSpeciStringWidth((T_pWSTR)Ustr, CURRENT_FONT_SIZE, strlen);
    strRect.left = EditItemRect.left + (T_POS)((EditItemRect.width - StrWidth) / 2);
    strRect.top = EditItemRect.top + (EditItemRect.height - g_Font.CHEIGHT) / 2;
    
    if (pPreview->pToolBar->ShownMode == TB_eMODE_SHOWN_ON_YUV)
    {
    UDispSpeciStringOnYUV(pY, pU, pV, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, strRect.left, strRect.top, \
                          (T_pWSTR)Ustr, COLOR_BLACK, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Ustr));
    }
    else
    {
//    Fwl_UDispSpeciString(HRGB_LAYER, strRect.left, strRect.top, (T_pWSTR)Ustr, COLOR_BLACK,
//                         CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Ustr));
    Fwl_Osd_DrawUStringByGray(strRect.left, strRect.top, (T_pWSTR)Ustr, (T_U16)Utl_UStrLen(Ustr)
    					,COLOR_WHITE,CURRENT_FONT_SIZE);

                         
    }
}

static T_VOID CamPrev_CapRecSwitchCB(T_U32 CamMode)
{
    if (gs.CamMode == CAM_DV)
    {
        gs.CamMode = CAM_DC;        
    }
    else if (gs.CamMode == CAM_DC)
    {
        gs.CamMode = CAM_DV;
    }
    
    Preview_SwitchCamMode(pPreview, gs.CamMode);
	
	pPreview->OldFocuBtnId = pPreview->pToolBar->pFocusBtn->Id;
	Preview_ToolBarFree(pPreview);
}

static T_VOID CamPrev_ModeSelectCB(T_U32 ShotMode)
{
    T_U32 width;
    T_U32 height;
    
    if (gs.CamMode == CAM_DV)
    {
        Fwl_Print(C3, M_CAMERA,  "CAM_DV mode! there maybe some error in the function!\n");
        return;
    }

    if (gs.DCShotMode != ShotMode)
    {
        gs.DCShotMode= (T_U8)ShotMode;
        
        Preview_GetCurWinSize(pPreview, CAM_DC, &width, &height);
        Preview_ResetWindow(pPreview, width, height);

        if (DC_MULTI_SHOT == gs.DCShotMode)
        {
            ToolBar_DisableButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id);
        }
        else
        {
            ToolBar_EnableButton(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id);
        }
    }
}

static T_VOID CamPrev_CamSizeCB(T_U32 CamSize)
{
    T_U32 width;
    T_U32 height;
	T_U8 i;
    
    if (gs.CamMode == CAM_DC)
    {
        gs.camPhotoMode = (T_CAMERA_MODE)CamSize;
    }
    else if (gs.CamMode == CAM_DV)
    {	
    	for(i=0; i<eRECORD_MEDIA_NUM; i++)
    	{
        	gs.camRecMode[i] = (T_CAMERA_MODE)CamSize;
    	}
    }
    
    Preview_GetCurWinSize(pPreview, gs.CamMode, &width, &height);
    Preview_ResetWindow(pPreview, width, height);
}

static T_VOID CamPrev_Brightness_ClickCB(T_eKEY_ID KeyId)
{
    if (kbLEFT == KeyId && Fwl_CameraBrightnessCanDec(gs.CamBrightness))
    {
        gs.CamBrightness--;
    }
    else if (kbRIGHT == KeyId && Fwl_CameraBrightnessCanInc(gs.CamBrightness))
    {
        gs.CamBrightness++;
    }
    else
    {
        Fwl_Print(C3, M_CAMERA,  "inavalid key!!!\n");
        return;
    }

    pPreview->brightness = gs.CamBrightness;
    Fwl_CameraChangeBrightness(gs.CamBrightness);
}

static T_VOID CamPrev_Brightness_ShowCB(T_RECT EditItemRect)
{
    //draw brightness on yuv
    T_USTR_INFO UstrTmp;

    Utl_UItoa((T_S32)gs.CamBrightness, UstrTmp, 8);
    CamPrev_ShowEditStr(UstrTmp, EditItemRect);
}

static T_VOID CamPrev_Contrast_ClickCB(T_eKEY_ID KeyId)
{
    if (kbLEFT == KeyId && Fwl_CameraContrastCanDec(gs.CamContrast))
    {
        gs.CamContrast--;
    }
    else if (kbRIGHT == KeyId && Fwl_CameraContrastCanInc(gs.CamContrast))
    {
        gs.CamContrast++;
    }
    else
    {
        Fwl_Print(C3, M_CAMERA,  "inavalid key!!!\n");
        return;
    }

    Fwl_CameraChangeContrast(gs.CamContrast);
}

static T_VOID CamPrev_Contrast_ShowCB(T_RECT EditItemRect)
{
    //draw contrast value on yuv
    T_USTR_INFO UstrTmp;

    Utl_UItoa((T_S32)gs.CamContrast, UstrTmp, 8);
    CamPrev_ShowEditStr(UstrTmp, EditItemRect);
}

static T_VOID CamPrev_Saturation_ClickCB(T_eKEY_ID KeyId)
{
    if (kbLEFT == KeyId && Fwl_CameraSaturationCanDec(gs.CamSaturation))
    {
        gs.CamSaturation--;
    }
    else if (kbRIGHT == KeyId && Fwl_CameraSaturationCanInc(gs.CamSaturation))
    {
        gs.CamSaturation++;
    }
    else
    {
        Fwl_Print(C3, M_CAMERA,  "inavalid key!!!\n");
        return;
    }
	Fwl_Print(C3, M_CAMERA,  "hxq  CamPrev_Saturation_ClickCB--gs.CamSaturation = %d\n",gs.CamSaturation);//hxq.
    Fwl_CameraChangeSaturation(gs.CamSaturation);

}

static T_VOID CamPrev_Saturation_ShowCB(T_RECT EditItemRect)
{
    //draw Saturation value on yuv
    T_USTR_INFO UstrTmp;

    Utl_UItoa((T_S32)gs.CamSaturation, UstrTmp, 8);
    CamPrev_ShowEditStr(UstrTmp, EditItemRect);
}

static T_VOID CamPrev_NightModeCB(T_U32 NightModeFlag)
{
	Fwl_CamerChangeNightMode(&(gs.camFlashMode));
    Fwl_CameraSetNightMode((T_U8)gs.camFlashMode);
}

static T_VOID CamPrev_CapDelayCB(T_U32 DelayTime)
{
    if (DelayTime != (T_U32)gs.ShotDelayCount)
    {
        gs.ShotDelayCount = (T_U8)DelayTime;
        pPreview->shot_delay_count = gs.ShotDelayCount;
    }
}

static T_VOID CamPrev_FlashLightCB(T_U32 OpenFlag)
{
    if (!gs.CamFlashlight)
    {
        gs.CamFlashlight = AK_TRUE;
    }
    else
    {
        gs.CamFlashlight = AK_FALSE;
    }

    pPreview->bFlashLight = gs.CamFlashlight;
}

static T_VOID CamPrev_CapQualityCB(T_U32 CapQuality)
{
    gs.camPhotoQty = (T_U8)CapQuality;
}

static T_VOID CamPrev_RecFileTypeCB(T_U32 fileType)
{
    gs.CamRecFileType = (T_U8)fileType;

	ToolBar_DelButtonOption(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id);
	Preview_ToolBarInitRecFileType(pPreview);
	ToolBar_SetFocusOption(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, gs.camRecMode[gs.CamRecFileType]);
}

static T_VOID CamPrev_DetectModeCB(T_U32 DetectMode)
{
    gb.isDetectMode = (T_U8)DetectMode;
}

static T_VOID CamPrev_CycleModeCB(T_U32 CycleMode)
{
    gb.isCycMode = (T_U8)CycleMode;
}

static T_VOID CamPrev_KacaModeCB(T_U32 openFlag)
{
	gs.camSoundSw = (T_U16)openFlag;
}


static T_VOID CamPrev_SetCB(T_VOID)
{
	//then set call back functions for all the buttons.
    ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.CapRecSwitch.Id, \
                                                CamPrev_CapRecSwitchCB);

    if (gs.CamMode == CAM_DC)
    {
    	ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.ModeSelect.Id, \
                                                    CamPrev_ModeSelectCB);
    	ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.DelayCap.Id, \
                                                    CamPrev_CapDelayCB);
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.FlashLight.Id, \
                                                    CamPrev_FlashLightCB);
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.PhotoQuality.Id, \
                                                    CamPrev_CapQualityCB);
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.KacaMode.Id, \
                                                    CamPrev_KacaModeCB);
    }
    else
    {
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.RecFileType.Id, \
                                                    CamPrev_RecFileTypeCB);
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.DetectMode.Id, \
                                                    CamPrev_DetectModeCB);
        ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.CycleMode.Id, \
                                                    CamPrev_CycleModeCB);
    }

    ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.PhotoSize.Id, \
                                                CamPrev_CamSizeCB);
    ToolBar_SetEditButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.Brightness.Id, \
                                               CamPrev_Brightness_ClickCB);
    ToolBar_SetEditButtonShowCB(pPreview->pToolBar, pPreview->ButtonIcon.Brightness.Id, \
                                              CamPrev_Brightness_ShowCB);
    ToolBar_SetEditButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.Contrast.Id, \
                                               CamPrev_Contrast_ClickCB);
    ToolBar_SetEditButtonShowCB(pPreview->pToolBar, pPreview->ButtonIcon.Contrast.Id, \
                                              CamPrev_Contrast_ShowCB);
    ToolBar_SetEditButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.Saturation.Id, \
                                               CamPrev_Saturation_ClickCB);
    ToolBar_SetEditButtonShowCB(pPreview->pToolBar, pPreview->ButtonIcon.Saturation.Id, \
                                              CamPrev_Saturation_ShowCB);
    ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.NightMode.Id, \
                                                CamPrev_NightModeCB);
    /* ToolBar_SetNormalButtonClickCB(pPreview->pToolBar, pPreview->ButtonIcon.SetPath.Id, \
                                                AK_NULL);
         */
}

#endif

