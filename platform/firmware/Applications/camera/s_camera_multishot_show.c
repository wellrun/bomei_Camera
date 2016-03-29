

// show photo

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
#include "Eng_font.h"
#include "Eng_dynamicfont.h"
#include "fwl_oscom.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "Fwl_Graphic.h"
#include "fwl_display.h"
#include "Fwl_tscrcom.h"


#define     CAM_SHOW_REFRESH_NONE   0
#define     CAM_SHOW_REFRESH_PHOTO  1
#define     CAM_SHOW_REFRESH_FOCUS_FRAME 2
#define     CAM_SHOW_REFRESH_ALL    0xffffffff

#define     CAM_MULTI_SHOW_TIME      3    // s

#ifdef CAMERA_SUPPORT
static T_RECT lcd_rect[] = {

    {CAM_IMG_INTERVAL, CAM_IMG_INTERVAL, CAM_MULTI_IMG_WIDHT, CAM_MULTI_IMG_HEIGHT},                                                // No.1
    {(CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_WIDHT, CAM_IMG_INTERVAL, CAM_MULTI_IMG_WIDHT, CAM_MULTI_IMG_HEIGHT},                         // No.2
    {(CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_WIDHT, (CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_HEIGHT, CAM_MULTI_IMG_WIDHT, CAM_MULTI_IMG_HEIGHT}, // No.3
    {CAM_IMG_INTERVAL, (CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_HEIGHT, CAM_MULTI_IMG_WIDHT, CAM_MULTI_IMG_HEIGHT},                        // No.4
    };
#endif


#ifdef CAMERA_SUPPORT
typedef struct{
    T_U8    *pY;
    T_U8    *pU;
    T_U8    *pV;
    T_U32    width;
    T_U32    height;
    T_U16    focusId;        // foucus photo index
    T_U32    photoTotal;
    T_U32    refresh;
    T_BOOL   mode;          // 1:all 0:focus one
    T_TIMER  show_timer_id;
} T_CAM_MULTI_SHOW_PARM;

static T_CAM_MULTI_SHOW_PARM *pCamShowParm;

static T_VOID CamMultiShow_DrawFocusFrame(T_U16 focus_id);
static T_VOID MultiShot_MapTSCR_To_Key(T_MMI_KEYPAD *key, T_POS x, T_POS y);
static T_VOID MultiShot_UserKey_Handle(T_MMI_KEYPAD phyKey, T_EVT_PARAM * pEventParm);

#endif


void initcamera_multishot_show(void)
{
#ifdef CAMERA_SUPPORT
    pCamShowParm = (T_CAM_MULTI_SHOW_PARM*)Fwl_Malloc(sizeof(T_CAM_MULTI_SHOW_PARM));
    AK_ASSERT_PTR_VOID(pCamShowParm, "pCamShowParm malloc fail");

    pCamShowParm->mode = AK_TRUE;
    pCamShowParm->focusId = 0;
    pCamShowParm->pY = AK_NULL;
    pCamShowParm->pU = AK_NULL;
    pCamShowParm->pV = AK_NULL;

    pCamShowParm->show_timer_id = ERROR_TIMER;
#endif
}

void exitcamera_multishot_show(void)
{
#ifdef CAMERA_SUPPORT
    if (ERROR_TIMER != pCamShowParm->show_timer_id)
    {
        Fwl_StopTimer(pCamShowParm->show_timer_id);
        pCamShowParm->show_timer_id = ERROR_TIMER;
    }

    if (AK_NULL != pCamShowParm)
    {
        pCamShowParm = Fwl_Free(pCamShowParm);
        pCamShowParm = AK_NULL;
    }
#endif
}

T_VOID paintcamera_multishot_show(void)
{
#ifdef CAMERA_SUPPORT
    T_U32 i = 0;
    T_U8    *y = AK_NULL;
    T_U8    *u = AK_NULL;
    T_U8    *v = AK_NULL;
	T_RECT  foucsRect = {0}, dstWin = {0};
	T_BOOL  ret = AK_FALSE;


    if (pCamShowParm->refresh != CAM_SHOW_REFRESH_NONE)
    {
        if (pCamShowParm->mode) // show all photo
        {
        	if ((pCamShowParm->refresh & CAM_SHOW_REFRESH_ALL) == CAM_SHOW_REFRESH_ALL)
            {
                //Fwl_FillSolid(HRGB_LAYER, COLOR_BLUE);
                Fwl_DrawRect(HRGB_LAYER, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), COLOR_WHITE);
                Fwl_DrawRect(HRGB_LAYER, 1, 1, (T_LEN)(Fwl_GetLcdWidth() - 2), (T_LEN)(Fwl_GetLcdHeight() - 2), COLOR_WHITE);
				Fwl_DrawRect(HRGB_LAYER, 2, 2, (T_LEN)(Fwl_GetLcdWidth() - 4), (T_LEN)(Fwl_GetLcdHeight() - 4), COLOR_BLUE);
        		Fwl_DrawRect(HRGB_LAYER, 3, 3, (T_LEN)(Fwl_GetLcdWidth() - 6), (T_LEN)(Fwl_GetLcdHeight() - 6), COLOR_BLUE);

                CamMultiShow_DrawFocusFrame(pCamShowParm->focusId);

                Fwl_RefreshDisplay();
				
                for (i = 0; i < pCamShowParm->photoTotal; i++)
                {
                    T_STR_INFO  tmpstr;
                    T_USTR_INFO u_tmpstr;

                    /**YUV format: 4:2:0*/
                    y = pCamShowParm->pY + (pCamShowParm->width * pCamShowParm->height * i);
                    u = pCamShowParm->pU + (((pCamShowParm->width * pCamShowParm->height) >> 2) * i);
                    v = pCamShowParm->pV + (((pCamShowParm->width * pCamShowParm->height) >> 2) * i);

					Fwl_CameraGetFocusWin(&foucsRect, gb.nZoomInMultiple, 10, 
										   pCamShowParm->width, pCamShowParm->height);

					dstWin.left   = lcd_rect[i].left;
					dstWin.top	  = lcd_rect[i].top;
					dstWin.width  = lcd_rect[i].width;
					dstWin.height = lcd_rect[i].height;
					
					ret = Fwl_YuvZoom(y, u, v, (T_LEN)pCamShowParm->width, &foucsRect, Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, RGB565, &dstWin);
					
					if (!ret)
					{
					   Fwl_Print(C3, M_CAMERA,  "paintcamera_multishot_show Failure.\n"); 
					}
					
                    //Fwl_SetDispBuf(lcd_rect[i].left, lcd_rect[i].top, lcd_rect[i].width, lcd_rect[i].height, pCaptureShow->mj);
                    
                    sprintf(tmpstr, "No.%ld", i + 1);
                    
			        Eng_StrMbcs2Ucs(tmpstr, u_tmpstr);
			        
			        Fwl_UDispSpeciString(HRGB_LAYER,\
			                (T_POS)(lcd_rect[i].left + lcd_rect[i].width - 40),\
			                (T_POS)(lcd_rect[i].top + 5),\
			                u_tmpstr, COLOR_ORANGE, CURRENT_FONT_SIZE,\
			                (T_U16)Utl_UStrLen(u_tmpstr));
                }

                Fwl_RefreshDisplay();
            }
			
            else if ((pCamShowParm->refresh & CAM_SHOW_REFRESH_FOCUS_FRAME) == CAM_SHOW_REFRESH_FOCUS_FRAME)
            {
                CamMultiShow_DrawFocusFrame(pCamShowParm->focusId);
                Fwl_RefreshDisplay();
            }
        }
		
		// show focus one
        else if ((pCamShowParm->refresh & CAM_SHOW_REFRESH_PHOTO) == CAM_SHOW_REFRESH_PHOTO)
        {
            T_STR_INFO  tmpstr;
            T_USTR_INFO u_tmpstr;

            //Fwl_FillSolid(HRGB_LAYER, COLOR_BLUE);
            Fwl_DrawRect(HRGB_LAYER, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), COLOR_WHITE);
            Fwl_DrawRect(HRGB_LAYER, 1, 1, (T_LEN)(Fwl_GetLcdWidth() - 2), (T_LEN)(Fwl_GetLcdHeight() - 2), COLOR_WHITE);
			Fwl_DrawRect(HRGB_LAYER, 2, 2, (T_LEN)(Fwl_GetLcdWidth() - 4), (T_LEN)(Fwl_GetLcdHeight() - 4), COLOR_BLUE);
			Fwl_DrawRect(HRGB_LAYER, 3, 3, (T_LEN)(Fwl_GetLcdWidth() - 6), (T_LEN)(Fwl_GetLcdHeight() - 6), COLOR_BLUE);

			Fwl_RefreshDisplay();
			
            /**YUV format: 4:2:0*/
            y = pCamShowParm->pY + (pCamShowParm->width * pCamShowParm->height * pCamShowParm->focusId);
            u = pCamShowParm->pU + (((pCamShowParm->width * pCamShowParm->height) >> 2) * pCamShowParm->focusId);
            v = pCamShowParm->pV + (((pCamShowParm->width * pCamShowParm->height) >> 2) * pCamShowParm->focusId);


			Fwl_CameraGetFocusWin(&foucsRect, gb.nZoomInMultiple, 10, 
								   pCamShowParm->width, pCamShowParm->height);
			
			dstWin.left   = CAM_IMG_INTERVAL;
			dstWin.top	  = CAM_IMG_INTERVAL;
			dstWin.width  = CAM_ONE_IMG_WIDHT;
			dstWin.height = CAM_ONE_IMG_HEIGHT;
			
			ret = Fwl_YuvZoom(y, u, v, (T_LEN)pCamShowParm->width, &foucsRect, Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, RGB565, &dstWin);
			
			if (!ret)
			{
			   Fwl_Print(C3, M_CAMERA,  "paintcamera_multishot_show Failure.\n"); 
			}

			//Fwl_RefreshRect(HRGB_LAYER, pCaptureShow->mj, CAM_ONE_IMG_WIDHT, CAM_ONE_IMG_HEIGHT, CAM_IMG_INTERVAL, CAM_IMG_INTERVAL, CAM_ONE_IMG_WIDHT, CAM_ONE_IMG_HEIGHT, 1);
            
            sprintf(tmpstr, "No.%d", pCamShowParm->focusId + 1);
            Eng_StrMbcs2Ucs(tmpstr, u_tmpstr);
            Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(CAM_IMG_INTERVAL + CAM_ONE_IMG_WIDHT - 40), (T_POS)(CAM_IMG_INTERVAL + 5), u_tmpstr, COLOR_ORANGE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(u_tmpstr));

            Fwl_RefreshDisplay();
        }
        
    }

    pCamShowParm->refresh = CAM_SHOW_REFRESH_NONE;
#endif
}

unsigned char handlecamera_multishot_show(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef CAMERA_SUPPORT
    T_MMI_KEYPAD phyKey;
    T_POS x;
    T_POS y;
	T_CAPTURE *pCaptureShow;
	
    if (IsPostProcessEvent(event))
    {
        pCamShowParm->refresh = CAM_SHOW_REFRESH_ALL;
        return 1;
    }

    if (M_EVT_MULTI_SHOW == event)
    {
        pCaptureShow = (T_CAPTURE*)pEventParm->p.pParam1;

        pCamShowParm->pY = pCaptureShow->pY;
        pCamShowParm->pU = pCaptureShow->pU;
        pCamShowParm->pV = pCaptureShow->pV;

        pCamShowParm->width = pCaptureShow->window_width;
        pCamShowParm->height = pCaptureShow->window_height;

        Fwl_Print(C3, M_CAMERA,  "muti_show:pCamShowParm->width=%d\n",pCamShowParm->width);
        Fwl_Print(C3, M_CAMERA,  "muti_show:pCamShowParm->height=%d\n",pCamShowParm->height);

        pCamShowParm->photoTotal = pCaptureShow->photoTotal;

        if (ERROR_TIMER == pCamShowParm->show_timer_id)
        {
            pCamShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_MULTI_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
			//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME time1 = %u\n",get_tick_count());
#endif
        }

        pCamShowParm->refresh = CAM_SHOW_REFRESH_ALL;
        return 0;
    }

    switch (event)
    {
    case M_EVT_TOUCH_SCREEN:
        x = (T_POS)pEventParm->s.Param2;
        y = (T_POS)pEventParm->s.Param3;
    
        phyKey.keyID = kbNULL;
        phyKey.pressType = PRESS_SHORT;
    
        switch (pEventParm->s.Param1) 
        {
        case eTOUCHSCR_UP:
    
            /*transforming the point(x,y)  to the corresponding key */
            MultiShot_MapTSCR_To_Key(&phyKey, x, y);
    
            MultiShot_UserKey_Handle(phyKey, pEventParm);
            break;
			
        case eTOUCHSCR_DOWN:
             break;
			 
        case eTOUCHSCR_MOVE:
             break;
			 
        default:
             break;
        }

        break;
		
    case M_EVT_USER_KEY:
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;
		MultiShot_UserKey_Handle(phyKey, pEventParm);
        break;
		
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pCamShowParm->show_timer_id
			&& (pEventParm->w.Param2 >= CAM_MULTI_SHOW_TIME*1000))
        {
            Fwl_StopTimer(pCamShowParm->show_timer_id);
            pCamShowParm->show_timer_id = ERROR_TIMER;
#ifdef OS_ANYKA
			//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME time2 = %u\n",get_tick_count());
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

#ifdef CAMERA_SUPPORT
static T_VOID CamMultiShow_DrawFocusFrame(T_U16 focus_id)
{
    T_U32 i = 0;
    T_RECT frame[] = {
                {CAM_IMG_INTERVAL - 2, CAM_IMG_INTERVAL - 2, CAM_MULTI_IMG_WIDHT + 4, CAM_MULTI_IMG_HEIGHT + 4},                                                // No.1
                {(CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_WIDHT - 2, CAM_IMG_INTERVAL - 2, CAM_MULTI_IMG_WIDHT + 4, CAM_MULTI_IMG_HEIGHT + 4},                         // No.2
                {(CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_WIDHT - 2, (CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_HEIGHT - 2, CAM_MULTI_IMG_WIDHT + 4, CAM_MULTI_IMG_HEIGHT + 4}, // No.3
                {CAM_IMG_INTERVAL - 2, (CAM_IMG_INTERVAL << 1) + CAM_MULTI_IMG_HEIGHT - 2, CAM_MULTI_IMG_WIDHT + 4, CAM_MULTI_IMG_HEIGHT + 4},                        // No.4
                };

    for (i=0; i < pCamShowParm->photoTotal; i++)
    {
        Fwl_DrawRect(HRGB_LAYER, frame[i].left, frame[i].top, frame[i].width, frame[i].height, COLOR_BLUE);
        Fwl_DrawRect(HRGB_LAYER, (T_POS)(frame[i].left + 1), (T_POS)(frame[i].top + 1), (T_POS)(frame[i].width - 2), (T_POS)(frame[i].height - 2), COLOR_BLUE);
    }

    Fwl_DrawRect(HRGB_LAYER, frame[focus_id].left, frame[focus_id].top, frame[focus_id].width, frame[focus_id].height, COLOR_ORANGE);
    Fwl_DrawRect(HRGB_LAYER, (T_POS)(frame[focus_id].left + 1), (T_POS)(frame[focus_id].top + 1), (T_POS)(frame[focus_id].width - 2), (T_POS)(frame[focus_id].height - 2), COLOR_ORANGE);
}
#endif

#ifdef CAMERA_SUPPORT
static T_VOID MultiShot_MapTSCR_To_Key(T_MMI_KEYPAD *key, T_POS x, T_POS y)
{
    T_U32 i;

    if (pCamShowParm->mode != AK_TRUE)
    {
        key->keyID = kbCLEAR;
        return;
    }
    else
    {
        key->keyID = kbOK;
    }

    for (i=0; i<pCamShowParm->photoTotal; ++i)
    {
        if (PointInRect(&lcd_rect[i], x, y))
        {
            pCamShowParm->focusId = (T_U16)(i);
            break;
        }
    }

    return;
}
#endif

#ifdef CAMERA_SUPPORT
static T_VOID MultiShot_UserKey_Handle(T_MMI_KEYPAD phyKey, T_EVT_PARAM * pEventParm)
{
    switch (phyKey.keyID)
    {
    case kbOK:
        if (pCamShowParm->mode == AK_TRUE)
        {
            pCamShowParm->mode = AK_FALSE;
            pCamShowParm->refresh |= CAM_SHOW_REFRESH_PHOTO;
        }

        /*show time reset*/
        if (ERROR_TIMER != pCamShowParm->show_timer_id)
        {
            Fwl_StopTimer(pCamShowParm->show_timer_id);
            pCamShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_MULTI_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
			//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME newtime1 = %u\n",get_tick_count());
#endif
        }
        break;
		
    case kbCLEAR:
        if (pCamShowParm->mode == AK_TRUE)
        {
            if (ERROR_TIMER != pCamShowParm->show_timer_id)
            {
                Fwl_StopTimer(pCamShowParm->show_timer_id);
                pCamShowParm->show_timer_id = ERROR_TIMER;
            }

            m_triggerEvent(M_EVT_RETURN2, pEventParm);
        }
        else
        {
            pCamShowParm->mode = AK_TRUE;
            pCamShowParm->refresh |= CAM_SHOW_REFRESH_ALL;

            /*show time reset*/
            if (ERROR_TIMER != pCamShowParm->show_timer_id)
            {
                Fwl_StopTimer(pCamShowParm->show_timer_id);
                pCamShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_MULTI_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
				//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME newtime1 = %u\n",get_tick_count());
#endif
            }
        }
        break;
		
    case kbLEFT:
	case kbRIGHT:
        if (pCamShowParm->mode == AK_TRUE)
        {
            if (pCamShowParm->focusId == 1)
            {
                pCamShowParm->focusId = 0;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
			else if (pCamShowParm->focusId == 0)
            {
                pCamShowParm->focusId = 1;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
            else if (pCamShowParm->focusId == 2)
            {
                pCamShowParm->focusId = 3;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
			else if (pCamShowParm->focusId == 3)
            {
                pCamShowParm->focusId = 2;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }

            /*show time reset*/
            if (ERROR_TIMER != pCamShowParm->show_timer_id)
            {
                Fwl_StopTimer(pCamShowParm->show_timer_id);
                pCamShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_MULTI_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
				//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME newtime1 = %u\n",get_tick_count());
#endif
            }
        }
        break;
		
    
	case kbDOWN:	
    case kbUP:
        if (pCamShowParm->mode == AK_TRUE)
        {
            if (pCamShowParm->focusId == 3)
            {
                pCamShowParm->focusId = 0;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
            else if (pCamShowParm->focusId == 0)
            {
                pCamShowParm->focusId = 3;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
			else if (pCamShowParm->focusId == 2)
            {
                pCamShowParm->focusId = 1;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }
			else if (pCamShowParm->focusId == 1)
            {
                pCamShowParm->focusId = 2;
                pCamShowParm->refresh |= CAM_SHOW_REFRESH_FOCUS_FRAME;
            }

            /*show time reset*/
            if (ERROR_TIMER != pCamShowParm->show_timer_id)
            {
                Fwl_StopTimer(pCamShowParm->show_timer_id);
                pCamShowParm->show_timer_id = Fwl_SetTimerSecond(CAM_MULTI_SHOW_TIME, AK_TRUE);
#ifdef OS_ANYKA
				//Fwl_Print(C3, M_CAMERA,"CAM_MULTI_SHOW_TIME newtime1 = %u\n",get_tick_count());
#endif
            }
        }
        break;
    default:
        break;
    }
}
#endif

