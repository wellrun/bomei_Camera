
#ifndef __CTL_VIDEORECORD_H__
#define __CTL_VIDEORECORD_H__

#include "Ctl_AVIPlayer.h"
#include "log_media_recorder.h"
#include "Fwl_osfs.h"
#include "Eng_GblString.h"
#include "ctl_msgbox.h"
#include "ctl_preview.h"
#include "lib_image_api.h"

#ifdef CAMERA_SUPPORT

/**
 * Define video recorder refresh control
 */
#define CAMERA_RECORD_REFRESH_ALL       (0xffff)
#define CAMERA_RECORD_REFRESH_NONE      (0x0)
#define CAMERA_RECORD_REFRESH_BACK      (0x1<<1)
#define CAMERA_RECORD_SHOW_TOOLBAR      (0x1<<3)
#define CAMERA_RECORD_UNSHOW_TOOLBAR    (0x1<<4)
#define CAMERA_RECORD_SHOW_MSGBOX       (0x1<<5)
#define CAMERA_RECORD_UNSHOW_MSGBOX     (0x1<<6)



typedef struct{
    T_MSGBOX            msgbox;         // msgbox handle
	T_VIDEO_REC_UI		paintData;      // ui resource dada info
    T_USTR_FILE         curPath;        //current record file path
    T_USTR_FILE         fileName;       //current record file name
    T_U32               videoSize;      //record data size include index
    T_U32               recTime;        //record time
    T_U32               recMaxTime;        //record time
    PAINTINFO           *pPaintParm;    // record paint parameter
    volatile T_U16      refreshFlag;    // record paint flag for ui
    T_U32               width;          // video record width
    T_U32               height;         // video record height
    T_U32               videoSrcWidth;  // video src width
    T_U32               videoSrcHeight; // video src widht

    //the following elements is added for dealing the event of touch screen 
    T_BOOL              toolBarShowFlag; // toolbar paint flag, AK_TRUE:paint, else not paint
    T_TIMER             toolBarTimerID;  // refresh toolbar timer


	T_HANDLE			hMRec; // vidoe record handle
	T_HANDLE			hZoom; // video zoom handle
	T_HANDLE			hDisp; // video asyn display handle

	T_BOOL				isExitWarning;

	J_OSD_Info			pJ_OSD_Info;
    J_OSD_Info          pJ_OSD_Info_Zoom;
    T_U16 		        strOSDBuf[30];
	T_TIMER             OSDTimerID;    
}T_VIDEO_RECORD;

/**
 * @brief The function paint
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec      the point of video record control
 * @ret T_U16
 */
T_U16 VideoRecord_Paint(T_VIDEO_RECORD *pVideoRec);

/**
 * @brief The function paint
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec      the point of video record control
 * @ret T_U16
 */
T_BOOL VideoRecord_Init(T_VIDEO_RECORD *pVideoRec);

/*
 * @brief   init paint resource and parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: init success, else fail
 */
T_BOOL VideoRecord_PaintInit(T_VIDEO_RECORD *pVideoRec, T_VIDEO_REC_UI *pPaintData);

/**
 * @brief The function start recorder
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 * @return T_REC_ERROR_STATUS 
 * @retval  REC_ERROR_OK : success, else fail.
 */
T_REC_ERROR_STATUS VideoRecord_StartRecord(T_VIDEO_RECORD *pVideoRec);

/*
 * @brief  close record
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] isSave       if save record file
 * @param[in] strID         msgbox tips string
 */
T_VOID VideoRecord_Close(T_VIDEO_RECORD* pVideoRec, T_BOOL isSave,T_eCSTM_STR_ID strID);

/**
 * @brief The function processes event
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 * @param[in] Event            event code
 * @param[in] pEventParm        the parameter of event
 */
T_eBACK_STATE VideoRecord_Handle(T_VIDEO_RECORD* pVideoRec, T_EVT_CODE Event, T_EVT_PARAM *pEventParm);

/**
 * @brief The function processes Video record control free
 *
 * @author \b zhengwenbo
 * @date 2006-07-10
 * @param[in] pVideoRec      the point of video record control
 */
T_VOID VideoRecord_Free(T_VIDEO_RECORD *pVideoRec);

/**
 * @brief The function set refresh flag
 *
 * @author \b zhengwenbo
 * @date 2006-07-12
 * @param[in] pVideoRec   the point of record control
 * @param[in] flag  refresh flag
 */
T_VOID VideoRecord_SetRefresh(T_VIDEO_RECORD *pVideoRec, T_U16 flag);

/*
 * @brief   switch refresh  by diplay mode
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      the point of video record control
 * @param[in] tvOutMode     switch mode
 * @return	AK_TRUE:  switch  success, else fail
 */
//T_BOOL VideoRecord_SwitchTvOut(T_VIDEO_RECORD *pVideoRec,DISPLAY_TYPE_DEV tvOutMode);

/*
 * @brief   display error pop msgbox
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec      video record control  handle
 * @param[in] tId:  title title id
 * @param[in] sId:  title string id
 * @param[in] retLevel:  return level
 * @param[in] useInnerDisp: if use  asyn display
 */
T_VOID VideoRecord_DispErrMsgbox(T_VIDEO_RECORD *pVideoRec, T_S16 tId, T_S16 sId, T_U16 retLevel, T_BOOL useInnerDisp);

/*
 * @brief   preload paint resource
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pPaintData: paint data handle
 * @return	AK_TRUE: load success, else fail
 */
T_BOOL VideoRecord_IconPreLoad(T_VIDEO_REC_UI *pPaint);


/*
 * @brief   init video souce process task
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: init success, else fail
 */
T_BOOL VideoRecord_SrcInit(T_VIDEO_RECORD *pVideoRec);

/*
 * @brief   deinit video souce process task
 * @author WangXi
 * @date	2011-10-25
 * @param[in] pVideoRec: VideoRec handle
 * @return	AK_TRUE: deinit success, else fail
 */
T_BOOL VideoRecord_SrcDeInit(T_VIDEO_RECORD *pVideoRec);

/*
 * @brief    get chip is 04 or not
 * @author 
 * @date    2012-03-08
 * @param[in]  T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE:is 04, else 02
 */
T_BOOL Is_04CHIP(T_VOID);

T_S32 OSD_Obtain_Semaphore(T_VOID);
T_S32 OSD_Release_Semaphore(T_VOID);


#endif
#endif // #ifndef __CTL_VIDEORECORD_H__

