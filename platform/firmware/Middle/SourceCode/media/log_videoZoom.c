/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_videoZoom.c
 * Function:camera output asyn zoom logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/
#include "Drv_api.h"
#include "AKSubThread.h"
#include "AKAppMgr.h"
#include "log_videoZoom.h"
#include "Log_MotionDetec.h"
#include "Eng_Time.h"
#include "fwl_font.h"
#include "Fwl_gui.h"
#include "Ctl_VideoRecord.h"

#ifdef CAMERA_SUPPORT

//=================  Macro  Define (Debug)===========================

//#define DEBUG_VIDEOZOOM_DRYRUN  
//#define DEBUG_VIDEOZOOM_TRACERUN

extern T_VIDEO_RECORD *pVideoRec;

#ifdef PLATFORM_DEBUG_VER
#define VIDEOZOOM_DEBUG_VER
#endif

/**
 * watchdog time (second) if zero, wil close watch dog 
 */
#ifdef VIDEOZOOM_DEBUG_VER
#define ZOOM_TASK_WATCH_DOG_SEC   (4)
#endif

/**
 *enable debug zoom stream opt
 */
#define VIDEOZOOM_STRM_DEBUG_ENABLE   (0)

#define ZOOM_LOG_DEBUG             //AkDebugOutput

//====================== Macro  Define ===========================
//-------------- Task Runtime Parameter Define--- 
/**
 * video zoom task priority
 */
#define ZOOM_TASK_PRIO            (95)

/**
 * video zoom task time slice
 */
#define ZOOM_TASK_SLICE           (2)

/**
 * video zoom task run stack size(byte)
 */
#define ZOOM_TASK_STACK           (4*1024)

//-------------- Zoom Task Configure Define--- 
/**
 * video zoom task name length
 */
#define ZOOM_TASK_NAME_LEN        (12)

/**
 * video zoom  for encode stream length
 */
#define VIDEOZOOM_ENCODE_STRM_LEN    (3)


/**
 * video zoom  for display stream length
 */
#define VIDEOZOOM_DISP_STRM_LEN     (2)

/**
 *enable camera interupt  send msg to trigger  zoomtask
 */
#define VIDEOZOOM_CAM_INT_MSG_LEN   (12)

/**
 * init 2d hardware in  this module, (lcd version has inited at outside, so if use lcd ver, need not open)
 */
//#define VIDEOZOOM_2D_INIT_INNER

/**
 * video zoom  focus video src max level 
 */
#define ZOOM_FOCUSE_LVL_MAX        (10)

/**
 * video zoom  for motion detect min interval
 */
#define ZOOM_MDETECT_INTERVAL_MIN  (20)

/**
 * video zoom  for motion detect  ignore pre frames
 */
#define ZOOM_MDETECT_IGNORE_FRAMES  (10)

//==================== Macro  Define (Ctrl)==========================
//-------------- Zoom Task Status Define--- 
/**
 *task status: init status, meaning task is no ready
 */
#define EVT_ZOOM_NULL         (0)

/**
 *task status: suspend, meaning task is do nothing
 */
#define EVT_ZOOM_SUSPEND      (0x01<<1)

/**
 * task status: run, meaning task is do someting
 */
#define EVT_ZOOM_RUN          (0x01<<2)

/**
 * task status: close, meaning task is cosed
 */
#define EVT_ZOOM_CLOSE        (0x01<<3)

//-------------- Zoom Function Selection Configure--- 
/**
 *zoom status: enable display zoom
 */
#define ZOOM_MODE_DISP    (0x1<<1)

/**
 * zoom status: enable encode zoom
 */
#define ZOOM_MODE_ENCODE  (0x1<<2)

/**
 * zoom status: disable all zoom
 */
#define ZOOM_MODE_NONE    (0)

//===================== Type Define =============================
/**
 * zoom msg que type define
 */
typedef struct
{
	T_pVOID    m_pQueueAddr;// msg que address
	T_hQueue   m_hQueue;// msg que handle
    T_U32      m_queNum;// msg que len
    T_U32      m_msgSize;// msg que atomic size
} T_ZOOM_MSG_QUE, *T_PZOOM_MSG_QUE;

/**
 * zoom msg type define
 */
typedef struct
{
    T_U32 m_MsgVal;// msg content
} T_ZOOM_MSG;

/**
 * video src  define
 */
typedef struct tagVideoSource
{
    T_CAMERA_BUFFER camBuffer1;// camera init buffer 1
    T_CAMERA_BUFFER camBuffer2;//camera init buffer 2
    T_CAMERA_BUFFER camBuffer3;//camera init buffer 3
} T_ZOOM_CAM_SRC;

/**
 * zoom strm postproc handle
 */
typedef struct tagZoomStrmExternaltProc
{
    T_ZOOM_FRM_PROC proc;
    T_HANDLE        hdl;
} T_ZOOM_EXT_PROC;

/**
 * zoom strm proc order
 */
typedef struct tagZoomStrmGetOrdeProc
{
    T_ZOOM_FRM_GET_ORDER proc;
    T_HANDLE             hdl;
} T_ZOOM_GET_ORDER;




/**
 * video zoom control  define
 */
typedef struct {
    ISubThread         *trd;// video zoom thread handle
    T_hSemaphore       mutex;// mutex
    T_S8               name[ZOOM_TASK_NAME_LEN];// thread name
    _VOLATILE T_U32    runStatus;// thread status
    //-----trans address protect -----
    T_U16              transSemInit;// stream trans add num
    T_hSemaphore       transSem;   //  address trans protect
    //---------------------------
    _VOLATILE IFrmStrm *EncStrm;  // encode stream  source
    T_ZOOM_EXT_PROC    EncStrmPre; // encode stream post process handle
    T_ZOOM_EXT_PROC    EncStrmPost; // encode stream post process handle
    //---------------------------
    _VOLATILE IFrmStrm *DispStrm; // display stream source
    T_ZOOM_EXT_PROC    DispStrmPre; // encode stream post process handle
    T_ZOOM_EXT_PROC    DispStrmPost;// display stream post process handle
    T_ZOOM_GET_ORDER   DispStrmOrder;
    //-----Video Source Frame Info----
    T_FRM_DATA         srcFrmData; // current video source data info
    T_BOOL             isSrcInit;  // video source device init flag
    T_ZOOM_CAM_SRC     srcCambuffer;// video source init data
    T_U32              refCnt;// video src update count
    T_HANDLE           motionDetectorHdl; //motion detect handle
    T_U32              motionDetectTimeMs;// motion detect  time(ms)
    T_U32              motionDetectInterval;// detect two frame  interval time (ms)
    T_BOOL             motionDetectEnable;// is need Detect two frame
    T_U32              motionDetectCnt;// motion detect  times
    _VOLATILE T_BOOL   motionDetectStatus;// motion detect switch 
    //---------------------------
    _VOLATILE T_U32    zoomMode;// zoom switch  for encode and display
    //-------Video Frame Focus------
    T_U8               focusLvl;// zoom for foucs video src
    T_BOOL             isEnableFocusWin;//is need open video focus function
    //----------------------------
    //利用Camera特性，直接从VGA缩放到QVGA，不再2D
    T_BOOL             Is02_8M_QVGA;//is 02 chip 8M QVGA recorder
} T_VideoZoomCtrl;




//====================== Macro  Define(Function)=====================

#define ZOOM_MUTEX_INIT(mutex)       ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define ZOOM_MUTEX_IS_ERR(mutex)     (((mutex) <= 0) && ((mutex) > -100))
#define ZOOM_MUTEX_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define ZOOM_MUTEX_TRY_LOCK(mutex)    (0 < (AK_Try_Obtain_Semaphore((mutex), AK_SUSPEND)))
#define ZOOM_MUTEX_UNLOCK(mutex)      AK_Release_Semaphore((mutex))
#define ZOOM_MUTEX_DEINIT(mutex)      {\
    if (!(ZOOM_MUTEX_IS_ERR(mutex)))\
    {\
        AK_Delete_Semaphore(mutex);\
        (mutex) = 0;\
    }\
}

#define ZOOM_SEM_INIT(sem,cnt)       ((sem) = AK_Create_Semaphore((cnt), AK_PRIORITY))
#define ZOOM_SEM_IS_ERR(sem)         (((sem) <= 0) && ((sem) > -100))
#define ZOOM_SEM_VAL(sem)            (AK_Get_SemVal(sem))
#define ZOOM_SEM_WAIT(sem)            AK_Obtain_Semaphore((sem), AK_SUSPEND)
#define ZOOM_SEM_TRY_WAIT(sem)        (0 < (AK_Try_Obtain_Semaphore((sem), AK_SUSPEND)))
#define ZOOM_SEM_POST(sem)            AK_Release_Semaphore((sem))
#define ZOOM_SEM_DEINIT(sem)      {\
    if (!(ZOOM_SEM_IS_ERR(sem)))\
    {\
        AK_Delete_Semaphore(sem);\
        (sem) = 0;\
    }\
}

#define   ZOOM_IS_ENABLE_DISP(pCtrl)  (((pCtrl)->zoomMode & ZOOM_MODE_DISP) && (AK_NULL != (pCtrl)->DispStrm))
#define   ZOOM_IS_ENABLE_ENC(pCtrl)   (((pCtrl)->zoomMode & ZOOM_MODE_ENCODE) && (AK_NULL != (pCtrl)->EncStrm))

//========================= Global Variable  Define ===================
#ifdef VIDEOZOOM_DEBUG_VER
static T_U32       gZoomCamPreTime = 0,gZoomFps = 0;
#endif

//=======================Local Function  Declare=====================
#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
T_PZOOM_MSG_QUE    gZoomCamMsgQue  = AK_NULL;// camera stream ready msgque
/*
 * @brief   create message queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in] queNum: msgQue length
 * @param[in] msgSize:message size	
 * @return	 --0 failed, else msgQue handle
 */
static T_PZOOM_MSG_QUE  Zoom_MsgQueCreate(T_U32 queNum, T_U32 msgSize);

/*
 * @brief   destory message queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 */
static T_VOID Zoom_MsgQueDestory(T_PZOOM_MSG_QUE pRecMsgQue);

/*
 * @brief   receive message from queen head
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 * @param[in] pMsg: msg
 * @return	resulst T_BOOL--success, else fail
 */
static T_BOOL Zoom_MsgRec(T_PZOOM_MSG_QUE pRecMsgQue,T_pVOID pMsg);

/*
 * @brief   send message to queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 * @param[in] pMsg: msg
 * @param[in] isHead:is send to queen head
 * @return	resulst T_BOOL--success, else fail
 */
static T_BOOL Zoom_MsgSend(T_PZOOM_MSG_QUE pRecMsgQue,T_pVOID pMsg,T_BOOL isHead);
#endif
/*
 * @brief   set video zoom  task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]status:status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_CtrlEvtSet(T_VideoZoomCtrl * pCtrl, T_U32 status);

/*
 * @brief   frame scale
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pSrcFrame: source frame
 * @param[in/out] pDestFrame: destination frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_FrameScale(T_FRM_DATA *pSrcFrame,    T_FRM_DATA *pDestFrame);

/*
 * @brief   update source video stream from camera
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_UpdateVideoSrcStrm(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   put source frame to display stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pDestFrame: destination frame
 * @param[in/out] pSrcFrame: source frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_PutToDispProc(T_FRM_DATA *pDestFrame, T_FRM_DATA *pSrcFrame,T_VideoZoomCtrl *hdl);

/*
 * @brief   put source frame to encode stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pDestFrame: destination frame
 * @param[in/out] pSrcFrame: source frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_PutToEncodeProc(T_FRM_DATA *pDestFrame, T_FRM_DATA *pSrcFrame,T_VideoZoomCtrl *hdl);

/*
 * @brief   create a stream by init parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] ppi: stream handle
 * @param[in]len:stream length(the fream queen node count)
 * @param[in]isDebugMode:stream debug function enable
 * @param[in]frameInfo:stream info for all node
 * @param[in]transMode:stream transfer mode
 * @param[in]getMode:stream get process mode
 * @param[in]getOldEnable:stream get old node enable(when there is no new node for read)
 * @param[in]putMode:stream  put process mode
 * @param[in]putOldEnable:stream get old node enable(when there is no new node for write)
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_StreamInit(IFrmStrm **ppi,T_U32 len, T_BOOL isDebugMode,
                                    T_FRM_INFO *frameInfo, T_STRM_TYPE transMode, 
                                    T_STRM_BLK_TYPE getMode, T_BOOL getOldEnable,
                                    T_STRM_BLK_TYPE putMode, T_BOOL putOldEnable);

/*
 * @brief   create  task for process zoom frame
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]isStart:is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_TaskCreate(T_VideoZoomCtrl * pCtrl, T_BOOL isStart);

/*
 * @brief   destory  zoom task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_TaskDestory(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   get address transfer stream count
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	 the count of transfering address stream
 */
static T_U16 Zoom_GetTransAddrCnt(T_VideoZoomCtrl *pCtrl);

/*
 * @brief   update address transfer stream count and status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmUpdate(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   wait address transfer stream used finished
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmWait(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   cancle address stream transfering 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmPost(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   reset address stream transfering 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmReset(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   set  zoom function selection flag
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]flag:function selection flag
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_SetSrcModeFlag(T_VideoZoomCtrl * pCtrl, T_U32 flag);

/*
 * @brief     clear  zoom function selection flag
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]flag:function selection flag
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_ClearSrcModeFlag(T_VideoZoomCtrl * pCtrl, T_U32 flag);

/*
 * @brief      zoom process  function for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 */
static T_VOID VideoZoom_TaskHandle(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   open camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcOpen(T_VideoZoomCtrl * pCtrl);

/*
 * @brief   close camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcClose(T_VideoZoomCtrl * hdl);

/*
 * @brief   start camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcStart(T_VideoZoomCtrl * hdl, T_U32 width, T_U32 height);

/*
 * @brief   restart camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcRestart(T_VideoZoomCtrl * pCtrl, T_U32 width, T_U32 height);

/*
 * @brief   stop camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcStop(T_VideoZoomCtrl * hdl);

/*
 * @brief   update camera stream source information
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoFocusWinUpdate(T_VideoZoomCtrl * pCtrl, T_BOOL isOnVideo);

/*
 * @brief   update camera stream source motion detect information
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]pData:the current detecte data for compare
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_DetectStatusUpdate(T_VideoZoomCtrl * pCtrl, T_U8 *pData);

/*
 * @brief   DeInit camera stream source 
 * @author WangXi
 * @date	2011-10-25
 */
static T_BOOL Zoom_CameraSrcDeInit(T_VOID);

/*
 * @brief   Init camera stream source 
 * @author WangXi
 * @date	2011-10-25
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @param[in]YUV1~YUV3:video source buffer
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_BOOL Zoom_CameraSrcInit(T_U32 width, T_U32 height,T_CAMERA_BUFFER *YUV1,T_CAMERA_BUFFER *YUV2,T_CAMERA_BUFFER *YUV3);

/*
 * @brief   callback function for Init camera stream source 
 * @author WangXi
 * @date	2011-10-25
 */
static T_VOID Zoom_CameraSrcReady_Cbf(T_VOID);

/*
 * @brief  check  video display  info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in/out] FrameInfo: frame information
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_CheckDispInfo(T_VideoZoomCtrl * pCtrl, T_FRM_INFO *FrameInfo);

//==================== Function  Define ===================

#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
/*
 * @brief   destory message queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 */
static T_VOID Zoom_MsgQueDestory(T_PZOOM_MSG_QUE pRecMsgQue)
{
	if (AK_NULL != pRecMsgQue)
	{
		if (AK_IS_VALIDHANDLE(pRecMsgQue->m_hQueue))
		{
			AK_Delete_Queue(pRecMsgQue->m_hQueue);
			pRecMsgQue->m_hQueue = AK_INVALID_QUEUE;
		}

		if (AK_NULL != pRecMsgQue->m_pQueueAddr)
		{
			Fwl_Free(pRecMsgQue->m_pQueueAddr);
			pRecMsgQue->m_pQueueAddr = AK_NULL;
		}

        Fwl_Free(pRecMsgQue);
	}
}

/*
 * @brief   create message queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in] queNum: msgQue length
 * @param[in] msgSize:message size	
 * @return	 --0 failed, else msgQue handle
 */
static T_PZOOM_MSG_QUE  Zoom_MsgQueCreate(T_U32 queNum, T_U32 msgSize)
{
    T_PZOOM_MSG_QUE pRecMsgQue = AK_NULL;
    T_U32 queSize = 0;

	pRecMsgQue = Fwl_Malloc(sizeof(T_ZOOM_MSG_QUE));
    if (AK_NULL == pRecMsgQue)
	{
		Fwl_Print(C1, M_VZOOM, "MsgQue Is NULL\n");
		return AK_NULL;
	}
	
    // get message queen size(byte)
    queSize = queNum * msgSize;
	pRecMsgQue->m_pQueueAddr = Fwl_Malloc(queSize);
	if (AK_NULL == pRecMsgQue->m_pQueueAddr)
	{
		Fwl_Print(C1, M_VZOOM, "m_pQueueAddr Malloc Error\n");
		goto ErrorQuit;
	}
	
	pRecMsgQue->m_queNum = queNum;
	pRecMsgQue->m_msgSize = msgSize;

	pRecMsgQue->m_hQueue = AK_Create_Queue(pRecMsgQue->m_pQueueAddr,queSize, AK_FIXED_SIZE,
                                           pRecMsgQue->m_msgSize, AK_FIFO);

	if (AK_IS_INVALIDHANDLE(pRecMsgQue->m_hQueue))
	{
		goto ErrorQuit;
	}
	return pRecMsgQue;
ErrorQuit:
	Zoom_MsgQueDestory(pRecMsgQue);
    return AK_FALSE;
}

/*
 * @brief   send message to queen
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 * @param[in] pMsg: msg
 * @param[in] isHead:is send to queen head
 * @return	resulst T_BOOL--success, else fail
 */
static T_BOOL Zoom_MsgSend(T_PZOOM_MSG_QUE pRecMsgQue,T_pVOID pMsg,T_BOOL isHead)
{
	T_S32 lRet = -1;

	if ((AK_NULL != pRecMsgQue) && (AK_IS_VALIDHANDLE(pRecMsgQue->m_hQueue)))
	{
		if (isHead)
		{
			lRet = AK_Send_To_Front_of_Queue(pRecMsgQue->m_hQueue, 
									  pMsg, pRecMsgQue->m_msgSize, 
									  AK_NO_SUSPEND);
		}
		else
		{
			lRet = AK_Send_To_Queue(pRecMsgQue->m_hQueue, 
									pMsg, pRecMsgQue->m_msgSize, 
									AK_NO_SUSPEND);
		}
		
		if (AK_SUCCESS != lRet)
		{
			//Fwl_Print(C3, M_VZOOM, "SendMsg ret = %d\n",lRet);//AK_Send_To_Queue
		}
	}

	return (AK_SUCCESS == lRet);
}

/*
 * @brief   receive message from queen head
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pRecMsgQue: msgQue handle
 * @param[in] pMsg: msg
 * @return	resulst T_BOOL--success, else fail
 */
static T_BOOL Zoom_MsgRec(T_PZOOM_MSG_QUE pRecMsgQue,T_pVOID pMsg)
{
	T_S32 lRet = -1;
	T_U32 actual_size = 0;
	
	if ((AK_NULL != pRecMsgQue) && (AK_IS_VALIDHANDLE(pRecMsgQue->m_hQueue)))
	{
		lRet = AK_Receive_From_Queue(pRecMsgQue->m_hQueue, 
								     pMsg, pRecMsgQue->m_msgSize, 
								     &actual_size,AK_SUSPEND);
		
		if (AK_SUCCESS != lRet)
		{
			//Fwl_Print(C3, M_VZOOM, "RecMsg ret = %d\n",lRet);
		}
	}

	return (AK_SUCCESS == lRet);
}
#endif

/*
 * @brief   set video zoom  task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in/out]
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_CtrlEvtSet(T_VideoZoomCtrl * pCtrl, T_U32 status)
{
    AK_ASSERT_PTR(pCtrl, "Param err", AK_EBADPARAM);


    if (pCtrl->runStatus == status)
    {
        Fwl_Print(C3, M_VZOOM, "Zoom status has seted\n");
        return AK_SUCCESS;
    }

    // directly set, need not wait
    if (EVT_ZOOM_RUN != pCtrl->runStatus)
    {
       pCtrl->runStatus = status;
       Fwl_Print(C3, M_VZOOM, "Zoom@@set event Ok\n");
       return AK_SUCCESS;
    }
    else
    {
        Fwl_Print(C3, M_VZOOM, "Zoom::set event 0x%x begin...\n", status);
        ZOOM_MUTEX_LOCK(pCtrl->mutex);
        pCtrl->runStatus = status;
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
        Fwl_Print(C3, M_VZOOM, "Zoom::set event Ok\n", status);
    }
    return AK_SUCCESS;
}


/*
 * @brief   Init camera stream source 
 * @author WangXi
 * @date	2011-10-25
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @param[in]YUV1~YUV3:video source buffer
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_BOOL Zoom_CameraSrcInit(T_U32 width, T_U32 height,T_CAMERA_BUFFER *YUV1,T_CAMERA_BUFFER *YUV2,T_CAMERA_BUFFER *YUV3)
{
    T_BOOL ret = AK_FALSE;

#ifdef OS_ANYKA
   	ret = camstream_init(width, height, YUV1, YUV2, YUV3);
    camstream_set_callback(Zoom_CameraSrcReady_Cbf);//camstream_set_callback
#endif

    return ret;
}



/*
 * @brief   callback function for Init camera stream source 
 * @author WangXi
 * @date	2011-10-25
 */
static T_VOID Zoom_CameraSrcReady_Cbf(T_VOID)
{
#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
    if (AK_NULL != gZoomCamMsgQue)
    {
        T_ZOOM_MSG  msg;
        Zoom_MsgSend(gZoomCamMsgQue, (T_pVOID)&msg, AK_FALSE);
    }
#endif
 
#ifdef VIDEOZOOM_DEBUG_VER
    //AkDebugOutput("tick=%d\n",get_tick_count());
    if ((get_tick_count()-gZoomCamPreTime) >= 1000)
    {
        Fwl_Print(C3, M_VZOOM, "=>%d\n",gZoomFps);
        gZoomCamPreTime = get_tick_count();
        gZoomFps = 0;
    }
    else
    {
        gZoomFps++;
    }
 #endif
}


/*
 * @brief   DeInit camera stream source 
 * @author WangXi
 * @date	2011-10-25
 */
static T_BOOL Zoom_CameraSrcDeInit(T_VOID)
{
#ifdef OS_ANYKA
    camstream_set_callback(AK_NULL);//camstream_set_callback
    camstream_stop();
#endif
    return AK_TRUE;
}

/*
 * @brief   put source frame to encode stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pDestFrame: destination frame
 * @param[in/out] pSrcFrame: source frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_PutToEncodeProc(T_FRM_DATA *pDestFrame, T_FRM_DATA *pSrcFrame,T_VideoZoomCtrl *pCtrl)
{
    if (AK_NULL == pDestFrame)
    {
        return AK_SUCCESS;
    }

    if (AK_NULL == pDestFrame->pBuffer)
    {
        return AK_SUCCESS;
    }

    if (AK_NULL != pCtrl->EncStrmPre.proc)
    {
        (*pCtrl->EncStrmPre.proc)(pSrcFrame, pCtrl->EncStrmPre.hdl);
    } 
    
    // must have dest store space and different scale
    if (eSTRM_TYPE_DATA == IFrmStrm_GetTransType(pCtrl->EncStrm))//eSTRM_TYPE_DISP don't join in encode
    {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "Put <DATA>  Enc begin..\n");
#endif
        //t = get_tick_count();
        Zoom_FrameScale(pSrcFrame,pDestFrame);
        //Fwl_Print(C3, M_VZOOM, "t0=%d\n",get_tick_count()-t);
    }
    else
    {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "Put <Addr>  Enc begin..\n");
#endif
    }
    
    //t = get_tick_count();

    if (AK_NULL != pCtrl->EncStrmPost.proc)
    {
        (*pCtrl->EncStrmPost.proc)(pDestFrame, pCtrl->EncStrmPost.hdl);
    }
    
    //Fwl_Print(C3, M_VZOOM, "t1=%d\n",get_tick_count()-t);
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
    Fwl_Print(C3, M_VZOOM, "Put Enc end..\n");
#endif
    return AK_SUCCESS;

}

/*
 * @brief   put source frame to display stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pDestFrame: destination frame
 * @param[in/out] pSrcFrame: source frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_PutToDispProc(T_FRM_DATA *pDestFrame, T_FRM_DATA *pSrcFrame, T_VideoZoomCtrl *pCtrl)
{
    //T_U32 t = 0;
    
    if (AK_NULL == pDestFrame)
    {
        return AK_SUCCESS;
    }

    if (AK_NULL == pDestFrame->pBuffer)
    {
        return AK_SUCCESS;
    }

    if (AK_NULL != pCtrl->DispStrmPre.proc)
    {
        (*pCtrl->DispStrmPre.proc)(pSrcFrame, pCtrl->DispStrmPre.hdl);
    } 
    
    if (eSTRM_TYPE_DATA == IFrmStrm_GetTransType(pCtrl->DispStrm)
		|| eSTRM_TYPE_DISP == IFrmStrm_GetTransType(pCtrl->DispStrm))
    {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "Put <DATA>  Disp begin..\n");
#endif
        //t = get_tick_count();
        memcpy(pDestFrame->pBuffer , pSrcFrame->pBuffer,  pSrcFrame->info.size);
     // Zoom_FrameScale(pSrcFrame, pDestFrame);
       // Fwl_Print(C3, M_VZOOM, "t2=%d\n",get_tick_count()-t);
    }
    else
    {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "Put <Addr>  Disp begin..\n");
#endif
    }

    if (AK_NULL != pCtrl->DispStrmPost.proc)
    {
        (*pCtrl->DispStrmPost.proc)(pDestFrame, pCtrl->DispStrmPost.hdl);
    }
    
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
    Fwl_Print(C3, M_VZOOM, "Put Disp end..\n");
#endif
    return AK_SUCCESS;
}

/*
 * @brief      zoom process  function for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 */
static T_VOID  VideoZoom_TaskHandle(T_VideoZoomCtrl * pCtrl)
{
    if (pCtrl)
    {
        Fwl_Print(C3, M_VZOOM, "%s task entry\n", (char*)pCtrl->name);
    }
    
    while(pCtrl)
    {
#if (ZOOM_TASK_WATCH_DOG_SEC > 0)    
        AK_Feed_Watchdog(ZOOM_TASK_WATCH_DOG_SEC);
#endif
        ZOOM_MUTEX_LOCK(pCtrl->mutex);
        if ((ZOOM_MODE_NONE == pCtrl->zoomMode) || (EVT_ZOOM_RUN != pCtrl->runStatus))
        {
            ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
            if (EVT_ZOOM_CLOSE == pCtrl->runStatus)
            {
               break;
            }
            else if (EVT_ZOOM_SUSPEND == pCtrl->runStatus)
            {
		        Fwl_Print(C3, M_VZOOM, "Zoom task is ready to suspend!\n");
			    ISubThread_Suspend(pCtrl->trd);
			    while(EVT_ZOOM_SUSPEND ==pCtrl->runStatus)
			    {
			    	AK_Sleep(1);
			    }
		        Fwl_Print(C3, M_VZOOM, "Zoom task have restored to run from suspend!\n");
			    continue;
            }
            else
            {
                AK_Sleep(1);
                continue;
            }
        }
        
        if (AK_IS_FAILURE(Zoom_UpdateVideoSrcStrm(pCtrl)))
        {
            ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
            //AK_Sleep(1);
            continue;
        }

        if (ZOOM_IS_ENABLE_ENC(pCtrl))
        {
             IFrmStrm_PutFrameByProc(pCtrl->EncStrm, Zoom_PutToEncodeProc,&(pCtrl->srcFrmData),(T_pVOID)pCtrl);
        }

        if (ZOOM_IS_ENABLE_DISP(pCtrl))
        {
            IFrmStrm_PutFrameByProc(pCtrl->DispStrm, Zoom_PutToDispProc, &(pCtrl->srcFrmData), (T_pVOID)pCtrl);
        }
       
        if (pCtrl->motionDetectEnable \
            && (0 != pCtrl->motionDetectorHdl))
        {
            Zoom_DetectStatusUpdate(pCtrl, pCtrl->srcFrmData.pBuffer);
        }
        
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }

    Fwl_Print(C3, M_VZOOM, "\n+++++Zoom task exit++++++++\n");
}


/*
 * @brief   get address transfer stream count
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	 the count of transfering address stream
 */
static T_U16 Zoom_GetTransAddrCnt(T_VideoZoomCtrl *pCtrl)
{
    T_U16 cnt = 0;
    
    if (AK_NULL != pCtrl)
    {
        if (ZOOM_IS_ENABLE_DISP(pCtrl)\
            && (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->DispStrm)))
        {
            cnt++;
        }
        
        if (ZOOM_IS_ENABLE_ENC(pCtrl)\
            && (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->EncStrm)))
        {
            cnt++;
        }
    }

    return cnt;
}

/*
 * @brief   update source video stream from camera
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_UpdateVideoSrcStrm(T_VideoZoomCtrl * pCtrl)
{
    T_CAMERA_BUFFER *yuv = AK_NULL;
    T_ZOOM_MSG  msg = {0};
    
    if (AK_NULL == pCtrl)
    {
        return AK_EBADPARAM;
    }
    
#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
    if (AK_NULL != gZoomCamMsgQue)
    {
        if (!Zoom_MsgRec(gZoomCamMsgQue, (T_pVOID)&msg))
        {
            return AK_EFAILED;
        }
    }
#endif

#ifdef OS_ANYKA
    if (!camstream_ready())
    {
        return AK_EFAILED;
    }    
#endif 

    Zoom_TransStrmWait(pCtrl);

#ifdef OS_ANYKA
    yuv =  camstream_get(); 
#endif

    if ((AK_NULL != yuv) && (AK_NULL != yuv->dY))
    {
        pCtrl->srcFrmData.pBuffer     =  yuv->dY;
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "\n\t[CAM_%d]:buf=0x%x(%dx%d)\n", 
            pCtrl->refCnt++,pCtrl->srcFrmData.pBuffer,pCtrl->srcFrmData.info.width,
        pCtrl->srcFrmData.info.height);
#endif
        return AK_SUCCESS;
    }
    else
    {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
        Fwl_Print(C3, M_VZOOM, "Get Next Src\n");
#endif
       //透传资源释放
       Zoom_TransStrmPost(pCtrl);
    }
    
    return AK_EFAILED;
}


/*
 * @brief   create a stream by init parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] ppi: stream handle
 * @param[in]len:stream length(the fream queen node count)
 * @param[in]isDebugMode:stream debug function enable
 * @param[in]frameInfo:stream info for all node
 * @param[in]transMode:stream transfer mode
 * @param[in]getMode:stream get process mode
 * @param[in]getOldEnable:stream get old node enable(when there is no new node for read)
 * @param[in]putMode:stream  put process mode
 * @param[in]putOldEnable:stream get old node enable(when there is no new node for write)
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_StreamInit(IFrmStrm **ppi,T_U32 len, T_BOOL isDebugMode,
                                    T_FRM_INFO *frameInfo, T_STRM_TYPE transMode, 
                                    T_STRM_BLK_TYPE getMode, T_BOOL getOldEnable,
                                    T_STRM_BLK_TYPE putMode, T_BOOL putOldEnable)
{
    T_STRM_INIT_PARAM InitParam  = {0};
    IFrmStrm         *pStrm      = AK_NULL;
    T_S32             lRet       = AK_EFAILED;

    if ((AK_NULL == ppi) || (AK_NULL == frameInfo))
    {
        Fwl_Print(C2, M_VZOOM, "Param Err @%d\n",__LINE__);
        return AK_EBADPARAM;
    }

    if (AK_NULL != (*ppi))
    {
        Fwl_Print(C2, M_VZOOM, "Param Err @%d\n",__LINE__);
        return AK_EBADPARAM;
    }
    
    memcpy(&InitParam.frameInfo, frameInfo, sizeof(T_FRM_INFO));
    InitParam.len               = len;
    InitParam.transMode         = transMode;
    InitParam.getProcMode       = getMode;
    InitParam.putProcMode       = putMode;
    InitParam.enableReadFromOld = getOldEnable;
    InitParam.enableWriteToOld  = putOldEnable;
    InitParam.enableDebugMode   = isDebugMode;
    //--create test-------------------------------------
    lRet = CFrmStrm_New(&pStrm, InitParam);

    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C1, M_VZOOM, "init error\n");
        return lRet;
    }
    
#ifdef VIDEOZOOM_DEBUG_VER
    IFrmStrm_Dump(pStrm);
#endif
    *ppi = pStrm;

    return AK_SUCCESS;
}


/*
 * @brief   update camera stream source motion detect information
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]pData:the current detecte data for compare
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_DetectStatusUpdate(T_VideoZoomCtrl * pCtrl, T_U8 *pData)
{
    T_U32 t = 0;
    
    AK_ASSERT_PTR(pCtrl, "Zoom_DetectStatusUpdate: param err", AK_EBADPARAM);


    if (pCtrl->motionDetectCnt < ZOOM_MDETECT_IGNORE_FRAMES)
    {
       pCtrl->motionDetectCnt++;
    }
    
    t = get_tick_count();
    if ((t - pCtrl->motionDetectTimeMs) >= pCtrl->motionDetectInterval)
    {
        if (pCtrl->motionDetectCnt < ZOOM_MDETECT_IGNORE_FRAMES)
        {
           pCtrl->motionDetectStatus = AK_FALSE;
        }
        else
        {
            pCtrl->motionDetectStatus = MDetect_IsMoving(pCtrl->motionDetectorHdl);
        }
        pCtrl->motionDetectTimeMs = t;
	    MDetect_UpdateData(pCtrl->motionDetectorHdl,pData);
    }

    return AK_SUCCESS;
}


/*
 * @brief   wait address transfer stream used finished
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmWait(T_VideoZoomCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Zoom_TaskCreate: param err", AK_EBADPARAM);

    if (pCtrl->transSemInit == 0)
    {
        return AK_SUCCESS;
    }

    if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
    {
        if (ZOOM_IS_ENABLE_ENC(pCtrl))
        {
            if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->EncStrm))
            {
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
                Fwl_Print(C3, M_VZOOM, "$0+");
#endif
                ZOOM_SEM_WAIT(pCtrl->transSem);
#ifdef DEBUG_VIDEOZOOM_TRACERUN    
                Fwl_Print(C3, M_VZOOM, "$0-");
#endif
            }
        }
        
        if (ZOOM_IS_ENABLE_DISP(pCtrl))
        {
            if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->DispStrm))
            {
                ZOOM_SEM_WAIT(pCtrl->transSem);
            }
        }
    }
    
    return AK_SUCCESS;
}



/*
 * @brief   cancle address stream transfering 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmPost(T_VideoZoomCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Zoom_TaskCreate: param err", AK_EBADPARAM);

    if (pCtrl->transSemInit == 0)
    {
        return AK_SUCCESS;
    }

    if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
    {
        if (ZOOM_IS_ENABLE_ENC(pCtrl))
        {
            if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->EncStrm))
            {
                if (ZOOM_SEM_VAL(pCtrl->transSem) < pCtrl->transSemInit)
                {
                    ZOOM_SEM_POST(pCtrl->transSem);
                }
            }
        }
        
        if (ZOOM_IS_ENABLE_DISP(pCtrl))
        {
            if (ZOOM_SEM_VAL(pCtrl->transSem) < pCtrl->transSemInit)
            {
                if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->DispStrm))
                {
                    ZOOM_SEM_POST(pCtrl->transSem);
                }
            }
        }
    }
    

    return AK_SUCCESS;
}


/*
 * @brief   update address transfer stream count and status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmUpdate(T_VideoZoomCtrl * pCtrl)
{
    T_U16 cnt = 0;
    
    AK_ASSERT_PTR(pCtrl, "Zoom_TaskCreate: param err", AK_EBADPARAM);

    cnt = pCtrl->transSemInit;
    pCtrl->transSemInit = Zoom_GetTransAddrCnt(pCtrl);

    if(cnt == pCtrl->transSemInit)
    {
        return AK_SUCCESS;
    }
    
    if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
    {
        ZOOM_SEM_DEINIT(pCtrl->transSem);
    }

    if (pCtrl->transSemInit == 0)
    {
        return AK_SUCCESS;
    }
    
    ZOOM_SEM_INIT(pCtrl->transSem, pCtrl->transSemInit);

    return AK_SUCCESS;
}



/*
 * @brief   reset address stream transfering 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TransStrmReset(T_VideoZoomCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Zoom_TransStrmReset: param err", AK_EBADPARAM);

    if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
    {
        ZOOM_SEM_DEINIT(pCtrl->transSem);
    }

    pCtrl->transSemInit = 0;

    return AK_SUCCESS;
}

/*
 * @brief   set  zoom function selection flag
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]flag:function selection flag
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_SetSrcModeFlag(T_VideoZoomCtrl * pCtrl, T_U32 flag)
{
    T_U32 tmpflag = 0;

    AK_ASSERT_PTR(pCtrl, "Zoom_SetSrcModeFlag: param err", AK_EBADPARAM);

    tmpflag = pCtrl->zoomMode;
    tmpflag |= flag;
    
    pCtrl->zoomMode = tmpflag;

    return AK_SUCCESS;
}

/*
 * @brief     clear  zoom function selection flag
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]flag:function selection flag
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_ClearSrcModeFlag(T_VideoZoomCtrl * pCtrl, T_U32 flag)
{
    T_U32 tmpflag = 0;

    AK_ASSERT_PTR(pCtrl, "Zoom_ClearSrcModeFlag: param err", AK_EBADPARAM);

    tmpflag = pCtrl->zoomMode;
    tmpflag &= ~flag;
    
    pCtrl->zoomMode = tmpflag;

    return AK_SUCCESS;
}


/*
 * @brief   create  task for process zoom frame
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]isStart:is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_TaskCreate(T_VideoZoomCtrl * pCtrl, T_BOOL isStart)
{
    T_S8 *pName = "Zoom";
    
    T_SUBTHREAD_INITPARAM    param;

    AK_ASSERT_PTR(pCtrl, "Zoom_TaskCreate: param err", AK_EBADPARAM);

    sprintf(pCtrl->name, "%s", pName);
    
#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
    if (AK_NULL == gZoomCamMsgQue)
    {
        gZoomCamMsgQue = Zoom_MsgQueCreate(VIDEOZOOM_CAM_INT_MSG_LEN, sizeof(T_ZOOM_MSG));
        if (AK_NULL == gZoomCamMsgQue)
        {
            Fwl_Print(C1, M_VZOOM, "create Zoom MsgQue err\n");
            return AK_EFAILED;
        }
    }
#endif    
    ZOOM_MUTEX_INIT(pCtrl->mutex);

    if (ZOOM_MUTEX_IS_ERR(pCtrl->mutex))
    {
        Fwl_Print(C1, M_VZOOM, "create mutex err\n");
        return AK_EFAILED;
    }

    pCtrl->transSemInit = 0;

    param.byPriority       = ZOOM_TASK_PRIO;
    param.ulTimeSlice      = ZOOM_TASK_SLICE;
    param.ulStackSize      = ZOOM_TASK_STACK;
    param.wMainThreadCls   = AKAPP_CLSID_MEDIA;

    param.pUserData        = (T_pVOID)pCtrl;
    param.fnEntry          = VideoZoom_TaskHandle;
    param.fnAbort          = AK_NULL; 
    param.pcszName         = pName;

    if(AK_SUCCESS != CSubThread_New(&(pCtrl->trd), &param, isStart))
    {
        Fwl_Print(C1, M_VZOOM, "Create %s_Thread Sub Thread Failure!\n",param.pcszName);
        return AK_EFAILED;
    }

    return AK_SUCCESS;
}


/*
 * @brief   destory  zoom task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  Zoom_TaskDestory(T_VideoZoomCtrl * pCtrl)
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
    
    if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
    {
        ZOOM_SEM_DEINIT(pCtrl->transSem);
    }
    
    if (!ZOOM_MUTEX_IS_ERR(pCtrl->mutex))
    {
        ZOOM_MUTEX_DEINIT(pCtrl->mutex);
    }

#ifdef VIDEOZOOM_CAM_INT_MSG_LEN
    if (AK_NULL != gZoomCamMsgQue)
    {
        Zoom_MsgQueDestory(gZoomCamMsgQue);
        gZoomCamMsgQue = AK_NULL;
    }
#endif   

    return AK_SUCCESS;
}

/*
 * @brief   get  frame from encode stream by process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] procFun: get process
 * @param[in] param: get process callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_GetEncFrameByProc(T_HANDLE hdl, T_STEM_GET_PROC procFun,T_pVOID param)
{
    T_S32 lRet = AK_EFAILED;
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "VideoZoom_GetEncFrameByProc: param err", AK_EBADPARAM);
    

    if ((EVT_ZOOM_RUN == pCtrl->runStatus) \
        && ZOOM_IS_ENABLE_ENC(pCtrl))
    { 
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Enc begin\n");
        //ZOOM_MUTEX_LOCK(pCtrl->mutex);
        lRet = IFrmStrm_GetFrameByProc(pCtrl->EncStrm,procFun, param);
        if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
        {
            if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->EncStrm))
            {
                // release address transfer semphore 
                if (ZOOM_SEM_VAL(pCtrl->transSem) < pCtrl->transSemInit)
                {
                    ZOOM_SEM_POST(pCtrl->transSem);
                }
            }
        }
        //ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Enc end\n");
    }
    return lRet;
}

/*
 * @brief   get  encode stream zoom process is enable
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_TRUE--enable, else disable
 */
T_BOOL VideoZoom_IsEnableEnc(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    if (AK_NULL == pCtrl)
    {
        return AK_FALSE;
    }
    

    if (AK_NULL == pCtrl->EncStrm)
    {
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Enc Null\n");
        return AK_FALSE;
    }

    return (T_BOOL)(pCtrl->zoomMode & (ZOOM_MODE_ENCODE));
}


/*
 * @brief   get  frame from display stream by process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] procFun: get process
 * @param[in] param: get process callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_GetDispFrameByProc(T_HANDLE hdl, T_STEM_GET_PROC procFun,T_pVOID param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32 lRet = AK_EFAILED;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_GetDispFrameByProc: param err", AK_EBADPARAM);

    if ((EVT_ZOOM_RUN == pCtrl->runStatus) \
        && ZOOM_IS_ENABLE_DISP(pCtrl))
    {
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Disp begin\n");
        //ZOOM_MUTEX_LOCK(pCtrl->mutex);
        lRet = IFrmStrm_GetFrameByProc(pCtrl->DispStrm,procFun, param);
        
        if (!ZOOM_SEM_IS_ERR(pCtrl->transSem))
        {
            if (eSTRM_TYPE_ADDR == IFrmStrm_GetTransType(pCtrl->DispStrm))
            {
                // release address transfer semphore 
                if (ZOOM_SEM_VAL(pCtrl->transSem) < pCtrl->transSemInit)
                {
                    ZOOM_SEM_POST(pCtrl->transSem);
                }
            }
        }
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Disp end\n");
        //ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }
    
    return lRet;
}



T_BOOL VideoZoom_IsEnableDisp(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;


    if (AK_NULL == pCtrl)
    {
        return AK_FALSE;
    }

    if (AK_NULL == pCtrl->DispStrm)
    {
        //Fwl_Print(C3, M_VZOOM, "Call Zoom Get Enc Null\n");
        return AK_FALSE;
    }

    return (T_BOOL)(pCtrl->zoomMode & ZOOM_MODE_DISP);
}


/*
 * @brief   touch  encode stream status by frame buffer address
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] addr: frame buffer address
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_TouchEncFrameByAddr(T_HANDLE hdl, T_U8 *addr)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_TouchEncFrameByAddr: param err", AK_EBADPARAM);

    if (AK_NULL == pCtrl->EncStrm)
    {
        return AK_EFAILED;
    }
    else
    { 
        return IFrmStrm_TouchFrmByAddr(pCtrl->EncStrm, addr);
    }
}


/*
 * @brief   touch  display stream status by frame buffer address
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] addr: frame buffer address
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_TouchDispFrameByAddr(T_HANDLE hdl, T_U8 *addr)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_TouchDispFrameByAddr: param err", AK_EBADPARAM);


    if (AK_NULL == pCtrl->DispStrm)
    {
        return AK_EFAILED;
    }
    else
    { 
        return IFrmStrm_TouchFrmByAddr(pCtrl->DispStrm, addr);
    }
}

/*
 * @brief  check  video display  info
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in/out] FrameInfo: frame information
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_CheckDispInfo(T_VideoZoomCtrl * pCtrl, T_FRM_INFO *FrameInfo)
{
    T_RECT          rect={0};
    T_U32           destW=0, destH=0;
    T_U32           srcW=0,  srcH=0;

    AK_ASSERT_PTR(pCtrl, "Zoom_CheckDispInfo: src err", AK_EBADPARAM);

    destW = FrameInfo->rect.width;
    destH = FrameInfo->rect.height;
    srcW  = pCtrl->srcFrmData.info.rect.width;
    srcH  = pCtrl->srcFrmData.info.rect.height;

    //adjust display rect  by source  window size
    if (Fwl_CameraGetClipWin(&rect, srcW, srcH, destW, destH,eCAMCLIP_AUTO))
    {
        // calculate the absolute position
        FrameInfo->rect.left  += rect.left;
        FrameInfo->rect.top   += rect.top;
 
        FrameInfo->rect.width  = rect.width;
        FrameInfo->rect.height = rect.height;
        
        Fwl_Print(C3, M_VZOOM, "Disp[%dx%d]:%dx%d=>(%d,%d)%dx%d\n",
        destW, destH, srcW, srcH, FrameInfo->rect.left, FrameInfo->rect.top, 
        FrameInfo->rect.width, FrameInfo->rect.height); 

        return AK_SUCCESS;
    }
    
    return AK_EFAILED;
}

/*
 * @brief   open encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in/out] EncFrameInfo: stream information
 * @return  resulst AK_SUCCESS--success, else fail
 * @return  resulst AK_TRUE--enable, else disable
 */
T_S32 VideoZoom_OpenEncSrc(T_HANDLE hdl,T_FRM_INFO *EncFrameInfo, T_BOOL isEnableFocus)
{
#if (VIDEOZOOM_ENCODE_STRM_LEN <= 0)
#error  zoom encode stream length config err
#endif
    T_VideoZoomCtrl       *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32            lRet  = AK_EFAILED;
    T_STRM_TYPE      transtype = eSTRM_TYPE_DATA;
    T_STRM_BLK_TYPE  putSrcBlkType = eSTRM_BLK_TYPE_NOBLK;
    IFrmStrm        *tmpStrm = AK_NULL;
    T_FRM_INFO       encStrmInfo = {0};
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_OpenEnc: param err", AK_EBADPARAM);
    AK_ASSERT_PTR(EncFrameInfo, "VideoZoom_OpenEnc: param err", AK_EBADPARAM);

    encStrmInfo.type      = EncFrameInfo->type;
    encStrmInfo.width     = EncFrameInfo->width;
    encStrmInfo.height    = EncFrameInfo->height;
    encStrmInfo.size      = EncFrameInfo->size;
    encStrmInfo.rect.left = 0;
    encStrmInfo.rect.top  = 0;
    encStrmInfo.rect.width  = (T_LEN)encStrmInfo.width;
    encStrmInfo.rect.height = (T_LEN)encStrmInfo.height;


    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    //release old instance of stream
    if (AK_NULL != pCtrl->EncStrm)
    {
        Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_ENCODE);
        tmpStrm = pCtrl->EncStrm;
        pCtrl->EncStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
        tmpStrm = AK_NULL;
    }

    pCtrl->isEnableFocusWin  = AK_FALSE;

//编码流采用地址方式传送，即直接使用camera stream数据    
    transtype     = eSTRM_TYPE_ADDR;
    putSrcBlkType = eSTRM_BLK_TYPE_NOBLK;

    lRet = Zoom_StreamInit(&tmpStrm, VIDEOZOOM_ENCODE_STRM_LEN, VIDEOZOOM_STRM_DEBUG_ENABLE,
           &encStrmInfo,      transtype,
           eSTRM_BLK_TYPE_BLK, AK_FALSE,
           putSrcBlkType, AK_FALSE);

    if (AK_IS_FAILURE(lRet))
    {
         Fwl_Print(C3, M_VZOOM, "Create Enc Strm Err @%d\n",__LINE__);
    }
    else
    {
        pCtrl->EncStrm = tmpStrm;
    }

	ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    return lRet;

}


/*
 * @brief   close encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_CloseEncSrc(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    IFrmStrm  *tmpStrm = AK_NULL;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_CloseEncSrc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    if (AK_NULL != pCtrl->EncStrm)
    {
        Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_ENCODE);
        tmpStrm = pCtrl->EncStrm;

        pCtrl->EncStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
    }
    Zoom_TransStrmUpdate(pCtrl);
    
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}


/*
 * @brief   enable encode stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnable: is enable
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_EnableEncSrc(T_HANDLE hdl, T_BOOL isEnable)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_BOOL isCurEnable = AK_FALSE;

    if (AK_NULL == pCtrl)
    {
       return AK_EBADPARAM;
    }


    if (pCtrl->zoomMode & ZOOM_MODE_ENCODE)
    {
        isCurEnable = AK_TRUE;
    }
    else
    {
        isCurEnable = AK_FALSE;
    }

    if (isCurEnable != isEnable)
    {
        // reset address transfer first(this semphore maybe locked at other thread)
        Zoom_TransStrmReset(pCtrl);

        // cancle stream protect
        if (AK_NULL != pCtrl->EncStrm)
        {
            IFrmStrm_Stop(pCtrl->EncStrm);
        }

        ZOOM_MUTEX_LOCK(pCtrl->mutex);

        if (isEnable)
        {
            // rebuilt  stream protect  info
            if (AK_NULL != pCtrl->EncStrm)
            {
                IFrmStrm_Start(pCtrl->EncStrm);
            }
            Zoom_SetSrcModeFlag(pCtrl, ZOOM_MODE_ENCODE);
        }
        else
        {
            Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_ENCODE);
        }

        // rebuild address transfer protect information
        Zoom_TransStrmUpdate(pCtrl);
        
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }
    return AK_SUCCESS;

}


/*
 * @brief   set  encode stream zoom process information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pInfo: zoom information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetEncInfo(T_HANDLE hdl, T_FRM_INFO *pInfo)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetEncInfo: param err", AK_EBADPARAM);
    AK_ASSERT_PTR(pInfo, "VideoZoom_SetEncInfo: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    if (AK_NULL != pCtrl->EncStrm)
    {
        IFrmStrm_SetFrmInfo(pCtrl->EncStrm, pInfo);
    }
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}



/*
 * @brief   set  encode stream zoom process prepare callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom prepare callback
 * @param[in] param: oom prepare  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetEncPreProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetEncPreProc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->EncStrmPre.proc = proc;
    pCtrl->EncStrmPre.hdl  = param;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);

    return AK_SUCCESS;
}



/*
 * @brief   set  encode stream zoom process post callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom post callback
 * @param[in] param: zoom post callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetEncPostProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetEncPostProc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->EncStrmPost.proc = proc;
    pCtrl->EncStrmPost.hdl  = param;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}


/*
 * @brief   open display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in/out] EncFrameInfo: stream information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_OpenDispSrc(T_HANDLE hdl,T_FRM_INFO *DispFrameInfo)
{
#if (VIDEOZOOM_DISP_STRM_LEN <= 0)
#error  zoom display stream length config err
#endif
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32            lRet  = AK_EFAILED;
    IFrmStrm       *tmpStrm = AK_NULL;
//    DISPLAY_TYPE_DEV curDispmode;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_OpenDisp: param err", AK_EBADPARAM);
    AK_ASSERT_PTR(DispFrameInfo, "VideoZoom_OpenDisp: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    if (AK_NULL != pCtrl->DispStrm)
    {
        Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_DISP);
        tmpStrm = pCtrl->DispStrm;
        pCtrl->DispStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
        tmpStrm = AK_NULL;
    }

/*
    // check frame info  by zoon info
    curDispmode = Fwl_GetDispalyType();
    if (curDispmode < DISPLAY_TVOUT_PAL)
    {
        lRet = Zoom_CheckDispInfo(pCtrl, DispFrameInfo);
    }
    else
    {
        lRet = AK_SUCCESS;
    }
    
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_VZOOM, "get Disp Clip Win Err @%d\n",__LINE__);
    }
    else */
    if (AK_NULL == pCtrl->DispStrm)
    {//VIDEOZOOM_STRM_DEBUG_ENABLE
       lRet = Zoom_StreamInit(&tmpStrm, VIDEOZOOM_DISP_STRM_LEN, VIDEOZOOM_STRM_DEBUG_ENABLE,
              DispFrameInfo, eSTRM_TYPE_ADDR,
              eSTRM_BLK_TYPE_BLK, AK_FALSE,
              eSTRM_BLK_TYPE_NOBLK, AK_FALSE);
       
       if (AK_IS_FAILURE(lRet))
       {
           Fwl_Print(C3, M_VZOOM, "Create Disp Strm Err @%d\n",__LINE__);
       }
       else
       {
           pCtrl->DispStrm = tmpStrm;
       }
    }
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    return lRet;

}


/*
 * @brief   close display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_CloseDispSrc(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    IFrmStrm  *tmpStrm = AK_NULL;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_CloseDispSrc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    if (AK_NULL != pCtrl->DispStrm)
    {
        Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_DISP);
        tmpStrm = pCtrl->DispStrm;
        pCtrl->DispStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
    }
    
    Zoom_TransStrmUpdate(pCtrl);
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}



/*
 * @brief   enable display stream zoom process
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnable: is enable
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_EnableDispSrc(T_HANDLE hdl, T_BOOL isEnable)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_BOOL isCurEnable = AK_FALSE;

    if (AK_NULL == pCtrl)
    {
       return AK_EBADPARAM;
    }

    if (pCtrl->zoomMode & ZOOM_MODE_DISP)
    {
        isCurEnable = AK_TRUE;
    }
    else
    {
        isCurEnable = AK_FALSE;
    }
    
    if (isCurEnable != isEnable)
    {
        // reset address transfer first(this semphore maybe locked at other thread)
        Zoom_TransStrmReset(pCtrl);

        //cancle stream protect
        if (AK_NULL != pCtrl->DispStrm)
        {
            IFrmStrm_Stop(pCtrl->DispStrm);
        }
    
        ZOOM_MUTEX_LOCK(pCtrl->mutex);

        if (isEnable)
        {
            // rebuilt  stream protect  info
            if (AK_NULL != pCtrl->DispStrm)
            {
                IFrmStrm_Start(pCtrl->DispStrm);
            }
            Zoom_SetSrcModeFlag(pCtrl, ZOOM_MODE_DISP);
        }
        else
        {
            Zoom_ClearSrcModeFlag(pCtrl, ZOOM_MODE_DISP);
        }
        
        // rebuild address transfer protect information
        Zoom_TransStrmUpdate(pCtrl);

        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }
    
    return AK_SUCCESS;

}

/*
 * @brief   set  display stream zoom process post callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom post callback
 * @param[in] param: zoom post callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetDispPostProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetDispPostProc: param err", AK_EBADPARAM);


    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->DispStrmPost.proc = proc;
    pCtrl->DispStrmPost.hdl  = param;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}


/*
 * @brief   set  display stream zoom process prepare callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: zoom prepare callback
 * @param[in] param: oom prepare  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetDispPreProc(T_HANDLE hdl, T_ZOOM_FRM_PROC proc, T_HANDLE param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetDispPostProc: param err", AK_EBADPARAM);


    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->DispStrmPre.proc = proc;
    pCtrl->DispStrmPre.hdl  = param;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);

    return AK_SUCCESS;
}


/*
 * @brief   set  display stream zoom process older judgement  callback
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] proc: older judgement callback
 * @param[in] param: older judgement  callback parameter
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetDispOlderProc(T_HANDLE hdl, T_ZOOM_FRM_GET_ORDER proc, T_HANDLE param)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetDispPostProc: param err", AK_EBADPARAM);


    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->DispStrmOrder.proc = proc;
    pCtrl->DispStrmOrder.hdl  = param;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}


/*
 * @brief   set  display stream zoom process information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pInfo: zoom information
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetDispInfo(T_HANDLE hdl, T_FRM_INFO *pInfo)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32            lRet  = AK_EFAILED;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetDispInfo: param err", AK_EBADPARAM);
    AK_ASSERT_PTR(pInfo, "VideoZoom_SetDispInfo: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    if (AK_NULL != pCtrl->DispStrm)
    {
        lRet = Zoom_CheckDispInfo(pCtrl, pInfo);
        if (AK_IS_SUCCESS(lRet))
        {
            IFrmStrm_SetFrmInfo(pCtrl->DispStrm, pInfo);
        }
        else
        {
            Fwl_Print(C1, M_VZOOM, "get Disp Clip Win Err @%d\n",__LINE__);
        }
    }
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;

}



/*
 * @brief   create video zoom handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in] focusLvl: default focuslvevel	
 * @param[in] width/height:video srouce size	
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE VideoZoom_Open(T_U8 focusLvl, T_U32 width, T_U32 height)
{
    T_VideoZoomCtrl              *hdl      = AK_NULL;
    T_S32                  lRet      = AK_EFAILED;
    

    hdl = Fwl_Malloc(sizeof(T_VideoZoomCtrl));
     
    if (AK_NULL == hdl)
    {
        Fwl_Print(C3, M_VZOOM, "Malloc Err @%d\n",__LINE__);
        return 0;
    }

    memset(hdl, 0 , sizeof(T_VideoZoomCtrl));
    
    hdl->zoomMode             = ZOOM_MODE_NONE;
    hdl->runStatus            = EVT_ZOOM_NULL; 
    hdl->focusLvl             = focusLvl;
#if (SDRAM_MODE == 8)
    hdl->isEnableFocusWin     = AK_FALSE;
#else
	hdl->isEnableFocusWin     = AK_TRUE;
#endif
    hdl->motionDetectInterval = ZOOM_MDETECT_INTERVAL_MIN;
    hdl->motionDetectEnable   = AK_FALSE;
    hdl->Is02_8M_QVGA         = AK_FALSE;
    
    lRet = Zoom_VideoSrcOpen(hdl);
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_VZOOM, "open src Err @%d\n",__LINE__);
        VideoZoom_Close((T_HANDLE)hdl);
        hdl = AK_NULL;
        return 0;
    }

    lRet = Zoom_VideoSrcStart(hdl, width, height);
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_VZOOM, "Start src Err @%d\n",__LINE__);
        VideoZoom_Close((T_HANDLE)hdl);
        hdl = AK_NULL;
        return 0;
    }
    
    
    lRet = Zoom_TaskCreate(hdl, AK_FALSE);
    
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_VZOOM, "Task Create Err @%d\n",__LINE__);
        VideoZoom_Close((T_HANDLE)hdl);
        hdl = AK_NULL;
        return 0;
    }
    Fwl_Print(C3, M_VZOOM, "Create Stream hdl=%x @%d\n",hdl,__LINE__);
    return (T_HANDLE)hdl;
}


/*
 * @brief   start video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_Start(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_Start: param err", AK_EBADPARAM);

#ifdef VIDEOZOOM_2D_INIT_INNER    
    Enable_2DModule(); 
#endif
    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_RUN);
    ISubThread_Resume(pCtrl->trd);


	if (!pCtrl->motionDetectEnable)
		MDetect_Stop(pCtrl->motionDetectorHdl);

    return AK_SUCCESS;
}


/*
 * @brief   restart video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] width/height:video srouce size	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_Restart(T_HANDLE hdl, T_U32 width, T_U32 height)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32      lRet  = AK_EFAILED;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_Restart: param err", AK_EBADPARAM);

    if ((0 == width) || (0 == height))
    {
        Fwl_Print(C2, M_VZOOM, "VideoZoom_Restart: param err\n");
        return AK_EFAILED;
    }
    
    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_SUSPEND);
    
    lRet = Zoom_VideoSrcRestart(pCtrl,width,height);
    
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C1, M_VZOOM, "restart src Err @%d\n",__LINE__);
        return AK_EFAILED;
    }

    
    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_RUN);
    
    return AK_SUCCESS;
}


/*
 * @brief   pause video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_Pause(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "VideoZoom_Pause: param err", AK_EBADPARAM);

    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_SUSPEND);

	if (!pCtrl->motionDetectEnable)
	{
		T_U16 width,height;

		MDetect_GetWin(pCtrl->motionDetectorHdl, &width,&height);
		MDetect_Start(pCtrl->motionDetectorHdl, width,height);
	}
    
    return AK_SUCCESS;
}

/*
 * @brief   close video zoom  process
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32  VideoZoom_Close(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    IFrmStrm       *tmpStrm = AK_NULL;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_Close: param err", AK_EBADPARAM);

    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_CLOSE);

    pCtrl->zoomMode = ZOOM_MODE_NONE;
    Zoom_VideoSrcStop(pCtrl);
    
#ifdef VIDEOZOOM_2D_INIT_INNER    
    Close_2DModule();
#endif

    Zoom_TaskDestory(pCtrl);
    
    if (AK_NULL != pCtrl->DispStrm)
    {
        tmpStrm = pCtrl->DispStrm;
        pCtrl->DispStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
    }
    
    if (AK_NULL != pCtrl->EncStrm)
    {
        tmpStrm = pCtrl->EncStrm;
        pCtrl->EncStrm = AK_NULL;
        IFrmStrm_Release(tmpStrm);
    }
    
    Zoom_VideoSrcClose(pCtrl);

    Fwl_Free(pCtrl);
    Fwl_Print(C3, M_VZOOM, "VideoZoom_Close Ok!\n");
    return AK_SUCCESS;
}

/*
 * @brief   get camera focus is enable
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return  resulst AK_TRUE--enable, else disable
 */
T_BOOL VideoZoom_IsEnableFocusWin(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "VideoZoom_IsEnableFocusWin: param err", AK_FALSE);

    return pCtrl->isEnableFocusWin;
}

/*
 * @brief   set camera stream source focus level information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] focusLvl: focus level
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_SetFocusLvl(T_HANDLE hdl, T_U8 focusLvl, T_BOOL isOnVideo)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "VideoZoom_SetFocusLvl: param err", AK_EBADPARAM);

    if ((focusLvl <= ZOOM_FOCUSE_LVL_MAX)\
        && (focusLvl != pCtrl->focusLvl))
    {
        ZOOM_MUTEX_LOCK(pCtrl->mutex);
        pCtrl->focusLvl = focusLvl;
        Zoom_VideoFocusWinUpdate(pCtrl, isOnVideo);
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }
    
    return AK_SUCCESS;
}

/*
 * @brief   get camera focus windows
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[out] rect: focus windows
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_GetFoucsWin(T_HANDLE hdl, T_RECT *rect)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;

    AK_ASSERT_PTR(pCtrl, "VideoZoom_GetFoucsWin: param err", AK_EBADPARAM);
    AK_ASSERT_PTR(rect, "VideoZoom_GetFoucsWin: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    rect->left   = pCtrl->srcFrmData.info.rect.left;
    rect->top    = pCtrl->srcFrmData.info.rect.top;
    rect->width  = pCtrl->srcFrmData.info.rect.width;
    rect->height = pCtrl->srcFrmData.info.rect.height;
    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    
    return AK_SUCCESS;
}

/*
 * @brief   close camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_CloseVideoSrc(T_HANDLE hdl)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    AK_ASSERT_PTR(pCtrl, "VideoZoom_CloseVideoSrc: param err", AK_EBADPARAM);

    Zoom_CtrlEvtSet(pCtrl,EVT_ZOOM_SUSPEND);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    pCtrl->zoomMode = ZOOM_MODE_NONE;

    Zoom_VideoSrcStop(pCtrl);
    Zoom_VideoSrcClose(pCtrl);

    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);

    return AK_SUCCESS;
}



/*
 * @brief   open camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_OpenVideoSrc(T_HANDLE hdl, T_U32 width, T_U32 height)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32      lRet      = AK_EFAILED;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_OpenVideoSrc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);
    
    lRet = Zoom_VideoSrcOpen(pCtrl);
    
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C3, M_VZOOM, "open src Err @%d\n",__LINE__);
        return AK_EFAILED;
    }
    lRet = Zoom_VideoSrcRestart(pCtrl, width, height);

    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);

    return lRet;
}

/*
 * @brief   restart camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_RestartVideoSrc(T_HANDLE hdl, T_U32 width, T_U32 height)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_S32      lRet      = AK_EFAILED;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_RestartVideoSrc: param err", AK_EBADPARAM);

    ZOOM_MUTEX_LOCK(pCtrl->mutex);

    lRet = Zoom_VideoSrcRestart(pCtrl, width, height);

    ZOOM_MUTEX_UNLOCK(pCtrl->mutex);

    return lRet;
}


/*
 * @brief   enable motion detect function
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isEnalbe: is open
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_DetectEnable(T_HANDLE hdl, T_BOOL isEnalbe)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    AK_ASSERT_PTR(pCtrl, "VideoZoom_DetectEnable: param err", AK_EBADPARAM);

    if (isEnalbe != pCtrl->motionDetectEnable)
    {
        ZOOM_MUTEX_LOCK(pCtrl->mutex);
        pCtrl->motionDetectEnable = isEnalbe;
        pCtrl->motionDetectCnt = 0;
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }

    return AK_SUCCESS;
}




/*
 * @brief   set  motion detect interval
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] intervalMs: detect interval time(millsecond)
 * @return  resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoZoom_DetectSetInterval(T_HANDLE hdl, T_U32 intervalMs)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    AK_ASSERT_PTR(pCtrl, "VideoZoom_DetectSetInterval: param err", AK_EBADPARAM);

    if (intervalMs  > ZOOM_MDETECT_INTERVAL_MIN)
    {
        ZOOM_MUTEX_LOCK(pCtrl->mutex);
        pCtrl->motionDetectInterval= intervalMs;
        ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
    }

    return AK_SUCCESS;
}


/*
 * @brief   motion detect  current status is mvoing
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] pData: current detect data
 * @return  resulst AK_TRUE--moving, else no moving
 */
T_BOOL VideoZoom_DetectIsMoving(T_HANDLE hdl, T_U8 *pData)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_BOOL isMoving = AK_FALSE;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_DetectIsMoving: param err", AK_FALSE);

    if ((0 != pCtrl->motionDetectorHdl) \
        && (pCtrl->motionDetectEnable) )
    {
        if(MDetect_IsRun(pCtrl->motionDetectorHdl))
        {
           ZOOM_MUTEX_LOCK(pCtrl->mutex);
           Zoom_DetectStatusUpdate(pCtrl, pData);
           isMoving = pCtrl->motionDetectStatus;
           ZOOM_MUTEX_UNLOCK(pCtrl->mutex);
        }
    }
    
    return isMoving;
}


/*
 * @brief   motion detect  current status is same with the parmeter isMoving
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] hdl: zoom handle
 * @param[in] isMoving: detect status check
 * @return  resulst AK_TRUE--same, else not same
 */
T_BOOL VideoZoom_DetectStatusCheck(T_HANDLE hdl, T_BOOL isMoving)
{
    T_VideoZoomCtrl *pCtrl = (T_VideoZoomCtrl *)hdl;
    T_BOOL     detectStatus = AK_FALSE;
    
    AK_ASSERT_PTR(pCtrl, "VideoZoom_DetectStatusCheck: param err", !isMoving);

    if ((0 != pCtrl->motionDetectorHdl) \
        && (pCtrl->motionDetectEnable) )
    {   
         // if  ismoving
         if (isMoving)
         {
            return pCtrl->motionDetectStatus;
         }
         // if is not moving
         else
         {
             return !(pCtrl->motionDetectStatus);
         }
    }
    else
    {
        detectStatus = !isMoving;
    }
    
    return detectStatus;
}

/*
 * @brief   frame scale
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pSrcFrame: source frame
 * @param[in/out] pDestFrame: destination frame
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_FrameScale(T_FRM_DATA *pSrcFrame, T_FRM_DATA *pDestFrame)
{
    T_U32 yuvInAddr[3];
    T_U32 yuvOutAddr[3];
    T_U8  srcNum;
    T_U8  destNum;
    T_U8  ret = 0;
    T_U32 size = 0;

    AK_ASSERT_PTR(pSrcFrame, "Zoom_FrameScale: src err", AK_EBADPARAM);
    AK_ASSERT_PTR(pSrcFrame->pBuffer, "Zoom_FrameScale: src err", AK_EBADPARAM);
    AK_ASSERT_PTR(pDestFrame, "Zoom_FrameScale: dest err", AK_EBADPARAM);
    AK_ASSERT_PTR(pDestFrame->pBuffer, "Zoom_FrameScale: dest err", AK_EBADPARAM);

    if (FORMAT_YUV420 == pSrcFrame->info.type)
    {
        srcNum = 3;
        size = (pSrcFrame->info.width* pSrcFrame->info.height);
        yuvInAddr[0] = (T_U32)((T_U8 *)pSrcFrame->pBuffer);
         yuvInAddr[1] = yuvInAddr[0] + size;
         yuvInAddr[2] = yuvInAddr[1] + (size>>2);
    }
    else if (FORMAT_RGB565 == pSrcFrame->info.type)
    {
        srcNum = 1;
        yuvInAddr[0] = (T_U32)((T_U8 *)pSrcFrame->pBuffer);
        yuvInAddr[1] = 0;
        yuvInAddr[2] = 0;
    }
    else
    {
           return AK_EBADPARAM;
    }

    if (FORMAT_YUV420 == pDestFrame->info.type)
    {
        destNum = 3;
        size = (pDestFrame->info.width* pDestFrame->info.height);
        yuvOutAddr[0] = (T_U32)((T_U8 *)pDestFrame->pBuffer);
         yuvOutAddr[1] = yuvOutAddr[0] + size;
         yuvOutAddr[2] = yuvOutAddr[1] + (size>>2);
    }
    else if (FORMAT_RGB565 == pDestFrame->info.type)
    {
        destNum = 1;
        yuvOutAddr[0] = (T_U32)((T_U8 *)pDestFrame->pBuffer);
        yuvOutAddr[1] = 0;
        yuvOutAddr[2] = 0;
    }
    else
    {
       return AK_EBADPARAM;
    }

    //Fwl_ScaleParamCheck(&pSrcFrame->info.rect, &pDestFrame->info.rect,
                        // pSrcFrame->info.width, pSrcFrame->info.height);
/*DEBUG_VIDEOZOOM_TRACERUN    
    Fwl_Print(C3, M_VZOOM, ">>>>>>Scale_%dx%d,size=%d,type=%d,rect(%d,%d,%dx%d)"\
        "==> %dx%d,size=%d,type=%d,rect(%d,%d,%dx%d)",
        pSrcFrame->info.width,pSrcFrame->info.height,
        pSrcFrame->info.size,pSrcFrame->info.type,
        pSrcFrame->info.rect.left,pSrcFrame->info.rect.top,
        pSrcFrame->info.rect.width,pSrcFrame->info.rect.height,

        pDestFrame->info.width,pDestFrame->info.height,
        pDestFrame->info.size,pDestFrame->info.type,
        pDestFrame->info.rect.left,pDestFrame->info.rect.top,
        pDestFrame->info.width,pDestFrame->info.height);
*/

     //Fwl_ScaleConvertNoBlock
    //Fwl_Print(C3, M_VZOOM, "{Scale");
    ret = Fwl_ScaleConvert(yuvInAddr, srcNum, 
         (T_U16)pSrcFrame->info.width, &pSrcFrame->info.rect, pSrcFrame->info.type,
         yuvOutAddr, destNum, 
         (T_U16)pDestFrame->info.width,&pDestFrame->info.rect,pDestFrame->info.type,
          0, 0);
    //Fwl_Print(C3, M_VZOOM, "%d}", ret);

    return ret;
}



/*
 * @brief   open camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   Zoom_VideoSrcOpen(T_VideoZoomCtrl * pCtrl)
{
    T_U32 szbuf = 0;
    T_U32 width =0 , height = 0;

    AK_ASSERT_PTR(pCtrl, "Zoom_VideoSrcOpen: srcHdl err", AK_EBADPARAM);

    if (!Fwl_CameraGetMaxSize(&width, &height))
    {
        Fwl_Print(C2, M_VZOOM, "get Camera size err @%d\n",__LINE__);
        return AK_EFAILED;
    }
    // 计算最大支持的象素占用的字节
    szbuf = (width * height) * 3/2;
    //-------------------------------  
    if (AK_NULL == pCtrl->srcCambuffer.camBuffer1.dY)
    {
       pCtrl->srcCambuffer.camBuffer1.dY = (T_U8 *)Fwl_Malloc(szbuf + 64);
    }

    if (AK_NULL == pCtrl->srcCambuffer.camBuffer1.dY)
    {
       Fwl_Print(C1, M_VZOOM, "Malloc Err @%d\n",__LINE__);
       goto MALLOC_REC_ERROR;
    }
    //-------------------------------  
    if (AK_NULL == pCtrl->srcCambuffer.camBuffer2.dY)
    {
       pCtrl->srcCambuffer.camBuffer2.dY = (T_U8 *)Fwl_Malloc(szbuf + 64);
    }

    if (AK_NULL == pCtrl->srcCambuffer.camBuffer2.dY)
    {
       Fwl_Print(C1, M_VZOOM, "Malloc Err @%d\n",__LINE__);
       goto MALLOC_REC_ERROR;
    }    
    //-------------------------------  
    if (AK_NULL == pCtrl->srcCambuffer.camBuffer3.dY)
    {
       pCtrl->srcCambuffer.camBuffer3.dY = (T_U8 *)Fwl_Malloc(szbuf + 64);
    }
    
    if (AK_NULL == pCtrl->srcCambuffer.camBuffer3.dY)
    {
        Fwl_Print(C1, M_VZOOM, "Malloc Err @%d\n",__LINE__);
        goto MALLOC_REC_ERROR;
    }
    
    pCtrl->srcFrmData.info.size  = szbuf;
    pCtrl->srcFrmData.info.type  = FORMAT_YUV420;

    if (0 == pCtrl->motionDetectorHdl)
    {
        pCtrl->motionDetectorHdl  = MDetect_Open(width, height, 0);
    }
    Fwl_Print(C3, M_VZOOM, "Open Src Ok (Max pixel:%d) \n", szbuf);

    return AK_SUCCESS;

MALLOC_REC_ERROR:

    Zoom_VideoSrcClose(pCtrl);

    return AK_ENOMEMORY;
}

//----
/*
 * @brief   start camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   Zoom_VideoSrcStart(T_VideoZoomCtrl * pCtrl, T_U32 width, T_U32 height)
{
    T_U32 szbuf = 0;
    T_BOOL ret;
    
    AK_ASSERT_PTR(pCtrl, "Zoom_VideoSrcStart: srcHdl err", AK_EBADPARAM);

	if (width < 640 || height < 480)
	{
	//camera 的VGA 的输出通常是正常的，但QVGA和CIF的输出可能不正常
	//使用VGA的输出，然后通过camera 控制器进行缩放
	    Fwl_CameraSetToRec(640, 480);
	    ret = Fwl_CameraSetWindows(640, 480);
	    if (!ret)
	    {
	        Fwl_Print(C2, M_VZOOM, "Set Win Fail, W: %d H: %d\n", width, height);
	        return AK_EBADPARAM;
	    }
	}else
	{
	    Fwl_CameraSetToRec(width, height);
	    ret = Fwl_CameraSetWindows(width, height);
	    if (!ret)
	    {
	        Fwl_Print(C2, M_VZOOM, "Set Win Fail, W: %d H: %d\n", width, height);
	        return AK_EBADPARAM;
	    }
    }
    //-------------------------------  

    szbuf = width * height;
    pCtrl->srcCambuffer.camBuffer1.dU = pCtrl->srcCambuffer.camBuffer1.dY + szbuf;
    pCtrl->srcCambuffer.camBuffer1.dV = pCtrl->srcCambuffer.camBuffer1.dU + (szbuf>>2);

    pCtrl->srcCambuffer.camBuffer2.dU = pCtrl->srcCambuffer.camBuffer2.dY + szbuf;
    pCtrl->srcCambuffer.camBuffer2.dV = pCtrl->srcCambuffer.camBuffer2.dU + (szbuf>>2);

    pCtrl->srcCambuffer.camBuffer3.dU = pCtrl->srcCambuffer.camBuffer3.dY + szbuf;
    pCtrl->srcCambuffer.camBuffer3.dV = pCtrl->srcCambuffer.camBuffer3.dU + (szbuf>>2);

    pCtrl->isSrcInit = Zoom_CameraSrcInit(width, height,
                &pCtrl->srcCambuffer.camBuffer1, \
                &pCtrl->srcCambuffer.camBuffer2, \
                &pCtrl->srcCambuffer.camBuffer3);

    if (pCtrl->isSrcInit)
    {
        pCtrl->srcFrmData.info.width  = width;
        pCtrl->srcFrmData.info.height = height;
        Zoom_VideoFocusWinUpdate(pCtrl, AK_FALSE);
    }else
    {
    	Fwl_Print(C3, M_VZOOM,"Zoom_CameraSrcInit failed!\n");
    	return AK_EFAILED;
    }

    if (0 != pCtrl->motionDetectorHdl)
    {
        pCtrl->motionDetectCnt = 0;
        MDetect_Start(pCtrl->motionDetectorHdl,width, height);
    }

    pCtrl->motionDetectTimeMs = get_tick_count();
    Fwl_Print(C3, M_VZOOM, "Start Src %s <%dx%d> \n", pCtrl->isSrcInit?"Ok":"Failed", pCtrl->srcFrmData.info.width,
                                        pCtrl->srcFrmData.info.height);

    return AK_SUCCESS;
}


/*
 * @brief   restart camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @param[in]width:video source width
 * @param[in]height:video source height
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   Zoom_VideoSrcRestart(T_VideoZoomCtrl * pCtrl, T_U32 width, T_U32 height)
{
    T_S32      lRet  = AK_EFAILED;
    AK_ASSERT_PTR(pCtrl, "Zoom_VideoSrcRestart: src err", AK_EBADPARAM);
    
    if ((pCtrl->srcFrmData.info.width != width) \
        || (pCtrl->srcFrmData.info.height != height))
    {
#if 1
        // stop pre config
        Zoom_VideoSrcStop(pCtrl);
        lRet = Zoom_VideoSrcStart(pCtrl, width,height);
#else
        camstream_change(width, height,
                    &pCtrl->srcCambuffer.camBuffer1, \
                    &pCtrl->srcCambuffer.camBuffer2, \
                    &pCtrl->srcCambuffer.camBuffer3);
        pCtrl->srcFrmData.info.width  = width;
        pCtrl->srcFrmData.info.height = height;

        Zoom_VideoFocusWinUpdate(pCtrl, AK_TRUE);
#endif
        if (AK_IS_FAILURE(lRet))
        {
            Fwl_Print(C1, M_VZOOM, "restart src Err @%d\n",__LINE__);
            return AK_EFAILED;
        }
    }
    else
    {
        pCtrl->motionDetectCnt = 0;
    }
    return AK_SUCCESS;
}

/*
 * @brief   stop camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcStop(T_VideoZoomCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Zoom_VideoSrcStop: src err", AK_EBADPARAM);

    if (0 != pCtrl->motionDetectorHdl)
    {
        MDetect_Stop(pCtrl->motionDetectorHdl);
    }

    if (pCtrl->isSrcInit)
    {
        Zoom_CameraSrcDeInit();
        pCtrl->isSrcInit = AK_FALSE;
        pCtrl->srcFrmData.info.width  = 0;
        pCtrl->srcFrmData.info.height = 0;
    }
    return AK_SUCCESS;
}


/*
 * @brief   close camera stream source
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoSrcClose(T_VideoZoomCtrl * pCtrl)
{
     AK_ASSERT_PTR(pCtrl, "Zoom_VideoSrcClose: src err", AK_EBADPARAM);

     if (0 != pCtrl->motionDetectorHdl)
     {
         MDetect_Close(pCtrl->motionDetectorHdl);
         pCtrl->motionDetectorHdl       = 0;
     }

     /**free YUV of video record*/
     if (AK_NULL != pCtrl->srcCambuffer.camBuffer1.dY)
     {
         pCtrl->srcCambuffer.camBuffer1.dY = Fwl_Free(pCtrl->srcCambuffer.camBuffer1.dY);
         pCtrl->srcCambuffer.camBuffer1.dU = AK_NULL;
         pCtrl->srcCambuffer.camBuffer1.dV = AK_NULL;
     }
     
     if (AK_NULL != pCtrl->srcCambuffer.camBuffer2.dY)
     {
         pCtrl->srcCambuffer.camBuffer2.dY = Fwl_Free(pCtrl->srcCambuffer.camBuffer2.dY);
         pCtrl->srcCambuffer.camBuffer2.dU = AK_NULL;
         pCtrl->srcCambuffer.camBuffer2.dV = AK_NULL;
     }
     
     if (AK_NULL != pCtrl->srcCambuffer.camBuffer3.dY)
     {
         pCtrl->srcCambuffer.camBuffer3.dY = Fwl_Free(pCtrl->srcCambuffer.camBuffer3.dY);
         pCtrl->srcCambuffer.camBuffer3.dU = AK_NULL;
         pCtrl->srcCambuffer.camBuffer3.dV = AK_NULL;
     }
     pCtrl->srcFrmData.info.size = 0;
     return AK_SUCCESS;
}

  
/*
 * @brief   update camera stream source information
 * @author WangXi
 * @date    2011-10-25
 * @param[in/out] pCtrl: zoom handle
 * @return  resulst AK_SUCCESS--success, else fail
 */
static T_S32 Zoom_VideoFocusWinUpdate(T_VideoZoomCtrl * pCtrl, T_BOOL isOnVideo)
{
#ifdef OS_ANYKA
//    T_U32 size;
//    T_CAMERA_BUFFER camBuffer1;// camera init buffer 1
//    T_CAMERA_BUFFER camBuffer2;//camera init buffer 2
//    T_CAMERA_BUFFER camBuffer3;//camera init buffer 3
    
    AK_ASSERT_PTR(pCtrl, "Zoom_VideoFocusWinUpdate: src err", AK_EBADPARAM);

    Fwl_CameraGetFocusWin(&pCtrl->srcFrmData.info.rect, pCtrl->focusLvl, ZOOM_FOCUSE_LVL_MAX,
                       pCtrl->srcFrmData.info.width,pCtrl->srcFrmData.info.height);
            
    Fwl_Print(C3, M_VZOOM, "\nFocus %d lvl Win %dx%d =>(%d,%d)%dx%d\n",pCtrl->focusLvl,
        pCtrl->srcFrmData.info.width,pCtrl->srcFrmData.info.height,
        pCtrl->srcFrmData.info.rect.left,pCtrl->srcFrmData.info.rect.top,
        pCtrl->srcFrmData.info.rect.width,pCtrl->srcFrmData.info.rect.height);
#endif

    return AK_SUCCESS;
}

#endif


