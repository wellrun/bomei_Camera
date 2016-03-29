

// show shot photo

#include "Fwl_public.h"
#include "eng_string.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "ctl_preview.h"
#include "ctl_capture.h"
#include "fwl_osmalloc.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "fwl_keyhandler.h"
#include "Fwl_Image.h"
#include "fwl_oscom.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"

#include "fwl_graphic.h"
#include "fwl_display.h"

#define     CAM_SHOT_REFRESH_NONE   0
#define     CAM_SHOT_REFRESH_PHOTO  1
#define     CAM_SHOT_REFRESH_FOCUS_FRAME 2
#define     CAM_SHOT_REFRESH_ALL    0xffffffff

#define     CAM_SHOT_SHOW_TIME      3    // s

#ifdef CAMERA_SUPPORT
typedef struct{
    T_U8    *pY;
    T_U8    *pU;
    T_U8    *pV;
    T_U32    width;
    T_U32    height;
    T_U32    refresh;
    T_TIMER  show_timer_id;
} T_CAM_SHOT_SHOW_PARM;

static T_CAM_SHOT_SHOW_PARM *pCamShotShowParm;
#endif


void initcamera_shot_show(void)
{
#ifdef CAMERA_SUPPORT
    pCamShotShowParm = (T_CAM_SHOT_SHOW_PARM*)Fwl_Malloc(sizeof(T_CAM_SHOT_SHOW_PARM));
    AK_ASSERT_PTR_VOID(pCamShotShowParm, "pCamShotShowParm malloc fail");

    pCamShotShowParm->pY = AK_NULL;
    pCamShotShowParm->pU = AK_NULL;
    pCamShotShowParm->pV = AK_NULL;

    pCamShotShowParm->show_timer_id = ERROR_TIMER;

#endif    
}

void exitcamera_shot_show(void)
{
#ifdef CAMERA_SUPPORT
    if (ERROR_TIMER != pCamShotShowParm->show_timer_id)
    {
        Fwl_StopTimer(pCamShotShowParm->show_timer_id);
        pCamShotShowParm->show_timer_id = ERROR_TIMER;
    }
    
    if (AK_NULL != pCamShotShowParm)
    {
        pCamShotShowParm = Fwl_Free(pCamShotShowParm);
        pCamShotShowParm = AK_NULL;
    }

#endif    
}

void paintcamera_shot_show(void)
{
#ifdef CAMERA_SUPPORT
	T_RECT  foucsRect = {0}, dstWin = {0};
//	T_BOOL  ret = AK_FALSE;
    T_CAMERA_MODE photoMode = 0;

    if (pCamShotShowParm->refresh != CAM_SHOT_REFRESH_NONE
		&& pCamShotShowParm->refresh & CAM_SHOT_REFRESH_ALL == CAM_SHOT_REFRESH_ALL)
    {
        Fwl_DrawRect(HRGB_LAYER, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), COLOR_WHITE);
        Fwl_DrawRect(HRGB_LAYER, 1, 1, (T_LEN)(Fwl_GetLcdWidth() - 2), (T_LEN)(Fwl_GetLcdHeight() - 2), COLOR_WHITE);
		Fwl_DrawRect(HRGB_LAYER, 2, 2, (T_LEN)(Fwl_GetLcdWidth() - 4), (T_LEN)(Fwl_GetLcdHeight() - 4), COLOR_BLUE);
        Fwl_DrawRect(HRGB_LAYER, 3, 3, (T_LEN)(Fwl_GetLcdWidth() - 6), (T_LEN)(Fwl_GetLcdHeight() - 6), COLOR_BLUE);
        
		Fwl_RefreshDisplay();

		photoMode = gs.camPhotoMode;
		switch(photoMode)
		{		
			case CAMERA_MODE_UXGA:
                //Fwl_YuvZoom() only support 1280x1280~18x18, so 1600x1200 zoom 10~20
				Fwl_CameraGetFocusWin(&foucsRect, (gb.nZoomInMultiple + 10), 20, 
							   pCamShotShowParm->width, pCamShotShowParm->height);
		
				break;		
			default:	
				Fwl_CameraGetFocusWin(&foucsRect, gb.nZoomInMultiple, 10, 
							   pCamShotShowParm->width, pCamShotShowParm->height);
		
				break;
		}		
		
		dstWin.left   = CAM_IMG_INTERVAL;
		dstWin.top	  = CAM_IMG_INTERVAL;
		dstWin.width  = CAM_ONE_IMG_WIDHT;
		dstWin.height = CAM_ONE_IMG_HEIGHT;
/*		
		ret = Fwl_YuvZoom(pCamShotShowParm->pY, pCamShotShowParm->pU, pCamShotShowParm->pV, 
			                 (T_LEN)pCamShotShowParm->width, &foucsRect, Fwl_GetDispMemory565(),
			                  MAIN_LCD_WIDTH, RGB565, &dstWin);
		if (!ret)
		{
		   Fwl_Print(C3, M_CAMERA,  "paintcamera_shot_show Failure.\n"); 
		}
*/
		Fwl_RefreshYUV1(pCamShotShowParm->pY, pCamShotShowParm->pU, pCamShotShowParm->pV, 
			                 pCamShotShowParm->width, pCamShotShowParm->height
			                 , dstWin.left,dstWin.top,dstWin.width,dstWin.height);

		Fwl_RefreshDisplay();		
		
    }
 
    pCamShotShowParm->refresh = CAM_SHOT_REFRESH_NONE;
#endif   
}

unsigned char handlecamera_shot_show(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef CAMERA_SUPPORT
    T_MMI_KEYPAD phyKey;
	T_CAPTURE *pCaptureShotShow;
    
    if (IsPostProcessEvent(event))
    {
        pCamShotShowParm->refresh = CAM_SHOT_REFRESH_ALL;
        return 1;
    }

    if (M_EVT_SHOT_SHOW == event)
    {
        pCaptureShotShow = (T_CAPTURE*)pEventParm->p.pParam1;
        
        pCamShotShowParm->pY = pCaptureShotShow->pY;
        pCamShotShowParm->pU = pCaptureShotShow->pU;
        pCamShotShowParm->pV = pCaptureShotShow->pV;

        pCamShotShowParm->width = pCaptureShotShow->window_width;
        pCamShotShowParm->height = pCaptureShotShow->window_height;

        if (ERROR_TIMER == pCamShotShowParm->show_timer_id)
        {        	
            pCamShotShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_SHOT_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
			//Fwl_Print(C3, M_CAMERA,"CAM_SHOT_SHOW_TIME time1 = %u\n",get_tick_count());
#endif
        }

        pCamShotShowParm->refresh = CAM_SHOT_REFRESH_ALL;
        return 0;
    }

    switch (event)
    {
    case M_EVT_USER_KEY:
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

        switch (phyKey.keyID)
        {
        case kbCLEAR:
            if (ERROR_TIMER != pCamShotShowParm->show_timer_id)
            {
                Fwl_StopTimer(pCamShotShowParm->show_timer_id);
                pCamShotShowParm->show_timer_id = ERROR_TIMER;
            }
            
            m_triggerEvent(M_EVT_RETURN2, pEventParm);
            break;
			
        default:
            break;
        }
        break;
		
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pCamShotShowParm->show_timer_id
			&& (pEventParm->w.Param2 >= CAM_SHOT_SHOW_TIME*1000))
        {       	
            Fwl_StopTimer(pCamShotShowParm->show_timer_id);
            pCamShotShowParm->show_timer_id = ERROR_TIMER;
#ifdef OS_ANYKA
			//Fwl_Print(C3, M_CAMERA,"CAM_SHOT_SHOW_TIME time2 = %u\n",get_tick_count());
#endif
            m_triggerEvent(M_EVT_RETURN2, pEventParm);
        }
        break;
		
    default:
        break;
    }
#endif    

    return 0;
}


