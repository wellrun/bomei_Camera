/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_MediaEncode.c
 * Function:video encode logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/
#include "gbl_macrodef.h"
#include "akappmgr.h"
#include "akos_api.h"
#include "AKFrameStream.h"
#include "drv_api.h"
#include "File.h"
#include "lib_media_global.h"
#include "lib_image_api.h"
#include "lib_sdcodec.h"
#include "Lib_sdfilter.h"
#include "video_stream_lib.h"
#include "media_player_lib.h"
#include "media_recorder_lib.h"
#include "fwl_font.h"
#include "Eng_DynamicFont.h"

#include "fwl_sysevent.h"
#include "fwl_osmalloc.h"
#include "fwl_oscom.h"
#include "fwl_osfs.h"
#include "fwl_wavein.h"
#include "eng_debug.h"
#include "Eng_ImgDec.h"
#include "Eng_String_UC.h"
#include "Eng_dataconvert.h"
#include "Fwl_display.h"
#include "Eng_Time.h"
#include "Log_RecAudio.h"
#include "log_mediaplayer.h"
#include "log_MediaEncode.h"
#include "Eng_Math.h"
#include "Fwl_ImageLib.h"
#include "Ctl_VideoRecord.h"


//#define SUPPORT_VIDEOREC_AUDIO_DENOICE  
extern T_VOID Sd_SetAudioCB(T_AUDIO_FILTER_CB_FUNS* audioCB);

extern T_VOID Sd_SetCodecCB(T_AUDIO_CB_FUNS *cbFun);

#ifdef CAMERA_SUPPORT

extern T_VIDEO_RECORD *pVideoRec;

//=================  Macro  Define (Debug)===========================

#ifdef PLATFORM_DEBUG_VER
#define MENC_DEBUG_VER
#endif


#ifdef MENC_DEBUG_VER
#define STR_THIS_FILE       "log_MediaEncode.c: "
#else
#define STR_THIS_FILE        ""
#endif

//====================== Macro  Define ===========================
#define AUDIO_OUTPUT_BUFFER_SIZE        (10*1024)

/**
 * record index write size (size per second)
 */
#define INDEX_FILE_SIZE_SEC             (16)

/**
 * encode close file use asyn mode (1 use asyn, else close)
 */
#define ENABLE_REC_NO_INTERVAL         (1)

/**
 * insert video frame default percent(if insert frame is over this limit,  theere will an encode error happen)
 * here means the max insert frame is FPS*20%100(i.e., FPS/5)
 */
#define VIDEO_FRAME_INSERT_PERCENT_DEFAULT         (20)


// yuv to jpeg encode use interrupt mode
#define IMG_YUV2JPEG_INT

#define STAMP_LEFT		10
#define STAMP_TOP		10
//===================== Type Define =============================

/**
 * media encode control  struct
 */
typedef struct tagMediaEncode
{
    /* extra user configurate parameter */
    T_REC_VIDEO_INIT_PARAM    videoParam; // video encode param
    T_REC_AUDIO_INIT_PARAM    audioParam; // audio encode param
    T_REC_CTRL_INIT_PARAM     recCtlParam;// encode ctrl param

    /* runtime status and recorde information */
    T_MENC_INFO               recInfo;

    //cofig at run
    T_USTR_FILE             recFileName;//curretn  file save path
    T_U32                   recRunTimeMsLimt;//encode time limit(millsecond), if zero will not limit
    T_U32                   recRunSizeByteLimt;//encode file limit(bytes), if zero will not limit
    T_U32                   restartCnt;// encode restart count statistics
    T_U32                   prePts;// encode pretimestamp
    T_U32                   preEncodeSysTime;// encode pre systemtime
    T_U32                   curEncodeSysTime;// encode current systemtime
    T_U32                   curMaxInsVideoFrm;
    T_BOOL                  isAudioInit;// audio inited flag
    T_BOOL                  isVideoInit;// video inted flag
    T_BOOL                  isVideoNeedDrop;// video encode flag
    /* config at initial */
    T_S32                    fd;/* recorde file handle */
    T_S32                    fd_index;// recored index file handle
    T_MEDIALIB_STRUCT        hMedia;/* media library handle */
    T_U32                    basePts;/* recorde start time (millisecond) */
    T_U32                    currPts;//encode current pts
    T_U32                    intervalPts;//two frame encode interval 
    T_U32                    firstStartup;//first start flag

    struct tagAudioEncode
    {
        /* input configuration */
        T_AUDIO_REC_INPUT        cfgInputInfo;//audio lib init info
        T_U32                    cfgDuration;// pcm one section time(ms)

        /* device handle and control information */
        T_pVOID                  hStreamInDevice;// pcm device handle
        T_VOID                   *hEncodeDevice;// audio encode handle 
        T_VOID                   *hAudioFilter;// audio filter handle 
        T_BOOL                   ctlNeedEncode;// if need encode pcm to other package

        /* data buffer */
        T_AUDIO_ENC_OUT_INFO    cfgOutputInfo;/* output configuration information */
        T_AUDIO_ENC_BUF_STRC    bufCtrl;
        T_ENCODE_OUTBUF         outputBuf;
        T_U32                   needSize;
        T_U32                   count;
    } audio;
} T_MEncodeCtrl;

/**
 * media encode status  struct
 */
struct tagEncodeDescriptor
{
    /* media library control */
    T_BOOL                    mediaLibInit;
    /* media task control */
    T_MEncodeCtrl             *current;
};


//====================== Macro  Define(Function)=====================
/**
 * estimate the size by  record time and vbps(sec*(byte/sec))
 */
#define VIDEO_SIZE_BYTE_SEC(sec,vbps)         (((sec)*((vbps)))>>3)

/**
 * min vbps limt, when estimate the file storage,if configure vbps is less than this val,
 * encoder will use below vbps limt
 */
#define VIDEO_ENCODE_VBPS_LIMIT              (10<<20ul)

/**
 * min encode time  limt, when estimate the file storage,if free space not enough,
 * encoder will use below time limt
 */
#define VIDEO_ENCODE_MIN_TIME_SEC            (10)


//========================= Global Variable  Define ===================
static struct tagEncodeDescriptor    m_MediaLibDesc = {AK_FALSE, 0};

static T_MEncodeCtrl *g_pEncodehdl = AK_NULL;	//由于回调函数无法传递参数，用全局变量传递句柄
T_BOOL g_bAudioEncoded = AK_FALSE;	//用于控制每帧数据只调用一次音频编码

//=======================extern Function  Declare====================
extern T_pWSTR Fwl_GetDefPath(T_DEFPATH_TYPE type);

//=======================Local Function  Declare=====================
static T_S32 openRecFile(T_REC_CTRL_INIT_PARAM *pCtlParam, T_REC_VIDEO_INIT_PARAM *pVideoParam, T_pWSTR pFileName);
static T_S32 closeRecFile(T_MEncodeCtrl * hdl,T_BOOL isSave, T_BOOL isAsyn);
static T_S32 openTempIndexFile(T_REC_CTRL_INIT_PARAM *pCtlParam);
static T_S32 closeTempIndexFile(T_MEncodeCtrl * hdl,T_BOOL isSave);

static T_BOOL audioEncodeParamCheck(T_REC_AUDIO_INIT_PARAM *pAudioParam);
static T_BOOL audioEncodeOpen(T_MEncodeCtrl * hdl,T_REC_AUDIO_INIT_PARAM *audioParam);
static T_BOOL audioEncodeStart(T_MEncodeCtrl * hdl);
static T_BOOL audioEncodeStop(T_MEncodeCtrl * hdl);
static T_BOOL audioEncodeClose(T_MEncodeCtrl * hdl);

static T_U32  videoEncodeMaxRecTime(T_REC_CTRL_INIT_PARAM *pCtlParam);
static T_U32  videoEncodeMaxRecSize(T_REC_CTRL_INIT_PARAM *pCtlParam, T_U32 resSize);
static T_U32  videoEncodeResSize( T_REC_CTRL_INIT_PARAM *pCtlParam);
static T_BOOL videoEncodeCheckSpace(T_MEncodeCtrl * hMRec,T_BOOL isAutoDelete,T_U32 reverseCount);
static T_VOID videoEncode_WaitCodec(T_VOID);
static T_BOOL videoEncodeYuv(T_pVOID handle, T_FRM_DATA *pSrcFrame, T_U32 interval);
static T_BOOL videoEncodeParamCheck(T_REC_VIDEO_INIT_PARAM *pVideoParam,T_REC_CTRL_INIT_PARAM *pCtlParam);
static T_BOOL videoEncodeOpen(T_MEncodeCtrl * hdl,  T_REC_VIDEO_INIT_PARAM *pVideoParam, T_REC_CTRL_INIT_PARAM *pCtlParam);
static T_BOOL videoEncodeStart(T_MEncodeCtrl * hdl);
static T_BOOL videoEncodeStop(T_MEncodeCtrl * hdl);
static T_BOOL videoEncodeClose(T_MEncodeCtrl * hdl, T_BOOL isSave);
static T_VOID audioEncodeGetOffBufData(T_MEncodeCtrl * hdl, T_U32 throwtime);


/*
 * @brief   set video frame insert max
 * @author WangXi
 * @date	2011-11-23
 * @param[in/out] hdl: encoder handle
 * @param[in] isEnable:  if open the drop frame function
 * @return	T_BOOL AK_TRUE--success, else fail
 */
static T_BOOL videoEncodeSetMaxDropFrame(T_MEncodeCtrl * hdl, T_U32 maxInsertFrame);

static T_S32 exFuncYUVENC(T_U8 *srcStream, T_U8 *dstStream, T_U32 *pSize, T_U32 pic_width, T_U32 pic_height, T_U32 quality);

//==================== Function  Define ===================
T_BOOL MEnc_Init(T_VOID)
{
    T_MEDIALIB_INIT_INPUT        initIn;
    T_MEDIALIB_INIT_CB            initCB;

    if (!m_MediaLibDesc.mediaLibInit)
    {
        MPlayer_SetInitParm(&initIn, &initCB);
        // if (!MediaLib_Init(&initIn, &initCB))// need not, this function will only call VideoStream_Init
        if (!VideoStream_Init(&initIn, &initCB))
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "MEnc_Init: failed");
            return AK_FALSE;
        }
        m_MediaLibDesc.mediaLibInit     = AK_TRUE;
    }
    return AK_TRUE;
}

T_BOOL MEnc_Destroy(T_VOID)
{
    if (m_MediaLibDesc.mediaLibInit)
    {
        // MediaLib_Destroy();
        VideoStream_Destroy();
        m_MediaLibDesc.mediaLibInit    = AK_FALSE;
    }

    return AK_TRUE;
}


T_BOOL MEnc_Suspend(T_HANDLE hdl, T_BOOL isSavePre)
{
    T_MEncodeCtrl *hMRec;
    hMRec = (T_MEncodeCtrl*)hdl;

    AK_ASSERT_PTR(hMRec, STR_THIS_FILE "MEnc_Suspend: parameter error", AK_FALSE);

    videoEncodeStop(hMRec);
    closeRecFile(hMRec,isSavePre, AK_TRUE);
    closeTempIndexFile(hMRec, AK_FALSE);
    return AK_TRUE;
}


// must call behind  MEnc_Suspend
T_REC_ERROR_STATUS MEnc_Resume(T_HANDLE hdl)
{
#ifdef OS_ANYKA
    T_MEncodeCtrl *hMRec;
    T_U32   ADbuffcount;
    hMRec = (T_MEncodeCtrl*)hdl;

    AK_ASSERT_PTR(hMRec, STR_THIS_FILE "MEnc_Resume: parameter error", REC_ERROR_ENCODE_EXP);

    if (!videoEncodeCheckSpace(hMRec,(hMRec->recCtlParam.cycRecTimeMs > 0),1))
    {
        return REC_ERROR_NO_SPACE;
    }
    hMRec->fd = openRecFile(&(hMRec->recCtlParam), &(hMRec->videoParam), hMRec->recFileName);
    if (_FOPEN_FAIL == hMRec->fd)
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openRecFile.Restart hMediaDest error.\n");
        return REC_ERROR_ENCODE_EXP;
    }
    hMRec->fd_index = openTempIndexFile(&(hMRec->recCtlParam));
    if (_FOPEN_FAIL == hMRec->fd_index)
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openTempIndexFile.Restart index error.\n");
        return REC_ERROR_ENCODE_EXP;
    }
    
    ADbuffcount = 0;
    waveInSetStatus(hMRec->audio.hStreamInDevice, WAVEIN_SET_COUNT, &ADbuffcount);
    MediaLib_Rec_UpdateIndexFile(hMRec->hMedia,hMRec->fd_index);
    if (!MediaLib_Rec_Restart(hMRec->hMedia, hMRec->fd, 0))
    {
        return REC_ERROR_ENCODE_EXP;
    }
    Fwl_Print(C3, M_MENCODE,  "Restart record...");
    Printf_UC(hMRec->recFileName);
    hMRec->restartCnt++;
#endif
    return REC_ERROR_OK;
}

T_REC_ERROR_STATUS MEnc_ReStart(T_HANDLE hdl, T_BOOL isSavePre)
{
    AK_ASSERT_PTR((T_MEncodeCtrl*)hdl, STR_THIS_FILE "MEnc_ReStart: parameter error", REC_ERROR_ENCODE_EXP);

    if (MEnc_Suspend(hdl, isSavePre))
    {
        return MEnc_Resume(hdl);
    }

    return REC_ERROR_ENCODE_EXP;
}
/*
 *
 *@return 0 -- error
 */
T_HANDLE MEnc_Open(T_REC_AUDIO_INIT_PARAM  *pRecAudioParam)
{
    T_MEncodeCtrl    *handle = AK_NULL;

    AK_ASSERT_PTR(pRecAudioParam, STR_THIS_FILE "MEnc_Open: parameter error", 0);
    
    /* module not initial */
    if (!m_MediaLibDesc.mediaLibInit)
    {
        return 0;
    }

    /* request resource */
    /* create handle struct */
    handle    = (T_MEncodeCtrl *)Fwl_Malloc(sizeof(T_MEncodeCtrl));
    if (AK_NULL == handle)
    {
        return 0;
    }
    memset(handle, 0, sizeof(T_MEncodeCtrl));

    handle->fd          = _FOPEN_FAIL;
    handle->fd_index    = _FOPEN_FAIL;
    handle->isAudioInit  = AK_FALSE;
    handle->isVideoInit = AK_FALSE;
    
    /* save parameter */
    memcpy(&handle->audioParam,  pRecAudioParam, sizeof(T_REC_AUDIO_INIT_PARAM));
    if (!audioEncodeParamCheck(&handle->audioParam))
    {
        Fwl_Free(handle);
        return 0;
    }
    
    /* open recorder */
    if (!audioEncodeOpen(handle, &handle->audioParam))
    {
        Fwl_Free(handle);
        return 0;
    }
    
    if (!audioEncodeStart(handle))
    {
        audioEncodeClose(handle);
        Fwl_Free(handle);
        return 0;
    }
    
    handle->isAudioInit = AK_TRUE;

    /* set task status */
    m_MediaLibDesc.current        = handle;

    /* exit critical */

    Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "MEnc_Open: OK!\n");
    return (T_HANDLE)handle;
}



/*
 *
 *@return 0 -- error
 */
T_REC_ERROR_STATUS MEnc_Start(T_HANDLE handle,T_REC_CTRL_INIT_PARAM   *pRecCtlParam,T_REC_AUDIO_INIT_PARAM  *pRecAudioParam,T_REC_VIDEO_INIT_PARAM  *pRecVideoParam)
{
    T_MEncodeCtrl    *rec = (T_MEncodeCtrl *)handle;

    
    AK_ASSERT_PTR(rec, STR_THIS_FILE "MEnc_Start: hdl error", REC_ERROR_ENCODE_EXP);
    AK_ASSERT_PTR(pRecCtlParam, STR_THIS_FILE "MEnc_Start: parameter error", REC_ERROR_ENCODE_EXP);
    AK_ASSERT_PTR(pRecVideoParam, STR_THIS_FILE "MEnc_Start: parameter error", REC_ERROR_ENCODE_EXP);
    
    /* module not initial */
    if (!m_MediaLibDesc.mediaLibInit)
    {
        return REC_ERROR_ENCODE_EXP;
    }

    /* save parameter */
    memcpy(&rec->videoParam,  pRecVideoParam, sizeof(T_REC_VIDEO_INIT_PARAM));
    memcpy(&rec->recCtlParam, pRecCtlParam, sizeof(T_REC_CTRL_INIT_PARAM));
    if (!videoEncodeParamCheck(&rec->videoParam,&rec->recCtlParam))
    {
        return REC_ERROR_ENCODE_EXP;
    }
    if (!videoEncodeCheckSpace(rec,(T_BOOL)(rec->recCtlParam.cycRecTimeMs > 0),0))
    {
        return REC_ERROR_NO_SPACE;
    }
    if (pRecCtlParam->asynWriteSize > 0)
    {
        if (Fwl_GetRemainRamSize() < pRecCtlParam->asynWriteSize)
        {
            pRecCtlParam->asynWriteSize = 0;
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openRecFile: mem not enough, close asyn write!!\n");
        }
        Fwl_InitAsyn(pRecCtlParam->asynWriteSize, pRecCtlParam->recFilePath);
    }
	
    rec->preEncodeSysTime = get_tick_count();
    /* open recorder */
    if (!videoEncodeOpen(rec, pRecVideoParam, pRecCtlParam))
    {
        if (rec->isAudioInit)
        {
            audioEncodeStop(rec);
            audioEncodeClose(rec);
        }
        return REC_ERROR_ENCODE_EXP;
    }

    if (!videoEncodeStart(rec))
    {
        videoEncodeClose(rec, AK_FALSE);
        if (rec->isAudioInit)
        {
            audioEncodeStop(rec);
            audioEncodeClose(rec);
        }
        return REC_ERROR_ENCODE_EXP;
    }

    videoEncodeSetMaxDropFrame(rec, 0);
    
    rec->isVideoInit = AK_TRUE;
    
    
    Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "MEnc_Start: OK!\n");
    return REC_ERROR_OK;
}


T_BOOL MEnc_Close(T_HANDLE handle, T_BOOL isSave)
{
    T_BOOL        retValue;
    T_MEncodeCtrl    *rec = (T_MEncodeCtrl *)handle;

    AK_ASSERT_PTR(rec, STR_THIS_FILE "MEnc_Close: parameter error", AK_FALSE);
    /* enter critical */
    /* module not initial */
    if (!m_MediaLibDesc.mediaLibInit)
    {
        return AK_FALSE;
    }

    /* close recorder */
    retValue = audioEncodeStop(rec);
    
    if (!audioEncodeClose(rec))
    {
        retValue = AK_FALSE;
    }
    
    if (!videoEncodeStop(rec))
    {
        retValue = AK_FALSE;
    }
    
    if (!videoEncodeClose(rec, isSave))
    {
        retValue = AK_FALSE;
    }
    
    if (rec->recCtlParam.asynWriteSize > 0)
    {
        Fwl_DeInitAsyn(rec->recCtlParam.recFilePath);
    }
    
#if (1 == ENABLE_REC_NO_INTERVAL)
    else
    {
        Fwl_AsynCloseFlushAll();
    }
#endif

    /* release resource */
    Fwl_Free(rec);

    /* set task status */
    m_MediaLibDesc.current = 0;

    /* exit critical */


    Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "MEnc_Close: OK!\n");
    return retValue;
}



T_BOOL MEnc_Ioctl(T_HANDLE handle, T_eMREC_IOCTL ctlType, T_VOID *arg)
{
    T_BOOL        retValue = AK_TRUE;

#ifdef OS_ANYKA
    T_MEncodeCtrl    *rec = (T_MEncodeCtrl*)handle;

    AK_ASSERT_PTR(rec, STR_THIS_FILE "MEnc_Ioctl: parameter error", AK_FALSE);

    /* module not initial */
    if (!m_MediaLibDesc.mediaLibInit)
    {
        return AK_FALSE;
    }

    switch (ctlType)
    {
    case eMREC_IOCTL_FILE_NAME:
        Utl_UStrCpyN((T_U16*)arg, rec->recFileName, FS_MAX_PATH_LEN);
        break;
        
    case eMREC_IOCTL_GET_REC_INFO:
        memcpy((T_U8 *)arg, (T_U8 *)&(rec->recInfo), sizeof(T_MENC_INFO));
        break;
    case eMREC_IOCTL_GET_REC_SIZE:
        if (_FOPEN_FAIL != rec->fd)
        {
            *(T_S32 *)arg = Fwl_GetFileLen(rec->fd);
        }
        else
        {
            retValue = AK_FALSE;
        }
        break;
    default:
        *(T_S32 *)arg    = -1;    /* error */
        break;
    }
#endif      

    return retValue;
}


/*
 * @brief   open/close video frame dropping automatic
 * @author WangXi
 * @date	2011-11-23
 * @param[in/out] hdl: encoder handle
 * @param[in] isEnable:  if open the drop frame function
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MEnc_EnableVideoFrameDrop(T_HANDLE hdl,T_BOOL isEnable)
{
    T_MEncodeCtrl * hMEnc = AK_NULL;
    
    hMEnc = (T_MEncodeCtrl*)hdl;
    if (AK_NULL == hMEnc)
    {
        Fwl_Print(C3, M_MENCODE, "VideoFrameDrop, param is null\n");
        return AK_EFAILED;
    }
    
    if (isEnable != hMEnc->isVideoNeedDrop)
    {
        T_U32 maxInsertFrm = 0;

        Fwl_Print(C3, M_MENCODE, "%s drop frame!\n", isEnable?"Start":"Stop");
		
        if (isEnable)
        {
            //calculate the max insert frame by user's configure
            maxInsertFrm = hMEnc->videoParam.FPS * hMEnc->recCtlParam.encMsLimitPerFrame / 1000;
        }
        else
        {
            maxInsertFrm = 0;
        }
        
        if (videoEncodeSetMaxDropFrame(hMEnc, maxInsertFrm))
        {
            hMEnc->isVideoNeedDrop = isEnable;
        }
    }
    
    return AK_SUCCESS;
}



T_S32 MEnc_ExceptionStatusCheck(T_HANDLE hdl,T_REC_ERROR_STATUS *status)
{
#ifdef OS_ANYKA
    T_U32     fileSz = 0;
    T_U32     videoSz = 0;
    T_U64_INT driverSize = {0};
    T_MEncodeCtrl * hMRec = AK_NULL;

    if (AK_NULL == status)// need not check
    {
        return AK_SUCCESS;
    }
        
    hMRec = (T_MEncodeCtrl*)hdl;
    if (AK_NULL == hMRec)
    {
        Fwl_Print(C3, M_MENCODE, "Sys exception, param is null\n");
        *status = REC_ERROR_ENCODE_EXP;
        return AK_EFAILED;
    }

    //------------------Media Lib Encode Status Check--------------------------
    /**exit from current state machine if error happens*/
    hMRec->recInfo.recStatus = MediaLib_Rec_GetStatus(hMRec->hMedia);
    if (MEDIALIB_REC_DOING != hMRec->recInfo.recStatus)
    {
        Fwl_Print(C3, M_MENCODE, "Sys exception,status=%d\n",hMRec->recInfo.recStatus);
        *status = REC_ERROR_ENCODE_EXP;
        return AK_EFAILED;
    }
    
    //------------------ Current Rec File Size Check----------------------------
    fileSz = Fwl_GetFileLen(hMRec->fd);
    videoSz = fileSz;
    if ((AK_NULL != hMRec->hMedia) && MediaLib_Rec_GetInfo(hMRec->hMedia, &(hMRec->recInfo.mediaInfo)))
    {
        videoSz = fileSz + (INDEX_FILE_SIZE_SEC * hMRec->recInfo.mediaInfo.total_video_frames);
        if (videoSz >= hMRec->recRunSizeByteLimt)
        {
            Fwl_Print(C3, M_MENCODE, "Rec Size(%d) Over Limit(%d)\n",videoSz,hMRec->recRunSizeByteLimt);
            *status = REC_ERROR_SIZE_LIMIT;
            return AK_EFAILED;
        }

        //-------------------current Encode Time Check-----------------------
        if (hMRec->recRunTimeMsLimt > 0)
        {
            if (hMRec->recInfo.mediaInfo.total_time_ms >= hMRec->recRunTimeMsLimt)
            {
                Fwl_Print(C3, M_MENCODE, "Rec time(%d) Over Limit(%d) ms\n",
                    hMRec->recInfo.mediaInfo.total_time_ms,hMRec->recRunTimeMsLimt);
                *status = REC_ERROR_TIME_LIMIT;
                return AK_EFAILED;
            }
        }
    }

    //--------------------------Free Size Chcke-----------------------------
    Fwl_FsGetFreeSize(hMRec->recFileName[0], &driverSize);
    /**calculate video size and record time*/
    if ((U64cmpU32(&driverSize, hMRec->recCtlParam.recResSize))<= 0)
    {
        Fwl_Print(C3, M_MENCODE, "No Enough Space,Remain 0x[%08x,%08x],Need 0x[%08x+%08x]\n",
            driverSize.high,driverSize.low,
            videoSz,hMRec->recCtlParam.recResSize);
        *status = REC_ERROR_NO_SPACE;
        return AK_EFAILED;
    }
    
    //------------------User's status Check----------------------------------
    if (AK_NULL != hMRec->recCtlParam.usrEncCheckCbf)
    {
       if (!(hMRec->recCtlParam.usrEncCheckCbf)(hMRec->recInfo))
       {
           Fwl_Print(C3, M_MENCODE, "user check status err\n");
           *status = REC_ERROR_ENCODE_EXP;
           return AK_EFAILED;
       }
    }
    //-----------------------------------------------------------------
    
    *status = REC_ERROR_OK;
 #endif
    return AK_SUCCESS;
}




/**
 * @brief deal with record err
 *
 * @author wangxi
 * @date 2011-05-10
 * @param[in] handler of video record
 * @param[in] exitMode
 * @param[in] if show message box or not
 */
T_BOOL  MEnc_ExceptionIsNeedRestart(T_HANDLE hdl, T_REC_ERROR_STATUS exitMode, T_BOOL *isNeedSavePre)
{
    T_MEncodeCtrl * hMRec = AK_NULL;
    T_BOOL isNeedRestart = AK_TRUE;

    hMRec = (T_MEncodeCtrl*)hdl;

    if ((AK_NULL == isNeedSavePre)|| (AK_NULL == hMRec) )
    {
        return AK_TRUE;
    }
    
    switch(exitMode)
    {
        case REC_ERROR_NO_SPACE:
            isNeedRestart = (hMRec->recCtlParam.cycRecTimeMs > 0);//循环录像无空间继续录像
            (*isNeedSavePre) = AK_TRUE;
            break;
        case REC_ERROR_SIZE_LIMIT:
            isNeedRestart = AK_TRUE;
            (*isNeedSavePre) = AK_TRUE;
            break;
        case REC_ERROR_TIME_LIMIT://超过限制时间
            isNeedRestart = (hMRec->recCtlParam.cycRecTimeMs > 0);
            (*isNeedSavePre) = AK_TRUE;
            break;
        case REC_ERROR_ENCODE_EXP:
            isNeedRestart = AK_FALSE;
            (*isNeedSavePre) = AK_FALSE;
            break;
        case REC_ERROR_DETECT_LIMIT:// 移动侦测超时处理
            isNeedRestart = (hMRec->recCtlParam.cycRecTimeMs > 0);
            (*isNeedSavePre) = AK_TRUE;
            break;
        default:
            isNeedRestart = AK_TRUE;//出现错误默认重新录制下个文件
            (*isNeedSavePre) = AK_TRUE;
            break;
    }
    
    return isNeedRestart;
}



static T_S32 openRecFile(T_REC_CTRL_INIT_PARAM *pCtlParam, T_REC_VIDEO_INIT_PARAM *pVideoParam, T_pWSTR pFileName)
{
    T_STR_INFO    suffix;
    T_STR_INFO    preFix;
    T_USTR_FILE  filename;
    T_S32 fd;
    T_S32 lRet = AK_EFAILED;
    T_BOOL isInMobileMedium = AK_FALSE;
    
    if (pCtlParam->cycRecTimeMs > 0)
    {
        Utl_StrCpy(preFix, "CYC_DV");
    }
    else
    {
        Utl_StrCpy(preFix, "DV");
    }
    
    if (eRECORD_MEDIA_3GP_MPEG4_AMR == pVideoParam->videoEncType)
    {
        Utl_StrCpy(suffix, ".3GP");
    }
    else
    {
        Utl_StrCpy(suffix, ".AVI");
    }
    lRet = MEnc_GetFileName(pCtlParam->recFilePath,(0 == pCtlParam->cycRecTimeMs),preFix, suffix, filename);

    if (AK_IS_FAILURE(lRet))
    {
       return _FOPEN_FAIL;
    }


    isInMobileMedium = Fwl_IsInMobilMedium(filename);
    if (pCtlParam->asynWriteSize > 0)
    {
        fd = Fwl_FileOpenAsyn(filename, _FMODE_CREATE, _FMODE_CREATE);

    }
    else
    {
        fd = Fwl_FileOpen(filename, _FMODE_CREATE, _FMODE_CREATE);
    }
    
    if (_FOPEN_FAIL == fd)
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openRecFile: open file failed.\n");
        return _FOPEN_FAIL;
    }

    else
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openRecFile:fp=0x%x\n", fd);
    }

    if (isInMobileMedium)
    {
        Fwl_SetFileBufferSize(fd,64);
    }
    Fwl_SetFileSize(fd,8<<20);

    if (AK_NULL != pFileName)
    {
        Utl_UStrCpy(pFileName, filename);
    }
    return fd;
}


static T_S32 openTempIndexFile(T_REC_CTRL_INIT_PARAM *pCtlParam)
{
    T_USTR_FILE  filename;
    T_S32 fd;
    T_STR_INFO    suffix;
    T_STR_INFO    preFix;
    T_S32 lRet = AK_EFAILED;
    T_BOOL isInMobileMedium = AK_FALSE;
    
    if (pCtlParam->cycRecTimeMs > 0)
    {
        Utl_StrCpy(preFix, "CYC_DV");
    }
    else
    {
        Utl_StrCpy(preFix, "DV");
    }

    Utl_StrCpy(suffix, ".Index");

    lRet = MEnc_GetFileName(pCtlParam->indexFilePath,AK_FALSE,preFix, suffix, filename);

    if (AK_IS_FAILURE(lRet))
    {
       return _FOPEN_FAIL;
    }
    
    isInMobileMedium = Fwl_IsInMobilMedium(filename);
    if (pCtlParam->asynWriteSize > 0)
    {
        fd = Fwl_FileOpenAsyn(filename, _FMODE_CREATE, _FMODE_CREATE);

    }
    else
    {
        fd = Fwl_FileOpen(filename, _FMODE_CREATE, _FMODE_CREATE);
    }
    
    if (_FOPEN_FAIL == fd)
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openTempIndexFile: open file failed.\n");
        return _FOPEN_FAIL;
    }

    else
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openTempIndexFile:fp=0x%x\n", fd);
    }
    
    if (isInMobileMedium)
    {
        Fwl_SetFileBufferSize(fd,32);
    }
    Fwl_SetFileSize(fd,40<<20);

    return fd;
}


static T_S32 closeTempIndexFile(T_MEncodeCtrl * hdl,T_BOOL isSave)
{
    AK_ASSERT_PTR(hdl, "Encode_MediaRecFileDelete: param err", AK_EBADPARAM);

    if (_FOPEN_FAIL != hdl->fd_index)
    {
        if (!isSave)
        {
           Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Del tmp index File(0x%x)..\n", hdl->fd_index);
           Fwl_FileDestroy(hdl->fd_index);
        }
        else
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Save tmp index File(0x%x)..\n", hdl->fd_index);
        }
        Fwl_FileClose(hdl->fd_index);
        hdl->fd_index = _FOPEN_FAIL;
    }
    return AK_SUCCESS;
}

static T_S32 closeRecFile(T_MEncodeCtrl * hdl,T_BOOL isSave, T_BOOL isAsyn)
{
    AK_ASSERT_PTR(hdl, "closeRecFile: param err", AK_EBADPARAM);

    if (_FOPEN_FAIL != hdl->fd)
    {
#if (1 == ENABLE_REC_NO_INTERVAL)
        if (isSave)
        {
           Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Save Rec File(0x%x)--\n", hdl->fd);
           Fwl_AsynCloseFile(hdl->fd);
        }
        else
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Del Rec File(0x%x)--\n", hdl->fd);
            Fwl_AsynDeleteFile(hdl->fd);
        }
#else
        if (!isSave)
        {
           Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Del Rec File(0x%x)..\n", hdl->fd);
           Fwl_FileDestroy(hdl->fd);
        }
        else
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE"Save Rec File(0x%x)..\n", hdl->fd);
        }
        
        Fwl_FileClose(hdl->fd);
#endif    
        hdl->fd = _FOPEN_FAIL;
    }
    return AK_SUCCESS;
}

static T_BOOL audioEncodeParamCheck(T_REC_AUDIO_INIT_PARAM *pAudioParam)
{
    AK_ASSERT_PTR(pAudioParam, STR_THIS_FILE "audioEncodeParamCheck: parameter error", AK_FALSE);

    if (pAudioParam->audioEncSamp < 8000)
    {
       pAudioParam->audioEncSamp = 8000;
    }
    
    return AK_TRUE;
}



static T_BOOL audioEncodeOpen(T_MEncodeCtrl * hdl,T_REC_AUDIO_INIT_PARAM *pAudioParam)
{
    T_AUDIO_REC_INPUT    *config;

    AK_ASSERT_PTR(hdl, STR_THIS_FILE "audioEncodeOpen: parameter error", AK_FALSE);

    /* config audio parameter */
    config = &(hdl->audio.cfgInputInfo);
    MRec_SetAudioRecInfo(config, pAudioParam->audioEncType);    
    config->enc_in_info.m_nSampleRate = pAudioParam->audioEncSamp;

    hdl->audio.outputBuf.pBuf    = (T_U8 *)Fwl_Malloc(AUDIO_OUTPUT_BUFFER_SIZE);
    AK_ASSERT_PTR(hdl->audio.outputBuf.pBuf, STR_THIS_FILE "audioEncodeOpen: malloc error", AK_FALSE);
    memset(hdl->audio.outputBuf.pBuf, 0, AUDIO_OUTPUT_BUFFER_SIZE);
    
    hdl->audio.outputBuf.bufSize= AUDIO_OUTPUT_BUFFER_SIZE;
    hdl->audio.outputBuf.dataLen= 0;

    hdl->audio.bufCtrl.buf_out    = hdl->audio.outputBuf.pBuf;
    hdl->audio.bufCtrl.len_out    = hdl->audio.outputBuf.bufSize;
    hdl->audio.bufCtrl.buf_in    = AK_NULL;
    hdl->audio.bufCtrl.len_in    = 0;

    hdl->audio.hEncodeDevice    = AK_NULL;
    hdl->audio.hStreamInDevice    = AK_NULL;

    if (_SD_MEDIA_TYPE_PCM == config->enc_in_info.m_Type)
        hdl->audio.cfgDuration        = 500;
    else
        hdl->audio.cfgDuration        = 500;
    
    hdl->audio.ctlNeedEncode    = ((_SD_MEDIA_TYPE_PCM == hdl->audio.cfgInputInfo.enc_in_info.m_Type) ? AK_FALSE : AK_TRUE);

    // 按字节算
    hdl->audio.needSize         = hdl->audio.cfgDuration * hdl->audio.cfgInputInfo.enc_in_info.m_nSampleRate * hdl->audio.cfgInputInfo.enc_in_info.m_nChannel * 2 / 1000;
    hdl->audio.count            = 0;

    waveInInit();
    return AK_TRUE;
}

static T_BOOL audioEncodeStart(T_MEncodeCtrl * hdl)
{
#ifdef OS_ANYKA
    T_WAVE_IN    waveInInput;

    AK_ASSERT_PTR(hdl, STR_THIS_FILE "audioEncodeStart: parameter error", AK_FALSE);

    if (hdl->audio.ctlNeedEncode)
    {
        hdl->audio.hEncodeDevice    = _SD_Encode_Open(&hdl->audio.cfgInputInfo, &hdl->audio.cfgOutputInfo);
        AK_ASSERT_PTR(hdl->audio.hEncodeDevice, STR_THIS_FILE "audioEncodeStart: _SD_Encode_Open error", AK_FALSE);
    }

    memset(&waveInInput,0,sizeof(waveInInput));
    waveInInput.waveInFmt.channel        = hdl->audio.cfgInputInfo.enc_in_info.m_nChannel;
    waveInInput.waveInFmt.sampleBits     = hdl->audio.cfgInputInfo.enc_in_info.m_BitsPerSample;
    waveInInput.waveInFmt.sampleRate     = hdl->audio.cfgInputInfo.enc_in_info.m_nSampleRate;
    waveInInput.volume = 1024;
    waveInInput.inputSrc = eINPUT_SOURCE_MIC;
    
    waveInOpen(&(hdl->audio.hStreamInDevice), &waveInInput);
#endif
    return AK_TRUE;
}

static T_VOID audioEncodeGetOffBufData(T_MEncodeCtrl * hdl, T_U32 throwtime)
{
	T_U32 totaldata;
	T_U32 tempdata;
	
	totaldata = (throwtime*2*(hdl->audio.cfgInputInfo.enc_in_info.m_nSampleRate)*(hdl->audio.cfgInputInfo.enc_in_info.m_nChannel))/1000;
	tempdata = 0;
	
	while (1)
    {
		if (waveInRead(hdl->audio.hStreamInDevice, (T_U8 **)(&hdl->audio.bufCtrl.buf_in), &hdl->audio.bufCtrl.len_in))
		{
			if (hdl->audio.bufCtrl.len_in > 1)
			{
				tempdata += hdl->audio.bufCtrl.len_in;						
			}
			
			if (tempdata >= totaldata)				
			{	
				Fwl_Print(C3, M_VIDEO, "audioEncodeGetOffBufData tempdata=%u,totaldata=%u\n",tempdata,totaldata);
				return;			
			}  
		}
	}
}


static T_BOOL audioEncodeStop(T_MEncodeCtrl * hdl)
{
    T_BOOL    ret = AK_FALSE;
    
#ifdef OS_ANYKA
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "audioEncodeStop: parameter error", AK_FALSE);

    if (hdl->audio.ctlNeedEncode && hdl->audio.count > 0)
    {
        while (hdl->audio.bufCtrl.len_in > 0)
        {
            MEnc_AudioPcmEncode(hdl);
        }

        if (hdl->audio.count > hdl->audio.needSize)
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "audioEncodeStop: error");
        }
        else if (hdl->audio.count > 0)
        {
            T_U32 i;
            T_U8 *p;

            i = hdl->audio.needSize - hdl->audio.count;

            p = Fwl_Malloc(2048);

            if (AK_NULL != p)
            {
                memset(p, 0, 2048);

                while (i > 0)
                {
                    if (i >= 2048)
                    {
                        hdl->audio.bufCtrl.len_in = 2048;
                        i -= 2048;
                    }
                    else
                    {
                        hdl->audio.bufCtrl.len_in = i;
                        i = 0;
                    }
                    
                    hdl->audio.bufCtrl.buf_in = p;

                    _SD_Encode(hdl->audio.hEncodeDevice, &(hdl->audio.bufCtrl));
                    hdl->audio.outputBuf.dataLen += hdl->audio.bufCtrl.len_out;
                    hdl->audio.bufCtrl.buf_out = ((T_U8 *)hdl->audio.outputBuf.pBuf + hdl->audio.outputBuf.dataLen);
                }

                MediaLib_Rec_ProcessAudio(hdl->hMedia, hdl->audio.outputBuf.pBuf, hdl->audio.outputBuf.dataLen);

                Fwl_Free(p);
            }
        }
    }

    if (waveInClose(hdl->audio.hStreamInDevice))
    {
        ret    = AK_TRUE;

        if (hdl->audio.ctlNeedEncode)
        {
            ret    = _SD_Encode_Close(hdl->audio.hEncodeDevice);
        }

        hdl->audio.outputBuf.dataLen= 0;
        hdl->audio.hEncodeDevice    = AK_NULL;
        hdl->audio.hStreamInDevice    = AK_NULL;

        hdl->audio.bufCtrl.buf_out    = hdl->audio.outputBuf.pBuf;
        hdl->audio.bufCtrl.len_out    = hdl->audio.outputBuf.bufSize;
        hdl->audio.bufCtrl.buf_in    = AK_NULL;
        hdl->audio.bufCtrl.len_in    = 0;

        hdl->audio.count            = 0;
    }
    else
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "audioEncodeStop: failed");
    }
#endif
    return ret;
}

static T_BOOL audioEncodeClose(T_MEncodeCtrl * hdl)
{
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "audioEncodeClose: parameter error", AK_FALSE);
    
    if (hdl->audio.outputBuf.pBuf)
    {
        Fwl_Free(hdl->audio.outputBuf.pBuf);
        hdl->audio.outputBuf.pBuf    = AK_NULL;
    }
    waveInDestroy();
    hdl->isAudioInit  = AK_FALSE;
    return AK_TRUE;
}




static T_U32 videoEncodeMaxRecTime(T_REC_CTRL_INIT_PARAM *pCtlParam)
{
    T_U32 timelimt_ms = 0;

    AK_ASSERT_PTR(pCtlParam, STR_THIS_FILE "videoEncodeMaxRecTime: parameter error", 0);
    if (pCtlParam->cycRecTimeMs > 0)
    {
        timelimt_ms = pCtlParam->cycRecTimeMs;
    }
    else
    {
        timelimt_ms = pCtlParam->recTimeSecLimit * 1000;
    }

    return timelimt_ms;
}


static T_U32 videoEncodeMaxRecSize(T_REC_CTRL_INIT_PARAM *pCtlParam, T_U32 resSize)
{
    //T_U16 driverId;
    T_U64_INT validFreeSize = {0};
    T_U64_INT usedSize = {0};
    T_U32 SizeMaxLimt = 0;
    
    AK_ASSERT_PTR(pCtlParam, STR_THIS_FILE "videoEncodeMaxRecSize: parameter error", 0);
    
    //设备空闲空间的大小
    Fwl_FsGetFreeSize(pCtlParam->recFilePath[0], &validFreeSize);

    if (pCtlParam->cycRecTimeMs > 0)
    {
        //文件夹下所有文件的大小
        Fwl_GetOldFileByCreateTime(pCtlParam->recFilePath,  AK_FALSE,0,
                                   AK_NULL,AK_NULL,AK_NULL,&usedSize);
        
        //<文件夹>和<设备空闲空间>总大小(可用空间)
        U64addU32(&validFreeSize,usedSize.high,usedSize.low);
    }

    //去除预留空间
    U64subU32(&validFreeSize,resSize);
    
    if (pCtlParam->cycRecTimeMs > 0)
    {        
		if (!pCtlParam->cycRecbySpace)
		{
			//计算可用空间的一半大小
        	U64RightShift(&validFreeSize,1);
			if (0 == validFreeSize.high)
	        {
	            SizeMaxLimt = validFreeSize.low;
	        }
	        //可用空间大于4G
	        else
	        {
	            SizeMaxLimt = pCtlParam->recSizeLimit;
	        }			
		}
		else
		{
			SizeMaxLimt = pCtlParam->cycRecbySpaceSize;
		}        
    }
    else
    {
        if (0 == validFreeSize.high)
        {
            SizeMaxLimt = validFreeSize.low;
        }
        
        //可用空间大于4G
        else
        {
            SizeMaxLimt = pCtlParam->recSizeLimit;
        }
    }
    
    if (SizeMaxLimt > pCtlParam->recSizeLimit)
    {
        SizeMaxLimt = pCtlParam->recSizeLimit;
    }
    
    else if (SizeMaxLimt < pCtlParam->recSizeLimit)
    {        
        if (SizeMaxLimt < resSize)
        {
            pCtlParam->cycRecbySpaceSize = SizeMaxLimt;
            pCtlParam->cycRecbySpace = AK_TRUE;
        }        
    }
    
    return SizeMaxLimt;

}


static T_U32 videoEncodeResSize(T_REC_CTRL_INIT_PARAM *pCtlParam)
{
    T_U32 ressize = 0;
    T_U64_INT totalSize = {0};

    AK_ASSERT_PTR(pCtlParam, STR_THIS_FILE "MEnc_GetReverseSize: parameter error", 0);
    
    
    //calculate the tmp Index file storage size  when encode 
    if (pCtlParam->cycRecTimeMs > 0)
    {
        ressize = INDEX_FILE_SIZE_SEC *(pCtlParam->cycRecTimeMs/1000);
    }
    else
    {
        ressize = INDEX_FILE_SIZE_SEC * pCtlParam->recTimeSecLimit;
    }

    //the min reverse size
    Fwl_FsGetSize(pCtlParam->recFilePath[0], &totalSize);
    
    // if  the storage of the driver is much than 4G byte
    if (totalSize.high > 0)
    {
        ressize += pCtlParam->recResSize;//(pCtlParam->recResSize * 2);
    }
    else
    {
        // if  storage much than 1G byte
        if (totalSize.low >= (1<<30))
        {
            ressize += pCtlParam->recResSize;//(pCtlParam->recResSize*2);
        }

        //  if  storage less than 1G byte
        else
        {
            ressize += pCtlParam->recResSize;
        }
    }

    Fwl_Print(C3, M_MENCODE, "videoEncodeResSize=%u\n",ressize);

    return  ressize;
}




static T_BOOL videoEncodeCheckSpace(T_MEncodeCtrl * hMRec,T_BOOL isAutoDelete,T_U32 reverseCount)
{
    T_USTR_FILE filePath;
    T_U64_INT  freeSize = {0};
    T_U32  reverseSizeMin = 0,limtsize = 0;
    T_BOOL noSpace = AK_FALSE;
    T_U32  fileBitSizePerSec = 0;
    T_BOOL isIncludeSubDir = AK_FALSE;
    T_U32  idx = 0;
    T_U32  recSec = 0;
    T_BOOL isSearched = AK_FALSE;
    
    AK_ASSERT_PTR(hMRec, STR_THIS_FILE "MEnc_CheckRecSpace: parameter error", AK_FALSE);

    //if driver not monted(meaning this device not insert or fixed)
    if (!Fwl_CheckDriverIsValid(hMRec->recCtlParam.recFilePath))
    {
        Fwl_Print(C3, M_MENCODE, "No Store Device For rec file!\n");
        return AK_FALSE;
    }

    if (!hMRec->recCtlParam.useMemIndex)
    {
        //if driver not monted(meaning this device not insert or fixed)
        if (!Fwl_CheckDriverIsValid(hMRec->recCtlParam.indexFilePath))
        {
            Fwl_Print(C3, M_MENCODE, "No Store Device For rec incexfile!\n");
            return AK_FALSE;
        }
    }

    //get the reverse size min limit
    reverseSizeMin            = videoEncodeResSize(&hMRec->recCtlParam);
    hMRec->recRunSizeByteLimt = videoEncodeMaxRecSize(&hMRec->recCtlParam, reverseSizeMin);
    hMRec->recRunTimeMsLimt   = videoEncodeMaxRecTime(&hMRec->recCtlParam);

    //size max limit is invalid
    if (0 == hMRec->recRunSizeByteLimt || hMRec->recRunSizeByteLimt > T_U32_MAX)
    {
        Fwl_Print(C3, M_MENCODE, "No Enough Storage,Reverse:%d,Limit:%d\n",reverseSizeMin,hMRec->recRunSizeByteLimt);
        return AK_FALSE;
    }
    
    //------------------
    //adjust vbps  size(byte), avoid vbps is too larger
    fileBitSizePerSec = hMRec->videoParam.vbps;
    
    if (fileBitSizePerSec <  VIDEO_ENCODE_VBPS_LIMIT)
    {
        fileBitSizePerSec = VIDEO_ENCODE_VBPS_LIMIT;
    }

    freeSize.high = 0;
    freeSize.low  = 0;
    Fwl_FsGetFreeSize(hMRec->recCtlParam.recFilePath[0],&freeSize);

    //------------------
    recSec = hMRec->recRunTimeMsLimt/1000;
    hMRec->recInfo.recMaxTimeSec = recSec;

    //------------------
    //calculate  encode  file size max limit by vbps(byte)
    if (recSec < VIDEO_ENCODE_MIN_TIME_SEC)
    {
        recSec = VIDEO_ENCODE_MIN_TIME_SEC;
    }
    
    if (hMRec->recCtlParam.cycRecTimeMs > 0)// cycle record use double cycle file size
    {
        limtsize = reverseSizeMin + VIDEO_SIZE_BYTE_SEC(recSec, fileBitSizePerSec)*2;
    }
    else // normal record use reverse file size
    {
        limtsize = reverseSizeMin;//reverseSizeMin*2;
    }
	
    Fwl_Print(C3, M_MENCODE,"first limtsize=%u,freeSize.high=%u,.low=%u\n",limtsize,freeSize.high,freeSize.low);
    //------------------
    noSpace = (U64cmpU32(&freeSize, limtsize) <= 0);

    if (isAutoDelete)
    {
        //if delete all file first 
        if (hMRec->recCtlParam.autoDelAllFile)
        {
            noSpace  = AK_TRUE;
        }

        // if free space not enough for record
        if ((!hMRec->recCtlParam.autoDelAllFile) \
            && noSpace && (hMRec->recCtlParam.cycRecTimeMs > 0)
            && (limtsize >= hMRec->recRunSizeByteLimt))
        {
        	limtsize = hMRec->recRunSizeByteLimt;
			
        	if (hMRec->recCtlParam.cycRecbySpace && (0 == freeSize.high) && (freeSize.low <= reverseSizeMin))
    		{    			
    			noSpace = AK_TRUE;
    		}
			else
			{	            
	            noSpace = (U64cmpU32(&freeSize, limtsize) <= 0);
			}          
        }

        Fwl_Print(C3, M_MENCODE, "DelMode=(%d,%d), Remain: 0x[%x,%x], Need: 0x%x(%d ms)\n",
            noSpace,hMRec->recCtlParam.autoDelAllFile,
            freeSize.high,freeSize.low,
            limtsize,hMRec->recRunTimeMsLimt);
    }
    
#ifdef TIME_STAMP_FILE_NAME
    isIncludeSubDir = !(hMRec->recCtlParam.cycRecTimeMs > 0);    
#else
    isIncludeSubDir = AK_FALSE;    
#endif    
    idx = 0;

#if (1 == ENABLE_REC_NO_INTERVAL)
    if (noSpace)
    {
        Fwl_AsynCloseFlushAll();
    }
#endif

    //delete old file until  there has enough space for store  encode file(this size is estimated)
    while (noSpace &&  isAutoDelete && (idx++ <1024))
    {
        T_USTR_FILE fileName;
        T_U32 reverseCnt = 0;
        
        if (isIncludeSubDir)
        {
            Utl_UStrCpyN(filePath,hMRec->recCtlParam.recFilePath, FS_MAX_PATH_LEN);
            isSearched = (Fwl_GetOldFileByCreateTime(hMRec->recCtlParam.recFilePath,  AK_TRUE,0,
                           fileName,AK_NULL,AK_NULL,AK_NULL) > 0);
            if (isSearched)
            {
                Utl_UStrCat(filePath, fileName);
                Utl_UStrCat(filePath, _T("/"));
            }
            reverseCnt = 0;
        }
        else
        {
            Utl_UStrCpyN(filePath,hMRec->recCtlParam.recFilePath, FS_MAX_PATH_LEN);
            isSearched = AK_TRUE;
            reverseCnt = reverseCount;
        }

        
        if (isSearched && Fwl_GetOldFileByCreateTime(filePath,  AK_FALSE,reverseCnt,
            fileName,AK_NULL,AK_NULL,AK_NULL) > 0)
        {
            Fwl_Print(C3, M_MENCODE, "Delete Old File...\n");
            Utl_UStrCat(filePath, fileName);
            Printf_UC(filePath);
            Fwl_FileDelete(filePath);
        }
        else if (isSearched && isIncludeSubDir)
        {
            Fwl_Print(C3, M_MENCODE, "Delete Old Dir...\n");
            Printf_UC(filePath);
            Fwl_FsRmDirTree(filePath);
        }
        else
        {
            break;
        }

        if (!hMRec->recCtlParam.autoDelAllFile)
        {
            freeSize.high = 0;
            freeSize.low  = 0;
            Fwl_FsGetFreeSize(hMRec->recCtlParam.recFilePath[0],&freeSize);
            Fwl_Print(C3, M_MENCODE, "Remain:0x[%x,%x] Need:0x%x\n",
                freeSize.high,freeSize.low,limtsize);
            noSpace = (U64cmpU32(&freeSize, limtsize) <= 0);
        }

        else
        {
            noSpace = AK_TRUE;
        }
    }

    //if delete all file and not enough space 
    if (noSpace && (hMRec->recCtlParam.autoDelAllFile))
    {
        freeSize.high = 0;
        freeSize.low  = 0;
        Fwl_FsGetFreeSize(hMRec->recCtlParam.recFilePath[0],&freeSize);
		Fwl_Print(C3, M_MENCODE, "second limtsize=%u,freeSize.high=%u,.low=%u\n",limtsize,freeSize.high,freeSize.low);
        noSpace = (U64cmpU32(&freeSize, limtsize) <= 0);
    }

    if (noSpace)
    {
        Fwl_Print(C3, M_MENCODE, "No Enough Storage.Need 0x%x,Remain 0x[%x,%x]\n",limtsize,freeSize.high,freeSize.low);
    }
    return !noSpace;
}



static T_BOOL videoEncodeParamCheck(T_REC_VIDEO_INIT_PARAM *pVideoParam,T_REC_CTRL_INIT_PARAM *pCtlParam)
{
    AK_ASSERT_PTR(pVideoParam, STR_THIS_FILE "videoEncodeParamCheck: parameter error", AK_FALSE);
    AK_ASSERT_PTR(pCtlParam, STR_THIS_FILE "videoEncodeParamCheck: parameter error", AK_FALSE);

    return AK_TRUE;
}


static T_VOID videoEncode_WaitCodec(T_VOID)
{
#ifdef OS_ANYKA
#ifdef IMG_YUV2JPEG_INT
    codec_wait_finish();
#else
    AK_Sleep(1);  
#endif
#endif
}


/*	此函数是注册给视频编码的delay回调的，
	实现音视频并行编码，提高效率，
	可能被调用0-N次。
	用g_bAudioEncoded标记控制，每帧数据只调用一次音频编码，
	若视频编码较快，未调用delay函数，
	则在MRec_FrameProc接口视频编码后再补一次音频编码
*/
static T_BOOL audioPcmEncode(T_U32 ulTicks)
{
	T_S32 ret = AK_EFAILED;

	//由于视频delay回调可能被调用多次，用标记控制，每帧数据只调用一次音频编码
	if (g_bAudioEncoded)
	{
		/*此处的sleep不能去掉，视频编码时默认delay回调要有一定耗时的，
		否则视频编码还未完成，而查询的循环很快被执行完，就会出现异常
		*/
		AK_Sleep(1);	
		return AK_TRUE;
	}
	
	ret = MEnc_AudioPcmEncode(g_pEncodehdl);

	g_bAudioEncoded = AK_TRUE;	//调用过音频编码了，标记置TRUE

	if (AK_SUCCESS == ret)
	{
		return AK_TRUE;
	}
	else
	{
		return AK_FALSE;
	}
}

static T_BOOL videoEncodeOpen(T_MEncodeCtrl * hdl,  T_REC_VIDEO_INIT_PARAM *pVideoParam, T_REC_CTRL_INIT_PARAM *pCtlParam)
{
#ifdef OS_ANYKA
    T_MEDIALIB_REC_OPEN_INPUT    openInput;
    T_MEDIALIB_REC_OPEN_OUTPUT    openOutput;
  
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeOpen: parameter error", AK_FALSE);

	g_pEncodehdl = hdl;
	

#ifdef IMG_YUV2JPEG_INT
    codec_intr_enable();
    Img_SetJpgTask_CB(videoEncode_WaitCodec);
#endif   
    /* setup video config information(i.e. recorder open input information) */
    memset((T_U8 *)&openInput, 0, sizeof(T_MEDIALIB_REC_OPEN_INPUT));
    openInput.m_hMediaDest                          = openRecFile(pCtlParam,pVideoParam, hdl->recFileName);

    if (_FOPEN_FAIL == openInput.m_hMediaDest)
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openInput.m_hMediaDest error.\n");
        return AK_FALSE;
    }

    hdl->fd                                         = openInput.m_hMediaDest;
    openInput.m_bCaptureAudio                       = AK_TRUE;
    openInput.m_bHighQuality                        = AK_FALSE;
    /* 内存计算：（帧率+每秒音频包数）×16是一秒的数据量, 多少秒就乘以多少，然后再加点余量 */
    if (pCtlParam->useMemIndex)
    {
        openInput.m_hIndexFile                       = 0;
        openInput.m_bIdxInMem                        = AK_TRUE;
        hdl->fd_index                                = _FOPEN_FAIL;
    }
    else
    {
        openInput.m_hIndexFile                       = openTempIndexFile(pCtlParam);
        if (_FOPEN_FAIL == openInput.m_hIndexFile)
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "openInput.m_hIndexFile error.\n");
            closeRecFile(hdl, AK_FALSE, AK_FALSE);
            return AK_FALSE;
        }
        openInput.m_bIdxInMem                       = AK_FALSE;
        hdl->fd_index                               = openInput.m_hIndexFile;
    }
    openInput.m_IndexMemSize                        = (pVideoParam->FPS + (1000 / hdl->audio.cfgDuration)) * 16 * pCtlParam->recTimeSecLimit + 1024;
    openInput.m_RecordSecond                        = 0;
    
    /* set video open info */
    openInput.m_VideoRecInfo.m_nWidth               = pVideoParam->videoWidth;
    openInput.m_VideoRecInfo.m_nHeight              = pVideoParam->videoHeight;
    openInput.m_VideoRecInfo.m_nFPS                 = pVideoParam->FPS;
    openInput.m_VideoRecInfo.m_nKeyframeInterval    = pVideoParam->keyfFameInterval;
    openInput.m_VideoRecInfo.m_nvbps                = pVideoParam->vbps;
	openInput.m_Reserved = 1;	//打开视频的delay回调
	
	switch (pVideoParam->videoEncType)
	{
		case eRECORD_MEDIA_AVI_MPEG4_PCM:
			openInput.m_MediaRecType 	= 	MEDIALIB_REC_AVI_NORMAL;
	        openInput.m_VideoRecInfo.m_eVideoType       = MEDIALIB_V_ENC_MPEG; // or MEDIALIB_V_ENC_H263,  MEDIALIB_V_ENC_MPEG; //MEDIALIB_V_ENC_MPEG; //
	        openInput.m_ExFunSetEncTask                 = AK_NULL;//Img_SetJPEGTaskFunc;    // NULL; // 
	        openInput.m_ExFunEnc                        = AK_NULL;            // NULL; // NULL; // 
			break;
		case eRECORD_MEDIA_AVI_MJPEG_PCM:
			openInput.m_MediaRecType 	= 	MEDIALIB_REC_AVI_NORMAL;
	        openInput.m_VideoRecInfo.m_eVideoType       = MEDIALIB_V_ENC_MJPG; // or MEDIALIB_V_ENC_H263,  MEDIALIB_V_ENC_MPEG; //MEDIALIB_V_ENC_MPEG; //
	        openInput.m_ExFunSetEncTask                 = AK_NULL;//Img_SetJPEGTaskFunc;    // NULL; // 
	        openInput.m_ExFunEnc                        = exFuncYUVENC;            // NULL; // NULL; // 
			break;
		case eRECORD_MEDIA_3GP_MPEG4_AMR:
			openInput.m_MediaRecType 	= 	MEDIALIB_REC_3GP;
	        openInput.m_VideoRecInfo.m_eVideoType       = MEDIALIB_V_ENC_MPEG; // or MEDIALIB_V_ENC_H263,  MEDIALIB_V_ENC_MPEG; //MEDIALIB_V_ENC_MPEG; //
	        openInput.m_ExFunSetEncTask                 = AK_NULL;//Img_SetJPEGTaskFunc;    // NULL; // 
	        openInput.m_ExFunEnc                        = AK_NULL;            // NULL; // NULL; // 
			break;
	}
	switch (hdl->audioParam.audioEncType)
	{
		case eRECORD_MODE_AMR:
		    openInput.m_AudioRecInfo.m_Type      =_SD_MEDIA_TYPE_AMR ;
			break;
		case eRECORD_MODE_WAV:
		    openInput.m_AudioRecInfo.m_Type      =_SD_MEDIA_TYPE_PCM;
			break;
		default:
		    openInput.m_AudioRecInfo.m_Type      =_SD_MEDIA_TYPE_PCM;
			break;
	}
    
    
    openInput.m_SectorSize                          = 2048;
    
    openInput.m_AudioRecInfo.m_nChannel             = 1;
    openInput.m_AudioRecInfo.m_BitsPerSample        = 16;
    openInput.m_AudioRecInfo.m_nSampleRate          = 8000;
    openInput.m_AudioRecInfo.m_ulDuration           = hdl->audio.cfgDuration; //500;
    
    /* set callback */
    Dmx_SetMediaLibCB(&openInput.m_CBFunc, AK_TRUE, AK_TRUE);

	//在视频编码的delay回调里做音频编码，实现音视频并行编码，提高效率
	openInput.m_CBFunc.m_FunRtcDelay = (MEDIALIB_CALLBACK_FUN_RTC_DELAY)audioPcmEncode;
    
#ifdef SUPPORT_VIDEOREC_AUDIO_DENOICE
	//只支持8k采样率，单通道的环境降噪
	if (openInput.m_AudioRecInfo.m_nSampleRate ==8000 &&
		openInput.m_AudioRecInfo.m_nChannel ==1)
	{
		T_AUDIO_FILTER_INPUT filter_input;
		
		Sd_SetAudioCB(&filter_input.cb_fun);
		filter_input.m_info.m_Type = _SD_FILTER_DENOICE;
		filter_input.m_info.m_SampleRate = openInput.m_AudioRecInfo.m_nSampleRate;
		filter_input.m_info.m_Channels = openInput.m_AudioRecInfo.m_nChannel;
		filter_input.m_info.m_BitsPerSample = openInput.m_AudioRecInfo.m_BitsPerSample;
		filter_input.m_info.m_Private.m_NR.NR_Level = 1;
		filter_input.m_info.m_Private.m_NR.ASLC_ena = 1;
		hdl->audio.hAudioFilter = _SD_Filter_Open(&filter_input);
		
		 Fwl_Print(C3, M_MENCODE,"Get audiofilter info:");
		_SD_GetAudioFilterVersions(&filter_input.cb_fun);
		 Fwl_Print(C3, M_MENCODE,"@open sound filter : pMEncoder->pAudFilter is %d\n", hdl->audio.hAudioFilter);
	}
	else
	{
		hdl->audio.hAudioFilter = AK_NULL;
	}
#endif	

    /* open recorder */
    hdl->hMedia = MediaLib_Rec_Open(&openInput, &openOutput);
    if (!(hdl->hMedia))
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "videoEncodeOpen: MediaLib_Rec_Open return NULL.\n");
        closeTempIndexFile(hdl, AK_FALSE);
        closeRecFile(hdl, AK_FALSE,AK_FALSE);
        return AK_FALSE;
    }
    
    if (!MediaLib_Rec_GetInfo(hdl->hMedia, &(hdl->recInfo.mediaInfo)))
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "videoEncodeOpen: MediaLib_Rec_GetInfo failed.\n");
        MediaLib_Rec_Close(hdl->hMedia);
        closeTempIndexFile(hdl, AK_FALSE);
        closeRecFile(hdl, AK_FALSE,AK_FALSE);
        return AK_FALSE;
    }

    hdl->recInfo.recStatus = MediaLib_Rec_GetStatus(hdl->hMedia);
#endif
    return AK_TRUE;
}

static T_BOOL videoEncodeStart(T_MEncodeCtrl * hdl)
{
#ifdef OS_ANYKA
	AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeStart: parameter error", AK_FALSE);
	AK_ASSERT_PTR(hdl->hMedia, STR_THIS_FILE "videoEncodeStart: error", AK_FALSE);
    
    if (!MediaLib_Rec_Start(hdl->hMedia))
    {
        Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "videoEncodeStart: failed");
        return AK_FALSE;
    }

    hdl->basePts           = 0;
    hdl->currPts           = 0;
    hdl->intervalPts       = 1000 / hdl->videoParam.FPS;
    hdl->firstStartup      = 0;
    hdl->curMaxInsVideoFrm = hdl->videoParam.FPS * VIDEO_FRAME_INSERT_PERCENT_DEFAULT / 100;

#endif
    return AK_TRUE;
}

static T_BOOL videoEncodeStop(T_MEncodeCtrl * hdl)
{
#ifdef OS_ANYKA
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeStop: parameter error", AK_FALSE);
    
    if (hdl->hMedia)
        MediaLib_Rec_Stop(hdl->hMedia);
#endif   
    return AK_TRUE;
}

static T_BOOL videoEncodeClose(T_MEncodeCtrl * hdl, T_BOOL isSave)
{
#ifdef OS_ANYKA
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeClose: parameter error", AK_FALSE);
    
    if (hdl->hMedia)
    {
        MediaLib_Rec_Close(hdl->hMedia);
        hdl->hMedia    = AK_NULL;
    }
    
    hdl->isVideoInit = AK_FALSE;
    closeTempIndexFile(hdl, AK_FALSE);
    closeRecFile(hdl, isSave,AK_FALSE);
#ifdef IMG_YUV2JPEG_INT
    codec_intr_disable();
    Img_SetJpgTask_CB(AK_NULL);
#endif    
#ifdef SUPPORT_VIDEOREC_AUDIO_DENOICE
	if (AK_NULL !=hdl->audio.hAudioFilter)
	{
		_SD_Filter_Close(hdl->audio.hAudioFilter);
		hdl->audio.hAudioFilter = AK_NULL;
		
	}
#endif

#endif   
    return AK_TRUE;
}

static T_VOID audioEncodeHandle(T_MEncodeCtrl * hdl)
{
#ifdef OS_ANYKA
    T_U32 temp = 0;

    if (hdl->audio.count > hdl->audio.needSize)
    {
        temp = hdl->audio.count - hdl->audio.needSize;
        hdl->audio.bufCtrl.len_in = hdl->audio.bufCtrl.len_in - temp;
    }

    if (hdl->audio.ctlNeedEncode)
    {
        _SD_Encode(hdl->audio.hEncodeDevice, &(hdl->audio.bufCtrl));
        hdl->audio.outputBuf.dataLen += hdl->audio.bufCtrl.len_out;
    }
    else
    {
        memcpy(hdl->audio.bufCtrl.buf_out, hdl->audio.bufCtrl.buf_in, hdl->audio.bufCtrl.len_in);
        hdl->audio.outputBuf.dataLen += hdl->audio.bufCtrl.len_in;
    }

    if (temp > 0)
    {
        hdl->audio.bufCtrl.buf_in = ((T_U8 *)hdl->audio.bufCtrl.buf_in + hdl->audio.bufCtrl.len_in);
    }
    
    hdl->audio.bufCtrl.len_in = temp;
    hdl->audio.bufCtrl.buf_out = ((T_U8 *)hdl->audio.outputBuf.pBuf + hdl->audio.outputBuf.dataLen);

    if (hdl->audio.count >= hdl->audio.needSize)
    {
        hdl->basePts += hdl->audio.cfgDuration;
        hdl->currPts = hdl->basePts;

        if (!MediaLib_Rec_ProcessAudio(hdl->hMedia, hdl->audio.outputBuf.pBuf, hdl->audio.outputBuf.dataLen))
        {
            Fwl_Print(C3, M_MENCODE, STR_THIS_FILE "MEnc_AudioPcmEncode: MediaLib_Rec_ProcessAudio error");
        }

        hdl->audio.outputBuf.dataLen = 0;
        hdl->audio.bufCtrl.buf_out = hdl->audio.outputBuf.pBuf;
        hdl->audio.count = hdl->audio.count - hdl->audio.needSize;
    }  
 #endif
}



/*
 * @brief   set video frame insert max
 * @author WangXi
 * @date	2011-11-23
 * @param[in/out] hdl: encoder handle
 * @param[in] isEnable:  if open the drop frame function
 * @return	T_BOOL AK_TRUE--success, else fail
 */
static T_BOOL videoEncodeSetMaxDropFrame(T_MEncodeCtrl * hdl, T_U32 maxInsertFrame)
{
#ifdef OS_ANYKA
    T_U32 maxInsertFrm = 0;
    T_U32 miniInsertFrm = 0;
    
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeSetMaxDropFrame: parameter error", AK_FALSE);

    // calculate the min insert frame
    miniInsertFrm = hdl->videoParam.FPS * VIDEO_FRAME_INSERT_PERCENT_DEFAULT / 100;
    maxInsertFrm  = maxInsertFrame;
    //adjust the max frame
    if (maxInsertFrm < miniInsertFrm)
    {
        maxInsertFrm = miniInsertFrm;
    }
    
    if ((AK_NULL != hdl->hMedia) && (maxInsertFrm != hdl->curMaxInsVideoFrm))
    {
        if (MediaLib_Rec_SetVideoMaxInsert(hdl->hMedia , maxInsertFrm))
        {
            hdl->curMaxInsVideoFrm = maxInsertFrm;
            Fwl_Print(C3, M_MENCODE, "VideoFrameDrop, Set MaxInsert is %d frames Ok.\n",
                                      hdl->curMaxInsVideoFrm);
        }
        else
        {
            Fwl_Print(C2, M_MENCODE, "VideoFrameDrop Error, Insert %d/%d frames.\n",
                                      maxInsertFrm, hdl->curMaxInsVideoFrm);
        }
    }
#endif   

    return AK_TRUE;
}

static T_BOOL videoEncodeYuv(T_pVOID handle, T_FRM_DATA *pSrcFrame, T_U32 interval)
{
    T_S32            video_time = 0;
#ifdef OS_ANYKA
    T_U32           videoCurPts = 0;
    T_MEncodeCtrl * hdl = (T_MEncodeCtrl *)handle;	
	T_U32 theused;
	T_U32 thebuffer;
	T_U32 thepercent;
    static T_U8 sense_cnt=0;//侦测计数
    static T_U8 encode_cnt=0;//编码帧数
    static T_BOOL sense_flag=AK_TRUE;//侦测标志
    static T_BOOL sense_cnt_flag=AK_FALSE;//侦测计数标志
    
    
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "videoEncodeYuv param Er", AK_FALSE);
    
    if (!waveInGetStatus(hdl->audio.hStreamInDevice, WAVEIN_CUR_TIME,&videoCurPts))
    {
        videoCurPts = 0;
    }

    if (0 == videoCurPts)
    {
       hdl->prePts = 0;
    }
    else if (videoCurPts == hdl->prePts)
    {
        Fwl_Print(C3, M_MENCODE, "#pts is fast\n");
        videoCurPts += interval;
    }
    else if (videoCurPts > (hdl->prePts + 150))
    {
        Fwl_Print(C3, M_MENCODE, "#pts is slowly.delay=%u/%u pts<%u-%u>\n",
                  videoCurPts - hdl->prePts, interval, videoCurPts,hdl->prePts);
    }
    //Fwl_Print(C3, M_MENCODE, "<M");
	if (Fwl_GetAsynBufInfo(&theused,&thebuffer))
	{
		thepercent = (10*theused)/thebuffer;
        
        if (sense_flag) // 1
        {
            if (sense_cnt_flag) // 2
            {
                if (thepercent < 8)
                {               
                    encode_cnt += 1;
                    
                    if (encode_cnt > 30)
                    {                    
                        encode_cnt = 0;
                        sense_cnt_flag = AK_FALSE; // 2
                    }

                    MediaLib_Rec_SetDropFrame(hdl->hMedia,0);
                }
                else
                {                
                    sense_cnt += 1;
                    encode_cnt = 0;
                    
                    if (sense_cnt >= 3)
                    {                    
                        sense_flag = AK_FALSE; // 3
                    }
                    else
                    {
                        MediaLib_Rec_SetDropFrame(hdl->hMedia,3);
                        MediaLib_Rec_SetBufferingInfo(hdl->hMedia, (thebuffer-theused));                                  
                    }
                }
            }            
            else
            {
                if (thepercent >= 8) // 1
                {
                    MediaLib_Rec_SetDropFrame(hdl->hMedia,3);
        			MediaLib_Rec_SetBufferingInfo(hdl->hMedia, (thebuffer-theused));

                    sense_cnt += 1;
                    sense_cnt_flag = AK_TRUE; // 2
                    encode_cnt = 0;
                }
                else
                {
                    MediaLib_Rec_SetDropFrame(hdl->hMedia,0);
                }
            }            
        }
        
        if (sense_cnt >= 3) // 3
        {
    	    switch(thepercent)
    	    {
    	        case 10:	            
    	        case 9:	            
    			case 8:
    	            MediaLib_Rec_SetDropFrame(hdl->hMedia,3);
    				MediaLib_Rec_SetBufferingInfo(hdl->hMedia, (thebuffer-theused));
    	            break;
    			case 7:	            
    			case 6:
    	            MediaLib_Rec_SetDropFrame(hdl->hMedia,2);
    				MediaLib_Rec_SetBufferingInfo(hdl->hMedia, (thebuffer-theused));
    	            break;
    			case 5:	            
    			case 4:	            
    			case 3:
    	            MediaLib_Rec_SetDropFrame(hdl->hMedia,1);
    				MediaLib_Rec_SetBufferingInfo(hdl->hMedia, (thebuffer-theused));
    	            break;
    			case 2:	            
    			case 1:	            
    	        default:
    				MediaLib_Rec_SetDropFrame(hdl->hMedia,0);
    	            break;
    		}
        }
	}
	else
	{	
		MediaLib_Rec_SetDropFrame(hdl->hMedia,0);
	}

    video_time = MediaLib_Rec_ProcessVideo(hdl->hMedia, (T_U8 *)(pSrcFrame->pBuffer), videoCurPts);
    
    hdl->prePts              = videoCurPts;
#endif
    //Fwl_Print(C3, M_MENCODE, ">");
    return (video_time >= 0);
}


T_S32 MEnc_AudioPcmEncode(T_HANDLE handle)
{
    T_S32         ret = AK_EFAILED;
#ifdef OS_ANYKA
    T_MEncodeCtrl * hdl = (T_MEncodeCtrl *)handle;
	T_BOOL		readflag = AK_FALSE;
	T_U8		readcnt = 0;
    
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "MEnc_AudioPcmEncode param Er", AK_EBADPARAM);

    /* encode */
    while (0 != hdl->audio.bufCtrl.len_in)
    {
        audioEncodeHandle(hdl);
    }

	/*为防止没有读到音频数据时，导致音频时间戳没变化，
	视频经常需要调整，而产生音视频同步异常问题，
	这里用readflag标记控制，要保证读到音频数据。
	*/
	for (readcnt=0; readcnt<10; readcnt++)
	{
    	while (waveInRead(hdl->audio.hStreamInDevice, &(hdl->audio.bufCtrl.buf_in), &(hdl->audio.bufCtrl.len_in)))
	    {
	    	readflag = AK_TRUE;	//读到音频数据，readflag置TRUE
			
#ifdef SUPPORT_VIDEOREC_AUDIO_DENOICE

			if(AK_NULL != hdl->audio.hAudioFilter)
			{
				T_AUDIO_FILTER_BUF_STRC filter_buf;

				filter_buf.buf_in  = (hdl->audio.bufCtrl.buf_in);
				filter_buf.len_in  = (hdl->audio.bufCtrl.len_in);
				filter_buf.buf_out = (hdl->audio.bufCtrl.buf_in);
				filter_buf.len_out = (hdl->audio.bufCtrl.len_in);
				
				_SD_Filter_Control(hdl->audio.hAudioFilter,&filter_buf);
			}
#endif			
	    
	        if (0 == hdl->firstStartup)
	        {
				audioEncodeGetOffBufData(hdl, 500);
				hdl->audio.bufCtrl.len_in = 0;
	            hdl->firstStartup = 1;
	            return AK_EFAILED;
	        }
	        
	        ret = AK_SUCCESS;
	        hdl->audio.count += hdl->audio.bufCtrl.len_in;
	        waveInSetStatus(hdl->audio.hStreamInDevice, WAVEIN_BUF_COUNT, &(hdl->audio.bufCtrl.len_in));
	        audioEncodeHandle(hdl);

	        while (hdl->audio.bufCtrl.len_in)
	        {
	            audioEncodeHandle(hdl);
	        }

	    }

		if (readflag)
		{
			break;
		}
		else
		{
			//此处的sleep不能去掉，否则执行太快，循环N次还是读不到音频数据
			AK_Sleep(1);
		}

	}
#endif

	return ret;
}




T_S32 MEnc_VideoFrameEncode(T_HANDLE handle,T_FRM_DATA *pSrcFrame)
{
    T_BOOL ret = AK_FALSE;
#ifdef OS_ANYKA
    T_MEncodeCtrl * hdl = (T_MEncodeCtrl *)handle;
    T_U32       interval = 0;

    AK_ASSERT_PTR(pSrcFrame, "MEnc_VideoFrameEncode: src err", AK_EBADPARAM);
    AK_ASSERT_PTR(pSrcFrame->pBuffer, "MEnc_VideoFrameEncode: src buffer err", AK_EBADPARAM);
    AK_ASSERT_PTR(hdl, STR_THIS_FILE "MEnc_VideoFrameEncode param Er", AK_EBADPARAM);

    if (FORMAT_YUV420 != pSrcFrame->info.type)
    {
        Fwl_Print(C3, M_MENCODE, "type error");
        return AK_EBADPARAM;
    }

    hdl->curEncodeSysTime = get_tick_count();//hdl->recInfo.mediaInfo.total_time_ms;//
    
    interval = hdl->curEncodeSysTime - hdl->preEncodeSysTime;
    //if need not encode video frame
    if ((hdl->isVideoNeedDrop) && (hdl->recCtlParam.encMsLimitPerFrame > 0))
    {
        if ((interval + hdl->intervalPts) <= hdl->recCtlParam.encMsLimitPerFrame)
        {
//	        Fwl_Print(C3, M_MENCODE, "drop one frame!");
            return AK_SUCCESS;
        }
    }
    
    hdl->preEncodeSysTime = hdl->curEncodeSysTime;

    ret = videoEncodeYuv(hdl, pSrcFrame, interval);
    
#endif

    if (ret)
    {
        return AK_SUCCESS;
    }
    
    else
    {
         return AK_EFAILED;
    }
}

static T_S32 exFuncYUVENC(T_U8 *srcStream, T_U8 *dstStream, T_U32 *pSize, T_U32 pic_width, T_U32 pic_height, T_U32 quality)
{
    T_BOOL       ret = AK_FALSE;
#ifdef OS_ANYKA
    IMG_T_U8    *psrcY;
    IMG_T_U8    *psrcU;
    IMG_T_U8    *psrcV;
//	J_OSD_Info  *pOSDTime;	

    quality = (quality >= 15)?quality:15; //quality 5~85

	psrcY    = srcStream;
    psrcU    = psrcY + (pic_width * pic_height);
    psrcV    = psrcU + ((pic_width * pic_height) >> 2);
	
    ret = Img_YUV2JPEG_Stamp_Mutex(psrcY, psrcU,psrcV, dstStream, pSize,pic_width, pic_height, quality, AK_NULL);
	
/*
    if (Is_04CHIP())
	{
	    psrcY    = pSrcFrame->pBuffer;
        psrcU    = psrcY + (pSrcFrame->info.width * pSrcFrame->info.height);
        psrcV    = psrcU + ((pSrcFrame->info.width * pSrcFrame->info.height) >> 2);

        pOSDTime = &(pVideoRec->pJ_OSD_Info_Zoom);
        
        OSD_Obtain_Semaphore();            
        ret = Img_YUV2JPEG_OSD_Mutex(psrcY, psrcU, psrcV, dstStream, pSize, pSrcFrame->info.width, pSrcFrame->info.height, pic_width, pic_height, quality, pOSDTime);
        OSD_Release_Semaphore();
    }
    else
    {
        
        psrcY    = pSrcFrame->pBuffer;
        psrcU    = psrcY + (pic_width * pic_height);
        psrcV    = psrcU + ((pic_width * pic_height) >> 2);

        ret = Img_YUV2JPEG_Mutex(psrcY, psrcU, psrcV, dstStream, pSize, pic_width, pic_height, quality);
    }
*/
#endif
    return ret;
}

#endif

T_VOID MRec_SetAudioRecInfo(T_AUDIO_REC_INPUT *audioRec, T_eREC_MODE audioType)
{
    Sd_SetCodecCB(&audioRec->cb_fun);

    audioRec->enc_in_info.m_BitsPerSample            = 16;
    audioRec->enc_in_info.m_nChannel                = 1;
    
    switch (audioType)
    {
    case eRECORD_MODE_AMR:        
        audioRec->enc_in_info.m_nSampleRate                = AUDIOREC_SAMPLE_RATE_AMR;
        audioRec->enc_in_info.m_private.m_amr_enc.mode    = AMR_ENC_MR515; //AMR_ENC_MR122;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_AMR;
        break;
        
    case eRECORD_MODE_ADPCM_IMA:
        audioRec->enc_in_info.m_nSampleRate                = gs.AudioRecordRate;
        audioRec->enc_in_info.m_private.m_adpcm.enc_bits= 4;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_ADPCM_IMA;
        break;

    case eRECORD_MODE_MP3:
        audioRec->enc_in_info.m_nSampleRate             = gs.AudioRecordRate;
        audioRec->enc_in_info.m_Type                     = _SD_MEDIA_TYPE_MP3;
        audioRec->enc_in_info.m_private.m_mp3.bitrate     = 0;
        audioRec->enc_in_info.m_private.m_mp3.mono_from_stereo = 0;
        break;
    case eRECORD_MODE_AAC:
        audioRec->enc_in_info.m_nSampleRate             = gs.AudioRecordRate;
        audioRec->enc_in_info.m_Type                     = _SD_MEDIA_TYPE_AAC;
        break;
        
    case eRECORD_MODE_WAV:
    default:
        audioRec->enc_in_info.m_nSampleRate                = gs.AudioRecordRate;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_PCM;
        break;
    }
}





T_S32 MEnc_FrameAddStampOnYuv(T_FRM_DATA *pDestFrame, T_eFONT setFont)
{
    T_U8        strbuf[30] = {0};
	T_U16 		Ustrbuf[30] = {0};
	T_U32 		timeWidth;
	T_SYSTIME	systime;
	T_eFONT     fontType = FONT_16;
    T_U32 		fontHeight = 0;
    T_U32 		size = 0;
    T_U16 		len = 0;
    T_U8 		*y = AK_NULL;
    T_U8 		*u = AK_NULL;
    T_U8 		*v = AK_NULL;
    T_POS 		xPos = 0;
    T_POS 		yPos = 0;
    
    AK_ASSERT_PTR(pDestFrame, "Zoom_FrameScale: dest err", AK_EBADPARAM);
    AK_ASSERT_PTR(pDestFrame->pBuffer, "Zoom_FrameScale: dest err", AK_EBADPARAM);

    if (FORMAT_YUV420 != pDestFrame->info.type)
    {
        Fwl_Print(C3, M_VZOOM, "type error");
        return AK_EBADPARAM;
    }
	
    size = (pDestFrame->info.width* pDestFrame->info.height);
    y = pDestFrame->pBuffer;
    u = (T_U8 *)y + size; 
    v = (T_U8 *)u + (size >> 2); 
    
	systime = GetSysTime();
    sprintf(strbuf,"%04d-%02d-%02d %02d:%02d:%02d",
        systime.year,systime.month,systime.day,
        systime.hour,systime.minute,systime.second);    

    if (setFont < FONTLIB_NUM)
    {
        fontType = setFont;
    }
    else
    {
        if (pDestFrame->info.rect.width >= 640)
        {
            fontType   = FONT_16;
        }		
        else
        {
            fontType   = FONT_12;
        }
    }

	Eng_StrMbcs2Ucs(strbuf, Ustrbuf);
	len = (T_U16)Utl_UStrLen(Ustrbuf); 
	timeWidth = UGetSpeciStringWidth(Ustrbuf, (T_FONT)fontType, (T_U16)len);
	fontHeight = (T_U32)GetFontHeight((T_U8)fontType);
    xPos = STAMP_LEFT;
    yPos = STAMP_TOP;
	Fwl_UDispSpeciStringOnYUV(y, u, v, pDestFrame->info.width, pDestFrame->info.height,
					xPos, yPos, Ustrbuf, (T_COLOR)COLOR_WHITE, (T_FONT)fontType, (T_U16)len); 

    return AK_SUCCESS;
}


T_S32 MEnc_GetFileName(T_USTR_FILE curPath, T_BOOL isMkSubFolder, 
                 T_STR_NAME preName, T_STR_NAME sufName, T_pWSTR pFileName)
{
    T_STR_INFO tmpstr;
    T_USTR_FILE filePath, FolderName, dateName, timeName ;
    T_U16 num = 0;
    T_SYSTIME sysTime;

    /** make directory */
    if (!Fwl_FsMkDirTree(curPath))
    {
        Printf_UC(curPath);
        Fwl_Print(C3, M_MENCODE, "MEnc_GetFileName:MkDirTree Err.\n");
        return AK_EFAILED;
    }
    Utl_UStrCpy(FolderName, curPath);
    sysTime = GetSysTime();
    // Date Directory
    if (isMkSubFolder)
    {
        sprintf(tmpstr, "%04d%02d%02d", sysTime.year, sysTime.month, sysTime.day);
        Eng_StrMbcs2Ucs(tmpstr, dateName);
        Utl_UStrCat(FolderName, dateName);
        Utl_UStrCat(FolderName, _T("/"));
        if (!Fwl_FsMkDirTree(FolderName))
        {
            Printf_UC(FolderName);
            Fwl_Print(C3, M_MENCODE, "MEnc_GetFileName:MkDirTree Err.\n");
            return AK_EFAILED;
        }

        // Time File Name
        sprintf(tmpstr, "%s_%02d%02d%02d", preName, sysTime.hour, sysTime.minute, sysTime.second);
    }
    else
    {
        // 循环录像
        sprintf(tmpstr, "%s_%04d%02d%02d%02d%02d%02d", preName, sysTime.year, sysTime.month, sysTime.day, sysTime.hour, sysTime.minute, sysTime.second);
    }
    
    Eng_StrMbcs2Ucs(tmpstr, timeName);
    do
    {    
        Utl_UStrCpy(filePath, FolderName);
        Utl_UStrCat(filePath, timeName);

        if (num++ > 0)
        {
            sprintf(tmpstr, "_%u", num);
            Utl_UStrCat(filePath, _T(tmpstr));
        }        

        // Suffix Name
        Utl_UStrCat(filePath, _T(sufName));
        if (!Fwl_FileExist(filePath))
        {
            // The File Path Is NOT Exist, OK
            break;
        }
    }while (AK_TRUE && num < 1024);

    if (num >= 1024)
    {
        Fwl_Print(C3, M_MENCODE, "MEnc_GetFileName Get File Name Failure.\n");
        
        return AK_EFAILED;
    }
    
    Printf_UC(filePath);
    Utl_UStrCpy(pFileName, filePath);

    return AK_SUCCESS;
}




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
T_BOOL MRec_GetStampFileName(T_USTR_FILE curPath, T_STR_NAME preName, T_STR_NAME sufName, T_pWSTR pFileName)
{
    T_pFILE fp;
    T_STR_INFO tmpstr;
    T_USTR_FILE filePath, foldername, dateName, timeName ;
    T_U16 num = 0;
    T_SYSTIME sysTime;
    
    /** make directory */
    if (!Fwl_FsMkDirTree(curPath))
    {
        return AK_FALSE;
    }

    Utl_UStrCpy(foldername, curPath);
    sysTime = GetSysTime();

    // Date Directory
    sprintf(tmpstr, "%04d%02d%02d", sysTime.year, sysTime.month, sysTime.day);
    Eng_StrMbcs2Ucs(tmpstr, dateName);
    
    Utl_UStrCat(foldername, dateName);
    Utl_UStrCat(foldername, _T("/"));
    Fwl_FsMkDirTree(foldername);

    if (!Fwl_FsMkDirTree(foldername))
    {
        return AK_FALSE;
    }
    // Time File Name
    sprintf(tmpstr, "%s_%02d%02d%02d", preName, sysTime.hour, sysTime.minute, sysTime.second);
    Eng_StrMbcs2Ucs(tmpstr, timeName);

    do
    {        
        Utl_UStrCpy(filePath, foldername);
        Utl_UStrCat(filePath, timeName);

        if (num++ > 0)
        {
            sprintf(tmpstr, "_%u", num);
            Utl_UStrCat(filePath, _T(tmpstr));
        }        

        // Suffix Name
        Utl_UStrCat(filePath, _T("."));
        Utl_UStrCat(filePath, _T(sufName));
     
        if (_FOPEN_FAIL == (fp = Fwl_FileOpen(filePath, _FMODE_READ, _FMODE_READ)))
        {
            // The File Path Is NOT Exist, OK
            break;
        }

        Fwl_FileClose(fp);
        
    }while (AK_TRUE && num < 1024);
    
    if (num >= 1024)
    {
        Fwl_Print(C3, M_MENCODE, "Get System Time File Name Failure.\n");
        
        return AK_FALSE;
    }
    
    Fwl_Print(C3, M_MENCODE, "->ADD file :");
    Printf_UC(filePath);

    Utl_UStrCpy(pFileName, filePath);

    return AK_TRUE;
}






