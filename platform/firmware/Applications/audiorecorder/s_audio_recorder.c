/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd.
 * All rights reserved.
 *
 * File Name：s_audio_recorder.c
 * Function：process voice recording function
 *
 * Author：Liu_Zhenwu
 * Date：2006-03-04
 * Version：1.0
 *
 * Modify note: 2012-3-3 by hoube.
 *
 * Reversion:
 * Author:
 * Date:
**************************************************************************/
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Ctl_Progress.h"
#include "Ctl_MsgBox.h"
#include "Eng_KeyMapping.h"
#include "Eng_ScreenSave.h"
#include "Eng_Graph.h"
#include "Lib_res_port.h"
#include "Fwl_pfAudio.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "Eng_String.h"
#include "Eng_String_UC.h"
#include "Ctl_Fm.h"
#include "fwl_oscom.h"
#include "fwl_pfkeypad.h"
#include "Eng_Math.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "AKError.h"
#include "akos_api.h"
#include "fwl_display.h"
#include "log_media_recorder.h"
#include "Fwl_tscrcom.h"
#include "Ctl_RecAudio.h"

#define AUDIOREC_REFRESH_ALL                    0xFF
#define AUDIOREC_REFRESH_NONE                   0x00

#define AUDIOREC_REFRESH_STATUS                 0x01
#define AUDIOREC_REFRESH_PROGRESS               0x02
#define AUDIOREC_REFRESH_TIME                   0x04
#define AUDIOREC_REFRESH_AUDIO_WAVE             0x08

#define AUDIOREC_TIME_INTERVAL             		200   // the interval of refresh time in recording

typedef enum {
	eDISPLAY_NOERROR = 0,
	eDISPLAY_REC_NO_SPACE,		//no record space error
	eDISPLAY_SAVE_FAILED,		//save failed error
	eDISPLAY_SAVE_OK,			//save ok

	eDISPLAY_UNKNOWN			//unknown error
} T_eMSGBOX_DISCODE;


typedef struct {
    T_pCDATA        pBckGrnd;
    T_pCDATA        pProgBar;
    T_pCDATA        pProgBlck;
    T_pCDATA        pTmBckGrnd;
    T_pCDATA        pStatIcon[2];
    T_pCDATA        pEqBckGrnd;
    T_pCDATA        pEqPnt0;
    T_pCDATA        pEqPnt1;
    T_pCDATA        pNumImag[10];
    T_pCDATA        pSeparator;
    
    T_RECT          BckgrndRect;
    T_RECT          ProgBarRect;
    T_RECT          ProgBlckRect;
    T_RECT          TmBckGrndRect;
    T_RECT          StatIconRect;
    T_RECT          EqBckGrndRect;
    T_RECT          EqPnt0Rect;
    T_RECT          EqPnt1Rect;
    T_RECT          NumImagRect;
    T_RECT          SeparatorRect;

    T_RECT          RcrTmRect;
} T_AUDIOREC_RESOURCE;

/* record context */
typedef struct {
    T_MSGBOX        	MsgBox;       	//message box
    
	T_AUDIOREC_RESOURCE	res;
    T_U8            	refresh;        //refresh flag
    T_TIMER         	refresh_time_id;
	T_SYSTIME			display_time;	//for display

	T_S32				isExitWarning;
} T_AUDIOREC_SM;

/* record info */
static T_AUDIOREC_SM *pAudioRecSM = AK_NULL;


static T_VOID audioRcrd_LoadRes(T_VOID);
static T_VOID audioRcrd_ShowRcrdTime(T_AUDIOREC_RESOURCE  *pAudioRcrdRes);
static T_VOID audioRcrd_ShowRcrdStateIcon(T_AUDIOREC_RESOURCE  *pAudioRcrdRes);
static T_VOID audioRcrd_ShowRcrdProgress(T_AUDIOREC_RESOURCE  *pAudioRcrdRes);
static T_VOID audioRcrd_ShowRcrdEqualizer(T_AUDIOREC_RESOURCE  *pAudioRcrdRes);
static T_VOID audioRcrd_RstDisplayTime(T_VOID);
static T_VOID audioRcrd_ShowExitWarning(T_VOID);


static T_VOID audioRcrd_LoadRes(T_VOID)
{
    T_AUDIOREC_RESOURCE  *pAudioRcrRes = &pAudioRecSM->res;
	
    T_pRECT pBckgrndRect 	= &pAudioRecSM->res.BckgrndRect;
    T_pRECT pProgBarRect 	= &pAudioRecSM->res.ProgBarRect;
    T_pRECT pProgBlckRect 	= &pAudioRecSM->res.ProgBlckRect;
    T_pRECT pTmBckGrndRect 	= &pAudioRecSM->res.TmBckGrndRect;
    T_pRECT pStatIconRect 	= &pAudioRecSM->res.StatIconRect;
    T_pRECT pEqBckGrndRect 	= &pAudioRecSM->res.EqBckGrndRect;
    T_pRECT pEqPnt0Rect 	= &pAudioRecSM->res.EqPnt0Rect;
    T_pRECT pEqPnt1Rect 	= &pAudioRecSM->res.EqPnt1Rect;
    T_pRECT pNumImagRect 	= &pAudioRecSM->res.NumImagRect;
    T_pRECT pSeparatorRect 	= &pAudioRecSM->res.SeparatorRect;
    T_pRECT pRcrTmRect 		= &pAudioRecSM->res.RcrTmRect;
    T_U32   i = 0;

    //background
    pAudioRcrRes->pBckGrnd  = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_BACKGROUND, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pBckGrnd, &pBckgrndRect->width, &pBckgrndRect->height, AK_NULL);
    pBckgrndRect->left = (Fwl_GetLcdWidth() - pBckgrndRect->width) / 2;
    pBckgrndRect->top = (Fwl_GetLcdHeight() - pBckgrndRect->height) / 2;

    //progress bar
    pAudioRcrRes->pProgBar = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_PROGRESS_BAR, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pProgBar, &pProgBarRect->width, &pProgBarRect->height, AK_NULL);
    pProgBarRect->left = pBckgrndRect->left + (pBckgrndRect->width - pProgBarRect->width) / 2;
    pProgBarRect->top = pBckgrndRect->top + (pBckgrndRect->height - TOP_BAR_HEIGHT);

    //progress block
    pAudioRcrRes->pProgBlck = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_PROGRESS_BLOCK, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pProgBlck, &pProgBlckRect->width, &pProgBlckRect->height, AK_NULL);
    pProgBlckRect->left = pProgBarRect->left;
    pProgBlckRect->top = pProgBarRect->top + (pProgBarRect->height - pProgBlckRect->height) / 2;

    //record status
    pAudioRcrRes->pStatIcon[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_STATUS_STOP, AK_NULL);
    pAudioRcrRes->pStatIcon[1] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_STATUS_START, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pStatIcon[0], &pStatIconRect->width, &pStatIconRect->height, AK_NULL);
    pStatIconRect->left = pBckgrndRect->width / 2;
    pStatIconRect->top = pProgBarRect->top - TOP_BAR_HEIGHT - pStatIconRect->height;

    //record time background
    pAudioRcrRes->pTmBckGrnd = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_PROGRESS_TIME, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pTmBckGrnd, &pTmBckGrndRect->width, &pTmBckGrndRect->height, AK_NULL);
    pTmBckGrndRect->left = pStatIconRect->left;
    pTmBckGrndRect->top = pStatIconRect->top - TOP_BAR_HEIGHT- pTmBckGrndRect->height;

    //equalizer BackGround
    pAudioRcrRes->pEqBckGrnd = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_EQUALIZER_BACKGROUND, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pEqBckGrnd, &pEqBckGrndRect->width, &pEqBckGrndRect->height, AK_NULL);
    pEqBckGrndRect->left = pTmBckGrndRect->left + pTmBckGrndRect->width - pEqBckGrndRect->width;
    pEqBckGrndRect->top = pStatIconRect->top + (pStatIconRect->height - pEqBckGrndRect->height) / 2;
    
    //equalizer Point0
    pAudioRcrRes->pEqPnt0 = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_EQUALIZER_POINT0, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pEqPnt0, &pEqPnt0Rect->width, &pEqPnt0Rect->height, AK_NULL);
    pEqPnt0Rect->left= pEqBckGrndRect->left + 3;
    pEqPnt0Rect->top = pEqBckGrndRect->top + 5;

    //equalizer Point1
    pAudioRcrRes->pEqPnt1 = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIOREC_EQUALIZER_POINT1, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pEqPnt1, &pEqPnt1Rect->width, &pEqPnt1Rect->height, AK_NULL);
    pEqPnt1Rect->left = pEqPnt0Rect->left;
    pEqPnt1Rect->top = pEqPnt0Rect->top + pEqPnt0Rect->height + 1;

    //number image
    for (i=0; i<10; i++)
    {
        pAudioRcrRes->pNumImag[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_NUMBER_0 + i, AK_NULL);
    }
    AKBmpGetInfo(pAudioRcrRes->pNumImag[0], &pNumImagRect->width, &pNumImagRect->height, AK_NULL);
    pNumImagRect->left = 0;
    pNumImagRect->top = 0;

    //separator
    pAudioRcrRes->pSeparator = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_COLON, AK_NULL);
    AKBmpGetInfo(pAudioRcrRes->pSeparator, &pSeparatorRect->width, &pSeparatorRect->height, AK_NULL);
    pSeparatorRect->left = 0;
    pSeparatorRect->top = 0;

    //record time rect
    pRcrTmRect->width = 8 * pNumImagRect->width + 6;    //6表示每个数字的图片间还有一个像素宽的间隔
    pRcrTmRect->height = pNumImagRect->height;
    pRcrTmRect->left = pTmBckGrndRect->left + (pTmBckGrndRect->width - pRcrTmRect->width) / 2;
    pRcrTmRect->top = pTmBckGrndRect->top + (pTmBckGrndRect->height - pRcrTmRect->height) / 2;
}


/*
 *@brief: show record info
 */
T_VOID audioRcrd_ShowRecInfo(T_eMSGBOX_DISCODE discode, T_pCWSTR content, T_U16 retLevel)
{
    T_U16   utext[2];

    utext[0] = 0;

    MsgBox_InitStr(&pAudioRecSM->MsgBox, retLevel, utext, utext, MSGBOX_INFORMATION);
    switch (discode)
    {
	case eDISPLAY_REC_NO_SPACE:
		MsgBox_CatText(&pAudioRecSM->MsgBox, GetCustomString(csSAVESPACEFULL));
		break;

    case eDISPLAY_SAVE_FAILED:
       	MsgBox_CatText(&pAudioRecSM->MsgBox, GetCustomString(csAUDIOREC_SAVE_FAILURE));
     	break;

    case eDISPLAY_SAVE_OK:
	  	MsgBox_CatText(&pAudioRecSM->MsgBox, GetCustomString(csAUDIOREC_SAVE_OK));
		break;

    case eDISPLAY_UNKNOWN:
		break;
		
    default:
    	break;
    }

    if (AK_NULL != content)
    {
        MsgBox_AddLine(&pAudioRecSM->MsgBox, content);
    }

    MsgBox_SetDelay(&pAudioRecSM->MsgBox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (vT_EvtParam*)&pAudioRecSM->MsgBox);
    pAudioRecSM->refresh = AUDIOREC_REFRESH_ALL; //need to refresh
}


/*
 *@brief: clear record display or play display
 */
static T_VOID audioRcrd_RstDisplayTime(T_VOID)
{
    pAudioRecSM->display_time.second = 0;
    pAudioRecSM->display_time.minute = 0;
    pAudioRecSM->display_time.hour = 0;

	pAudioRecSM->refresh |= AUDIOREC_REFRESH_TIME|AUDIOREC_REFRESH_PROGRESS;
}


static T_VOID audioRcrd_ShowRcrdTime(T_AUDIOREC_RESOURCE  *pAudioRcrdRes)
{
    T_pRECT     pNumImgRect = &pAudioRcrdRes->NumImagRect;
    T_pRECT     pRcrTmRect = &pAudioRcrdRes->RcrTmRect;
    T_pRECT     pTmBckGrndRect = &pAudioRcrdRes->TmBckGrndRect;
    T_U32       num = 0;
    T_POS       showPosX = 0;
    T_U8        hour, minute, second;
    
    if ((pAudioRecSM->refresh & AUDIOREC_REFRESH_TIME) >  0)
    {
        hour    = pAudioRecSM->display_time.hour;
        minute  = pAudioRecSM->display_time.minute;
        second  = pAudioRecSM->display_time.second;

        //show time rect background        
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pTmBckGrndRect->left, pTmBckGrndRect->top,
                        pAudioRcrdRes->pTmBckGrnd, AK_NULL, AK_FALSE);

        //show time:second
        showPosX = pRcrTmRect->left + pRcrTmRect->width - pNumImgRect->width - 1; 
        num = second % 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);

        showPosX = showPosX - pNumImgRect->width - 1; 
        num = second / 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);

        //show separator
        showPosX = showPosX - pNumImgRect->width; 
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pSeparator,
                            &g_Graph.TransColor, AK_FALSE);

        //show minute
        showPosX = showPosX - pNumImgRect->width - 1; 
        num = minute % 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);

        showPosX = showPosX - pNumImgRect->width - 1; 
        num = minute / 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);

        //show separator
        showPosX = showPosX - pNumImgRect->width; 
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pSeparator,
                            &g_Graph.TransColor, AK_FALSE);

        //show hour
        showPosX = showPosX - pNumImgRect->width - 1; 
        num = hour % 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);

        showPosX = showPosX - pNumImgRect->width - 1; 
        num = hour / 10;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, showPosX, pRcrTmRect->top, pAudioRcrdRes->pNumImag[num],
                            &g_Graph.TransColor, AK_FALSE);
    }
}

static T_VOID audioRcrd_ShowRcrdStateIcon(T_AUDIOREC_RESOURCE  *pAudioRcrdRes)
{
    T_pRECT pStatIcnRect = &pAudioRcrdRes->StatIconRect;

    if ((pAudioRecSM->refresh & AUDIOREC_REFRESH_STATUS) > 0)
    {
        switch (Ctl_RecAudio_GetCurState())
        {
        case eSTAT_REC_STOP:
        	Fwl_AkBmpDrawFromString(HRGB_LAYER, pStatIcnRect->left, pStatIcnRect->top,
                        pAudioRcrdRes->pStatIcon[0], &g_Graph.TransColor, AK_FALSE);
       		break;

        case eSTAT_RECORDING:
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pStatIcnRect->left, pStatIcnRect->top,
                        pAudioRcrdRes->pStatIcon[1], &g_Graph.TransColor, AK_FALSE);
            break;

        default:
            break;
        }
    }
}

static T_VOID audioRcrd_ShowRcrdProgress(T_AUDIOREC_RESOURCE  *pAudioRcrdRes)
{
    T_pRECT     pProgBarRect = &pAudioRcrdRes->ProgBarRect;
    T_pRECT     pProgBlckRect = &pAudioRcrdRes->ProgBlckRect;
    T_POS       prgStart = 0;

    if ((pAudioRecSM->refresh & AUDIOREC_REFRESH_PROGRESS) > 0)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pProgBarRect->left, pProgBarRect->top,
                        pAudioRcrdRes->pProgBar, &g_Graph.TransColor, AK_FALSE);

        if (Ctl_RecAudio_GetMaxLenCanRec() > 0)
        {
			prgStart = (T_POS)(pProgBlckRect->left + (pProgBarRect->width - pProgBlckRect->width)* ((float)Ctl_RecAudio_GetCurRecedLen() / Ctl_RecAudio_GetMaxLenCanRec()));
        }
        else
        {
            prgStart = pProgBlckRect->left;
        }

        Fwl_AkBmpDrawFromString(HRGB_LAYER, prgStart, pProgBlckRect->top, pAudioRcrdRes->pProgBlck,
                        &g_Graph.TransColor, AK_FALSE);
    }
}

static T_VOID audioRcrd_ShowRcrdEqualizer(T_AUDIOREC_RESOURCE  *pAudioRcrdRes)
{
    T_pRECT     pEqBckGrndRect = &pAudioRcrdRes->EqBckGrndRect;
    T_pRECT     pEqPntRect0 = &pAudioRcrdRes->EqPnt0Rect;
    T_pRECT     pEqPntRect1 = &pAudioRcrdRes->EqPnt1Rect;
    T_S16       wavTmp[10]={0};
    T_U16       wavValue[2] = {0, 0};
    T_U8        i = 0;

    pAudioRecSM->refresh |= AUDIOREC_REFRESH_AUDIO_WAVE;
    if ((pAudioRecSM->refresh & AUDIOREC_REFRESH_AUDIO_WAVE) > 0)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pEqBckGrndRect->left, pEqBckGrndRect->top,
                    pAudioRcrdRes->pEqBckGrnd, &g_Graph.TransColor, AK_FALSE);
        
		Ctl_RecAudio_GetCurSample(wavTmp);

        switch (gs.AudioRecordMode)
        {
        case eRECORD_MODE_AMR:
            wavValue[0] = abs(wavTmp[0])*8/0x1000;
            wavValue[1] = abs(wavTmp[1])*8/0x1000;
            break;
			
        case eRECORD_MODE_WAV:
            wavValue[0] = abs(wavTmp[0])*8/0x2000;
            wavValue[1] = abs(wavTmp[1])*8/0x2000;
            break;
			
		case eRECORD_MODE_MP3:
			wavValue[0] = abs(wavTmp[0])*8/0x2000;
			wavValue[1] = abs(wavTmp[1])*8/0x2000;
			break;
			
		case eRECORD_MODE_AAC:
			wavValue[0] = abs(wavTmp[0])*8/0x2000;
			wavValue[1] = abs(wavTmp[1])*8/0x2000;
			break;
			
        default:
            break;
        }

        if (wavValue[0] > 16)
        {
            wavValue[0] = 16;
        }

        if (wavValue[1] > 16)
        {
            wavValue[1] = 16;
        }

        for (i=0; i<wavValue[0]; i++)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, (T_POS)(pEqPntRect0->left + i*5), pEqPntRect0->top,
                            pAudioRcrdRes->pEqPnt0, &g_Graph.TransColor, AK_FALSE);
        }

        for (i=0; i<wavValue[1]; i++)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, (T_POS)(pEqPntRect1->left + i*5), pEqPntRect1->top,
                            pAudioRcrdRes->pEqPnt1, &g_Graph.TransColor, AK_FALSE);
        }
    }
}

static T_VOID audioRcrd_UserKey_HandleShortPress(T_MMI_KEYPAD phyKey)
{
    switch (Ctl_RecAudio_GetCurState())
    {
    case eSTAT_REC_STOP:
        if (kbOK == phyKey.keyID)
        {   
            AK_DEBUG_OUTPUT("\n@start to record\n");
			//begin record
            if (!Ctl_RecAudio_Start(AK_NULL, eREC_ACTION_LOOP))
            {
            	AK_DEBUG_OUTPUT("@start record failed\n\n");
				
				if (eERR_DISKSPACE_NOTENOUGH == Ctl_RecAudio_GetLastError())
				{
					audioRcrd_ShowRecInfo(eDISPLAY_REC_NO_SPACE, AK_NULL, 1);
				}
            }
        }

        if (kbCLEAR == phyKey.keyID)
        {
        	//exit recorder
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
        }
		
        break;

    case eSTAT_RECORDING: //recording now
        if (kbOK == phyKey.keyID)
        {
            //stop record
            AK_DEBUG_OUTPUT("\n@stop record\n");
			
            if (!Ctl_RecAudio_Stop(AK_TRUE))
            {
				AK_DEBUG_OUTPUT("@stop record failed\n\n");
				audioRcrd_ShowRecInfo(eDISPLAY_SAVE_FAILED, AK_NULL, 1);
			}
			else
			{
				audioRcrd_RstDisplayTime();
			}
        }

        break;

    default:
        AK_DEBUG_OUTPUT("@unknown record state!!!\n");
        break;
    }

    pAudioRecSM->refresh |= AUDIOREC_REFRESH_STATUS;
}

//handle user key events
static T_VOID audioRcrd_UserKey_Handle( T_MMI_KEYPAD phyKey, vT_EvtParam* pEventParm)
{
	if (PRESS_LONG == phyKey.pressType)
	{
		switch (phyKey.keyID)
		{
		case kbCLEAR:
			m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
			break;

		case kbOK:
			Fwl_KeyStop();
			break;

		default:	
			break;
		}
	}
	else
	{
		audioRcrd_UserKey_HandleShortPress(phyKey);
	}
}

//handle touch sreen events,  transforming it to corresponding key evnet
static T_MMI_KEYPAD audioRcrd_MapTSCR_To_Key(T_POS x, T_POS y)
{
    T_MMI_KEYPAD    phyKey;
    T_RECT          rect;
    T_AUDIOREC_RESOURCE* pResource;
    
    phyKey.keyID = kbNULL;
    phyKey.pressType = PRESS_SHORT;
    
    pResource = &pAudioRecSM->res;
    
    //get the rect of cancel button
    rect = TopBar_GetRectofCancelButton();
    
    //hit cancel button
    if (PointInRect(&rect, x, y))
    {
        phyKey.keyID = kbCLEAR;
        phyKey.pressType = PRESS_SHORT;
    }

    //hit play/pause button
    if (PointInRect(&pResource->StatIconRect, x, y))
    {
        phyKey.keyID = kbOK;
    }

    return phyKey;
}


/**
 * @author: hoube
 */
static T_VOID audioRcrd_ShowExitWarning(T_VOID)
{
	T_RECT msgRect;
	
	MsgBox_InitStr(&pAudioRecSM->MsgBox, 0, GetCustomString(csFM_DEL_ALL_NOTE),
			Res_GetStringByID(eRES_STR_IS_SAVE_AUDIO), MSGBOX_QUESTION | MSGBOX_YESNO);
	MsgBox_Show(&pAudioRecSM->MsgBox);
	MsgBox_GetRect(&pAudioRecSM->MsgBox, &msgRect);
	Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
}


void paintaudio_recorder(void);
static void resume_arec(void)
{
	T_S32 curRecTime = 0;
	
	//fresh tiime display by timer count
	curRecTime = Ctl_RecAudio_GetCurRecTime();
	pAudioRecSM->display_time.hour = (T_U8)(curRecTime/3600);
	pAudioRecSM->display_time.minute = (T_U8)((curRecTime%3600)/60);
	pAudioRecSM->display_time.second = (T_U8)(curRecTime%60);
	pAudioRecSM->refresh |= AUDIOREC_REFRESH_TIME|AUDIOREC_REFRESH_PROGRESS;

	paintaudio_recorder();
}

#endif


T_VOID initaudio_recorder(T_VOID)
{
#ifdef SUPPORT_AUDIOREC

	AK_DEBUG_OUTPUT("\n enter initaudio_recorder\n");
	
    AudioPlayer_Stop();

    //init variables here
    pAudioRecSM = (T_AUDIOREC_SM *)Fwl_Malloc(sizeof(T_AUDIOREC_SM));
    AK_ASSERT_PTR_VOID(pAudioRecSM, "pAudioRecSM malloc failed!!!\n");
    Utl_MemSet(pAudioRecSM, 0, sizeof(T_AUDIOREC_SM));

	audioRcrd_LoadRes();
	audioRcrd_RstDisplayTime();
	pAudioRecSM->refresh = AUDIOREC_REFRESH_ALL;
	
	//start the timer for refreshing time
    pAudioRecSM->refresh_time_id = Fwl_SetTimerMilliSecond(AUDIOREC_TIME_INTERVAL, AK_TRUE);

#ifdef SUPPORT_AUDIOREC_DENOICE
	if (8000 != gs.AudioRecordRate)
	{
		gs.bAudioRecDenoice = AK_FALSE;
	}
#endif

    //record init
	Ctl_RecAudio_Init(gs.AudioRecordMode, gs.AudioRecordRate, eINPUT_SOURCE_MIC, 0);
	
	m_regResumeFunc(resume_arec);
	pAudioRecSM->isExitWarning = 0;

	AK_DEBUG_OUTPUT("leave initaudio_recorder\n\n");
#endif
}

T_VOID exitaudio_recorder(T_VOID)
{
#ifdef SUPPORT_AUDIOREC

	AK_DEBUG_OUTPUT("\n enter exitaudio_recorder\n");
    /**stop refresh time timer*/
    if (ERROR_TIMER != pAudioRecSM->refresh_time_id)
    {
        Fwl_StopTimer(pAudioRecSM->refresh_time_id);
        pAudioRecSM->refresh_time_id = ERROR_TIMER;
    }

    //exit with record state, stop it
    if (eSTAT_REC_STOP != Ctl_RecAudio_GetCurState())
    {
		Ctl_RecAudio_Stop(AK_TRUE);
    }
	
    Ctl_RecAudio_Destroy();

    //free all the allocated resource
    pAudioRecSM = Fwl_Free(pAudioRecSM);

	AK_DEBUG_OUTPUT("leave exitaudio_recorder\n\n");
#endif
}

T_VOID paintaudio_recorder(T_VOID)
{
#ifdef SUPPORT_AUDIOREC

    if (AUDIOREC_REFRESH_ALL == pAudioRecSM->refresh)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pAudioRecSM->res.BckgrndRect.left,
            pAudioRecSM->res.BckgrndRect.top, pAudioRecSM->res.pBckGrnd,
            AK_NULL, AK_FALSE);
    }
    
    audioRcrd_ShowRcrdTime(&pAudioRecSM->res);
    audioRcrd_ShowRcrdStateIcon(&pAudioRecSM->res);
    audioRcrd_ShowRcrdProgress(&pAudioRecSM->res);
    audioRcrd_ShowRcrdEqualizer(&pAudioRecSM->res);

	if (0 != pAudioRecSM->isExitWarning)
	{
		MsgBox_SetRefresh(&pAudioRecSM->MsgBox, CTL_REFRESH_ALL);
		MsgBox_Show(&pAudioRecSM->MsgBox);
	}	

    if (AUDIOREC_REFRESH_NONE != pAudioRecSM->refresh)
    {
        TopBar_Show(TB_REFRESH_ALL);
        GE_StartShade();
        Fwl_RefreshDisplay();
        pAudioRecSM->refresh = AUDIOREC_REFRESH_NONE;
    }
#endif
}


unsigned char handleaudio_recorder(vT_EvtCode event, vT_EvtParam* pEventParm)
{
#ifdef SUPPORT_AUDIOREC

	T_MMI_KEYPAD	phyKey;

    if (IsPostProcessEvent(event))
    {
        pAudioRecSM->refresh = AUDIOREC_REFRESH_ALL;
        return 1;
    }

	if (VME_EVT_TIMER == event && pEventParm->w.Param1 == (T_U32)pAudioRecSM->refresh_time_id)
	{
		T_S32 curRecTime = 0;
		
		if (eSTAT_RECORDING == Ctl_RecAudio_GetCurState())
		{//fresh tiime display by timer count
			curRecTime = Ctl_RecAudio_GetCurRecTime();
			pAudioRecSM->display_time.hour = (T_U8)(curRecTime/3600);
			pAudioRecSM->display_time.minute = (T_U8)((curRecTime%3600)/60);
			pAudioRecSM->display_time.second = (T_U8)(curRecTime%60);
			pAudioRecSM->refresh |= AUDIOREC_REFRESH_TIME|AUDIOREC_REFRESH_PROGRESS;
		}
	}

	if (0 != pAudioRecSM->isExitWarning)
	{
		T_eBACK_STATE reState = eStay;
		
		reState = MsgBox_Handler(&pAudioRecSM->MsgBox, event, pEventParm);
		if (eNext == reState)
		{
			pAudioRecSM->isExitWarning = 0;
			pAudioRecSM->refresh = AUDIOREC_REFRESH_ALL;
			paintaudio_recorder();
			
			if (eSTAT_REC_STOP != Ctl_RecAudio_GetCurState())
			{
				if (!Ctl_RecAudio_Stop(AK_TRUE))
				{
					audioRcrd_ShowRecInfo(eDISPLAY_SAVE_FAILED, AK_NULL, 1);
				}
			}
			
			m_triggerEvent(M_EVT_EXIT, AK_NULL);
			return 0;
		}
		else if (eReturn == reState)
		{
			if (eSTAT_REC_STOP != Ctl_RecAudio_GetCurState())
			{
				Ctl_RecAudio_Stop(AK_FALSE);
			}
			
			m_triggerEvent(M_EVT_EXIT, AK_NULL);
		}
		else if (eStay == reState)
		{
			if (eSTAT_REC_STOP == Ctl_RecAudio_GetCurState())
			{
				pAudioRecSM->isExitWarning = 0;
				pAudioRecSM->refresh = AUDIOREC_REFRESH_ALL;
			}
		}
		
		return 0;
	}

    if (M_EVT_USER_KEY == event)
    {
		phyKey.keyID 	 = pEventParm->c.Param1;
     	phyKey.pressType = pEventParm->c.Param2;
		
		if (kbCLEAR == phyKey.keyID && eSTAT_REC_STOP != Ctl_RecAudio_GetCurState())
		{
			audioRcrd_ShowExitWarning();
			pAudioRecSM->isExitWarning = 1;
			return 0;
		}
		
		audioRcrd_UserKey_Handle(phyKey, pEventParm);

		return 0;
	}

    if (M_EVT_TOUCH_SCREEN == event)
	{
		T_POS x = (T_POS)pEventParm->s.Param2;
		T_POS y = (T_POS)pEventParm->s.Param3;
		
		phyKey.keyID = kbNULL;
		phyKey.pressType = PRESS_SHORT;
		
		switch (pEventParm->s.Param1) 
		{
		case eTOUCHSCR_UP:
			phyKey = audioRcrd_MapTSCR_To_Key(x, y);
			
			if (kbCLEAR == phyKey.keyID && eSTAT_REC_STOP != Ctl_RecAudio_GetCurState())
			{
				audioRcrd_ShowExitWarning();
				pAudioRecSM->isExitWarning = 1;
				return 0;
			}
			
			audioRcrd_UserKey_Handle(phyKey, pEventParm);
			break;
			
		case eTOUCHSCR_DOWN:
			 break;
			 
		case eTOUCHSCR_MOVE:
			 break;
			 
		default:
			 break;
		}

		return 0;
	}

#endif
    return 0;
}

/* end of the file */
