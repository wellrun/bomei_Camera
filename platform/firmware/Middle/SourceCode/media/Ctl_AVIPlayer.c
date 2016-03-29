#include "anyka_types.h"
#include "Ctl_AVIPlayer.h"



#ifdef SUPPORT_VIDEOPLAYER
#include "fwl_vme.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Initialize.h"
#include "Lib_event.h"
#include "eng_string.h"
#include "Eng_ImgConvert.h"
#include "Eng_Graph.h"
#include "Eng_AkBmp.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_ScreenSave.h"
#include "Eng_Time.h"
#include "Lib_res_port.h"
#include "hal_timer.h"
#include "Fwl_pfAudio.h"
#include "Fwl_waveout.h"
#include "Fwl_pfDisplay.h"
#include "Fwl_osfs.h"
#include "fwl_oscom.h"
#include "string.h"
#include "Eng_AutoPowerOff.h"
#include "Ctl_FileList.h"
#include "eng_dataconvert.h"
#include "fwl_keyhandler.h"
#include "Fwl_rtc.h"
#include "lib_image_api.h"
#include "AKAppMgr.h"
#include "lib_sdfilter.h"
#include "arch_lcd.h"
#include "hal_keypad.h"
#include "Lib_state_api.h"
#include "fwl_graphic.h"
#include "hal_sysdelay.h"
#include "Fwl_display.h"
#include "Fwl_tscrcom.h"

#include "Log_MediaPlayer.h"
#include "arch_gui.h"
#include "fwl_public.h"
#include "svc_medialist.h"
#include "Fwl_DispOsd.h"




#define VIDEO_FLOATBAR_TRANS                5      //0(完全透明)~8(不透明)

#define VIDEO_FILENAME_MAX_WIDTH            (VOLUME_SPEAK_LEFT - 4 - CUR_FILE_NAME_LEFT)     //(BACK_GROUND_WIDTH - 240)

#define AVI_WINDOW_TAILSPACE                1
#define VIDEO_HOLD_MEM_SIZE                 (800 << 10)
#define NORMALSPEED_INTERVAL_MILLISECONDS   20
#define FASTSPEED_INTERVAL_MILLISECONDS     250
#define WINDOW_PRG_INTERVAL                 200                // 200ms
#define TOTAL_TIME_ICONS                    17

#define AUTO_FULLSCREEN_ENABLE              0x00
#define AUTO_FULLSCREEN_DISABLE             0xff
#define AUTO_FULLSCREEN_TIME                0x03


#define LCD_WIDTH							MAIN_LCD_WIDTH
#define LCD_HEIGHT							MAIN_LCD_HEIGHT

T_AVI_PLAYER *pAVIPlayer = AK_NULL;
static T_USTR_FILE  filename;
static T_eMPLAYER_STATUS savStatus = MPLAYER_ERR;
static T_U32 AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;

static T_RECT PaintRect;

static const T_U8 *srcImgY;
static const T_U8 *srcImgU;
static const T_U8 *srcImgV;
static T_U16 uDispWidth ;
static T_U16 uDispHeight;
static T_U16 uOriWidth;
static T_U16 uOriHeight;


static vT_EvtParam EvtParam;
static T_hSemaphore hEnableShow = -1;

static T_VOID AVIPlayer_EndOneFile(T_END_TYPE endtype);
static T_eBACK_STATE AVIPlayer_UserKey_Handle(T_MMI_KEYPAD phyKey);
static T_MMI_KEYPAD AVIPlayer_MapTSCR_To_Key(T_POS x, T_POS y, T_EVT_PARAM *pEventParm);
static T_VOID AVIPlayer_SwitchWindowMode(T_VOID);
#endif


extern T_eMEDIALIB_VIDEO_CODE g_tCurrVideoType;


#ifdef SUPPORT_VIDEOPLAYER

/**
 * @brief initialize of player resource
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @retval AK_TRUE success
 */

static T_BOOL AVIPlayer_Malloc_IconBuf(T_VOID)
{
    pAVIPlayer->window.ProgressBar.data = AK_NULL;
    pAVIPlayer->window.ProgressBarBack.data = AK_NULL;
    pAVIPlayer->window.ProgressBlock.data = AK_NULL;
	pAVIPlayer->window.ProgressBlockPress.data = AK_NULL;

    pAVIPlayer->window.windowBack.data = AK_NULL;
    pAVIPlayer->window.prev.data = AK_NULL;
    pAVIPlayer->window.prevPress.data = AK_NULL;
    pAVIPlayer->window.next.data = AK_NULL;
    pAVIPlayer->window.nextPress.data = AK_NULL;
    pAVIPlayer->window.start.data = AK_NULL;
	pAVIPlayer->window.startPress.data = AK_NULL;
    pAVIPlayer->window.pause.data = AK_NULL;
	pAVIPlayer->window.pausePress.data = AK_NULL;
    pAVIPlayer->window.Back.data = AK_NULL;
    pAVIPlayer->window.volumeProgressBar.data = AK_NULL;
    pAVIPlayer->window.volumeProgressBarBack.data = AK_NULL;
    pAVIPlayer->window.volumeProgressBlock.data = AK_NULL;
    pAVIPlayer->window.volumeProgressBlockPress.data = AK_NULL;
	pAVIPlayer->window.complete.data = AK_NULL;
    pAVIPlayer->window.completePress.data = AK_NULL;

    return AK_TRUE;
}


/**
 * @brief refresh window of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] refresh flag
 * @return returns 
 * @retval T_VOID
 */
static T_VOID AVIPlayer_Window_SetRefresh(T_U8 refreshFlag)
{
    pAVIPlayer->window.Refresh |= refreshFlag;
}

static T_VOID AVIPlayer_SetPrgBlkHitRect(T_VOID)
{
    //Set hit progressblock bar rect of Touch screen
	pAVIPlayer->window.PrgBlkHitRect.left   = pAVIPlayer->window.ProgressBar.pos.left;
    pAVIPlayer->window.PrgBlkHitRect.top    = pAVIPlayer->window.ProgressBar.pos.top-10;
    pAVIPlayer->window.PrgBlkHitRect.width  = pAVIPlayer->window.ProgressBar.pos.width;
    pAVIPlayer->window.PrgBlkHitRect.height = pAVIPlayer->window.ProgressBar.pos.height+20;
}

static T_VOID AVIPlayer_SetHitVolRect(T_VOID)
{
    //Set hit volume bar rect of Touch screen
	pAVIPlayer->window.Volume.left   = pAVIPlayer->window.volumeProgressBar.pos.left;
    pAVIPlayer->window.Volume.top    = pAVIPlayer->window.volumeProgressBar.pos.top-10;
    pAVIPlayer->window.Volume.width  = pAVIPlayer->window.volumeProgressBar.pos.width;
    pAVIPlayer->window.Volume.height = pAVIPlayer->window.volumeProgressBar.pos.height+20;
}

/**
 * @brief initialize window of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_Window_Init(T_VOID)
{
    AVIPlayer_GetRes();
	AVIPlayer_SetPrgBlkHitRect();
	AVIPlayer_SetHitVolRect();
    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_ALL);
}

/**
 * @brief update volume block location
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_UpdateVolumeBlockLoc(T_VOID)
{
  T_U16 vol;
  T_LEN width;


	if (!pAVIPlayer->bAudioEnable)
	{
		vol = pAVIPlayer->volume_old;
	}
	else
	{
		vol = Fwl_GetAudioVolume();
	}
	
	width = (pAVIPlayer->window.volumeProgressBar.pos.width\
			- pAVIPlayer->window.volumeProgressBlock.pos.width/2);

    vol = vol*width/(AK_VOLUME_MAX);
    vol = (vol > width) ? width : vol;

	if (pAVIPlayer->fVolPress)
	{
		pAVIPlayer->window.volumeProgressBlockPress.pos.left=
						vol+pAVIPlayer->window.volumeProgressBar.pos.left;
	}
	else
	{
		pAVIPlayer->window.volumeProgressBlock.pos.left=
						vol+pAVIPlayer->window.volumeProgressBar.pos.left;
	}
}

/**
 * @brief load player ui resource
 *
 * @author \b zhengwenbo
 * @date 2008-05-27
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID AVIPlayer_GetRes(T_VOID)
{
    T_LEN bmp_width, bmp_height;
    T_U8 deep;

    ////////////////////////////////////////////////////////////////////////////////////////////
	// progress groudback
    pAVIPlayer->window.Back.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_BACK, &pAVIPlayer->window.Back.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.Back.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.Back.pos.left = (Fwl_GetLcdWidth()-bmp_width)>>1;	// Central
    pAVIPlayer->window.Back.pos.top  = 0;
    pAVIPlayer->window.Back.pos.width = bmp_width;
    pAVIPlayer->window.Back.pos.height = bmp_height;

	//  完成图标
    pAVIPlayer->window.complete.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_COMPLETE, &pAVIPlayer->window.complete.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.complete.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.complete.pos.left = pAVIPlayer->window.Back.pos.left+10;
    pAVIPlayer->window.complete.pos.top  = pAVIPlayer->window.Back.pos.top + ((pAVIPlayer->window.Back.pos.height-bmp_height) >> 1);
    pAVIPlayer->window.complete.pos.width = bmp_width;
    pAVIPlayer->window.complete.pos.height = bmp_height;

	// 完成按下图标
    pAVIPlayer->window.completePress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_COMPLETE_PRESS, &pAVIPlayer->window.completePress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.completePress.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.completePress.pos.left = pAVIPlayer->window.Back.pos.left + 10;
    pAVIPlayer->window.completePress.pos.top  = pAVIPlayer->window.Back.pos.top + ((pAVIPlayer->window.Back.pos.height-bmp_height) >> 1);
    pAVIPlayer->window.completePress.pos.width = bmp_width;
    pAVIPlayer->window.completePress.pos.height = bmp_height;

	// progress
    pAVIPlayer->window.ProgressBar.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS, &pAVIPlayer->window.ProgressBar.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.ProgressBar.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.ProgressBar.pos.left = pAVIPlayer->window.complete.pos.left
    										 + pAVIPlayer->window.complete.pos.width
    										 + pAVIPlayer->window.Back.pos.width/2
    										 - (pAVIPlayer->window.complete.pos.width+10)/2
    										 - bmp_width/2 ;
    pAVIPlayer->window.ProgressBar.pos.top  = pAVIPlayer->window.Back.pos.top + pAVIPlayer->window.Back.pos.height*3/8;
    pAVIPlayer->window.ProgressBar.pos.width = bmp_width;
    pAVIPlayer->window.ProgressBar.pos.height = bmp_height;

    
    // progress back
	pAVIPlayer->window.ProgressBarBack.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS_GRAY, &pAVIPlayer->window.ProgressBarBack.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.ProgressBarBack.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.ProgressBarBack.pos.left = pAVIPlayer->window.complete.pos.left
    										 + pAVIPlayer->window.complete.pos.width
    										 + pAVIPlayer->window.Back.pos.width/2
    										 - (pAVIPlayer->window.complete.pos.width+5)/2
    										 - bmp_width/2 ;
    pAVIPlayer->window.ProgressBarBack.pos.top  = pAVIPlayer->window.ProgressBar.pos.top;
    pAVIPlayer->window.ProgressBarBack.pos.width = bmp_width;
    pAVIPlayer->window.ProgressBarBack.pos.height = bmp_height;


	//progress block
	pAVIPlayer->window.ProgressBlock.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_BLOCK, &pAVIPlayer->window.ProgressBlock.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.ProgressBlock.data, &bmp_width, &bmp_height, &deep);
	pAVIPlayer->window.ProgressBlock.pos.left= pAVIPlayer->window.ProgressBar.pos.left;	// pAVIPlayer->window.Back.pos.left;	// +pAVIPlayer->window.Back.pos.width/2;
    pAVIPlayer->window.ProgressBlock.pos.top = pAVIPlayer->window.ProgressBar.pos.top-5;
	pAVIPlayer->window.ProgressBlock.pos.width = bmp_width;
    pAVIPlayer->window.ProgressBlock.pos.height = bmp_height;

	pAVIPlayer->window.ProgressBlockPress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_BLOCK_PRESS, &pAVIPlayer->window.ProgressBlockPress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.ProgressBlockPress.data, &bmp_width, &bmp_height, &deep);
	pAVIPlayer->window.ProgressBlockPress.pos.left= pAVIPlayer->window.ProgressBar.pos.left;	// pAVIPlayer->window.Back.pos.left;	// +pAVIPlayer->window.Back.pos.width/2;
    pAVIPlayer->window.ProgressBlockPress.pos.top = pAVIPlayer->window.ProgressBar.pos.top-5;
	pAVIPlayer->window.ProgressBlockPress.pos.width = bmp_width;
    pAVIPlayer->window.ProgressBlockPress.pos.height = bmp_height;

    ////////////////////////////////////////////////////////////////////////////////// 
    pAVIPlayer->window.windowBack.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VIDEO, &pAVIPlayer->window.windowBack.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.windowBack.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.windowBack.pos.left   = (Fwl_GetLcdWidth()-bmp_width)>>1;
    pAVIPlayer->window.windowBack.pos.top    = (Fwl_GetLcdHeight()-bmp_height-15);
    pAVIPlayer->window.windowBack.pos.width  = bmp_width;
    pAVIPlayer->window.windowBack.pos.height = bmp_height;

    pAVIPlayer->window.prev.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PREV, &pAVIPlayer->window.prev.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.prev.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.prev.pos.left   = pAVIPlayer->window.windowBack.pos.left + 20;
    pAVIPlayer->window.prev.pos.top    = pAVIPlayer->window.windowBack.pos.top + 12;
    pAVIPlayer->window.prev.pos.width  = bmp_width;
    pAVIPlayer->window.prev.pos.height = bmp_height;

    pAVIPlayer->window.prevPress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PREV_PRESS, &pAVIPlayer->window.prevPress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.prevPress.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.prevPress.pos.left   = pAVIPlayer->window.windowBack.pos.left + 20;
    pAVIPlayer->window.prevPress.pos.top    = pAVIPlayer->window.windowBack.pos.top + 12;
    pAVIPlayer->window.prevPress.pos.width  = bmp_width;
    pAVIPlayer->window.prevPress.pos.height = bmp_height;

    pAVIPlayer->window.next.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_NEXT, &pAVIPlayer->window.next.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.next.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.next.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width
    										 - bmp_width - 20;
    pAVIPlayer->window.next.pos.top  = pAVIPlayer->window.windowBack.pos.top + 12;
    pAVIPlayer->window.next.pos.width = bmp_width;
    pAVIPlayer->window.next.pos.height = bmp_height;

    pAVIPlayer->window.nextPress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_NEXT_PRESS, &pAVIPlayer->window.nextPress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.nextPress.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.nextPress.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width
    										 - bmp_width - 20;
    pAVIPlayer->window.nextPress.pos.top  = pAVIPlayer->window.windowBack.pos.top + 12;
    pAVIPlayer->window.nextPress.pos.width = bmp_width;
    pAVIPlayer->window.nextPress.pos.height = bmp_height;

    pAVIPlayer->window.start.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_START, &pAVIPlayer->window.start.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.start.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.start.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.start.pos.top  = pAVIPlayer->window.windowBack.pos.top + 9;
    pAVIPlayer->window.start.pos.width = bmp_width;
    pAVIPlayer->window.start.pos.height = bmp_height;

	pAVIPlayer->window.startPress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_START_PRESS, &pAVIPlayer->window.startPress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.startPress.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.startPress.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.startPress.pos.top  = pAVIPlayer->window.windowBack.pos.top + 9;
    pAVIPlayer->window.startPress.pos.width = bmp_width;
    pAVIPlayer->window.startPress.pos.height = bmp_height;

    pAVIPlayer->window.pause.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PAUSE, &pAVIPlayer->window.pause.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.pause.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.pause.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.pause.pos.top  = pAVIPlayer->window.windowBack.pos.top + 9;
    pAVIPlayer->window.pause.pos.width = bmp_width;
    pAVIPlayer->window.pause.pos.height = bmp_height;

    pAVIPlayer->window.pausePress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PAUSE_PRESS, &pAVIPlayer->window.pausePress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.pausePress.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.pausePress.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.pausePress.pos.top  = pAVIPlayer->window.windowBack.pos.top + 9;
    pAVIPlayer->window.pausePress.pos.width = bmp_width;
    pAVIPlayer->window.pausePress.pos.height = bmp_height;
	///////////////////////////////////////////////////////////////////////////////////////////
	// volume progress
    pAVIPlayer->window.volumeProgressBar.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS, &pAVIPlayer->window.volumeProgressBar.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.volumeProgressBar.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.volumeProgressBar.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.volumeProgressBar.pos.top  = pAVIPlayer->window.windowBack.pos.top + pAVIPlayer->window.windowBack.pos.height*3/4;
    pAVIPlayer->window.volumeProgressBar.pos.width = bmp_width;
    pAVIPlayer->window.volumeProgressBar.pos.height = bmp_height;

	// volume progress back
	pAVIPlayer->window.volumeProgressBarBack.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS_GRAY, &pAVIPlayer->window.volumeProgressBarBack.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.volumeProgressBarBack.data, &bmp_width, &bmp_height, &deep);
    pAVIPlayer->window.volumeProgressBarBack.pos.left = pAVIPlayer->window.windowBack.pos.left
    										 + pAVIPlayer->window.windowBack.pos.width/2
    										 - bmp_width/2;
    pAVIPlayer->window.volumeProgressBarBack.pos.top  = pAVIPlayer->window.windowBack.pos.top + pAVIPlayer->window.windowBack.pos.height*3/4;
    pAVIPlayer->window.volumeProgressBarBack.pos.width = bmp_width;
    pAVIPlayer->window.volumeProgressBarBack.pos.height = bmp_height;

	//volume progress block
	pAVIPlayer->window.volumeProgressBlock.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_BLOCK, &pAVIPlayer->window.volumeProgressBlock.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.volumeProgressBlock.data, &bmp_width, &bmp_height, &deep);
	pAVIPlayer->window.volumeProgressBlock.pos.left= pAVIPlayer->window.windowBack.pos.left+pAVIPlayer->window.windowBack.pos.width/2;
    pAVIPlayer->window.volumeProgressBlock.pos.top = pAVIPlayer->window.windowBack.pos.top +pAVIPlayer->window.windowBack.pos.height*3/4-5;
	pAVIPlayer->window.volumeProgressBlock.pos.width = bmp_width;
    pAVIPlayer->window.volumeProgressBlock.pos.height = bmp_height;

	pAVIPlayer->window.volumeProgressBlockPress.data = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_BLOCK_PRESS, &pAVIPlayer->window.volumeProgressBlockPress.dataLen);
	AKBmpGetInfo(pAVIPlayer->window.volumeProgressBlockPress.data, &bmp_width, &bmp_height, &deep);
	pAVIPlayer->window.volumeProgressBlockPress.pos.left= pAVIPlayer->window.windowBack.pos.left+pAVIPlayer->window.windowBack.pos.width/2;
    pAVIPlayer->window.volumeProgressBlockPress.pos.top = pAVIPlayer->window.windowBack.pos.top +pAVIPlayer->window.windowBack.pos.height*3/4-5;
	pAVIPlayer->window.volumeProgressBlockPress.pos.width = bmp_width;
    pAVIPlayer->window.volumeProgressBlockPress.pos.height = bmp_height;
    AVIPlayer_UpdateVolumeBlockLoc();
    
}

T_VOID Second2HHMMSS(T_STR_FILE timestr, T_U32 second)
{
	T_U16 sec = 0, min = 0, hour = 0;

	sec		= (T_U16)second % 60;
	min		= (T_U16)(second / 60) % 60;
	hour	= (T_U16)second / 3600;

	hour	= hour >= 100 ? 0 : hour;

	sprintf(timestr, "%.2d:%.2d:%.2d", hour, min, sec);
}

/**
 * @brief Refresh play time
 *       convert the number to image
 * @author jiangguiliang
 * @date  2007-03-16
 * @param cruTime       current playing time
 * @param totalTime     file total time
 * @return T_VOID
 * @retval
 */
T_VOID AVIPlayer_Window_Refresh_Time(T_U32 curTime, T_U32 totalTime)
{
    T_RECT rect;
    T_STR_FILE timestr;
    T_STR_FILE timestr1;

    rect.left = CURTIME_LEFT;
    rect.top = CURTIME_TOP;
    rect.width = CURTIME_WIDTH * TOTAL_TIME_ICONS;
    rect.height = CURTIME_HEIGHT;

    Second2HHMMSS(timestr, curTime);
	Second2HHMMSS(timestr1, totalTime);

/*
	Fwl_DispString(HRGB_LAYER, 
					pAVIPlayer->window.ProgressBarBack.pos.left, 
					(T_POS)(pAVIPlayer->window.ProgressBarBack.pos.top+12), 
					timestr, (T_U16)strlen(timestr), COLOR_WHITE, CURRENT_FONT_SIZE);
	Fwl_DispString(HRGB_LAYER, 
					(T_POS)(pAVIPlayer->window.ProgressBarBack.pos.left+pAVIPlayer->window.ProgressBarBack.pos.width-7*8), 
					(T_POS)(pAVIPlayer->window.ProgressBarBack.pos.top+12), 
					timestr1, (T_U16)strlen(timestr1), COLOR_WHITE, CURRENT_FONT_SIZE);
*/
	Fwl_Osd_DrawStringByGray(pAVIPlayer->window.ProgressBarBack.pos.left, 
					(T_POS)(pAVIPlayer->window.ProgressBarBack.pos.top+12), 
					timestr, (T_U16)strlen(timestr), COLOR_WHITE, CURRENT_FONT_SIZE);
	
	Fwl_Osd_DrawStringByGray((T_POS)(pAVIPlayer->window.ProgressBarBack.pos.left+pAVIPlayer->window.ProgressBarBack.pos.width-7*8), 
					(T_POS)(pAVIPlayer->window.ProgressBarBack.pos.top+12), 
					timestr1, (T_U16)strlen(timestr1), COLOR_WHITE, CURRENT_FONT_SIZE);
}

/**
 * @brief   video player UI
 * @author  hjhua
 * @date    2011-05-04 
 * @param   alpha1  alpha of progress background
 * @param   alpha2  alpha of control and voume background
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */

static T_BOOL AVIPlayer_Window_Paint(T_U8 alpha1, T_U8 alpha2)
{
	//T_RECT srcRect;
	T_RECT rect;

	if (FULL_PLAY_MODE == pAVIPlayer->play_mode
		|| AK_NULL == pAVIPlayer)
	{
		return AK_FALSE;
	}

	//////////////////////////////////////////////////////////////////
	// alpha show playing progress back			   
/*	srcRect.left   = 0;
	srcRect.top    = 0;
	srcRect.width  = pAVIPlayer->window.Back.pos.width;
	srcRect.height = pAVIPlayer->window.Back.pos.height;
	
	Fwl_AKBmpAlphaShow(pAVIPlayer->window.Back.data, 
					   pAVIPlayer->window.Back.pos.width,
					   srcRect,
					   Fwl_GetDispMemory565(),
					   Fwl_GetLcdWidth(),
					   pAVIPlayer->window.Back.pos,
	 				   alpha1);
*/
	
	Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.Back.pos, pAVIPlayer->window.Back.data);

	
/*	srcRect.left   = 0;
	srcRect.top    = 0;
	srcRect.width  = pAVIPlayer->window.windowBack.pos.width;
	srcRect.height = pAVIPlayer->window.windowBack.pos.height;
	
	Fwl_AKBmpAlphaShow(pAVIPlayer->window.windowBack.data, 
						pAVIPlayer->window.windowBack.pos.width,
						srcRect,
						Fwl_GetDispMemory565(),
						Fwl_GetLcdWidth(),
						pAVIPlayer->window.windowBack.pos,
						alpha2); 	
*/

	Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.windowBack.pos, pAVIPlayer->window.windowBack.data);

	///////////////////////////////////////////////////////////////////////////
	// show playing progress 
	if (1)
	{		
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER, 
								pAVIPlayer->window.ProgressBarBack.pos.left, 
								pAVIPlayer->window.ProgressBarBack.pos.top,
								pAVIPlayer->window.ProgressBarBack.data, 
								&g_Graph.TransColor, AK_FALSE);
*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.ProgressBarBack.pos, pAVIPlayer->window.ProgressBarBack.data);

		rect.left = pAVIPlayer->window.ProgressBar.pos.left;
		rect.top = pAVIPlayer->window.ProgressBar.pos.top;
		if (pAVIPlayer->fSeekPress)
		{
			rect.width = pAVIPlayer->window.ProgressBlockPress.pos.left-pAVIPlayer->window.ProgressBar.pos.left+2;
		}
		else
		{
			rect.width = pAVIPlayer->window.ProgressBlock.pos.left-pAVIPlayer->window.ProgressBar.pos.left+2;
		}
		
		rect.height = pAVIPlayer->window.ProgressBar.pos.height;
		
/*		Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
									pAVIPlayer->window.ProgressBar.pos.left, 
									pAVIPlayer->window.ProgressBar.pos.top,
									&rect, 
									pAVIPlayer->window.ProgressBar.data, 
									&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&rect, pAVIPlayer->window.ProgressBar.data);
					
		if (pAVIPlayer->fSeekPress)
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
									(T_POS)(pAVIPlayer->window.ProgressBlockPress.pos.left-2), 
									pAVIPlayer->window.ProgressBlockPress.pos.top,
									pAVIPlayer->window.ProgressBlockPress.data, 
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.ProgressBlockPress.pos, pAVIPlayer->window.ProgressBlockPress.data);
		}
		else
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
									(T_POS)(pAVIPlayer->window.ProgressBlock.pos.left-2), 
									pAVIPlayer->window.ProgressBlock.pos.top,
									pAVIPlayer->window.ProgressBlock.data, 
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.ProgressBlock.pos, pAVIPlayer->window.ProgressBlock.data);
		}

		// complete button
		if (pAVIPlayer->fCompletePress)
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
								pAVIPlayer->window.completePress.pos.left, 
								pAVIPlayer->window.completePress.pos.top,
								pAVIPlayer->window.completePress.data, 
								&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.completePress.pos, pAVIPlayer->window.completePress.data);
		}
		else
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
								pAVIPlayer->window.complete.pos.left, 
								pAVIPlayer->window.complete.pos.top,
								pAVIPlayer->window.complete.data, 
								&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.complete.pos, pAVIPlayer->window.complete.data);
		}
		
		// time
		AVIPlayer_Window_Refresh_Time(pAVIPlayer->window.curTime.time, pAVIPlayer->window.totalTime.time);
	}				   

	//////////////////////////////////////////////////////////////////
	// alpha show
	
	
								
	//////////////////////////////////////////////////////////////////
	// show prev/next/start/pause/volume progress
	
	if (pAVIPlayer->fPrevPress)
	{
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER, 
							pAVIPlayer->window.prevPress.pos.left,
							pAVIPlayer->window.prevPress.pos.top,
							pAVIPlayer->window.prevPress.data,
							&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.prevPress.pos, pAVIPlayer->window.prevPress.data);
	}
	else
	{
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER, 
							pAVIPlayer->window.prev.pos.left,
							pAVIPlayer->window.prev.pos.top,
							pAVIPlayer->window.prev.data,
							&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.prev.pos, pAVIPlayer->window.prev.data);
	}

	if (pAVIPlayer->fNextPress)
	{
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER,
							pAVIPlayer->window.nextPress.pos.left,
							pAVIPlayer->window.nextPress.pos.top,
							pAVIPlayer->window.nextPress.data,
							&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.nextPress.pos, pAVIPlayer->window.nextPress.data);
	}
	else
	{
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER,
							pAVIPlayer->window.next.pos.left,
							pAVIPlayer->window.next.pos.top,
							pAVIPlayer->window.next.data,
							&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.next.pos, pAVIPlayer->window.next.data);
	}
	
	if (MPLAYER_PLAY == AVIPlayer_GetStatus())
	{
		if(!pAVIPlayer->fPausePress)
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER,
									pAVIPlayer->window.pause.pos.left,
									pAVIPlayer->window.pause.pos.top,
									pAVIPlayer->window.pause.data,
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.pause.pos, pAVIPlayer->window.pause.data);
		}
		else
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER,
									pAVIPlayer->window.pausePress.pos.left,
									pAVIPlayer->window.pausePress.pos.top,
									pAVIPlayer->window.pausePress.data,
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.pausePress.pos, pAVIPlayer->window.pausePress.data);

		}
	}
	else
	{
		if(!pAVIPlayer->fStartPress)
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER,
									pAVIPlayer->window.start.pos.left,
									pAVIPlayer->window.start.pos.top,
									pAVIPlayer->window.start.data,
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.start.pos, pAVIPlayer->window.start.data);
		}
		else
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER,
									pAVIPlayer->window.startPress.pos.left,
									pAVIPlayer->window.startPress.pos.top,
									pAVIPlayer->window.startPress.data,
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.startPress.pos, pAVIPlayer->window.startPress.data);

		}
	}
	
	if (1)	
	{
/*		Fwl_AkBmpDrawFromString(HRGB_LAYER, 
								pAVIPlayer->window.volumeProgressBarBack.pos.left, 
								pAVIPlayer->window.volumeProgressBarBack.pos.top,
								pAVIPlayer->window.volumeProgressBarBack.data, 
								&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.volumeProgressBarBack.pos, pAVIPlayer->window.volumeProgressBarBack.data);

		rect.left = pAVIPlayer->window.volumeProgressBar.pos.left;
		rect.top = pAVIPlayer->window.volumeProgressBar.pos.top;

		if(pAVIPlayer->fVolPress)
		{
			rect.width = pAVIPlayer->window.volumeProgressBlockPress.pos.left-pAVIPlayer->window.volumeProgressBar.pos.left+2;
		}
		else
		{
			rect.width = pAVIPlayer->window.volumeProgressBlock.pos.left-pAVIPlayer->window.volumeProgressBar.pos.left+2;
		}
		
		rect.height = pAVIPlayer->window.volumeProgressBar.pos.height;
		
/*		Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
									pAVIPlayer->window.volumeProgressBar.pos.left, 
									pAVIPlayer->window.volumeProgressBar.pos.top,
									&rect, 
									pAVIPlayer->window.volumeProgressBar.data, 
									&g_Graph.TransColor, AK_FALSE);*/
		Fwl_Osd_DrawStreamBmpByGray(&rect, pAVIPlayer->window.volumeProgressBar.data);
					
		if(pAVIPlayer->fVolPress)
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
									(T_POS)(pAVIPlayer->window.volumeProgressBlockPress.pos.left-2), 
									pAVIPlayer->window.volumeProgressBlockPress.pos.top,
									pAVIPlayer->window.volumeProgressBlockPress.data, 
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.volumeProgressBlockPress.pos, pAVIPlayer->window.volumeProgressBlockPress.data);
		}
		else
		{
/*			Fwl_AkBmpDrawFromString(HRGB_LAYER, 
									(T_POS)(pAVIPlayer->window.volumeProgressBlock.pos.left-2), 
									pAVIPlayer->window.volumeProgressBlock.pos.top,
									pAVIPlayer->window.volumeProgressBlock.data, 
									&g_Graph.TransColor, AK_FALSE);*/
			Fwl_Osd_DrawStreamBmpByGray(&pAVIPlayer->window.volumeProgressBlock.pos, pAVIPlayer->window.volumeProgressBlock.data);
		}
	}

	return AK_TRUE;
}

T_VOID AVIPlayer_AspectRatio(T_pRECT paintWin, T_LEN srcW, T_LEN srcH, T_LEN dstW, T_LEN dstH)
{
	if (0 == srcW || 0 == srcH)
		return;
	
	if (srcW*dstH >= srcH*dstW)
	{
		paintWin->width 	= dstW;
		paintWin->height	= srcH * pAVIPlayer->paintInfo.width / srcW;
	}
	else
	{
		paintWin->height 	= dstH;
		paintWin->width 	= srcW * pAVIPlayer->paintInfo.height / srcH;
	}

    paintWin->left 	=  (dstW - paintWin->width) >> 1;
    paintWin->top 	=  (dstH - paintWin->height) >> 1;
}

static T_VOID AVIPlayer_CheckSize(T_U16 srcW, T_U16 srcH)
{
	AK_ASSERT_PTR_VOID(pAVIPlayer, "pAVIPlayer Is Invalid");
	
    if (!pAVIPlayer->WideScrnShow)
    {
		pAVIPlayer->paintInfo.left 	= 0;
        pAVIPlayer->paintInfo.top 	= 0;
        pAVIPlayer->paintInfo.width = LCD_WIDTH;
        pAVIPlayer->paintInfo.height = LCD_HEIGHT;        

        return;
    }

	AVIPlayer_AspectRatio((T_pRECT)&pAVIPlayer->paintInfo, srcW, srcH, LCD_WIDTH, LCD_HEIGHT);
}


T_VOID AVIPlayer_YUV2DispBuff(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 srcH)
{
	//T_RECT srcRECT;

	AK_ASSERT_PTR_VOID(y, "y Is Invalid");
	AK_ASSERT_PTR_VOID(u, "u Is Invalid");
	AK_ASSERT_PTR_VOID(v, "v Is Invalid");

	//Fwl_InitRect(&srcRECT, 0, 0, oriW, oriH);
	
	if (srcW < MIN_FRAME_WIDTH || srcH < MIN_FRAME_HEIGHT)
	{
		AK_DEBUG_OUTPUT("Frame Width/Height Less Than %d.\n", MIN_FRAME_WIDTH);
		
		AutoFullScreenCount = AUTO_FULLSCREEN_DISABLE;
		
		return;
	}    
   
	if (srcW<<1 < LCD_WIDTH)//宽、高的缩放不能超过2倍
	{
		PaintRect.width 	= srcW<<1;
		PaintRect.height 	= (srcH<<1) < LCD_HEIGHT ? srcH<<1: LCD_HEIGHT;
		PaintRect.left 		= (LCD_WIDTH - PaintRect.width) >> 1;
		PaintRect.top 		= (LCD_HEIGHT - PaintRect.height) >> 1;
	}
	else if (pAVIPlayer)
	{
		PaintRect.left 		= pAVIPlayer->paintInfo.left;
		PaintRect.top 		= pAVIPlayer->paintInfo.top;
		PaintRect.width 	= pAVIPlayer->paintInfo.width;
		PaintRect.height	= pAVIPlayer->paintInfo.height;
	}
	else
	{
		Fwl_InitRect(&PaintRect, 0, 0, LCD_WIDTH, LCD_HEIGHT);
	}

	//Fwl_YuvZoom(y, u, v, srcW, &srcRECT, Fwl_GetDispMemory565(), LCD_WIDTH, RGB565, &PaintRect);
	Fwl_RefreshYUV1(y, u, v, srcW, srcH, PaintRect.left, PaintRect.top, PaintRect.width, PaintRect.height);
}

/**
 * @brief show a frame
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] Y
 * @param[in] U
 * @param[in] src_width 
 * @param[in] src_height
 * @param[in] ori_height
 * @return returns 
 * @retval T_VOID
 */

T_VOID AVIPlayer_ShowFrame(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 srcH, T_U16 oriW, T_U16 oriH)
{
	if (AK_NULL == pAVIPlayer || AK_NULL == y || AK_NULL == u || AK_NULL == v)
	{
		return;
	}
	
	srcImgY 			= y;
	srcImgU 			= u;
	srcImgV 			= v;
       
	uDispWidth 			= srcW;
	uDispHeight			= srcH;
	uOriWidth			= oriW;
	uOriHeight 			= oriH;

	if (pAVIPlayer->bCheckSize)
    {
    	T_RECT rect = {0};
		
		//Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
		RectInit(&rect, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight());
		Fwl_Osd_FillSolidRectByGray(&rect, COLOR_BLACK);
        AVIPlayer_CheckSize(srcW, srcH);

        pAVIPlayer->bCheckSize = AK_FALSE;
    }

	if (FULL_PLAY_MODE == pAVIPlayer->play_mode
		&& DISPLAY_LCD_1 < Fwl_GetDispalyType())
	{
		//Fwl_RefreshTVOUT(y, u, v, srcW, oriW, oriH);
		Fwl_Osd_ClearDispBuf();
		
		AVIPlayer_YUV2DispBuff(y, u, v, srcW, srcH);
		Fwl_Osd_RefreshDisplay();
		//Fwl_RefreshDisplay();
		Fwl_Refresh_Output();
	}
	else
	{
		Fwl_Osd_ClearDispBuf();
		
		AVIPlayer_YUV2DispBuff(y, u, v, srcW, srcH);

		if (PaintRect.top >= pAVIPlayer->window.Back.pos.top + pAVIPlayer->window.Back.pos.height
			&& PaintRect.top + PaintRect.height <= pAVIPlayer->window.windowBack.pos.top)
			AVIPlayer_Window_Paint(15, 15);
		else if (PaintRect.top < pAVIPlayer->window.Back.pos.top + pAVIPlayer->window.Back.pos.height
			&& PaintRect.top + PaintRect.height <= pAVIPlayer->window.windowBack.pos.top)
			AVIPlayer_Window_Paint(8, 15);
		else if (PaintRect.top >= pAVIPlayer->window.Back.pos.top + pAVIPlayer->window.Back.pos.height
			&& PaintRect.top + PaintRect.height > pAVIPlayer->window.windowBack.pos.top)
			AVIPlayer_Window_Paint(15, 8);
		else
			AVIPlayer_Window_Paint(8, 8);

		Fwl_Osd_RefreshDisplay();
		Fwl_RefreshDisplayByColor(COLOR_BLACK);
		Fwl_Refresh_Output();
	}
}

T_VOID AVIPlayer_EndOneFile(T_END_TYPE endtype)
{
    EvtParam.w.Param2 = (T_U32)endtype;
    m_triggerEvent(VME_EVT_MEDIA, &EvtParam);
}

/**
 * @brief initialize player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval AK_TRUE  initialize success
 * @retval AK_FALSE initialize fail
 * 
 */

T_BOOL AVIPlayer_Init(T_VOID)
{
	
	hEnableShow = AK_Create_Semaphore(1, AK_PRIORITY);
    pAVIPlayer = (T_AVI_PLAYER *)Fwl_Malloc(sizeof(T_AVI_PLAYER));
    AK_ASSERT_PTR(pAVIPlayer, "AVIPlayer_Init():pAVIPlayer mallo fail!", AK_FALSE);
	
    AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;

    Utl_MemSet(pAVIPlayer,0,sizeof(T_AVI_PLAYER));
	
    //pAVIPlayer->tid = ERROR_TIMER;
    pAVIPlayer->prg_timer_id = ERROR_TIMER;
    pAVIPlayer->curIndex = 0;
	pAVIPlayer->bAudioEnable = AK_TRUE;
    pAVIPlayer->initialized  = AK_FALSE;
    pAVIPlayer->fSeekType    = SEEK_TYPE_INVALID;
    pAVIPlayer->fNextPress   = AK_FALSE;
    pAVIPlayer->fPrevPress   = AK_FALSE;
    pAVIPlayer->fCompletePress = AK_FALSE;
	pAVIPlayer->fStartPress  = AK_FALSE;
    pAVIPlayer->fPausePress  = AK_FALSE;
	pAVIPlayer->fSeekPress   = AK_FALSE;
	pAVIPlayer->fVolPress    = AK_FALSE;
	
    AVIPlayer_Malloc_IconBuf();
	
    Utl_UStrEmpty(pAVIPlayer->window.curFile.UText);

    pAVIPlayer->window.curFile.uTextOffset = 0;
    pAVIPlayer->window.curFile.bScroll = AK_FALSE;

	
    /**start progress bar paint timer*/
    pAVIPlayer->prg_timer_id = Fwl_SetTimerMilliSecond(WINDOW_PRG_INTERVAL, AK_TRUE);

	Fwl_KeypadSetSingleMode();
    ScreenSaverDisable();
    // Fwl_AudioEnableDA();
    //DAC_SetHeadPhone(HEADPHONE_ON);
    
    return AK_TRUE;
}
/**
 * @brief free resource
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

T_VOID AVIPlayer_Free(T_VOID)
{
    if (pAVIPlayer == AK_NULL)
        return;

    /**stop progress bar paint timer*/
    if (ERROR_TIMER != pAVIPlayer->prg_timer_id)
    {
        Fwl_StopTimer(pAVIPlayer->prg_timer_id);
        pAVIPlayer->prg_timer_id = ERROR_TIMER;
    }
	AVIPlayer_Stop(T_END_TYPE_USER);

	AK_Sleep(10);
	
    pAVIPlayer = Fwl_Free(pAVIPlayer);
  	
    AK_DEBUG_OUTPUT("AVIPlayer_Free(): Fwl_Free(pAVIPlayer).\r\n");

    //Fwl_AudioSetTrack(AUDIO_CHANNEL_STEREO);
    ScreenSaverEnable();

	AK_Delete_Semaphore(hEnableShow);
	hEnableShow = -1;
}
/**
 * @brief get current file mark
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] fname name of current file
 * @return returns 
 * @retval T_VOID
 *
 */

T_VOID AVIPlayer_GetCurFileMark(T_USTR_FILE fname)
{
	T_U16 i;
    T_USTR_FILE path, name;
	T_U16 UStrWidth = 0;
    T_U16 uTextLen = 0;
	
	//init the window
    AVIPlayer_Window_Init();
    //avoid that filenames overlap when switch the movie
    Utl_UStrEmpty(pAVIPlayer->window.curFile.UText);
	
	Utl_USplitFilePath(fname, path, name);
    //Utl_USplitFileName(name, nm, ext);
    Utl_UStrCpyN(pAVIPlayer->window.curFile.UText, name, 256);
    pAVIPlayer->window.curFile.uTextOffset = 0;

    uTextLen = (T_U16)Utl_UStrLen(pAVIPlayer->window.curFile.UText);
    UStrWidth = (T_U16)UGetSpeciStringWidth(pAVIPlayer->window.curFile.UText, CURRENT_FONT_SIZE, uTextLen);
    if (UStrWidth <= VIDEO_FILENAME_MAX_WIDTH)
    {
        pAVIPlayer->window.curFile.bScroll = AK_FALSE;
        pAVIPlayer->window.curFile.pos.x = CUR_FILE_NAME_LEFT + ((VIDEO_FILENAME_MAX_WIDTH - UStrWidth) >> 1);
    }
    else
    {
        pAVIPlayer->window.curFile.bScroll = AK_TRUE;
        pAVIPlayer->window.curFile.pos.x = CUR_FILE_NAME_LEFT;
    }
	
    //Get Current File Bookmark
    for (i=0; i<gs.AVIBookMarkCount; i++)
    {
        if (Utl_UStrCmp(gs.AVIBookmark[i].file, pAVIPlayer->window.curFile.UText) == 0)
        {
        	// Find Matching File
            break;
        }
    }

    if (i >= gs.AVIBookMarkCount)
    {
        if (gs.AVIBookMarkCount < MAX_AVI_BOOKMARK)
        {
            gs.AVIBookMarkCount++;
            pAVIPlayer->curIndex = gs.AVIBookMarkCount-1;
        }
        else
        {
            T_U16 j;
            T_U16 k = 0;

            for (j=1; j<gs.AVIBookMarkCount; j++)
                if (gs.AVIBookmark[j].savTime < gs.AVIBookmark[k].savTime)
                    k=j;

            pAVIPlayer->curIndex = k;
        }

        Utl_UStrCpy(gs.AVIBookmark[pAVIPlayer->curIndex].file, pAVIPlayer->window.curFile.UText);
        gs.AVIBookmark[pAVIPlayer->curIndex].currTime = 0;
        // gs.AVIBookmark[pAVIPlayer->curIndex].savTime = Fwl_RTCGetCount();
    }
    else
    {
        pAVIPlayer->curIndex = i;
    }
}
/**
 * @brief deal with opening video player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] fname name of playing file
 * @return returns T_AVIPLAY_ERR
 * @retval AVIPLAY_OK
 * @retval AVIPLAY_PLAYATL
 * @retval AVIPLAY_OPENERR
 * @retval AVIPLAY_NOTIMER
 * @retval AVIPLAY_PUPLICATE
 */

T_AVIPLAY_ERR AVIPlayer_Open(T_USTR_FILE fname)
{  
    srcImgY = AK_NULL;
	srcImgU = AK_NULL;
	srcImgV = AK_NULL;
	uDispWidth = 0;
	uDispHeight= 0; 
	uOriHeight = 0;
	
    memset(&EvtParam, 0, sizeof(EvtParam));
    
    Fwl_SetAudioVolumeStatus(AK_TRUE);
    pAVIPlayer->bAudioEnable = AK_TRUE;
    pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
    pAVIPlayer->WideScrnShow = AK_TRUE;
	pAVIPlayer->bCheckSize = AK_TRUE;

    AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;
#if 0	
    if (!AVIPlayer_IsSupportFileType(fname))
    {
    	AK_DEBUG_OUTPUT("AVIPlayer_Open(): Can NOT Support File Type.\n");
        return AVIPLAY_OPENERR;
    }
#endif
    if (pAVIPlayer->initialized)
    {
    	AK_DEBUG_OUTPUT("AVIPlayer_Open(): Have Been Initialized.\n");
        return AVIPLAY_DUPLICATE;
    }

    Utl_UStrCpy(filename, fname);
   	
	AVIPlayer_GetCurFileMark(filename);
	
	if(!MPlayer_Open(filename, AK_TRUE))
	{
		AK_DEBUG_OUTPUT("AVIPlayer_Open(): MT Media File Open Failed!\n");
       
        return AVIPLAY_OPENERR;
	}    

    MPlayer_SetShowFrameCB(AVIPlayer_ShowFrame);
    MPlayer_SetEndCB(AVIPlayer_EndOneFile);
	
	pAVIPlayer->initialized = AK_TRUE;
	
    Fwl_AudioSetTrack(eSOUND_TRACK_STEREO);	// _SD_CHANNEL_STEREO;
    pAVIPlayer->bAudioEnable = AK_TRUE;
    
    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_STATUS|WINDOW_REFRESH_FILENAME);

    pAVIPlayer->window.totalTime.time = MPlayer_GetTotalTime()/ 1000;   //Get file total playing time from video lib;

    Fwl_AudioSetVolume(Fwl_GetAudioVolume());

    /**disable auto power off*/
    AutoPowerOffDisable(FLAG_VIDEO);
	
	WaveOut_SetFade(1000, FADE_STATE_IN);
	
	if (!MPlayer_Play(gs.AVIBookmark[pAVIPlayer->curIndex].currTime))
	{
		return AVIPLAY_PLAYFAIL;
	}
		
    Fwl_Print(C3, M_VIDEO, "avi play ok");
    
    return AVIPLAY_OK;
}

T_BOOL AVIPlayer_Paint(T_VOID)
{
	return AK_TRUE;
}

/**
 * @brief deal with pause of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_Pause(T_VOID)
{
    HJH_DEBUG("pause.\n");

	// deal with fade out
	WaveOut_SetFade(200, FADE_STATE_OUT);
	AK_Sleep(40);
			
    MPlayer_Pause();
	// refresh window
    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_STATUS);
	AutoPowerOffEnable(FLAG_VIDEO); /**Enable power off*/  
}

 /**
 * @brief deal with resume of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_Resume(T_VOID)
{
    HJH_DEBUG("resume.\r\n");

    if (pAVIPlayer->bAudioEnable)//not mute
    {
        Fwl_AudioSetVolume(Fwl_GetAudioVolume());
        Fwl_SetAudioVolumeStatus(AK_TRUE);
    }
    
    else//mute
    {     
        Fwl_AudioMute();
        Fwl_SetAudioVolumeStatus(AK_FALSE);
    }
	
	WaveOut_SetFade(1000, FADE_STATE_IN);
	
	MPlayer_Resume();
    
    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_STATUS);

    /**disable auto power off*/
    AutoPowerOffDisable(FLAG_VIDEO);
}
/**
 * @brief deal with stop of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

T_BOOL AVIPlayer_Stop(T_END_TYPE endType)
{
	T_BOOL ret = AK_TRUE;
	
    if ((pAVIPlayer == AK_NULL) || (!pAVIPlayer->initialized))
        return ret;

	if (T_END_TYPE_USER == endType
		|| T_END_TYPE_ERR == endType)
	{
		WaveOut_SetFade(50, FADE_STATE_OUT);
		AK_Sleep(15);
	}
	else if (T_END_TYPE_EXIT == endType)
	{
		WaveOut_SetFade(1000, FADE_STATE_OUT);
		AK_Sleep(100);
	}
	
    if (AVIPlayer_GetStatus() != MPLAYER_ERR)
    {
        T_U32 curtime = MPlayer_GetCurTime();	
        
        if (curtime +1000*10 < MPlayer_GetTotalTime()
			&& 1000 < curtime 
			&& MPlayer_AllowSeek())        
        {
            gs.AVIBookmark[pAVIPlayer->curIndex].currTime = curtime;
        }
        else
        {
            gs.AVIBookmark[pAVIPlayer->curIndex].currTime = 0;
        }
    }

    AK_DEBUG_OUTPUT("AVIPlayer_Stop(): Index = %d, curtime = %d.\n",pAVIPlayer->curIndex, gs.AVIBookmark[pAVIPlayer->curIndex].currTime);

    ret = MPlayer_Close();
    
    if (!pAVIPlayer->bAudioEnable)
    {
        pAVIPlayer->bAudioEnable = AK_TRUE;
        Fwl_SetAudioVolumeStatus(AK_TRUE);
        Fwl_AudioSetVolume(pAVIPlayer->volume_old);
    }

     pAVIPlayer->initialized = AK_FALSE;   
    
    AutoPowerOffEnable(FLAG_VIDEO); /**Enable power off*/  

    return ret;
}
/**
 * @brief deal with suspend of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

T_VOID AVIPlayer_OperateSuspend(T_VOID)
{
    HJH_DEBUG("AVIPlayer_OperateSuspend.\n");
    
	if (pAVIPlayer->initialized)
	{
	    savStatus = AVIPlayer_GetStatus();
	   
		MPlayer_Pause();
	}
	else
	{
		savStatus = MPLAYER_ERR;
	}

    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_STATUS);
}
/**
 * @brief deal with resume of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

T_VOID AVIPlayer_OperateResume(T_VOID)
{
    AK_DEBUG_OUTPUT("AVIPlayer_OperateResume\r\n");
    
    switch (savStatus)
    {
    case MPLAYER_PLAY:
		MPlayer_Resume();
        break;

    case MPLAYER_PAUSE:
		MPlayer_Pause();
        break;
        
    default:
        break;               
    }

    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_ALL);
}

/**
 * @brief timer of refresh progress block
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_HandleEvtTimer(T_VOID)
{
	if (MPLAYER_PAUSE == AVIPlayer_GetStatus() && AK_NULL != srcImgY)
    {
      	//for paint alterable image when pause
        AVIPlayer_ShowFrame(srcImgY, srcImgU, srcImgV, uDispWidth, uDispHeight, uOriWidth, uOriHeight); 
    }
            
	if (pAVIPlayer->initialized)
    {
        if (MPLAYER_STOP == AVIPlayer_GetStatus())
        {
            pAVIPlayer->window.curTime.time = 0;    //set curTime = 0;
            pAVIPlayer->window.ProgressBlock.pos.left = pAVIPlayer->window.ProgressBar.pos.left ;
        }
        //if dosen`t hold slide Progress block, refresh curtime    
        else if(!pAVIPlayer->fSeekPress)
        {
            T_U32 cur_time;
            T_U32 total_time = MPlayer_GetTotalTime();

			//if (pAVIPlayer->fSeekType == SEEK_TYPE_INVALID)
            {
            	cur_time = MPlayer_GetCurTime();
            }
            //else
            {
           // 	cur_time = pAVIPlayer->SeekTime * 1000;
            }

            pAVIPlayer->window.curTime.time = cur_time / 1000;      //Get curTime from video lib;
           
            if (cur_time >= total_time)
            {
				pAVIPlayer->window.ProgressBlock.pos.left 
                  	= (T_POS)(pAVIPlayer->window.ProgressBar.pos.left 
                  			  + pAVIPlayer->window.ProgressBar.pos.width
                  			  - pAVIPlayer->window.ProgressBlock.pos.width/2);  
            }
            else
            {
				pAVIPlayer->window.ProgressBlock.pos.left
					=(T_POS)(pAVIPlayer->window.ProgressBar.pos.left 
					+ ((T_U64)cur_time * (pAVIPlayer->window.ProgressBar.pos.width - pAVIPlayer->window.ProgressBlock.pos.width/2) / total_time)); //dengzhou						
            }

        }

		if (!MPlayer_HasVideo())
		{		
			Fwl_Osd_ClearDispBuf();
			
			AVIPlayer_Window_Paint(15, 15);
			
			Fwl_Osd_RefreshDisplay();
			Fwl_RefreshDisplay();
			Fwl_Refresh_Output();
		}
          
    }
}
/**
 * @brief timer of seeking function,and if full screen showing
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_HandlePubTimer(T_VOID)
{
	//static T_U16 time	= 0;
	//T_U16 interval		= 0;
	
#if 1
	//pAVIPlayer->SeekTime = MPlayer_GetCurTime()/1000;
#else
    // seek 
    if (pAVIPlayer->fSeekType != SEEK_TYPE_INVALID)
    {
    	time++;
    	pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
    	AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;
		// interval add following time
    	interval = (time/4)*6 + 2;
    	
		if (pAVIPlayer->fSeekType == SEEK_TYPE_BACKWARD)
		{
			if ((T_S32)(pAVIPlayer->SeekTime-interval) > 0)
			{
				pAVIPlayer->SeekTime -= interval;
			}
			else
			{
				pAVIPlayer->SeekTime = 0;
			}
		}
		else if (pAVIPlayer->fSeekType == SEEK_TYPE_FORWARD)
		{
			if (pAVIPlayer->SeekTime+interval < pAVIPlayer->window.totalTime.time)
			{
				pAVIPlayer->SeekTime += interval;
			}
			else
			{
				pAVIPlayer->SeekTime = pAVIPlayer->window.totalTime.time;
			}
		}
	}
	else
	{
		time = 0;
	}
#endif

	if (srcImgY			// The Value Will NULL When Video Decoder NOT Support
		&& AutoFullScreenCount < AUTO_FULLSCREEN_TIME 
        && MPLAYER_PLAY == AVIPlayer_GetStatus())
    {
        AutoFullScreenCount++;
    }
    else if ( AutoFullScreenCount != AUTO_FULLSCREEN_DISABLE 
      	&& AutoFullScreenCount >= AUTO_FULLSCREEN_TIME
       	&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE 
        && MPLAYER_STOP != AVIPlayer_GetStatus())
    {     
        pAVIPlayer->play_mode = FULL_PLAY_MODE;
		pAVIPlayer->bCheckSize = AK_TRUE;
    }
	
}
/**
 * @brief deal with touching screen event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pEventParm
 * @return returns 
 * @retval T_eBACK_STATE
 */

static T_eBACK_STATE AVIPlayer_HandleTScr(T_EVT_PARAM *pEventParm)
{
	T_POS x = (T_POS)pEventParm->s.Param2;
    T_POS y = (T_POS)pEventParm->s.Param3;

    switch (pEventParm->s.Param1) 
    {
    case eTOUCHSCR_UP:
        /* if the point(x,y) hit in the control buttons rect,  
                  transform it to the corresponding key */
        return AVIPlayer_UserKey_Handle(AVIPlayer_MapTSCR_To_Key(x, y, pEventParm));
        break;
        
    case eTOUCHSCR_DOWN:
		AutoFullScreenCount = AUTO_FULLSCREEN_DISABLE;

		if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
	    {
	        AVIPlayer_SwitchWindowMode();
	    }
		
    	//hit previous button
	    if (PointInRect(&(pAVIPlayer->window.prev.pos), x, y)
			&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE)
	    {
	        pAVIPlayer->fPrevPress = AK_TRUE;
	    }
	    //hit next button
	    if (PointInRect(&(pAVIPlayer->window.next.pos), x, y)
			&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE)
	    {
	        pAVIPlayer->fNextPress = AK_TRUE;
	    }
	    //hit complete button
	    if (PointInRect(&(pAVIPlayer->window.complete.pos), x, y)
			&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE)
	    {
	        pAVIPlayer->fCompletePress = AK_TRUE;
	    }
		//hit Pause button
		if (PointInRect(&(pAVIPlayer->window.pause.pos), x, y)
			&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE
			&& MPLAYER_PLAY == AVIPlayer_GetStatus())
	    {
	        pAVIPlayer->fPausePress = AK_TRUE;
			pAVIPlayer->fStartPress = AK_FALSE;
	    }
		//hit Start button
	    else if(PointInRect(&(pAVIPlayer->window.start.pos), x, y)
				&& pAVIPlayer->play_mode == WINDOW_PLAY_MODE)
	    {
	        pAVIPlayer->fStartPress = AK_TRUE;
			pAVIPlayer->fPausePress = AK_FALSE;
	    }

		//hold Progress block
		if(PointInRect(&(pAVIPlayer->window.ProgressBlock.pos), x, y))
		{
			pAVIPlayer->fSeekPress = AK_TRUE;
			pAVIPlayer->window.ProgressBlockPress.pos.left = pAVIPlayer->window.ProgressBlock.pos.left;
		}

		//hold volume block
		if(PointInRect(&(pAVIPlayer->window.volumeProgressBlock.pos), x, y))
		{
			pAVIPlayer->fVolPress = AK_TRUE;
			pAVIPlayer->window.volumeProgressBlockPress.pos.left = pAVIPlayer->window.volumeProgressBlock.pos.left;
		}
        break;
         
    case eTOUCHSCR_MOVE:
		//whether not in hold progress block mode
		if (!pAVIPlayer->fSeekPress && !pAVIPlayer->fVolPress) 
		{
			//hit previous button
			if (PointInRect(&(pAVIPlayer->window.prev.pos), x, y))
		    {
		        pAVIPlayer->fPrevPress = AK_TRUE;
		    }
			else
			{
				pAVIPlayer->fPrevPress = AK_FALSE;

			}
		    //hit next button
		    if (PointInRect(&(pAVIPlayer->window.next.pos), x, y))
		    {
		        pAVIPlayer->fNextPress = AK_TRUE;
		    }
			else
			{
				pAVIPlayer->fNextPress = AK_FALSE;

			}
		    //hit complete button
		    if (PointInRect(&(pAVIPlayer->window.complete.pos), x, y))
		    {
		        pAVIPlayer->fCompletePress = AK_TRUE;
		    }
			else
			{
				pAVIPlayer->fCompletePress = AK_FALSE;

			}
			//hit Pause button or hit Start button
		    if (PointInRect(&(pAVIPlayer->window.pause.pos), x, y))
		    {
		    	if(MPLAYER_PLAY == AVIPlayer_GetStatus())
		    	{
					pAVIPlayer->fPausePress = AK_TRUE;
		    	}
				else
				{
					pAVIPlayer->fStartPress = AK_TRUE;
				}
				
		    }
			else
			{
				pAVIPlayer->fPausePress = AK_FALSE;
				pAVIPlayer->fStartPress = AK_FALSE;
			}
		}

	 	// hit  progress slide rect, go to the correspongding position
		if (pAVIPlayer->fSeekPress
			&& PointInRect(&(pAVIPlayer->window.Back.pos), x, y)
			&& (MPLAYER_STOP != AVIPlayer_GetStatus()))
		{
			T_U32 SeekTime = 0;
			T_U32 total_time = MPlayer_GetTotalTime();
			
			SeekTime = (x - pAVIPlayer->window.ProgressBarBack.pos.left) <= 0
						? 0	
						: (total_time * (x - pAVIPlayer->window.ProgressBarBack.pos.left) 
									  / pAVIPlayer->window.ProgressBarBack.pos.width);
			
			if (SeekTime >= total_time)
            {
            	SeekTime = total_time;
				pAVIPlayer->window.ProgressBlockPress.pos.left 
                  	= (T_POS)(pAVIPlayer->window.ProgressBar.pos.left 
                  			  + pAVIPlayer->window.ProgressBar.pos.width
                  			  - pAVIPlayer->window.ProgressBlock.pos.width/2);  
            }
            else
            {
				pAVIPlayer->window.ProgressBlockPress.pos.left
					= (T_POS)(pAVIPlayer->window.ProgressBar.pos.left 
							  + ((T_U64)SeekTime 
							  * (pAVIPlayer->window.ProgressBar.pos.width - pAVIPlayer->window.ProgressBlock.pos.width/2) 
							  / total_time));					
            }
			
			pAVIPlayer->window.curTime.time = SeekTime/1000;
		}
		else if(pAVIPlayer->fSeekPress
				&& !PointInRect(&(pAVIPlayer->window.Back.pos), x, y))
		{
			pAVIPlayer->fSeekPress = AK_FALSE;
		}
		
		//hit volume bar rect
		if (pAVIPlayer->bAudioEnable 
			&& pAVIPlayer->fVolPress == AK_TRUE)
		{
			T_S16 SeekVol = 0;
			
		    // the volume rang is integer from 0 to AK_VOLUME_MAX
			SeekVol = (AK_VOLUME_MAX * (x - pAVIPlayer->window.Volume.left) % pAVIPlayer->window.Volume.width <= 0)
						? (AK_VOLUME_MAX * (x - pAVIPlayer->window.Volume.left) / pAVIPlayer->window.Volume.width)
						: (AK_VOLUME_MAX * (x - pAVIPlayer->window.Volume.left) / pAVIPlayer->window.Volume.width) + 1;
			
			if (SeekVol < 0)
			{
				SeekVol = 0;
			}
			else if (SeekVol > AK_VOLUME_MAX)
			{
				SeekVol = AK_VOLUME_MAX;
			}
			
			if (SeekVol != Fwl_GetAudioVolume())
			{
				Fwl_AudioSetVolume(SeekVol);
				AVIPlayer_UpdateVolumeBlockLoc();
			}
		}
		else if(pAVIPlayer->fVolPress == AK_TRUE
				&& !PointInRect(&(pAVIPlayer->window.windowBack.pos), x, y))
		{
			pAVIPlayer->fVolPress = AK_FALSE;
			pAVIPlayer->window.volumeProgressBlock.pos.left = pAVIPlayer->window.volumeProgressBlockPress.pos.left;
		}
		break;
         
    default:
         break;
    }

	return eStay;
}
/**
 * @brief handle of player
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] event
 * @param[in] pEventParm
 * @return returns 
 * @retval T_VOID
 */

T_eBACK_STATE AVIPlayer_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	T_MMI_KEYPAD phyKey;
	
	switch (event)
	{
	case VME_EVT_TIMER:
		if ((T_U32)pAVIPlayer->prg_timer_id == pEventParm->w.Param1)
            AVIPlayer_HandleEvtTimer();
            
    	break;

	case M_EVT_PUB_TIMER:
		AVIPlayer_HandlePubTimer();
		break;

	case M_EVT_USER_KEY:
		phyKey.keyID = pEventParm->c.Param1;
        phyKey.pressType = pEventParm->c.Param2;

        return AVIPlayer_UserKey_Handle(phyKey);
		break;

	case M_EVT_TOUCH_SCREEN:
		return AVIPlayer_HandleTScr(pEventParm);
		break;

	default:
		break;
	}
    
    return eStay;
}
/**
 * @brief swith window mode, if show control panel
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] T_VOID
 * @return returns 
 * @retval T_VOID
 */

static T_VOID AVIPlayer_SwitchWindowMode(T_VOID)
{
	pAVIPlayer->play_mode = WINDOW_PLAY_MODE;

    AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_ALL);
        
}
/**
 * @brief deal with short press of key event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] keyID
 * @return returns 
 * @retval T_eBACK_STATE
 */

static T_eBACK_STATE AVIPlayer_UserKey_HandleShortPress(T_U32 keyID)
{	
	T_eMPLAYER_STATUS avi_status = AVIPlayer_GetStatus();

	switch(keyID)
    {
    case kbOK:
        AK_DEBUG_OUTPUT("state=%d\n", avi_status);

		if(avi_status == MPLAYER_PLAY)
        {				
            AVIPlayer_Pause();
        }                
        else  if(avi_status == MPLAYER_PAUSE)
        {
            Fwl_AudioSetVolume(Fwl_GetAudioVolume());
            AVIPlayer_Resume();
        }                
        else
        {
			WaveOut_SetFade(1000, FADE_STATE_IN);
			MPlayer_Start(0);
			
			return eStay;
        }        
        break;

    case kbLEFT:
        if (avi_status == MPLAYER_PLAY)
        {   			
            if (pAVIPlayer->bAudioEnable)
            {
                 pAVIPlayer->bAudioEnable = AK_FALSE;
				 pAVIPlayer->volume_old = Fwl_GetAudioVolume();
                 Fwl_AudioMute();
                 Fwl_SetAudioVolumeStatus(AK_FALSE);
            }
            else
            {
                pAVIPlayer->bAudioEnable = AK_TRUE;
                Fwl_AudioSetVolume(pAVIPlayer->volume_old);
                Fwl_SetAudioVolumeStatus(AK_TRUE);
            }

        }        
        break;

    case kbRIGHT:
        if (avi_status == MPLAYER_PLAY && pAVIPlayer->bAudioEnable)
        {   
        	if (Fwl_AudioChangeTrack())
                AK_DEBUG_OUTPUT("change channel OK");
            else
                AK_DEBUG_OUTPUT("change channel failed");

        }        
        break;

    case kbUP:
 		if (avi_status == MPLAYER_PLAY || avi_status == MPLAYER_PAUSE)
        {
            if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
            {	
                pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
            }
        }
		
		if (pAVIPlayer->fSeekType == SEEK_TYPE_BACKWARD)
			return eNext;
        break;

    case kbDOWN:
        if (avi_status == MPLAYER_PLAY || avi_status == MPLAYER_PAUSE)
        {
            if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
            {
                pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
            }
        }        
				
		if (pAVIPlayer->fSeekType == SEEK_TYPE_FORWARD)
			return eNext;
        break;

    case kbMENU:
        if (avi_status != MPLAYER_STOP && MPlayer_HasVideo())
        {
            if (pAVIPlayer->play_mode == WINDOW_PLAY_MODE)
            {
                pAVIPlayer->play_mode = FULL_PLAY_MODE;
            }
            else
            {
                pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
                AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;
            }
			
			pAVIPlayer->bCheckSize = AK_TRUE;
		}

        break;

    case kbSWA:		        
        if (WINDOW_PLAY_MODE == pAVIPlayer->play_mode && MPlayer_HasVideo())
        {
            pAVIPlayer->play_mode = FULL_PLAY_MODE;
			
        }

		if (!pAVIPlayer->WideScrnShow)
		{
            pAVIPlayer->WideScrnShow = AK_TRUE;
		}
        else
        {
            pAVIPlayer->WideScrnShow = AK_FALSE;
		}

		pAVIPlayer->bCheckSize = AK_TRUE;
        break;

    case kbCLEAR:

 		AVIPlayer_Stop(T_END_TYPE_EXIT);
        return eReturn;
              
    default:
        break;
    }

    return eStay;
}
/**
 * @brief deal with long press of key event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] keyID
 * @return returns 
 * @retval T_eBACK_STATE
 */

static T_eBACK_STATE AVIPlayer_UserKey_HandleLongPress(T_U32 keyID)
{
	T_eMPLAYER_STATUS avi_status = AVIPlayer_GetStatus();
	
	switch(keyID)
    {
    case kbOK:
        if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
        {
            pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
            AutoFullScreenCount = AUTO_FULLSCREEN_DISABLE;
        }

        Fwl_KeyStop();
        
        if (MPLAYER_STOP != avi_status)
        {
			WaveOut_SetFade(1000, FADE_STATE_OUT);
			AK_Sleep(100);
			
			MPlayer_Stop();
        }

        pAVIPlayer->window.curTime.time = 0;                    //set curTime = 0 when stop to play;
        pAVIPlayer->window.ProgressBlock.pos.left = pAVIPlayer->window.ProgressBar.pos.left ;

        AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_ALL);

        // Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);

        AK_Sleep(10);

		AVIPlayer_ShowFrame(srcImgY, srcImgU, srcImgV, uDispWidth, uDispHeight, uOriWidth, uOriHeight);

		Fwl_Invalidate();

        gs.AVIBookmark[pAVIPlayer->curIndex].currTime = 0;

        /**Enable power off*/
        AutoPowerOffEnable(FLAG_VIDEO);        
        break;

	case kbUP:
    case kbDOWN:
        Fwl_KeyStop();

        HJH_DEBUG("++++++long press+++++++\n",pAVIPlayer->SeekTime);

        if((avi_status == MPLAYER_PLAY  || avi_status == MPLAYER_PAUSE) 
			&& MPlayer_AllowSeek())
        {
        	if (avi_status == MPLAYER_PLAY)
        	{
        		AVIPlayer_Pause();
        	}
        	
            if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
            {
                pAVIPlayer->play_mode = WINDOW_PLAY_MODE;
            }

            if (kbUP == keyID)
            {
            	pAVIPlayer->fSeekType = SEEK_TYPE_BACKWARD;
				//pAVIPlayer->SeekTime = MPlayer_GetCurTime()/1000 - 2 > 0 ? 
				//						MPlayer_GetCurTime()/1000 - 2 : 0;
				MPlayer_FastRewind(MPlayer_GetCurTime());
            }
			else
			{
                pAVIPlayer->fSeekType = SEEK_TYPE_FORWARD; 
				//pAVIPlayer->SeekTime = MPlayer_GetCurTime()/1000 + 3;

				MPlayer_FastForward(MPlayer_GetCurTime());
			}

            HJH_DEBUG("first SeekTime=%d\n",pAVIPlayer->SeekTime);
        }
        break;

	case kbCLEAR:
        return eHome;
/*
    case kbUP:
    	pAVIPlayer->fPrevPress = AK_TRUE;
    	break;
    case kbDOWN:
    	pAVIPlayer->fNextPress = AK_TRUE;
    	break;
*/
    default:
        break;
    }
   
	return eStay;
}

/**
 * @brief deal with key event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] phykey keyID and press type(short press or long press)
 * @return returns 
 * @retval T_eBACK_STATE
 */

static T_eBACK_STATE AVIPlayer_UserKey_Handle(T_MMI_KEYPAD phyKey)
{
    T_eMPLAYER_STATUS avi_status;

    avi_status = AVIPlayer_GetStatus();

    //switch to WINDOW_PLAY_MODE
    if ((phyKey.keyID != kbMENU) 
		&& (phyKey.keyID != kbCLEAR) 
		&& (phyKey.keyID != kbSWA) 
		&& (avi_status != MPLAYER_STOP) 
		&& (pAVIPlayer->play_mode == FULL_PLAY_MODE))
    {
        AVIPlayer_SwitchWindowMode();
    }

    AutoFullScreenCount = AUTO_FULLSCREEN_ENABLE;

#if (KEYPAD_NUM == 7)
    if (phyKey.keyID == kbUP && phyKey.pressType == PRESS_SHORT)
    {
        phyKey.keyID = kbVOICE_UP;
		
        if (Fwl_GetAudioVolumeStatus())
            Fwl_AudioSetVolume(Fwl_AudioVolumeAdd());
    }

    if (phyKey.keyID == kbDOWN && phyKey.pressType == PRESS_SHORT)
    {
        phyKey.keyID = kbVOICE_DOWN;
		
        if (Fwl_GetAudioVolumeStatus())
            Fwl_AudioSetVolume(Fwl_AudioVolumeSub());
    }
#endif

    if (phyKey.keyID == kbVOICE_UP || phyKey.keyID == kbVOICE_DOWN )
    {	
		AVIPlayer_UpdateVolumeBlockLoc();
        AVIPlayer_Window_SetRefresh(WINDOW_REFRESH_ALL);

        if (avi_status == MPLAYER_STOP)
        {
			AVIPlayer_ShowFrame(srcImgY, srcImgU, srcImgV, uDispWidth, uDispHeight, uOriWidth, uOriHeight);				
        }

        return eStay;
    }

    if (phyKey.pressType == PRESS_SHORT)
    {
        return AVIPlayer_UserKey_HandleShortPress(phyKey.keyID);
    }
    
    else if (phyKey.pressType == PRESS_LONG)
    {
        return AVIPlayer_UserKey_HandleLongPress(phyKey.keyID);
    }
    else if (phyKey.pressType == PRESS_UP)
    {
    	if ((pAVIPlayer->fSeekType == SEEK_TYPE_BACKWARD&&phyKey.keyID == kbUP)
    	  ||(pAVIPlayer->fSeekType == SEEK_TYPE_FORWARD&&phyKey.keyID == kbDOWN))
    	{
			pAVIPlayer->fSeekType = SEEK_TYPE_INVALID;

			HJH_DEBUG("SeekTime=%d,seek_type=%d\n",pAVIPlayer->SeekTime*1000,pAVIPlayer->fSeekType);

			WaveOut_SetFade(1000, FADE_STATE_IN);
			
    		MPlayer_Seek(/*pAVIPlayer->SeekTime*1000*/MPlayer_GetCurTime());

			if(!pAVIPlayer->bAudioEnable)// mute
			{
				Fwl_AudioMute();
			}
    	}
    	else if (phyKey.keyID == kbUP)
    	{
    		pAVIPlayer->fPrevPress = AK_FALSE;
    	}
    	else if (phyKey.keyID == kbDOWN)
    	{
    		pAVIPlayer->fNextPress = AK_FALSE;
    	}
    }

    return eStay;
}


#endif

T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName)
{
	T_BOOL ret = AK_FALSE;

#ifdef SUPPORT_VIDEOPLAYER

    T_U8 FileType;
    
    T_MEDIALIB_CHECK_OUTPUT checkOutput;
    memset(&checkOutput,0,sizeof(checkOutput));

    FileType = Utl_GetFileType((T_pWSTR)pFileName);

    switch (FileType)
    {
    case FILE_TYPE_AVI:
    case FILE_TYPE_AKV:
    case FILE_TYPE_3GP:
    case FILE_TYPE_MP4:
    case FILE_TYPE_FLV:
	// case FILE_TYPE_RMVB:
	// case FILE_TYPE_RM:
	// case FILE_TYPE_MKV:

        ret = AK_TRUE;
        break;
    default:
    	break;
    }
    
    if (FILE_TYPE_MP4 == FileType)
    {
    	T_MEDIALIB_CHECK_OUTPUT ckOut;
		T_eMEDIALIB_MEDIA_TYPE type;
      
		type = Media_CheckFile(&ckOut, (T_pVOID)pFileName, AK_TRUE);
		
		if (MEDIALIB_MEDIA_UNKNOWN == type || !ckOut.m_bHasVideo)
            ret = AK_FALSE;  
    }
#endif
    return ret;
}


T_BOOL AVIPlayer_IsSupportSeekType(T_eMEDIALIB_MEDIA_TYPE mediaType)
{
    T_BOOL ret = AK_TRUE;

    switch (mediaType)
    {
    case MEDIALIB_MEDIA_FLV:
        ret = AK_FALSE;
        break;
    }

    return ret;
}

T_AVIPLAY_ERR AVIPlayer_CheckVideoFile(T_pCWSTR pFilePath)
{    
#ifdef SUPPORT_VIDEOPLAYER
	T_MEDIALIB_CHECK_OUTPUT ckOut;
	T_eMEDIALIB_MEDIA_TYPE type;	
      
	type = Media_CheckFile(&ckOut, (T_pVOID)pFilePath, AK_TRUE);
	
	if (type >= MEDIALIB_MEDIA_AKV  
		&& type <= MEDIALIB_MEDIA_REAL
        && ckOut.m_bHasVideo)
        {
            return AVIPLAY_OK;
        }
#endif
    return AVIPLAY_OPENERR;
}

#ifdef SUPPORT_VIDEOPLAYER
T_VOID AVIPlayer_UpdateVideoList(T_VOID)
{
	MList_AddItem(Fwl_GetDefPath(eVIDEO_PATH), AK_TRUE, AK_FALSE, eMEDIA_LIST_VIDEO);
}




T_BOOL AVIPlayer_PlayFromFile(T_pWSTR FullPath)
{
    Fwl_Print(C3, M_VIDEO, "Calling AVIPlayer_PlayFromFile() ... ...\n");
    if (FullPath == AK_NULL)
        return AK_FALSE;

    if (AVIPLAY_OK != AVIPlayer_Open(FullPath))
    {
		Fwl_Print(C3, M_VIDEO, "avi open error");
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief set text scroll offset
 *
 * @author zhengwenbo
 * @date  2007-03-20
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID AVIPlayer_SetTextOffset(T_VOID)
{
    T_U16 uTextLen = 0;

    if (pAVIPlayer && pAVIPlayer->window.curFile.bScroll)
    {
        uTextLen = (T_U16)Utl_UStrLen(pAVIPlayer->window.curFile.UText);
        pAVIPlayer->window.curFile.uTextOffset++;
        
        if (pAVIPlayer->window.curFile.uTextOffset + AVI_WINDOW_TAILSPACE > uTextLen)
        {
            pAVIPlayer->window.curFile.uTextOffset = 0;
        }
    }
}

/**
 * @brief get medialib status
 *
 * @author zhengwenbo
 * @date  2009-03-02
 * @param T_VOID
 * @return T_eMEDIALIB_STATUS
 * @retval
 */
T_eMPLAYER_STATUS  AVIPlayer_GetStatus(T_VOID)
{        
    return MPlayer_GetStatus();
}

T_BOOL AVIPlayer_IsPlaying(T_VOID)
{
    if (pAVIPlayer != AK_NULL)
        return AK_TRUE;
    else
        return AK_FALSE;
}

static T_MMI_KEYPAD AVIPlayer_MapTSCR_To_Key(T_POS x, T_POS y, T_EVT_PARAM *pEventParm)
{
    T_MMI_KEYPAD    phyKey;
    T_AVI_WINDOW*   pWindow;
    
    phyKey.keyID = kbNULL;
    phyKey.pressType = PRESS_SHORT;
    
    pWindow = &pAVIPlayer->window;

    if (pAVIPlayer->play_mode == FULL_PLAY_MODE)
    {
        return phyKey;
    }

	//whether in hold progress block mode
	if (!pAVIPlayer->fSeekPress && !pAVIPlayer->fVolPress)	
	{
	    //hit play/pause button
	    if (PointInRect(&pWindow->start.pos, x, y))
	    {
	        phyKey.keyID = kbOK;
			pAVIPlayer->fPausePress = AK_FALSE;
			pAVIPlayer->fStartPress = AK_FALSE;
	    }
	    
	    //hit complete button
	    //if (PointInRect(&pWindow->complete.pos, x, y))
	    //{
	    //    phyKey.keyID = kbOK;
	    //    phyKey.pressType = PRESS_LONG;
	    //}

	    //hit previous button, pass the key type to the s_video_player
	    if (PointInRect(&pWindow->prev.pos, x, y)
			&& pAVIPlayer->fPrevPress)
	    {
	        pEventParm->c.Param1 = kbUP;
	        pEventParm->c.Param2 = PRESS_LONG;
	        pAVIPlayer->fPrevPress = AK_FALSE;
	    }

	    //hit next button, pass the key type to the s_video_player
	    if (PointInRect(&pWindow->next.pos, x, y)
			&& pAVIPlayer->fNextPress)
	    {
	        pEventParm->c.Param1 = kbDOWN;
	        pEventParm->c.Param2 = PRESS_LONG;
	        pAVIPlayer->fNextPress = AK_FALSE;
	    }

	    //hit complete button
	    if (PointInRect(&pWindow->complete.pos, x, y))
	    {
	        phyKey.keyID = kbCLEAR;
	        pAVIPlayer->fCompletePress = AK_FALSE;
	    }
    }

	// hit  progress rect, go to the correspongding position
    if (pAVIPlayer->fSeekPress
		&& PointInRect(&(pAVIPlayer->window.Back.pos), x, y))
    {
		// deal with fade out
		WaveOut_SetFade(500, FADE_STATE_OUT);
		AK_Sleep(50);

		if(pAVIPlayer->window.curTime.time < pAVIPlayer->window.totalTime.time)
		{
			Fwl_AudioSeek(pAVIPlayer->window.curTime.time * 1000);
		}
		else
		{
			Fwl_AudioSeek(pAVIPlayer->window.totalTime.time * 1000);
		}
		
		if(!pAVIPlayer->bAudioEnable)// mute
		{
			Fwl_AudioMute();
		}

		//Stop hold progress Block
		pAVIPlayer->fSeekPress = AK_FALSE;
   }

	//Stop Press vol Block
	if (pAVIPlayer->fVolPress)
	{
		pAVIPlayer->fVolPress = AK_FALSE;
		pAVIPlayer->window.volumeProgressBlock.pos.left = pAVIPlayer->window.volumeProgressBlockPress.pos.left;
	}

    return phyKey;
}

#endif

// End of This File

