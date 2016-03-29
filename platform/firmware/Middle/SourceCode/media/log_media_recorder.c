/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name:log_media_recorder.c
 * Function: video recorder logic
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
#include "log_media_recorder.h"

//=================  Macro  Define (Debug)===========================
#ifdef CAMERA_SUPPORT

extern T_BOOL g_bAudioEncoded;

extern T_BOOL Is_04CHIP(T_VOID);

#ifdef PLATFORM_DEBUG_VER
#define MREC_DEBUG_VER
#endif
/**
  watch dog time(second), if zero will close
 */
#ifdef MREC_DEBUG_VER
#define MREC_TASK_WATCH_DOG_SEC   (4)
#endif

#ifdef MREC_DEBUG_VER
#define STR_THIS_FILE        "log_media_recorder"
#else
#define STR_THIS_FILE        ""
#endif

//#define DEBUG_MREC_DRYRUN    
//#define DEBUG_MREC_TRACERUN


//====================== Macro  Define ===========================
//-------------- Task Runtime Parameter Define--- 
/**
 * media record  task priority
 */
#define MREC_TASK_PRIO            (90)

/**
 * media record task time slice
 */
#define MREC_TASK_SLICE           (2)

/**
 *media record task run stack size(byte)
 */
#define MREC_TASK_STACK           (16*1024)

//--------------Media Record Task Configure Define--- 
/**
 * media record task name length
 */
#define MREC_TASK_NAME_LEN        (12)

//==================== Macro  Define (Ctrl)=========================
//--------------Media Record Task Status Define--- 
/**
 *task status: init status, meaning task is no ready
 */
#define EVT_MREC_NULL        (0)

/**
 *task status: suspend, meaning task is do nothing
 */
#define EVT_MREC_SUSPEND     (0x01<<1)

/**
 *task status: run, meaning task is do someting
 */
#define EVT_MREC_RUN         (0x01<<2)

/**
 *task status: close, meaning task is cosed
 */
#define EVT_MREC_CLOSE       (0x01<<3)



//===================== Type Define =============================
/**
 * video recorder control  struct
 */
typedef struct {
    ISubThread       *trd; // media recorder thread handle
    T_hSemaphore     mutex;// thread  share zone protect mutex
    T_S8             name[MREC_TASK_NAME_LEN];//this task name

    _VOLATILE T_HANDLE SrcCtrl;// extern zoom moudule handle
    T_HANDLE          EncodeHdl;// inner media encode proc handle
    _VOLATILE T_U32   runStatus;// current task status
    T_U32             NoMovingTimeLimt;// record detect motion how long time is static pic
    T_U32             NoMovingTimsMs;// current static pic time
} T_MRecordrCtrl;


//====================== Macro  Define(Function)=====================
#define MREC_MUTEX_INIT(mutex)       ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define MREC_MUTEX_IS_ERR(mutex)     (((mutex) <= 0) && ((mutex) > -100))
#define MREC_MUTEX_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define MREC_MUTEX_TRY_LOCK(mutex)    (0 < (AK_Try_Obtain_Semaphore((mutex), AK_SUSPEND)))
#define MREC_MUTEX_UNLOCK(mutex)      AK_Release_Semaphore((mutex))
#define MREC_MUTEX_DEINIT(mutex)      {\
    if (!(MREC_MUTEX_IS_ERR(mutex)))\
    {\
        AK_Delete_Semaphore(mutex);\
        (mutex) = 0;\
    }\
}
#define VIDEO_MREC_STOP(param)         VME_ReTriggerUniqueEvent(USER_EVT_STOP_REC, param)

//=======================Local Function  Declare=====================
/*
 * @brief   update video recorder task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder task handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_CtrlEvtUpdate(T_MRecordrCtrl * hdl, T_U32 status);

/*
 * @brief   set video recorder task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder task handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_CtrlEvtSet(T_MRecordrCtrl * hdl, T_U32 status);

/*
 * @brief   video recorder process for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 */
static T_VOID  MRec_TaskHandle(T_MRecordrCtrl * hdl);

/*
 * @brief   create recorder  task
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @param[in] isStart: is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_TaskCreate(T_MRecordrCtrl * hdl, T_BOOL isStart);

/*
 * @brief   destory recorder task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_TaskDestory(T_MRecordrCtrl * pCtrl);

/*
 * @brief   motion detect process when recorder  
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else detect time overflow limit
 */
static T_S32   MRec_MotionDetectCheck(T_MRecordrCtrl * pCtrl, T_BOOL *isMove);

/*
 * @brief   process frame whitch get from zoom stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_FrameProc(T_FRM_DATA *pFrame,    T_MRecordrCtrl *hdl);

/*
 * @brief   post process recorder zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of encode stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_ZoomPostProc(T_FRM_DATA *pFrame, T_HANDLE hdl);

/*
 * @brief   prepare process recorder zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of encode stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_ZoomPreProc(T_FRM_DATA *pFrame, T_HANDLE hdl);

//==================== Function  Define ===================
/*
 * @brief   update video recorder task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder task handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_CtrlEvtUpdate(T_MRecordrCtrl * hdl, T_U32 status)
{
    AK_ASSERT_PTR(hdl, "Param err", AK_EBADPARAM);

    // directly set, need not wait
    if (status != hdl->runStatus)
    {
        Fwl_Print(C3, M_MENCODE, "MRec::update event 0x%x begin.\n", status);
        hdl->runStatus = status;
        Fwl_Print(C3, M_MENCODE, "MRec::update event Ok\n", status);
    }
    
    return AK_SUCCESS;
}

/*
 * @brief   set video recorder task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder task handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_CtrlEvtSet(T_MRecordrCtrl * hdl, T_U32 status)
{
    AK_ASSERT_PTR(hdl, "Param err", AK_EBADPARAM);

    if (hdl->runStatus == status)
    {
        Fwl_Print(C3, M_MENCODE, "MRec status has seted\n");
        return AK_SUCCESS;
    }

    // directly set, need not wait
    if (EVT_MREC_RUN != hdl->runStatus)
    {
       hdl->runStatus = status;
       Fwl_Print(C3, M_MENCODE, "MRec@@set event Ok\n");
       return AK_SUCCESS;
    }
    
    /*else if ((AK_NULL != hdl->trd)\
            && (AK_READY == ISubThread_GetState(hdl->trd)))
    {
        Fwl_Print(C3, M_MENCODE, ">>set event 0x%x begin...\n", status);
        MREC_MUTEX_LOCK(hdl->mutex);
        hdl->runStatus = status;
        MREC_MUTEX_UNLOCK(hdl->mutex);
        Fwl_Print(C3, M_MENCODE, "<<set event Ok\n", status);

    }*/
    else
    {
        Fwl_Print(C3, M_MENCODE, "MRec::set event 0x%x begin...\n", status);
        MREC_MUTEX_LOCK(hdl->mutex);
        hdl->runStatus = status;
        MREC_MUTEX_UNLOCK(hdl->mutex);
        Fwl_Print(C3, M_MENCODE, "MRec::set event Ok\n", status);
    }
    return AK_SUCCESS;
}


/*
 * @brief   post process recorder zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of encode stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_ZoomPostProc(T_FRM_DATA *pFrame, T_HANDLE hdl)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "MRec_ZoomPostProc: param err", AK_EBADPARAM);

	AK_ASSERT_PTR(pFrame, "MRec_ZoomPostProc: param err", AK_EBADPARAM);

    // if support dynamic change camera focus, stamp will past at after zoom process
    if (VideoZoom_IsEnableFocusWin(pCtrl->SrcCtrl))
    {
        MEnc_FrameAddStampOnYuv(pFrame,0xFF);
    }

	return AK_SUCCESS;
}



/*
 * @brief   prepare process recorder zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of encode stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_ZoomPreProc(T_FRM_DATA *pFrame, T_HANDLE hdl)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "MRec_ZoomPostProc: param err", AK_EBADPARAM);

	AK_ASSERT_PTR(pFrame, "MRec_ZoomPostProc: param err", AK_EBADPARAM);

    // if  not support dynamic change camera focus, stamp will past at before zoom process
    if (!VideoZoom_IsEnableFocusWin(pCtrl->SrcCtrl))
    {
        MEnc_FrameAddStampOnYuv(pFrame,0xFF);
    }

	return AK_SUCCESS;
}



/*
 * @brief   motion detect process when recorder  
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else detect time overflow limit
 */
static T_S32 MRec_MotionDetectCheck(T_MRecordrCtrl * pCtrl, T_BOOL *isMove)
{
    T_BOOL isNotMoving  = AK_FALSE;
    T_U32  t = 0;

    if ((AK_NULL != pCtrl) && (pCtrl->NoMovingTimeLimt > 0))
    {
       t = get_tick_count();
       isNotMoving = VideoZoom_DetectStatusCheck(pCtrl->SrcCtrl, AK_FALSE);

       (*isMove) = !isNotMoving;

       // moving, update the latest system time
       if (!isNotMoving)
       {
           pCtrl->NoMovingTimsMs    = t;
       }
       // not moving,  cmp with pre nomoving  systime
       else if ((t - pCtrl->NoMovingTimsMs) >= pCtrl->NoMovingTimeLimt)
       {
           Fwl_Print(C3, M_MENCODE, "Detect Over time\n");
           pCtrl->NoMovingTimsMs    = t;
           return AK_EFAILED;
       }
    }
    
    return AK_SUCCESS;
}


/*
 * @brief   create recorder  task
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @param[in] isStart: is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   MRec_TaskCreate(T_MRecordrCtrl * pCtrl, T_BOOL isStart)
{
    T_S8 *pName = "Encode";
    T_SUBTHREAD_INITPARAM    param;

    AK_ASSERT_PTR(pCtrl, "Param err", AK_EBADPARAM);


    sprintf(pCtrl->name, "%s", pName);

    MREC_MUTEX_INIT(pCtrl->mutex);
    if (MREC_MUTEX_IS_ERR(pCtrl->mutex))
    {
        Fwl_Print(C3, M_MENCODE, "media create Muex@%d\n",__LINE__);
        return AK_EFAILED;
    }

    param.byPriority       = MREC_TASK_PRIO;
    param.ulTimeSlice      = MREC_TASK_SLICE;
    param.ulStackSize      = MREC_TASK_STACK;
    param.wMainThreadCls   = AKAPP_CLSID_MEDIA;

    param.pUserData        = (T_pVOID)pCtrl;
    param.fnEntry          = MRec_TaskHandle;
    param.fnAbort          = AK_NULL; 
    param.pcszName         = pName;

    if(AK_SUCCESS != CSubThread_New(&(pCtrl->trd), &param, AK_FALSE))
    {
        Fwl_Print(C3, M_MENCODE, "Create %s_Thread Sub Thread Failure!\n",param.pcszName);
        return AK_EFAILED;
    }


    return AK_SUCCESS;
}


/*
 * @brief   destory recorder task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  MRec_TaskDestory(T_MRecordrCtrl * pCtrl)
{
    if (AK_NULL == pCtrl)
    {
        return AK_SUCCESS;
    }
          

    if (AK_NULL != pCtrl->trd)
    {
        ISubThread_Terminate(pCtrl->trd);
        ISubThread_Exit(pCtrl->trd);
        pCtrl->trd = AK_NULL;
    }
    
    if (!MREC_MUTEX_IS_ERR(pCtrl->mutex))
    {
        MREC_MUTEX_DEINIT(pCtrl->mutex);
    }
    
    
    return AK_SUCCESS;
}


////////////////////////////////////////////////////////
/*
 * @brief   close video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSave:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Close(T_HANDLE handle, T_BOOL isSave)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;

    AK_ASSERT_PTR(pCtrl, "MRec_Close: param err", AK_EBADPARAM);

    MREC_MUTEX_LOCK(pCtrl->mutex);
    VideoZoom_CloseEncSrc(pCtrl->SrcCtrl);
    MRec_CtrlEvtUpdate(pCtrl,EVT_MREC_CLOSE);
    MREC_MUTEX_UNLOCK(pCtrl->mutex);

    MRec_TaskDestory(pCtrl);
    if (pCtrl->EncodeHdl)
    {
        MEnc_Close(pCtrl->EncodeHdl, isSave);
        pCtrl->EncodeHdl = 0;
    }
    Fwl_Free(pCtrl);
    AK_DEBUG_OUTPUT("MRec_Close Ok!\n");

    return AK_SUCCESS;
}

/*
 * @brief   open video recorder handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pAudioParm: default audio encode param	
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE MRec_Open(T_REC_AUDIO_INIT_PARAM *pAudioParm)
{
    T_MRecordrCtrl    *hdl    = AK_NULL;
    T_S32         lRet   = AK_EFAILED;

    if (0 == pAudioParm)
    {
        Fwl_Print(C3, M_MENCODE, "Param Err ,pAudioParm  invalid @%d\n",__LINE__);
        return 0;
    }

   //create handle struct  
    hdl = Fwl_Malloc(sizeof(T_MRecordrCtrl));
    if (AK_NULL == hdl)
    {
        Fwl_Print(C3, M_MENCODE, "Malloc Err @%d\n",__LINE__);
        return 0;
    }
    memset(hdl, 0 , sizeof(T_MRecordrCtrl));

    hdl->runStatus   = EVT_MREC_NULL;


    hdl->EncodeHdl = MEnc_Open(pAudioParm);
    if (0 == hdl->EncodeHdl)
    {
        Fwl_Print(C3, M_MENCODE, "media create recoredr Err @%d\n",__LINE__);
        MRec_Close((T_HANDLE)hdl, AK_FALSE);
        hdl = AK_NULL;
        return 0;
    }
    
    lRet = MRec_TaskCreate(hdl, AK_FALSE);

    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_MENCODE, "Task Create Err @%d\n",__LINE__);
        MRec_Close((T_HANDLE)hdl, AK_FALSE);
        hdl = AK_NULL;
        return 0;
    }
    hdl->runStatus   = EVT_MREC_NULL;
    
    return (T_HANDLE)hdl;
}



/*
 * @brief   start video record  porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in/out] srcCtlHdl: zoom handle
 * @param[in] pRecCtlParam: record control param	
 * @param[in] pAudioParm: audio encode param	
 * @param[in] pRecVideoParam: video encode param	
 * @return	resulst REC_ERROR_OK--success, else fail
 */
T_REC_ERROR_STATUS MRec_Start(T_HANDLE handle,T_HANDLE srcCtlHdl, T_REC_CTRL_INIT_PARAM   *pRecCtlParam, 
    T_REC_AUDIO_INIT_PARAM  *pRecAudioParam,T_REC_VIDEO_INIT_PARAM  *pRecVideoParam)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;
    T_REC_ERROR_STATUS  ret = REC_ERROR_NULL;
    T_S32      lRet  = AK_EFAILED;
    T_FRM_INFO    srcInfo = {0};
    
    AK_ASSERT_PTR(pCtrl, "MRec_Start: param err", REC_ERROR_ENCODE_EXP);
    AK_ASSERT_PTR(pRecCtlParam,  "MRec_Start: parameter error", REC_ERROR_ENCODE_EXP);
    AK_ASSERT_PTR(pRecVideoParam,  "MRec_Start: parameter error", REC_ERROR_ENCODE_EXP);

    if (0 == srcCtlHdl)
    {
        Fwl_Print(C3, M_MENCODE, "Param Err ,zoom hdl invalid @%d\n",__LINE__);
        return REC_ERROR_ENCODE_EXP;
    }
    srcInfo.type   = FORMAT_YUV420;
    srcInfo.width  = pRecVideoParam->videoWidth;
    srcInfo.height = pRecVideoParam->videoHeight;
    srcInfo.size   = srcInfo.height*srcInfo.width*3/2;

    // below param for camera windows
    srcInfo.rect.left   = 0;
    srcInfo.rect.top    = 0;
    srcInfo.rect.width  = srcInfo.width;
    srcInfo.rect.height = srcInfo.height;

    lRet = VideoZoom_OpenEncSrc(srcCtlHdl, &srcInfo, pRecCtlParam->isEnableFocus);
    if (AK_IS_FAILURE(lRet))
    {
         Fwl_Print(C3, M_MENCODE, "Create Enc Strm Err @%d\n",__LINE__);
         return REC_ERROR_ENCODE_EXP;
    }

    pCtrl->SrcCtrl  = srcCtlHdl;

    if(0 == pCtrl->EncodeHdl)
    {
        return REC_ERROR_ENCODE_EXP;
    }
    
    ret = MEnc_Start(pCtrl->EncodeHdl,pRecCtlParam, pRecAudioParam, pRecVideoParam);
    if (REC_ERROR_OK == ret)
    {
        pCtrl->NoMovingTimeLimt   = pRecCtlParam->detectNoMovingTimeMs;

        //add stamptime
        VideoZoom_SetEncPreProc(pCtrl->SrcCtrl, MRec_ZoomPreProc, (T_HANDLE)pCtrl);
        VideoZoom_SetEncPostProc(pCtrl->SrcCtrl, MRec_ZoomPostProc, (T_HANDLE)pCtrl);
        
        VideoZoom_EnableEncSrc(pCtrl->SrcCtrl, AK_TRUE);
        VideoZoom_DetectEnable(pCtrl->SrcCtrl, (T_BOOL)(pCtrl->NoMovingTimeLimt > 0));
        MRec_CtrlEvtSet(pCtrl,EVT_MREC_RUN);
        ISubThread_Resume(pCtrl->trd);
        pCtrl->NoMovingTimsMs  = get_tick_count();
    }
    else
    {
        MEnc_Close(pCtrl->EncodeHdl, AK_FALSE);
        pCtrl->EncodeHdl = 0;
    }
    
    return ret;

}

/*
 * @brief   stop video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSave:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Stop(T_HANDLE handle, T_BOOL isSave)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;
    
    AK_ASSERT_PTR(pCtrl, "MRec_Stop: param err", AK_EFAILED);

    MRec_CtrlEvtSet(pCtrl,EVT_MREC_SUSPEND);
    if (0 != pCtrl->EncodeHdl)
    {
         MEnc_Suspend(pCtrl->EncodeHdl,isSave);
    }
    
    VideoZoom_CloseEncSrc(pCtrl->SrcCtrl);
    
    return AK_SUCCESS;
}



/*
 * @brief   pause video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Pause(T_HANDLE handle)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;
    
    AK_ASSERT_PTR(pCtrl, "MRec_Pause: param err", AK_EFAILED);

    Fwl_Print(C3, M_MENCODE, "MRec::Pause begin.\n");
    MREC_MUTEX_LOCK(pCtrl->mutex);

    VideoZoom_EnableEncSrc(pCtrl->SrcCtrl, AK_FALSE);
    MRec_CtrlEvtUpdate(pCtrl,EVT_MREC_SUSPEND);

    MREC_MUTEX_UNLOCK(pCtrl->mutex);
    Fwl_Print(C3, M_MENCODE, "MRec::Pause Ok.\n");

    return AK_SUCCESS;
}

/*
 * @brief   get recorder information by ctrltype
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] ctlType: ctrl type
 * @param[out] arg:information buffer
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 MRec_Ioctl(T_HANDLE handle, T_eMREC_IOCTL ctlType, T_VOID *arg)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;
    T_BOOL ret = AK_FALSE;
    
    AK_ASSERT_PTR(pCtrl, "MRec_Ioctl: param err", AK_SUCCESS);

    if (0 != pCtrl->EncodeHdl)
    {
        MREC_MUTEX_LOCK(pCtrl->mutex);
        ret = MEnc_Ioctl(pCtrl->EncodeHdl, ctlType, arg);
        MREC_MUTEX_UNLOCK(pCtrl->mutex);
    }

    if (ret)
    {
        return AK_SUCCESS;
    }
    else
    {
        return AK_EFAILED;
    }
}

/*
 * @brief   restart video record porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: video recorder handle
 * @param[in] isSavePre:is save record file	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_REC_ERROR_STATUS MRec_Restart(T_HANDLE handle, T_BOOL isSavePre)
{
    T_MRecordrCtrl *pCtrl = (T_MRecordrCtrl *)handle;
    T_REC_ERROR_STATUS ret = REC_ERROR_NULL;
    
    AK_ASSERT_PTR(pCtrl, "MRec_Restart: param err", AK_FALSE);

    Fwl_Print(C3, M_MENCODE, "MRec::Restart begin.\n");
    MREC_MUTEX_LOCK(pCtrl->mutex);

    MRec_CtrlEvtUpdate(pCtrl,EVT_MREC_SUSPEND);
    VideoZoom_EnableEncSrc(pCtrl->SrcCtrl, AK_FALSE);

    if (0 != pCtrl->EncodeHdl)
    {
        ret = MEnc_ReStart(pCtrl->EncodeHdl, isSavePre);
    }
    
    if (REC_ERROR_OK == ret)
    {
        Fwl_Print(C3, M_MENCODE, "MRec::Restart Ok.\n");
        VideoZoom_EnableEncSrc(pCtrl->SrcCtrl, AK_TRUE);
        MRec_CtrlEvtUpdate(pCtrl,EVT_MREC_RUN);
    }
    
    else
    {
        Fwl_Print(C3, M_MENCODE, "MRec::Restart failed.\n");
    }

    MREC_MUTEX_UNLOCK(pCtrl->mutex);
    
    return ret;
}


/*
 * @brief   process frame whitch get from zoom stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in/out] hdl: recorder handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 MRec_FrameProc(T_FRM_DATA *pFrame,    T_MRecordrCtrl *hdl)
{
    if (AK_NULL == pFrame)
    {
        return AK_SUCCESS;
    }


    if (AK_NULL == pFrame->pBuffer)
    {
        return AK_SUCCESS;
    }

    if (EVT_MREC_RUN != hdl->runStatus)
    {
        Fwl_Print(C3, M_MENCODE, "<%s:Pause-->\n",hdl->name);
        return AK_SUCCESS;
    }
    
    if (0 != hdl->EncodeHdl)
    {
#ifdef DEBUG_MREC_TRACERUN
        Fwl_Print(C3, M_MENCODE, "<%s:begin>:_%dx%d--\n", 
            hdl->name,pFrame->info.width,pFrame->info.height);
#endif
		
#ifndef DEBUG_MREC_DRYRUN 
		/*每帧编码前，音频编码标记置FALSE，
		若视频的delay回调里调用了音频编码，则标记会被置TRUE
		*/
		g_bAudioEncoded = AK_FALSE;	
        MEnc_VideoFrameEncode(hdl->EncodeHdl, pFrame);
		if (!g_bAudioEncoded)
		{
			//若未在视频编码的delay回调里做音频编码，则在此处补做
        	MEnc_AudioPcmEncode(hdl->EncodeHdl);
		}

#endif
#ifdef DEBUG_MREC_TRACERUN
        Fwl_Print(C3, M_MENCODE, "\n<Enc:end>+++++++++++++++\n");
#endif
    }
    else
    {
        Fwl_Print(C3, M_MENCODE, "\n<Enc:Error:not init>\n");
    }
    return AK_SUCCESS;
}



/*#if 0
        T_U32 tmp_event;
        if (!MREC_MUTEX_TRY_LOCK(hdl->mutex))
        {
#ifdef DEBUG_MREC_TRACERUN
            Fwl_Print(C3, M_MENCODE, "Wait_0x%x_status=%d\n",hdl->SrcCtrl,hdl->runStatus);
#endif
            continue;
        }
        //status = AK_Retrieve_Events(hdl->cmdEvtGroup, EVT_ENC_ALL_EVENT, AK_OR_CONSUME, &tmp_event, AK_NO_SUSPEND);
        //status = AK_Retrieve_Events(hdl->cmdEvtGroup, EVT_ENC_ALL_EVENT, AK_OR_CONSUME, &tmp_event, AK_SUSPEND);
        if (AK_SUCCESS == status)
        {
            MRec_TaskEventHandle(hdl,tmp_event);
        }
        else
        {
            Fwl_Print(C3, M_MENCODE, "$");
        }
#endif
 */

/*
 * @brief   video recorder process for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: recorder handle
 */
static T_VOID  MRec_TaskHandle(T_MRecordrCtrl * hdl)
{
    T_S32 lRet = AK_EFAILED;
    T_REC_ERROR_STATUS  encStatus = 0;
    T_REC_ERROR_STATUS  ret = 0;
    T_BOOL isRestart    = AK_FALSE;
    T_BOOL isExecption  = AK_FALSE;
    T_BOOL isSavePre    = AK_FALSE;
    
    if (AK_NULL != hdl)
    {
        Fwl_Print(C3, M_MENCODE, "%s task entry\n", (char*)hdl->name);
    }
    

    while(hdl)
    {
#ifdef MREC_TASK_WATCH_DOG_SEC    
        AK_Feed_Watchdog(MREC_TASK_WATCH_DOG_SEC);
#endif
        //Fwl_Print(C3, M_MENCODE, "<E");
        MREC_MUTEX_LOCK(hdl->mutex);
        
        //Fwl_Print(C3, M_MENCODE, ">");
#ifndef DEBUG_MREC_DRYRUN        
        if ((EVT_MREC_RUN == hdl->runStatus) && (0 != hdl->EncodeHdl))
        {
            if (VideoZoom_IsEnableEnc(hdl->SrcCtrl))
            {
                lRet = VideoZoom_GetEncFrameByProc(hdl->SrcCtrl,MRec_FrameProc, (T_pVOID)hdl);
            }
            else
            {
                //Fwl_Print(C3, M_MENCODE, "encode wait...\n");
                lRet = AK_EFAILED;
            }

             // if excption is occus, 
            if (AK_IS_SUCCESS(lRet))
            {
                isExecption = AK_FALSE;
                isSavePre   = AK_TRUE;

                if (AK_IS_FAILURE(MEnc_ExceptionStatusCheck(hdl->EncodeHdl, &encStatus)))
                {
                    hdl->runStatus = EVT_MREC_SUSPEND;
                    isExecption    = AK_TRUE;
                }    
                else 
                {   // get video current motion status
                    lRet = MRec_MotionDetectCheck(hdl, &isRestart);

                    // if  video is moving, disable drop frame
                    if (isRestart)
                    {
                        MEnc_EnableVideoFrameDrop(hdl->EncodeHdl, AK_FALSE);
                    }
                    //if video is no moving , enable drop frame automatic
                    else if (AK_IS_FAILURE(lRet))
                    {
                        //hdl->runStatus = EVT_MREC_SUSPEND;
                        //isExecption    = AK_TRUE;
                        //encStatus      = REC_ERROR_DETECT_LIMIT;
                        MEnc_EnableVideoFrameDrop(hdl->EncodeHdl, AK_TRUE);
                    }
                }


                if (isExecption)
                {
                    VideoZoom_EnableEncSrc(hdl->SrcCtrl, AK_FALSE);
                    isRestart  = MEnc_ExceptionIsNeedRestart(hdl->EncodeHdl,encStatus, &isSavePre);

                    if (isRestart)
                    {
                        // save file and restart next
                        ret = MEnc_ReStart(hdl->EncodeHdl, isSavePre);
                        if (REC_ERROR_OK == ret)
                        {
                            VideoZoom_EnableEncSrc(hdl->SrcCtrl, AK_TRUE);
                            hdl->runStatus = EVT_MREC_RUN;
                            Fwl_Print(C3, M_MENCODE, "Recorder Next.\n");
                        }
                        else
                        {
                           // restart err, send msg to app, file has save
                            Fwl_Print(C3, M_MENCODE, "Recorder ReStart err(status %d)!\n", ret);
                            VIDEO_MREC_STOP(ret);// send restart err msg to app
                        }
                    }
                    else
                    {
                        Fwl_Print(C3, M_MENCODE, "Recorder Stop(status %d).\n", encStatus);
                        MEnc_Suspend(hdl->EncodeHdl, isSavePre);
                        VIDEO_MREC_STOP(encStatus);// send recording err msg to app
                    }
                }
            }
        }
        else if (EVT_MREC_CLOSE == hdl->runStatus)
        {
            MREC_MUTEX_UNLOCK(hdl->mutex);
            break;
        }
        else
        {
           AK_Sleep(1);
        }
#endif
        MREC_MUTEX_UNLOCK(hdl->mutex);
    }

    Fwl_Print(C3, M_MENCODE, "\n+++++Meida Rec task exit++++++++\n");
}

#endif



