#ifndef __FWL_WAVE_H__
#define __FWL_WAVE_H__

#include "akdefine.h"

#define DA_BUF_MAX_NUM      100
#define INVALID_TIME_STAMP  0xFFFFFFFF

typedef enum{
	FADE_STATE_NUL,
	FADE_STATE_IN,
	FADE_STATE_OUT,
	FADE_STATE_CLOSE,
	FADE_STATE_NUM,
}T_eFADE_STATE;

typedef struct
{
	T_eFADE_STATE	FadeState;	//	flag for fade, 1 to do fade in, 2 to do fade out, 0 to do nothing
	
	T_U16	volStep;
	T_U16	FadeStep;
	T_U16	CurNum;
}T_FADE_CTRL;

typedef struct
{
    T_U32 sampleRate;
    T_U16 sampleBits;
    T_U16 channel;
}T_PCM_FORMAT;

typedef enum
{
    eDA_STATE_RUN,
    eDA_STATE_STOP,
    eDA_STATE_EMPTY
}T_eDA_STATE;

/*Define Audio Track LEFT/RIGHT/STEREO */
typedef enum{
	eSOUND_TRACK_LEFT = 0,
	eSOUND_TRACK_RIGHT,
	eSOUND_TRACK_STEREO,
	eSOUND_TRACK_NUM,
}T_eSOUND_TRACK;

typedef T_VOID (*T_DataFinishedCB)(T_VOID);

typedef struct
{
    //T_DA_BUF DABuf[DA_BUF_MAX_NUM];
    //T_U8    rpIndex;//read pointer
    //T_U8    tpIndex;//DA playing pointer
    //T_U8    wpIndex;//write pointer
    //T_BOOL  bFull;//DA 能否继续送
	//T_BOOL  bWriteFull;//能否继续写
    T_eDA_STATE     daState;   
   // T_hSemaphore    tp_mutex;//avoid tp is updated by multi-thread 
    T_U8    DABufNum;
    T_U16   OneBufSize;
	T_U32	timeStamp;
	T_U8	*pCurDaBuf;
	T_U16	datalen;
}T_DABUF_MGR;

typedef struct
{
    T_PCM_FORMAT PCMFmt;
    T_U16   	volume;
	T_U16		curVolume;
    T_eSOUND_TRACK track;	// Sound Track, LEFT/RIGHT/STEREO
    T_DataFinishedCB dataFinishedCB;
    T_DABUF_MGR daBufMgr;
	T_FADE_CTRL	fadeCtrl;
    T_U32   	totalAudioLen;// unit is OneBufSize
    T_U32   	refSampleRate;	//!< For Calculate Tempo Playing Time
    T_BOOL  	bPlayOver;	// if DA has played over all the data or not
    T_BOOL  	bWriteOver;	// if has more data to write into DA buf
    T_BOOL 		bInit;
    T_hSemaphore mutex; 	//avoid write and clean are acted at one time by different thread 
}T_WAVE_OUT;



typedef enum
{
    WAVEOUT_VOLUME,             //volume
    WAVEOUT_TRACK,            	//Sound Track, LEFT/RIGHT/STEREO
    WAVEOUT_REMAIN_DATA,        //remain data lenth in DA buf
    WAVEOUT_SPACE_SIZE,         //the space size of DA buf  
    WAVEOUT_PLAY_OVER,          //if DA has played over all the data or not
    WAVEOUT_WRITE_OVER,         //if has more data to write into DA buf or not
    WAVEOUT_CURRENT_TIME,       //current time from begin
    WAVEOUT_TOTAL_BUFSIZE,      //total DA buf size
    WAVEOUT_ONE_BUFSIZE,			//one DA buf size
    WAVEOUT_FORMAT
}T_eWAVEOUT_STATUS;

typedef struct
{
    T_U32 sampleRate;
    T_U32 sampleBits;
    T_U32 channel;
}T_WAVE_IN_FORMAT;

typedef enum
{
    eINPUT_SOURCE_MIC,
    eINPUT_SOURCE_LINEIN,
    eINPUT_SOURCE_I2S,
    eINPUT_SOURCE_UNKNOW
}T_eINPUT_SOURCE;

typedef struct
{
    T_WAVE_IN_FORMAT	waveInFmt;
    T_S32 				volume;
    T_eINPUT_SOURCE 	inputSrc;
	T_U32 				waveInBufCount;
	T_U32 				OneBufSize;
}T_WAVE_IN;


#endif
