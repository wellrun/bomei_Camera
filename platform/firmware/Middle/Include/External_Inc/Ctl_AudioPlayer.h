
/**
 * @filename Ctl_AudioPlayer.h
 * @brief   AudioPlayer definition and function prototype
 *
 * This file declare Anyka AudioPlayer Module interfaces.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Li Zhuobin
 * @date    2006-08-21
 * @version 1.0
 * @ref
 */
 
#ifndef __CTL_AUDIOPLAYER_H__
#define __CTL_AUDIOPLAYER_H__

#include "Ctl_FileList.h"
#include "Ctl_MsgBox.h"
#include "fwl_keyhandler.h"
#include "Log_MediaPlayer.h"
#include "Ctl_APlayerList.h"
#include "svc_medialist.h"

#ifdef __cplusplus
extern "C" {
#endif

/** AudioPlayer init return value */
typedef enum _AUDIOPLAYER_INIT_RET {
    AUDIOPLAYER_INIT_ERROR = 0,         /**< AudioPlayer init error */
    AUDIOPLAYER_INIT_PLAY,              /**< AudioPlayer have init */
    AUDIOPLAYER_INIT_OK,                /**< AudioPlayer init success */
    AUDIOPLAYER_INIT_RET_NUM            /**< quantity of AudioPlayer init return value */
} T_AUDIOPLAYER_INIT_RET;

/** AudioPlayer handle ret */
typedef enum _AUDIOPLAYER_HANDLE_RET {
    AUDIOPLAYER_HANDLE_ERROR = 0,       /**< AudioPlayer handle error */
    AUDIOPLAYER_HANDLE_NONE,            /**< AudioPlayer handle not handle this action */
    AUDIOPLAYER_HANDLE_STAY,            /**< stay AudioPlayer module */
    AUDIOPLAYER_HANDLE_AUDIOSWITCH,     /**< AudioPlayer switch a audio */
    AUDIOPLAYER_HANDLE_MENU,            /**< AudioPlayer go to config menu */
    AUDIOPLAYER_HANDLE_STATECHANGE,     /**< AudioPlayer change state */

    AUDIOPLAYER_HANDLE_SWITCH,          /**< AudioPlayer switch audio */
        
    AUDIOPLAYER_HANDLE_EXIT,            /**< exit AudioPlayer module */
    AUDIOPLAYER_HANDLE_EXITHOME,        /**< exit AudioPlayer module and go to main menu */
    AUDIOPLAYER_HANDLE_LIST,            /**< AudioPlayer go to list interface */

    AUDIOPLAYER_HANDLE_VOICEREFRESH,    /**< AudioPlayer change voice */
    AUDIOPLAYER_HANDLE_PROGRESSREFRESH, /**< AudioPlayer progress refresh */
    AUDIOPLAYER_HANDLE_NUM              /**< quantity of AudioPlayer return value */
} T_AUDIOPLAYER_HANDLE_RET;

/** AudioPlayer action */
typedef enum _AUDIOPLAYER_ACT {
  AUDIOPLAYER_ACT_NONE = 0,             /**< no action */

  AUDIOPLAYER_ACT_STOP,                 /**< go to stop state */
  AUDIOPLAYER_ACT_PLAY,                 /**< go to play state */
  AUDIOPLAYER_ACT_PLAY_AB,              /**< go to AB play state */
  AUDIOPLAYER_ACT_FORWARD,              /**< go to forward state */
  AUDIOPLAYER_ACT_BACKWARD,             /**< go to backward state */
  AUDIOPLAYER_ACT_AUDITION,             /**< go to audition state */
  AUDIOPLAYER_ACT_PAUSE,                /**< go to pause state */
  

  AUDIOPLAYER_ACT_MOVEBACK,             /**< go to move back state */
  AUDIOPLAYER_ACT_MOVEFORWARE,          /**< go to move forware state */

  AUDIOPLAYER_ACT_MARK_A,               /**< mark audio a point */
  
  AUDIOPLAYER_ACT_STOP_PLAY_AB,         /**< stop ab play and exit ab play state */
  AUDIOPLAYER_ACT_STOP_FORWARD,         /**< stop forware and exit forward state */
  AUDIOPLAYER_ACT_STOP_BACKWARD,        /**< stop backward and exit backward state */
  AUDIOPLAYER_ACT_STOP_PAUSE,           /**< exit pause state */
  
  
  AUDIOPLAYER_ACT_EXIT,                 /**< exit */
  AUDIOPLAYER_ACT_EXITHOME,             /**< exit to home */
  AUDIOPLAYER_ACT_LIST,                 /**< go list */
  AUDIOPLAYER_ACT_CONFIG,               /**< go to config */
  
  AUDIOPLAYER_ACT_SPEEDCHANGE,          /**< change audio play speed */
  
  AUDIOPLAYER_ACT_AUTOSWITCH,           /**< auto switch to next audio */
  AUDIOPLAYER_ACT_SWITCH_PREV,          /**< switch to previous audio */
  AUDIOPLAYER_ACT_SWITCH_NEXT,          /**< switch to next audio */

  AUDIOPLAYER_ACT_VOL_CHANGE,           /**< change volume */
  AUDIOPLAYER_ACT_NUM                   /**< AudioPlayer action quantity */      
} T_AUDIOPLAYER_ACT;

/** AudioPlayer state */
typedef enum _AUDIOPLAYER_STATE {
    AUDIOPLAYER_STATE_NONE = 0,         /**< not AudioPlayer state */
    AUDIOPLAYER_STATE_STOP,             /**< stop state */
    AUDIOPLAYER_STATE_PLAY,             /**< simple play state */ 
    AUDIOPLAYER_STATE_BACKGROUNDPLAY,   /**< background play state */
    AUDIOPLAYER_STATE_AB_PLAY,          /**< AB play state */  
    AUDIOPLAYER_STATE_AUDITION,         /**< audition state, several seconds play*/ 
    AUDIOPLAYER_STATE_PAUSE,            /**< pause state */
    AUDIOPLAYER_STATE_FORWARD,          /**< forward state */         
    AUDIOPLAYER_STATE_BACKWARD,         /**< backward state */ 
    AUDIOPLAYER_STATE_NUM               /**< AudioPlayer state quantity */     
} T_AUDIOPLAYER_STATE;  

typedef enum _AUDIOPLAYER_INTERFACE {
    AUDIOPLAYER_INTERFACE_NONE = 0,
    AUDIOPLAYER_INTERFACE_MAIN,
    AUDIOPLAYER_INTERFACE_MENU,
    AUDIOPLAYER_INTERFACE_LIST,
    AUDIOPLAYER_INTERFACE_NUM
} T_AUDIOPLAYER_INTERFACE;
                       
/** AudioPlayer channel */                             
typedef enum _AUDIOPLAYER_CHANNEL {
  AUDIOPLAYER_CHANNEL_NONE = 0,         /**< no channel ouput */
  AUDIOPLAYER_CHANNEL_LEFT,             /**< just left channel ouput */
  AUDIOPLAYER_CHANNEL_RIGHT,            /**< just right channel ouput */
  AUDIOPLAYER_CHANNEL_MONO,             /**< just one channel ouput */
  AUDIOPLAYER_CHANNEL_STEREO,           /**< two channel ouput */
  AUDIOPLAYER_CHANNEL_NUM               /**< AudioPlayer output channel quantity */
} T_AUDIOPLAYER_CHANNEL;


/** update audio lib return value */ 
typedef enum{
    UPDATE_ERROR = 0,           
    UPDATE_SUCCESS,             
    UPDATE_FAIL          
}T_AUDIOPLAYER_UPDATE;




/** @{@name Define the state handle function type */
/** state handle function type */
typedef T_AUDIOPLAYER_HANDLE_RET    (*T_fAUDIOPLAYER_STATE_HANDLE)(T_AUDIOPLAYER_ACT action);

/** @} */

/** @{@name Define the callback function type */
/** callback function for refresh the interface */
typedef T_VOID                      (*T_fAUDIOPLAYER_REFRESH_CALLBACK)(T_VOID);

/** callback function for get lyric */
typedef T_BOOL                      (*T_fAUDIOPLAYER_GET_LYRIC_CALLBACK)(T_pCWSTR pAudioPath);

/** callback function for change audio name in interface */
typedef T_VOID                      (*T_fAUDIOPLAYER_SET_NAMEINFO_CALLBACK)(T_VOID);

/** callback function for check hit which button  */
typedef T_MMI_KEYPAD                (*T_fAUDIOPLAYER_HIT_BUTTON_CALLBACK)(T_POS x, T_POS y, T_EVT_PARAM *pEventParm);

/** @} */

/** Define AudioPlayer struct */
typedef struct _AUDIOPLAYER {
    T_ICONEXPLORER                  *pIconExplorer;         /**< AudioPlayer Menu, for show */
    T_AUDIOPLAYER_ACT               Action;                 /**< AudioPlayer Action */
    T_AUDIOPLAYER_STATE             CurState;               /**< AudioPlayer current state */
    T_AUDIOPLAYER_STATE             OldState;               /**< AudioPlayer old state */
    T_U8                            *pMidiBuf;              /**< AudioPlayer midi buf pointer */
    T_TIMER                         PlayTimer;              /**< AudioPlayer play timer */
    T_TIMER                         AuditionTimer;          /**< AudioPlayer Audition timer id*/      

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
    T_TIMER                         SeekTimer;           	/**< AudioPlayer Seektimer id*/      
#endif

    T_U32                           CurTime;                /**< the current position of playing audio */  
    T_U32                           TotalTime;              /**< the total length of playing audio */
    T_U32                           TimeBeforSeek;          /**< the positon before seek */
    T_U32                           PlayFileId;             /**< current play audio id */
    // T_SONG_INFO                     PalySongInfo;          // this is new added !!!, CurMetaInfo will be discard 
    T_USTR_FILE                     PlayFileName;           /**< current play audio file name */
	T_USTR_FILE                     path;           		/**< current play audio paht name */
    T_eMEDIALIB_MEDIA_TYPE          CurType;                /**< the type of playing audio */
   // _SD_T_META_INFO                 CurMetaInfo;            // CurMetaInfo will be discard !!!
    T_U32                           Repeat_A;               /**< the position of repeat A */
    T_U32                           Repeat_B;               /**< the position of repeat buf */
    T_AUDIOPLAYER_CHANNEL           Channel;                /**< AudioPlayer channel */
    T_U32                           SuspendFlag;            /**< hang flag */       
    T_fAUDIOPLAYER_STATE_HANDLE     fState_Handle;          /**< the function pointer of audio action handle */
    T_fAUDIOPLAYER_REFRESH_CALLBACK fRefreshCallback;       /**< callback function for interface refresh */
    T_fAUDIOPLAYER_GET_LYRIC_CALLBACK fGetLyricCallback;    /**< callback function for getting lyric */
    T_fAUDIOPLAYER_SET_NAMEINFO_CALLBACK fSetNameInfoCallback;  /**< callback function for changing audio name in interface */
    T_AUDIOPLAYER_INTERFACE         Interface;              /**< AudioPlayer in menu, list, background or main Interface */
    T_fAUDIOPLAYER_HIT_BUTTON_CALLBACK   fHitButtonCallback;     /**<check hit whick button*/

    T_MSGBOX                        MsgBox;                 /**<show message>*/
	T_BOOL							bAllowSeek;
	T_USTR_FILE                     pathExplorer;        	/**< path when play from explorer */
} T_AUDIOPLAYER;

/**
 * @brief   creat and init the AudioPlayer module.
 *
 * Should be called before use AudioPlayer module.
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_AUDIOPLAYER *
 * @retval  AK_TRUE  init ok
 * @retval  AK_FALSE init error
 */
T_AUDIOPLAYER_INIT_RET AudioPlayer_Init(T_VOID);

/**
 * @brief   AudioPlayer module handle function.
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] Event input Event
 * @param   [in] pParam Event parm pointer

 */
T_AUDIOPLAYER_HANDLE_RET AudioPlayer_Handler(T_EVT_CODE event, T_EVT_PARAM *pEventParm);


//T_VOID AudioPlayer_SyncFocusWithPlayList(T_VOID);

/**
 * @brief   Destroy the AudioPlayer module.
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval  T_VOID
 */
T_VOID AudioPlayer_Destroy(T_VOID);

/**
 * @brief   free the memory.
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval  T_VOID
 */
T_VOID AudioPlayer_Free(T_VOID);

/**
 * @brief   Get AudioPlayer current state
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_AUDIOPLAYER_STATE
 * @retval  AudioPlayer current state
 */
T_AUDIOPLAYER_STATE AudioPlayer_GetCurState(T_VOID);

T_AUDIOPLAYER_STATE AudioPlayer_GetOldState(T_VOID);


/**
 * @brief   auto switch to next song
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  support
 * @retval  AK_FALSE not support
 */
T_BOOL AudioPlayer_AutoSwitch(T_U8 endType);

/**
 * @brief   pause the song, use it when the audio data is not enough
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AudioPlayer_Suspend(T_VOID);

/**
 * @brief   resume to play the song, use it after AUDIOPLAYER_Suspend
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AudioPlayer_Resume(T_VOID);

/**
 * @brief   check the file is playing or not
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  the file is playing
 * @retval  AK_FALSE the file is not playing
 */
T_BOOL AudioPlayer_IsPlayingFile(T_pCWSTR pFilepath);

/**
 * @brief   get the type of the playing file
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  play file type
 */
T_U32  AudioPlayer_GetPlayFileType(T_VOID);

/**
 * @brief   get the pointer of filelist struct
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_FILELIST  *
 * @retval  the pointer of filelist struct when success
 */
T_FILELIST  *AudioPlayer_GetFilelist(T_VOID);

/**
 * @brief   just change the state 
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] NewState new AudioPlayer sub state
 * @return  T_BOOL
 * @retval  AK_TRUE  change state success
 * @retval  AK_FALSE change state error
 */
T_BOOL AudioPlayer_ChangeState(T_AUDIOPLAYER_STATE NewState);

/**
 * @brief   B seek to A 
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AudioPlayer_BSeekToA(T_VOID);

/**
 * @brief   get current play time of the playing audio
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  current play time
 */
T_U32 AudioPlayer_GetPlayTime(T_VOID);

/**
 * @brief   set current play time of the playing audio
 * 
 * @author  wangxuwen
 * @date    2008-08-01
 * @param   T_BOOL
 * @return  T_U32
 * @retval  current play time
 */
T_BOOL AudioPlayer_SetPlayTime(T_U32 CurTime);


/**
 * @brief   get total time of the playing audio
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  audio total time
 */
T_U32 AudioPlayer_GetTotalTime(T_VOID);

/**
 * @brief   tune the audio total time when audio play time is more than total time  
 * 
 * it is call when audio play time more than total time, call it just for displaying
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  AK_TRUE success
 * @retval  AK_FALSE error
 */
T_BOOL AudioPlayer_TuneTotalTime(T_VOID);

/**
 * @brief   set Interface refresh callback function 
 * 
 * input Interface refresh function to AudioPlayer module
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] callbackfunc callback function pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  set callback function success
 * @retval  AK_FALSE set callback function error
 */
T_BOOL AudioPlayer_SetRefreshCallback(T_fAUDIOPLAYER_REFRESH_CALLBACK callbackfunc);

/**
 * @brief   check hit point in which button callback function 
 * 
 * * @author  wangxuwen
 * @date    2008-07-31
 * @param   [in] callbackfunc callback function pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  set callback function success
 * @retval  AK_FALSE set callback function error
 */

T_BOOL AudioPlayer_SetHitButtonCallback(T_fAUDIOPLAYER_HIT_BUTTON_CALLBACK callbackfunc);

/**
 * @brief   ask A point of the playing audio is marked or not 
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  A pointe is marked
 * @retval  AK_FALSE A pointe is not marked
 */
T_BOOL AudioPlayer_IsMarkPointA(T_VOID);

/**
 * @brief   ask B point of the playing audio is marked or not 
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  B pointe is marked
 * @retval  AK_FALSE B pointe is not marked
 */
T_BOOL AudioPlayer_IsMarkPointB(T_VOID);

/**
 * @brief   AudioPlayer open a file and play
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [T_pCSTR] pFilepath file path of opened file
 * @param   [T_U32] FileId id of opened file
 * @return  T_BOOL
 * @retval  AK_TRUE  open file success
 * @retval  AK_FALSE open file error
 */
T_BOOL AudioPlayer_OpenFile(T_pWSTR pFilepath, T_U32 FileId);

/**
 * @brief	AudioPlayer stop
 * 
 * @author	xuyr
 * @date	2009-09-7
 * @param	T_VOID
 * @return	T_VOID
 */
T_VOID AudioPlayer_Stop_Step1(T_VOID);
/**
 * @brief	if in the background state, the AudioPlayer module will be free
 * 
 * @author	xuyr
 * @date	2009-09-7
 * @param	T_VOID
 * @return	T_VOID
 */
T_VOID AudioPlayer_Stop_Step2(T_VOID);
/**
 * @brief   AudioPlayer stop, if in the background state, the AudioPlayer module will be free
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AudioPlayer_Stop(T_VOID);



/**
 * @brief   start audition
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  start audition success
 * @retval  AK_FALSE start audition error
 */
T_BOOL AudioPlayer_StartAudition(T_VOID);

/**
 * @brief   stop audition
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  stop audition success
 * @retval  AK_FALSE stop audition error
 */
T_BOOL AudioPlayer_StopAudition(T_VOID);

/**
 * @brief   get current playing audio name
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_pCSTR
 * @retval  current playing audio name
 */
T_pCWSTR AudioPlayer_GetPlayAudioName(T_VOID);

/**
 * @brief   get current playing audio path
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_pCSTR
 * @retval  current playing audio path
 */
T_pCWSTR AudioPlayer_GetPlayAudioPath(T_VOID);

T_pSONG_INFO AudioPlayer_GetPlayAudioInfo(T_VOID);

T_VOID AudioPlayer_GetFocusFilePath(T_pWSTR pFilePath);

T_ICONEXPLORER *AudioPlayer_GetIconExplorer(T_VOID);

/**
 * @brief   get focus audio name
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_pCSTR
 * @retval  focus audio name
 */
T_pCWSTR AudioPlayer_GetFocusName(T_VOID);

/**
 * @brief   set fetch lyric callback function
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] callbackfunc get lyric callback function pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  set callback function success
 * @retval  AK_FALSE set callback function error
 */
T_BOOL AudioPlayer_SetFetchLyricCallback(T_fAUDIOPLAYER_GET_LYRIC_CALLBACK callbackfunc);

/**
 * @brief   get AudioPlayer suspend flag
 * 
 * At some time, the audio will be suspended, such as open some large photo
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  suspend flag
 */
T_U32 AudioPlayer_GetSuspendFlag(T_VOID);

/**
 * @brief   set change audio name callback function
 * 
 * switch audio, display audio name change at the some time
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] callbackfunc change audio name callback function pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  set callback function success
 * @retval  AK_FALSE set callback function error
 */
T_BOOL AudioPlayer_SetNameInfoCallback(T_fAUDIOPLAYER_SET_NAMEINFO_CALLBACK callbackfunc);

/**
 * @brief   chage audio name in Interface
 * 
 * call T_fAUDIOPLAYER_SET_NAMEINFO_CALLBACK callback function in this function
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AudioPlayer_ChangName(T_VOID);

T_BOOL AudioPlayer_StopPlayTimer(T_VOID);

T_AUDIOPLAYER_HANDLE_RET AudioPlayer_ForwardStateHandle(T_AUDIOPLAYER_ACT action);
T_AUDIOPLAYER_HANDLE_RET AudioPlayer_BackwardStateHandle(T_AUDIOPLAYER_ACT action);

T_VOID audio_stop_callback(T_END_TYPE endType);


#ifdef __cplusplus
}
#endif

#endif

