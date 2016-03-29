/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_MediaEncode.h
 * Function:video encode logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/

#ifndef H_LOG_MEDIA_ENCODE_WX_2011_09
#define H_LOG_MEDIA_ENCODE_WX_2011_09
#include "AKFrameStream.h"
#include "lib_media_struct.h"
#include "eng_font.h"

#define ADD_STAMP_ON_YUV   2  // 0: NoShow 1:show when Zoom 2:show when Encode  

#define AUDIOREC_MINIMAL_SPACE          (32*1024*1024)

#define AUDIOREC_SAMPLE_RATE_AMR        8000        //amr samplerate must be 8000
#define AUDIOREC_SAMPLE_RATE_WAV        8000        //8k
#define AUDIOREC_SAMPLE_RATE_ADPCM      8000        //8k
#define AUDIOREC_SAMPLE_RATE_MP3          8000 //32000
#define AUDIOREC_SAMPLE_RATE_AAC          8000  

#define AUDIOREC_CHANNEL_MP3            1            //  mp3

#ifdef CAMERA_SUPPORT

/**
 * current encode extend status   
 */
typedef struct
{
    T_MEDIALIB_REC_INFO    mediaInfo;   //consist of fix and dynamic information 
    T_eMEDIALIB_REC_STATUS recStatus; // consist of fix and dynamic status
    T_U32                  recMaxTimeSec;//encode time estimate(second)
} T_MENC_INFO;

/**
 * encode status check callback for user  
 */
typedef T_BOOL (*MREC_STATUS_CHECK_CB) (T_MENC_INFO recInfo);


/**
  * io control  flag
 */
typedef enum
{
    eMREC_IOCTL_FILE_NAME = 0,
    eMREC_IOCTL_GET_REC_INFO,
    eMREC_IOCTL_GET_REC_SIZE,
    eMREC_IOCTL_MAX
} T_eMREC_IOCTL;

/**
 * encode status 
 */
typedef enum {
    REC_ERROR_NULL,
    REC_ERROR_OK,
    REC_ERROR_ENCODE_EXP,//encode has exception
    REC_ERROR_NO_SPACE,//no enough space for store file
    REC_ERROR_NO_MEM,// no enough memery for system continuing
    REC_ERROR_DETECT_LIMIT,//motion detect is overflow the time limit
    REC_ERROR_SIZE_LIMIT,//record file size is overflow the size limit
    REC_ERROR_TIME_LIMIT,//record time is overflow the time limit of user's configure
} T_REC_ERROR_STATUS;



/**
 * encode control parameter 
 */
typedef struct
{
    /*CYC  Recorde flag*/
    T_U32                     cycRecTimeMs;// if zero will close cyc, else is cyc time
    //---- limit  ---------
    T_U32                     recTimeSecLimit; // rec time limit (sec),if zero will default  limit
    T_U32                     recSizeLimit; // rec file size limit (Bytes), 
    T_U32                     recResSize; //  rec file reverse free spzce limit, if zero will not limit

    /*media file name and path*/
    T_USTR_FILE               recFilePath;// rec file path
    /*media file index  store path*/
    T_USTR_FILE               indexFilePath;// index file path
    /*if index use  mem  but file */
    T_BOOL                    useMemIndex;//if false, will use file for store index    
    T_BOOL                    autoDelAllFile;// at cyc recored , when need delete file
                                             // if this flag is false,will not delete all file
    T_BOOL                    isEnableFocus;// flag for enable video change foucs 
    T_BOOL                    cycRecbySpace;
	T_U32                     cycRecbySpaceSize;
	T_U32					  asynWriteSize;// asyn write size
	T_U32                     detectNoMovingTimeMs;//time (ms) for detect static frame 
    T_U32                     encMsLimitPerFrame;//defalut  video encode interval(when 
                                              //video is not moving, this config will be used)
    MREC_STATUS_CHECK_CB      usrEncCheckCbf;// function of callback for user check encode status
} T_REC_CTRL_INIT_PARAM;

/**
 * audio encode control parameter 
 */
typedef struct
{
    T_U8  audioEncType;/*audio type : eRECORD_MODE_WAV /eRECORD_MODE_AMR*/
    T_U32 audioEncSamp;/*samp*/

} T_REC_AUDIO_INIT_PARAM;

/**
 * video encode control parameter 
 */
typedef struct
{
    //======================
    T_U8        videoEncType;/*video type : MEDIALIB_REC_AVI_NORMAL/MEDIALIB_REC_3GP*/
    //======================
    T_U32        videoWidth;// meidia 's video width
    T_U32        videoHeight;// media's video height 
    T_U32        viedoSrcWinWidth; //encode srouce frame size (if zero, will auto adapt source size)
    T_U32        viedoSrcWinHeight;
    //======================
    T_U16        FPS;// record fps
    T_U32        vbps;// record vbps
    T_U16        keyfFameInterval;// interval frame
} T_REC_VIDEO_INIT_PARAM;


T_BOOL MEnc_Init(T_VOID);
T_BOOL MEnc_Destroy(T_VOID);

T_HANDLE MEnc_Open(T_REC_AUDIO_INIT_PARAM  *pRecAudioParam);
T_REC_ERROR_STATUS MEnc_Start(T_HANDLE handle,T_REC_CTRL_INIT_PARAM   *pRecCtlParam,T_REC_AUDIO_INIT_PARAM  *pRecAudioParam,T_REC_VIDEO_INIT_PARAM  *pRecVideoParam);
T_REC_ERROR_STATUS MEnc_Resume(T_HANDLE hdl);
T_REC_ERROR_STATUS MEnc_ReStart(T_HANDLE hdl, T_BOOL isSavePre);
T_BOOL MEnc_Suspend(T_HANDLE hdl, T_BOOL isSavePre);

T_BOOL MEnc_Close(T_HANDLE handle, T_BOOL isSave);
T_BOOL MEnc_Ioctl(T_HANDLE handle, T_eMREC_IOCTL ctlType, T_VOID * arg);
T_S32  MEnc_VideoFrameEncode(T_HANDLE handle,T_FRM_DATA *pSrcFrame);
T_S32  MEnc_AudioPcmEncode(T_HANDLE handle);
T_S32  MEnc_ExceptionStatusCheck(T_HANDLE hdl,T_REC_ERROR_STATUS *status);
T_BOOL MEnc_ExceptionIsNeedRestart(T_HANDLE hdl, T_REC_ERROR_STATUS exitMode, T_BOOL *isNeedSavePre);


T_S32  MEnc_FrameAddStampOnYuv(T_FRM_DATA *pDestFrame, T_eFONT setFont);
T_S32  MEnc_GetFileName(T_USTR_FILE curPath, T_BOOL isMkSubFolder, 
                 T_STR_NAME preName, T_STR_NAME sufName, T_pWSTR pFileName);

T_VOID MRec_SetAudioRecInfo(T_AUDIO_REC_INPUT *audioRec, T_eREC_MODE audioType);


/*
 * @brief   open/close video frame dropping automatic
 * @author WangXi
 * @date	2011-11-23
 * @param[in/out] hdl: encoder handle
 * @param[in] isEnable:  if open the drop frame function
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  MEnc_EnableVideoFrameDrop(T_HANDLE hdl,T_BOOL isEnable);


/**
 * @brief Get Current System Time File Name For Work Directory
 * @note     1. 按生成文件的日期在工作目录下创建相应的日期文件夹, 所有此日期生成的文件放置在相应的目录里面;
 *            2. 取系统时间(HHMMSS)作为文件名, 如此文件名已存在,则加上序号后缀"_N"(N 从2~1024).
 *            3. 对各个作业生成的文件用前缀给予区别. 拍照文件前缀"IMG_", 录音文件前缀"REC_", 录像文件前缀"DV_".
 * @date 2011-4-19
 * @author     Xie_wenzhong
 * @param    curPath        [in]    Work Directory
 * @param    sufName        [in]    Suffix Name
 * @param    pFileName    [out] File Name (Include Path)
 * @return    T_BOOL
 * @retval    AK_TRUE    Get File Name ok
 * @retval    AK_FALSE    Get File Name fail
 */
T_BOOL MRec_GetStampFileName(T_USTR_FILE curPath, T_STR_NAME preName, T_STR_NAME sufName, T_pWSTR pFileName);

#endif
#endif

