/**
 * @file: Ctl_RecAudio.h
 * @brief: provide function interfaces that control REC and gain REC info
 * 
 * @author: hoube
 * @date:  2012-3-3
 */


#ifndef  _CTL_RECAUDIO_H_
	#define _CTL_RECAUDIO_H_

#include "fwl_public.h"
#include "fwl_wave.h"
#include "gbl_macrodef.h"
#include "fwl_sysevent.h"


/* define record abort code */
typedef enum {
	eABORT_NULL = 0,
	eABORT_STORAGEMEDIUM_NOT_EXIST,
	eABORT_DISKSPACE_NOT_ENOUGH,
	eABORT_REACH_MAXLENCANREC,
	eABORT_REACH_MAXRECTIME,
	eABORT_WRITEFILE_ERROR,
	eABORT_NORMAL_STOPREC
} T_eABORT_CODE;

/* define all the REC state */
typedef enum {
	eSTAT_REC_STOP = 0,  	//record stoped
    eSTAT_RECORDING,    	//foreground recording
    eSTAT_RECORDING_BG,		//background recording
    eSTAT_UNKNOWN
}T_eREC_STAT;

/* define record action */
typedef enum {
	eREC_ACTION_ONCE = 0,
	eREC_ACTION_LOOP,
	eREC_ACTION_UNKNOWN
} T_eREC_ACTION;

/* define last-error code value that record control functions set */
typedef enum {
	///
	eERR_NONE_ERROR						=0x00000000,
	eERR_EVENTGROUP_CREATEFAILED		=0x00000001,
	eERR_EVENTGROUP_TIMEOUT				=0x00000002,
	///Ctl_RecAudio_Init
	eERR_CTL_XXX_INIT_BEGIN				=0x00010000,
	eERR_DEFRECDIR_CREATE_FAILED,
	eERR_ASYNBUFFER_INIT_FAILED,
	eERR_MEDIAENCODER_INIT_FAILED,
	eERR_CTL_XXX_INIT_END,
	///Ctl_RecAudio_Start
	eERR_CTL_XXX_START_BEGIN			=0x00020000,
	eERR_NOT_STOPSTATE,
	eERR_DISKSPACE_NOTENOUGH,
	eERR_FILENAME_MAKEFAILED,
	eERR_RECFILE_CREATEFAILED,
	eERR_MEDIAENCODER_STARTUP_FAILED,
	eERR_RECTIMER_CREATEFAILED,
	eERR_CTL_XXX_START_END,
	///Ctl_RecAudio_Stop
	eERR_CTL_XXX_STOP_BEGIN				=0x00030000,
	eERR_ALREADY_IS_STOPSTATE,
	eERR_MEDIAENCODER_STOP_FAILED,
	eERR_CTL_XXX_STOP_END,
	///Ctl_RecAudio_Destroy
	eERR_CTL_XXX_DESTROY_BEGIN			=0x00040000,
	eERR_ASYNBUFFER_DELETE_FAILED,
	eERR_MEDIAENCODER_DESTROY_FAILED,
	eERR_CTL_XXX_DESTROY_END,
	///other
	
	eERR_UNKNOWN_ERROR
} T_eLAST_ERROR;


/**
 * @brief:
 * @note : call this func before all other record func is called,and call only once before record. 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eREC_MODE rec_mode , specifies format of record file
 * @param: T_U16 rec_rate , specifies record sample rate
 * @param: T_eINPUT_SOURCE , specifies device object that to recording , only support MIC and LINEIN at present
 * @param: T_U32 rec_flag, reserved.
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Init(T_eREC_MODE mode, T_U32 sample_rate, T_eINPUT_SOURCE src, T_U32 flag);

/**
 * @brief: start record
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_USTR_FILE filename. Pointer to the buffer for the record file name string that not include path. 
 *               If filename is AK_NULL or action is eREC_ACTION_LOOP,will use inner file naming format.
 * @param: T_eREC_ACTION rec_action , specifies the action that record:eREC_ACTION_ONCE or eREC_ACTION_LOOP
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Start(T_USTR_FILE filename, T_eREC_ACTION action);

/**
 * @brief: stop record
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_BOOL bSaveRecFile , decide save record file whether or not after stop record
 * @param: T_eREC_ACTION rec_action , specifies the action that record:eREC_ACTION_ONCE or eREC_ACTION_LOOP
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Stop(T_BOOL bSaveRecFile);

/**
 * @brief: release record resource
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Destroy(T_VOID);

/**
 * @brief: handle the record START/STOP/REC event, read/filter/encode/save the PCM data, and monitor record exception
 * @note: 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_SYS_EVTID eEvent
 * @param: T_SYS_PARAM *pEvtParam
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_HandleMsg(T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);


/********************************************************************/
/********************************************************************/

/**
 * @brief: get last sample data
 * @note :  
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_S16 sample[2]
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_GetCurSample(T_S16 sample[2]);

/**
 * @brief: get current record state
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_S32
 * @retval: 
 */
T_eREC_STAT Ctl_RecAudio_GetCurState(T_VOID);

/**
 * @brief: get last error code
 * @note: 1.You should call this function immediately when a function's return value indicates that 
 *            such a call will return useful data. 
 *            That is because some functions will set lasterror to 0 when them is called, wiping out the error code 
 *            set by the recently failed function. 
 *            2.at present,functions list that can wiping out the error code,as follows:
 *                  Ctl_RecAudio_Init
 *			Ctl_RecAudio_Start
 *			Ctl_RecAudio_Stop
 *			Ctl_RecAudio_Destroy
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_eLAST_ERROR
 * @retval: 
 */
T_eLAST_ERROR Ctl_RecAudio_GetLastError(T_VOID);

/**
 * @brief: get current record time
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval: 
 */
T_U32 Ctl_RecAudio_GetCurRecTime(T_VOID);

/**
 * @brief: get current recorded data length
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval 
 */
T_U32 Ctl_RecAudio_GetCurRecedLen(T_VOID);

/**
 * @brief: get current max length of can record, it is real time.
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval : value is 0~RECFILE_MAXLEN
 */
T_U32 Ctl_RecAudio_GetMaxLenCanRec(T_VOID);

/**
 * @brief: see about whether medium of placed record file exist or not.
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_IsExistMedium(T_VOID);

/**
 * @brief:  
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: bToBgRec , AK_TRUE--to background record; AK_FALSE--to foreground record.
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.
 */
T_BOOL Ctl_RecAudio_ToBgRec(T_BOOL bToBgRec);

/**
 * @brief: 
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eINPUT_SOURCE input_src
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_IsBgRec(T_eINPUT_SOURCE input_src);

/**
 * @brief: obtain record stop state
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: if record is stoped already,return AK_TRUE; if is other state, return AK_FALSE.
 */
T_BOOL Ctl_RecAudio_IsStoped(T_VOID);


#endif  // _CTL_RECAUDIO_H_

