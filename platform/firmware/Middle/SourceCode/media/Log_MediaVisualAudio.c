/**
 * @file Log_MediaVisualAudio.c
 * @brief Video Decoder Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Huang_Xueying
 * @date 2011-7-9
 * @version 1.0
 */

#include "Lib_state.h"

#ifdef SUPPORT_VISUAL_AUDIO
#include "VAapi.h"
#include "Fwl_oscom.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Fwl_osFS.h"
#include "Eng_TopBar.h"

#include "Eng_AkBmp.h"
#include "eng_dataconvert.h"
#include "Ctl_msgbox.h"
#include "lib_image_api.h"
#include "Eng_Graph.h"
#include "fwl_osfs.h"
#include "Lib_state_api.h"
#include "Akos_api.h"
#include "AKError.h"
#include "AkAppMgr.h"
#include "Log_MediaVisualAudio.h"

#include "fwl_display.h"
#include "Eng_ImgDec.h"
#include "Eng_ImgConvert.h"
#include "Arch_gui.h"
#include "Fwl_graphic.h"

#define VA_SHOW_TIME_INTERVAL            30
#define VA_SHOW_FREQ_INTERVAL            50

#define VA_SHOW_DRAW_WIDTH               MAIN_LCD_WIDTH
#define VA_SHOW_DRAW_HEIGHT              MAIN_LCD_HEIGHT
#define VA_SHOW_DRAW_TOP                 0
#define VA_SHOW_DRAW_LEFT                0
#define VA_SHOW_DRAW_BOTTOM              VA_SHOW_DRAW_HEIGHT

#define VA_SHOW_DRAW_BUF				 LCD_WIDTH*LCD_HEIGHT*2

#define LCD_WIDTH						 240
#define LCD_HEIGHT						 180
#define LCD_TOP							 0
#define LCD_LEFT						 0

typedef struct{
  T_U8  * pVabuf;
  T_U8  * dstBuf;
  T_RECT srcRect;
  T_RECT dstRect;
}VA_VISUAL_DISBUF;

T_BOOL 				 bVAShow_data_ready    = AK_FALSE;
VA_Handle 			 VA_VisualAudio_handle = AK_NULL;

static VA_E_DRAWTYPE VA_drawType           = VA_DRAW_FLAME;
static VA_CallBackFunc*  vaVisualAudio_cb  = AK_NULL;
static VA_VISUAL_DISBUF* vaVisualDisBuf	   = AK_NULL;

static T_U32         VA_Show_Interval      = VA_SHOW_FREQ_INTERVAL;
static T_TIMER	     VA_ShowTimer          = ERROR_TIMER;

extern T_VOID Sd_SetCodecCB(T_AUDIO_CB_FUNS *cbFun);

#ifdef OS_ANYKA
extern T_U8*  lcd_get_disp_buf(T_VOID);
#else
T_U8 gb_Display[320*240*3];
#endif

static T_pVOID VA_Malloc(T_U32 size)
{
	return (void *)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID VA_Free(T_pVOID var)
{
    return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

/** set up the show area*/
static T_BOOL VA_SetVisualDisbuf(VA_VISUAL_DISBUF* VisualDisBuf)
{
	AK_ASSERT_PTR(VisualDisBuf, "Input Parameter Is Invalid ", AK_FALSE);
	memset(VisualDisBuf, 0, sizeof(VA_VISUAL_DISBUF));
	
	VisualDisBuf->pVabuf = Fwl_Malloc(VA_SHOW_DRAW_BUF);

	if(AK_NULL == VisualDisBuf->pVabuf)
	{
		Fwl_Print(C3, M_AUDIO, "ERR: VisualDisBuf->pVabuf Malloc is Fail!");
		return AK_FALSE;
	}

	VisualDisBuf->srcRect.height = LCD_HEIGHT;
	VisualDisBuf->srcRect.left   = LCD_LEFT;
	VisualDisBuf->srcRect.top	 = LCD_TOP;	
	VisualDisBuf->srcRect.width  = LCD_WIDTH;

	VisualDisBuf->dstRect.height = VA_SHOW_DRAW_HEIGHT;
	VisualDisBuf->dstRect.left   = VA_SHOW_DRAW_LEFT;
	VisualDisBuf->dstRect.top	 = VA_SHOW_DRAW_TOP; 
	VisualDisBuf->dstRect.width  = VA_SHOW_DRAW_WIDTH;

	return AK_TRUE;
}

T_VOID VA_Fill_AudioBuf(T_S16* src,T_U32 len)
{
	T_S16* pVaRawdata = src;
	T_S32* pVadata;
	T_U32  i = 0;

	pVadata = (T_S32*)vaGetAudioBuf(VA_VisualAudio_handle);
	for (i=0; i<len; i++)
	{
		*(pVadata) = (T_S32)((pVaRawdata[0] + pVaRawdata[1])>>1);
		pVaRawdata += 2;
		pVadata++;
	}
			
	bVAShow_data_ready = AK_TRUE;
}

T_VOID VA_Increase_Draw_Type(T_VOID)
{
	//myHandle *Handle = (myHandle *)VA_VisualAudio_handle;
	
	VA_drawType++;

	if (VA_drawType > VA_DRAW_WING)
	{
		VA_drawType = VA_DRAW_BAR;
	}
	
	if (VA_DRAW_WING == VA_drawType)
	{
		VA_Show_Interval = VA_SHOW_TIME_INTERVAL;
	}
	else
	{
		VA_Show_Interval = VA_SHOW_FREQ_INTERVAL;
	}

	if (AK_NULL != VA_VisualAudio_handle)
	{
		vaSetDrawType(VA_VisualAudio_handle, VA_drawType);
	}
}

T_BOOL VA_GetAudioData(T_S32 * AudioBuf, VA_E_DATA_TYPE dataType, T_U16 dataLen)
{
	T_AUDIO_CB_FUNS audio_cbfun;
	T_BOOL 			ret;
	//T_U32  			DAbufLen=0;

	Sd_SetCodecCB(&audio_cbfun);

	if (AK_NULL == AudioBuf)
	{
		return AK_FALSE;
	}

	switch(dataType)
	{
	case VA_DATA_TIME:
		ret = AK_TRUE;
		break;

	case VA_DATA_FREQ:
		ret = _SD_GetAudioSpectrum(AudioBuf, dataLen, &audio_cbfun);
		break;

	default:
		return AK_FALSE;
	}

	return ret;
}

static T_VOID VA_SetVisualAudioCB(VA_CallBackFunc * visualaudioCB)
{
	AK_ASSERT_PTR_VOID(visualaudioCB, "Input Parameter Is Invalid ");
	memset(visualaudioCB, 0, sizeof(VA_CallBackFunc));
	
	visualaudioCB->malloc = VA_Malloc;
    visualaudioCB->free	  = VA_Free;
    visualaudioCB->printf = AkDebugOutput;
	visualaudioCB->GetAudioData = VA_GetAudioData;
}

static T_VOID VA_Show_Callback_Func(T_TIMER timer_id, T_U32 delay)
{
    AK_ASSERT_PTR_VOID(VA_VisualAudio_handle, "VA_Show_Callback_Func(): VA_VisualAudio_handle null");
	
	if (bVAShow_data_ready)
	{
		bVAShow_data_ready = AK_FALSE;

		vaDraw(VA_VisualAudio_handle);

	 	//if the show area no need to be full screen 
		Fwl_RGB565BitBlt(vaVisualDisBuf->pVabuf, LCD_WIDTH, &vaVisualDisBuf->srcRect, 
			lcd_get_disp_buf(), VA_SHOW_DRAW_WIDTH, FORMAT_RGB565, &vaVisualDisBuf->dstRect);

		//Fwl_RefreshDisplay();
		IAppMgr_PostUniqueEvt(AK_GetAppMgr(), AKAPP_CLSID_MMI, EVT_VISUAL_REFRESH);
	}

}


/**
* @brief Initialize the visual audio lib resource
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author HuangXueying
* @Param[in] VOID
* @Date 2011-7-9
* @Return VOID
*/
T_BOOL VisualAudio_Init(T_VOID)
{
	//T_RECT DrawRect;
	//T_U8* lcd_disp_addr;

	if (AK_NULL != vaVisualAudio_cb)
	{
		Fwl_Free(vaVisualAudio_cb);
		vaVisualAudio_cb = AK_NULL;
	}
	
	vaVisualAudio_cb = (VA_CallBackFunc* )Fwl_Malloc(sizeof(VA_CallBackFunc));

	if (AK_NULL == vaVisualAudio_cb)
	{
		Fwl_Print(C3, M_AUDIO, "ERR: vaVisualAudio_cb Malloc is Fail!");
		return AK_FALSE;
	}
	
	VA_SetVisualAudioCB(vaVisualAudio_cb);
	
	//show at area buf
	if (AK_NULL != vaVisualDisBuf)
	{
		Fwl_Free(vaVisualDisBuf);
		vaVisualDisBuf = AK_NULL;
	}
		
	vaVisualDisBuf = (VA_VISUAL_DISBUF *)Fwl_Malloc(sizeof(VA_VISUAL_DISBUF));
	
	if (AK_NULL == vaVisualDisBuf)
	{
		Fwl_Print(C3, M_AUDIO, "ERR: vaVisualDisBuf Malloc is Fail!");
		return AK_FALSE;
	}
	
	if(AK_FALSE == VA_SetVisualDisbuf(vaVisualDisBuf))
		return AK_FALSE;

	//lcd_disp_addr = (T_U8*)(lcd_get_disp_buf()+VA_SHOW_DRAW_TOP*VA_SHOW_DRAW_WIDTH*VA_RGB565_BPP);
	VA_VisualAudio_handle = vaVisualAudioInit(vaVisualAudio_cb,	VA_RGB565,
#ifdef OS_ANYKA
							
							 //prepare as an example: if the show area no need to be full screen
                             //MAIN_LCD_WIDTH, (T_U16)(MAIN_LCD_HEIGHT-VA_SHOW_DRAW_TOP-VA_SHOW_DRAW_BOTTOM), (T_U32)lcd_disp_addr);
							//MAIN_LCD_WIDTH, (T_U16)(MAIN_LCD_HEIGHT), (T_U32)lcd_get_disp_buf());
							LCD_WIDTH, (T_U16)(LCD_HEIGHT), (T_U32)(vaVisualDisBuf->pVabuf));
#else	
                            MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, &gb_Display);
#endif
	/*
	//prepare as an example: if the draw area need to be set
	RectInit(&DrawRect, VA_SHOW_DRAW_LEFT, VA_SHOW_DRAW_TOP, VA_SHOW_DRAW_WIDTH, VA_SHOW_DRAW_HEIGHT);
	if(!vaSetProPara(VA_VisualAudio_handle, VA_DRAW_RECT , &DrawRect))
	{	
		AkDebugOutput("Set Draw Area ERROR");
		return; 
	}
	*/

	if (AK_NULL == VA_VisualAudio_handle)
	{
		Fwl_Print(C3, M_AUDIO, "ERR: VisualAudio Init is Fail!");
		Fwl_Free(vaVisualAudio_cb);
		vaVisualAudio_cb = AK_NULL;
		return AK_FALSE;
	}

	vaSetDrawType(VA_VisualAudio_handle, VA_drawType);

	if (ERROR_TIMER != VA_ShowTimer)
	{
		Fwl_StopTimer(VA_ShowTimer);
		VA_ShowTimer = ERROR_TIMER;
	}
	
	VA_ShowTimer = Fwl_SetMSTimerWithCallback(VA_Show_Interval, AK_TRUE, VA_Show_Callback_Func);

	return AK_TRUE;
}

T_BOOL VisualAudio_IsInit(T_VOID)
{
	return (AK_NULL != VA_VisualAudio_handle);
}

/**
* @brief release the visual audio lib resource
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author HuangXueying
* @Param[in] VOID
* @Date 2011-7-9
* @Return VOID
*/
T_VOID VisualAudio_Realease(T_VOID)
{
	if (ERROR_TIMER != VA_ShowTimer)
	{
		Fwl_StopTimer(VA_ShowTimer);
		VA_ShowTimer = ERROR_TIMER;
	}
	
	vaVisualAudioRelease(VA_VisualAudio_handle);
	VA_VisualAudio_handle = AK_NULL;
	
	if (AK_NULL != vaVisualAudio_cb)
	{
		Fwl_Free(vaVisualAudio_cb);
		vaVisualAudio_cb = AK_NULL;
	}
	
	if (AK_NULL != vaVisualDisBuf)
	{
		Fwl_Free(vaVisualDisBuf->pVabuf);
		Fwl_Free(vaVisualDisBuf);
		vaVisualDisBuf = AK_NULL;
	}
}

#endif

