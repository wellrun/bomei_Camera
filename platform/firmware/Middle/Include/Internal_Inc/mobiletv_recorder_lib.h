/**
 * @file mobiletv_recorder_lib.h
 * @brief This file provides AVI recording for mobile tv functions(such as cmmb avi record)
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Hu Jing, Su Dan
 * @date 2009-6-10
 * @update date 2012-1-17
 * @version 0.1.2
 * @version ever green group version: x.x
 * @note
	The following is an example to use recording APIs
   @code
T_VOID record_mobile_tv(char* filaname);
T_VOID main(int argc, char* argv[])
{
	T_MEDIALIB_INIT_CB init_cb;
	T_MEDIALIB_INIT_INPUT init_input;

	init();	// initial file system, memory, camera, lcd and etc.

	init_cb_func_init(&init_cb);	//initial callback function pointer

	init_input.m_ChipType = MEDIALIB_CHIP_AK8801;
	init_input.m_AudioI2S = I2S_UNUSE;

	if (MediaLib_Init(&init_input, &init_cb) == AK_FALSE)
	{
		return;
	}
	//above only call one time when system start

	//play film or music
	record_mobile_tv(argv[1]);

	//below only call one time when system close
	MediaLib_Destroy();
	return;
}

T_VOID record_mobile_tv(char *filename)
{
	T_S32 fid;

	T_MOBILETV_REC_OPEN_PARA media_rec_open_Para;

	T_eMEDIALIB_REC_STATUS rec_status;
	char press_key;
	T_U8 tmp_buf[1600];
	T_U8 video_buf[PIC_SIZE];
	T_MOBILETV_REC_INFO RecInfo;
	T_VOID *hMedia;
	T_S32 pts;
	T_U32 audio_tytes = 0;
	T_U32 video_tytes = 0;

	fid = FileOpen(filename);
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return;
	}

	memset(&rec_open_input, 0, sizeof(T_MEDIALIB_REC_OPEN_INPUT));

	media_rec_open_Para.mediaRecType = MEDIALIB_REC_AVI_NORMAL;

	media_rec_open_Para.hMediaDest = fid;
	media_rec_open_Para.bCaptureAudio = 1;
	media_rec_open_Para.bIdxInMem = 1;
	media_rec_open_Para.IndexMemSize = 0;

	// set video open info
	media_rec_open_Para.fps = 25;
	media_rec_open_Para.width = 320;
	media_rec_open_Para.height = 240;
	media_rec_open_Para.video_type = MEDIALIB_VIDEO_H264;//set mjpeg encode function
	
	// set audio open info
	media_rec_open_Para.bitsPerSample = 16;
	media_rec_open_Para.channels = 2;
	media_rec_open_Para.nSampleRate = 24000;
	media_rec_open_Para.timeScale = 1000;
	
	open_cb_func_init(&(media_rec_open_Para.m_CBFunc));	//initial callback function pointer;
	
	hMedia = MobileTV_Rec_Open(&media_rec_open_Para);
	if (AK_NULL == hMedia)
	{
		printf("##MOVIE: MobileTV_Rec_Open Return NULL\r\n");
		FileClose(fid);
		return;
	}

	if (AK_FALSE == MobileTV_Rec_Start(hMedia))
	{
		MobileTV_Rec_Close(hMedia);
		FileClose(fid);
		return;
	}

	while(1)
	{
		if (media_rec_open_Para.bCaptureAudio)
		{
			audio_tytes = get_audio_data(tmp_buf, &pts);//get audio data from audio encoder
			if (audio_tytes ! = 0)
			{
				if(MobileTV_Rec_AddAudioData(hMedia, (T_U8 *)tmp_buf, audio_tytes, pts) == AK_FALSE)
				{
					printf("MobileTV_Rec_AddAudioData error\r\n");
					break;
				}
			}
		}
		tickcount = get_system_time_ms();//get current time in ms from starting recording
		video_tytes = get_yuv_data(video_buf, &pts);
		if (video_tytes != 0)
		{
			video_time = MobileTV_Rec_AddVideoData(hMedia, video_buf, video_tytes, pts);
			if (video_time < 0)
			{
				printf("MobileTV_Rec_AddVideoData error\r\n");
				break;
			}
		}

		press_key = is_stop_button();//check whether stop
		if (press_key)
		{
			break;
		}
		rec_status = MobileTV_Rec_GetInfo(hMedia, &RecInfo);
		if (MOBILETV_REC_DOING != RecInfo.record_status)
		{
			break;
		}
	}

	MobileTV_Rec_Stop(hMedia);
	MobileTV_Rec_Close(hMedia);

	FileClose(fid);

	return;
}
	@endcode

 ***************************************************/

#ifndef _MOBILETV_RECORDER_LIB_H_
#define _MOBILETV_RECORDER_LIB_H_

#include "lib_media_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _MobileTV_Rec_Status
{
	MOBILETV_REC_OPEN,
	MOBILETV_REC_STOP,
	MOBILETV_REC_DOING,
	MOBILETV_REC_SYSERR,
	MOBILETV_REC_MEMFULL,
	MOBILETV_REC_WAITING
}T_eMOBILETV_REC_STATUS;

typedef struct _T_MobileTV_REC_OPEN
{
	T_BOOL	bCaptureAudio;
	T_U16	width;
	T_U16	height;
	T_U16	fps;
	T_U32	nSampleRate;
	T_U16	channels;
	T_U16	bitsPerSample;
	T_U16	reserved;
	T_S32	hMediaDest;
	T_S32	hIndexFile;
	T_U32	timeScale;

	T_eMEDIALIB_VIDEO_CODE	video_type;
	T_eMEDIALIB_AUDIO_CODE	audio_type;
	T_eMEDIALIB_REC_TYPE	mediaRecType;

	T_BOOL	bIdxInMem;		//flag indicating Index is saved in memory
	T_U32	IndexMemSize;	//index size set by system

	T_MEDIALIB_CB	m_CBFunc;
} T_MOBILETV_REC_OPEN_PARA;

typedef struct
{
	//dynamic
	T_eMOBILETV_REC_STATUS record_status;
	T_U32	total_frames;		//total frames, include video frames and audio packets
	T_U32	total_video_frames;	//total video frames
	T_U32	info_bytes;			//expect free space, to save header or index
	T_U32	file_bytes;			//current file size
}T_MOBILETV_REC_INFO;

/**
 * @brief Open a resource
 *
 * @author Hu_Jing
 * @param	media_rec_open_Para		[in]	pointer of T_MOBILETV_REC_OPEN_PARA struct
 * @return T_pVOID
 * @retval	AK_NULL		open failed
 * @retval	other		open ok
 */
T_pVOID MobileTV_Rec_Open(T_MOBILETV_REC_OPEN_PARA *media_rec_open_Para);

/**
 * @brief Close recorder
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @return T_U32
 * @retval	1		Close ok
 * @retval	0		Close fail
 */
T_U32 MobileTV_Rec_Close(T_pVOID pRecorder);

/**
 * @brief Start recording
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @return T_eMOBILETV_REC_STATUS
 */
T_eMOBILETV_REC_STATUS MobileTV_Rec_Start(T_pVOID pRecorder);

/**
 * @brief Stop recording
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @return T_eMOBILETV_REC_STATUS
 */
T_eMOBILETV_REC_STATUS MobileTV_Rec_Stop(T_pVOID pRecorder);

/**
 * @brief process audio data
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @return T_eMOBILETV_REC_STATUS
 */
T_eMOBILETV_REC_STATUS MobileTV_Rec_Handle(T_pVOID pRecorder);

/**
 * @brief add audio data
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @param	addr			[in]	audio data address
 * @param	size			[in]	audio data size
 * @param	timeStamp		[in]	timestamp of this data
 * @return T_U32
 * @retval	0		add failed
 * @retval	1		add ok
 */
T_U32 MobileTV_Rec_AddAudioData(T_pVOID pRecorder, T_pVOID addr, T_U32 size, T_U32 timeStamp);

/**
 * @brief add video data
 *
 * @author Hu_Jing
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @param	addr			[in]	video data address
 * @param	size			[in]	video data size
 * @param	timeStamp		[in]	timestamp of this data
 * @return T_U32
 * @retval	0		add failed
 * @retval	1		add ok
 */
T_U32 MobileTV_Rec_AddVideoData(T_pVOID pRecorder, T_pVOID addr, T_U32 size, T_U32 timeStamp);

/**
 * @brief Get information
 *
 * @author Su_Dan
 * @param	pRecorder		[in]	pointer which is returned by MobileTV_Rec_Open function
 * @param	pInfo			[out]	pointer of T_MOBILETV_REC_INFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		get info ok
 * @retval	AK_FALSE	get info fail
 */
T_BOOL MobileTV_Rec_GetInfo(T_pVOID pRecorder, T_MOBILETV_REC_INFO *pInfo);

#ifdef __cplusplus
}
#endif

#endif//_MOBILETV_RECORDER_LIB_H_
