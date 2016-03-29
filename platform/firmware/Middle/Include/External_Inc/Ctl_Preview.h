
#ifndef __CTL_PREVIEW_H__
#define __CTL_PREVIEW_H__

#include    "Ctl_ToolBar.h"

/**refresh flag*/
#define CAMERA_PREVIEW_REFRESH_NONE 0
#define CAMERA_PREVIEW_REFRESH_YUV  1
#define CAMERA_PREVIEW_REFRESH_BLACK 0x02

#define PREVIEW_DETECT_INTERVAL   (200) //  ms
#define PREVIEW_INTERVAL          (20) //  ms
#define SHOT_INTERVAL_TIME        (1000)
#define ZOOM_LVL_SET_INTERVAL_TIME   (2000)

#if 0
#define CAM_PRE_WINDOW_PIXEL_WIDTH     1280
#define CAM_PRE_WINDOW_PIXEL_HEIGHT    720
#else
#define CAM_PRE_WINDOW_PIXEL_WIDTH     640
#define CAM_PRE_WINDOW_PIXEL_HEIGHT    480
#endif

#define CAM_DC                      1
#define CAM_DV                      2
#define CAM_STICKER                 4

#define DC_NORMAL_SHOT              1
#define DC_MULTI_SHOT               2
#if (LCD_CONFIG_WIDTH == 800)

#define CAMMODE_PIC_WIDTH           40
#define CAMMODE_PIC_HEIGHT          84

#define CAMMENU_PIC_WIDTH           40
#define CAMMENU_PIC_HEIGHT          42

#define CAM_PREV_WIN_W				480
#define CAM_PREV_WIN_H				360	// 384	// (640X480)/800

#else 
#if (LCD_CONFIG_WIDTH == 480)
#define CAMMODE_PIC_WIDTH           24
#define CAMMODE_PIC_HEIGHT          48

#define CAMMENU_PIC_WIDTH           24
#define CAMMENU_PIC_HEIGHT          24

#define CAM_PREV_WIN_W				PREVIEW_WIDTH//PREVIEW_WIDTH
#define CAM_PREV_WIN_H				PREVIEW_HEIGHT//PREVIEW_HEIGHT

#else
#if (LCD_CONFIG_WIDTH == 320)
#define CAMMODE_PIC_WIDTH           24
#define CAMMODE_PIC_HEIGHT          48

#define CAMMENU_PIC_WIDTH           24
#define CAMMENU_PIC_HEIGHT          24

#define CAM_PREV_WIN_W				640
#define CAM_PREV_WIN_H				480

#endif
#endif
#endif

#define CAMMODE_PIC_LEFT            (MAIN_LCD_WIDTH- CAMMODE_PIC_WIDTH - 2)
#define CAMMODE_PIC_TOP             ((MAIN_LCD_HEIGHT- CAMMODE_PIC_HEIGHT) / 2)


#define CAMMENU_PIC_LEFT            CAMMODE_PIC_LEFT
#define CAMMENU_PIC_TOP             (CAMMODE_PIC_TOP + CAMMODE_PIC_HEIGHT + 5)

#define CAMOK_PIC_TOP               (CAMMODE_PIC_TOP - CAMMENU_PIC_HEIGHT - 5)
#define CAMRET_PIC_TOP              (CAMMENU_PIC_TOP + CAMMENU_PIC_HEIGHT + 5)



extern T_BOOL gbCameraPreviewOsdFreshFlag;



#define ERROR_BUTTON_ID             0xffffffff

typedef struct
{
    T_U32               width;
    T_U32               height;
    T_U32               c1;             //used for tmp
    T_U32               c2;             //used for tmp
    T_U16               c3;
    T_U16               c4;
    T_U16               cnt;
    T_U8                *ybuf;
    T_U8                *ubuf;
    T_U8                *vbuf;
}T_SHOT_PARAM;


typedef struct
{
    T_U32               Id;
    T_U16               Name[40];
    T_pSTATEICON_DATA   stateIcon;
}T_CAM_BUTTON_RES;

typedef struct
{
    T_CAM_BUTTON_RES   CapRecSwitch;   //拍照录像切换
    T_CAM_BUTTON_RES   ModeSelect;     //模式选择:普通，连拍
    T_CAM_BUTTON_RES   PhotoSize;      //size
    T_CAM_BUTTON_RES   Brightness;     //明暗度
    T_CAM_BUTTON_RES   Contrast;       //对比度
    T_CAM_BUTTON_RES   Saturation;     //饱和度
    T_CAM_BUTTON_RES   NightMode;      //夜间模式开关
    T_CAM_BUTTON_RES   DelayCap;       //延时拍摄
    T_CAM_BUTTON_RES   FlashLight;     //闪光灯开关
    T_CAM_BUTTON_RES   PhotoQuality;   //拍摄质量
    T_CAM_BUTTON_RES   SavePath;       //设置存储路径
    T_CAM_BUTTON_RES   RecFileType;    //录像格式切换:AVI,3GP
    T_CAM_BUTTON_RES   DetectMode;     //录像移动侦测:移动侦测,普通模式
    T_CAM_BUTTON_RES   CycleMode;      //录像循环:循环录像,普通模式
    T_CAM_BUTTON_RES   KacaMode;      //Kaca open or close
}T_CAM_BUTON_ICON;


//=======录像UI======

typedef enum
{
    CAMERA_RECORD_ICON_SAVE = 0,
    CAMERA_RECORD_ICON_CANCEL,
    CAMERA_RECORD_ICON_NUM,
}T_CAMERA_RECORD_TOOLBAR_ICON;

typedef struct{    
    T_pDATA             redicon;        //red icom    
    T_LEN               iconWidth;      //width of icon
    T_LEN               iconHeight;     //height of icon
}T_VIDEO_REDICON;



typedef struct {
	T_VIDEO_REDICON     recordingIcon; 
    T_RECT              toolBarIconRect[CAMERA_RECORD_ICON_NUM]; 
    T_pDATA             toolBarIconData[CAMERA_RECORD_ICON_NUM];
} T_VIDEO_REC_UI;

typedef enum
{
    VIDEO_REC_INTERVAL_NONE = 0,
    VIDEO_REC_INTERVAL_2    = 2,
    VIDEO_REC_INTERVAL_5    = 5,
    VIDEO_REC_INTERVAL_15   = 15,
    VIDEO_REC_INTERVAL_30   = 30    
}T_VIDEO_REC_INTERVAL;


//=====================

typedef struct{
    T_MSGBOX    msgbox;
    T_U8        *pY;
    T_U8        *pU;
    T_U8        *pV;
    T_U8        *pY2;
    T_U8        *pU2;
    T_U8        *pV2;
    T_U8        *bufPtr;

    T_pTOOLBAR  pToolBar;
    T_CAM_BUTON_ICON ButtonIcon;
    T_U32       OldFocuBtnId;
    T_BOOL      SuspendToCam;

    T_BOOL      bFlashLight;
    T_TIMER     prev_timer_id;      //view timer
    T_TIMER     delay_shot_tm_id;     //shot delay timer, interval is 1s
    T_U8        shot_delay_count;      //delay time from press ok to finish shot, total delay time = shot_delay_count * 1s
    T_U16       refreshFlag;        //refresh lcd

    T_U32       prev_window_width;      // preview window width
    T_U32       prev_window_height;     // preview window height
    T_U8        camMode;                // camera mode: shot, multi-shot, video recorder
    T_U32       firstSrcWidth;          // Preview first Width

    T_U8        brightness;
    T_U16       camFocusLevel;          // 0 1 2 3 4 5


    T_TIMER     zoomlevel_shown_timer_id;
    T_BOOL      bCameraInit;            // 1: init ok    0:init fail
    T_BOOL      bCameraOpen;            // 1: init ok    0:init fail

    T_pDATA		pCamDCMode;
    T_pDATA		pCamDVMode;
    T_pDATA		pCamMenu;
    T_pDATA		pCamOk;
    T_pDATA		pCamRet;

    T_RECT      CamDCModeRect;
    T_RECT      CamDVModeRect;
    T_RECT      CamMenuRect;
    T_RECT      CamOkRect;
    T_RECT      CamRetRect;

    T_SHOT_PARAM shotParm;

	T_VIDEO_REC_UI recordPaintData;
	T_HANDLE			hZoom;
	T_eAniMenuLevelType	AniLevel;
}T_PREVIEW;


T_VOID Preview_Free(T_PREVIEW *pPreview);
T_BOOL Preview_Init(T_PREVIEW *pPreview);
T_eBACK_STATE Preview_handle(T_PREVIEW* pPreview, T_EVT_CODE Event, T_EVT_PARAM *pEventParm);
T_VOID Preview_SetFocusLevel(T_PREVIEW *pPreview, T_U16 level);
T_U16 Preview_GetRefresh(T_PREVIEW *pPreview);
T_VOID Preview_SetRefresh(T_PREVIEW *pPreview, T_U16 refresh);
T_VOID Preview_DispInfoMsgbox(T_PREVIEW *pPreview, T_pCWSTR title, T_pCWSTR content, T_U16 retLevel);
T_BOOL Preview_SetFeature(T_PREVIEW *pPreview);
T_VOID Preview_Catch_a_YUV(T_PREVIEW *pPreview);
T_VOID Preview_Catch_a_YUV_NoWait(T_PREVIEW *pPreview);
T_VOID Preview_UpdateVar(T_PREVIEW*pPreview);
T_VOID Preview_Invalidate_YUV(T_PREVIEW *pPreview);
T_BOOL Preview_CameraStart(T_PREVIEW *pPreview);
T_VOID Preview_GetFocusRect(T_S16 *focusWidth ,  T_S16 *focusHeight , T_U32 focusLevel, T_CAMERA_MODE Cammode);

T_BOOL Preview_CamStrmOpen(T_PREVIEW *pPreview);
T_BOOL Preview_CamStrmClose(T_PREVIEW *pPreview);

T_VOID Preview_SwitchCamMode(T_PREVIEW *pPreview, T_U8 cam_mode);

//add for camera preview toolbar .
T_BOOL Preview_ToolBarNew(T_PREVIEW *pPreview);
T_VOID Preview_ToolBarFree( T_PREVIEW *pPreview);
T_BOOL Preview_ToolBarInitRecFileType(T_PREVIEW *pPreview);

T_VOID Preview_GetCurWinSize(T_PREVIEW* pPreview, T_U8 camMode, T_U32 *pWidth, T_U32 *pHeight);
T_VOID Preview_ResetWindow(T_PREVIEW* pPreview, T_U32 width, T_U32 height);
T_VOID Preview_SetFocusWindow(T_PREVIEW* pPrev);

#endif // #ifndef __CTL_PREVIEW_H__

