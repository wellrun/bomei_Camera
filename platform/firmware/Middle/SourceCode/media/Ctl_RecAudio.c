/**
  * @file: Ctl_RecAudio.c
  * @brief: provide function interfaces that control REC and get REC info
  * 
  * @author: hoube
  * @date:  2012-3-3
 */

#include "Fwl_osMalloc.h"
#include "Fwl_osCom.h"
#include "Lib_event.h"
#include "Eng_String_UC.h"
#include "lib_sdfilter.h"
#include "fwl_wavein.h"
#include "eng_string.h"
#include "akos_api.h"
#include "string.h"
#include "eng_debug.h"
#include "Log_RecAudio.h"
#include "Log_MediaEncode.h"

#include "Fwl_Initialize.h"
#include "Eng_Math.h"
#include "Ctl_RecAudio.h"
#include "fwl_public.h"

#include "Ctl_MsgBox.h"
#include "AKAppMgr.h"
#include "Eng_ScreenSave.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_MsgQueue.h"


#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))

/* wave header structure */
typedef struct {
    T_CHR  riff[4];         // "RIFF"
    T_S32  file_size;       // in bytes
    T_CHR  wavefmt_[8];     // "WAVE"
    T_S32  chunk_size;      // in bytes (16 for PCM)
    T_S16  format_tag;      // 1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law
    T_S16  num_chans;       // 1=mono, 2=stereo
    T_S32  sample_rate;
    T_S32  bytes_per_sec;
    T_S16  bytes_per_samp;  // 2=16-bit mono, 4=16-bit stereo
    T_S16  bits_per_samp;
    T_CHR  data[4];         // "data"
    T_S32  data_length;     // in bytes
} T_WAVE_HEADER;


/* adpcm header structure */
typedef struct {
    T_S16  wFormatTag;		// wave_format_dvi_adpcm / wave_format_ima_adpcm (0x0011)
    T_S16  nChannels;  		//channel 
    T_S32  nSamplesPerSec;  // samplerate
    T_S32  nAvgBytesPerSec; // 每秒多少个字节, 4055 = 256*8000/505  
    T_S16  nBlockAlign;    	// 数据块的调整数, 取决于每个采样点的位数 - 256 
    T_S16  wBitsPerSample; 	// 每样本数据位数 - 4  
    T_S16  cbSize;         	// 保留参数- 2 
    T_S16  wSamplesPerBlock;// 每个数据块包含采样点数, 505
} T_IMA_ADPCM_WAVEFORMAT;

typedef struct {
    T_CHR  riff[4];        	// "RIFF"
    T_S32  file_size;       // in bytes
    T_CHR  wavefmt_[8];    	// "WAVE"
    T_S32  imachunk_size; 	// in bytes (0x14 for IMA ADPCM)
    T_IMA_ADPCM_WAVEFORMAT ima_format;
    T_CHR  fact[4];         //"fact"
    T_S32  factchunk_size;  //fact chunk size
    T_S32  factdata_size;  	//fact data size
    T_CHR  data[4];         // "data"
    T_S32  data_length;     // in bytes
} T_ADPCM_HEADER;


/*
 * default setting of wave header for recording
 * at sample rate of 8000, mono, 16bits, in PCM format
 */
#ifdef  OS_ANYKA
T_WAVE_HEADER wh = {
	{'R','I','F','F'}, 		    		//"RIFF"
	0,
	{'W','A','V','E','f','m','t',' '},  //"WAVEfmt "
	0x10,                       		//in bytes (16 for PCM)
	0x1,                        		//1=PCM, 2=ADPCM, 3=IEEE float, 6=A-Law, 7=Mu-Law
	1,                          		//1=mono, 2=stereo
	8000,                       		//default 8k sample rate
	0,
	2,                          		//bits_per_samp / 8 * num_chans
	16,                         		//default 16bits
	{'d','a','t','a'},          		//"data"
	0
};

T_ADPCM_HEADER adpcmh = {
	{'R','I','F','F'}, 		    		//"RIFF"
	0,                          		//file size
	{'W','A','V','E','f','m','t',' '},  //"WAVEfmt "
	0x14,                       		//IMAADPCMWAVEFORMAT struct size(0x14 for IMA ADPCM)
	{0x11,                      		//IMA adpcm format
	1,                          		//channel 1=mono, 2=stereo
	8000,                       		//default 8k sample rate
	0,                          		//bytes_per_sec
	2,                          		//block align
	16,                         		//bits per sample
	2,                          		//reserve bit
	505},                       		//samples per packet
	{'f','a','c','t'},          		//"fact"
	4,                          		//fact chuck size
	0,                          		//the number of samples
	{'d','a','t','a'},          		//"data"
	0                           		//data len
};
#endif

static T_U8 ANYKA_ID[] 	 = {0x00,0x41,0x4e,0x59,0x4b,0x41,0x0d,0x0a};// For not DTX Mode
static T_U8 AMR_HEADER[] = {0x23,0x21,0x41,0x4d,0x52,0x0A};


#define RECFILE_MAX_NUM					999999
#define LOOP_DEPTH						1024


#define AUDIO_ENCODE_INTERVAL           20      	// the interval of read-filter-encoded PCM data, units is millisecond
#define RECFILEHEAD_UPDATE_INTERVAL		0x0f		// the interval of update record file header, units is second

#define RECAUDIO_ASYNWRITE_BUFSIZE		(64*3*1024) // 192KB

#define RECFILE_MAXLEN      			(3.9*1024*1024*1024)
#define RECTIME_MAXVALUE      			(10*3600)	// the max record time, in seconds

#define EVENT_START_REC					(1<<0)
#define EVENT_STOP_REC					(1<<1)

/* define the extension name */
#define AMR_FILE_EXT        			"amr"
#define WAV_FILE_EXT        			"wav"
#define MP3_FILE_EXT       				"mp3"
#define AAC_FILE_EXT        			"aac"


typedef struct {
    T_MEncoder *	pMEncoder;  	//media encoder structure handle

	T_TIMER     	recTimerID;		//REC timer identification
	
    T_U32       	curRecTime;  	//current record time already(seconds)
    T_U32			curRecedLen;	//current record data length already(byte)
    T_U32       	maxLenCanRec;	//max length that can record(byte)

	T_eREC_STAT 	curState;		//current record state 
    T_eLAST_ERROR	lastError;		//last-error code value, that control functions set
	
    T_eINPUT_SOURCE inputSrc;
    T_eREC_MODE		recMode;
	T_U32			recRate;
		
    T_hFILE 		rec_fd;         //record file handle
    T_WSTR_FILEPATH recDir;			//hold the record directory, prevent appear error when default record path is changed.
    T_USTR_FILE     recFile;    	//full path of record file

	T_U32			recFlag;		//reserved
	T_eREC_ACTION	recAction;		//record action : record one time or record loop

	T_hEventGroup	hEvent;
}T_RECAUDIO_CONTEXT;

static T_RECAUDIO_CONTEXT *pRecAudio_Context = AK_NULL;

static T_U32  GetMaxLenCanRec(T_VOID);
static T_VOID RecUpdateHeader(T_VOID);
static T_BOOL MakeRecFileName(T_pWSTR recFile ,T_pSTR preName);

static T_VOID SetCurState(T_eREC_STAT state);
static T_VOID SetLastError(T_eLAST_ERROR err);
static T_VOID StartRecord(T_eREC_ACTION action);
static T_VOID StopRecord_Step1(T_VOID);
static T_VOID StopRecord_Step2(T_BOOL bSaveRecFile);
static T_VOID Exit_ScreenSave(T_VOID);
static T_VOID RECAbort_ShowInfo(T_eABORT_CODE abort_code);
static T_VOID RECAbort_ShowInfoNoSendMsg(T_eABORT_CODE abort_code);

static T_BOOL CreateRecFile(T_VOID);
static T_VOID RecordTimerCB(T_TIMER timer_id, T_U32 delay);


/**
 * @brief: get maximal record data length that can record.
 * @note: if disk space is less that AUDIOREC_MINIMAL_SPACE,return 0; max return value is RECFILE_MAXLEN.
 *			if (freesize   ≤AUDIOREC_MINIMAL_SPACE)
 *				MaxLenCanRec = 0;
 *			else
 *				curRecedLen≤(MaxLenCanRec = freesize+curRecedLen)≤RECFILE_MAXLEN;
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval: 0 ~ RECFILE_MAXLEN
 */
static T_U32 GetMaxLenCanRec(T_VOID)
{
	T_U64_INT freesize;

	Fwl_FsGetFreeSize(pRecAudio_Context->recDir[0], &freesize);
	
	if (U64cmpU32(&freesize, AUDIOREC_MINIMAL_SPACE) <= 0)
	{
		return 0;
	}

	U64subU32(&freesize, AUDIOREC_MINIMAL_SPACE);	//the value of size be changed!

	U64addU32(&freesize, 0, pRecAudio_Context->curRecedLen);

	if (1 == U64cmpU32(&freesize, (T_U32)RECFILE_MAXLEN))
	{
		return (T_U32)RECFILE_MAXLEN;
	}
	else
	{
		return freesize.low;
	}
}


/**
 * @brief: update record file header,and flush record file
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_VOID
 * @retval:
 */
static T_VOID RecUpdateHeader(T_VOID)
{
#ifdef OS_ANYKA

	T_U32 oldFilePtr = 0;
	
	if (FS_INVALID_HANDLE != pRecAudio_Context->rec_fd)
	{
		Fwl_Print(C3, M_REC_A, "$update head");
		
		oldFilePtr = Fwl_FileTell(pRecAudio_Context->rec_fd);
		
		switch (pRecAudio_Context->recMode)
		{
		case eRECORD_MODE_AMR:
			Fwl_FileWrite(pRecAudio_Context->rec_fd,ANYKA_ID,8);	 // For not DTX Mode
			break;
			
		case eRECORD_MODE_WAV:
			Fwl_FileSeekEx(pRecAudio_Context->rec_fd, 0, _FSEEK_SET);
			wh.file_size = pRecAudio_Context->curRecedLen + sizeof(wh) - 8;
			wh.data_length = pRecAudio_Context->curRecedLen;
			wh.sample_rate = pRecAudio_Context->recRate;
			Fwl_FileWrite(pRecAudio_Context->rec_fd, &wh, sizeof(wh));
			break;
			
		case eRECORD_MODE_ADPCM_IMA:
			Fwl_FileSeekEx(pRecAudio_Context->rec_fd, 0, _FSEEK_SET);
			adpcmh.ima_format.nAvgBytesPerSec = pRecAudio_Context->pMEncoder->audEndOutInfo.m_Private.m_adpcm.nAvgBytesPerSec;
			adpcmh.ima_format.nBlockAlign = pRecAudio_Context->pMEncoder->audEndOutInfo.m_Private.m_adpcm.nBlockAlign;
			adpcmh.ima_format.nSamplesPerSec = pRecAudio_Context->pMEncoder->audEndOutInfo.nSamplesPerSec;
			adpcmh.ima_format.wBitsPerSample = pRecAudio_Context->pMEncoder->audEndOutInfo.m_Private.m_adpcm.wBitsPerSample;
			adpcmh.ima_format.wSamplesPerBlock = pRecAudio_Context->pMEncoder->audEndOutInfo.m_Private.m_adpcm.nSamplesPerPacket;
			adpcmh.file_size = pRecAudio_Context->curRecedLen + sizeof(adpcmh) - 8;
			adpcmh.factdata_size = pRecAudio_Context->curRecTime*adpcmh.ima_format.nSamplesPerSec;
			adpcmh.data_length = pRecAudio_Context->curRecedLen;
			adpcmh.ima_format.nSamplesPerSec = pRecAudio_Context->recRate;
			Fwl_FileWrite(pRecAudio_Context->rec_fd, &adpcmh, sizeof(adpcmh));
			break;
			
		case eRECORD_MODE_MP3:
		case eRECORD_MODE_AAC:
			break;
			
		default:
			Fwl_Print(C3, M_REC_A, "@unknown record mode");
			return;
		}
		
	Fwl_FileDataFlush(pRecAudio_Context->rec_fd);
	Fwl_FileSeekEx(pRecAudio_Context->rec_fd, oldFilePtr, _FSEEK_SET);
	}

#endif
}


/**
 * @brief: produce record file main name
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_pWSTR recFile, pointer to the buffer for the full name of record file
 * @param: T_pSTR preName, specify prefix that record file main name
 * @return: T_BOOL
 * @retval: If make successal, return AK_TRUE; or, return AK_FALSE.
 */ 
static T_BOOL MakeRecFileName(T_pWSTR recFile ,T_pSTR preName)
{
	T_STR_INFO  suffixName;

#ifndef TIME_STAMP_FILE_NAME

    T_USTR_FILE name, path;
    T_U32       i = 0;    
    T_STR_INFO  tmpstr;
    T_pFILE fp;

#endif

	switch (pRecAudio_Context->recMode)
	{
	case eRECORD_MODE_AMR:
		sprintf(suffixName, AMR_FILE_EXT);
		break;

	case eRECORD_MODE_MP3:
		sprintf(suffixName, MP3_FILE_EXT);
		break;

	case eRECORD_MODE_AAC:
		sprintf(suffixName, AAC_FILE_EXT);
		break;

	default:
		sprintf(suffixName, WAV_FILE_EXT);
		break;
	}

#ifdef TIME_STAMP_FILE_NAME

	return MRec_GetStampFileName(pRecAudio_Context->recDir, preName, suffixName, recFile);
#else

    /**make directory*/
    Fwl_FsMkDirTree(pRecAudio_Context->recDir);

    gs.AudioRecFileNum++;
	
    while (AK_TRUE)
    {
        if (RECFILE_MAX_NUM < gs.AudioRecFileNum)
        {
            gs.AudioRecFileNum = 1;
        }

        sprintf(tmpstr, "RECORD_%06ld.%s", gs.AudioRecFileNum, suffixName);
        
        Eng_StrMbcs2Ucs(tmpstr, name);

        if (LOOP_DEPTH < i )
        {
            return AK_FALSE;
        }

        Utl_UStrCpy(path, pRecAudio_Context->recDir);
        Utl_UStrCat(path, name);
		
        if (_FOPEN_FAIL != (fp = Fwl_FileOpen(path, _FMODE_READ, _FMODE_READ)))
        {
            Fwl_FileClose(fp);
        }
        else
        {
            break;
        }
		
        gs.AudioRecFileNum++;
        i++;
    }
    
    Utl_UStrCpy(recFile, path);    

    Fwl_Print(C3, M_REC_A, "@new record file name is %s\n", recFile);
    
    return AK_TRUE;
	
#endif
}


/**
 * @brief: create new record file, AsynWrite mode
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: If create successal, return AK_TRUE; or, return AK_FALSE.
 */
static T_BOOL CreateRecFile(T_VOID)
{
#ifdef OS_ANYKA

	pRecAudio_Context->rec_fd = Fwl_FileOpenAsyn(pRecAudio_Context->recFile, _FMODE_CREATE, _FMODE_CREATE);

	if (FS_INVALID_HANDLE == pRecAudio_Context->rec_fd)
	{
		Fwl_Print(C2, M_REC_A, "++create the wave file failed:%s\n", pRecAudio_Context->recFile);
		return AK_FALSE;
	}
		
	switch (pRecAudio_Context->recMode)
	{
	case eRECORD_MODE_AMR:
		Fwl_FileWrite(pRecAudio_Context->rec_fd, &AMR_HEADER,6);
		break;
		
	case eRECORD_MODE_WAV:
		wh.sample_rate = pRecAudio_Context->recRate;
		wh.bytes_per_sec = wh.bytes_per_samp * wh.sample_rate;
		Fwl_FileWrite(pRecAudio_Context->rec_fd, &wh, sizeof(wh));
		break;
		
	case eRECORD_MODE_ADPCM_IMA:
		adpcmh.ima_format.nSamplesPerSec = pRecAudio_Context->recRate;
		Fwl_FileWrite(pRecAudio_Context->rec_fd, &adpcmh, sizeof(adpcmh));
		break;

	case eRECORD_MODE_MP3:
	case eRECORD_MODE_AAC:
		break;
		
	default:
		Fwl_Print(C2, M_REC_A, "+unknown record mode");
		return AK_FALSE;
	}

	RecUpdateHeader();

	return AK_TRUE;
#endif

	return AK_FALSE;
}


/**
 * @brief: callback function of record timer
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_TIMER timer_id
 * @param: T_U32 delay
 * @return: T_VOID
 * @retval: 
 */
static T_VOID RecordTimerCB(T_TIMER timer_id, T_U32 delay)
{
    T_SYS_MAILBOX mbox;

    mbox.event = EVT_RECAUDIO_REC;
	
	IAppMgr_PostUniqueEvt(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mbox);
}


/**
 * @brief: show abort info
 * @note: inner function
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eABORT_CODE abort_code
 * @return: T_VOID
 * @retval:
 */
static T_VOID RECAbort_ShowInfo(T_eABORT_CODE abort_code)
{
	T_USTR_FILE ustrMsgInfo;
    T_SYS_MAILBOX mailbox;
	
	mailbox.event = M_EVT_Z05COM_MSG;
    mailbox.param.lParam = (T_U32)AK_NULL;	
	
	Fwl_Print(C3, M_REC_A, "@RECAbort_ShowInfo:%d", abort_code);

	switch (abort_code)
	{
	case eABORT_WRITEFILE_ERROR:
		Utl_UStrCpy(ustrMsgInfo, GetCustomString(csAUDIOREC_RECORD_FAILURE));
		Utl_UStrCat(ustrMsgInfo, _T("\n"));
		Utl_UStrCat(ustrMsgInfo, Res_GetStringByID(eRES_STR_AUDIOREC_WRITEFILE_ERROR));
		MsgQu_Push(GetCustomTitle(ctWARNING), ustrMsgInfo, MSGBOX_INFORMATION, MSGBOX_DELAY_1);
	break;

	case eABORT_STORAGEMEDIUM_NOT_EXIST:
		Utl_UStrCpy(ustrMsgInfo, GetCustomString(csAUDIOREC_RECORD_FAILURE));
		Utl_UStrCat(ustrMsgInfo, _T("\n"));
		Utl_UStrCat(ustrMsgInfo, Res_GetStringByID(eRES_STR_AUDIOREC_STORAGEMEDIUM_NOTEXIST));
		MsgQu_Push(GetCustomTitle(ctWARNING), ustrMsgInfo, MSGBOX_INFORMATION, MSGBOX_DELAY_1);
	break;

	case eABORT_DISKSPACE_NOT_ENOUGH:
		Utl_UStrCpy(ustrMsgInfo, GetCustomString(csSAVESPACEFULL));
		Utl_UStrCat(ustrMsgInfo, pRecAudio_Context->recFile);
		MsgQu_Push(GetCustomTitle(ctWARNING), ustrMsgInfo, MSGBOX_INFORMATION, MSGBOX_DELAY_1);
	break;

	case eABORT_NORMAL_STOPREC:
	case eABORT_REACH_MAXRECTIME:
	case eABORT_REACH_MAXLENCANREC:
		Utl_UStrCpy(ustrMsgInfo, GetCustomString(csAUDIOREC_SAVE_OK));
		Utl_UStrCat(ustrMsgInfo, _T("\n"));
		Utl_UStrCat(ustrMsgInfo, pRecAudio_Context->recFile);
		MsgQu_Push(GetCustomTitle(ctWARNING), ustrMsgInfo, MSGBOX_INFORMATION, MSGBOX_DELAY_1);
	break;
	
	default:

	break;
	}
	
	AK_PostEvent(&mailbox);
}




/**
 * @brief: show abort info 
 * @note: inner function
 * @author: hoube
 * @date: 2012-4-20
 * @param: T_eABORT_CODE abort_code
 * @return: T_VOID
 * @retval:
 */
static T_VOID RECAbort_ShowInfoNoSendMsg(T_eABORT_CODE abort_code)
{
	T_MSGBOX msgbox;
	T_RECT   msgbox_rect;
	T_pDATA  tmp_lcd_buffer;
		
	Fwl_Print(C3, M_REC_A, "@RECAbort_ShowInfo:%d", abort_code);

	switch (abort_code)
	{
	case eABORT_WRITEFILE_ERROR:
		MsgBox_InitStr(&msgbox, 0, AK_NULL, GetCustomString(csAUDIOREC_RECORD_FAILURE), MSGBOX_INFORMATION);
		MsgBox_AddLine(&msgbox, Res_GetStringByID(eRES_STR_AUDIOREC_WRITEFILE_ERROR));
	break;

	case eABORT_STORAGEMEDIUM_NOT_EXIST:
		MsgBox_InitStr(&msgbox, 0, AK_NULL, GetCustomString(csAUDIOREC_RECORD_FAILURE), MSGBOX_INFORMATION);
		MsgBox_AddLine(&msgbox, Res_GetStringByID(eRES_STR_AUDIOREC_STORAGEMEDIUM_NOTEXIST));
	break;

	case eABORT_DISKSPACE_NOT_ENOUGH:
		MsgBox_InitStr(&msgbox, 0, AK_NULL, GetCustomString(csSAVESPACEFULL), MSGBOX_INFORMATION);
		MsgBox_AddLine(&msgbox, pRecAudio_Context->recFile);
	break;

	case eABORT_NORMAL_STOPREC:
	case eABORT_REACH_MAXRECTIME:
	case eABORT_REACH_MAXLENCANREC:
		MsgBox_InitStr(&msgbox, 0, AK_NULL, GetCustomString(csAUDIOREC_SAVE_OK), MSGBOX_INFORMATION);
		MsgBox_AddLine(&msgbox, pRecAudio_Context->recFile);
	break;
	
	default:

	break;
	}

	MsgBox_GetRect(&msgbox, &msgbox_rect);
	tmp_lcd_buffer = Fwl_SaveDisplayRegion(msgbox_rect);
	MsgBox_SetRefresh(&msgbox, CTL_REFRESH_ALL);
	MsgBox_Show(&msgbox);
	Fwl_RefreshDisplay();
	Fwl_MiniDelay(1500);
	
	Fwl_RestoreDisplayRegion(tmp_lcd_buffer, msgbox_rect);
	Fwl_RefreshDisplay();
}


/**
 * @brief: will exit SM,if it is in screen save SM. or,reset correlative counter,so have enough time to deal with abort notice
 * @note: inner function
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_VOID
 * @retval: 
 */
static T_VOID Exit_ScreenSave(T_VOID)
{
	if (AK_GetScreenSaverStatus())
	{
		VME_ReTriggerUniqueEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_NULL);

		while(AK_GetScreenSaverStatus())	//wait for until exit screen saver
		{
			Fwl_MiniDelay(20);
		}
		Fwl_Print(C3, M_REC_A, "Exit_ScreenSave:exit screen saver already!");
	}
	else
	{
		UserCountDownReset();
	}
}

/**
 * @brief: set current record state
 * @note :
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eREC_STAT state
 * @return: T_VOID
 * @retval:
 */
static T_VOID SetCurState(T_eREC_STAT state)
{
	AK_ASSERT_PTR_VOID(pRecAudio_Context, "pRecAudio_Context == AK_NULL");

	pRecAudio_Context->curState = state;
	Fwl_Print(C3, M_REC_A, "@current state:%d", state);
}


/**
 * @brief: set last error code
 * @note :
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eLAST_ERROR err
 * @return: T_VOID
 * @retval:
 */
static T_VOID SetLastError(T_eLAST_ERROR err)
{
	AK_ASSERT_PTR_VOID(pRecAudio_Context, "pRecAudio_Context == AK_NULL");

	pRecAudio_Context->lastError = err;
	Fwl_Print(C3, M_REC_A, "@last error code:0x%08x", err);
}


/**
 * @brief: start record
 * @note: inner function
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eREC_ACTION rec_action , specifies the action that record:eREC_ACTION_ONCE or eREC_ACTION_LOOP
 * @return: T_VOID
 * @retval:
 */
static T_VOID StartRecord(T_eREC_ACTION action)
{
#ifdef OS_ANYKA
	
	AutoPowerOffDisable(FLAG_RECAUDIO);

	pRecAudio_Context->curRecTime = 0;
	pRecAudio_Context->curRecedLen = 0;
	pRecAudio_Context->recAction = action;
    pRecAudio_Context->maxLenCanRec = GetMaxLenCanRec();
    Fwl_Print(C3, M_REC_A, "can Record max file len is %u", pRecAudio_Context->maxLenCanRec);

	if (0 == pRecAudio_Context->maxLenCanRec)
	{
		SetLastError(eERR_DISKSPACE_NOTENOUGH);
		goto END;
	}

	if (0 == pRecAudio_Context->recFile[0] || eREC_ACTION_LOOP == pRecAudio_Context->recAction)
	{
		T_STR_20 preName;
		
		if (eINPUT_SOURCE_MIC == pRecAudio_Context->inputSrc)
		{
			Utl_StrCpy(preName, "REC");
		}
		else if (eINPUT_SOURCE_LINEIN == pRecAudio_Context->inputSrc)
		{
			Utl_StrCpy(preName, "FM");
		}
		else
		{
		}
		
		if (!MakeRecFileName(pRecAudio_Context->recFile, preName))				// make record file name
		{
			SetLastError(eERR_FILENAME_MAKEFAILED);
			goto END;
		}
	}

	if (!CreateRecFile())
	{
		SetLastError(eERR_RECFILE_CREATEFAILED);
		goto END;	
	}

    if (!MEncoder_Start(pRecAudio_Context->pMEncoder, pRecAudio_Context->rec_fd, pRecAudio_Context->inputSrc))
    {
		Fwl_FileDestroy(pRecAudio_Context->rec_fd);
		Fwl_FileClose(pRecAudio_Context->rec_fd);
		SetLastError(eERR_MEDIAENCODER_STARTUP_FAILED);
		goto END;
    }

	if (eSTAT_REC_STOP == pRecAudio_Context->curState)
	{
		SetCurState(eSTAT_RECORDING);
	}
	else
	{
		SetCurState(pRecAudio_Context->curState);
	}
	
	pRecAudio_Context->recTimerID = Fwl_SetMSTimerWithCallback(AUDIO_ENCODE_INTERVAL, AK_TRUE, RecordTimerCB);
    if (ERROR_TIMER == pRecAudio_Context->recTimerID)
    {
    	MEncoder_Stop(pRecAudio_Context->pMEncoder);
		Fwl_FileDestroy(pRecAudio_Context->rec_fd);
		Fwl_FileClose(pRecAudio_Context->rec_fd);
		SetLastError(eERR_RECTIMER_CREATEFAILED);
		SetCurState(eSTAT_REC_STOP);
		goto END;
    }

	return;
	
END:
	AutoPowerOffEnable(FLAG_RECAUDIO);

#endif
}


/**
 * @brief: step 1 of stop record, to stop record timer and medium encoder.
 * @note : inner function
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_VOID
 * @retval:
 */
static T_VOID StopRecord_Step1(T_VOID)
{
#ifdef OS_ANYKA

	Fwl_Print(C3, M_REC_A, "--Stop_1");

	if (ERROR_TIMER != pRecAudio_Context->recTimerID)
	{
		Fwl_StopTimer(pRecAudio_Context->recTimerID);
		pRecAudio_Context->recTimerID = ERROR_TIMER;
		AK_Reset_Queue(IThread_GetQueue(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA)));
	}

	if (!MEncoder_Stop(pRecAudio_Context->pMEncoder))
	{
		SetLastError(eERR_MEDIAENCODER_STOP_FAILED);
	}
	
#endif
}


/**
 * @brief: step 2 of stop record, to save or delete record file and do other.
 * @note : inner function
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_BOOL bSaveRecFile , decide save record file whether or not after stop record
 * @return: T_VOID
 * @retval:
 */
static T_VOID StopRecord_Step2(T_BOOL bSaveRecFile)
{
#ifdef OS_ANYKA
	
	Fwl_Print(C3, M_REC_A, "--Stop_2");
	
	if (bSaveRecFile)
	{
		RecUpdateHeader();
		
		Printf_UC(pRecAudio_Context->recFile);
		Fwl_Print(C3, M_REC_A, "@total rec time:%d , total rec len:%u",
				pRecAudio_Context->curRecTime, Fwl_FileTell(pRecAudio_Context->rec_fd));
	}
	else
	{
		Fwl_FileDestroy(pRecAudio_Context->rec_fd);
	}
	
	Fwl_FileClose(pRecAudio_Context->rec_fd);

	pRecAudio_Context->rec_fd		= FS_INVALID_HANDLE;
	pRecAudio_Context->curRecTime	= 0;
	pRecAudio_Context->curRecedLen	= 0;
	pRecAudio_Context->maxLenCanRec = 0;

	
	
	AutoPowerOffEnable(FLAG_RECAUDIO);
		
#endif
}


/***********************Control Functions--begin************************/

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
T_BOOL Ctl_RecAudio_Init(T_eREC_MODE mode, T_U32 sample_rate, T_eINPUT_SOURCE src, T_U32 flag)
{
	Fwl_Print(C3, M_REC_A, "Enter Ctl_RecAudio_Init!");
	
	AK_ASSERT_VAL(AK_NULL == pRecAudio_Context, "pRecAudio_Context is malloced already", AK_FALSE);

	pRecAudio_Context = (T_RECAUDIO_CONTEXT *)Fwl_Malloc(sizeof(T_RECAUDIO_CONTEXT));
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context malloc failed", AK_FALSE);
	Utl_MemSet(pRecAudio_Context, 0, sizeof(T_RECAUDIO_CONTEXT));
	
	pRecAudio_Context->pMEncoder = (T_MEncoder*)Fwl_Malloc(sizeof(T_MEncoder));
	AK_ASSERT_PTR(pRecAudio_Context->pMEncoder, "pRecAudio_Context->pMEncoder malloc failed", AK_FALSE);
	Utl_MemSet(pRecAudio_Context->pMEncoder, 0, sizeof(T_MEncoder));

	//set record mode
	Fwl_Print(C3, M_REC_A, "mode:%d, rate:%d, src:%d", mode, sample_rate, src);
	pRecAudio_Context->recMode    = mode;
	pRecAudio_Context->recRate    = sample_rate;
	pRecAudio_Context->inputSrc   = src;
	pRecAudio_Context->recFlag    = flag;

	pRecAudio_Context->recTimerID = ERROR_TIMER;
	pRecAudio_Context->curState   = eSTAT_REC_STOP;
	pRecAudio_Context->lastError  = eERR_NONE_ERROR;
	pRecAudio_Context->rec_fd     = FS_INVALID_HANDLE;
	pRecAudio_Context->recDir[0]  = 0;
	pRecAudio_Context->recFile[0] = 0;
	pRecAudio_Context->recAction  = eREC_ACTION_ONCE;

	if (eINPUT_SOURCE_MIC == pRecAudio_Context->inputSrc)
	{
		Utl_UStrCpy(pRecAudio_Context->recDir, Fwl_GetDefPath(eAUDIOREC_PATH));
	} 
	else if (eINPUT_SOURCE_LINEIN == pRecAudio_Context->inputSrc)
	{
		Utl_UStrCpy(pRecAudio_Context->recDir, Fwl_GetDefPath(eAUDIOREC_PATH));
	}
	else
	{
		Fwl_Print(C2, M_REC_A, "this input src is nonsupport!");
	}
	
	if (!Fwl_FsMkDirTree(pRecAudio_Context->recDir))
	{
		SetLastError(eERR_DEFRECDIR_CREATE_FAILED);
		return AK_FALSE;
	}

	if (!MEncoder_Init(pRecAudio_Context->pMEncoder, pRecAudio_Context->recMode, 
			pRecAudio_Context->recRate))
	{
		SetLastError(eERR_MEDIAENCODER_INIT_FAILED);
		return AK_FALSE;
	}

	if (!Fwl_InitAsyn(RECAUDIO_ASYNWRITE_BUFSIZE, pRecAudio_Context->recDir))
	{
		SetLastError(eERR_ASYNBUFFER_INIT_FAILED);
		return AK_FALSE;
	}

	pRecAudio_Context->hEvent = -1;

	Fwl_Print(C3, M_REC_A, "Leave Ctl_RecAudio_Init!");
	return AK_TRUE;
}


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
T_BOOL Ctl_RecAudio_Start(T_USTR_FILE filename, T_eREC_ACTION action)
{
#ifdef OS_ANYKA

	T_SYS_MAILBOX mbox;
	T_U32 retrieved_events;

	Fwl_Print(C3, M_REC_A, "Enter Ctl_RecAudio_Start!");

	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", AK_FALSE);

	pRecAudio_Context->lastError = eERR_NONE_ERROR;

	if (eSTAT_REC_STOP != pRecAudio_Context->curState)
	{
		SetLastError(eERR_NOT_STOPSTATE);
		return AK_FALSE;
	}

	if (AK_NULL != filename && eREC_ACTION_LOOP != action)
	{
		Utl_UStrCpy(pRecAudio_Context->recFile, pRecAudio_Context->recDir);
		Utl_UStrCat(pRecAudio_Context->recFile, filename);
		Printf_UC(pRecAudio_Context->recFile);
	}
	else
	{
		pRecAudio_Context->recFile[0] = 0;
	}

	pRecAudio_Context->hEvent = AK_Create_Event_Group();
	if (AK_IS_INVALIDHANDLE(pRecAudio_Context->hEvent))
	{
		SetLastError(eERR_EVENTGROUP_CREATEFAILED);
		return AK_FALSE;
	}

	///send record start event
	mbox.event 			= EVT_RECAUDIO_START;
	mbox.param.w.Param1 = action;
	IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mbox);
	
	Fwl_Print(C3, M_REC_A, "+++start REC:Waiting...");
	
	if (AK_TIMEOUT == AK_Retrieve_Events(pRecAudio_Context->hEvent, EVENT_START_REC, AK_OR_CONSUME, &retrieved_events, 1000))
	{
		SetLastError(eERR_EVENTGROUP_TIMEOUT);
		AK_Delete_Event_Group(pRecAudio_Context->hEvent);
		pRecAudio_Context->hEvent = -1;
		return AK_FALSE;
	}
	else
	{
		AK_Delete_Event_Group(pRecAudio_Context->hEvent);
		pRecAudio_Context->hEvent = -1;
	}
	
	Fwl_Print(C3, M_REC_A, "+++start REC:Continue!\n");

	
	if (eERR_NONE_ERROR == pRecAudio_Context->lastError)
	{
		return AK_TRUE;
	}

#endif

	Fwl_Print(C3, M_REC_A, "Leave Ctl_RecAudio_Start!");
	return AK_FALSE;
}


/**
 * @brief: stop record
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_BOOL bSaveRecFile , decide save record file whether or not after stop record
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Stop(T_BOOL bSaveRecFile)
{
#ifdef OS_ANYKA

	T_SYS_MAILBOX mbox;
	T_U32 retrieved_events;

	Fwl_Print(C3, M_REC_A, "Enter Ctl_RecAudio_Stop!");

	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", AK_FALSE);

	pRecAudio_Context->lastError = eERR_NONE_ERROR;

	if (eSTAT_REC_STOP == pRecAudio_Context->curState)
	{
		SetLastError(eERR_ALREADY_IS_STOPSTATE);
		return AK_FALSE;
	}

	if (MEncoder_GetCurTime(pRecAudio_Context->pMEncoder) < 1000)
	{
		Fwl_MiniDelay(abs(1000-MEncoder_GetCurTime(pRecAudio_Context->pMEncoder)));
	}

	///send record stop event
	pRecAudio_Context->hEvent = AK_Create_Event_Group();
	
	if (AK_IS_INVALIDHANDLE(pRecAudio_Context->hEvent))
	{
		SetLastError(eERR_EVENTGROUP_CREATEFAILED);
		return AK_FALSE;
	}
	else
	{
		mbox.event = EVT_RECAUDIO_STOP;
		mbox.param.w.Param1 = bSaveRecFile;
		IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mbox);
		
		Fwl_Print(C3, M_REC_A, "+++stop REC:Waiting...");
		
		if (AK_TIMEOUT == AK_Retrieve_Events(pRecAudio_Context->hEvent, EVENT_STOP_REC, AK_OR_CONSUME, &retrieved_events, 3500))
		{
			SetLastError(eERR_EVENTGROUP_TIMEOUT);
			AK_Delete_Event_Group(pRecAudio_Context->hEvent);
			pRecAudio_Context->hEvent = -1;
			return AK_FALSE;
		}
		else
		{
			AK_Delete_Event_Group(pRecAudio_Context->hEvent);
			pRecAudio_Context->hEvent = -1;
		}
		
		Fwl_Print(C3, M_REC_A, "+++stop REC:Continue!");
	}


	if (eERR_NONE_ERROR == pRecAudio_Context->lastError)
	{
		return AK_TRUE;
	}

#endif

	Fwl_Print(C3, M_REC_A, "Leave Ctl_RecAudio_Stop!");
	return AK_FALSE;
}


/**
 * @brief: release record resource
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.To get extended error information, call Ctl_RecAudio_GetLastError. 
 */
T_BOOL Ctl_RecAudio_Destroy(T_VOID)
{
	T_BOOL retval = AK_TRUE;
	
	Fwl_Print(C3, M_REC_A, "Enter Ctl_RecAudio_Destroy!");
	
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", AK_FALSE);

	if (eSTAT_REC_STOP == pRecAudio_Context->curState)
	{
		pRecAudio_Context->lastError = eERR_NONE_ERROR;

		if (!Fwl_DeInitAsyn(pRecAudio_Context->recDir))
		{
			SetLastError(eERR_ASYNBUFFER_DELETE_FAILED);
			retval = AK_FALSE;
		}

		if (!MEncoder_Destroy(pRecAudio_Context->pMEncoder))
		{
			SetLastError(eERR_MEDIAENCODER_DESTROY_FAILED);
			retval = AK_FALSE;
		}
		
		if (AK_NULL != pRecAudio_Context->pMEncoder)
		{
			Fwl_Free(pRecAudio_Context->pMEncoder);
			pRecAudio_Context->pMEncoder = AK_NULL;
		}

		if (AK_NULL != pRecAudio_Context)
		{
			Fwl_Free(pRecAudio_Context);
			pRecAudio_Context = AK_NULL;
		}
	}
	else
	{
		Fwl_Print(C2, M_REC_A, "Ctl_RecAudio_Destroy failed, because curState is %d", pRecAudio_Context->curState);
		retval = AK_FALSE;
	}

	Fwl_Print(C3, M_REC_A, "Leave Ctl_RecAudio_Destroy!");
	return retval;
}


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
T_BOOL Ctl_RecAudio_HandleMsg(T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
#ifdef OS_ANYKA

    T_BOOL bError = AK_FALSE;
	T_U32  preRecTime = 0;
	
	AK_ASSERT_PTR(pRecAudio_Context, "AK_NULL == pRecAudio_Context", AK_FALSE);

	switch (eEvent)
	{
	case EVT_RECAUDIO_START:
		StartRecord(pEvtParam->w.Param1);
		
		AK_Set_Events(pRecAudio_Context->hEvent, EVENT_START_REC, AK_OR);
		break;
		
    case EVT_RECAUDIO_REC:
		if (FS_INVALID_HANDLE != pRecAudio_Context->rec_fd 
				&& eSTAT_REC_STOP != pRecAudio_Context->curState) 
		{
			///read/filter/encode PCM data and write into file.
			pRecAudio_Context->curRecedLen += MEncoder_HandleAudioEncode(pRecAudio_Context->pMEncoder, &bError);

			///update record file head info.
			preRecTime = pRecAudio_Context->curRecTime;
			pRecAudio_Context->curRecTime = MEncoder_GetCurTime(pRecAudio_Context->pMEncoder) / 1000;
			if (0 != pRecAudio_Context->curRecTime && preRecTime != pRecAudio_Context->curRecTime
				&& (0 == (pRecAudio_Context->curRecTime & RECFILEHEAD_UPDATE_INTERVAL)))
			{
				T_S32 write_len = 0;
				
				if (pRecAudio_Context->pMEncoder->OutBuf.dataLen > 0)
				{
					write_len = Fwl_FileWrite(pRecAudio_Context->rec_fd,
										pRecAudio_Context->pMEncoder->OutBuf.pBuf,
										pRecAudio_Context->pMEncoder->OutBuf.dataLen);
					if (write_len != pRecAudio_Context->pMEncoder->OutBuf.dataLen)
					{
						Fwl_Print(C2, M_REC_A, "+write data error");
						bError = AK_TRUE;
					}
					
					Fwl_Print(C3, M_REC_A, "###=>%d", write_len);
					pRecAudio_Context->curRecedLen += write_len;
					pRecAudio_Context->pMEncoder->OutBuf.dataLen = 0;
					pRecAudio_Context->pMEncoder->audEncBuf.buf_out = pRecAudio_Context->pMEncoder->OutBuf.pBuf;
				}

				RecUpdateHeader();
			}

			///check whether appear write file errors.
			if (bError)
			{
				if (!Fwl_CheckDriverIsValid(pRecAudio_Context->recFile))
				{
				  /* storage medium is pulled out
					ignore it here,to handle by ddpublicdetecthandler()
					reason:cause filesystem die lock when write operate and SD/MMC/USB HOST unmount operate are occur synchronously */
					Fwl_Print(C3, M_REC_A, "Ctl_RecAudio_HandleMsg:storage medium is pulled out 1");
					return AK_FALSE;
				}
				else
				{
					Exit_ScreenSave();
					StopRecord_Step1();
					RECAbort_ShowInfo(eABORT_WRITEFILE_ERROR);
					StopRecord_Step2(AK_FALSE);
					SetCurState(eSTAT_REC_STOP);
					
					if (eSTAT_RECORDING_BG == pRecAudio_Context->curState)
					{
						SetCurState(eSTAT_REC_STOP);
						Fwl_Print(C3, M_REC_A, "call Ctl_RecAudio_Destroy inside");
						Ctl_RecAudio_Destroy();
					}
					else
					{
						SetCurState(eSTAT_REC_STOP);
					}
					
					return AK_TRUE;
				}
			}

			///
			if (pRecAudio_Context->curRecTime >= RECTIME_MAXVALUE)
			{
				//reach max record time
				Exit_ScreenSave();
				StopRecord_Step1();
				RECAbort_ShowInfo(eABORT_REACH_MAXRECTIME);
				StopRecord_Step2(AK_TRUE);
				
				if (eREC_ACTION_LOOP == pRecAudio_Context->recAction)
				{
					//if loop record, then start record again automatically
					T_SYS_MAILBOX mbox;
					mbox.event = EVT_RECAUDIO_START;
					mbox.param.w.Param1 = pRecAudio_Context->recAction;
					IAppMgr_PostEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mbox); 					
					Fwl_Print(C3, M_REC_A, "++to start record again automatically!");
				}
				else
				{
					if (eSTAT_RECORDING_BG == pRecAudio_Context->curState)
					{
						SetCurState(eSTAT_REC_STOP);
						Fwl_Print(C3, M_REC_A, "call Ctl_RecAudio_Destroy inside");
						Ctl_RecAudio_Destroy();
					}
					else
					{
						SetCurState(eSTAT_REC_STOP);
					}
				}

				return AK_TRUE;
			}
						
			///
			pRecAudio_Context->maxLenCanRec = GetMaxLenCanRec();
			
			if (pRecAudio_Context->curRecedLen > pRecAudio_Context->maxLenCanRec)
			{
				T_U64_INT freesize;

				Fwl_FsGetFreeSize(pRecAudio_Context->recDir[0], &freesize);
				
				if (!Fwl_CheckDriverIsValid(pRecAudio_Context->recFile))
				{
				  /* storage medium is pulled out
					ignore it here,to handle by ddpublicdetecthandler()
					reason:will cause filesystem die lock when write operate and SD/MMC/USB HOST unmount operate are occur synchronously */
					Fwl_Print(C3, M_REC_A, "Ctl_RecAudio_HandleMsg:storage medium is pulled out 2");
					return AK_FALSE;
				} 
				else if (U64cmpU32(&freesize, AUDIOREC_MINIMAL_SPACE) <= 0)
				{
					//diskspace is not enough
					Exit_ScreenSave();
					StopRecord_Step1();
					RECAbort_ShowInfo(eABORT_DISKSPACE_NOT_ENOUGH);
					StopRecord_Step2(AK_TRUE);
					SetCurState(eSTAT_REC_STOP);
				}
				else
				{
					//reach max record length
					Exit_ScreenSave();
					StopRecord_Step1();	
					RECAbort_ShowInfo(eABORT_REACH_MAXLENCANREC);
					StopRecord_Step2(AK_TRUE);
					
					if (eREC_ACTION_LOOP == pRecAudio_Context->recAction)
					{
						//if loop record, then start record again automatically
						T_SYS_MAILBOX mbox;
						mbox.event = EVT_RECAUDIO_START;
						mbox.param.w.Param1 = pRecAudio_Context->recAction;
						IAppMgr_PostEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mbox);						
						Fwl_Print(C3, M_REC_A, "++to start record again automatically!");
					}
					else
					{
						if (eSTAT_RECORDING_BG == pRecAudio_Context->curState)
						{
							SetCurState(eSTAT_REC_STOP);
							Fwl_Print(C3, M_REC_A, "call Ctl_RecAudio_Destroy inside");
							Ctl_RecAudio_Destroy();
						}
						else
						{
							SetCurState(eSTAT_REC_STOP);
						}
					}
				}

				return AK_TRUE;
			}
					
		}
		else
		{
			Fwl_Print(C3, M_REC_A, "@EVT_RECAUDIO_REC:is eSTAT_REC_STOP already!\n");
		}
		
		break;
		
    case EVT_RECAUDIO_STOP:
		if (eSTAT_REC_STOP != pRecAudio_Context->curState)
		{
			//if right to stop record audio,
			StopRecord_Step1();

			if (!Fwl_CheckDriverIsValid(pRecAudio_Context->recFile))
			{
				StopRecord_Step2(AK_FALSE);
				RECAbort_ShowInfoNoSendMsg(eABORT_STORAGEMEDIUM_NOT_EXIST);
				SetCurState(eSTAT_REC_STOP);
			}
			else
			{
				StopRecord_Step2(pEvtParam->w.Param1);
				if (pEvtParam->w.Param1)
				{
					RECAbort_ShowInfoNoSendMsg(eABORT_NORMAL_STOPREC);
				}
				SetCurState(eSTAT_REC_STOP);
			}
		}
		else
		{
			Fwl_Print(C3, M_REC_A, "@EVT_RECAUDIO_STOP:is eSTAT_REC_STOP already!");
		}
		
		AK_Set_Events(pRecAudio_Context->hEvent, EVENT_STOP_REC, AK_OR);
		
		break;

	default:
		Fwl_Print(C2, M_REC_A, "@unknown event!");
		break;
	}
	
#endif	

    return AK_TRUE;
}

/***********************Control Functions--end**************************/


/**********************Info Get Functions--begin*************************/

/**
 * @brief: get last sample data
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_S16 sample[2]
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_GetCurSample(T_S16 sample[2])
{
	AK_ASSERT_PTR(pRecAudio_Context, "MEncoder_GetCurSample is  NULL", AK_FALSE);

	MEncoder_GetCurSample(pRecAudio_Context->pMEncoder, sample);
    
	return AK_TRUE;
}


/**
 * @brief: get current record state
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_S32
 * @retval: 
 */
T_eREC_STAT Ctl_RecAudio_GetCurState(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", eSTAT_UNKNOWN);

	return pRecAudio_Context->curState;
}


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
T_eLAST_ERROR Ctl_RecAudio_GetLastError(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", -1);
	
	return pRecAudio_Context->lastError;
}


/**
 * @brief: get current record time
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval: 
 */
T_U32 Ctl_RecAudio_GetCurRecTime(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", 0);

	return pRecAudio_Context->curRecTime;
}


/**
 * @brief: get current recorded data length
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval 
 */
T_U32 Ctl_RecAudio_GetCurRecedLen(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", 0);

	return pRecAudio_Context->curRecedLen;
}


/**
 * @brief: get current max length of can record, it is real time.
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_U32
 * @retval : value is 0~RECFILE_MAXLEN
 */
T_U32 Ctl_RecAudio_GetMaxLenCanRec(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", 0);

	return pRecAudio_Context->maxLenCanRec;
}


/**
 * @brief: see about whether medium of placed record file exist or not.
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_IsExistMedium(T_VOID)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", AK_TRUE);

	return Fwl_CheckDriverIsValid(pRecAudio_Context->recFile);
}

/**********************Info Get Functions--end**************************/


#endif	// #define (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))


/**
 * @brief:  
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: bToBgRec , AK_TRUE--to background record; AK_FALSE--to foreground record.
 * @return: T_BOOL
 * @retval: AK_TRUE is success,or failed.
 */
T_BOOL Ctl_RecAudio_ToBgRec(T_BOOL bToBgRec)
{
	AK_ASSERT_PTR(pRecAudio_Context, "pRecAudio_Context == AK_NULL", AK_FALSE);

	if (bToBgRec)
	{
		if (eSTAT_RECORDING == pRecAudio_Context->curState)
		{
			SetCurState(eSTAT_RECORDING_BG);
			return AK_TRUE;
		}
	}
	else
	{
		if (eSTAT_RECORDING_BG == pRecAudio_Context->curState)
		{
			SetCurState(eSTAT_RECORDING);
			return AK_TRUE;
		}
	}

	return AK_FALSE;
}


/**
 * @brief: check record background record state
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_eINPUT_SOURCE input_src, 
 * @return: T_BOOL
 * @retval: 
 */
T_BOOL Ctl_RecAudio_IsBgRec(T_eINPUT_SOURCE input_src)
{
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))

#ifdef OS_ANYKA
	if (AK_NULL != pRecAudio_Context
		&& pRecAudio_Context->inputSrc == input_src)
	{
		return (eSTAT_RECORDING_BG == pRecAudio_Context->curState);
	}
#endif

#endif

	return AK_FALSE;
}


/**
 * @brief: check record stop state
 * @note : 
 * @author: hoube
 * @date: 2012-3-3
 * @param: T_VOID
 * @return: T_BOOL
 * @retval: if record is stoped already,return AK_TRUE; if is other state, return AK_FALSE.
 */
T_BOOL Ctl_RecAudio_IsStoped(T_VOID)
{
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))

#ifdef OS_ANYKA
	if (AK_NULL != pRecAudio_Context)
	{
		return (eSTAT_REC_STOP == pRecAudio_Context->curState);
	}
#endif

#endif

	return AK_TRUE;
}

