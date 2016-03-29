/**
 * @file Gbl_Global.h
 * @brief This header file is for global data
 *
 */

#ifndef __GBL_GLOBAL_H__
#define __GBL_GLOBAL_H__

#include "akdefine.h"
#include "Gbl_Resource.h"
#include "gbl_macrodef.h"
#include "anyka_types.h"
#include "eng_alarm.h"
#include "fwl_usb.h"
#include "fwl_display.h"
#include "Fwl_calibrate.h"
#include "Lib_media_struct.h"
#ifdef SUPPORT_FLASH
#include "AF_Player.h"
#endif

#ifdef OS_WIN32
    #include "stdio.h"
    #include "math.h"
    #define _ram
    #define __ram
#else
    #define _ram
    #define __ram
#endif


typedef struct {
    T_U16               KeyLightCount;          //
    T_BOOL              bEarphoneStatus;        //for earphone, TRUE earphone in, false earphone out
    T_BOOL              InPublicMessage;
    T_BOOL              KeyLocked;
    T_BOOL              MainMenuRefresh;
    T_BOOL              PubMsgAllow;

    T_U8                RtcType;
    T_BOOL              bRtcPwrOn; //judge whether the system is power on by RTC
    
    T_TIMER             s_public_timer_id;

    T_BOOL              PowerOffFlag;
    T_BOOL              PowerOffStatus;
    T_BOOL              ResetAfterPowerOff;
	T_BOOL              PowerLowBattery;
	
    T_BOOL              InUsbMessage;

    T_U32               Voltage[SAVE_VOLTAGE_NUM];
    T_U8                VoltageIdx;

#ifdef USB_HOST
    //USB DISK Handle Flag
    T_BOOL              bUDISKAvail;
    T_pVOID             driverUDISK;    //In USB HOST mode ,it means the U disk has been mounted! the type is T_PDRIVER
#endif

#ifdef CAMERA_SUPPORT
	T_BOOL              ChangePath;
	T_S8                nZoomInMultiple; // 1-10 grade
	T_U8                isCycMode; // 1 :CYC, 0:Normal
	T_U8                isDetectMode; // 1 :detectmode, 0:Normal
    T_BOOL              VideoIsRecording;   //0:not recording  1:is recording video file
    T_BOOL              CamRecQuality;
#endif
    
    T_U16               AudioPlaySpeed;     // now suport mp3, amr speed change
    T_U16               AudioPreTime;       /* Audio pre-listen time(seconds) */

    T_BOOL              AlarmDataChanged;   //0:no change  1:changed   used for refresh
    T_U8                AlarmForbidFlag;    //>0:quit some state
    T_BOOL              bInExplorer;

    T_BOOL              bAudioPlaySM;       //in audioplay state machine

    T_BOOL              bAlarming;          //0:not alarming   1:is alarming
    T_BOOL              listchanged;
    T_U32               CommonMesgNumber;

#ifdef TOUCH_SCR
    T_BOOL              bTscrIsValid;    
#endif
} T_GLOBAL;


typedef struct
{
	T_S8 hour;
	T_S8 minute;
	T_U8 index;
}T_WORLDZONEMAP;

typedef struct __VKEY
{
    T_S32   nKeyType; // keyboard ,paddle1 or paddle2
    T_S32   nKeyValue;
}T_GAME_VKEY;

typedef struct {
    T_USTR_FILE     file;
    T_S32           currTime;
    T_S32           savTime;
} T_AVI_BOOKMARK;

typedef struct {
    T_S8                Version[64];        /* system version */
    T_RES_LANGUAGE      Lang;               /* Current system language */
    T_U8                FontSize;           /* Current system font size */
    T_U16               SysVolume;          /* System volume */
    T_U8                LcdBrightness;      /* back light of LCD */
    T_U8                LcdContrast;        /* contrast of LCD: -0x20 -- 0x20 */
    T_U8                SpeakerGain;        /* speaker gain: 0 - 4 */

    T_U8                PowerOnMedia;       /* to set power on media play  */
    T_U8                PowerOffMedia;      /* to set power off media play  */
    T_BOOL              UserStdbyBkImg;     /* 1:user standby image   0:default standby image */
    T_BOOL              MenuPic;            /* Main menu picture switch */
    T_BOOL              bVoiceWakeup;		/* enable or disable Voice Wakeup  */
#ifdef SUPPORT_VISUAL_AUDIO	
	T_BOOL				bVisualAudio;		/* enable or disable Visual Audio  */
#endif
	T_BOOL				bPlayer2Saver; 	/* switch to Screen saver whether from s_audio_player*/

    T_eTIME_MINUTE      PoffTM;             /* delay time(minutes) of power off */
    T_eTIME_MINUTE      LowBatTM;           /* delay time(minutes) of low battery alarm */
    T_eTIME_SECOND      ScSaverTM;          /* delay time(seconds) of displaying screen save */
    T_U16               KeyLightTM;         /* Key light opening time*/

    T_DAY_FORMAT        DayFormat;          /* day format */
    T_U16               DaySeparator;       /* day separator */
    T_TIME_FORMAT       TimeFormat;         /* time format */
    T_U16               TimeSeparator;      /* time separator */

    T_ALARM_TYPE        AlarmClock;
    T_BOOL              LatestIsDayAlarm;   //ak_true: dayalarm; ak_false: week_alarm.
    T_BOOL              AlarmDelay;         // when alarm power on,the register RTC_ALARM_MASK_REG
                                            //   will be cleared, used for alarm delay

    T_U8                AudioRepMode;       /* audio repeat mode */
    T_U8                AudioToneMode;      /* audio tone mode */
	T_U8                AudioPitchMode;      /* audio pitch mode */
    T_U8                VideoRepMode;       /* video repeat mode */   //add by he

#ifdef CAMERA_SUPPORT
    T_U8                CamBrightness;      /* Camera Capture Brightness */
    T_U8                CamSaturation;           /* Camera Capture Color Saturation */
    T_U8                CamContrast;        /* Camera Capture Contrast */
    T_U8                ShotDelayCount;           /* Camera Capture Delay Timer */
    T_U32               CapFileNum;         /* Camera Capture photo number */
    T_BOOL              CamFlashlight;      /* Camera Capture open flag */
    T_U8                CamMode;            /*Camera mode: DC mode or DV mode*/
    T_U8                DCShotMode;        /*DC mode: DC normal shot or multi shot*/
//--------------------------------------


    T_CAMERA_MODE       camPhotoMode;
    T_CAMERA_MODE       camMultiShotMode;
    T_CAMERA_MODE       camRecMode[eRECORD_MEDIA_NUM]; /*Camera record mode*/
    T_U8                camPhotoQty;       // 0:high 1:middle  2:low
	T_U32               RecVbps;
    T_U16               camVideoQty;
//--------------------------------------
    T_BOOL              camAutoSave;
    T_U16               camSoundSw; // 0: off; 1: on;
    T_U16               camFlashSw; // 0: off; 1: on;
    T_U16               camFlashMode;//0:normal 1:moon
    T_U16               camColorEffect;//0 1 2 3 4
    T_U16               camVideoAudio; // 0: off; 1: on;
    T_U32               RawSum; //GBL_RAWSUM

    T_BOOL              VideoRecFlashlight; /* video record use flashlight or not */
    T_U32               RecFileNum;         /* Record file number */
    T_U8                CamRecFileType;     /* Record file type: AVI, 3GP*/
#endif

    T_U8                ImgSlideInterval;   /* image slide show interval */


    T_USTR_FILE         PathPonVideo;       /* Power on video path */
    T_USTR_FILE         PathPoffVideo;      /* Power off video path */
    T_USTR_FILE         PathPonAudio;       /* power on audio path */
    T_USTR_FILE         PathPoffAudio;      /* power off audio path */
    T_USTR_FILE         PathPonPic;         /* power on image path */
    T_USTR_FILE         PathPoffPic;        /* power off image path */
    T_USTR_FILE         PathStdbPic;        /* stand by image path */
    T_USTR_FILE         PathMenuPic;        /* Main men background image path */

    T_USTR_FILE         DefPath[eDEFPATH_NUM];   // default path

    T_eREC_MODE   		AudioRecordMode;    //audio recorder mode: amr or wav
    T_U32				AudioRecordRate;	//audio recorder rate
#ifdef SUPPORT_AUDIOREC_DENOICE
	T_BOOL				bAudioRecDenoice;	//AK_TRUE:progress audio record denoice operation
#endif

    T_U16               AVIBookMarkCount;
//  T_U16               AVIIndex[MAX_AVI_BOOKMARK];
    T_AVI_BOOKMARK      AVIBookmark[MAX_AVI_BOOKMARK];

    
    T_BOOL                emtest;
    T_BOOL              bSetRTCcount;
    T_U32               SysTimeYear;

#ifdef TOUCH_SCR
    T_MATRIX            matrixPtr;
    T_U32               nADCCoordMode; //the mode of the ADC coordinate system
#endif

    T_eAniMenuLevelType AniSwitchLevel;  //Animate Menu Level
    
    T_U32               AudioRecFileNum; // Audio Record File Number

    T_WORLDZONEMAP      curTimeZone;

	T_BOOL				sysbooterrstatus;	

	T_CHR				UsbDescStr[MAX_NUM_USB_DESC_STR];


#ifdef SUPPORT_NETWORK
	T_U8				macaddr[6];
	T_U32				ipaddr;
	T_USTR_FILE			sendfile;
	T_U32				ping_rmt_ip;
	T_U32				tcpclt_rmt_ip;
	T_U32				udp_rmt_ip;
	T_U32				listen_port;
	T_U32				tcpclt_rmt_port;
	T_U32				tcpclt_lc_port;
	T_U32				udp_rmt_port;
	T_U32				udp_lc_port;
#endif

} T_GLOBAL_S;

extern T_GLOBAL_S  gs;//gs_profile[4]

extern T_GLOBAL gb;




#endif/* GBL_GLOBAL_H */
