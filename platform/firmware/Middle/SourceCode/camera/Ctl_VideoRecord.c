/*
MODELNAME: VIDEO RECORD CONTROL
DISCRIPTION:
AUTHOR:ZHENGWENBO
DATE:2006-7-10
*/
#include "Fwl_public.h"

#ifdef CAMERA_SUPPORT


#include "Fwl_Initialize.h"
#include "fwl_osfs.h"
#include "eng_string.h"
#include "eng_gblstring.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "eng_graph.h"
#include "fwl_osMalloc.h"
#include "fwl_keyhandler.h"
#include "ctl_videorecord.h"
#include "fwl_pfcamera.h"
#include "fwl_pfkeypad.h"
#include "Fwl_Image.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "eng_font.h"
#include "eng_debug.h"
#include "Eng_Math.h"
#include "fwl_oscom.h"
#include "lib_image_api.h"
#include "fwl_pfdisplay.h"
#include "arch_lcd.h"
#include "file.h"
#include "Eng_Dynamicfont.h"
#include "fwl_pfdisplay.h"
#include "fwl_display.h"
#include "Fwl_sd.h"
#include "Eng_Time.h"
#include "Drv_api.h"
#include "Fwl_tscrcom.h"
#include "fwl_graphic.h" 
#include "log_media_recorder.h" 
#include "fwl_font.h"
#include "Fwl_disposd.h"

/**
 *
 */
 
//-------------------------Public  Configure---------------------------------
/**
 * single file size limit(less than 4G bytes, if greater than this val, recorder will auto record next)
 */
#define REC_FILE_SIZE_MAX                 ((4*1024-10)*1024*1024ul)

/**
 * recorder reverse size(this space will not be used)
 */
#define REC_VIDEO_RES_SIZE                (32ul<<20)

/**
 * recorder video frame count min limit (if less than this val, recoreder will auto delete file) 
 */
#define REC_FRAME_CNT_MIN                  (3)

//-------------------------Cycle recorder Configure---------------------------
/**
 * cycle record file  time limit(millsecond)
 */
#define REC_CYC_INTERVAL                    (2*60*1000ul) 

/**
 *when cycle record file, if there is not neough space file, use this val decide how to delete file 
 *1:delete all file first, then check free space, 
 *0:check free space first, then delete file until there has enough storage
 */
#define CHK_REC_SPACE_BY_DELALL       (0)  


//------------------------- Normal  recorder Configure------------------------
/**
 * record max time limit (second);
 * if this macro is equal  zero, that meaning not time limit
 */
#define REC_LIMIT_TIME                (240*60ul)

//----------------------MotionDetect   Configure -----------------------------
/**
 * motion detect interval time(millsecond)
 */
#define REC_MDETECT_INTERVAL              (1000ul)


/**
 *the  time limit  for  Detecting the video is not Motion moving(millsecond)
 */
#define REC_MDETECT_NO_MOVING_LIMIT       (1000 * 60)  

/**
 *define the  encode interval  when the video is not any Motion moving(millsecond)
 * i.e.  how time be spent encode one frame, here is means 1 second
 * remark: this config is only be used  when the motion detect no moving
 */
#define REC_MDETECT_ENCODE_INTERVAL       (1000) 


//------------------------- recorder ui display Configure -----------------------
 /**
 *enable preload recorder UI resource 
 */
#define REC_ICON_PRE_LOAD

/**
 *tool bar refresh time(millsecond) 
 */
#define CAMERA_TOOLBAR_SHOWN_TIME  (2*1000) //ms 

/**
 *  pop menu show mode:No Show
 */
#define REC_NO_MENU             (0)

/**
 *  pop menu show mode:show menu with video backgrand
 */
#define REC_MENU_WITH_DV        (1)

/**
 * pop menu show mode:show menu with black backgrand
 */
#define REC_MENU_WITHOUT_DV     (2) 

/**
 * current pop menu show mode
 */
#define REC_MENU_SHOW_MODE      REC_MENU_WITHOUT_DV//REC_MENU_WITHOUT_DV


//------------------------Menu UI Configure -------------------------------

/**
 * lcd  width(default UI max width)
 */
#define CAMERA_RECORD_LCD_WIDTH         (Fwl_GetLcdWidth())

/**
 * lcd  width(default UI max height)
 */
#define CAMERA_RECORD_LCD_HEIGHT        (Fwl_GetLcdHeight())


/**
 * toobar  paint  right  position  offset
 */
#define RECORD_TOOLBAR_OFFSET_X    5

/**
 * toobar  paint  top  position  offset
 */
#define RECORD_TOOLBAR_V_INTERVAL  10

/**
 * OSD ZoomIn Times
 * (must even && OSD_ZOOMIN_720P>=OSD_ZOOMIN_X)
 */
#define OSD_ZOOMIN_720P             4
#define OSD_ZOOMIN_X                2
/**
 *OSD change time(millsecond) 
 */
#define OSD_CHANGE_TIME             (500) //ms 

#define STAMP_FILL_WIDTH			150
#define STAMP_FILL_HEIGHT			30

#define STAMP_FILL_WIDTH_TVOUT		300
#define STAMP_FILL_HEIGHT_TVOUT		30

#define REC_IMG_LEFT				20
#define REC_IMG_TOP					10

#define REC_IMG_LEFT_TVOUT			40
#define REC_IMG_TOP_TVOUT			10

#define REC_TIME_STR_LEFT			60
#define REC_TIME_STR_TOP			10

#define REC_TIME_STR_LEFT_TVOUT		120
#define REC_TIME_STR_TOP_TVOUT		10

#define BG_FILL_COLOR				0x000f0f0f

static T_BOOL VideoRecord_RecSave(T_VIDEO_RECORD *pVideoRec,T_BOOL isShowUI );
static T_VOID VideoRecord_RecCancel(T_VIDEO_RECORD *pVideoRec, T_eCSTM_STR_ID strID);
static T_BOOL VideoRecord_PreClose(T_VIDEO_RECORD *pVideoRec);
static T_VOID VideoRecord_Stop(T_VIDEO_RECORD* pVideoRec, T_BOOL isSave,T_eCSTM_STR_ID strID);
static T_BOOL VideoRecord_RecFree(T_VIDEO_RECORD *pVideoRec);

static T_eBACK_STATE VideoRecord_UserKey_Handle(T_MMI_KEYPAD phyKey, T_EVT_PARAM * pEventParm);
static T_VOID VideoRecord_TSCR_Handle(T_MMI_KEYPAD *phyKey, T_EVT_PARAM * pEventParm);
static T_VOID VideoRecord_MapTSCR_To_Key(T_MMI_KEYPAD *key, T_POS x, T_POS y);


static T_BOOL VideoRecord_PaintDataLoad(T_VIDEO_REC_UI *pPaintData, T_BOOL isCanKill);
static T_VOID VideoRecord_DispInfoMsgbox(T_VIDEO_RECORD *pVideoRec, T_pCWSTR title, T_pCWSTR content, T_U16 retLevel);
static T_BOOL VideoRecord_GetToolBarShowFlag(T_VOID);
static T_VOID VideoRecord_ShowToolBar(T_BOOL shownFlag, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect);

static T_S32  VideoRecord_PaintIcon_Cbf(T_FRM_DATA *pDispFrame);
static T_BOOL VideoRecord_IsNeedPaintMenu(T_VOID);
static T_S32  VideoRecord_PaintMenu_Cbf(T_VOID);
static T_U16  VideoRecord_PaintIcon(T_VIDEO_RECORD *pVideoRec, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect);
//static T_VOID VideoRecord_ShowStamp(T_VIDEO_RECORD *pVideoRec, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect);
static T_VOID VideoRecord_StampTime(T_U16 nZoom, T_VIDEO_RECORD *pVideoRec, T_eFONT setFont);
static T_BOOL OSD_ZoomIn(T_U16 nZoom, J_OSD_Info *osdinfo_zoom, J_OSD_Info *osdinfo);

static T_BOOL VideoRecord_GetRecConfig(T_VIDEO_RECORD *pVideoRec,T_REC_CTRL_INIT_PARAM *recCfg);
static T_BOOL VideoRecord_StatusCheck_Cbf(T_MENC_INFO recInfo);

extern E_LCD_TYPE VideoDisp_LCD_Type(T_VOID);
extern T_VIDEO_RECORD *pVideoRec;
static T_hSemaphore osd_semaphore = 0;


/*
 * @brief   callback function for get recorder status
 * @author WangXi
 * @date	2011-10-25
 * @param[in] encInfo: recorder information
 * @param[in] encStatus: recorder status
 * @return	AK_TRUE: get Ok, else fail
 */
static T_BOOL VideoRecord_StatusCheck_Cbf(T_MENC_INFO recInfo)
{
    if (AK_NULL != pVideoRec)
    {
        pVideoRec->recTime    = recInfo.mediaInfo.total_time_ms/1000;
        pVideoRec->recMaxTime = recInfo.recMaxTimeSec;
        return AK_TRUE;
    }
    return AK_FALSE;
}

/**
 * @brief The function processes Video record control init
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 * @return returns the init result
 * @retval AK_TRUE init successfully
 * @retval AK_FALSE init fail
 * @retval AK_FALSE point is NULL
 */
T_BOOL VideoRecord_Init(T_VIDEO_RECORD *pVideoRec)
{
    AK_ASSERT_PTR(pVideoRec, "VideoRecord_Init():pVideoRec should not be NULL\n", AK_FALSE);
	
    /**init variable*/
    pVideoRec->refreshFlag = CAMERA_RECORD_REFRESH_NONE;

	pVideoRec->hZoom = 0;
	pVideoRec->hMRec = 0;
	pVideoRec->hDisp = 0;
	
    /**mapping file path*/
    Utl_UStrCpy(pVideoRec->curPath, (T_pCWSTR)Fwl_GetDefPath(eVIDEOREC_PATH));

    pVideoRec->recTime = 0;
	pVideoRec->videoSize = 0;
    
	pVideoRec->isExitWarning				= AK_FALSE;

    //Init screen operation
    pVideoRec->toolBarShowFlag              = AK_FALSE;
    pVideoRec->toolBarTimerID               = ERROR_TIMER;

	//Init OSD	
	pVideoRec->pJ_OSD_Info.osdWidth = 16;
	pVideoRec->pJ_OSD_Info.osdHeight = 16;
	pVideoRec->pJ_OSD_Info.osdRGB565 = AK_NULL;
	pVideoRec->pJ_OSD_Info.osd_H_offset = 16;
	pVideoRec->pJ_OSD_Info.osd_V_offset = 16;
	pVideoRec->pJ_OSD_Info.alpha = 16;

    pVideoRec->pJ_OSD_Info_Zoom.osdWidth = 64;
	pVideoRec->pJ_OSD_Info_Zoom.osdHeight = 64;
	pVideoRec->pJ_OSD_Info_Zoom.osdRGB565 = AK_NULL;
	pVideoRec->pJ_OSD_Info_Zoom.osd_H_offset = 16;
	pVideoRec->pJ_OSD_Info_Zoom.osd_V_offset = 16;
	pVideoRec->pJ_OSD_Info_Zoom.alpha = 16;

    pVideoRec->OSDTimerID = ERROR_TIMER;
    pVideoRec->OSDTimerID = Fwl_SetTimerMilliSecond(OSD_CHANGE_TIME, AK_TRUE);

    return AK_TRUE;
}



/*
 * @brief   init video souce process task
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: init success, else fail
 */
T_BOOL VideoRecord_SrcInit(T_VIDEO_RECORD *pVideoRec)
{
	AK_ASSERT_PTR(pVideoRec, "VideoRecord_SrcInit():pVideoRec should not be NULL\n", AK_FALSE);

    // init display task 
    pVideoRec->hDisp = VideoDisp_Open();
    if (0 == pVideoRec->hDisp) 
    {
        Fwl_Print(C3, M_CAMERA,  "VideoDisp_Open Er\n");
        return AK_FALSE;
    }

    // set callback for show ui
    VideoDisp_SetPaintUiCbf(pVideoRec->hDisp,
        VideoRecord_PaintIcon_Cbf,
        VideoRecord_IsNeedPaintMenu,
        VideoRecord_PaintMenu_Cbf);


    // start zoom process
    if (AK_IS_FAILURE(VideoZoom_Start(pVideoRec->hZoom)))
    {
        Fwl_Print(C3, M_CAMERA,  "VideoZoom_Start Er\n");
        return AK_FALSE;
    }

    // set detect interval time
    VideoZoom_DetectSetInterval(pVideoRec->hZoom,REC_MDETECT_INTERVAL);

    //start display process by current display mode 
    if (AK_IS_FAILURE(VideoDisp_Start(pVideoRec->hDisp, 
        pVideoRec->hZoom, 
        Fwl_GetDispalyType())))
    {
        Fwl_Print(C3, M_CAMERA,  "VideoDisp_Start Er\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}

/*
 * @brief   deinit video souce process task
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: deinit success, else fail
 */
T_BOOL VideoRecord_SrcDeInit(T_VIDEO_RECORD *pVideoRec)
{
	AK_ASSERT_PTR(pVideoRec, "VideoRecord_SrcDeInit():pVideoRec should not be NULL\n", AK_FALSE);

#ifdef OS_ANYKA
    if ((T_HANDLE)0 != pVideoRec->hDisp)
    {
        // close display process 
        VideoDisp_Close(pVideoRec->hDisp);
        pVideoRec->hDisp = (T_HANDLE)0;
    }

    if ((T_HANDLE)0 != pVideoRec->hZoom)
    {
        // pause display process 
        VideoZoom_Pause(pVideoRec->hZoom);
        pVideoRec->hZoom = (T_HANDLE)0;
    }

#endif

    return AK_TRUE;
}


/*
 * @brief   init paint resource and parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: init success, else fail
 */
T_BOOL VideoRecord_PaintInit(T_VIDEO_RECORD *pVideoRec, T_VIDEO_REC_UI *pPaintData)
{
	T_U32 wi,hi;

	AK_ASSERT_PTR(pVideoRec, "VideoRecord_PaintInit():pVideoRec should not be NULL\n", AK_FALSE);
	AK_ASSERT_PTR(pPaintData, "VideoRecord_PaintInit():pPaintData should not be NULL\n", AK_FALSE);

	if (0 == pVideoRec->hZoom) 
	{
		Fwl_Print(C3, M_CAMERA,  "PaintInit:VideoZoom_Open Er\n");
		return AK_FALSE;
	}

    pVideoRec->pPaintParm = (PAINTINFO*)Fwl_Malloc(sizeof(PAINTINFO));
    AK_ASSERT_PTR(pVideoRec->pPaintParm, "VideoRecord_PaintInit(): pVideoRec->pPaintParm malloc fail\n", AK_FALSE);


#ifdef REC_ICON_PRE_LOAD	
    // get paint resource from  preload resource
	memcpy(&pVideoRec->paintData, pPaintData, sizeof(T_VIDEO_REC_UI));
#else
    // load paint resource
    if (!VideoRecord_PaintDataLoad(&pVideoRec->paintData, AK_TRUE))
    {
        AkDebugOutput("VideoRecord_PaintDataLoad Err\n");
        return AK_FALSE;
    }
#endif

    // get video size by record file type
	Fwl_GetRecFrameSize(gs.camRecMode[gs.CamRecFileType], &wi, &hi);

    //init parameter of paint
    if ((wi<Fwl_GetLcdWidth()) && (hi<Fwl_GetLcdHeight()))
    {
        pVideoRec->pPaintParm->width  = (T_LEN)wi;
        pVideoRec->pPaintParm->height = (T_LEN)hi;
        pVideoRec->pPaintParm->left   = (T_POS)(Fwl_GetLcdWidth() - pVideoRec->pPaintParm->width) >> 1;
        pVideoRec->pPaintParm->top    = (T_POS)(Fwl_GetLcdHeight() - pVideoRec->pPaintParm->height) >> 1;

		if (pVideoRec->pPaintParm->left < 0)
			pVideoRec->pPaintParm->left = 0;

		if (pVideoRec->pPaintParm->top < 0)
			pVideoRec->pPaintParm->top = 0;
    }
	
	else //  CAMERA_MODE_VGA / CAMERA_MODE_CIF / ... ...
    {
        pVideoRec->pPaintParm->width  = Fwl_GetLcdWidth();
        pVideoRec->pPaintParm->height = Fwl_GetLcdHeight();
        pVideoRec->pPaintParm->left   = 0;
        pVideoRec->pPaintParm->top    = 0;
    }

    pVideoRec->pPaintParm->rotate  = Fwl_GetLcdDegree();
    pVideoRec->pPaintParm->bCamera = AK_TRUE;

#ifdef OS_ANYKA
	Fwl_LcdRotate(pVideoRec->pPaintParm->rotate);
#endif

    return AK_TRUE;
}

/*
 * @brief   load paint resource
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pPaintData: paint data handle
 * @param[in] isCanKill:  resouce load is kill
 * @return	AK_TRUE: load success, else fail
 */
static T_BOOL VideoRecord_PaintDataLoad(T_VIDEO_REC_UI *pPaintData, T_BOOL isCanKill)
{
    T_U32   len;
    //T_LEN   NumWidth;
    T_U8    i;
	T_U32   totalHeight;

	AK_ASSERT_PTR(pPaintData, "VideoRecord_PaintDataLoad():pPaintData should not be NULL\n", AK_FALSE);

   //recording mark icon init
   pPaintData->recordingIcon.redicon = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_VIDEOREC_STATUS_RECORDING, AK_NULL);
   AKBmpGetInfo(pPaintData->recordingIcon.redicon, &pPaintData->recordingIcon.iconWidth, &pPaintData->recordingIcon.iconHeight, AK_NULL);
   
   // toolbar resource init
   pPaintData->toolBarIconData[CAMERA_RECORD_ICON_SAVE] = 
		   Res_GetBinResByID(AK_NULL, isCanKill, eRES_BMP_CAMERA_RECORDER_SAVE, &len);
   
   AKBmpGetInfo(pPaintData->toolBarIconData[CAMERA_RECORD_ICON_SAVE], &pPaintData->toolBarIconRect[CAMERA_RECORD_ICON_SAVE].width, &pPaintData->toolBarIconRect[CAMERA_RECORD_ICON_SAVE].height, AK_NULL);
   
   pPaintData->toolBarIconData[CAMERA_RECORD_ICON_CANCEL] = 
		   Res_GetBinResByID(AK_NULL, isCanKill, eRES_BMP_CAMERA_RECORDER_CANCEL, &len);
   
   AKBmpGetInfo(pPaintData->toolBarIconData[CAMERA_RECORD_ICON_CANCEL], &pPaintData->toolBarIconRect[CAMERA_RECORD_ICON_CANCEL].width, &pPaintData->toolBarIconRect[CAMERA_RECORD_ICON_CANCEL].height, AK_NULL);
   
   for (i=0; i<CAMERA_RECORD_ICON_NUM; ++i)
   {
	   pPaintData->toolBarIconRect[i].left = CAMERA_RECORD_LCD_WIDTH - pPaintData->toolBarIconRect[i].width - RECORD_TOOLBAR_OFFSET_X;
   
	   totalHeight = pPaintData->toolBarIconRect[i].height;
   }
   
   totalHeight += (CAMERA_RECORD_ICON_NUM - 1) * RECORD_TOOLBAR_V_INTERVAL;
   
   pPaintData->toolBarIconRect[CAMERA_RECORD_ICON_SAVE].top = (T_POS)((CAMERA_RECORD_LCD_HEIGHT - totalHeight)/2);
   
   for (i=CAMERA_RECORD_ICON_SAVE+1; i<CAMERA_RECORD_ICON_NUM; ++i)
   {
	   pPaintData->toolBarIconRect[i].top = pPaintData->toolBarIconRect[i-1].top + pPaintData->toolBarIconRect[i-1].height + RECORD_TOOLBAR_V_INTERVAL;
   }

   return AK_TRUE;
}


/*
 * @brief   preload paint resource
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pPaintData: paint data handle
 * @return	AK_TRUE: load success, else fail
 */
T_BOOL VideoRecord_IconPreLoad(T_VIDEO_REC_UI *pPaint)
{
#ifdef REC_ICON_PRE_LOAD	
	return VideoRecord_PaintDataLoad(pPaint, AK_FALSE);
#else
    return AK_TRUE;
#endif
}

/*
 * @brief   load paint resource
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] dstBuf: paint data handle
 * @param[in] bufW: destbuf  width
 * @param[in] bufH: destbuf  height
 * @param[in] rect:  destbuf  valid zone
 */
/*static T_VOID VideoRecord_ShowStamp(T_VIDEO_RECORD *pVideoRec, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect)
{
	T_U8		strbuf[30] = {0};
	T_U16 		Ustrbuf[30] = {0};
	T_U32 		timeWidth;
	T_SYSTIME	systime;
    T_U16       len = 0;
	
	systime = GetSysTime();
	sprintf(strbuf,"%04d/%02d/%02d %02d:%02d:%02d",
		systime.year,systime.month,systime.day,
		systime.hour,systime.minute,systime.second);
    
	Eng_StrMbcs2Ucs(strbuf, Ustrbuf);
	len = (T_U16)Utl_UStrLen(Ustrbuf); 
	timeWidth = UGetSpeciStringWidth(Ustrbuf, (T_FONT)FONT_16, (T_U16)len);
	Fwl_UDispSpeciStringOnRGB(dstBuf, bufW, bufH, (T_POS)(rect.left + rect.width - timeWidth - 4),
						rect.top, Ustrbuf, (T_COLOR)COLOR_RED, RGB565, (T_FONT)FONT_16, (T_U16)len);		
}*/

/**
 * @brief The function paint
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec      the point of video record control
 */
static T_U16 VideoRecord_PaintIcon(T_VIDEO_RECORD *pVideoRec, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect)
{
	T_U16 flag;
    T_U16 leftPos, topPos;
	T_RECT		fillrect = {0};
	T_RECT		imgrect = {0};
    T_RECT		osddisprect = {0};
    T_U32 		recTime;
    T_U32 		recSec = 0;
    T_U32 		recMin = 0;
    T_U32 		recHour = 0;
    T_U8  		recTimeStr[8];
	T_U8  		recordtime[10] = {0};
	T_LEN		fillwidth = 0;
	T_LEN		fillheight = 0;
	T_POS		imgleft = 0;
	T_POS		imgtop = 0;
	T_POS		timeleft = 0;
	T_POS		timetop = 0;
	
    flag = pVideoRec->refreshFlag;
	leftPos = 4;//左偏移量
	topPos = 4;//上偏移量
    
	//show lcd stamp
	//VideoRecord_ShowStamp(pVideoRec, dstBuf, bufW, bufH, rect);    
    
    //show focus level string
	if (VideoZoom_IsEnableFocusWin(pVideoRec->hZoom))
	{
        T_U8 focusTips[16] = {0};
        T_U16 len = 0;
        
        Fwl_CameraGetFocusTips((T_U32)gb.nZoomInMultiple, focusTips);
        len = (T_U16)strlen(focusTips);
        if (len > 0)
        {
            Fwl_DispStringOnRGB(dstBuf, bufW, bufH, (T_POS)(rect.left+leftPos), (T_POS)(rect.top+g_Font.SCHEIGHT+topPos), 
                                focusTips, (T_U16)len, COLOR_RED, RGB565, (T_FONT)0);
        }
	}

/*	暂时去除录像时的图标和时间戳
    // show recording mark icon
	Fwl_AkBmpDrawFromStringOnRGB(dstBuf, bufW, bufH,
				(T_POS)(rect.left+leftPos), (T_POS)(rect.top+topPos), pVideoRec->paintData.recordingIcon.redicon, &g_Graph.TransColor, AK_FALSE, (T_U8)RGB565);

	
	//show record time
	VideoRecord_RecTimeShow(pVideoRec, dstBuf, bufW, bufH, rect, FONT_16);
*/

	//show two icon
	if (flag & CAMERA_RECORD_UNSHOW_TOOLBAR)//open/close toolbar
    {        
        pVideoRec->refreshFlag = CAMERA_RECORD_REFRESH_NONE;
		VideoRecord_ShowToolBar(AK_FALSE, dstBuf, bufW, bufH, rect);
    }
    else if (flag & CAMERA_RECORD_SHOW_TOOLBAR)
    {
    	if (E_LCD_TYPE_TVOUT != VideoDisp_LCD_Type())
        {
        	VideoRecord_ShowToolBar(AK_TRUE, dstBuf, bufW, bufH, rect);
    	}
    }    

	//录像时的图标和时间戳
	Fwl_Osd_ClearDispBuf();  

	recTime = pVideoRec->recTime;	
    recSec = (recTime >= 60) ? (recTime%60) : recTime;
    recMin = (recTime >= 60) ? ((recTime >= 3600) ? ((recTime%3600)/60) : (recTime/60)) : 0;
    recHour = (recTime>=3600) ? (recTime/3600) : 0;
    recHour = (recHour>=100) ? 0 : recHour;

    recTimeStr[0] = (T_U8)((recHour>=10) ? (recHour/10) : 0);
    recTimeStr[1] = (T_U8)(recHour - (recTimeStr[0]*10));
    recTimeStr[2] = (T_U8)((recMin>=10) ? (recMin/10) : 0);
    recTimeStr[3] = (T_U8)(recMin - (recTimeStr[2]*10));    
    recTimeStr[4] = (T_U8)((recSec>=10) ? (recSec/10) : 0);
    recTimeStr[5] = (T_U8)(recSec - (recTimeStr[4]*10));
	
	sprintf(recordtime, "%d%d:%d%d:%d%d", recTimeStr[0], recTimeStr[1], recTimeStr[2], recTimeStr[3], recTimeStr[4], recTimeStr[5]);


	if (DISPLAY_LCD_1 < Fwl_GetDispalyType())
	{
		//tvout下，由于画面里的时间字串跟随画面放大了，
		//	因此要遮盖区域大点才能遮盖住
		fillwidth	= STAMP_FILL_WIDTH_TVOUT;
		fillheight	= STAMP_FILL_HEIGHT_TVOUT;
		imgleft		= REC_IMG_LEFT_TVOUT;
		imgtop		= REC_IMG_TOP_TVOUT;
		timeleft	= REC_TIME_STR_LEFT_TVOUT;
		timetop		= REC_TIME_STR_TOP_TVOUT;
	}
	else
	{
		fillwidth	= STAMP_FILL_WIDTH;
		fillheight	= STAMP_FILL_HEIGHT;
		imgleft		= REC_IMG_LEFT;
		imgtop		= REC_IMG_TOP;
		timeleft	= REC_TIME_STR_LEFT;
		timetop		= REC_TIME_STR_TOP;
	}

	RectInit(&fillrect, 0, 0, fillwidth, fillheight);
	Fwl_Osd_FillSolidRectByGray(&fillrect, COLOR_BLACK);

	RectInit(&imgrect, imgleft, imgtop, pVideoRec->paintData.recordingIcon.iconWidth, pVideoRec->paintData.recordingIcon.iconHeight);
	Fwl_Osd_DrawStreamBmpByGray(&imgrect, pVideoRec->paintData.recordingIcon.redicon);
		
	Fwl_Osd_DrawStringByGray(timeleft, timetop, 
					recordtime, (T_U16)strlen(recordtime), COLOR_WHITE, CURRENT_FONT_SIZE);

	RectInit(&osddisprect, 0, 0, MAIN_LCD_WIDTH, fillheight);
	Fwl_Osd_RefreshDisplayRect(&osddisprect);
	
	return 0;
}

/**
 * @brief The function processes Video record control free
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 */
T_VOID VideoRecord_Free(T_VIDEO_RECORD *pVideoRec)
{
    //T_CAMERA_RECORD_TOOLBAR_ICON i;

    AK_ASSERT_PTR_VOID(pVideoRec, "VideoRecord_Free():pVideoRec should not be NULL\n");
	
	if (ERROR_TIMER != pVideoRec->toolBarTimerID)
	{
		Fwl_StopTimer(pVideoRec->toolBarTimerID);
		pVideoRec->toolBarTimerID = ERROR_TIMER;
	}

    if (ERROR_TIMER != pVideoRec->OSDTimerID)
	{
		Fwl_StopTimer(pVideoRec->OSDTimerID);
		pVideoRec->OSDTimerID = ERROR_TIMER;
	}
	
#ifdef OS_ANYKA
    if ((T_HANDLE)0 != pVideoRec->hMRec)
    {
        MRec_Close(pVideoRec->hMRec, AK_FALSE);
        pVideoRec->hMRec = (T_HANDLE)0;
    }
#endif
    // Deint video Zoom and display
    VideoRecord_SrcDeInit(pVideoRec);

    /**free memory of mem and YUV, and disable camera interrupt*/
    VideoRecord_RecFree(pVideoRec);

    /**close flash light*/
#ifdef CAMERA_FLASHLIGHT
    if (gs.VideoRecFlashlight)
    {
        Fwl_CameraFlashOff(); 
		Fwl_CameraFlashClose();
    }
#endif
}

/**
 * @author: Bennis Zhang
 */
static T_VOID showExitWarning(T_VIDEO_RECORD * pVideoRec)
{
#if (REC_NO_MENU != REC_MENU_SHOW_MODE)
    MsgBox_InitStr(&pVideoRec->msgbox, 0, GetCustomString(csFM_DEL_ALL_NOTE), Res_GetStringByID(eRES_STR_IS_SAVE_VIDEO), MSGBOX_QUESTION | MSGBOX_YESNO);
#if (REC_MENU_WITH_DV == REC_MENU_SHOW_MODE)
	VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_SHOW_MSGBOX);
#elif (REC_MENU_WITHOUT_DV == REC_MENU_SHOW_MODE)
    Fwl_FillSolid(HRGB_LAYER, BG_FILL_COLOR);    
    MsgBox_Show(&pVideoRec->msgbox);
    Fwl_RefreshDisplay();
#endif
#endif
}


/**
 * @brief deal with warnning 
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] Event
 * @param[in] pEventParm
 * @return T_eBACK_STATE 
 */
T_eBACK_STATE VideoRec_UserEventHandle(T_MMI_KEYPAD phyKey, T_EVT_PARAM *pEventParm)
{
#if (REC_NO_MENU != REC_MENU_SHOW_MODE)
	if ((phyKey.keyID == kbCLEAR) && (!pVideoRec->isExitWarning))
	{			 
		VideoRecord_PreClose(pVideoRec);
		showExitWarning(pVideoRec);
		pVideoRec->isExitWarning = AK_TRUE;
		return eStay;
	}
#endif
    return VideoRecord_UserKey_Handle(phyKey, pEventParm);
}

/**
 * @brief deal with warnning 
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] Event
 * @param[in] pEventParm
 * @return T_eBACK_STATE 
 */
T_eBACK_STATE VideoRec_ExitWarnHandle(T_EVT_PARAM *pEventParm, T_EVT_CODE Event)
{
	T_eBACK_STATE reState = eStay;
	T_MMI_KEYPAD    phyKey;
	
	reState = MsgBox_Handler(&pVideoRec->msgbox, Event, pEventParm);
#if (REC_MENU_WITH_DV == REC_MENU_SHOW_MODE)
	VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_SHOW_MSGBOX);
#elif (REC_MENU_WITHOUT_DV == REC_MENU_SHOW_MODE)
    MsgBox_Show(&pVideoRec->msgbox);
    Fwl_RefreshDisplay();
#endif

	if (reState == eNext)
	{
        //pVideoRec->isExitWarning = AK_FALSE;
        VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_UNSHOW_MSGBOX);
		if (pVideoRec->recTime)
		{
			phyKey.keyID = kbOK;
			phyKey.pressType = PRESS_SHORT;
		}
		else
		{
			phyKey.keyID = kbCLEAR;
			phyKey.pressType = PRESS_SHORT;
		}
	}
	else if (reState == eReturn)
	{
		phyKey.keyID = kbCLEAR;
		phyKey.pressType = PRESS_SHORT;
        //pVideoRec->isExitWarning = AK_FALSE;
        VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_UNSHOW_MSGBOX);
	}
	else
	{
		return eStay;
	}

	return VideoRecord_UserKey_Handle(phyKey, pEventParm);
}



/**
 * @brief The function processes event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 * @param[in] Event            event code
 * @param[in] pEventParm        the parameter of event
 */
T_eBACK_STATE VideoRecord_Handle(T_VIDEO_RECORD* pVideoRec, T_EVT_CODE Event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE reState = eStay;
    T_MMI_KEYPAD    phyKey;
	T_REC_ERROR_STATUS ret;	
	
    AK_ASSERT_PTR(pVideoRec, "VideoRecord_Handle():pVideoRec should not be NULL\n", eStay);
    AK_ASSERT_PTR(pEventParm, "VideoRecord_Handle():pEventParm should not be NULL\n", eStay);

#if (REC_NO_MENU != REC_MENU_SHOW_MODE)
	if (pVideoRec->isExitWarning)
	{
		return VideoRec_ExitWarnHandle(pEventParm, Event);
	}
#endif

    switch (Event)
    {
    case M_EVT_TOUCH_SCREEN:
        VideoRecord_TSCR_Handle(&phyKey, pEventParm);

		return VideoRec_UserEventHandle(phyKey, pEventParm);
        break;
		
    case M_EVT_USER_KEY:
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;
        
		return VideoRec_UserEventHandle(phyKey, pEventParm);
        break;
		
	case USER_EVT_STOP_REC:
		ret = (T_REC_ERROR_STATUS)pEventParm->w.Param1;
		VideoRecord_Close(pVideoRec,(T_BOOL)(REC_ERROR_ENCODE_EXP != ret), csSTAND_BY);
		reState = eReturn;
		break;
		
    case VME_EVT_TIMER:
		if ((pEventParm->w.Param1 == (T_U32)pVideoRec->toolBarTimerID)\
		     && (pEventParm->w.Param2 == CAMERA_TOOLBAR_SHOWN_TIME))
		 {
			 pVideoRec->refreshFlag |= CAMERA_RECORD_UNSHOW_TOOLBAR;
			 if (pVideoRec->toolBarTimerID != ERROR_TIMER)
			 {		  
				 Fwl_StopTimer(pVideoRec->toolBarTimerID);
				 pVideoRec->toolBarTimerID = ERROR_TIMER;
			 }
		 }

        if ((pEventParm->w.Param1 == (T_U32)pVideoRec->OSDTimerID)\
		     && (pEventParm->w.Param2 == OSD_CHANGE_TIME))
		 {			 
			 if (pVideoRec->OSDTimerID != ERROR_TIMER)
			 {		  
                if (1280 == pVideoRec->width && 720 == pVideoRec->height)
                {
                    VideoRecord_StampTime(OSD_ZOOMIN_720P, pVideoRec, FONT_16);
                }
                else
                {
                    VideoRecord_StampTime(OSD_ZOOMIN_X, pVideoRec, FONT_16);
                }
			 }
		 }	
        break;
        
    default:
        break;
    }

    return reState;
}


static T_BOOL OSD_ZoomIn(T_U16 nZoom, J_OSD_Info *osdinfo_zoom, J_OSD_Info *osdinfo)
{
	T_U32 width;
	T_U32 height;
	T_U32 i, j;
	
	width = osdinfo_zoom->osdWidth;
	height = osdinfo_zoom->osdHeight;
    
	for (j=0; j<height; j++)
	{
		for (i=0; i<width; i++)
		{
			osdinfo_zoom->osdRGB565[j*width+i]= osdinfo->osdRGB565[(j/nZoom)*(width/nZoom)+(i/nZoom)];
		}
	}
	
	return AK_TRUE;	
}

static T_VOID VideoRecord_StampTime(T_U16 nZoom, T_VIDEO_RECORD *pVideoRec, T_eFONT setFont)
{   
	T_U8		strbuf[30] = {0};
	T_U16 		Ustrbuf[30] = {0};
	T_U32 		timeWidth;
	T_SYSTIME	systime;
    T_U16       len = 0;
	T_eFONT     fontType = FONT_16;	
	T_U16 		fontHeight;  
    T_U32       size;
    T_U32       size_zoom;    
    
    if (setFont < FONTLIB_NUM)
    {
        fontType = setFont;
    }	
	
	systime = GetSysTime();
	sprintf(strbuf,"%04d/%02d/%02d %02d:%02d:%02d",
		systime.year,systime.month,systime.day,
		systime.hour,systime.minute,systime.second);
		
	Eng_StrMbcs2Ucs(strbuf, Ustrbuf);
    len = (T_U16)Utl_UStrLen(Ustrbuf); 
    
    if (0 != Utl_UStrCmp(pVideoRec->strOSDBuf, Ustrbuf))
    {        
        Eng_StrMbcs2Ucs(strbuf, pVideoRec->strOSDBuf);
        
        if (AK_NULL == pVideoRec->pJ_OSD_Info.osdRGB565)    
    	{    	    
    	    timeWidth = UGetSpeciStringWidth(Ustrbuf, (T_FONT)fontType, (T_U16)len);
    	    fontHeight = GetFontHeight((T_U8)fontType);

        	while(0 != (timeWidth%16))
        	{
        		timeWidth += 1;
        	}

        	while(0 != (fontHeight%16))
        	{
        		fontHeight += 1;
        	}

            pVideoRec->pJ_OSD_Info.osdWidth = (T_U16)timeWidth;
    	    pVideoRec->pJ_OSD_Info.osdHeight = (T_U16)fontHeight;    
            size = (pVideoRec->pJ_OSD_Info.osdWidth * pVideoRec->pJ_OSD_Info.osdHeight)*2;
            pVideoRec->pJ_OSD_Info.osdRGB565 = Fwl_Malloc(size);

            if (AK_NULL == pVideoRec->pJ_OSD_Info.osdRGB565)
            {
                Fwl_Print(C3, M_CAMERA,  "pJ_OSD_Info.osdRGB565 malloc fail\n");
                return;                
            }

            memset(pVideoRec->pJ_OSD_Info.osdRGB565, 0, size);
    	}
            
        if (AK_NULL == pVideoRec->pJ_OSD_Info_Zoom.osdRGB565)
    	{
    	    //the buffer must support the 720P
    	    size_zoom = ((pVideoRec->pJ_OSD_Info.osdWidth*OSD_ZOOMIN_720P)*(pVideoRec->pJ_OSD_Info.osdHeight*OSD_ZOOMIN_720P))*2;
    		pVideoRec->pJ_OSD_Info_Zoom.osdRGB565 = Fwl_Malloc(size_zoom);
            
            if (AK_NULL == pVideoRec->pJ_OSD_Info_Zoom.osdRGB565)
            {
                Fwl_Print(C3, M_CAMERA,  "pJ_OSD_Info_Zoom.osdRGB565 malloc fail\n");
                return;                
            }

            memset(pVideoRec->pJ_OSD_Info_Zoom.osdRGB565, 0, size_zoom);
    	}
        
        if ((AK_NULL != pVideoRec->pJ_OSD_Info.osdRGB565) && (AK_NULL != pVideoRec->pJ_OSD_Info_Zoom.osdRGB565))
     	{
     	    OSD_Obtain_Semaphore();
            
     	    size = (pVideoRec->pJ_OSD_Info.osdWidth * pVideoRec->pJ_OSD_Info.osdHeight)*2;
            size_zoom = ((pVideoRec->pJ_OSD_Info.osdWidth*OSD_ZOOMIN_720P)*(pVideoRec->pJ_OSD_Info.osdHeight*OSD_ZOOMIN_720P))*2;

            memset(pVideoRec->pJ_OSD_Info.osdRGB565, 0, size);
            memset(pVideoRec->pJ_OSD_Info_Zoom.osdRGB565, 0, size_zoom);
            
            Fwl_UDispSpeciStringOnRGB((T_U8 *)(pVideoRec->pJ_OSD_Info.osdRGB565), pVideoRec->pJ_OSD_Info.osdWidth, pVideoRec->pJ_OSD_Info.osdHeight,
    								  0, 0, Ustrbuf, (T_COLOR)COLOR_RED, RGB565, (T_FONT)fontType, (T_U16)len);

            pVideoRec->pJ_OSD_Info_Zoom.osdWidth = (T_U16)pVideoRec->pJ_OSD_Info.osdWidth*nZoom;
    	    pVideoRec->pJ_OSD_Info_Zoom.osdHeight = (T_U16)pVideoRec->pJ_OSD_Info.osdHeight*nZoom;
            
            OSD_ZoomIn(nZoom, &(pVideoRec->pJ_OSD_Info_Zoom), &(pVideoRec->pJ_OSD_Info));           

            OSD_Release_Semaphore();
     	}
    }   
}



/**
 * @brief The function display infomation message box
 *
 * @author \b zhengwenbo
 * @date 2006-07-11
 * @param[in] pVideoRec      the point of video record control
 * @param[in] title     the tile of message box
 * @param[in] content       the content of message box
 * @param[in] retLevel  the reture level of message box
 */
static T_VOID VideoRecord_DispInfoMsgbox(T_VIDEO_RECORD *pVideoRec, T_pCWSTR title, T_pCWSTR content, T_U16 retLevel)
{
#if (REC_NO_MENU != REC_MENU_SHOW_MODE)
    MsgBox_InitStr(&pVideoRec->msgbox, retLevel, title, content, MSGBOX_INFORMATION);
#if (REC_MENU_WITH_DV == REC_MENU_SHOW_MODE)
    VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_SHOW_MSGBOX);
    AK_Sleep(100);
    VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_UNSHOW_MSGBOX);
#elif (REC_MENU_WITHOUT_DV == REC_MENU_SHOW_MODE)
    Fwl_FillSolid(HRGB_LAYER, BG_FILL_COLOR);
    MsgBox_Show(&pVideoRec->msgbox);
    Fwl_RefreshDisplay();
#endif
#endif
}



/*
 * @brief   display error pop msgbox
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      video record control  handle
 * @param[in] tId:  title title id
 * @param[in] sId:  title string id
 * @param[in] retLevel:  return level
 * @param[in] useInnerDisp: if use  asyn display
 */
T_VOID VideoRecord_DispErrMsgbox(T_VIDEO_RECORD *pVideoRec, T_S16 tId, T_S16 sId, T_U16 retLevel, T_BOOL useInnerDisp)
{
#if (REC_NO_MENU != REC_MENU_SHOW_MODE)
    MsgBox_InitAfx(&pVideoRec->msgbox, retLevel, tId, sId, MSGBOX_INFORMATION);
#if (REC_MENU_WITH_DV == REC_MENU_SHOW_MODE)
    if (useInnerDisp && (0 != pVideoRec->hDisp))
    {
        VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_SHOW_MSGBOX);
        AK_Sleep(100);
        VideoRecord_SetRefresh(pVideoRec, CAMERA_RECORD_UNSHOW_MSGBOX);
    }
    else
#endif
    {
        Fwl_FillSolid(HRGB_LAYER, BG_FILL_COLOR);    
        MsgBox_Show(&pVideoRec->msgbox);
        Fwl_RefreshDisplay();
        AK_Sleep(100);
    }
#endif   
}


/**
 * @brief The function paint
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec      the point of video record control
 * @ret T_U16
 */
T_U16 VideoRecord_Paint(T_VIDEO_RECORD *pVideoRec)
{
    T_U16 flag;

    flag = pVideoRec->refreshFlag;
    //Fwl_Print(C3, M_CAMERA,  "DEBUG::**************the flag is %d", flag);

	if (CAMERA_RECORD_REFRESH_NONE == flag)
	{
		Fwl_Print(C3, M_CAMERA,  "REC REFRESH NONE\n");
		return CAMERA_RECORD_REFRESH_NONE;
	}

    if (flag & CAMERA_RECORD_REFRESH_BACK)
    {
        Fwl_FillSolid(HRGB_LAYER, BG_FILL_COLOR);
    }

    
    if (flag & CAMERA_RECORD_UNSHOW_MSGBOX)//open/close msgbox
    {
        pVideoRec->refreshFlag = CAMERA_RECORD_REFRESH_NONE;
    }
    else if (flag & CAMERA_RECORD_SHOW_MSGBOX)
    {
        MsgBox_SetRefresh(&pVideoRec->msgbox, CTL_REFRESH_ALL);
		MsgBox_Show(&pVideoRec->msgbox);
    }
    else if (flag & CAMERA_RECORD_UNSHOW_TOOLBAR)//open/close toolbar
    {        
        pVideoRec->refreshFlag = CAMERA_RECORD_REFRESH_NONE;		
    }
    
	return flag;
}


/*
 * @brief   callback function for check if asyn display pop menu
 * @author WangXi
 * @date	2011-10-25
 * @return	AK_TRUE: asyn show pop menu, else need not  show
 */
static T_BOOL VideoRecord_IsNeedPaintMenu(T_VOID)
{
	if (AK_NULL == pVideoRec)
	{
         return AK_FALSE;
	}
    
    return (CAMERA_RECORD_REFRESH_NONE != pVideoRec->refreshFlag);
}


/*
 * @brief   callback function for paint pop menu on 565 buffer
 * @author WangXi
 * @date	2011-10-25
 * @return	AK_SUCCESS: paint success, else fail
 */
static T_S32 VideoRecord_PaintMenu_Cbf(T_VOID)
{
	AK_ASSERT_PTR(pVideoRec, "pVideoRec Is Invalid", AK_EFAILED);

    VideoRecord_Paint(pVideoRec);
	return AK_SUCCESS;
}


/*
 * @brief   callback function for paint icon on video frame
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pDispFrame    
 * @return	AK_SUCCESS: paint icon success, else fail.
 */
static T_S32 VideoRecord_PaintIcon_Cbf(T_FRM_DATA *pDispFrame)
{
	if (AK_NULL == pVideoRec)
	{
		 Fwl_Print(C3, M_CAMERA,  "VideoRecord_paint need not frefesh @%d\n", __LINE__);
         return AK_EFAILED;
	}

	if (!pDispFrame || !pDispFrame->pBuffer)
	{
	    return AK_EFAILED;
	}
	VideoRecord_PaintIcon(pVideoRec, pDispFrame->pBuffer, (T_LEN)pDispFrame->info.width, (T_LEN)pDispFrame->info.height, pDispFrame->info.rect);

   return AK_SUCCESS;
}


/*
 * @brief  get recorder config from ui 
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[out] recCfg       the recorder config
 * @return	AK_TRUE:  get config success, else fail
 */
static T_BOOL VideoRecord_GetRecConfig(T_VIDEO_RECORD *pVideoRec,T_REC_CTRL_INIT_PARAM *recCfg)
{  
	if ((AK_NULL == recCfg) || (AK_NULL == pVideoRec))
	{
		Fwl_Print(C3, M_CAMERA,  "Param Er");
		return AK_FALSE;
	}
	Utl_UStrCpy(recCfg->recFilePath, Fwl_GetDefPath(eVIDEOREC_PATH));
	//Eng_StrMbcs2Ucs("D:/AKREC/",recCfg->recFilePath);
	Utl_UStrCpy(recCfg->indexFilePath, Fwl_GetDefPath(eRECIDX_PATH));
	Fwl_ResetDirRoot(recCfg->recFilePath, recCfg->indexFilePath);
	//Eng_StrMbcs2Ucs("D:/AKINDEX/",recCfg->indexFilePath);
	Printf_UC(recCfg->recFilePath);
	Printf_UC(recCfg->indexFilePath);   

    if (gb.isCycMode > 0)
	{
		recCfg->cycRecTimeMs = (gb.isCycMode*60*1000ul);
	}
	else
	{
		recCfg->cycRecTimeMs = 0;
	}
	
	recCfg->cycRecbySpace 		  = AK_FALSE;
	recCfg->cycRecbySpaceSize	  = 0;
	recCfg->recTimeSecLimit       = REC_LIMIT_TIME;
	recCfg->recSizeLimit          = (T_U32)REC_FILE_SIZE_MAX;
	recCfg->recResSize            = REC_VIDEO_RES_SIZE;
	recCfg->useMemIndex           = 0;
#if (SDRAM_MODE == 8)	
	recCfg->asynWriteSize         = (256<<10);
#else
	recCfg->asynWriteSize         = (3<<20);
#endif
	recCfg->autoDelAllFile        = CHK_REC_SPACE_BY_DELALL;

    recCfg->isEnableFocus     = AK_FALSE;
   
    
    recCfg->usrEncCheckCbf        = VideoRecord_StatusCheck_Cbf;
	if (gb.isDetectMode)
	{
        recCfg->encMsLimitPerFrame    = REC_MDETECT_ENCODE_INTERVAL;
		recCfg->detectNoMovingTimeMs  = REC_MDETECT_NO_MOVING_LIMIT;
	}
    else
    {
		recCfg->detectNoMovingTimeMs  = 0;
        recCfg->encMsLimitPerFrame    = 0;
	}
	return AK_TRUE;
}

/*
 * @brief   switch refresh  by diplay mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] tvOutMode     switch mode
 * @return	AK_TRUE:  switch  success, else fail
 */
/*T_BOOL VideoRecord_SwitchTvOut(T_VIDEO_RECORD *pVideoRec,DISPLAY_TYPE_DEV tvOutMode)
{
	T_S32 lRet = AK_EFAILED;
	
	if (AK_NULL == pVideoRec)
	{
		Fwl_Print(C3, M_CAMERA,  "Param Er");
		return AK_FALSE;
	}

	lRet = VideoDisp_SwitchTvOut(pVideoRec->hDisp, tvOutMode, gs.LcdBrightness);

	if (AK_IS_SUCCESS(lRet))
	{
        return AK_TRUE;
	}
	else
	{
       return AK_FALSE;
	}
}*/

/**
 * @brief The function start recorder
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 * @return T_REC_ERROR_STATUS 
 * @retval  REC_ERROR_OK : success, else fail.
 */
T_REC_ERROR_STATUS VideoRecord_StartRecord(T_VIDEO_RECORD *pVideoRec)
{
#ifdef OS_ANYKA
	T_REC_ERROR_STATUS  ret = 0;
    T_REC_AUDIO_INIT_PARAM  recAudioParam;
	T_REC_VIDEO_INIT_PARAM	recVideoParam;
	T_REC_CTRL_INIT_PARAM	recCtlParam;

	if (AK_NULL == pVideoRec)
	{
		Fwl_Print(C3, M_CAMERA,  "Param Er");
		return REC_ERROR_NULL;
	}

	if (0 == pVideoRec->hZoom) 
	{
		Fwl_Print(C3, M_CAMERA,  "VideoZoom_Open Er\n");
		return REC_ERROR_ENCODE_EXP;
	}

    pVideoRec->hMRec						= (T_HANDLE)0;
	//--------------------Encode  -------------------------------------	
    memset(&recCtlParam,   0, sizeof(recCtlParam));
    memset(&recVideoParam, 0, sizeof(recVideoParam));
    memset(&recAudioParam, 0, sizeof(recAudioParam));

    
	VideoRecord_GetRecConfig(pVideoRec, &recCtlParam);

	switch (gs.CamRecFileType)
	{
		case eRECORD_MEDIA_AVI_MPEG4_PCM:
			recAudioParam.audioEncType = eRECORD_MODE_WAV;
			break;
		case eRECORD_MEDIA_AVI_MJPEG_PCM:
			recAudioParam.audioEncType = eRECORD_MODE_WAV;
			break;
		case eRECORD_MEDIA_3GP_MPEG4_AMR:
			recAudioParam.audioEncType = eRECORD_MODE_AMR;
			break;
	}
	recAudioParam.audioEncSamp = 8000;//gs.AudioRecordRate;

	recVideoParam.videoEncType		= gs.CamRecFileType;
	recVideoParam.videoWidth		= pVideoRec->width;
	recVideoParam.videoHeight		= pVideoRec->height;
        
    recVideoParam.viedoSrcWinWidth  = pVideoRec->videoSrcWidth;
    recVideoParam.viedoSrcWinHeight = pVideoRec->videoSrcHeight;
    
	recVideoParam.FPS				= 30;
	recVideoParam.keyfFameInterval	= 35;
#if (SDRAM_MODE == 8)	
	if (eRECORD_MEDIA_AVI_MJPEG_PCM == gs.CamRecFileType )
		recVideoParam.vbps				= 5<<20;
	else
		recVideoParam.vbps				= 2<<20;

#else
	recVideoParam.vbps				= 20<<20;
#endif

    Fwl_Print(C3, M_CAMERA,  "REC mode = %d, width = %d, height = %d.\n", gs.camRecMode[gs.CamRecFileType], pVideoRec->width, pVideoRec->height);

	pVideoRec->hMRec = MRec_Open(&recAudioParam);
	if (0 == pVideoRec->hMRec) 
	{
		return REC_ERROR_ENCODE_EXP;
	}

	ret =  MRec_Start(pVideoRec->hMRec,pVideoRec->hZoom,&recCtlParam, &recAudioParam, &recVideoParam);
	if(REC_ERROR_OK != ret)
	{
		Fwl_Print(C3, M_CAMERA,  "MRec_Start Er\n");
#ifdef OS_ANYKA
        if ((T_HANDLE)0 != pVideoRec->hMRec)
        {
            MRec_Close(pVideoRec->hMRec, AK_FALSE);
            pVideoRec->hMRec = (T_HANDLE)0;
        }
#endif
		return ret;
	}
    Fwl_Print(C3, M_CAMERA,  "start record...");
	MRec_Ioctl(pVideoRec->hMRec, eMREC_IOCTL_FILE_NAME, pVideoRec->fileName);
    Printf_UC(pVideoRec->fileName);

    /**open flash light*/
#ifdef CAMERA_FLASHLIGHT
    if (gs.VideoRecFlashlight)
    {
        Fwl_CameraFlashOn();
    }
#endif
#endif	
    
    return REC_ERROR_OK;
}


/*
 * @brief   preclose(pause) record
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] tvOutMode     switch mode
 * @return	AK_TRUE:  switch  success, else fail
 */
static T_BOOL VideoRecord_PreClose(T_VIDEO_RECORD *pVideoRec)
{
	if (AK_NULL != pVideoRec)
	{
		MRec_Pause(pVideoRec->hMRec);
#if (REC_MENU_WITH_DV != REC_MENU_SHOW_MODE)
        VideoDisp_Pause(pVideoRec->hDisp);
#endif

	}

	return AK_TRUE;
}

/*
 * @brief  stop record
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] isSave       if save record file
 * @param[in] strID         msgbox tips string
 */
static T_VOID VideoRecord_Stop(T_VIDEO_RECORD* pVideoRec, T_BOOL isSave,T_eCSTM_STR_ID strID)
{
	T_BOOL isShowUI = AK_FALSE;
	T_BOOL isNeedSaveFile = AK_FALSE;
	
	if (AK_NULL == pVideoRec)
	{
		Fwl_Print(C3, M_CAMERA,  "NeedNot %s",isSave?"Save":"Cancle");
		return;
	}

	isNeedSaveFile = isSave;
	if (isNeedSaveFile)
	{
	    // check file valid, if invalid, delete file
		if (!Fwl_CheckDriverIsValid(pVideoRec->curPath))
		{
			Fwl_Print(C3, M_CAMERA,  "Invalid Path");
			isNeedSaveFile = AK_FALSE;
		}
	}

	if (isNeedSaveFile)
	{
		T_MENC_INFO recInfo = {0};

		// check video frame count, if not enough for  minlimit, delet file
		MRec_Ioctl(pVideoRec->hMRec, eMREC_IOCTL_GET_REC_INFO, &recInfo);
		//Fwl_Print(C3, M_CAMERA,  "Video Fr.%d\n",recInfo.mediaInfo.total_video_frames);
		if (recInfo.mediaInfo.total_video_frames < REC_FRAME_CNT_MIN)
		{
			isNeedSaveFile = AK_FALSE;
		}
	}

	if (isNeedSaveFile)
	{
	    // get msgbox tips when save file
		if (csNULL != strID)
		{
			T_USTR_INFO strInfo={0};
			
			if (csSTAND_BY != strID)
			{
				Utl_UStrCpy(strInfo, GetCustomString(strID));
			}
			
			Utl_UStrCat(strInfo, GetCustomString(csVIDEO_RECORD_SAVE_NOW));
			Utl_UStrCat(strInfo,pVideoRec->fileName);

			VideoRecord_DispInfoMsgbox(pVideoRec, GetCustomString(csVIDEO_RECORD_SAVE), strInfo, 2);
			isShowUI = AK_TRUE;
		}
		else
		{
			isShowUI = AK_FALSE;
		}
		
		if (VideoRecord_RecSave(pVideoRec,isShowUI))
		{
			Fwl_Print(C3, M_CAMERA,  "Save Ok");
		}
		else
		{
			Fwl_Print(C3, M_CAMERA,  "Save Fail");
		}
	}

	else
	{
		Fwl_Print(C3, M_CAMERA,  "Cancel...");

		if (!Fwl_CheckDriverIsValid((T_pCWSTR)pVideoRec->curPath) && !(pVideoRec->isExitWarning))
    	{
			pVideoRec->isExitWarning = AK_FALSE;
			if (csNULL != strID)
			{
				T_USTR_INFO strInfo={0};
				Utl_UStrCpy(strInfo, GetCustomString(strID));
				Utl_UStrCat(strInfo, GetCustomString(csRecord_EXITING));
				VideoRecord_DispInfoMsgbox(pVideoRec, GetCustomTitle(ctHINT), strInfo, 2);
			}			
		}
		else
		{
			VideoRecord_RecCancel(pVideoRec,strID);			
		}
        
	}
}


/*
 * @brief  close record
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] isSave       if save record file
 * @param[in] strID         msgbox tips string
 */
T_VOID VideoRecord_Close(T_VIDEO_RECORD* pVideoRec, T_BOOL isSave,T_eCSTM_STR_ID strID)
{
	if (AK_NULL == pVideoRec)
	{
		Fwl_Print(C3, M_CAMERA,  "NeedNot %s",isSave?"Save":"Cancle");
		return;
	}	
	VideoRecord_PreClose(pVideoRec);
	VideoRecord_Stop(pVideoRec,isSave,strID);
}


/**
 * @brief The function finish recording and save data
 *
 * @author \b zhengwenbo
 * @date 2006-07-11
 * @param[in] pVideoRec   the point of record control
 * @return AK_TRUE: success     AK_FALSE:fail
 */
static T_BOOL VideoRecord_RecSave(T_VIDEO_RECORD *pVideoRec,T_BOOL isShowUI )
{
    T_BOOL ret = AK_FALSE;

#ifdef OS_ANYKA
    /**close file to save data*/
    if (AK_IS_SUCCESS(MRec_Stop(pVideoRec->hMRec,AK_TRUE)))
    {
        ret = AK_TRUE;
    }
#endif

    /**Display save fail*/
    if (isShowUI)
    {
		T_USTR_INFO strInfo;

		if(ret)
		{
			Utl_UStrCpy(strInfo, GetCustomString(csAUDIOREC_SAVE_OK));
		}
		else
		{
			Utl_UStrCpy(strInfo, GetCustomString(csCAMERA_FILE_SAVE_FAILED));
		}
		VideoRecord_DispInfoMsgbox(pVideoRec, GetCustomTitle(ctHINT), strInfo, 2);
    }

    return ret;
}

/**
 * @brief The function cancel recording and delete recording file
 *
 * @author \b zhengwenbo
 * @date 2006-07-11
 * @param[in] pVideoRec   the point of record control
 */
static T_VOID VideoRecord_RecCancel(T_VIDEO_RECORD *pVideoRec, T_eCSTM_STR_ID strID)
{	
    /**display message box of unsave*/
    if (csNULL != strID)
    {
		T_USTR_INFO strInfo={0};
        Utl_UStrCpy(strInfo, GetCustomString(strID));
		Utl_UStrCat(strInfo, GetCustomString(csVIDEO_RECORD_UNSAVE));
		VideoRecord_DispInfoMsgbox(pVideoRec, GetCustomTitle(ctHINT), strInfo, 2);
	}
#ifdef OS_ANYKA
    /**delete file for unsaving*/
	MRec_Stop(pVideoRec->hMRec,AK_FALSE);
#endif
}



/**
 * @brief The function set refresh flag
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec   the point of record control
 * @param[in] flag  refresh flag
 */
T_VOID VideoRecord_SetRefresh(T_VIDEO_RECORD *pVideoRec, T_U16 flag)
{
    AK_ASSERT_PTR_VOID(pVideoRec, "VideoRecord_SetRefresh():pVideoRec should not be NULL\n");

    if (CAMERA_RECORD_REFRESH_NONE == flag)
    {
        pVideoRec->refreshFlag = flag;
    }
    else
    {
        pVideoRec->refreshFlag |= flag;
    }
}

/*
 * @brief  the function free memory of video record
 * @author Zhengwenbo
 * @date    2006-8-3
 * @param[in] pVideoRec   the point of record control
 * @return return result
 * @retval AK_FALSE malloc fail
 * @retval AK_TRUE malloc success
 */
 static T_BOOL VideoRecord_RecFree(T_VIDEO_RECORD *pVideoRec)
 {
    AK_ASSERT_PTR(pVideoRec, "VideoRecord_RecFree(): input parameter is error\n", AK_FALSE);

    if (AK_NULL != pVideoRec->pPaintParm)
    {
        pVideoRec->pPaintParm = Fwl_Free(pVideoRec->pPaintParm);
    }

    return AK_FALSE;
}

/*
 * @brief    dealing the event of keypad
 * @author Zhengwenbo
 * @date    2008-11-24
 * @param[in]  T_MMI_KEYPAD phyKey
 * @param[in]  T_EVT_PARAM * pEventParm
 * @return  T_eBACK_STATE
 * @retval 
 */
static T_eBACK_STATE VideoRecord_UserKey_Handle(T_MMI_KEYPAD phyKey, T_EVT_PARAM * pEventParm)
{
    T_eBACK_STATE reState = eStay;
    
    switch (phyKey.keyID)
    {
    case kbCLEAR:   //cancel
		/**cancel recording and delete recording file*/		
		VideoRecord_Close(pVideoRec,AK_FALSE, csSTAND_BY);
		
		gb.isDetectMode = 0;//stop Detec
		
        if (PRESS_SHORT == phyKey.pressType)
        {
            reState = eReturn;
        }
        else
        {
            Fwl_KeyStop();
            reState = eHome;
        }
        break;

    case kbOK:          //saving
		/**close movie and close file*/
		VideoRecord_Close(pVideoRec,AK_TRUE, csSTAND_BY);
		reState = eReturn;
        break;
        
    case kbRIGHT:    
	case kbVOICE_UP:
        
		break;
        
	case kbLEFT:
	case kbVOICE_DOWN:
        
        break;
        
    default:
        break;
    }

    return reState;
}

/*
 * @brief    Mapping touch point to keypad according x and y
 * @author ZhuJing
 * @date    2008-11-24
 * @param[in]  T_MMI_KEYPAD *phyKey
 * @param[in]  T_POS x
 * @param[in]  T_POS y
 * @return  T_VOID
 * @retval 
 */
static T_VOID VideoRecord_MapTSCR_To_Key(T_MMI_KEYPAD *key, T_POS x, T_POS y)
{
    T_CAMERA_RECORD_TOOLBAR_ICON i;

    for (i = 0; i < CAMERA_RECORD_ICON_NUM; ++i)
    {
        if (PointInRect(&pVideoRec->paintData.toolBarIconRect[i], x, y))
        {
            break;
        }
    }

    switch(i)
    {
    case CAMERA_RECORD_ICON_SAVE:
        key->keyID = kbOK;
        pVideoRec->refreshFlag |= CAMERA_RECORD_UNSHOW_TOOLBAR;
		break;

    case CAMERA_RECORD_ICON_CANCEL:
        key->keyID = kbCLEAR;
        pVideoRec->refreshFlag |= CAMERA_RECORD_UNSHOW_TOOLBAR;
        break;

    default:
        break;
    }

    return;
}

/*
 * @brief    Dealing the event of touching screen
 * @author ZhuJing
 * @date    2008-11-24
 * @param[in]  T_MMI_KEYPAD *phyKey
 * @param[in]  T_EVT_PARAM * pEventParm
 * @return  T_VOID
 * @retval 
 */
static T_VOID VideoRecord_TSCR_Handle(T_MMI_KEYPAD *phyKey, T_EVT_PARAM * pEventParm)
{
    T_POS       x;
    T_POS       y;

    x = (T_POS)pEventParm->s.Param2;
    y = (T_POS)pEventParm->s.Param3;
    
    phyKey->keyID = kbNULL;
    phyKey->pressType = PRESS_SHORT;
    
    switch (pEventParm->s.Param1) 
    {
    case eTOUCHSCR_UP:  
        if (pVideoRec->toolBarTimerID != ERROR_TIMER)
        {
            Fwl_StopTimer(pVideoRec->toolBarTimerID);
            pVideoRec->toolBarTimerID = ERROR_TIMER;
        }
		
        if (VideoRecord_GetToolBarShowFlag())
        {        
            /*transforming the point(x,y)  to the corresponding key */
            VideoRecord_MapTSCR_To_Key(phyKey, x, y);

            if (phyKey->keyID != kbNULL)
            {
                break;
            }
        }
        pVideoRec->refreshFlag |= CAMERA_RECORD_SHOW_TOOLBAR;
        pVideoRec->toolBarTimerID = Fwl_SetTimerMilliSecond(CAMERA_TOOLBAR_SHOWN_TIME, AK_FALSE);
        break;
            
    case eTOUCHSCR_DOWN:
        break;
           
    case eTOUCHSCR_MOVE:
        break;
            
    default:
        break;
    }
}

/*
 * @brief    Painting or canceling the toolbar, including save and cancel irons.
 * @brief    Reseting the camera stream's paint param , left and width.       
 * @author ZhuJing
 * @date    2008-11-24
 * @param[in]  T_VOID
 * @return  T_VOID
 * @retval 
 */
static T_VOID VideoRecord_ShowToolBar(T_BOOL shownFlag, T_pDATA dstBuf, T_LEN bufW, T_LEN bufH, T_RECT rect)
{
    T_CAMERA_RECORD_TOOLBAR_ICON i;    
    T_RECT srcRect;	
	T_POS totalHeight;
		
    pVideoRec->toolBarShowFlag = shownFlag;	
		
    if (shownFlag)
    {
    	for (i=0; i<CAMERA_RECORD_ICON_NUM; ++i)
		{
			pVideoRec->paintData.toolBarIconRect[i].left = rect.left + rect.width - pVideoRec->paintData.toolBarIconRect[i].width - RECORD_TOOLBAR_OFFSET_X;
		   	totalHeight = pVideoRec->paintData.toolBarIconRect[i].height;
		}
	   
	   	totalHeight += (CAMERA_RECORD_ICON_NUM - 1) * RECORD_TOOLBAR_V_INTERVAL;
	   
	   	pVideoRec->paintData.toolBarIconRect[CAMERA_RECORD_ICON_SAVE].top = (T_POS)((CAMERA_RECORD_LCD_HEIGHT - totalHeight)/2);
	   
		for (i=CAMERA_RECORD_ICON_SAVE+1; i<CAMERA_RECORD_ICON_NUM; ++i)
		{
			pVideoRec->paintData.toolBarIconRect[i].top = pVideoRec->paintData.toolBarIconRect[i-1].top + pVideoRec->paintData.toolBarIconRect[i-1].height + RECORD_TOOLBAR_V_INTERVAL;
		}
	
        for (i = 0; i < CAMERA_RECORD_ICON_NUM; ++i)
        {
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.width  = pVideoRec->paintData.toolBarIconRect[i].width;
            srcRect.height = pVideoRec->paintData.toolBarIconRect[i].height;
			
            Fwl_AKBmpAlphaShow(pVideoRec->paintData.toolBarIconData[i], 
                       pVideoRec->paintData.toolBarIconRect[i].width,
                       srcRect,
                       dstBuf,
                       bufW,
                       pVideoRec->paintData.toolBarIconRect[i],
                       8);
        }

		
    }	
}


/*
 * @brief    get toolbar show flag
 * @author ZhuJing
 * @date    2008-11-24
 * @param[in]  T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE: need show toolbar, else need not show
 */
static T_BOOL VideoRecord_GetToolBarShowFlag(T_VOID)
{
    return pVideoRec->toolBarShowFlag;
}


/*
 * @brief    get chip is 04 or not
 * @author 
 * @date    2012-03-08
 * @param[in]  T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:is 04, else 02
 */
T_BOOL Is_04CHIP(T_VOID)
{
#ifdef OS_ANYKA
	if (CHIP_3771_04SH == drv_get_chip_version() || CHIP_3771_L == drv_get_chip_version())
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
#else
    return AK_FALSE;
#endif
}

T_S32 OSD_Obtain_Semaphore(T_VOID)
{
	if (0 == osd_semaphore)
	{
		osd_semaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	}
	
	return AK_Obtain_Semaphore(osd_semaphore, AK_SUSPEND);
}

T_S32 OSD_Release_Semaphore(T_VOID)
{
	if (0 == osd_semaphore)
	{
		return -1;
	}
	
	return AK_Release_Semaphore(osd_semaphore);
}


#endif


