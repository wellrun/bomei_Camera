/**
 * @file	sdfilter.h
 * @brief	Anyka Sound Device Module interfaces header file.
 *
 * This file declare Anyka Sound Device Module interfaces.\n
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @version V0.0.1
 * @ref
 */

#ifndef __SOUND_FILTER_H__
#define __SOUND_FILTER_H__

#include "lib_media_global.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup Audio Filter library
 * @ingroup ENG
 */
/*@{*/

/* @{@name Define audio version*/
/**	Use this to define version string */	
#define AUDIO_FILTER_VERSION_STRING		(T_U8 *)"AudioFilter Version V1.3.05"
/** @} */

#ifdef _WIN32
#define _SD_FILTER_EQ_SUPPORT    
#define _SD_FILTER_WSOLA_SUPPORT   
#define _SD_FILTER_3DSOUND_SUPPORT
#define _SD_FILTER_RESAMPLE_SUPPORT
#define _SD_FILTER_DENOISE_SUPPORT
#define _SD_FILTER_AGC_SUPPORT
#define _SD_FILTER_VOICECHANGE_SUPPORT
#define _SD_FILTER_PCMMIXER_SUPPORT
#endif

#if defined ANDROID
#define _SD_FILTER_EQ_SUPPORT    
#define _SD_FILTER_WSOLA_SUPPORT   
#define _SD_FILTER_3DSOUND_SUPPORT
#define _SD_FILTER_RESAMPLE_SUPPORT
#define _SD_FILTER_DENOISE_SUPPORT
#define _SD_FILTER_AGC_SUPPORT
//#define _SD_FILTER_VOICECHANGE_SUPPORT
#endif /*#if defined ANDROID*/

typedef enum
{
	_SD_FILTER_UNKNOWN ,
	_SD_FILTER_EQ ,
	_SD_FILTER_WSOLA ,
	_SD_FILTER_RESAMPLE,
	_SD_FILTER_3DSOUND,
	_SD_FILTER_DENOICE,
	_SD_FILTER_AGC,
	_SD_FILTER_VOICECHANGE,
    _SD_FILTER_PCMMIXER
}T_AUDIO_FILTER_TYPE;

typedef enum
{
	_SD_EQ_MODE_NORMAL,
	_SD_EQ_MODE_CLASSIC,
	_SD_EQ_MODE_JAZZ,
    _SD_EQ_MODE_POP,
    _SD_EQ_MODE_ROCK,
    _SD_EQ_MODE_EXBASS,
    _SD_EQ_MODE_SOFT,
    _SD_EQ_USER_DEFINE,
} T_EQ_MODE;

#define _SD_EQ_MAX_BANDS 10

typedef enum
{
	_SD_WSOLA_0_5 ,
	_SD_WSOLA_0_6 ,
	_SD_WSOLA_0_7 ,
	_SD_WSOLA_0_8 ,
	_SD_WSOLA_0_9 ,
	_SD_WSOLA_1_0 ,
	_SD_WSOLA_1_1 ,
	_SD_WSOLA_1_2 ,
	_SD_WSOLA_1_3 ,
	_SD_WSOLA_1_4 ,
	_SD_WSOLA_1_5 ,
	_SD_WSOLA_1_6 ,
	_SD_WSOLA_1_7 ,
	_SD_WSOLA_1_8 ,
	_SD_WSOLA_1_9 ,
	_SD_WSOLA_2_0 
}T_WSOLA_TEMPO;

typedef enum
{
	_SD_WSOLA_ARITHMATIC_0 , // 0:WSOLA, fast but tone bab
	_SD_WSOLA_ARITHMATIC_1   // 1:PJWSOLA, slow but tone well
}T_WSOLA_ARITHMATIC;


typedef enum
{
    RESAMPLE_ARITHMETIC_0 = 0,
    RESAMPLE_ARITHMETIC_1
}RESAMPLE_ARITHMETIC;

typedef enum
{
    _SD_OUTSR_UNKNOW = 0,
	_SD_OUTSR_48KHZ = 1,
	_SD_OUTSR_44KHZ,
	_SD_OUTSR_32KHZ,
	_SD_OUTSR_24KHZ,
	_SD_OUTSR_22KHZ,
	_SD_OUTSR_16KHZ,
	_SD_OUTSR_12KHZ,
	_SD_OUTSR_11KHZ,
	_SD_OUTSR_8KHZ
}T_RES_OUTSR;

typedef enum
{
    PITCH_NORMAL = 0,
    PITCH_CHILD_VOICE ,
    PITCH_MACHINE_VOICE,
    PITCH_ECHO_EFFECT,
    PITCH_ECHO_RESERVED
}T_PITCH_MODES;

typedef struct
{
	MEDIALIB_CALLBACK_FUN_MALLOC			Malloc;
	MEDIALIB_CALLBACK_FUN_FREE				Free;
	MEDIALIB_CALLBACK_FUN_PRINTF			printf;
	MEDIALIB_CALLBACK_FUN_RTC_DELAY			delay;
}T_AUDIO_FILTER_CB_FUNS;


typedef struct
{
	T_U32	m_Type;				//media type
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample 

	union {
		struct {
			T_EQ_MODE eqmode;	
            T_S16 preGain;      //-12 ~ 12 db
			// Reserved for Old Interface
	        T_S16    m_g;		    //gain by DB
			// New Interface
			// For User Presets
			T_U32 bands;      //1~10
			T_U32 bandfreqs[_SD_EQ_MAX_BANDS];
			T_S16 bandgains[_SD_EQ_MAX_BANDS];
            /* 
            设置每个频带的Q值，注意：
            1. bandQ赋值形式为 (T_U16)(x.xxx*(1<<10))
            2. bandQ如果设置为0，则采用库内部的默认值为 (T_U16)(1.22*(1<<10))
            3. x.xxx < 采样率/(2*该频带的中心频点), 并且x.xxx值必须小于64.000
            */
            T_U16 bandQ[_SD_EQ_MAX_BANDS];     // q < sr/(2*f)

            /*** for dc_remove ***/
            T_U8     dcRmEna;
            T_U32    dcfb;
		} m_eq;
		struct {
			T_WSOLA_TEMPO tempo;
            T_WSOLA_ARITHMATIC arithmeticChoice;
		} m_wsola;
		struct{
			T_U8 is3DSurround;
		}m_3dsound;
		struct {
			//目标采样率 1:48k 2:44k 3:32k 4:24K 5:22K 6:16K 7:12K 8:11K 9:8K
			T_RES_OUTSR  outSrindex;

			//设置最大输入长度(bytes)，open时需要用作动态分配的依据。
			//后面具体调用重采样时，输入长度不能超过这个值
			T_U32 maxinputlen; 

            // 由于outSrindex这个限制只能是enum中的几个，当希望的目标采样率是enum之外的值的时候，用这个参数。
            // 这个参数不是采样率的索引了，直接是目标采样率的值。例如8000， 16000 ...
            // 如果想让这个参数生效，必须设置outSrindex=0
            T_U32 outSrFree; 
            
            T_U32 reSampleArithmetic;
		}m_resample;
		struct{
			T_U16 AGClevel;  // make sure AGClevel < 32767
            /* used in AGC_1 */
            T_U32  max_noise;
            T_U32  min_noise;
            /* used in AGC_2 */
            T_U8  noiseReduceDis;  // 是否屏蔽自带的降噪功能
            T_U8  agcDis;  // 是否屏蔽自带的AGC功能
            /*
            agcPostEna：在agcDis==0的情况下，设置是否真正的AGC2库里面做AGC：
            0：表示真正在库里面做agc，即filter_control出来的数据是已经做好agc的；
            1: 表示库里面只要计算agc的gain值，不需要真正做agc处理；真正的agc由外面的调用者后续处理
            */
            T_U8  agcPostEna;  
            T_U16 maxGain;  // 最大放大倍数
            T_U16 minGain;  // 最小放大倍数
            T_U32 dc_freq;  // hz
            T_U32 nr_range; // 1~300,越低降噪效果越明显
		}m_agc;
		struct{
			T_U32 ASLC_ena;  // 0:disable aslc;  1:enable aslc
			T_U32 NR_Level;  //  0 ~ 4 越大,降噪越狠
		}m_NR;
		struct{
			T_PITCH_MODES pitchMode;  // 
		}m_pitch;
	}m_Private;
}T_AUDIO_FILTER_IN_INFO;

typedef struct
{
	T_AUDIO_FILTER_CB_FUNS	cb_fun;
	T_AUDIO_FILTER_IN_INFO	m_info;
}T_AUDIO_FILTER_INPUT;

typedef struct
{
	T_VOID *buf_in;
	T_U32 len_in;
	T_VOID *buf_out;
	T_U32 len_out;
    T_VOID *buf_in2;  //for mix pcm samples
	T_U32 len_in2;
}T_AUDIO_FILTER_BUF_STRC;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief	获取音效处理库版本信息.
 * @author	Deng Zhou
 * @date	2009-04-21
 * @param	[in] T_VOID
 * @return	T_S8 *
 * @retval	返回音效处理库版本号
 */
T_S8 *_SD_GetAudioFilterVersionInfo(void);

/**
 * @brief	获取音效库版本信息, 包括支持哪些功能.
 * @author  Tang Xuechai
 * @date	2014-05-05
 * @param	[in] T_AUDIO_FILTER_CB_FUNS
 * @return	T_S8 *
 * @retval	返回库版本号
 */
T_S8 *_SD_GetAudioFilterVersions(T_AUDIO_FILTER_CB_FUNS *cb);

/**
 * @brief	打开音效处理设备.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] filter_input:
 * 音效处理的输入结构
 * @return	T_VOID *
 * @retval	返回音频库内部解码结构的指针，空表示失败
 */
T_VOID *_SD_Filter_Open(T_AUDIO_FILTER_INPUT *filter_input);

/**
 * @brief	音效处理.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] audio_filter:
 * 音效处理内部解码保存结构
 * @param	[in] audio_filter_buf:
 * 输入输出buffer结构
 * @return	T_S32
 * @retval	返回音频库解码出的音频数据大小，以byte为单位
 */
T_S32 _SD_Filter_Control(T_VOID *audio_filter, T_AUDIO_FILTER_BUF_STRC *audio_filter_buf);

/**
 * @brief	关闭音效处理设备.
 * @author	Deng Zhou
 * @date	2008-04-10
 * @param	[in] audio_decode:
 * 音效处理内部解码保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  关闭成功
 * @retval	AK_FLASE :  关闭异常
 */
T_S32 _SD_Filter_Close(T_VOID *audio_filter);

/**
 * @brief	设置音效参数:播放速度,EQ模式.
 *          如果m_SampleRate,m_BitsPerSample,m_Channels三个有1个为0,则不改变任何音效,返回AK_TRUE
 * @author	Wang Bo
 * @date	2008-10-07
 * @param	[in] audio_filter:
 * 音效处理内部解码保存结构
 * @param	[in] info:
 * 音效信息保存结构
 * @return	T_S32
 * @retval	AK_TRUE :  设置成功
 * @retval	AK_FLASE :  设置异常
 */
T_S32 _SD_Filter_SetParam(T_VOID *audio_filter, T_AUDIO_FILTER_IN_INFO *info);

/**
 * @brief	快速重采样
 * @author	Tang_Xuechai
 * @date	    2013-07-03
 * @param	[in] audio_filter:
 *               音效处理内部解码保存结构
 * @param	[out] dstData 
 *               输出的pcm数据
 * @param	[in] srcData:
 *               输入的pcm数据
 * @param	[in] srcLen 
 *               输入pcm数据的byte数
 * @return	T_S32
 * @retval	>=0 :  重采样后的输出pcm数据的byte数
 * @retval	<0  :  重采样失败
 */
T_S32  _SD_Filter_Audio_Scale(T_VOID *audio_filter, T_S16 dstData[], T_S16 srcData[], T_U32 srcLen);


#ifdef __cplusplus
}
#endif

#endif
/* end of sdfilter.h */
/*@}*/
