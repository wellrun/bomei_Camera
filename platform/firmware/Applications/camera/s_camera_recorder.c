
/*
MODENAME: camera recorder
AUTHOR: zhengwenbo
CREATE DATE:   2006-7-3
LOG:
*/
#include "Fwl_public.h"
#ifdef CAMERA_SUPPORT

#include "eng_string.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "fwl_osmalloc.h"
#include "fwl_osfs.h"
#include "ctl_videorecord.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "fwl_display.h"


T_VIDEO_RECORD *pVideoRec = AK_NULL;
static  T_BOOL  VideoRecInitFlag = AK_FALSE;

T_VOID CameraRecorder_SaveFile(T_VOID);
static T_VOID CamRec_ResumeFunc(T_VOID);
static T_VOID CamRec_SuspendFunc(T_VOID);
static unsigned char CamRec_HandleInitEvt(T_EVT_PARAM *pEventParm);
#endif
void initcamera_recorder(void)
{
#ifdef CAMERA_SUPPORT
	Menu_FreeRes();
	Fwl_AudioDisableDA();
    TopBar_DisableShow();
    m_regResumeFunc(TopBar_DisableShow);
    
    VideoRecInitFlag = AK_FALSE;

    /**malloc video record control*/
    pVideoRec = (T_VIDEO_RECORD*)Fwl_Malloc(sizeof(T_VIDEO_RECORD));
    AK_ASSERT_PTR_VOID(pVideoRec, "initcamera_recorder(): pVideoRec malloc fail\n");    
	memset(pVideoRec, 0, sizeof(T_VIDEO_RECORD));
    
    m_regResumeFunc(CamRec_ResumeFunc);
    m_regSuspendFunc(CamRec_SuspendFunc);
    
    if (!VideoRecord_Init(pVideoRec))
    {
        Fwl_Print(C3, M_CAMERA,  "Warning: video record init fail!\n");
        return;
    }

    VideoRecInitFlag = AK_TRUE;
    gb.PubMsgAllow = AK_TRUE;
#endif
}

void exitcamera_recorder(void)
{
#ifdef CAMERA_SUPPORT

	if (AK_NULL != pVideoRec->pJ_OSD_Info.osdRGB565)
	{
		pVideoRec->pJ_OSD_Info.osdRGB565 = Fwl_Free(pVideoRec->pJ_OSD_Info.osdRGB565);
	}
    
    if (AK_NULL != pVideoRec->pJ_OSD_Info_Zoom.osdRGB565)
    {
        pVideoRec->pJ_OSD_Info_Zoom.osdRGB565 = Fwl_Free(pVideoRec->pJ_OSD_Info_Zoom.osdRGB565);
    }

	if (VideoRecInitFlag)
		VideoRecord_Free(pVideoRec);
	
    if (AK_NULL != pVideoRec)
    {
        pVideoRec = Fwl_Free(pVideoRec);
    }
    gb.PubMsgAllow = AK_TRUE;
    TopBar_EnableShow();
    
	Menu_LoadRes();
	Fwl_Print(C3, M_CAMERA,  "Exited Camera Recorder!\n");

#endif
}

void paintcamera_recorder(void)
{

}

unsigned char handlecamera_recorder(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef CAMERA_SUPPORT
    T_eBACK_STATE reState = eStay;
    
    if (IsPostProcessEvent(event))
    {
        if (M_EVT_Z99COM_POWEROFF == event)
        {
            Fwl_Print(C3, M_CAMERA,  "save record file, power off\n");
            /**save record file when poweroff(low battery)*/
            CameraRecorder_SaveFile();
        }

        /**Fill lcd with black solid color when lock message box eixt*/
        if (CAMERA_MODE_CIF != gs.camRecMode[gs.CamRecFileType])
        {
            VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_REFRESH_BACK);
        }

        return 1;
    }

    if (M_EVT_2 == event)
    {
    	 return CamRec_HandleInitEvt(pEventParm);
    }

    /**record control handle*/    
    reState = VideoRecord_Handle(pVideoRec, event, pEventParm);
    switch (reState)
    {        
    default:
        ReturnDefauleProc(reState, pEventParm);
        break;
    }
#endif
    
    return 0;
}

#ifdef CAMERA_SUPPORT




T_VOID CameraRecorder_Stop(T_BOOL isSave,T_eCSTM_STR_ID strID)
{
	if (AK_NULL == pVideoRec)
	{
		Fwl_Print(C3, M_CAMERA,  "NeedNot %s",isSave?"Save":"Cancle");
		return;
	}
	VideoRecord_Close(pVideoRec,isSave,strID);
}


/** 
 * @brief The function is to save file when poweroff(low battery)
 *
 * @author \b zhengwenbo
 * @date 2006-07-19
 */
T_VOID CameraRecorder_SaveFile(T_VOID)
{
    if (AK_NULL != pVideoRec)
    {
        CameraRecorder_Stop(AK_TRUE, csSTAND_BY);//csNULL
    }
}




/*T_VOID CameraRecorder_SwtitchTvOut(DISPLAY_TYPE_DEV tvOutMode)
{
    if (AK_NULL != pVideoRec)
    {
        VideoRecord_SwitchTvOut(pVideoRec, tvOutMode);//csNULL
    }
}*/

T_VOID DD_CameraRecorder_SaveFile(T_VOID)
{
	if (AK_NULL != pVideoRec)
    {
    	if (!Fwl_CheckDriverIsValid((T_pCWSTR)pVideoRec->curPath))
    	{
        	CameraRecorder_Stop(AK_FALSE, csSTAND_BY);//csNULL
    	}
		else
		{
			CameraRecorder_Stop(AK_TRUE, csSTAND_BY);//csNULL
		}
    }
}


static T_VOID CamRec_ResumeFunc(T_VOID)
{
    Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    Fwl_RefreshDisplay();    
}

static T_VOID CamRec_SuspendFunc(T_VOID)
{
#ifdef CI37XX_PLATFORM
    Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    Fwl_RefreshDisplay();   
#endif
}

static unsigned char CamRec_HandleInitEvt(T_EVT_PARAM *pEventParm)
{
	T_PREVIEW *pPreviewRec = AK_NULL;
	T_REC_ERROR_STATUS recvRet = 0;
    T_BOOL useInnerRefresh = AK_FALSE;
	
    /**get preview point from preview state machine*/
    pPreviewRec = (T_PREVIEW*)pEventParm->p.pParam1;
	
    pVideoRec->width 	      = pPreviewRec->shotParm.c1;
    pVideoRec->height 	      = pPreviewRec->shotParm.c2;
    pVideoRec->videoSrcWidth  = pPreviewRec->shotParm.width;
    pVideoRec->videoSrcHeight = pPreviewRec->shotParm.height;
	pVideoRec->hZoom          = pPreviewRec->hZoom;

	if (VideoRecord_PaintInit(pVideoRec,&pPreviewRec->recordPaintData))
	{
	    if (VideoRecord_SrcInit(pVideoRec))
	    {
            recvRet = VideoRecord_StartRecord(pVideoRec);   
            if (REC_ERROR_OK != recvRet)
            {
                VideoRecord_SrcDeInit(pVideoRec);
            }
	    }
	    
		if (REC_ERROR_OK == recvRet)
		{
            useInnerRefresh = AK_TRUE;
		}
	}
	
	else
	{
		recvRet = REC_ERROR_NO_MEM;
        useInnerRefresh = AK_FALSE;
	}
	
	if (REC_ERROR_OK != recvRet)
	{
		if (REC_ERROR_NO_SPACE == recvRet)
		{
			Fwl_Print(C3, M_CAMERA, "No Enough Space!\n");
			/**show not enough memory message box*/
            VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_REFRESH_BACK);
			VideoRecord_DispErrMsgbox(pVideoRec, ctFAILURE, csSAVESPACEFULL, 2, useInnerRefresh);

        }
		else
		{
            VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_REFRESH_BACK);
			VideoRecord_DispErrMsgbox(pVideoRec, ctFAILURE, csFAILURE_DONE, 2, useInnerRefresh);
		}
		m_triggerEvent(M_EVT_EXIT, pEventParm);
		return 0;
	}

    return 0;
}
#endif

