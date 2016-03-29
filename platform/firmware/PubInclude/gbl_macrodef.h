/**
 * @file Gbl_MacroDef.h
 * @brief This header file is for definition
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __GBL_MACRODEF_H__
#define __GBL_MACRODEF_H__

#include "anyka_types.h"

#define MIN(a, b)  ((a) < (b) ? (a) : (b)) 
#define MAX(a, b)  ((a) > (b) ? (a) : (b)) 

#define MAX_TELE_LEN			88		/**< maximum telephone number length*/
#define MIN_TELE_LEN			1		/**< GSM protocol defined that telenumber with length of 1 can be dialed in some cases */
#define MAX_TIME_LEN			40		/**< maximum time length*/
#define MAX_NAME_LEN			16		/**< maximum name length*/
#define MAX_PSWD_LEN			8		/**< maximum password length*/
#define MIN_PSWD_LEN			4		/**< minimum password length*/
#define MAX_FILEPATH_LEN		(259)//255		/**< max filepath string len (include path, and not include '\0')*/
#define MAX_USER_PROMPT_LENGTH  40		/**< maximum length of user prompt */

/* define maximum string buffer length (bytes) */
#define MAX_INFO_LEN        512         /**< define maximum string buffer length (bytes) */
#define MAX_FILENM_LEN      (259)   // 512     // maximum file name len, same as FS_MAX_PATH_LEN
#define MAX_MUSIC_LEN       512     // maximum music string len
#define MAX_TITLE_LEN       (MAX_FILENM_LEN-3)
#define MAX_FILE_LEN        3000

/* Define Maximum Path Search Transit Deep */
#if (SDRAM_MODE == 8)
#define MAX_PATH_DEEP		6
#else
#define MAX_PATH_DEEP		6
#endif

#define FONT_SIZE_NUM			5			/**< number of font size*/
#define	INITIALIZED_FLAG		0x51fa		/**< a random value for identify initialized or not */
#define MAX_DS_SETTING			5			/**< maximum quantity of data service setting*/
#define GBL_RAWSUM				0x5a5a5a5a

typedef T_CHR	T_STR_INFO[MAX_INFO_LEN+1];			/**< type definition for information string */
typedef T_CHR	T_STR_TELE[MAX_TELE_LEN+1];			/**< type definition for telenumber string */
typedef T_CHR	T_STR_NAME[MAX_NAME_LEN+1];			/**< type definition for telenumber string */
typedef T_CHR	T_STR_TIME[MAX_TIME_LEN+1];			/**< type definition for time string */
typedef T_CHR	T_STR_PSWD[MAX_PSWD_LEN+1];			/**< type definition for password string */
typedef T_CHR	T_STR_FILEPATH[ MAX_FILEPATH_LEN + 1 ];		/**< type definition for file path string*/
typedef T_CHR  	T_STR_PROMPT[MAX_USER_PROMPT_LENGTH+1]; /**< type definition for user prompt string*/
typedef T_CHR	T_STR_20[21];						/**< type definition for string of 20 characters*/
typedef T_CHR	T_STR_50[51];						/**< type definition for string of 50 characters*/
typedef T_CHR	T_STR_100[101];						/**< type definition for string of 100 characters*/
typedef T_CHR	T_STR_200[201];						/**< type definition for string of 200 characters*/
typedef T_CHR	T_STR_300[301];						/**< type definition for string of 300 characters*/
typedef T_CHR	T_STR_400[401];						/**< type definition for string of 400 characters*/
typedef T_CHR	T_STR_500[501];						/**< type definition for string of 500 characters*/
typedef T_CHR	T_STR_600[601];						/**< type definition for string of 600 characters*/
typedef T_CHR	T_STR_800[801];						/**< type definition for string of 800 characters*/
typedef T_CHR	T_STR_1024[1025];					/**< type definition for string of 1024 characters*/
typedef T_CHR	T_STR_8000[8001];					/**< type definition for string of 8000 characters*/
typedef T_CHR	T_STR_10240[10241];					/**< type definition for string of 10240 characters*/
	
typedef T_WCHR	T_WSTR_INFO[MAX_INFO_LEN+1];		
typedef T_WCHR	T_WSTR_TELE[MAX_TELE_LEN+1];		
typedef T_WCHR	T_WSTR_NAME[MAX_NAME_LEN+1];		
typedef T_WCHR	T_WSTR_TIME[MAX_TIME_LEN+1];		
typedef T_WCHR	T_WSTR_PSWD[MAX_PSWD_LEN+1];				
typedef T_WCHR	T_WSTR_FILEPATH[ MAX_FILEPATH_LEN + 1 ];
typedef T_WCHR  T_WSTR_PROMPT[MAX_USER_PROMPT_LENGTH+1]; /**< type definition for user prompt string*/
typedef T_WCHR	T_WSTR_20[21];						/**< type definition for string of 20 characters*/
typedef T_WCHR	T_WSTR_50[51];						/**< type definition for string of 50 characters*/
typedef T_WCHR	T_WSTR_100[101];
typedef T_WCHR	T_WSTR_200[201];						/**< type definition for string of 200 characters*/
typedef T_WCHR	T_WSTR_300[301];						/**< type definition for string of 300 characters*/
typedef T_WCHR	T_WSTR_400[401];						/**< type definition for string of 400 characters*/
typedef T_WCHR	T_WSTR_500[501];						/**< type definition for string of 500 characters*/
typedef T_WCHR	T_WSTR_600[601];						/**< type definition for string of 600 characters*/
typedef T_WCHR	T_WSTR_800[801];						/**< type definition for string of 800 characters*/
typedef T_WCHR	T_WSTR_1024[1025];					/**< type definition for string of 1024 characters*/
typedef T_WCHR	T_WSTR_8000[8001];					/**< type definition for string of 8000 characters*/
typedef T_WCHR	T_WSTR_10240[10241];					/**< type definition for string of 10240 characters*/

typedef T_U16   T_USTR_INFO[MAX_INFO_LEN+1];
typedef T_S8    T_STR_FILE[MAX_FILENM_LEN+1];
typedef T_U16   T_USTR_FILE[MAX_FILENM_LEN+1];


#define T_TSTR_INFO		T_WSTR_INFO
#define T_TSTR_TELE		T_WSTR_TELE
#define T_TSTR_NAME		T_WSTR_NAME
#define T_TSTR_TIME		T_WSTR_TIME
#define T_TSTR_PSWD		T_WSTR_PSWD
#define T_TSTR_FILEPATH	T_WSTR_FILEPATH
#define T_TSTR_PROMPT 	T_WSTR_PROMPT
#define T_TSTR_20		T_WSTR_20	
#define T_TSTR_50		T_WSTR_50
#define T_TSTR_100		T_WSTR_100
#define T_TSTR_200		T_WSTR_200
#define T_TSTR_300		T_WSTR_300
#define T_TSTR_400		T_WSTR_400
#define T_TSTR_500		T_WSTR_500
#define T_TSTR_600		T_WSTR_600
#define T_TSTR_800		T_WSTR_800
#define T_TSTR_1024		T_WSTR_1024
#define T_TSTR_8000		T_WSTR_8000
#define T_TSTR_10240	T_WSTR_10240

#define MAX_FILEPATH_LEN        (259)        /**< max filepath string len (include path, and not include '\0')*/

#define BATTERY_VALUE_WARN          3550
#define BATTERY_VALUE_MIN           3450
#define BATTERY_VALUE_MAX           4180
#define BATTERY_VALUE_TEST          3700
#define BATTERY_VALUE_INVALID       0xFFFFFFFF

/* define maximum quantity */
#define LCD0_SCBAR_SIDE             10  /* Side height of LCD1 scroll bar(pixel) */


/////----------- for ems config
#define RETURN_TO_HOME              100         //return 100 levels

#define INITIALIZED_FLAG            0x51fa      /* a random value for identify initialized or not */
#define BACKLIGHT_ON_ID             0x1000

#define GSMALPHASTART               127
#define GSMALPHAEND                 155
#define GSMALPHANUMBER              29

#define MAX_CALSS0_NUMBER           10       /* the max number of class0 to be displayed*/

#define AK_VOLUME_MAX               18
#define AK_VOLUME_DIGITALMAX        17
#define AK_DEF_VOLUME_VAL           (AK_VOLUME_MAX/2)
#define AK_VOLUME_HEADPMAX          3
#define AK_VOLUME_SPEAKMAX          5
#define DEF_CONTRAST_VAL            183
#define MAX_CONTRAST_VAL            (183+3)
#define MIN_CONTRAST_VAL            (183-3)
#define CONTRAST_LEVEL              1

#define LCD0_COLOR                  24  /* LCD1 color */
#define LCD1_COLOR                  24  /* LCD2 color */

/************************* default charset  ********************************/
#define CHARSET_BIG5                0x07EA   //not supplied yet
#define CHARSET_UCS2                0x03E8
#define CHARSET_UTF8                0xEA


#define SAVE_VOLTAGE_SHIFT          3
#define SAVE_VOLTAGE_NUM            10   // 1 << SAVE_VOLTAGE_SHIFT
#define MAX_AVI_BOOKMARK            50

#define NES_GAME_KEY_NUM            16  //8
#define SNES_GAME_KEY_NUM           24  //12
#define GBA_GAME_KEY_NUM            10
#define MD_GAME_KEY_NUM             24


#define SIM_GAME_NUM                4

#define SIM_GAM_MAX_KEY_NUM         24

#define MAX_NUM_USB_DESC_STR        16


typedef enum {
    eStay = 0,
    eReturn,
    eHome,
    eNext,
    eMenu,
    eOption
} T_eBACK_STATE;

typedef enum {
    kbNULL  = 0xFF,
    kbUP    = 0,      
    kbDOWN  = 1,         
    kbLEFT  = 2,         
    kbRIGHT = 3,
    kbVOICE_UP      = 4,     
    kbVOICE_DOWN    = 5,   
    kbMENU          = 6,         
    kbOK            = 7,
    kbSWA           = 8,          
    kbSWB           = 9,
    kbSWC           = 10, 
    kbSWX           = 11,          
    kbSWY           = 12,
    kbSWZ           = 13,
    kbSW3,  
    kbSW4,          
    kbSW5,
    kbCLEAR,
    kbSTART_MODULE,
    MAX_KEY_NUM
} T_eKEY_ID;

typedef enum {
        PADDLE_KEY_NULL  = 0xFF,
            
        PADDLE_R_DOWN = 0,      
        PADDLE_R_LEFT,      
        PADDLE_MIDDLE_LEFT, 
        PADDLE_MIDDLE_RIGHT,
        PADDLE_L_UP,        
        PADDLE_L_DOWN,      
        PADDLE_L_LEFT,      
        PADDLE_L_RIGHT,     
        PADDLE_R_RIGHT,     
        PADDLE_R_UP,        
        PADDLE_FRONT_LEFT,  
        PADDLE_FRONT_RIGHT, 

        PADDLE_KEY_NUM      //12
} T_ePADDLE_KEY_ID;


typedef enum {
    bNOCHARGE = 0,
    bCHARGING,
    bCHARGEFULL
} T_BatteryStatus;

typedef enum{
    eTIME_0_MIN     = 0,
    eTIME_1_MIN     = 1,
    eTIME_5_MIN     = 5,
    eTIME_10_MIN    = 10,
    eTIME_20_MIN    = 20,
    eTIME_30_MIN    = 30,
    eTIME_40_MIN    = 40,
    eTIME_50_MIN    = 50,
    eTIME_60_MIN    = 60,
    eTIME_70_MIN    = 70,
    eTIME_80_MIN    = 80,
    eTIME_90_MIN    = 90,
    eTIME_100_MIN   = 100,
    eTIME_110_MIN   = 110,
    eTIME_120_MIN   = 120
}T_eTIME_MINUTE;

typedef enum{
    eTIME_0_SEC     = 0,
    eTIME_1_SEC     = 1,
    eTIME_10_SEC    = 10,
    eTIME_20_SEC    = 20,
    eTIME_30_SEC    = 30,
    eTIME_40_SEC    = 40,
    eTIME_50_SEC    = 50,
    eTIME_60_SEC    = 60,
    eTIME_70_SEC    = 70,
    eTIME_80_SEC    = 80,
    eTIME_90_SEC    = 90,
    eTIME_100_SEC   = 100,
    eTIME_110_SEC   = 110,
    eTIME_120_SEC   = 120
}T_eTIME_SECOND;

typedef enum{
    eRECORD_MODE_AMR = 0,             	//以amr格式录音
    eRECORD_MODE_WAV,                 	//以wav格式录音
    eRECORD_MODE_ADPCM_IMA,             //adpcm record
    eRECORD_MODE_MP3,                 	// mp3 
    eRECORD_MODE_AAC,                 	//以AAC格式录音
    eRECORD_MODE_UNKNOW
}T_eREC_MODE;

typedef enum{
	eRECORD_MEDIA_AVI_MPEG4_PCM =0 ,
	eRECORD_MEDIA_AVI_MJPEG_PCM  ,
	eRECORD_MEDIA_3GP_MPEG4_AMR  ,
	eRECORD_MEDIA_NUM              //total record mode number, must be last one
}T_eREC_MEDIA_MODE;

typedef enum {
    eSYSTEM_PATH = 0,
    eAUDIO_PATH,
    eAUDIOREC_PATH,
    eAUDIOLIST_PATH,
    eVIDEO_PATH,
#ifdef CAMERA_SUPPORT
    eVIDEOREC_PATH,
#endif
    eVIDEOLIST_PATH,
    eIMAGE_PATH,
#ifdef CAMERA_SUPPORT
    eIMAGEREC_PATH,
#endif
	eRECIDX_PATH,
	eUPDATE_PATH,

#ifdef SUPPORT_NETWORK
	eNETWORK_PATH,
#endif

    eDEFPATH_NUM
} T_DEFPATH_TYPE;

typedef enum {
    USB_OUT = 0,            //USB 没有插入
    USB_INITIALIZING,       //USB 刚插入，正在尝试与PC连通
    USB_IN,                 //USB 已经插入并与PC连接
    USB_DISCONNECTED,       //USB 已经插入但与PC断开
    USB_STATUS_NUM
} T_USB_STATUS;

typedef enum {
    GAME_NONE = 0xff,
    GAME_NES = 0,
    GAME_SNES = 1,
    GAME_GBA = 2,
    GAME_MD = 3,
    GAME_PIGBOAT,
    GAME_EGG,
    GAME_RACE,
    GAME_21,
    GAME_7COLOR,
    GAME_RECT,
    GAME_BEAD,
    GAME_NUM
}T_GAME_MODE;


// Function return code define
/*
0xXX XX XXXX
    __ __ _____
      | |     |_________ Return code
      | |_____________ Internal modul No.
      |______________  Modul No.
*/


typedef enum{
    // Common
    eRET_PUB_BEIN           = 0x00000000,
    eRET_PUB_SUCCESS,
    eRET_PUB_FAIL,
    eRET_PUB_BAD_PARM,
    eRET_PUB_END            = 0x00ffffff,

    // File system
    eRET_FS_BEGIN           = 0x01000000,
    eRET_FS_READ_ERR,
    eRET_FS_WRITE_ERR,
    eRET_FS_OPEN_ERR,
    eRET_FS_END             = 0x01ffffff,

    // FM
    eRET_FM_BEGIN           = 0x02000000,

    eRET_FM_END             = 0x02ffffff,

    // Camera
    eRET_CAM_BEGIN          = 0x03000000,

    eRET_CAM_END            = 0x03ffffff,

    // Audio player
    eRET_AUDIO_BEGIN        = 0x04000000,

    eRET_AUDIO_END          = 0x04ffffff,

    // video player
    eRET_MOVIE_BEGIN        = 0x05000000,

    eRET_MOVIE_END          = 0x05ffffff,

     //File manage
    eRET_FILEMNG_BEGIN      = 0x06000000,

    eRET_FILEMNG_END        = 0x06ffffff,

    //Memory
    eRET_MEM_BEGIN          = 0x07000000,
    eRET_MEM_MALLOC_ERR,
    eRET_MEM_END            = 0x07ffffff,

    //ADD OHTER MODUL HERE

    //Driver
    eRET_DRV_BEGIN           = 0xfe000000,

    eRET_DRV_END             = 0xfeffffff,
    eRET_CODE_MAX            = 0xffffffff
}T_RET_CODE;


// wake up type define
#define WAKE_NULL              0x00            
#define WAKE_MASK               0xff

#define WAKE_PWROFF             0x01            // wake up from auto power-off alram
#define WAKE_ALARM              0x02
#define WAKE_GPIO               0x04             // wake up from usb plus in or out
#define WAKE_BATT_WARN          0x08             // wake up for low battery warning

//rtc type define
#define RTC_ALARM               0x00                // rtc for alarm
#define RTC_POWEROFF            0x01                //rtc for auto power off
#define RTC_BATT_WARN           0x02                 //low battery waring

//fm define
typedef enum {
    FM_AREA_EUROPE = 0,
    FM_AREA_JAPAN,
    FM_AREA_AMERICA,
    FM_AREA_NUM    
}T_FM_AREA;

#define FM_STATION_NUM           50
/*---------------------- phonebook structure difination end-----------------------*/

typedef enum {
    NONE_MEDIA =0,
    VIDEO_MEDIA,
    AUDIO_MEDIA,
    PICTURE_MEDIA,
    PIC_AUDIO_MEDIA
} T_MEDIA_TYPE;

typedef enum {
    GAME_AUDIO_OFF = 0,
    GAME_AUDIO_ON,
    GAME_AUDIO_AUTO
}T_GAME_AUDIO_TYPE;

//add by ww
typedef enum {
    DAY_FORMAT_YMD = 0,
    DAY_FORMAT_MDY,        
    DAY_FORMAT_DMY
}T_DAY_FORMAT;

typedef enum {
    TIME_FORMAT_12 = 0,
    TIME_FORMAT_24        
}T_TIME_FORMAT;
//end add

typedef enum{
    eFastLevel = 0,
    eMiddleLevel,
    eSlowlyLevel,
    eClose
}T_eAniMenuLevelType;    //Animate Menu level type

typedef enum
{
    INVALID_INPUT_TYPE = -1,
    KEYBOARD_TYPE = 0,
    PADDLE1_TYPE,
    PADDLE2_TYPE,

    MAX_INPUT_TYPE   

}T_eINPUT_TYPE;


typedef enum
{
    PADDLE_NORMAL_MODE,
    PADDLE_GAME_MODE,
    MAX_PADDLE_MODE
    
}T_PADDLE_MODE;

/** @}*/
#endif


