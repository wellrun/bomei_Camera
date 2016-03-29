/**
 * @file Log_MediaPlayer.h
 * @brief Media Player  Interface for Multi-thread Implementation
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 * @note The following is an example to use playing APIs
 * @code

 MPlayer_Init();

 if (MPlayer_Open( filename, AK_TRUE))
 {
 	MPlayer_SetShowFrameCB(AVIPlayer_ShowFrame);
 	MPlayer_SetEndCB(AVIPlayer_EndOneFile);  
	MPlayer_Play(0);

 	... ...

 	MPlayer_GetTotalTime();

	MPlayer_GetCurTime();

	MPlayer_HasAudio();

	MPlayer_HasVideo();

	MPlayer_AllowSeek();

 	MPlayer_GetWidth();

 	MPlayer_GetHeight();

 	MPlayer_GetStatus();

 	... ...
 	
 	MPlayer_Pause();
 	
 	MPlayer_Resume();

 	MPlayer_Seek(10000);

 	... ...

 	MPlayer_Close();

 }
 */

#ifndef _LOG_MEDIA_PLAYER_H_
#define _LOG_MEDIA_PLAYER_H_

#include "anyka_types.h"
#include "akdefine.h"
#include "Gbl_Macrodef.h"
#include "Fwl_vme.h"
#include "Fwl_sysevent.h"
#include "lib_media_struct.h"
#include "lib_media_global.h"
#include "lib_sdcodec.h"


#define EVT_PLAY_START	0x8101
#define EVT_PLAY_STOP	0x8102
#define EVT_PLAY_PAUSE	0x8104
#define EVT_PLAY_RESUME	0x8108
#define EVT_PLAY_CLOSE	0x810F
#define EVT_PLAY_AB		0x8111
#define EVT_PLAY_FF		0x8121
#define EVT_PLAY_FR		0x8122


#define MIN_FRAME_WIDTH		18		//!< 2D Module Surpport MIN Frame Width
#define MIN_FRAME_HEIGHT	18		//!< 2D Module Surpport MIN Frame Height


typedef enum {
#if 0
    MEDIA_EVT_PLAY = 0,
    MEDIA_EVT_PLAY_END,
    MEDIA_EVT_SEEK,
    MEDIA_EVT_SWITCH,
    MEDIA_EVT_AUTO_STOP,
    MEDIA_EVT_USER_STOP,   
    MEDIA_EVT_RES_FREE,
    MEDIA_EVT_AB_PLAY,
#endif    
    EVT_RECAUDIO_START,
    EVT_RECAUDIO_REC,
    EVT_RECAUDIO_STOP
}T_MEDIA_EVT;


typedef enum{
    T_END_TYPE_NORMAL,
    T_END_TYPE_USER,
    T_END_TYPE_ERR,
    T_END_TYPE_EXIT,
}T_END_TYPE;

typedef enum
{
	MPLAYER_CLOSE = 0,
	MPLAYER_STOP  = 1,	
	MPLAYER_END   = 2,
	MPLAYER_PAUSE,
	MPLAYER_OPEN,	
	MPLAYER_PLAY,	
	MPLAYER_FAST,
	MPLAYER_SEEKING,
	MPLAYER_SEEKED,
	MPLAYER_WAVEOUT,
	MPLAYER_ERR,
	MPLAYER_NUM,
}T_eMPLAYER_STATUS;

typedef enum{
    SEEK_TYPE_FORWARD =0,
    SEEK_TYPE_BACKWARD=1,
    SEEK_TYPE_INVALID =0xff,
}T_SEEK_TYPE;    

/*realize buffer read/seek same as file read/seek*/
typedef struct{
    T_pCDATA pBuf;		//!< start addr of buffer
    T_U32 	bufLen;		//!< buffer total len
    T_U32 	bufPos;		//!< buffer position pointer
}T_MEDIALIB_BUFFER;

typedef enum{
	T_SRC_TYPE_BUF = 0,
    T_SRC_TYPE_PATH,
    T_SRC_TYPE_FP,
    T_SRC_TYPE_NUM,
}T_SRC_TYPE;


typedef T_VOID (*T_fSHOWFRAME_CB)(T_U8 *y, T_U8 *u, T_U8 *v, T_U16 srcW, T_U16 srcH, T_U16 oriW, T_U16 oriH);
typedef T_VOID (*T_fEND_CB)(T_END_TYPE endType);

/*
 *@brief	Set Media Lib Callback Function
 *@param	cbFunc	[in] MEDIALIB Callback Struct
 *@param	isFile	[in] Is for File Or Buffer
 *@retval	T_MEDIALIB_CB*
 */
T_MEDIALIB_CB* Dmx_SetMediaLibCB(T_MEDIALIB_CB *cbFunc, T_BOOL isFile, T_BOOL openPRN);

/**
 * @brief Query  Media Type
 *
 * @author 	Xie_Wenzhong
 * @param	ckOut		[out] Has Audio / Video ?
 * @param	src			[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_eMEDIALIB_MEDIA_TYPE
 */ 
T_eMEDIALIB_MEDIA_TYPE Media_CheckFile(T_MEDIALIB_CHECK_OUTPUT *ckOut, T_pVOID src, T_BOOL isFile);

/**
 * @brief Query  Media Whether Has Audio
 *
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_BOOL
 * @retval	AK_FALSE	NOT
 * @retval	AK_TRUE	YES
 */ 
T_BOOL Media_HasAudio(T_pVOID fname, T_BOOL isFile);

/**
 * @brief Query  Media Whether Has Video
 *
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_BOOL
 * @retval	AK_FALSE	NOT
 * @retval	AK_TRUE	YES
 */ 
T_BOOL Media_HasVideo(T_pVOID fname, T_BOOL isFile);

/**
 * @brief Query  Media Audio Encoded Type
 *
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_AUDIO_TYPE
 */ 
T_AUDIO_TYPE Media_GetAudioType(T_pVOID fname, T_BOOL isFile);

/**
 * @brief Query  Media Video Encoded Type
 *
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_eVIDEO_DRV_TYPE
 */ 
T_U8 Media_GetVideoType(T_pVOID fname, T_BOOL isFile);

/**
 * @brief Query  Media File Header Type
 *
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in]	Media Is A File Or Buffer
 * @return 	T_eMEDIALIB_MEDIA_TYPE
 */ 
T_eMEDIALIB_MEDIA_TYPE Media_GetMediaType(T_pVOID fname, T_BOOL isFile);

/**
 * @brief Get  Media File Meta Infomation
 * @note	 After Used, Must Call MediaLib_Dmx_Close(T_MEDIALIB_STRUCT) Close Media Lib Handler
 * @author 	Xie_Wenzhong
 * @param	ppMetaInfo	[out] pointer of Meta Infomation
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	type		[in]	Media Is A File Or Buffer
 * @return 	T_MEDIALIB_STRUCT 
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */ 
T_pVOID Media_GetMetaInfo(T_MEDIALIB_META_INFO *pMetaInfo, T_pVOID src, T_SRC_TYPE type);

T_VOID Media_ReleaseMetaInfo(T_pVOID hMedia);

/**
 * @brief Get  Media File Meta Picture Data
 * @note 	After Used, Must Call Media_ReleasePicMetaInfo(T_pVOID pMetapic) to Release Memory
 * @author 	Xie_Wenzhong
 * @param	fname	[in]	pointer of File Name 
 * @param	picBuf	[out] pointer of Meta Picture Data
 * @param	picLen	[out] Length of Meta Picture Data
 * @return 	T_pVOID		Parameter of Function  Media_ReleasePicMetaInfo()
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pVOID Media_GetPicMetaInfo(T_pCWSTR filename, T_U8 **picBuf, T_U32 *picLen);

/**
 * @brief Release  Media File Meta Picture Data
 *
 * @author 	Xie_Wenzhong
 * @param	pMetapic	[in]	Return Value of Function Media_GetPicMetaInfo() 
 * @return 	T_pVOID	
 */
T_VOID Media_ReleasePicMetaInfo(T_pVOID pMetapic);

T_U32 Media_GetTotalTime(T_pVOID fname, T_BOOL isFile);

T_BOOL Media_QueryInfo(T_MEDIALIB_MEDIA_INFO* mediaInfo, T_pVOID fname, T_BOOL isFile);



/*****************************************************************************/
T_VOID MPlayer_SetInitParm(T_MEDIALIB_INIT_INPUT *pInitIn, T_MEDIALIB_INIT_CB *pIinitCB);


/**
 * @brief Initialize MediaLib & Media Player Struct
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Initialized fail
 * @retval	AK_TRUE	Initialized ok
 */
T_BOOL MPlayer_Init(T_VOID);

T_pVOID MPlayer_GetPlayer(T_VOID);

T_VOID MPlayer_HandleEvent(T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);


/**
 * @brief Open Media File
 *
 * @author 	Xie_Wenzhong
 * @param	src			[in]	pointer of File Name Or Buffer Initial Address
* @param	isFile		[in] The Media Is a File Or Buffer
 * @return 	T_BOOL
 * @retval	AK_FALSE	Open Failure
 * @retval	AK_TRUE	Open Success
 */
T_BOOL MPlayer_Open(T_pVOID src, T_BOOL isFile);

/**
 * @brief Play Current Opened Media File 
 *
 * @author 	Xie_Wenzhong
 * @param	position			[in]	Played Position of Media
 * @return 	T_BOOL
 * @retval	AK_FALSE	Play Failure
 * @retval	AK_TRUE	Play Success
 */
T_BOOL MPlayer_Play(T_U32 position);

/**
 * @brief Start Playing Media After Stoped Playing 
 *
 * @author 	Xie_Wenzhong
 * @param	pos			[in]	Played Position of Media
 * @return 	T_BOOL
 * @retval	AK_FALSE	Start Failure
 * @retval	AK_TRUE	Start Success
 */
T_BOOL MPlayer_Start(T_U32 pos);

/**
 * @brief Close Current Media Player
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Close Failure
 * @retval	AK_TRUE	Close Success
 */
T_BOOL MPlayer_Close(T_VOID);

/**
 * @brief Pause Current Media Player
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Pause Failure
 * @retval	AK_TRUE	Pause Success
 */
T_BOOL MPlayer_Pause(T_VOID);

/**
 * @brief Resume Current Media Player
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Resume Failure
 * @retval	AK_TRUE	Resume Success
 */
T_BOOL MPlayer_Resume(T_VOID);

/**
 * @brief Stop Current Media Player
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Stop Failure
 * @retval	AK_TRUE	Stop Success
 */
T_BOOL MPlayer_Stop(T_VOID);

/**
 * @brief From Special Position Play
 *
 * @author 	Xie_Wenzhong
 * @param	param		[in]	Playing Position
 * @return 	T_BOOL
 * @retval	AK_FALSE	Seek Failure
 * @retval	AK_TRUE	Seek Success
 */
T_BOOL MPlayer_Seek(T_U32 param);

/**
 * @brief Set Fast Froward Play
 *
 * @author 	Xie_Wenzhong
 * @param	pos		[in]	Playing Position
 * @return 	T_BOOL
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */
T_BOOL MPlayer_FastForward(T_U32 pos);

/**
 * @brief Set Fast Rewind Play
 *
 * @author 	Xie_Wenzhong
 * @param	pos		[in]	Playing Position
 * @return 	T_BOOL
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */
T_BOOL MPlayer_FastRewind(T_U32 pos);

/**
 * @brief Set Current Media Player Show Callback Function
 *
 * @author 	Xie_Wenzhong
 * @param	pShowFrame		[in]	pointer of Show Callback Function
 * @return	T_VOID
 */
T_VOID MPlayer_SetShowFrameCB(T_fSHOWFRAME_CB pShowFrame);

/**
 * @brief Set Current Media Player End Callback Function
 *
 * @author 	Xie_Wenzhong
 * @param	endCB		[in]	pointer of End Callback Function
 * @return	T_VOID
 */
T_VOID MPlayer_SetEndCB(T_fEND_CB endCB);

/**
 * @brief Manipulate Current Media Player End Callback Function
 *
 * @author 	Xie_Wenzhong
 * @param	T_VOID
 * @return	T_eMPLAYER_STATUS
 */
T_eMPLAYER_STATUS MPlayer_GetStatus(T_VOID);

/**
 * @brief Get Current Playing Media Total Time
 *
 * @author 	Xie_Wenzhong
 * @return 	T_U32
 * @retval	0		Failure
 * @retval	Others	Success
 */
T_U32 MPlayer_GetTotalTime(T_VOID);

/**
 * @brief Get Current Playing Media Current Time
 *
 * @author 	Xie_Wenzhong
 * @return 	T_U32
 * @retval	0		Failure
 * @retval	Others	Success
 */
T_U32 MPlayer_GetCurTime(T_VOID);

/**
 * @brief Get Current Playing Media Type
 *
 * @author 	Xie_Wenzhong
 * @return 	T_eMEDIALIB_MEDIA_TYPE
  */
T_eMEDIALIB_MEDIA_TYPE MPlayer_GetMediaType(T_VOID);

/**
 * @brief Get Current Playing Media Infomation
 *
 * @author 	Xie_Wenzhong
 * @return 	T_pVOID(T_MEDIALIB_DMX_INFO*)
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pVOID MPlayer_GetMediaInfo(T_VOID);

/**
 * @brief Get Current Playing Media Meta Infomation
 *
 * @author 	Xie_Wenzhong
 * @param	 metaInfo	 [out]	Meta Information Pointer
 * @return 	T_BOOL
 * @retval	AK_TRUE	Failure
 * @retval	AK_FALSE	Success
 */
T_BOOL MPlayer_GetMetaInfo(T_MEDIALIB_META_INFO** metaInfo);

/**
 * @brief Query Current Playing Media Whether Has Audio
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	NOT
 * @retval	AK_TRUE	YES
 */ 
T_BOOL MPlayer_HasAudio(T_VOID);

/**
 * @brief Query Current Playing Media Whether Has Video
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	NOT
 * @retval	AK_TRUE	YES
 */
T_BOOL MPlayer_HasVideo(T_VOID);

/**
 * @brief Query Current Playing Media Whether Allow Seek
 *
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	NOT
 * @retval	AK_TRUE	YES
 */
T_BOOL MPlayer_AllowSeek(T_VOID);

/**
 * @brief Get Current Playing Video Width
 *
 * @author 	Xie_Wenzhong
 * @return 	T_LEN
 * @retval	-1		Failure
 * @retval	Others	Success
 */
T_LEN MPlayer_GetWidth(T_VOID);

/**
 * @brief Get Current Playing Video Height
 *
 * @author 	Xie_Wenzhong
 * @return 	T_LEN
 * @retval	-1		Failure
 * @retval	Others	Success
 */
T_LEN MPlayer_GetHeight(T_VOID);

/**
 * @brief Set Current Playing Media Track
 *
 * @author 	Xie_Wenzhong
 * @param	track		[in]	Track Number: LEFT / RIGHT / STEREO
 * @return 	T_BOOL
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */
T_BOOL MPlayer_SetTrack(T_U8 track);

/**
 * @brief Get Current Player Audio Decoder Pointer
 *
 * @author 	Xie_Wenzhong
 * @return 	T_pVOID
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pVOID MPlayer_GetAudioDecoder(T_VOID);
/**
 * @brief Get Current Player Video Decoder Pointer
 *
 * @author 	Xie_Wenzhong
 * @return 	T_pVOID
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pVOID MPlayer_GetVideoDecoder(T_VOID);

/**
 * @brief Get Appointed Farme YUV Data From Video File
 *
 * @date 	July 5, 2011
 * @author 	Xie_Wenzhong
 * @param	fname		[in]	pointer of File Name Or Buffer Initial Address
 * @param	isFile		[in] The Media Is a File Or Buffer
 * @param	pBuf		[out] YUV Frame Data
 * @param	width		[in] YUV Frame Width
 * @param	height		[in] YUV Frame Height 
 * @param	pos			[in] Appointed Farme (millisecond)
 * @return 	T_BOOL
 * @retval	AK_FALSE	Get YUV Failure
 * @retval	AK_TRUE	Get YUV Success
 */
T_BOOL MPlayer_GetFrameYuv(T_pVOID fname, T_BOOL isFile, T_pDATA pBuf, T_LEN width, T_LEN height, T_S32 pos);

T_BOOL MP3Player_Init(T_VOID);

/**
 * @brief Open Mp3 audio decoder
 *
 * @author 	heyuanlong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Open Failure
 * @retval	AK_TRUE	Open Success
 */
T_BOOL MP3Player_OpenDec(T_VOID);

/**
 * @brief Open Mp3 audio buf
 *
 * @author 	heyuanlong
 * @param	src			[in]	pointer of Gam bgm Buffer Initial Address
 * @return 	T_BOOL
 * @retval	AK_FALSE	Open Failure
 * @retval	AK_TRUE	Open Success
 */
T_BOOL MP3MPlayer_Open(T_pVOID src);

/**
 * @brief Play Current Opened Mp3 BGM buffer
 *
 * @author 	heyuanlong
 * @param	position			[in]	Played Position of Media
 * @return 	T_BOOL
 * @retval	AK_FALSE	Open Failure
 * @retval	AK_TRUE	Open Success
 */
T_BOOL MP3Player_Play(T_U32 position);

/**
 * @brief Close Current Mp3 Player
 *
 * @author 	heyuanlong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Close Failure
 * @retval	AK_TRUE	Close Success
 */
T_BOOL MP3Player_Close(T_VOID);

#endif	// _LOG_MEDIA_PLAYER_H_

/*
 * End of File
 */
