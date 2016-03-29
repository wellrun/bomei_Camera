/**
 * @file video_stream_lib.h
 * @brief This file provides H263/MPEG4/FLV263/H264/RV/MPEG2 decoding and H263/MPEG4 encoding functions
 *
 * Copyright (C) 2014 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su Dan
 * @date 2008-2-4
 * @update date 2014-06-13
 * @version 0.8.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use H263/MPEG4/FLV263/H264/RV/MPEG2 decoding APIs
   @code

	解码部分1：使用fifo进行同步解码(open_input.m_ulBufSize不能为0)
	T_pVOID g_hVS;
	T_VIDEOLIB_OPEN_INPUT open_input;
	T_S32 audio_timestamp;
	T_S32 video_timestamp;
	T_VIDEO_DECODE_OUT VideoDecOut;

	memset(&open_input, 0, sizeof(T_VIDEOLIB_OPEN_INPUT));

	open_input.m_VideoType = VIDEO_DRV_MPEG;
	open_input.m_ulBufSize = 200*1024;
	open_input.m_bNeedSYN = AK_TRUE;
	open_input.m_bFixedStream = 1;
	open_input.m_uWidth = media_info.m_uWidth;
	open_input.m_uHeight = media_info.m_uHeight;

	open_input.m_CBFunc.m_FunPrintf = printf;
	open_input.m_CBFunc.m_FunMalloc = malloc;
	open_input.m_CBFunc.m_FunFree = free;
	open_input.m_CBFunc.m_FunMMUInvalidateDCache = MMU_InvalidateDCache;
	open_input.m_CBFunc.m_FunCleanInvalidateDcache = MMU_CleanInvalidateDcache;
	open_input.m_CBFunc.m_FunCheckDecBuf = CheckDecBuf;

	open_input.m_CBFunc.m_FunMapAddr = VD_EXfunc_MapAddr;
	open_input.m_CBFunc.m_FunUnmapAddr = VD_EXfunc_UnmapAddr;
	open_input.m_CBFunc.m_FunDMAFree = dma_free;
	open_input.m_CBFunc.m_FunDMAMalloc = dma_malloc;
	open_input.m_CBFunc.m_FunVaddrToPaddr = VD_EXfunc_VaddrToPaddr;
	open_input.m_CBFunc.m_FunRegBitsWrite =	VD_EXfunc_RegBitsWrite;

	open_input.m_CBFunc.m_FunVideoHWLock = VD_EXfunc_VideoHWLock;
	open_input.m_CBFunc.m_FunVideoHWUnlock = VD_EXfunc_VideoHWUnlock;

	g_hVS = VideoStream_Open(&open_input);
	if (AK_NULL == g_hVS)
	{
		return;
	}

	while(1)
	{
		if (pause_flag) //暂停后需要重新缓冲数据
		{
			if (VideoStream_Reset(g_hVS) == AK_FALSE) 
			{
				break;
			}
		}
		audio_timestamp = get_audio_timestap();	//由音频库实现
		video_timestamp = VideoStream_SYNDecode(g_hVS, &VideoDecOut, audio_timestamp);
		if (VideoDecOut.m_pBuffer != AK_NULL)
		{
			Display(VideoDecOut.m_pBuffer, 
			VideoDecOut.m_uDispWidth, 
			VideoDecOut.m_uDispHeight);
		}
		if (stop_flag)//检测到停止标志后退出while循环
		{
			break;
		}
	}
	VideoStream_Close(g_hVS);


	数据处理部分（即之前提到的回调函数）：
	T_BOOL video_call_back(DWORD timestamp_ms, void *data_pointer, DWORD length)
	{
		T_pDATA video_buf = AK_NULL;
		video_buf = VideoStream_GetAddr(g_hVS, length + 8);	//预留前8字节，因此要多加8
		if (AK_NULL == video_buf)
		{
			return AK_FALSE;
		}

		memcpy(video_buf + 4, &timestamp_ms, 4);//将timestamp放入5-8字节，1-4字节保留
		memcpy(video_buf + 8, data_pointer, length);//将视频数据拷入buffer

		return VideoStream_UpdateAddr(g_hVS, video_buf, length + 8);//更新视频数据
	}

*************************************************************

	//解码部分2：不使用fifo，输入一帧解码一帧(open_input.m_ulBufSize必须为0)
	T_pVOID g_hVS;
	T_VIDEOLIB_OPEN_INPUT open_input;
	T_VIDEO_DECODE_OUT VideoDecOut;
	T_pVOID pVlcBuf;
	T_U32 vlc_len;
	T_S32 retval;

	memset(&open_input, 0, sizeof(T_VIDEOLIB_OPEN_INPUT));

	open_input.m_VideoType = VIDEO_DRV_MPEG;
	open_input.m_ulBufSize = 0;
	open_input.m_bNeedSYN = AK_FALSE;
	open_input.m_bFixedStream = 1;
	open_input.m_uWidth = media_info.m_uWidth;
	open_input.m_uHeight = media_info.m_uHeight;

	open_input.m_CBFunc.m_FunPrintf = printf;
	open_input.m_CBFunc.m_FunMalloc = malloc;
	open_input.m_CBFunc.m_FunFree = free;
	open_input.m_CBFunc.m_FunMMUInvalidateDCache = MMU_InvalidateDCache;
	open_input.m_CBFunc.m_FunCleanInvalidateDcache = MMU_CleanInvalidateDcache;
	open_input.m_CBFunc.m_FunCheckDecBuf = CheckDecBuf;

	open_input.m_CBFunc.m_FunMapAddr = VD_EXfunc_MapAddr;
	open_input.m_CBFunc.m_FunUnmapAddr = VD_EXfunc_UnmapAddr;
	open_input.m_CBFunc.m_FunDMAFree = dma_free;
	open_input.m_CBFunc.m_FunDMAMalloc = dma_malloc;
	open_input.m_CBFunc.m_FunVaddrToPaddr = VD_EXfunc_VaddrToPaddr;
	open_input.m_CBFunc.m_FunRegBitsWrite =	VD_EXfunc_RegBitsWrite;

	open_input.m_CBFunc.m_FunVideoHWLock = VD_EXfunc_VideoHWLock;
	open_input.m_CBFunc.m_FunVideoHWUnlock = VD_EXfunc_VideoHWUnlock;

	g_hVS = VideoStream_Open(&open_input);
	if (AK_NULL == g_hVS)
	{
		return;
	}

	while(1)
	{
		pVlcBuf = get_vlc_data();
		vlc_len = get_vlc_data_len();	//获取码流数据
		retval = VideoStream_Decode(g_hVS, pVlcBuf, vlc_len, &VideoDecOut);
		if (VideoDecOut.m_pBuffer != AK_NULL)
		{
			Display(VideoDecOut.m_pBuffer, 
		VideoDecOut.m_uDispWidth, 
		VideoDecOut.m_uDispHeight);
		}
		if (stop_flag)//检测到停止标志后退出while循环
		{
			break;
		}
	}
	VideoStream_Close(g_hVS);


	@endcode

	************************************************************

 * @note
	The following is an example to use H263/MPEG4 encoding APIs
   @code

	编码部分：
	T_pVOID g_hVS;
	T_VIDEOLIB_ENC_OPEN_INPUT open_input;
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param;
	
	T_U8 curr_buf[PIC_SIZE];
	T_U8 vlc_buf[STREAM_MAX_SIZE];
	T_U8 *out_pic = AK_NULL;
	T_U32 lStreamLen = 0;
	
	open_input.m_VideoType = VIDEO_DRV_MPEG;
	open_input.m_ulWidth = 352;
	open_input.m_ulHeight = 288;
	open_input.m_ulMaxVideoSize = (((open_input.m_ulWidth*open_input.m_ulHeight>>1)+511)/512)*512;
	
	open_input.m_CBFunc.m_FunPrintf = printf;
	open_input.m_CBFunc.m_FunMalloc = malloc;
	open_input.m_CBFunc.m_FunFree = free;
	open_input.m_CBFunc.m_FunMMUInvalidateDCache = MMU_InvalidateDCache;
	open_input.m_CBFunc.m_FunCleanInvalidateDcache = MMU_CleanInvalidateDcache;
	open_input.m_CBFunc.m_FunCheckDecBuf = CheckDecBuf;

	open_input.m_CBFunc.m_FunMapAddr = VD_EXfunc_MapAddr;
	open_input.m_CBFunc.m_FunUnmapAddr = VD_EXfunc_UnmapAddr;
	open_input.m_CBFunc.m_FunDMAFree = dma_free;
	open_input.m_CBFunc.m_FunDMAMalloc = dma_malloc;
	open_input.m_CBFunc.m_FunVaddrToPaddr = VD_EXfunc_VaddrToPaddr;
	open_input.m_CBFunc.m_FunRegBitsWrite =	VD_EXfunc_RegBitsWrite;

	open_input.m_CBFunc.m_FunVideoHWLock = VD_EXfunc_VideoHWLock;
	open_input.m_CBFunc.m_FunVideoHWUnlock = VD_EXfunc_VideoHWUnlock;
	
	g_hVS = VideoStream_Enc_Open(&open_input);
	if (AK_NULL == g_hVS)
	{
		return;
	}

	video_enc_io_param.p_curr_data = curr_buf;	//输入要编码的图像地址
	video_enc_io_param.p_vlc_data = vlc_buf;	//编码后的码流地址
	video_enc_io_param.QP = 10;					//编码的QP值大小
	video_enc_io_param.mode = 0;				//编码模式，I帧为0，P帧为1
	video_enc_io_param.bInsertP = AK_FALSE;		//是否需要插帧，0为不要，1为要

	while (1)
	{
		lStreamLen = VideoStream_Enc_Encode(g_hVS, &video_enc_io_param);
		out_pic = VideoStream_Enc_GetDispData(g_hVS);
		if (out_pic != AK_NULL)
		{
			Display(out_pic, open_input.m_ulWidth, open_input.m_ulHeight);
		}
		video_enc_io_param.mode = 1;	//根据需要改变mode、QP、bInsertP等
		
		if (stop_flag)//检测到停止标志后退出while循环
		{
			break;
		}
	}

	VideoStream_Enc_Close(g_hVS);

	@endcode

***************************************************/

#ifndef _VIDEO_STREAM_CODEC_H_
#define _VIDEO_STREAM_CODEC_H_

#include "Lib_media_global.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_LIB_VERSION		"Video Lib V1.10.01"

typedef enum
{
	VIDEO_DRV_UNKNOWN = 0,
	VIDEO_DRV_H263,
	VIDEO_DRV_MPEG,
	VIDEO_DRV_FLV263,
	VIDEO_DRV_H264,
	VIDEO_DRV_RV,
	VIDEO_DRV_AVC1,
	VIDEO_DRV_MJPEG,
	VIDEO_DRV_MPEG2,
	VIDEO_DRV_H264DMX
}T_eVIDEO_DRV_TYPE;

//for mutex
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_MUTEX_CREATE)(void);
/*
	nTimeOut [input]	-1		: the function's time-out interval never elapses
						0		: the function returns immediately
						other	: specifies the time-out interval, in milliseconds
	return: 1-ok; 0-failed or timeout
*/
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_MUTEX_LOCK)(T_pVOID pMutex, T_S32 nTimeOut);
/*
	return: 1-ok; 0-failed
*/
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_MUTEX_UNLOCK)(T_pVOID pMutex);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_MUTEX_RELEASE)(T_pVOID pMutex);

typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF				m_FunPrintf;

	MEDIALIB_CALLBACK_FUN_MALLOC				m_FunMalloc;
	MEDIALIB_CALLBACK_FUN_FREE					m_FunFree;

	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunMMUInvalidateDCache;
	MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE	m_FunCleanInvalidateDcache;
	MEDIALIB_CALLBACK_FUN_CHECK_DEC_BUF			m_FunCheckDecBuf;
	MEDIALIB_CALLBACK_FUN_RTC_DELAY				m_FunRtcDelay;

	//add for Linux and CE
	MEDIALIB_CALLBACK_FUN_DMA_MALLOC			m_FunDMAMalloc;
	MEDIALIB_CALLBACK_FUN_DMA_FREE				m_FunDMAFree;
	MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR		m_FunVaddrToPaddr;
	MEDIALIB_CALLBACK_FUN_MAP_ADDR				m_FunMapAddr;
	MEDIALIB_CALLBACK_FUN_UNMAP_ADDR			m_FunUnmapAddr;
	MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE		m_FunRegBitsWrite;

	//add for hardware mutex
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK			m_FunVideoHWLock;
	MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK		m_FunVideoHWUnlock;

	//add for using api about fifo in multithread
	MEDIALIB_CALLBACK_FUN_MUTEX_CREATE			m_FunMutexCreate;
	MEDIALIB_CALLBACK_FUN_MUTEX_LOCK			m_FunMutexLock;
	MEDIALIB_CALLBACK_FUN_MUTEX_UNLOCK			m_FunMutexUnlock;
	MEDIALIB_CALLBACK_FUN_MUTEX_RELEASE			m_FunMutexRelease;
}T_VIDEOLIB_CB;

typedef struct
{
	T_eVIDEO_DRV_TYPE	m_VideoType;
	T_U32				m_ulBufSize;	//if 0 == m_ulBufSize, use VideoStream_Decode to decode;
	T_BOOL				m_bNeedSYN;
	T_BOOL				m_bFixedStream;	//if true, video stream from media file or buffer, else from such as cmmb
	T_VIDEOLIB_CB		m_CBFunc;
	T_eMEDIALIB_ROTATE	m_Rotate;

	T_U16				m_uWidth;
	T_U16				m_uHeight;
}T_VIDEOLIB_OPEN_INPUT;

typedef enum
{
	VIDEO_STREAM_OK,
	VIDEO_STREAM_SYS_ERR,	//such as malloc failed
	VIDEO_STREAM_IDR_ERR,	//idr frame not found
	VIDEO_STREAM_SYN_FAST,	//video time is too fast
	VIDEO_STREAM_CODEC_ERR,	//maybe data error or codec error
	VIDEO_STREAM_SYN_ERR,	//audio and video pts is out of range, should be error
	VIDEO_STREAM_NODATA_ERR	//no stream in fifo
}T_eVIDEOLIB_ERROR;

typedef struct
{
	T_U32	m_Width;
	T_U32	m_Height;
	T_U32	m_OriWidth;
	T_U32	m_OriHeight;
}T_VIDEOLIB_INFO;

/**
 * @brief Get video codec library version
 *
 * @author Su_Dan
 * @return const T_CHR *
 * @retval	version string
 */
const T_CHR *VideoLib_GetVersion(T_VOID);


/*
 * @brief Initial Video codec library and allocate global resource
 *
 * @author Su_Dan
 * @param	init_input	[in]	pointer of T_MEDIALIB_INIT_INPUT struct
 * @param	init_cb_fun	[in]	pointer of T_MEDIALIB_INIT_CB struct for callback func
 * @return T_BOOL
 * @retval	AK_TRUE		Initial ok
 * @retval	AK_FALSE	Initial fail
 */
T_BOOL VideoStream_Init(const T_MEDIALIB_INIT_INPUT *init_input, const T_MEDIALIB_INIT_CB *init_cb_fun);


/**
 * @brief Destroy Video codec library and free global resource
 *
 * @author Su_Dan
 * @return T_VOID
 */
T_VOID VideoStream_Destroy(T_VOID);


/**
 * @brief Initial Videostream and allocate resource
 *
 * @author Su_Dan
 * @param	open_input		[in]	pointer of T_VIDEOLIB_OPEN_INPUT
 * @return T_pVOID
 * @retval 	handle of videostream
 */
T_pVOID VideoStream_Open(T_VIDEOLIB_OPEN_INPUT *open_input);


/**
 * @brief Close Videostream
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL VideoStream_Close(T_pVOID hVS);


/**
 * @brief Get Videostream error
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_eVIDEOLIB_ERROR
 * @retval	see T_eVIDEOLIB_ERROR struct
 */
T_eVIDEOLIB_ERROR VideoStream_GetLastError(T_pVOID hVS);


/**
 * @brief Reset Videostream
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_BOOL
 * @retval	AK_TRUE		Reset ok
 * @retval	AK_FALSE	Reset fail
 */
T_BOOL VideoStream_Reset(T_pVOID hVS);


/**
 * @brief Resume Videostream, call it after pause or jpeg dec
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_BOOL
 * @retval	AK_TRUE		Resume ok
 * @retval	AK_FALSE	Resume fail
 */
T_BOOL VideoStream_Resume(T_pVOID hVS);


/**
 * @brief Get address to fill video data
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @param	size	[in]	video data size
 * @return T_pVOID
 * @retval	AK_NULL		Get fail
 * @retval	other		valid pointer
 */
T_pVOID VideoStream_GetAddr(T_pVOID hVS, T_U32 size);


/**
 * @brief Update address
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @param	pInaddr	[in]	pointer returned by VideoStream_GetAddr
 * @param	size	[in]	video data size, must equal to input of VideoStream_GetAddr
 * @return T_BOOL
 * @retval	AK_TRUE		Update ok
 * @retval	AK_FALSE	Update fail
 */
T_BOOL VideoStream_UpdateAddr(T_pVOID hVS, T_pVOID pInaddr, T_U32 size);


/**
 * @brief Get the size of free space
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_U32
 * @retval	0		no free space
 * @retval	other	free space in video buffer
 */
T_U32 VideoStream_GetFreeSpace(T_pVOID hVS);


/**
 * @brief Get the number of video frames in space
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_S32
 * @retval	<0		get failed
 * @retval	other	number of video frames in space
 */
T_S32 VideoStream_GetItemNum(T_pVOID hVS);


/**
 * @brief Decode header of stream while syn decoder mode
 *
 * @author Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return	T_S32
 * @retval	<=0			decode error
 * @retval	1			decode ok
 * @retval	2			unknown header
 */
T_S32 VideoStream_SYNDecodeHeader(T_pVOID hVS);


/**
 * @brief Handle decoding
 *
 * @author Su_Dan
 * @param	hVS				[in]	pointer returned by VideoStream_Open
 * @param	pVideoDecOut	[in]	pointer of T_VIDEO_DECODE_OUT struct
 * @param	ulMilliSec		[in]	play time of system or audio
 * @return T_S32
 * @retval	< 0		decode fail
 * @retval	other	video time in millisecond
 */
T_S32 VideoStream_SYNDecode(T_pVOID hVS, T_VIDEO_DECODE_OUT *pVideoDecOut, T_S32 ulMilliSec);


/**
 * @brief Close Videostream
 *
 * @author Su_Dan
 * @param	hVS			[in]	pointer returned by VideoStream_Open
 * @param	vlc_data	[in]	encoded data pointer
 * @param	vlc_len		[in]	encoded data length
 * @return	T_S32
 * @retval	<=0			decode error
 * @retval	1			decode ok
 * @retval	2			unknown header
 */
T_S32 VideoStream_DecodeHeader(T_pVOID hVS, T_pDATA vlc_data, T_U32 vlc_len);


/**
 * @brief Decode one frame
 *
 * @author Su_Dan
 * @param	hVS				[in]	pointer returned by VideoStream_Open
 * @param	pStreamBuf		[in]	stream buffer
 * @param	ulStreamLen		[in]	stream length
 * @param	pVideoDecOut	[in]	pointer of T_VIDEO_DECODE_OUT struct
 * @return T_S32
 * @retval	< 0		decode fail
 * @retval	other	decode ok
 */
T_S32 VideoStream_Decode(T_pVOID hVS, T_pVOID pStreamBuf, T_U32 ulStreamLen, T_VIDEO_DECODE_OUT *pVideoDecOut);


/*
 * @brief Change decode speed
 *
 * @author	Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @param	nStep	[in]	step to change, range [-10, 10]
 * @return	T_S32
 * @retval	< 0		error
 * @retval	other	speed value
 */
T_S32 VideoStream_DecodeChangeSpeed(T_pVOID hVS, T_S8 nStep);


/*
 * @brief Get video pts
 *
 * @author	Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return	T_S32
 * @retval	< 0		error
 * @retval	other	video pts
 */
T_S32 VideoStream_DecodeGetPTS(T_pVOID hVS);


/*
 * @brief Get video decoder info
 *
 * @author	Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @param	pInfo	[in]	pointer of T_VIDEOLIB_INFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		get info ok
 * @retval	AK_FALSE	get info fail
 */
T_BOOL VideoStream_DecodeGetInfo(T_pVOID hVS, T_VIDEOLIB_INFO *pInfo);


/*
 * @brief Get video decoder timestamp
 *
 * @author	Su_Dan
 * @param	hVS		[in]	pointer returned by VideoStream_Open
 * @return T_S32
 * @return T_S32
 * @retval	< 0		get fail
 * @retval	other	video time in millisecond
 */
T_S32 VideoStream_GetNextPts(T_pVOID hVS);


/**
 * @brief Handle special decoding while only keyframes in FIFO
 *
 * @author Su_Dan
 * @param	hVS				[in]	pointer returned by VideoStream_Open
 * @param	pVideoDecOut	[in]	pointer of T_VIDEO_DECODE_OUT struct
 * @param	bDrop			[in]	drop one frame data
 * @return T_S32
 * @retval	< 0		decode fail
 * @retval	other	video time in millisecond
 */
T_S32 VideoStream_SpecialDecode(T_pVOID hVS, T_VIDEO_DECODE_OUT *pVideoDecOut, T_BOOL bDrop);


/**
 * @brief Set rotate mode for decoding, only valid for h263/mpeg4sp/flv263
 *
 * @author Su_Dan
 * @param	hVS			[in]	pointer returned by VideoStream_Open
 * @param	rotate		[in]	rotate mode, see T_eMEDIALIB_ROTATE
 * @return T_BOOL
 * @retval	AK_TRUE		set ok
 * @retval	AK_FALSE	set fail
 */
T_BOOL VideoStream_SetRotate(T_pVOID hVS, T_eMEDIALIB_ROTATE rotate);


/***********************   encoder  **************************/
typedef struct
{
	T_eVIDEO_DRV_TYPE	m_VideoType;
	T_U32				m_ulWidth;			//宽度
	T_U32				m_ulHeight;			//高度
	T_U32				m_ulMaxVideoSize;	//存放码流的buffer空间大小，不可小于Width*Height/2，最好是512的整数倍
	T_VIDEOLIB_CB		m_CBFunc;
}T_VIDEOLIB_ENC_OPEN_INPUT;

typedef struct
{
	T_pDATA		p_curr_data;				//当前需要编码的YUV图像首地址
	T_pDATA		p_vlc_data;					//存储编码输出码流的首地址
	T_S32		QP;							//编码QP设置，如果是码率控制模式，则QP无效
	T_U8		mode;						//编码类型设置，0为I帧，1为P帧
	T_BOOL		bInsertP;					//是否是编码一帧插帧数据，只对mode为1有效
}T_VIDEOLIB_ENC_IO_PAR;

typedef struct
{
	T_U8	*m_pTimestampY;
	T_U8	*m_pTimestampU;
	T_U8	*m_pTimestampV;

	T_U32	m_ulOffsetX;
	T_U32	m_ulOffsetY;
	T_U32	m_ulWidth;
	T_U32	m_ulHeight;
}T_VIDEOLIB_ENC_TIMESTAMP_PAR;

typedef struct
{
	T_U32	m_nvbps;			//bits per second, zero to disable rate control
	T_U16	m_nFPS;				//frame rate

	T_U16	m_nAdjustInterval;	//how many frames should we adjust the QP for encode
	T_U8	m_nMinQP;			//[1,31], and no more than m_nMaxQP
	T_U8	m_nMaxQP;			//[1,31], and no less than m_nMinQP
}T_VIDEOLIB_ENC_RC_PAR;

/*
 * @brief Initial encoder to encode H263
 *
 * @author	Zhao_Xing
 * @param	open_input	[in]	pointer of T_VIDEOLIB_ENC_OPEN_INPUT
 * @return 	T_pVOID
 * @retval	AK_NULL		open failed
 * @retval	other		handle of videostream
 */
T_pVOID VideoStream_Enc_Open(const T_VIDEOLIB_ENC_OPEN_INPUT *open_input);

/*
 * @brief close video encoder
 *
 * @author 	Zhao_Xing
 * @param	hVideo		[in]	pointer returned by VideoStream_Enc_Open
 * @return 	T_VOID
 */
T_VOID VideoStream_Enc_Close(T_pVOID hVS);

/*
 * @brief Encode one H263 frame
 *
 * @author	Zhao_Xing
 * @param	hVideo				[in]		pointer returned by VideoStream_Enc_Open
 * @param	video_enc_io_param	[in/out]	pointer of T_VIDEOLIB_ENC_IO_PAR
 * @return 	T_S32
 * @retval	< 0		fail
 * @retval	other	encoded stream length
 */
T_S32 VideoStream_Enc_Encode(T_pVOID hVS, T_VIDEOLIB_ENC_IO_PAR *video_enc_io_param);


/*
 * @brief get the recon picture
 *
 * @author 	Zhao_Xing
 * @param	hVideo		[in]	pointer returned by VideoStream_Enc_Open
 * @return 	T_pDATA
 * @retval	AK_NULL		get failed
 * @retval	other		pointer to the YUV recon data encoded
 */
T_pDATA VideoStream_Enc_GetDispData(T_pVOID hVS);

/*
 * @brief set the timestamp parameters
 *
 * @author 	Xia_Jiaquan
 * @param	hVS				[in]	pointer returned by VideoStream_Enc_Open
 * @param	timestamp_param	[in]	pointer of T_VIDEOLIB_ENC_TIMESTAMP_PAR
 * @return 	T_S32
 * @retval	-1		set timestamp failed
 * @retval	0		set timestamp ok
 */
T_S32 VideoStream_Enc_SetTimeStamp(T_pVOID hVS, T_VIDEOLIB_ENC_TIMESTAMP_PAR *timestamp_param);

/**
 * @brief Set video bitrate
 *
 * @author 	Xia_Jiaquan
 * @param	hVS			[in]	pointer returned by VideoStream_Enc_Open
 * @param	rc_param	[in]	pointer of T_VIDEOLIB_ENC_RC_PAR for rate control
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL VideoStream_Enc_SetBitrate(T_pVOID hVS, T_VIDEOLIB_ENC_RC_PAR *rc_param);

/**
 * @brief Reset Encoder Param
 * @author 	Xia_Jiaquan
 * @param	hVS			[in]	pointer returned by VideoStream_Enc_Open
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL VideoStream_Enc_Reset(T_pVOID hVS);

#ifdef __cplusplus
}
#endif

#endif//_VIDEO_STREAM_CODEC_H_
