/*******************************************************************************
 * @file Ctl_AVIPlayer.h
 * @brief This header file is for menu definition and function prototype
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiuZhenwu
 * @date 2002-05-25
 * @version 1.0
 * @ref None
 *
 * @author LiuZhenwu
 * @date 2003-01-07
 * @version 1.1
 *******************************************************************************/

#ifndef __UTL_AVIPLAYER_H__
/**
 * @def __UTL_AVIPLAYER_H__
 *
 */
#define __UTL_AVIPLAYER_H__

#include "Log_MediaPlayer.h"
#include "Ctl_FileList.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WINDOW_REFRESH_NONE                     0x00
#define WINDOW_REFRESH_ALL                      0xFF

#define WINDOW_REFRESH_STATUS                   0x01
#define WINDOW_REFRESH_SOUNDBOX                 0x02
#define WINDOW_REFRESH_TIME                     0x04
#define WINDOW_REFRESH_FILENAME                 0x08
#define WINDOW_REFRESH_PROGRESSBAR              0x10
#define WINDOW_REFRESH_AUDIOICON                0x20
#define WINDOW_REFRESH_AUDIOBAR                 0x40

#define VIDEOLIST_MAX_ITEM_QTY                  1024

#define VIDEO_TIMER_NUMBER_QTY                  12
#define VIDEO_VOL_LEVEL_MAX                     10          // the number of volum icon


#if (LCD_CONFIG_WIDTH == 800)
/*define main window position*/
#define BACK_GROUND_WIDTH                       (Fwl_GetLcdWidth())
#define BACK_GROUND_HEIGHT                      89  //50
#define BACK_GROUND_LEFT                        0
#define BACK_GROUND_TOP                         ((Fwl_GetLcdHeight()) - BACK_GROUND_HEIGHT)

/*define progress bar icon position and size*/
#define PROG_BAR_LEFT                           117  //70
#define PROG_BAR_TOP                            (BACK_GROUND_TOP + 14 /*8*/)
#define PROG_BAR_WIDTH                          (BACK_GROUND_WIDTH - (PROG_BAR_LEFT * 2 + 16 /*10*/))
#define PROG_BAR_HEIGHT                         6  ////3
#define PROG_BAR_TOUCH_HEIGHT                   27 //15

/*define progress block icon position and size*/
#define PROG_BLOCK_WIDTH                        24  //14
#define PROG_BLOCK_HEIGHT                       16  //9
#define PROG_BLOCK_LEFT                         PROG_BAR_LEFT
#define PROG_BLOCK_TOP                          (PROG_BAR_TOP - (PROG_BLOCK_HEIGHT - PROG_BAR_HEIGHT)/2)

/*define current playing time icon position and each number icon size*/
#define CURTIME_LEFT                            64  //38
#define CURTIME_TOP                             (BACK_GROUND_TOP + 49/*28*/)
#define CURTIME_WIDTH                           9  //5
#define CURTIME_HEIGHT                          18  //10

/*define playing or pause icon position and size*/
#define PLAY_STATUS_LEFT                        11 //6
#define PLAY_STATUS_TOP                         (BACK_GROUND_TOP + 21/*12*/)
#define PLAY_STATUS_WIDTH                       50 //30
#define PLAY_STATUS_HEIGHT                      52 //30

/*define stop icon position */
#define STOP_STATUS_LEFT                        (BACK_GROUND_WIDTH - 60/*36*/)
#define STOP_STATUS_TOP                         PLAY_STATUS_TOP
#define STOP_STATUS_WIDTH                       50 //30
#define STOP_STATUS_HEIGHT                      52 //30

/*define previous file icon position */
#define PREVIOUS_FILE_LEFT                      71 //42
#define PREVIOUS_FILE_TOP                       (BACK_GROUND_TOP + 7/*4*/)
#define PREVIOUS_FILE_WIDTH                     34 //20
#define PREVIOUS_FILE_HEIGHT                    36 //20

/*define next file icon position */
#define NEXT_FILE_LEFT                          (BACK_GROUND_WIDTH - 130/*78*/)
#define NEXT_FILE_TOP                           PREVIOUS_FILE_TOP
#define NEXT_FILE_WIDTH                         34 //20
#define NEXT_FILE_HEIGHT                        36 //20

/*define return button position */
#define RETURN_BUTTON_LEFT                      (NEXT_FILE_LEFT + NEXT_FILE_WIDTH + 5/*3*/) 
#define RETURN_BUTTON_TOP                       PREVIOUS_FILE_TOP
#define RETURN_BUTTON_WIDTH                     34 //20
#define RETURN_BUTTON_HEIGHT                    36 //20

/*define sound tracks icon position */
#define SOUND_BOX_LEFT                          251 //150
#define SOUND_BOX_TOP                           (BACK_GROUND_TOP + 43 /*26*/)
#define SOUND_BOX_WIDTH                         34 //20
#define SOUND_BOX_HEIGHT                        36 //20

/*define volume bar icon position */
#define VOLUME_BAR_LEFT                         (BACK_GROUND_WIDTH - 133 /*80*/)
#define VOLUME_BAR_TOP                          (BACK_GROUND_TOP + 56/*32*/)
#define VOLUME_BAR_WIDTH                        5	// 2
#define VOLUME_BAR_HEIGHT                       20	// 11
#define VOLUME_BAR_TOUCH_WIDTH                  (19 * VOLUME_BAR_WIDTH)

/*define volume speaker icon position */
#define VOLUME_SPEAK_LEFT                       (BACK_GROUND_WIDTH - 170 /*102*/)
#define VOLUME_SPEAK_TOP                        (BACK_GROUND_TOP + 45 /*26*/)
#define VOLUME_SPEAK_WIDTH                      34 //20
#define VOLUME_SPEAK_HEIGHT                     36 //20

/*define playing file name display position */
#define CUR_FILE_NAME_LEFT                      (SOUND_BOX_LEFT + 40/*24*/)
#define CUR_FILE_NAME_TOP                       (BACK_GROUND_TOP + 49/*28*/)

/*define fastforward speed display position and size */
#define SPEED_ICON_LEFT                         (BACK_GROUND_WIDTH - 66/*40*/)
#define SPEED_ICON_TOP                          (BACK_GROUND_TOP + 17/*10*/)
#define SPEED_ICON_WIDTH                        50  //30
#define SPEED_ICON_HEIGHT                       36  //20

/*define pause icon display position */
#define MOVIE_PAUSE_WIDTH                   145  //86
#define MOVIE_PAUSE_HEIGHT                  65   //36

#else
#if (LCD_CONFIG_WIDTH == 480)

/*define main window position*/
#define BACK_GROUND_WIDTH                       (Fwl_GetLcdWidth())
#define BACK_GROUND_HEIGHT                      50
#define BACK_GROUND_LEFT                        0
#define BACK_GROUND_TOP                         ((Fwl_GetLcdHeight()) - BACK_GROUND_HEIGHT)

/*define progress bar icon position and size*/
#define PROG_BAR_LEFT                           70
#define PROG_BAR_TOP                            (BACK_GROUND_TOP + 8)
#define PROG_BAR_WIDTH                          (BACK_GROUND_WIDTH - (PROG_BAR_LEFT * 2 + 10))
#define PROG_BAR_HEIGHT                         3
#define PROG_BAR_TOUCH_HEIGHT                   15

/*define progress block icon position and size*/
#define PROG_BLOCK_WIDTH                        14
#define PROG_BLOCK_HEIGHT                       9
#define PROG_BLOCK_LEFT                         PROG_BAR_LEFT
#define PROG_BLOCK_TOP                          (PROG_BAR_TOP - (PROG_BLOCK_HEIGHT - PROG_BAR_HEIGHT)/2)

/*define current playing time icon position and each number icon size*/
#define CURTIME_LEFT                            38
#define CURTIME_TOP                             (BACK_GROUND_TOP + 28)
#define CURTIME_WIDTH                           5
#define CURTIME_HEIGHT                          10

/*define playing or pause icon position and size*/
#define PLAY_STATUS_LEFT                        6
#define PLAY_STATUS_TOP                         (BACK_GROUND_TOP + 12)
#define PLAY_STATUS_WIDTH                       30
#define PLAY_STATUS_HEIGHT                      30

/*define stop icon position */
#define STOP_STATUS_LEFT                        (BACK_GROUND_WIDTH - 36)
#define STOP_STATUS_TOP                         PLAY_STATUS_TOP
#define STOP_STATUS_WIDTH                       30
#define STOP_STATUS_HEIGHT                      30

/*define previous file icon position */
#define PREVIOUS_FILE_LEFT                      42
#define PREVIOUS_FILE_TOP                       (BACK_GROUND_TOP + 4)
#define PREVIOUS_FILE_WIDTH                     20
#define PREVIOUS_FILE_HEIGHT                    20

/*define next file icon position */
#define NEXT_FILE_LEFT                          (BACK_GROUND_WIDTH - 78)
#define NEXT_FILE_TOP                           PREVIOUS_FILE_TOP
#define NEXT_FILE_WIDTH                         20
#define NEXT_FILE_HEIGHT                        20

/*define return button position */
#define RETURN_BUTTON_LEFT                      (NEXT_FILE_LEFT + NEXT_FILE_WIDTH + 3) 
#define RETURN_BUTTON_TOP                       PREVIOUS_FILE_TOP
#define RETURN_BUTTON_WIDTH                     20
#define RETURN_BUTTON_HEIGHT                    20

/*define sound tracks icon position */
#define SOUND_BOX_LEFT                          150
#define SOUND_BOX_TOP                           (BACK_GROUND_TOP + 26)
#define SOUND_BOX_WIDTH                         20
#define SOUND_BOX_HEIGHT                        20

/*define volume bar icon position */
#define VOLUME_BAR_LEFT                         (BACK_GROUND_WIDTH - 80)
#define VOLUME_BAR_TOP                          (BACK_GROUND_TOP + 32)
#define VOLUME_BAR_WIDTH                        2
#define VOLUME_BAR_HEIGHT                       11
#define VOLUME_BAR_TOUCH_WIDTH                  (19 * VOLUME_BAR_WIDTH)

/*define volume speaker icon position */
#define VOLUME_SPEAK_LEFT                       (BACK_GROUND_WIDTH - 102)
#define VOLUME_SPEAK_TOP                        (BACK_GROUND_TOP + 26)
#define VOLUME_SPEAK_WIDTH                      20
#define VOLUME_SPEAK_HEIGHT                     20

/*define playing file name display position */
#define CUR_FILE_NAME_LEFT                      (SOUND_BOX_LEFT + 24)
#define CUR_FILE_NAME_TOP                       (BACK_GROUND_TOP + 28)

/*define fastforward speed display position and size */
#define SPEED_ICON_LEFT                         (BACK_GROUND_WIDTH - 40)
#define SPEED_ICON_TOP                          (BACK_GROUND_TOP + 10)
#define SPEED_ICON_WIDTH                        30
#define SPEED_ICON_HEIGHT                       20

/*define pause icon display position */
#define MOVIE_PAUSE_WIDTH                   86
#define MOVIE_PAUSE_HEIGHT                  36
#else
#if (LCD_CONFIG_WIDTH == 320)

/*define main window position*/
#define BACK_GROUND_WIDTH                       (Fwl_GetLcdWidth())
#define BACK_GROUND_HEIGHT                      50
#define BACK_GROUND_LEFT                        0
#define BACK_GROUND_TOP                         ((Fwl_GetLcdHeight()) - BACK_GROUND_HEIGHT)

/*define progress bar icon position and size*/
#define PROG_BAR_LEFT                           70
#define PROG_BAR_TOP                            (BACK_GROUND_TOP + 8)
#define PROG_BAR_WIDTH                          (BACK_GROUND_WIDTH - (PROG_BAR_LEFT * 2 + 10))
#define PROG_BAR_HEIGHT                         3
#define PROG_BAR_TOUCH_HEIGHT                   15

/*define progress block icon position and size*/
#define PROG_BLOCK_WIDTH                        14
#define PROG_BLOCK_HEIGHT                       9
#define PROG_BLOCK_LEFT                         PROG_BAR_LEFT
#define PROG_BLOCK_TOP                          (PROG_BAR_TOP - (PROG_BLOCK_HEIGHT - PROG_BAR_HEIGHT)/2)

/*define current playing time icon position and each number icon size*/
#define CURTIME_LEFT                            38
#define CURTIME_TOP                             (BACK_GROUND_TOP + 28)
#define CURTIME_WIDTH                           5
#define CURTIME_HEIGHT                          10

/*define playing or pause icon position and size*/
#define PLAY_STATUS_LEFT                        6
#define PLAY_STATUS_TOP                         (BACK_GROUND_TOP + 12)
#define PLAY_STATUS_WIDTH                       30
#define PLAY_STATUS_HEIGHT                      30

/*define stop icon position */
#define STOP_STATUS_LEFT                        (BACK_GROUND_WIDTH - 36)
#define STOP_STATUS_TOP                         PLAY_STATUS_TOP
#define STOP_STATUS_WIDTH                       30
#define STOP_STATUS_HEIGHT                      30

/*define previous file icon position */
#define PREVIOUS_FILE_LEFT                      42
#define PREVIOUS_FILE_TOP                       (BACK_GROUND_TOP + 4)
#define PREVIOUS_FILE_WIDTH                     20
#define PREVIOUS_FILE_HEIGHT                    20

/*define next file icon position */
#define NEXT_FILE_LEFT                          (BACK_GROUND_WIDTH - 78)
#define NEXT_FILE_TOP                           PREVIOUS_FILE_TOP
#define NEXT_FILE_WIDTH                         20
#define NEXT_FILE_HEIGHT                        20

/*define return button position */
#define RETURN_BUTTON_LEFT                      (NEXT_FILE_LEFT + NEXT_FILE_WIDTH + 3) 
#define RETURN_BUTTON_TOP                       PREVIOUS_FILE_TOP
#define RETURN_BUTTON_WIDTH                     20
#define RETURN_BUTTON_HEIGHT                    20

/*define sound tracks icon position */
#define SOUND_BOX_LEFT                          150
#define SOUND_BOX_TOP                           (BACK_GROUND_TOP + 26)
#define SOUND_BOX_WIDTH                         20
#define SOUND_BOX_HEIGHT                        20

/*define volume bar icon position */
#define VOLUME_BAR_LEFT                         (BACK_GROUND_WIDTH - 80)
#define VOLUME_BAR_TOP                          (BACK_GROUND_TOP + 32)
#define VOLUME_BAR_WIDTH                        2
#define VOLUME_BAR_HEIGHT                       11
#define VOLUME_BAR_TOUCH_WIDTH                  (19 * VOLUME_BAR_WIDTH)

/*define volume speaker icon position */
#define VOLUME_SPEAK_LEFT                       (BACK_GROUND_WIDTH - 102)
#define VOLUME_SPEAK_TOP                        (BACK_GROUND_TOP + 26)
#define VOLUME_SPEAK_WIDTH                      20
#define VOLUME_SPEAK_HEIGHT                     20

/*define playing file name display position */
#define CUR_FILE_NAME_LEFT                      (SOUND_BOX_LEFT + 24)
#define CUR_FILE_NAME_TOP                       (BACK_GROUND_TOP + 28)

/*define fastforward speed display position and size */
#define SPEED_ICON_LEFT                         (BACK_GROUND_WIDTH - 40)
#define SPEED_ICON_TOP                          (BACK_GROUND_TOP + 10)
#define SPEED_ICON_WIDTH                        30
#define SPEED_ICON_HEIGHT                       20

/*define pause icon display position */
#define MOVIE_PAUSE_WIDTH                   86
#define MOVIE_PAUSE_HEIGHT                  36


#else
#error "LCD NOT right!"
#endif
#endif
#endif


typedef struct {
    T_RECT pos;
    T_pDATA data;
    T_U32 dataLen;
} T_ELEM_IMG;

typedef struct {
    T_POINT pos;
    T_U32 data;
} T_ELEM_STR;

typedef struct {
    T_POINT pos;
    T_USTR_INFO data;
    T_USTR_INFO UData;
    T_U16 UDataLen;
}T_ELEM_USTR;

typedef struct {
    T_POINT pos;        //time number image start-position
    T_U32 time;         //play time (second)
} T_VIDEO_TIME;


typedef struct {
    T_POINT pos;
    T_STR_INFO Text;
    T_USTR_INFO UText;
    T_BOOL bScroll;         //text scroll enable flag
    T_U16  uTextOffset;
}T_VIDEO_USTR; //video file name

typedef struct
{
    T_POS left;
    T_POS top;
    T_LEN width;
    T_LEN height;
    T_U16 rotate;

    T_BOOL bCamera;
} PAINTINFO;                //The Frame show Info

typedef struct {
    T_ELEM_IMG ProgressBar;
    T_ELEM_IMG ProgressBarBack;
    T_ELEM_IMG ProgressBlock;
	T_ELEM_IMG ProgressBlockPress;

	T_ELEM_IMG windowBack;
    T_ELEM_IMG prev;
    T_ELEM_IMG prevPress;
    T_ELEM_IMG next;
    T_ELEM_IMG nextPress;
    T_ELEM_IMG start;
	T_ELEM_IMG startPress;
    T_ELEM_IMG pause;
	T_ELEM_IMG pausePress;

	T_ELEM_IMG Back;
    T_ELEM_IMG volumeProgressBar;
    T_ELEM_IMG volumeProgressBarBack;
    T_ELEM_IMG volumeProgressBlock;
	T_ELEM_IMG volumeProgressBlockPress;
    T_ELEM_IMG complete; 
    T_ELEM_IMG completePress;

    T_VIDEO_USTR    curFile;
    T_VIDEO_TIME    curTime;
    T_VIDEO_TIME    totalTime;

    T_U32           Refresh;
	T_RECT			PrgBlkHitRect; /*used for touch screen, be prone to  capture
									    while hitting the ProgressBlock bar rect */
    T_RECT          Volume;     /*used for touch screen, adjust the volume
                                                                while hitting the volume bar rect */
} T_AVI_WINDOW;

typedef enum
{
    AVIPLAY_OK,
    AVIPLAY_PLAYFAIL,
    AVIPLAY_OPENERR,
    AVIPLAY_NOTIMER,
    AVIPLAY_DUPLICATE
} T_AVIPLAY_ERR;

typedef struct
{
    T_ELEM_IMG icon;
    T_U16       value;
} T_AVI_ICON;

typedef enum
{
    WINDOW_PLAY_MODE = 0,
    FULL_PLAY_MODE
} T_AVIPLAY_MODE;


typedef struct
{
    T_U8    *pY;
    T_U8    *pU;
    T_U8    *pV;
    T_U16   left;
    T_U16   top;
    T_U16   width;
    T_U16   height;
}T_YUV_DATA;


typedef struct {
	T_BOOL 		bAudioEnable;    // 1: no mute  0:mute
    T_BOOL 		initialized;
    T_BOOL		fPrevPress;		// press or not
    T_BOOL  	fNextPress;		// press or not
    T_BOOL  	fCompletePress;	// press or not
    T_BOOL  	fStartPress;	// press or not
    T_BOOL  	fPausePress;	// press or not
    T_BOOL  	fSeekPress;		// press or not
    T_BOOL  	fVolPress;		// press or not
	T_BOOL  	WideScrnShow;
	T_BOOL 		bCheckSize; //check YUV display size in different play mode

    T_U16 			curIndex;
    T_AVIPLAY_MODE 	play_mode;
    T_SEEK_TYPE 	fSeekType;

    T_U32		SeekTime;
    T_TIMER 	tid;
    T_TIMER 	prg_timer_id;   //timer for painting  progress bar
    
    PAINTINFO 		paintInfo;
    T_AVI_WINDOW 	window;
    T_U16			volume_old;	//volume before mute
} T_AVI_PLAYER;

T_BOOL AVIPlayer_Init(T_VOID);
T_VOID AVIPlayer_Free(T_VOID);
T_AVIPLAY_ERR AVIPlayer_Open(T_USTR_FILE fname);
T_BOOL AVIPlayer_Paint(T_VOID);
//T_VOID AVIPlayer_Restart();
T_BOOL AVIPlayer_Stop(T_END_TYPE endType);
T_VOID AVIPlayer_OperateSuspend(T_VOID);
T_VOID AVIPlayer_OperateResume(T_VOID);
T_eBACK_STATE AVIPlayer_Handle(T_EVT_CODE event, T_EVT_PARAM *pEventParm);
T_LEN AVIPlayer_GetSrcWidth(T_VOID);
T_LEN AVIPlayer_GetSrcHeight(T_VOID);
T_S16 AVIPlayer_GetSrcFPS(T_VOID);
//T_eMOVIE_STATUS AVIPlayer_GetStatus();
T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName);
T_AVIPLAY_ERR AVIPlayer_CheckVideoFile(T_pCWSTR pFilePath);
T_VOID AVIPlayer_UpdateVideoList(T_VOID);
T_BOOL AVIPlayer_PlayFromFile(T_pWSTR FullPath);
T_BOOL AVIList_Add(T_pCWSTR pFilePath, T_BOOL SearchSub);
T_VOID AVIPlayer_SetTextOffset(T_VOID);
T_eMPLAYER_STATUS  AVIPlayer_GetStatus(T_VOID);

#ifdef RGB_LCD
T_BOOL AVIPlayer_NextFile(T_VOID);
T_BOOL AVIPlayer_PrevFile(T_VOID);
#endif
T_BOOL AVIPlayer_IsPlaying(T_VOID);
T_S32 AVIPlayer_Print(T_pCSTR s, ...);
T_VOID AVIPlayer_GetRes(T_VOID);
T_VOID AVIPlayer_YUV2DispBuff(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 srcH);

#ifdef __cplusplus
}
#endif

#endif

