/**
 * @file
 * @brief Initial functions
 *
 * @author ZouMai
 * @date 2001-07-23
 * @version 1.0
 */

#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Fwl_pfAudio.h"
#include "Fwl_pfCamera.h"
#include "Eng_Math.h"
#include "Lib_state.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfKeypad.h"
#include "ctl_preview.h"
#include "ctl_capture.h"
#include "file.h"
#include "Fwl_rtc.h"
#include "Fwl_pfdisplay.h"
#include "Video_stream_lib.h"
#include "lib_sdfilter.h"
#include "Media_player_lib.h"
#include "Hal_print.h"
#include "drv_api.h"
#include "fwl_tscrcom.h"
#include "akos_api.h"
#include "mount.h"
#include "arch_mmc_sd.h"
#include "fs.h"
#include "Fwl_vme.h"
#include "Fwl_sd.h"
#include "lib_geapi.h"
#include "fwl_net.h"

#ifdef OS_ANYKA
#include "lib_image_api.h"
    
#endif  //#ifdef OS_ANYKA
#define BATTERY_VALUE_CHANGE        4100

#ifdef USB_HOST
#define BATTERY_VALUE_WARN_USBHOST  3650
#endif

#ifdef OS_WIN32
#ifndef LCD_BKL_BRIGHTNESS_MAX
#define LCD_BKL_BRIGHTNESS_MAX 7
#endif
#endif



#if 1   //def OS_ANYKA
T_U8 *szLibNames[MAX_LIB_NUM] =
{
        "IMAGE_LIB:",
    
#ifdef SUPPORT_GE_SHADE
        "GESHADE_LIB:",
#endif

        "MEDIA_LIB:",
        "VIDEO_LIB:",
        "SDCODEC_LIB:",
        "SDFILTER_LIB:",
    
//        "SMCORE_LIB:",
//        "AKOS_LIB:",  
        "FS_LIB:",
        "MOUNT_LIB:",
       // "MTD_LIB:",
        "FHA_LIB:",
        "DRV_LIB:",
#ifdef SUPPORT_NETWORK
		"LWIP_LIB:",
#endif

};

_GetLibVersion GetLibVersions[MAX_LIB_NUM]; 

T_U8   szLibMacro[MAX_LIB_NUM][100];


#endif

const T_STR_FILE AK_DEFPATH[eDEFPATH_NUM] = {
    SYSTEM_PATH,
    AUDIO_PATH,
    AUDIOREC_PATH,
    AUDIOLIST_PATH,
    VIDEO_PATH,
#ifdef CAMERA_SUPPORT
    VIDEOREC_PATH,
#endif
    VIDEOLIST_PATH,
    IMAGE_PATH,
#ifdef CAMERA_SUPPORT
    IMAGEREC_PATH,
#endif
    RECIDX_PATH,
	UPDATE_PATH,
#ifdef SUPPORT_NETWORK
	NETWORK_PATH
#endif

};


// extern T_hSemaphore g_MediaListSem;//xuyr@
extern T_hSemaphore g_AudioPlayerSem;//xuyr@
extern T_VOID Init_WorldTimeZone(T_VOID);
extern T_U8 *FHA_get_version(T_VOID);


/*******************************************************************************
*
* Time zone for all city
*
*******************************************************************************/
T_U16 gb_CityCounts = 178;

T_S16 gb_TimeZone[] = 
{
    -12,-12,-11,-11,-10,-10,-9,-8,-8,-8,-8,-7,-7,-7,-6,    -6,-6,-6,-6,-6,-6,-6,-6,
    -6,-6,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-4,-4,-4,-4,-4,-4,-4,-4,
    -3,-3,-3,-3,-3,-3,-3,-2,-1,-1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,5,
    5,5,5,5,5,5,5,5,5,5,6,6,6,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,
    9,9,9,10,10,10,10,10,10,10,11,11,11,12,12,12,12,12,12,12,14,
};

T_S16 gb_TimeZoneMinutes[] = 
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,-50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,50,
    0,0,0,0,0,50,0,0,0,0,0,0,0,50,50,50,50,0,0,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,50,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,45,0,
};



/**
 * @brief Initialize global variable
 *
 * @autor @b    Baoli.Miao
 *  Initialize SIM card phonebook storage.
 *  Init phonebook map table and group map table.
 * @author ZouMai
 * @date 2001-07-23
 * @param
 * @return T_VOID
 * @retval
 */
T_VOID InitVariable(T_VOID)
{
    // g_MediaListSem = AK_Create_Semaphore(1, AK_PRIORITY);//xuyr@
    g_AudioPlayerSem = AK_Create_Semaphore(1, AK_PRIORITY);//xuyr@

    MList_Memset();

    gb.bEarphoneStatus = AK_FALSE;
    gb.KeyLightCount = 0;
    gb.InPublicMessage = AK_FALSE;
    gb.KeyLocked = AK_FALSE;
    gb.MainMenuRefresh = AK_TRUE;
    gb.PubMsgAllow = AK_FALSE;
    gb.AudioPreTime = 0;

    gb.RtcType = RTC_ALARM;
    gb.bRtcPwrOn = AK_FALSE;
    
    gb.s_public_timer_id = ERROR_TIMER;

    gb.PowerOffFlag = AK_FALSE;
    gb.PowerOffStatus = AK_FALSE;
    gb.ResetAfterPowerOff = AK_FALSE;
    gb.InUsbMessage = AK_FALSE;
    gb.PowerLowBattery = AK_FALSE;
    
    // check fm module is exist
    //gb.FmIsExist = Fwl_FmCheck();

    for(gb.VoltageIdx = 0; gb.VoltageIdx<SAVE_VOLTAGE_NUM; gb.VoltageIdx++)
        gb.Voltage[gb.VoltageIdx] = BATTERY_VALUE_INVALID;
    gb.VoltageIdx = 0;

#ifdef USB_HOST
    gb.bUDISKAvail = AK_FALSE;
    gb.driverUDISK = AK_NULL;
#endif

#ifdef CAMERA_SUPPORT
    gb.VideoIsRecording = AK_FALSE;
    gb.isDetectMode = AK_FALSE;
    gb.isCycMode = 0;
    gb.CamRecQuality = AK_TRUE;
    gb.nZoomInMultiple = 0;
    gb.ChangePath = AK_FALSE;
#endif

    gb.AlarmDataChanged = AK_FALSE;

    gb.bInExplorer = AK_FALSE;

    gb.bAudioPlaySM = AK_FALSE;  // in audio play state machine ?

    gb.bAlarming = AK_FALSE;
    gb.AudioPlaySpeed = _SD_WSOLA_1_0;
    gb.listchanged = AK_FALSE;
    
//    gb.BytesPerSector = 2048;


    Fwl_RandSeed();

#ifdef TOUCH_SCR
    Fwl_EnableTSCR();
#endif

//init the get version fun 
#ifdef OS_ANYKA

    GetLibVersions[IMAGE_LIB] = Img_GetVersionInfo;
    Utl_StrCpy(szLibMacro[IMAGE_LIB], IMAGE_LIB_VERSION);        
    
#ifdef SUPPORT_GE_SHADE
    GetLibVersions[GESHADE_LIB] = GE_GetVersionInfo;
    Utl_StrCpy(szLibMacro[IMAGE_LIB], GE_LIB_VERSION);    
#endif

    GetLibVersions[MEDIA_LIB] = MediaLib_GetVersion;
    Utl_StrCpy(szLibMacro[MEDIA_LIB], MEDIA_LIB_VERSION);

    GetLibVersions[VIDEO_LIB] = VideoLib_GetVersion;
    Utl_StrCpy(szLibMacro[VIDEO_LIB], VIDEO_LIB_VERSION);
    
    GetLibVersions[SDCODEC_LIB] = _SD_GetAudioCodecVersionInfo;
    Utl_StrCpy(szLibMacro[SDCODEC_LIB], AUDIOCODEC_VERSION_STRING); 
    
    GetLibVersions[SDFILTER_LIB] = _SD_GetAudioFilterVersionInfo;
    Utl_StrCpy(szLibMacro[SDFILTER_LIB], AUDIO_FILTER_VERSION_STRING);           

    GetLibVersions[FS_LIB] = FSLib_GetVersion;
    GetLibVersions[MOUNT_LIB] = FS_GetVersion;
    //GetLibVersions[MTD_LIB] = MtdLib_GetVersion;
    GetLibVersions[FHA_LIB] = FHA_get_version;

    GetLibVersions[DRV_LIB] = drvlib_version;
    Utl_StrCpy(szLibMacro[DRV_LIB], _DRV_LIB_VER); 
    
#ifdef SUPPORT_NETWORK
	GetLibVersions[LWIP_LIB] = Fwl_Lwip_GetVersion;
#endif

#endif // #ifdef OS_ANYKA
    AK_FUNCTION_LEAVE("InitVariable");

    return;
}


/**
 * @brief check lib version
 *
 * @author 
 * @date 
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: version consistent , AK_FALSE: version disaccord
 */
T_BOOL CheckLibVersion(T_VOID)
{
    T_BOOL bRet = AK_FALSE;    
    T_S32  nDiffCount = 0;


    T_S32 i = 0;
    T_S8  szRet = -10;

    for(i = 0; i < MAX_LIB_NUM; i++)
    {
        if(GetLibVersions[i] != AK_NULL)
        {
            szRet = Utl_StrCmp((T_U8 *)(GetLibVersions[i]()),szLibMacro[i]);
            if(0 != szRet)
            {
                nDiffCount = nDiffCount + 1;
                AK_DEBUG_OUTPUT("i = %d, %s is disaccord, the head file show \" %s \", the string in the lib is \"%s\" \n " ,i, szLibNames[i],  szLibMacro[i], GetLibVersions[i]());
            }
        }
        else
        {
            AK_DEBUG_OUTPUT(" i =%d ,the function of %s to get version is NULL" , i ,szLibNames[i] );
        }
    }

    if(nDiffCount <= 0)
    {
        bRet = AK_TRUE;
    }

    return bRet;    

}



/**
 * @brief Read user data
 *
 * @author ZouMai
 * @date 2001-12-12
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: success
 */
T_BOOL ReadUserdata(T_VOID)
{
    T_pFILE     fd;

    fd = Fwl_FileOpen(_T(USER_PROFILE), _FMODE_READ, _FMODE_READ);
    if (fd == _FOPEN_FAIL)
    {
        GetDefUserdata();
        return SaveUserdata();
    }

    // the user setting file is  exit, read content

    if (Fwl_GetFileLen(fd) != sizeof(T_GLOBAL_S))
    {
        Fwl_FileClose (fd);
        GetDefUserdata();
        return SaveUserdata();
    }

    if (strcmp(gs.Version,AK_VERSION_SOFTWARE)!=0)
    {
        Fwl_FileClose(fd);
        GetDefUserdata();
        return SaveUserdata();
    }

    // the user setting file is  exit, read content
    Fwl_FileRead(fd, (T_U8*)&gs, sizeof(T_GLOBAL_S));
    if ((UNICODE_DOT != gs.DaySeparator) && (UNICODE_COLON != gs.DaySeparator) 
        && (UNICODE_SOLIDUS != gs.DaySeparator) && (UNICODE_BAR != gs.DaySeparator))
    {
        gs.DaySeparator = UNICODE_COLON;
    }
    
    if ((UNICODE_DOT != gs.TimeSeparator) && (UNICODE_COLON != gs.TimeSeparator))
    {
        gs.TimeSeparator = UNICODE_COLON;
    }

    Fwl_FileClose (fd);

    return AK_TRUE;
}

/**
 * @brief Save user data
 *
 * @author ZouMai
 * @date 2001-12-12
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: success
 */
T_BOOL SaveUserdata(T_VOID)
{
    T_pFILE     fd;

    Fwl_CreateDefPath();

    fd = Fwl_FileOpen(_T(USER_PROFILE), _FMODE_CREATE, _FMODE_CREATE);
    if (FS_INVALID_HANDLE==fd)
    {
        return AK_FALSE;
    }
    Fwl_FileWrite(fd, (T_U8*)&gs, sizeof(T_GLOBAL_S));

    Fwl_FileClose(fd);

    return AK_TRUE;
}

/**
 * @brief Get default data
 *
 * @author ZouMai
 * @date 2001-12-12
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID GetDefUserdata(T_VOID)
{    
    T_S32   i = 0;

    strcpy(gs.Version,AK_VERSION_SOFTWARE); /* system version */
    gs.Lang = eRES_LANG_CHINESE_SIMPLE;         /* Current system language */
    gs.FontSize = CURRENT_FONT_SIZE;              /* Current system font size */
    gs.SysVolume = AK_DEF_VOLUME_VAL;       /* System volume */
    gs.LcdBrightness = LCD_BKL_BRIGHTNESS_MAX - 3;//1;  /* back light of LCD */
    gs.LcdContrast = 10;    /* contrast of LCD: 0 - 20 */
    gs.SpeakerGain = 1;     /* speaker gain: 0 - 4 */

    gs.UserStdbyBkImg = AK_FALSE;       /* Standby picture switch */
    gs.MenuPic = AK_FALSE;      /* Main menu switch */
    gs.bVoiceWakeup = AK_FALSE;    /* enable or disable Voice Wakeup  */

#ifdef SUPPORT_VISUAL_AUDIO    
    gs.bVisualAudio   = AK_FALSE;   /* enable or disable Visual Audio */
#endif
    gs.bPlayer2Saver = AK_FALSE;     /* switch to Screen saver whether from s_audio_player */

    gs.PoffTM = 30;             /* delay time(minutes) of power off */

    //ADD BY WW
    gs.DayFormat     = DAY_FORMAT_YMD;  
    gs.DaySeparator  = UNICODE_COLON;
    gs.TimeFormat    = TIME_FORMAT_24;
    gs.TimeSeparator = UNICODE_COLON;    
    //END OF ADD    
    gs.AlarmClock.DayAlarm = 0;
    gs.AlarmClock.WeekAlarm = 0;
    gs.AlarmClock.WeekAlarmAllType = 2;
    gs.AlarmClock.DayAlarmType = 0;
    gs.AlarmClock.AlarmSoundPathName[0] = 0;
    for(i=0;i<7;i++)
    {
        gs.AlarmClock.WeekAlarmEnable[i] = 0;
    }
    gs.LatestIsDayAlarm = AK_TRUE;
    gs.AlarmDelay = AK_FALSE;

    gs.LowBatTM = 5;            /* delay time(minutes) of low battery alarm */
    gs.ScSaverTM = 10;      /* delay time(milliseconds) of displaying screen save */
    gs.KeyLightTM = 10;     /* delay time(milliseconds) of key light */
    gs.FontSize = FONT_16;  // Current system font size

    gs.AudioRepMode = FILELIST_FETCH_SEQUENCE;        /* MP3 repeat mode */
    gs.AudioToneMode = _SD_EQ_MODE_NORMAL;    /* MP3 tone mode */	
    gs.AudioPitchMode = PITCH_NORMAL;    /* MP3 Pitch mode */
    gs.VideoRepMode = FILELIST_FETCH_SEQUENCE;  // add by he

#ifdef CAMERA_SUPPORT
    gs.CamBrightness = CAMERA_BRIGHTNESS_3;      /* Camera Capture Brightness */
    gs.CamSaturation = CAMERA_SATURATION_3;           /* Camera Capture Color Saturation */
    gs.CamContrast = CAMERA_CONTRAST_4;        /* Camera Capture Contrast */
    gs.ShotDelayCount = 0;            /* Camera Capture Delay Timer */
    gs.CamFlashlight = AK_TRUE;    /* Camera Capture use flashlight or not */

    gs.CamMode = CAM_DC;
    gs.DCShotMode = DC_NORMAL_SHOT;

    gs.CapFileNum = 0;          /* Camera Capture photo number */
    gs.RecFileNum = 0;          /* Recore Avi File number */
    gs.AudioRecFileNum = 0;     /* Audio record file number*/
    gs.CamRecFileType = eRECORD_MEDIA_AVI_MPEG4_PCM;

    gs.VideoRecFlashlight = AK_FALSE;
    gs.camPhotoQty = CAM_QLTY_HIGH;
    gs.camPhotoMode = CAMERA_MODE_VGA;    
    for (i=0; i<eRECORD_MEDIA_NUM; ++i)
        gs.camRecMode[i] = CAMERA_MODE_VGA;
    gs.camMultiShotMode = CAMERA_MODE_VGA;
    gs.camColorEffect = 0;
    gs.camFlashMode = 0;
    gs.camSoundSw = 0;
#endif

    gs.ImgSlideInterval = 0;    /* image slide internal */

    gs.PathPonVideo[0] = 0;             /* power on video init */
    gs.PathPoffVideo[0] = 0;            /* power off video init */
    
    gs.PathPonAudio[0] = 0;
    gs.PathPoffAudio[0] = 0;

    gs.PowerOnMedia = NONE_MEDIA;
    gs.PowerOffMedia = NONE_MEDIA;

    Utl_UStrCpy(gs.PathPonPic,  _T(PON_PIC));
    Utl_UStrCpy(gs.PathPoffPic, _T(POFF_PIC));
    Utl_UStrCpy(gs.PathStdbPic, _T(STDB_PIC));
    Utl_UStrCpy(gs.PathMenuPic, _T(MENU_PIC));

    Fwl_InitDefPath();

    gs.AudioRecordMode = eRECORD_MODE_WAV;
    gs.AudioRecordRate = 8000;
#ifdef SUPPORT_AUDIOREC_DENOICE
    gs.bAudioRecDenoice = AK_FALSE;
#endif

    gs.AVIBookMarkCount = 0;

    gs.emtest = AK_FALSE;

    gs.bSetRTCcount = AK_FALSE;
    gs.SysTimeYear = SYSTEM_DEFAULT_YEAR;

#ifdef TOUCH_SCR
    //set default calibrate data  data to zero
    gs.matrixPtr.X[0] = 0;
    gs.matrixPtr.X[1] = 0;
    gs.matrixPtr.X[2] = 0;
    gs.matrixPtr.X[3] = 0;
    gs.matrixPtr.Y[0] = 0;
    gs.matrixPtr.Y[1] = 0;
    gs.matrixPtr.Y[2] = 0;
    gs.matrixPtr.Y[3] = 0; 
    gs.nADCCoordMode = 0; //0x0e;//0; // default in mode 0  ,there are 8 mode
#endif

    gs.AniSwitchLevel = eMiddleLevel;  // added for animated manage
    
    Fwl_AudioVolumeInit();

    gs.curTimeZone.hour = 8;
    gs.curTimeZone.minute = 0;

#ifdef SUPPORT_SYS_SET
    Init_WorldTimeZone();
#endif
    gs.sysbooterrstatus = AK_FALSE;
    //========================================
#ifdef CAMERA_SUPPORT
    gb.isDetectMode = 0;
    gb.isCycMode    = 0;
#endif

#ifdef SUPPORT_NETWORK
	memset(gs.macaddr, 0, 6);
	IPADDR_CALC(&gs.ipaddr, 192, 168, 1, 1);
	gs.sendfile[0] = 0;
	IPADDR_CALC(&gs.ping_rmt_ip, 192, 168, 1, 2);
	IPADDR_CALC(&gs.tcpclt_rmt_ip, 192, 168, 1, 2);
	IPADDR_CALC(&gs.udp_rmt_ip, 192, 168, 1, 2);
	gs.listen_port = 2000;
	gs.tcpclt_rmt_port = 2000;
	gs.tcpclt_lc_port = 2001;
	gs.udp_rmt_port = 2000;
	gs.udp_lc_port = 2001;
#endif


    return;
}

static T_BOOL Fwl_CheckDefPath(T_pWSTR pPath)
{
    if ((Fwl_FsIsDir(pPath) || Fwl_IsRootDir(pPath)) && \
            Fwl_CheckDriverIsValid(pPath))
        return AK_TRUE;
    else
        return AK_FALSE;
}

T_VOID Fwl_InitDefPath(T_VOID)
{
    T_DEFPATH_TYPE type;
    T_USTR_FILE     Ustrbuf;

    for (type=0; type<eDEFPATH_NUM; type++)
    {
        Eng_StrMbcs2Ucs(AK_DEFPATH[type], Ustrbuf);
        Utl_UStrCpy(gs.DefPath[type], Ustrbuf);
    }
}

T_BOOL Fwl_CreateDefPath(T_VOID)
{
    T_BOOL ret = AK_TRUE;
    T_DEFPATH_TYPE type;

    for (type=0; type<eDEFPATH_NUM; type++)
    {
        if (!Fwl_FileExistAsc((T_pSTR)AK_DEFPATH[type]))
        {
            if (!Fwl_FsMkDirAsc((T_pSTR)AK_DEFPATH[type]))
            {
                ret = AK_FALSE;
            }
        }
    }

    return ret;
}

T_BOOL Fwl_SetDefPath(T_DEFPATH_TYPE type, T_pWSTR pPath)
{
    if (type >= eDEFPATH_NUM)
        return AK_FALSE;

    if (pPath == AK_NULL)
        return AK_FALSE;

    if (Utl_UStrLen(pPath) > MAX_FILENM_LEN)
        return AK_FALSE;

    if (Fwl_CheckDefPath(pPath) == AK_FALSE)
        return AK_FALSE;

    Utl_UStrCpy(gs.DefPath[type], pPath);

    return AK_TRUE;
}

T_pWSTR Fwl_GetDefPath(T_DEFPATH_TYPE type)
{
    T_USTR_FILE Ustrtmp;
    
    if (!Fwl_CheckDefPath(gs.DefPath[type]))
    {
        Eng_StrMbcs2Ucs(AK_DEFPATH[type], Ustrtmp);
        if (Fwl_FsIsDir(Ustrtmp) == AK_FALSE)
            Fwl_FsMkDir(Ustrtmp);

        Utl_UStrCpy(gs.DefPath[type], Ustrtmp);
    }

    return gs.DefPath[type];
}

T_BOOL Fwl_DirIsDefPath(T_pWSTR pPath)
{
    T_DEFPATH_TYPE type;
    T_USTR_FILE     Ustr_tmp;

    if (pPath == AK_NULL)
        return AK_FALSE;

    for (type=0; type<eDEFPATH_NUM; type++)
    {
        Eng_StrMbcs2Ucs(AK_DEFPATH[type], Ustr_tmp);
        if (Utl_UStrCmp(pPath, Ustr_tmp) == 0)
            return AK_TRUE;
    }

    return AK_FALSE;
}

