/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: log_videoDisp.c
 * Function:video asyn display logic
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/

#include "Drv_api.h"
#include "AKAppMgr.h"
#include "AKSubThread.h"
#include "log_videoZoom.h"
#include "log_videoDisp.h"
#include "Fwl_gui.h"

extern T_RECT PreView2DWin;

//=================  Macro  Define (Debug)===========================
#ifdef PLATFORM_DEBUG_VER
#define VIDEODISP_DEBUG_VER
#endif
/**
 * watch dog time (second)  if equal zero , will colse
 */
#ifdef VIDEODISP_DEBUG_VER
#define DISP_TASK_WATCH_DOG_SEC    (4)
#endif

//#define DEBUG_DISP_TRACERUN
#define DISP_LOG_DEBUG             //AkDebugOutput
//====================== Macro  Define ===========================
//-------------- Task Runtime Parameter Define--- 
/**
 * video display  task priority
 */
#define DISP_TASK_PRIO             (100)

/**
 * video display task time slice
 */
#define DISP_TASK_SLICE            (2)

/**
 * video display task run stack size(byte)
 */
#define DISP_TASK_STACK            (4*1024)


//--------------Video Display Task Configure Define--- 
/**
 * video display task name length
 */
#define DISP_TASK_NAME_LEN         (12)


// display deviece count 
#define   DISP_DEVIECE_CNT    (1)

//==================== Macro  Define (Ctrl)==========================
//--------------Video Display Task Status Define--- 
/**
 *task status: init status, meaning task is no ready
 */
#define EVT_DISP_NULL	    (0)

/**
 *task status: suspend, meaning task is do nothing
 */
#define EVT_DISP_SUSPEND	(0x01<<1)

/**
 *task status: run, meaning task is do someting
 */
#define EVT_DISP_RUN 	    (0x01<<2)

/**
 *task status: close, meaning task is cosed
 */
#define EVT_DISP_CLOSE	    (0x01<<3)

//===================== Type Define =============================
/**
 * video refresh mode
 */
typedef enum {
    DISP_REFR_MODE_PASSIVE = 0, //  refresh as passive mode, such as mpu.
    DISP_REFR_MODE_ACTIVE,  // refresh as active mode, such as rgb, tvout .etc
} T_DISP_REFRESH_MODE;

/**
 * video display control  struct
 */
typedef struct {
	ISubThread                   *trd; // video display thread handle
	T_hSemaphore                  mutex;// video display thread  share zone protect
	T_S8                          name[DISP_TASK_NAME_LEN];// name
    _VOLATILE T_HANDLE            SrcCtrl;// video display extern zoom moudle handle
	_VOLATILE T_U32               runStatus;//current status
	DISPLAY_TYPE_DEV              curDispMode;//current disp mode
	VIDEO_DISP_VIDEO_UI_CBF       video_ui_cbf;// user callback function for paint icon 
	                                           //on video
	VIDEO_DISP_MENU_UI_CBF        menu_ui_cbf;// user callback function for paint pop 
	                                          //menu(from ui ctrl default buffer )
	VIDEO_DISP_PAINT_UI_CHECK_CBF menu_ui_check;// user callback function for check if 
	                                            // need use pop menu buffer to display

    T_hSemaphore                  dispHwSem;// to protect refresh deviece'  work
    T_U32                         dispHwSemInit;

    T_DISP_REFRESH_MODE           curRefreshMode;
	T_U32                         curFps;// to store current refresh fps(for debug)
	T_U32                         curSysTime;// to calc current refresh fps(for debug)
} T_VideoDisplayCtrl;



//====================== Macro  Define(Function)=====================

#define DISP_MUTEX_INIT(mutex)       ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define DISP_MUTEX_IS_ERR(mutex)     (((mutex) <= 0) && ((mutex) > -100))
#define DISP_MUTEX_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define DISP_MUTEX_TRY_LOCK(mutex)    (0 < (AK_Try_Obtain_Semaphore((mutex), AK_SUSPEND)))
#define DISP_MUTEX_UNLOCK(mutex)      AK_Release_Semaphore((mutex))
#define DISP_MUTEX_DEINIT(mutex)      {\
	if (!(DISP_MUTEX_IS_ERR(mutex)))\
	{\
		AK_Delete_Semaphore(mutex);\
		(mutex) = 0;\
	}\
}

#define DISP_SEM_INIT(sem,cnt)       ((sem) = AK_Create_Semaphore((cnt), AK_PRIORITY))
#define DISP_SEM_IS_ERR(sem)         (((sem) <= 0) && ((sem) > -100))
#define DISP_SEM_VAL(sem)            (AK_Get_SemVal(sem))
#define DISP_SEM_WAIT(sem)            AK_Obtain_Semaphore((sem), AK_SUSPEND)
#define DISP_SEM_TRY_WAIT(sem)        (0 < (AK_Try_Obtain_Semaphore((sem), AK_SUSPEND)))
#define DISP_SEM_POST(sem)            AK_Release_Semaphore((sem))
#define DISP_SEM_DEINIT(sem)      {\
	if (!(DISP_SEM_IS_ERR(sem)))\
	{\
		AK_Delete_Semaphore(sem);\
		(sem) = 0;\
	}\
}

//====================== Local Function  Declare======================

/*
 * @brief   update video display task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_CtrlEvtUpdate(T_VideoDisplayCtrl * hdl, T_U32 status);

/*
 * @brief   set video display task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_CtrlEvtSet(T_VideoDisplayCtrl * hdl, T_U32 status);

/*
 * @brief   destory display task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_TaskDestory(T_VideoDisplayCtrl * pCtrl);


/*
 * @brief   create video display task
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] isStart: is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_TaskCreate(T_VideoDisplayCtrl * hdl, T_BOOL isStart);

/*
 * @brief   display process for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 */
static T_VOID VideoDisp_TaskHandle(T_VideoDisplayCtrl * hdl);

/*
 * @brief   get video display information by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: deviece refresh mode
 * @param[out] devInfo: display infomation
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_GetDevInfo(DISPLAY_TYPE_DEV  TvOutMode, T_FRM_INFO *devInfo);


/*
 * @brief   get video refresh zone by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: deviece refresh mode
 * @param[out] refreshRect: refresh zone
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_GetRefreshZone(DISPLAY_TYPE_DEV  TvOutMode, T_RECT *refreshRect);


/*
 * @brief   update video refresh informatin by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @param[in] TvOutMode: display mode
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_UpdateRefreshMode(T_VideoDisplayCtrl * pCtrl, DISPLAY_TYPE_DEV  TvOutMode);

/*
 * @brief   get display buffer size for refresh  by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: display mode
 * @return	the refresh buffer size(byte)
 */
 #if 0
static T_U32  VideoDisp_GetDevFrmSize(DISPLAY_TYPE_DEV  TvOutMode);
#endif

/*
 * @brief   get display buffer max for refresh  by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: display mode
 * @return	the refresh buffer max size(byte)
 */
 #if 0
static T_U32  VideoDisp_GetDevFrmMaxSize(DISPLAY_TYPE_DEV  TvOutMode);
#endif


/*
 * @brief   get display zoom order for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in] T_HANDLE: get order handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_ZoomOrderProc(T_HANDLE hdl);

/*
 * @brief   process frame whitch get from zoom stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_FrameProc(T_FRM_DATA *pFrame, T_VideoDisplayCtrl *pCtrl);

/*
 * @brief   prepare process display zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in] T_HANDLE: prepare process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_ZoomPreProc(T_FRM_DATA *pFrame, T_HANDLE hdl);

/*
 * @brief   post process display zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_ZoomPostProc(T_FRM_DATA *pFrame, T_HANDLE hdl);

/*
 * @brief   set display lib refresh callback(when refresh one frame, this func will be called)
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pFuncCB: deviece refresh finished callback function
 */
static T_VOID VideoDisp_SetRefreshCbf(T_REFRESH_CALLBACK  pFuncCB);

/*
 * @brief   define display lib refresh callback(when refresh one frame, this func will be called)
 * @author WangXi
 * @date	2011-10-25
 * @param[in] addr: deviece refresh buffer address
 * @param[in] Param: when start refresh, user's parameter  tansfer for callback
 */
static T_VOID VideoDisp_RefreshEndCbf(T_U8* addr,T_U8 *Param);

/*
 * @brief   refresh a frame with ui
 * @author WangXi
 * @date	2011-10-25
 * @param[[in/out] pFrame:  frame of display stream
 * @param[in] isShowPopMenu:  is enable show pop menu(refresh mmi control buffer)
 * @param[in] isEnableInnerTvout:  is enable show inner tvout(refresh inner tvout buffer)
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_RefreshFrame(T_FRM_DATA *pFrame, 
     			T_BOOL isEnableInnerTvout,T_VideoDisplayCtrl *pCtrl);

/*
 * @brief   init video refresh informatin 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_RefreshInit(T_VideoDisplayCtrl * pCtrl);

/*
 * @brief   DeInit video refresh informatin 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_RefreshDeInit(T_VideoDisplayCtrl * pCtrl);

/*
 * @brief   Wait video deviece refresh finished
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @param[in] pFrame: frame for refresh
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_RefreshWait(T_VideoDisplayCtrl * pCtrl, T_FRM_DATA *pFrame);

E_LCD_TYPE VideoDisp_LCD_Type(T_VOID);

//========================== Function  Define ======================
/*
 * @brief   update video display task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_CtrlEvtUpdate(T_VideoDisplayCtrl * hdl, T_U32 status)
{
    AK_ASSERT_PTR(hdl, "Param err", AK_EBADPARAM);

    // directly set, need not wait
    if (status != hdl->runStatus)
    {
        Fwl_Print(C3, M_DISPLAY, "Disp::update event 0x%x begin.\n", status);
        hdl->runStatus = status;
        Fwl_Print(C3, M_DISPLAY, "Disp::update event Ok\n", status);
    }
    
    return AK_SUCCESS;
}

/*
 * @brief   set video display task status
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] status: task status
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_CtrlEvtSet(T_VideoDisplayCtrl * hdl, T_U32 status)
{
	AK_ASSERT_PTR(hdl, "Param err", AK_EBADPARAM);

	if (hdl->runStatus == status)
	{
		Fwl_Print(C3, M_DISPLAY, "Disp status has seted\n");
		return AK_SUCCESS;
	}

	// directly set, need not wait
	if (EVT_DISP_RUN != hdl->runStatus)
    {
       hdl->runStatus = status;
	   Fwl_Print(C3, M_DISPLAY, "Disp@@set event Ok\n");
	   return AK_SUCCESS;
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY, "Disp::set event 0x%x begin...\n", status);
		DISP_MUTEX_LOCK(hdl->mutex);
		hdl->runStatus = status;
		DISP_MUTEX_UNLOCK(hdl->mutex);
		Fwl_Print(C3, M_DISPLAY, "Disp::set event Ok\n", status);
	}
	return AK_SUCCESS;
}


/*
 * @brief   refresh a frame with ui
 * @author WangXi
 * @date	2011-10-25
 * @param[[in/out] pFrame:  frame of display stream
 * @param[in] isShowPopMenu:  is enable show pop menu(refresh mmi control buffer)
 * @param[in] isEnableInnerTvout:  is enable show inner tvout(refresh inner tvout buffer)
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_RefreshFrame(T_FRM_DATA *pFrame, 
     			T_BOOL isEnableInnerTvout,T_VideoDisplayCtrl *pCtrl)
{
    T_U8  *realDispBuff = AK_NULL;
    T_RECT refreshZone = {0};
	T_U32 off = 0;
	
    realDispBuff    = pFrame->pBuffer;

#ifdef DEBUG_DISP_TRACERUN   
    Fwl_Print(C3, M_DISPLAY, ">>>>>>disp=0x%x_%dx%d,size=%d,type=%d,rect(%d,%d,%dx%d)\n",
    pFrame->pBuffer,pFrame->info.width,pFrame->info.height,
    pFrame->info.size,pFrame->info.type,
    pFrame->info.rect.left,pFrame->info.rect.top,
    pFrame->info.rect.width,pFrame->info.rect.height);
#endif	
    // pre paint some ui on video buffer, such as time stamp, process bar.etc
    if (AK_NULL != pCtrl->video_ui_cbf)
    {
        (*pCtrl->video_ui_cbf)(pFrame);
    }


    VideoDisp_GetRefreshZone(DISPLAY_LCD_0, &refreshZone);

#ifdef DEBUG_DISP_TRACERUN   
    Fwl_Print(C3, M_DISPLAY, ">>>>>>disp=0x%x_%x,%dx%d,size=%d,type=%d,rect(%d,%d,%dx%d)\n",
    pFrame->pBuffer,realDispBuff,pFrame->info.width,pFrame->info.height,
    pFrame->info.size,pFrame->info.type,
    refreshZone.left,refreshZone.top,
    refreshZone.width,refreshZone.height);
#endif

	off = pFrame->info.rect.width * pFrame->info.rect.height;
	Fwl_RefreshYUV1(realDispBuff, realDispBuff+off, realDispBuff+off+off/4, 
	               pFrame->info.rect.width,pFrame->info.rect.height,
	               refreshZone.left,refreshZone.top, refreshZone.width,refreshZone.height);
#ifdef OS_ANYKA
//    lcd_asyn_refresh_RGB(DISPLAY_LCD_0, &refreshZone, realDispBuff, (T_U8*)pCtrl);
#endif

    return AK_SUCCESS;
}


/*
 * @brief   get display zoom order for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in] T_HANDLE: get order handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_ZoomOrderProc(T_HANDLE hdl)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_ZoomOrderProc: param err", ZOOM_STRM_ORDER_LAST);

    if (DISP_REFR_MODE_ACTIVE == pCtrl->curRefreshMode)
    {
        return ZOOM_STRM_ORDER_FIRST;
    }
    else
    {
        return ZOOM_STRM_ORDER_LAST;
    }
}

/*
 * @brief   post process display zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in] T_HANDLE: post process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_ZoomPostProc(T_FRM_DATA *pFrame, T_HANDLE hdl)
{
/**
 *  add zoom module post proc function here
 */
    return AK_SUCCESS;
}

/*
 * @brief   prepare process display zoom frame for zoom task callback
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in] T_HANDLE: prepare process handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_ZoomPreProc(T_FRM_DATA *pFrame, T_HANDLE hdl)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_ZoomPreProc: param err", AK_EBADPARAM);

    if (AK_NULL == pFrame)
    {
        return AK_EBADPARAM;
    }
    
    if (AK_NULL == pFrame->pBuffer)
    {
        return AK_EBADPARAM;
    }

    if (pCtrl->dispHwSemInit > 0)
    {
        VideoDisp_RefreshWait(pCtrl, pFrame);
        //mini_delay(2);//avoid tvout refresh topline is flash
    }

	return AK_SUCCESS;
}


/*
 * @brief   process frame whitch get from zoom stream
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pFrame:  frame of display stream
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_FrameProc(T_FRM_DATA *pFrame, T_VideoDisplayCtrl *pCtrl)
{
    if (AK_NULL == pFrame)
    {
        return AK_SUCCESS;
    }

    if (AK_NULL == pFrame->pBuffer)
    {
        return AK_SUCCESS;
    } 

    VideoDisp_RefreshFrame(pFrame, AK_TRUE, pCtrl);

/*
    if (0 == pCtrl->dispHwSemInit)
    {
        VideoDisp_RefreshWait(pCtrl, pFrame);
    }
*/	
    return AK_SUCCESS;
}


/*
 * @brief   display process for task handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 */
static T_VOID VideoDisp_TaskHandle(T_VideoDisplayCtrl * hdl)
{
	if (hdl)
	{
		Fwl_Print(C3, M_DISPLAY, "%s task entry\n", (char*)hdl->name);
	}
	
    while(hdl)
    {
#ifdef DISP_TASK_WATCH_DOG_SEC 
        AK_Feed_Watchdog(DISP_TASK_WATCH_DOG_SEC);
#endif
		if (EVT_DISP_RUN == hdl->runStatus)
		{
            //Fwl_Print(C3, M_DISPLAY, "<-F");
            DISP_MUTEX_LOCK(hdl->mutex);
            //Fwl_Print(C3, M_DISPLAY, "->");
			if (VideoZoom_IsEnableDisp(hdl->SrcCtrl))
			{
				VideoZoom_GetDispFrameByProc(hdl->SrcCtrl, VideoDisp_FrameProc, (T_pVOID)hdl);
			}
            
            DISP_MUTEX_UNLOCK(hdl->mutex);
            //Fwl_Print(C3, M_DISPLAY, "{Display}");
		}
		else if (EVT_DISP_CLOSE ==hdl->runStatus)
		{
            break;
		}
        else
        {
            AK_Sleep(1);
        }
		//IFrmStrm_Dump(hdl->strm);
	}
	Fwl_Print(C3, M_DISPLAY, "\n+++++Display task exit++++++++\n");
	
}

/*
 * @brief   destory display task 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32  VideoDisp_TaskDestory(T_VideoDisplayCtrl * pCtrl)
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

	
	if (!DISP_MUTEX_IS_ERR(pCtrl->mutex))
	{
		DISP_MUTEX_DEINIT(pCtrl->mutex);
	}
	return AK_SUCCESS;
}


/*
 * @brief   create video display task
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] isStart: is task is start immediately
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32   VideoDisp_TaskCreate(T_VideoDisplayCtrl * hdl, T_BOOL isStart)
{
	T_S8 *pName = "Display";
	T_SUBTHREAD_INITPARAM	param;
	
	if (AK_NULL == hdl)
	{
		return AK_EBADPARAM;
	}


  	DISP_MUTEX_INIT(hdl->mutex);

	if (DISP_MUTEX_IS_ERR(hdl->mutex))
	{
		Fwl_Print(C3, M_DISPLAY, "create mutex err\n");
		return AK_EFAILED;
	}
	

	sprintf(hdl->name, "%s", pName);

	param.byPriority	   = DISP_TASK_PRIO;
	param.ulTimeSlice	   = DISP_TASK_SLICE;
	param.ulStackSize	   = DISP_TASK_STACK;
	param.wMainThreadCls   = AKAPP_CLSID_MEDIA;
	
	param.pUserData 	   = (T_pVOID)hdl;
	param.fnEntry		   = VideoDisp_TaskHandle;
	param.fnAbort		   = AK_NULL; 
	param.pcszName		   = pName;
	if(AK_SUCCESS != CSubThread_New(&(hdl->trd), &param, isStart))
	{
		Fwl_Print(C3, M_DISPLAY, "Create %s_Thread Sub Thread Failure!\n",param.pcszName);
		return AK_EFAILED;
	}

	return AK_SUCCESS;
}


/*
 * @brief   get display buffer size for refresh  by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: display mode
 * @return	the refresh buffer size(byte)
 */
 #if 0
static T_U32 VideoDisp_GetDevFrmSize(DISPLAY_TYPE_DEV  TvOutMode)
{
    T_U32 width = 0, height = 0;

    Fwl_GetDispFrameRect(TvOutMode, &width, &height, AK_NULL);

    return width * height * 2;
}
#endif
/*
 * @brief   get display buffer max for refresh  by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: display mode
 * @return	the refresh buffer max size(byte)
 */
 #if 0
static T_U32 VideoDisp_GetDevFrmMaxSize(DISPLAY_TYPE_DEV  TvOutMode)
{
#ifdef DISP_TVOUT_USE_INNER_BUFF
    return VideoDisp_GetDevFrmSize(DISPLAY_LCD_0);;
#else
    return VideoDisp_GetDevFrmSize(DISPLAY_TVOUT_PAL);// use  max size for switch between lcd and  tvout
#endif
}
#endif

/*
 * @brief   get video display information by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: deviece refresh mode
 * @param[out] devInfo: display infomation
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_GetDevInfo(DISPLAY_TYPE_DEV  TvOutMode, T_FRM_INFO *devInfo)
{
    T_U32 Width2D, Height2D;    
    T_U32 WidthTV, HeightTV;
    T_U32 WidthTVFact, HeightTVFact;
    
	AK_ASSERT_PTR(devInfo, "VideoDisp_GetDevInfo: param err", AK_EBADPARAM);

    if (Fwl_GetDispFrameRect(TvOutMode, &devInfo->width, &devInfo->height, &devInfo->rect))
    {
 //     devInfo->type   = FORMAT_RGB565;
 //		devInfo->size   = devInfo->width*devInfo->height*2;
 		devInfo->type   = FORMAT_YUV420;
        devInfo->size   = devInfo->width*devInfo->height*3/2;
        
        if (TvOutMode >= DISPLAY_TVOUT_PAL)
        {
            Width2D  = PreView2DWin.width;
            Height2D = PreView2DWin.height;
            WidthTV  = devInfo->rect.width;
            HeightTV = devInfo->rect.height;
            WidthTVFact  = devInfo->rect.width + devInfo->rect.left*2;
            HeightTVFact = devInfo->rect.height + devInfo->rect.top*4;
    
            devInfo->rect.width = (T_U16)(Width2D*WidthTV/MAIN_LCD_WIDTH);
            devInfo->rect.height = (T_U16)(Height2D*HeightTV/MAIN_LCD_HEIGHT);
            
            if (0 != (devInfo->rect.width) % 2)
            {
                devInfo->rect.width = (T_U16)(devInfo->rect.width + 1);
            }            
            
            if (0 != (devInfo->rect.height) % 2)
            {
                devInfo->rect.height = (T_U16)(devInfo->rect.height + 1);
            }            
           
            devInfo->rect.left  = (T_U16)(WidthTVFact - devInfo->rect.width)/2;
            devInfo->rect.top   = (T_U16)(HeightTVFact - devInfo->rect.height)/2;

            Fwl_Print(C3, M_DISPLAY, "TVOUT DispSize[(%d,%d),%dx%d]\n",
                devInfo->rect.left,devInfo->rect.top,devInfo->rect.width,devInfo->rect.height);        
        }                
    }
    else
    {
        Fwl_Print(C3, M_DISPLAY, "VideoDisp_GetDevInfo Err @%d\n",__LINE__);
		return AK_EFAILED;
    }
	return AK_SUCCESS;
}


/*
 * @brief   update video refresh informatin by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @param[in] TvOutMode: display mode
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_UpdateRefreshMode(T_VideoDisplayCtrl * pCtrl, DISPLAY_TYPE_DEV  TvOutMode)
{
	AK_ASSERT_PTR(pCtrl, "VideoDisp_ZoomPostProc: param err", AK_EBADPARAM);

    DISP_SEM_DEINIT(pCtrl->dispHwSem);
	
#if 1	//目前不支持异步刷新，统一用DISP_REFR_MODE_PASSIVE
	pCtrl->curRefreshMode  =  DISP_REFR_MODE_PASSIVE;
#else
	if (TvOutMode >= DISPLAY_TVOUT_PAL)
	{
		pCtrl->curRefreshMode  =  DISP_REFR_MODE_ACTIVE;
	}

	else
	{
		if (E_LCD_TYPE_MPU == VideoDisp_LCD_Type())
		{
			pCtrl->curRefreshMode  =  DISP_REFR_MODE_PASSIVE;
		}
		else
		{
			pCtrl->curRefreshMode  =  DISP_REFR_MODE_ACTIVE;
		}
	}
#endif

    if (DISP_REFR_MODE_PASSIVE == pCtrl->curRefreshMode)
    {
        pCtrl->dispHwSemInit = 0;
    }

    else
    {
        pCtrl->dispHwSemInit = DISP_DEVIECE_CNT;
    }

    
    DISP_SEM_INIT(pCtrl->dispHwSem, pCtrl->dispHwSemInit);
    if (DISP_SEM_IS_ERR(pCtrl->dispHwSem))
    {
        return AK_EFAILED;
    }
    
    else
    {
        return AK_SUCCESS;
    }
}


/*
 * @brief   get video refresh zone by deviece refresh mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] TvOutMode: deviece refresh mode
 * @param[out] refreshRect: refresh zone
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_GetRefreshZone(DISPLAY_TYPE_DEV  TvOutMode, T_RECT *refreshZone)
{
    T_U32 width = 0, height = 0;
    
    AK_ASSERT_PTR(refreshZone, "GetRefreshZone: param err", AK_EBADPARAM);

    Fwl_GetDispFrameRect(TvOutMode, &width, &height, AK_NULL);

    refreshZone->left   = 0;
    refreshZone->top    = 0;
    refreshZone->width  = (T_LEN)width;
    
    if (TvOutMode >= DISPLAY_TVOUT_PAL)
    {
        refreshZone->height = (T_LEN)(height >> 1);
    }

    else
    {
        refreshZone->height = (T_LEN)height;
    }
	return AK_SUCCESS;
}

/*
 * @brief   start video display porcess
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in/out] srcCtlHdl: zoom handle
 * @param[in] tvOutMode:video srouce size	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Start(T_HANDLE hdl,T_HANDLE srcCtlHdl, DISPLAY_TYPE_DEV tvOutMode)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;
	T_S32      lRet  = AK_EFAILED;
	T_FRM_INFO    srcInfo = {0};
	
	AK_ASSERT_PTR(pCtrl, "VideoDisp_Start: param err", AK_EBADPARAM);
	if (0 == srcCtlHdl)
	{
        Fwl_Print(C3, M_DISPLAY, "Param Err ,zoom hdl invalid @%d\n",__LINE__);
		return AK_EFAILED;
	}

    lRet = VideoDisp_GetDevInfo(tvOutMode, &srcInfo);

	if (AK_IS_FAILURE(lRet))
	{
		 Fwl_Print(C3, M_DISPLAY, "VideoDisp_GetDevInfo Err @%d\n",__LINE__);
		 return AK_EFAILED;
	}
    // modify size  to max
    //srcInfo.size = VideoDisp_GetDevFrmMaxSize(tvOutMode);
    lRet = VideoZoom_OpenDispSrc(srcCtlHdl, &srcInfo);
	if (AK_IS_FAILURE(lRet))
	{
		 Fwl_Print(C3, M_DISPLAY, "Create Disp Strm Err @%d\n",__LINE__);
		 return AK_EFAILED;
	}
    

	pCtrl->curDispMode     = tvOutMode;
	pCtrl->SrcCtrl         = srcCtlHdl;
	VideoDisp_UpdateRefreshMode(pCtrl, tvOutMode);
	VideoZoom_SetDispOlderProc(pCtrl->SrcCtrl, VideoDisp_ZoomOrderProc, (T_HANDLE)pCtrl);
	VideoZoom_SetDispPostProc(pCtrl->SrcCtrl, VideoDisp_ZoomPostProc, (T_HANDLE)pCtrl);
	VideoZoom_SetDispPreProc(pCtrl->SrcCtrl, VideoDisp_ZoomPreProc, (T_HANDLE)pCtrl);
	VideoZoom_EnableDispSrc(pCtrl->SrcCtrl, AK_TRUE);
	
	pCtrl->curSysTime  = get_tick_count();
	VideoDisp_SetRefreshCbf(VideoDisp_RefreshEndCbf);
	VideoDisp_CtrlEvtSet(pCtrl,EVT_DISP_RUN);
	ISubThread_Resume(pCtrl->trd);

	return AK_SUCCESS;
}


/*
 * @brief   switch video refresh mode by  display mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] tvOutMode: display mode
 * @param[in] brightness: display brightness val
 * @return	resulst AK_SUCCESS--success, else fail
 */
/*T_S32 VideoDisp_SwitchTvOut(T_HANDLE hdl, DISPLAY_TYPE_DEV tvOutMode, T_U8 brightness)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;
	T_FRM_INFO    srcInfo = {0};
	T_S32      lRet  = AK_EFAILED;
	
	AK_ASSERT_PTR(pCtrl, "VideoDisp_SwitchTvOut: param err", AK_EBADPARAM);

    if (tvOutMode == pCtrl->curDispMode)
    {
		Fwl_Print(C3, M_DISPLAY, "Disp mode has been set\n");
		return AK_SUCCESS;
	}

    lRet = VideoDisp_GetDevInfo(tvOutMode, &srcInfo);
	if (AK_IS_FAILURE(lRet))
	{
		 Fwl_Print(C3, M_DISPLAY, "VideoDisp_GetDevInfo Err @%d\n",__LINE__);
		 return AK_EFAILED;
	}
    srcInfo.size = VideoDisp_GetDevFrmMaxSize(tvOutMode);

    //cancle deviece protect
    DISP_SEM_DEINIT(pCtrl->dispHwSem);

	DISP_MUTEX_LOCK(pCtrl->mutex);
	
    // update status to suspend
    VideoDisp_CtrlEvtUpdate(pCtrl, EVT_DISP_SUSPEND);

    // disable zoom display stream
	VideoZoom_EnableDispSrc(pCtrl->SrcCtrl, AK_FALSE);

    AK_Sleep(5);

    //switch deviece display type 
    Fwl_SetDisplayType(tvOutMode);  
    if (DISPLAY_LCD_0 == tvOutMode)
    {
        Fwl_LcdRotate(LCD_90_DEGREE);
    }
    Fwl_SetBrightness(DISPLAY_LCD_0, brightness);
#ifdef DISP_TVOUT_USE_INNER_BUFF
    lRet = VideoDisp_GetDevInfo(tvOutMode, &pCtrl->tvoutFrmData.info);
#else
    lRet = VideoZoom_SetDispInfo(pCtrl->SrcCtrl, &srcInfo);
#endif
    pCtrl->curDispMode     = tvOutMode;

    //rebuild deviece protect info
	if (AK_IS_SUCCESS(lRet))
	{
        lRet = VideoDisp_UpdateRefreshMode(pCtrl, tvOutMode);
	}

    AK_Sleep(8);
    
	if (AK_IS_SUCCESS(lRet))
	{
	    // enable zoom display stream
        VideoZoom_EnableDispSrc(pCtrl->SrcCtrl, AK_TRUE);

        // enable display process
        VideoDisp_CtrlEvtUpdate(pCtrl, EVT_DISP_RUN);
        Fwl_Print(C3, M_DISPLAY, "VideoZoom_SetDispInfo Ok\n");
	}
    else
    {
        Fwl_Print(C3, M_DISPLAY, "VideoZoom_SetDispInfo Err @%d\n",__LINE__);
    }
    
	DISP_MUTEX_UNLOCK(pCtrl->mutex);
	
	return lRet;
}*/

/*
 * @brief   set display lib refresh callback(when refresh one frame, this func will be called)
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pFuncCB: deviece refresh finished callback function
 */
static T_VOID VideoDisp_SetRefreshCbf(T_REFRESH_CALLBACK  pFuncCB)
{
    Fwl_Print(C3, M_DISPLAY, "VideoDisp_SetRefreshCbf %x beign\n", pFuncCB);
#ifdef OS_ANYKA
//    lcd_set_refresh_finish_callback(pFuncCB);
#endif
    Fwl_Print(C3, M_DISPLAY, "VideoDisp_SetRefreshCbf %x end\n", pFuncCB);
}


/*
 * @brief   close video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Close(T_HANDLE hdl)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_Close: param err", AK_EBADPARAM);

	if (NULL != pCtrl)
	{
	    DISP_SEM_DEINIT(pCtrl->dispHwSem);

	    DISP_MUTEX_LOCK(pCtrl->mutex);
		VideoZoom_CloseDispSrc(pCtrl->SrcCtrl);
		VideoDisp_CtrlEvtUpdate(pCtrl,EVT_DISP_CLOSE);
	    DISP_MUTEX_UNLOCK(pCtrl->mutex);

		VideoDisp_SetRefreshCbf(AK_NULL);
		VideoDisp_TaskDestory(pCtrl);
	    VideoDisp_RefreshDeInit(pCtrl);

		Fwl_Free(pCtrl);
	}
	
	Fwl_Print(C3, M_DISPLAY, "VideoDisp_Close Ok!\n");
	return AK_SUCCESS;
}


/*
 * @brief   pause video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Pause(T_HANDLE hdl)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_Pause: param err", AK_EBADPARAM);

    Fwl_Print(C3, M_DISPLAY, "Disp::Pause Begin.\n");

	if (NULL != pCtrl)
	{
	    DISP_SEM_DEINIT(pCtrl->dispHwSem);

	    VideoZoom_EnableDispSrc(pCtrl->SrcCtrl, AK_FALSE);

	    DISP_MUTEX_LOCK(pCtrl->mutex);

		VideoDisp_CtrlEvtUpdate(pCtrl,EVT_DISP_SUSPEND);

	    DISP_MUTEX_UNLOCK(pCtrl->mutex);
	}
	
    Fwl_Print(C3, M_DISPLAY, "Disp::Pause Ok\n");

	Fwl_TurnOff_YUV();

	return AK_SUCCESS;
}


/*
 * @brief   restart video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_Restart(T_HANDLE hdl)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_Restart: param err", AK_EBADPARAM);

    Fwl_Print(C3, M_DISPLAY, "Disp::Restart Begin.\n");
    DISP_MUTEX_LOCK(pCtrl->mutex);
    VideoDisp_UpdateRefreshMode(pCtrl, pCtrl->curDispMode);
	VideoDisp_CtrlEvtUpdate(pCtrl,EVT_DISP_RUN);
    VideoZoom_EnableDispSrc(pCtrl->SrcCtrl, AK_TRUE);

    DISP_MUTEX_UNLOCK(pCtrl->mutex);
    Fwl_Print(C3, M_DISPLAY, "Disp::Restart Ok\n");

	return AK_SUCCESS;
}

/*
 * @brief   DeInit video refresh informatin 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_RefreshDeInit(T_VideoDisplayCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Param err", AK_EBADPARAM);

    return AK_SUCCESS;
}


/*
 * @brief   Wait video deviece refresh finished
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @param[in] pFrame: frame for refresh
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_RefreshWait(T_VideoDisplayCtrl * pCtrl, T_FRM_DATA *pFrame)
{
    if (!DISP_SEM_IS_ERR(pCtrl->dispHwSem))
    {
        //DISP_LOG_DEBUG("WaitBegin:%x, %d\n", pFrame->pBuffer, get_tick_count());
        DISP_SEM_WAIT(pCtrl->dispHwSem);
        //DISP_LOG_DEBUG("WaitEnd:%x, %d\n", pFrame->pBuffer, get_tick_count());
    }

    return AK_SUCCESS;
}

/*
 * @brief   init video refresh informatin 
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] pCtrl: display handle
 * @return	resulst AK_SUCCESS--success, else fail
 */
static T_S32 VideoDisp_RefreshInit(T_VideoDisplayCtrl * pCtrl)
{
    AK_ASSERT_PTR(pCtrl, "Param err", AK_EBADPARAM);
    
    return AK_SUCCESS;
}


/*
 * @brief   open video display handle
 * @author WangXi
 * @date	2011-10-25
 * @return	resulst AK_NULL--error, else zoom handle
 */
T_HANDLE VideoDisp_Open(T_VOID)
{
	T_VideoDisplayCtrl      *hdl   = AK_NULL;
	T_S32                  lRet   = AK_EFAILED;


    hdl = Fwl_Malloc(sizeof(T_VideoDisplayCtrl));

	if (AK_NULL == hdl)
	{
        Fwl_Print(C3, M_DISPLAY, "Disp Malloc Err @%d\n",__LINE__);
		return 0;
	}

	memset(hdl, 0 , sizeof(T_VideoDisplayCtrl));
	
	hdl->runStatus = EVT_DISP_NULL;

    lRet = VideoDisp_RefreshInit(hdl);

    if (AK_IS_FAILURE(lRet))
    {
		Fwl_Print(C3, M_DISPLAY, "RefreshInit Err@%d\n",__LINE__);
		VideoDisp_Close((T_HANDLE)hdl);
		hdl = AK_NULL;
		return 0;
    }

	lRet = VideoDisp_TaskCreate(hdl, AK_FALSE);

	if (AK_IS_FAILURE(lRet))
	{
		Fwl_Print(C3, M_DISPLAY, "Task Create Err @%d\n",__LINE__);
		VideoDisp_Close((T_HANDLE)hdl);
		hdl = AK_NULL;
		return 0;
	}
    
	return (T_HANDLE)hdl;
}


/*
 * @brief   close video display handle
 * @author WangXi
 * @date	2011-10-25
 * @param[in/out] hdl: display handle
 * @param[in] videoUiCbf:paint icon on video frame  cbf	
 * @param[in] isPaintMenuCbf:is show pop menu cbf	
 * @param[in] menuPainCbf:paint pop menu cbf	
 * @return	resulst AK_SUCCESS--success, else fail
 */
T_S32 VideoDisp_SetPaintUiCbf(T_HANDLE hdl,VIDEO_DISP_VIDEO_UI_CBF videoUiCbf,
	VIDEO_DISP_PAINT_UI_CHECK_CBF isPaintMenuCbf,VIDEO_DISP_MENU_UI_CBF menuPainCbf)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)hdl;

	AK_ASSERT_PTR(pCtrl, "VideoDisp_SetPaintUiCbf: param err", AK_EBADPARAM);

	pCtrl->video_ui_cbf  = videoUiCbf;
	pCtrl->menu_ui_check = isPaintMenuCbf;
	pCtrl->menu_ui_cbf   = menuPainCbf;
	
	return	AK_SUCCESS;
}

/*
 * @brief   define display lib refresh callback(when refresh one frame, this func will be called)
 * @author WangXi
 * @date	2011-10-25
 * @param[in] addr: deviece refresh buffer address
 * @param[in] Param: when start refresh, user's parameter  tansfer for callback
 */
static T_VOID VideoDisp_RefreshEndCbf(T_U8* addr,T_U8 *Param)
{
	T_VideoDisplayCtrl *pCtrl = (T_VideoDisplayCtrl *)Param;
	
	if (AK_NULL == pCtrl)
	{
         return;
	}

    if (!DISP_SEM_IS_ERR(pCtrl->dispHwSem))
    {
        if (0 == pCtrl->dispHwSemInit)
        {
            if (0 == DISP_SEM_VAL(pCtrl->dispHwSem))
            {
                DISP_SEM_POST(pCtrl->dispHwSem);
            }
        }
        else
        {
            if (DISP_SEM_VAL(pCtrl->dispHwSem) < pCtrl->dispHwSemInit)
            {
            	//AK_Sleep(2);
                DISP_SEM_POST(pCtrl->dispHwSem);
            }
        }
        //Fwl_Print(C3, M_DISPLAY, "%x<<<\n", addr);
    }


#ifdef VIDEODISP_DEBUG_VER
    {
    	T_U32 curTime = 0;
        
    	pCtrl->curFps++;
    	curTime = get_tick_count();
    	if (curTime - pCtrl->curSysTime >= 1000)
    	{
    		Fwl_Print(C3, M_DISPLAY, "@%d@\n",pCtrl->curFps);
    		pCtrl->curSysTime = curTime;
    		pCtrl->curFps = 0;
    	}
    }
#endif
}

E_LCD_TYPE VideoDisp_LCD_Type(T_VOID)
{
#ifdef OS_ANYKA
	return lcd_get_type();
#else
	return 0;
#endif
}



