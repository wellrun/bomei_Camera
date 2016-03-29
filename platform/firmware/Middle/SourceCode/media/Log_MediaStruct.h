#ifndef _LOG_MEDIA_STRUCT_H_
#define _LOG_MEDIA_STRUCT_H_
/**
 * @file Log_MediaStruct.c
 * @brief Media Player Public Type & Macro for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#include "Anyka_Types.h"
#include "Hal_Timer.h"
#include "AKOS_Api.h"
#include "Fwl_osfs.h"
#include "Lib_Media_Global.h"
#include "Lib_Media_struct.h"
#include "Media_Demuxer_Lib.h"
#include "Log_MediaPlayer.h"

#define MEDIA_FAST_SWITCH
#undef  MEDIA_FAST_SWITCH

#define MPLAYER_ERROR		-1
#define FUNC_NAME_LEN		32
#define MAX_WAIT_MSG_NUM	200

#define EVT_AUDIO_ENDA 	0x2101
#define EVT_AUDIO_DISDA	0x2102
#define EVT_AUDIO_CLOSE	0x200F
#define EVT_AUDIO_SCAN	0x2010
#define EVT_AUDIO_FULL	0x2001
#define EVT_AUDIO_STOP	0x2002
#define EVT_AUDIO_START	0x2004
#define EVT_AUDIO_MP3SCAN	0x2012
#define EVT_AUDIO_MP3START	0x2014
#define EVT_AUDIO_MP3CLOSE	0x2016
#define EVT_AUDIO_MP3STOP	0x2018
#define EVT_AUDIO_FILTER	0x2022

#define EVT_VIDEO_EXIT	0x40FF
#define EVT_VIDEO_CLOSE	0x400F
#define EVT_VIDEO_SCAN	0x4010
#define EVT_VIDEO_STOP	0x4002

#define EVT_MPU_REFRESH	0x0101

#define EVT_THREAD_EXIT	0xF0FF

// Relative Thread Poll Time Interval
#define DMX_SCAN_INTERVAL	0x0028	// 40ms	3C	// 60ms 
#define AUDIO_SCAN_INTERVAL	0x001E	// 30ms
#define VIDEO_SCAN_INTERVAL	0x0019	// 25ms
#define VIDEO_FAST_INTERVAL 0x0190	// 400ms

/*audio filter num*/
#define AUDIO_FILTER_NUM	3

#define NORM_DEC_FAIL_NUM	40
#define MAX_DEC_FAIL_NUM	100


/*decode buffer*/
typedef struct{
    T_U8            *pBuf;//point to decode buf
    T_U32           bufLen;//buffer len
    T_U32           allDataLen;//all decoded data len
    T_U32           remainDataLen;//remain data len
}T_AUDIO_BUF;


typedef struct tag_AudioFilter{

	struct{
		T_pVOID	hFilt;			//!< Audio Lib Filter Handler
		T_U8	mode;			//!< EQ Mode / TEMPO Value(0.5 ~ 1.5)
	}filter[AUDIO_FILTER_NUM];	//!< [0]:eq, [1]:tempo, others:reserve
	
	T_AUDIO_BUF	filtBuf;
	T_U8 		flag;			//!< bit0: eq, bit1: tempo, others: reserve

}T_AUDIO_FILTER, *T_pAUDIO_FILTER;


typedef struct tag_AudioDecode{
	T_pVOID				  hSD;	//!< Audio Lib Decoder Handler
	T_MEDIALIB_AUDIO_INFO audioInfo;
	T_AUDIO_DECODE_OUT	  decOut;
	T_pAUDIO_FILTER 	  pSdFilt;
	T_TIMER 			  timer;
	T_U8				  decNum;	//!< Decoded Times In ONE Timer Event
	T_BOOL				  bFadeOpen;
	
}T_AUDIO_DEC, *T_pAUDIO_DEC;

typedef struct tag_Mp3AudDecode{
	T_pVOID				  hMP3;	//!< mp3 Lib Decoder Handler
	T_MEDIALIB_AUDIO_INFO audioInfo;
	T_AUDIO_DECODE_OUT	  decOut;
	T_TIMER 			  timer;
	T_U8				  decNum;	//!< Decoded Times In ONE Timer Event
	
}T_MP3AUD_DEC, *T_pMP3AUD_DEC;


typedef struct tag_Demuxer{
	T_pVOID				hMedia;		//!< Media Lib Demuxer Handler
	T_MEDIALIB_DMX_INFO dmxInfo;	//!< Media Information
	T_hFILE				hFile;		//!< File Handler / Buffer Init Address
	T_U32				startTime;	//!< Start Playing Position Per Times
	T_U32				zeroTime;	//!< System Tick, ms
	T_U32				curTime;	//!< Relative Zero, ms
	T_TIMER 			timer;		//!< Demuxer Thread Driver Timer
	T_U8 				isFile;		//!< Bit 0: Src File Flag, 1: File, 0: Buffer; 

}T_DEMUXER, *T_pDEMUXER;

typedef struct tag_VidoeDecode{
	T_pVOID				hVS;		//!< Video Stream Decoder Handler
	T_VIDEO_DECODE_OUT 	videoOut;
	T_fSHOWFRAME_CB 	showCB;		//!< Vidoe Show Callback Function
	T_TIMER 	timer;

}T_VIDEO_DEC, *T_pVIDEO_DEC;


typedef struct tag_MultiThreadMediaPlayer{
	T_pDEMUXER			pDmx;		//!< Media Demuxer Pointer
	T_pAUDIO_DEC		pAudio;		//!< Audio Decoder Pointer
	T_pVIDEO_DEC		pVideo;		//!< Video Decoder Pointer

	volatile T_eMPLAYER_STATUS	status;
	T_U8				init;

	T_fEND_CB 			endCB;		//!< End Callback Function
	T_TIMER				tmExit;

}T_MT_MPLAYER, *T_pMT_MPLAYER;

typedef struct tag_MultiThreadMP3layer{
	T_pMP3AUD_DEC		pMp3Aud;		//!< Audio Decoder Pointer

	volatile T_hFILE	hFile;
	volatile T_BOOL		decOpen;
	volatile T_eMPLAYER_STATUS	status;
	T_U8				init;

}T_MT_MP3PLAYER, *T_pMT_MP3PLAYER;

#if 0

typedef void (*T_fThreadEntryCB)(T_U32, T_pVOID);

typedef struct tag_ThreadParm{
	
	T_fThreadEntryCB fEntry;
	T_U8*			name[FUNC_NAME_LEN];
	T_U32 			argc;
	T_pVOID			argv;
	T_pVOID			stackAddr;
	T_U32 			stack_size;
	T_OPTION 		priority;
	T_U32 			time_slice;
	T_OPTION 		preempt;
	T_OPTION 		auto_start;
}T_THREAD_PARM, *T_pTHREAD_PARM;


typedef struct tag_GeneralThread{
	
	T_pTHREAD_PARM	parm;	
	T_hTask			task;
	T_hEventGroup	evGroup;

	volatile T_TIMER timer;

}T_GENRL_THREAD, *T_pGENRL_THREAD;


T_VOID DriveEvtCB_Demux(T_TIMER timer_id, T_U32 delay);
T_VOID DriveEvtCB_Audio(T_TIMER timer_id, T_U32 delay);
T_VOID DriveEvtCB_Video(T_TIMER timer_id, T_U32 delay);


T_BOOL GenrlThread_StartTimer(T_pGENRL_THREAD thread, T_U32 millSec, T_fVTIMER_CALLBACK cbFunc);

T_VOID GenrlThread_StopTimer(T_pGENRL_THREAD thread);

T_pTHREAD_PARM 	GenrlThread_Init(T_fThreadEntryCB entry, T_U8* name, T_U32 argc, T_pVOID argv, T_U8, T_U32);

T_BOOL GenrlThread_Create(T_pGENRL_THREAD thread);

T_BOOL GenrlThread_Destroy(T_pGENRL_THREAD thread);

#define GenrlThread_PostEvent(thread,event,operation) AK_Set_Events((thread)->evGroup,(event),(operation))

T_VOID GenrlThread_Exit(T_pGENRL_THREAD thread);
#endif

#endif // _LOG_MEDIA_STRUCT_H_
