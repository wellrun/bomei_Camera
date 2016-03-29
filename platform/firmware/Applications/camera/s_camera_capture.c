/*
MODENAME: camera capture
AUTHOR: zhengwenbo
CREATE DATE:   2006-7-3
LOG:
*/
#include "Fwl_public.h"


#ifdef CAMERA_SUPPORT

#include "eng_string.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "ctl_preview.h"
#include "ctl_capture.h"
#include "fwl_osmalloc.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "Fwl_Image.h"
#include "fwl_pfaudio.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

static T_VOID CameraCapture_SuspendFunc(T_VOID);

T_CAPTURE *pCapture = AK_NULL;
#endif

void initcamera_capture(void)
{
#ifdef CAMERA_SUPPORT
    TopBar_DisableShow();
    m_regResumeFunc(TopBar_DisableShow);
    m_regSuspendFunc(CameraCapture_SuspendFunc);
    
    /*pCapture malloc*/
    if (AK_NULL == (pCapture = (T_CAPTURE*)Fwl_Malloc(sizeof(T_CAPTURE))))
    {
        AK_ASSERT_PTR_VOID(pCapture, "initcamera_capture(): pCapture malloc fail\n");
        return;
    }

    if (!Capture_Init(pCapture))
    {
        Fwl_Print(C3, M_CAMERA,  "initcamera_capture(): capture control init fail\n");
        return;
    }

#endif
}

void exitcamera_capture(void)
{
#ifdef CAMERA_SUPPORT
    //Fwl_AudioDisableDAAndDsp();
	//Fwl_AudioDisableDA();
	
    Capture_Free(pCapture);	
    pCapture = Fwl_Free(pCapture);
#endif
}

void paintcamera_capture(void)
{
#ifdef CAMERA_SUPPORT
#endif
}

unsigned char handlecamera_capture(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef CAMERA_SUPPORT
    T_eBACK_STATE retState = eStay;
    T_PREVIEW *pPreview = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    /*receive camera parameters from preview state machine*/
    if (M_EVT_1 == event)
    {
        pPreview = (T_PREVIEW*)pEventParm->p.pParam1;

        Capture_SetParm(pCapture, pPreview);

        if (!(pCapture->width == pCapture->window_width &&
	        	pCapture->height == pCapture->window_height
	        	))
		{
            MsgBox_InitAfx(&pCapture->msgbox, 2, ctERROR, csNOT_SUPT_SIZE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
            return 0;
			
		}
        
        if ((pCapture->free_size.low < JPG_BUF_SIZE) && ((pCapture->free_size.high < 1)))
        {
            Fwl_Print(C3, M_CAMERA,  "free size = %d, memory is not enough.\n", pCapture->free_size.low);
			
            MsgBox_InitAfx(&pCapture->msgbox, 2, ctERROR, csSAVESPACEFULL, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
            return 0;
        }
        
        if (!Capture_MallocBuff(pCapture))
        {
            MsgBox_InitAfx(&pCapture->msgbox, 2, ctERROR, csMEM_NOT_ENOUGH, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pCapture->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCapture->msgbox);
            return 0;
        }  

    }
    
    retState = Capture_Handle(pCapture, event, pEventParm);

    if (eReturn == retState)
    {
        m_triggerEvent(M_EVT_EXIT, pEventParm);
    }
    else if (eNext == retState)
    {
        pEventParm->p.pParam1 = (T_pVOID*)pCapture;

        if (gs.DCShotMode == DC_NORMAL_SHOT)
        {
            m_triggerEvent(M_EVT_SHOT_SHOW, pEventParm);
        }
        else    // multi shot
        {
            m_triggerEvent(M_EVT_MULTI_SHOW, pEventParm);
        }
    }
#endif

    return 0;
}
#ifdef CAMERA_SUPPORT
static T_VOID CameraCapture_SuspendFunc(T_VOID)
{
    /**for background show black when display message box*/
    Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    Fwl_RefreshDisplay();
 
}
#endif 

/** End of File */
