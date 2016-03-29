
#include "Gbl_Global.h"
#ifdef SUPPORT_EXPLORER
#include "Fwl_public.h"
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_Progress.h"
#include "Fwl_pfKeypad.h"
#include "Eng_FileManage.h"
#include "Eng_ScreenSave.h"
#include "Fwl_osFS.h"
#include "Lib_state.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_DataConvert.h"
#include "Eng_String.h"
#include "fwl_keyhandler.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"
#include "Eng_Math.h"

//set the block size pasted per time
#define TIME_STR_LEN    8
#define SPACE_SIZE_KEEP (5<<20)

#define FILE_COPY_INIT(mutex)        ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define FILE_COPY_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define FILE_COPY_UNLOCK(mutex)      AK_Release_Semaphore((mutex))
#define FILE_COPY_DEINIT(mutex)      {\
		AK_Delete_Semaphore(mutex);\
		(mutex) = -1;\
}

typedef struct {
	T_U16 FileName[FS_MAX_PATH_LEN+1];
	T_U32 CurPos;
	T_U32 CurFileSize;
}T_FILE_COPY_INIT;

typedef struct {
    T_MSGBOX        msgbox;                     /**< display the pasting info */
    T_PGBAR         		pgbar;                      /**< display the pasting progress */
    T_USTR_FILE     		SrcPath;
    T_USTR_FILE     		DestPath;
    T_BOOL          		CutFlag;
    T_BOOL          		FoldeFlag;
    T_U32           		TempTime;
    T_U8            		StrLeaveTime[TIME_STR_LEN]; /** the left time, in str form */
    
	T_FILE_COPY_INIT        FileCurCopyInit;
	T_BOOL					CopyStopFlg;               /*if is false ,copy is stop,and del cur copy file*/
	T_FS_COPY_CALLBACK  	CopyBackFum;
	T_BOOL          		SameFile;
	T_S32           		DisTimes;
	T_U16           		LastFileName[FS_MAX_PATH_LEN+1];
	T_PCOPY_CTRL    		thrdCtrl;
    volatile T_U32          StartTime;                  /**< the time start to paste file */
    volatile T_U32          DelayTime;                  /**< the delay time ,since the StartTime */
	volatile T_U8           MboxReturnLevel;
	volatile e_FILE_COPY_STATE CopyState;
	//T_BOOL          bUsbIn;
} T_EXPLORER_PASTE_FILE_PARM;


static T_EXPLORER_PASTE_FILE_PARM *Explorer_Paste_File_Parm = AK_NULL;
static T_hSemaphore hPasteFileLock = -1;

void resumepaste_file(void);
T_U32 get_msgbox_maxtcol(T_MSGBOX *mbox);
extern T_VOID  MsgBox_ClearContent_EX(T_MSGBOX *mbox);

static T_BOOL Explorer_FileCopyCallBack(T_VOID *pData, T_U16 *FileName,T_U32 CurPos, T_U32 FileSize);
static T_VOID Explorer_FileCopyDisplayScale(T_VOID);
//static T_VOID Explorer_FileCopyStarttimer(T_BOOL isEnable);

static T_BOOL Explorer_FileCopyCallBack(T_VOID *pData, T_U16 *FileName,T_U32 CurPos, T_U32 FileSize)
{
	//T_FILE_MANAGE_RETURN_VALUE  RetValue;

	FILE_COPY_LOCK(hPasteFileLock);

	if (AK_NULL != FileName)
	{
		Utl_UStrCpyN(Explorer_Paste_File_Parm->FileCurCopyInit.FileName,FileName,FS_MAX_PATH_LEN);
		Explorer_Paste_File_Parm->FileCurCopyInit.CurPos = CurPos;
		Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize = FileSize;

		if(0 != Utl_UStrCmp(Explorer_Paste_File_Parm->FileCurCopyInit.FileName,Explorer_Paste_File_Parm->LastFileName)) 
		{
			Explorer_Paste_File_Parm->DelayTime = 0;
			Explorer_Paste_File_Parm->StartTime = Fwl_GetTickCount();
			Utl_UStrCpyN(Explorer_Paste_File_Parm->LastFileName,Explorer_Paste_File_Parm->FileCurCopyInit.FileName,FS_MAX_PATH_LEN);
		}
	}
	
	FILE_COPY_UNLOCK(hPasteFileLock);
	
	return Explorer_Paste_File_Parm->CopyStopFlg;
}

static T_VOID Explorer_FileCopyState(e_FILE_COPY_STATE State)
{	
	FILE_COPY_LOCK(hPasteFileLock);

	Explorer_Paste_File_Parm->CopyState = State;
	
	FILE_COPY_UNLOCK(hPasteFileLock);

	Fwl_Print(C3, M_EXPLORER, "@@Copy State:%d",State);
}

static T_VOID Explorer_FileCopyDisplayScale(T_VOID)
{
    T_U32       current_time;
    T_U32       maxtcol;
	T_U32       speed;
	T_S32       tm_leaving;
	T_USTR_INFO uText;
	T_U32       curvalue;
    T_USTR_FILE     FileName;
    T_pRECT     	pMBoxRct;
	T_U64_INT       disCurVil;
	T_U8            ret;
	
	FILE_COPY_LOCK(hPasteFileLock);
	
	Fwl_Print(C3, M_EXPLORER, "File Copy:CurFileSize:%d,CurPos:%d",Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize,Explorer_Paste_File_Parm->FileCurCopyInit.CurPos);
	if (Explorer_Paste_File_Parm->FileCurCopyInit.CurPos == Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize)
	{		
		Explorer_Paste_File_Parm->DelayTime = 0;
		Explorer_Paste_File_Parm->StartTime = Fwl_GetTickCount();
		Explorer_Paste_File_Parm->FileCurCopyInit.CurPos = 0;
		Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize = 0;
	}
	
	Explorer_Paste_File_Parm->DisTimes++;
	//handle the file paste process,as following:
	Explorer_Paste_File_Parm->TempTime = Explorer_Paste_File_Parm->DelayTime;
	Explorer_Paste_File_Parm->DelayTime = Fwl_GetTickCount();
	
	pMBoxRct = &Explorer_Paste_File_Parm->msgbox.res.MsgBkImgRct;
	
	Utl_UStrCpy(FileName, Explorer_Paste_File_Parm->FileCurCopyInit.FileName);
		
	current_time = (Explorer_Paste_File_Parm->DelayTime - Explorer_Paste_File_Parm->StartTime)/1000;
	/***当文件粘贴过程延时达1s时，显示粘贴进度***/
	//消息框显示文件名
	if(Explorer_Paste_File_Parm->CutFlag)
	{
		if(Explorer_Paste_File_Parm->DisTimes <= 2)
		{				  
			MsgBox_InitAfx(&Explorer_Paste_File_Parm->msgbox, 0, ctHINT, csEXPLORER_CUT, MSGBOX_INFORMATION|MSGBOX_EXIT);
		}
		else
		{				  
			MsgBox_ClearContent_EX(&Explorer_Paste_File_Parm->msgbox);
			MsgBox_AddLine(&Explorer_Paste_File_Parm->msgbox, GetCustomString(csEXPLORER_CUT));					  
		}								
	}
	else
	{
		if(Explorer_Paste_File_Parm->DisTimes <= 2) 	 
		{				  
			MsgBox_InitAfx(&Explorer_Paste_File_Parm->msgbox, 0, ctHINT, csEXPLORER_COPY, MSGBOX_INFORMATION|MSGBOX_EXIT);
		}
		else
		{
			MsgBox_ClearContent_EX(&Explorer_Paste_File_Parm->msgbox);
			MsgBox_AddLine(&Explorer_Paste_File_Parm->msgbox, GetCustomString(csEXPLORER_COPY)); 				   
		}
	}				 
	
	maxtcol = get_msgbox_maxtcol(&Explorer_Paste_File_Parm->msgbox);
	
	if (Utl_UStrLen(FileName) > maxtcol*2-1)
	{
		T_U8 i;
		T_USTR_FILE shortname;
	
		Utl_UStrCpy(shortname, FileName);
		shortname[maxtcol*2-1] = 0;
		for (i=1; i<4;i++)
		{
			shortname[maxtcol*2-2-i] = UNICODE_DOT;
		}
		 //if(nTimes <= 2)		
		 {					  
			MsgBox_AddLine(&Explorer_Paste_File_Parm->msgbox, shortname);
		 }
		
	}
	else
	{				 
		MsgBox_AddLine(&Explorer_Paste_File_Parm->msgbox, FileName);
	}
	if(current_time >= 1)
	{
		if (Explorer_Paste_File_Parm->StrLeaveTime[0] == '\0' || \
				Explorer_Paste_File_Parm->DelayTime != Explorer_Paste_File_Parm->TempTime)
		{
			speed = (Explorer_Paste_File_Parm->FileCurCopyInit.CurPos)/current_time;
			tm_leaving = (Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize - Explorer_Paste_File_Parm->FileCurCopyInit.CurPos)/(speed+1)+1;
			Utl_Itoa(tm_leaving, Explorer_Paste_File_Parm->StrLeaveTime, TIME_STR_LEN);
		}
	
		Utl_UStrCpy(uText, GetCustomString(csEXPLORER_WAIT));
		Utl_UStrCat(uText, _T(" "));
		Utl_UStrCat(uText, _T(Explorer_Paste_File_Parm->StrLeaveTime));
		Utl_UStrCat(uText, _T(" s"));
	
		//AK_DEBUG_OUTPUT("waiting time = %s, tm_leaving=%d", Explorer_Paste_File_Parm.StrLeaveTime,tm_leaving);			 
		
		MsgBox_AddLine(&Explorer_Paste_File_Parm->msgbox, uText);					
	
		MsgBox_SetRefresh(&Explorer_Paste_File_Parm->msgbox,CTL_REFRESH_CONTENT);
		
		//MsgBox_Show(&Explorer_Paste_File_Parm.msgbox); 
		//Fwl_RefreshDisplay();
	
		ret = U32MultiplyU32_REU64(Explorer_Paste_File_Parm->FileCurCopyInit.CurPos,25,&disCurVil);
		if (0 == ret)
		{
			U64DivideU32(disCurVil,Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize,&curvalue);
		}
		else
		{
			curvalue = 12;
			Fwl_Print(C2, M_EXPLORER, "file copy U32MultiplyU32_REU64 fail");
		}

		PgBar_Init(&Explorer_Paste_File_Parm->pgbar, (T_POS)(pMBoxRct->left + 8),
					(T_LEN)(pMBoxRct->top + pMBoxRct->height - 14),
					(T_LEN)(pMBoxRct->width - 16), 12, 0, PGBAR_PILE_ON);
		PgBar_SetValue(&Explorer_Paste_File_Parm->pgbar, (T_U16)curvalue, 25);
	}
	
	//if(nTimes <= 2)
	{
		MsgBox_Show(&Explorer_Paste_File_Parm->msgbox);
	}
	
	if (current_time >= 1)
		PgBar_Show(&Explorer_Paste_File_Parm->pgbar);

	FILE_COPY_UNLOCK(hPasteFileLock);

	Fwl_RefreshDisplay();
	
}
#endif
/*---------------------- BEGIN OF STATE s_explorer_paste_file ------------------------*/
void initexplorer_paste_file(void)
{
#ifdef SUPPORT_EXPLORER
    //FreqMgr_StateCheckIn(FREQ_FACTOR_FILE_ACCESS, FREQ_PRIOR_HIGH);
    
    Explorer_Paste_File_Parm = (T_EXPLORER_PASTE_FILE_PARM *)Fwl_Malloc(sizeof(T_EXPLORER_PASTE_FILE_PARM));
    AK_ASSERT_PTR_VOID(Explorer_Paste_File_Parm, "Explorer_Paste_File_Parm: malloc error");
	memset(Explorer_Paste_File_Parm,0,sizeof(T_EXPLORER_PASTE_FILE_PARM));

	ScreenSaverDisable();
    //m_regResumeFunc(resumepaste_file);
    
	FILE_COPY_INIT(hPasteFileLock);
	
    Explorer_Paste_File_Parm->CutFlag = FileMgr_GetCutFlag();
    Explorer_Paste_File_Parm->FoldeFlag = FileMgr_GetFolderFlag();
	Explorer_Paste_File_Parm->DisTimes = 1;
	Explorer_Paste_File_Parm->FileCurCopyInit.CurPos = 0;
	Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize = 0;
 	Explorer_Paste_File_Parm->CopyStopFlg = AK_TRUE;
	Explorer_Paste_File_Parm->CopyState = eFS_COPY_OtherState;
	Explorer_Paste_File_Parm->SameFile = AK_FALSE;
	
	Explorer_Paste_File_Parm->CopyBackFum.Callback = (F_CopyCallback)Explorer_FileCopyCallBack;
	Explorer_Paste_File_Parm->CopyBackFum.SetState = (F_CopySetState)Explorer_FileCopyState;

	if (Explorer_Paste_File_Parm->FoldeFlag) //控制返回哪一个状态机
		Explorer_Paste_File_Parm->MboxReturnLevel = 3;
	else
		Explorer_Paste_File_Parm->MboxReturnLevel = 2;
 #endif
 }

void exitexplorer_paste_file(void)
{
#ifdef SUPPORT_EXPLORER

	if ((Explorer_Paste_File_Parm->CutFlag == AK_TRUE) && (Explorer_Paste_File_Parm->FoldeFlag != AK_TRUE))
        FileMgr_SetManageState(FILE_NULL);
    //deal with event :SD card in/out or USB in when pasteing
    if (eFS_COPY_ING == Explorer_Paste_File_Parm->CopyState) //SW10A00001018
    {
        Fwl_Print(C2, M_EXPLORER, "explorer_paste_file():  unexpected exit !!");
		
		Explorer_Paste_File_Parm->CopyStopFlg = AK_FALSE;
		/*SW10A00002222 如果没有这个等待，后台线程函数还没有退出就去KILL线程，会死机*/
		while(1) 
		{
			AK_Sleep(40);
			if (eFS_COPY_ING != Explorer_Paste_File_Parm->CopyState)
				break;
		}
    }
    ScreenSaverEnable();

	memset(Explorer_Paste_File_Parm->FileCurCopyInit.FileName,0,FS_MAX_PATH_LEN);
	memset(Explorer_Paste_File_Parm->LastFileName,0,FS_MAX_PATH_LEN);

	FILE_COPY_LOCK(hPasteFileLock);

	Fwl_KillCopyThead(&Explorer_Paste_File_Parm->thrdCtrl);

	FILE_COPY_UNLOCK(hPasteFileLock);

	Fwl_Free(Explorer_Paste_File_Parm);
	Explorer_Paste_File_Parm = AK_NULL;
		
	FILE_COPY_DEINIT(hPasteFileLock);
#endif
}

void paintexplorer_paste_file(void)
{
#ifdef SUPPORT_EXPLORER
	if (eFS_COPY_ING == Explorer_Paste_File_Parm->CopyState) 
	{
		Explorer_FileCopyDisplayScale();
	}
#endif
}

unsigned char handleexplorer_paste_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_EXPLORER

    T_U16       driverid1,driverid2;
	T_U64_INT   free_size = {0};    
	T_BOOL      ret=AK_FALSE;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }
		
    switch(event)
    {
        case M_EVT_PASTE_FILE:
			Explorer_Paste_File_Parm->SameFile =  (T_BOOL)((T_U32)(pEventParm->p.pParam3));
			
            /***取得进行粘贴操作的源文件和目标文件的信息***/
            if (!Explorer_Paste_File_Parm->FoldeFlag)
            {
                FileMgr_GetSrcPath(Explorer_Paste_File_Parm->SrcPath);
                FileMgr_GetDestPath(Explorer_Paste_File_Parm->DestPath);
                //FileMgr_GetFileName(Explorer_Paste_File_Parm.FileName);
            }
            else
            {            
                Utl_UStrCpy(Explorer_Paste_File_Parm->SrcPath, pEventParm->p.pParam1);
                Utl_UStrCpy(Explorer_Paste_File_Parm->DestPath, pEventParm->p.pParam2);
            }

            driverid1 = FileMgr_GetDriverId(Explorer_Paste_File_Parm->SrcPath);
            driverid2 = FileMgr_GetDriverId(Explorer_Paste_File_Parm->DestPath);

            /***若同驱动器且为剪切***/
            if (Explorer_Paste_File_Parm->CutFlag && (driverid1 == driverid2))//SW10A00002116
            {
            	T_BOOL filemove_result = AK_FALSE;

				if (Fwl_FsIsFile(Explorer_Paste_File_Parm->SrcPath))
				{
					if (FileMgr_CheckFileIsExist(Explorer_Paste_File_Parm->DestPath))
                    	Fwl_FileDelete(Explorer_Paste_File_Parm->DestPath);
									
					filemove_result = Fwl_FileMove(Explorer_Paste_File_Parm->SrcPath,Explorer_Paste_File_Parm->DestPath);
					
					if(filemove_result)
					{
						Explorer_Paste_File_Parm->CopyState = eFS_COPY_Success;
					}
					else
					{
						Explorer_Paste_File_Parm->CopyState = eFS_COPY_Fail;
					}
					
					break;
				}
				else
				{
					filemove_result = Fwl_FolderMove(Explorer_Paste_File_Parm->SrcPath,Explorer_Paste_File_Parm->DestPath,\
						 			Explorer_Paste_File_Parm->CopyBackFum,&(Explorer_Paste_File_Parm->thrdCtrl));
					if (!filemove_result)
					{
						Fwl_Print(C1, M_EXPLORER, "Exlporer Folder Move Fail,return!!");
						Explorer_Paste_File_Parm->CopyState = eFS_COPY_Fail;
					}
				}
                break;
            }

			Explorer_Paste_File_Parm->StartTime = Fwl_GetTickCount();
            Explorer_Paste_File_Parm->StrLeaveTime[0] = '\0';
			Explorer_Paste_File_Parm->DelayTime = 0;
			
			ret = Fwl_FileCopy(Explorer_Paste_File_Parm->SrcPath,Explorer_Paste_File_Parm->DestPath,\
						Explorer_Paste_File_Parm->SameFile,AK_NULL,\
						Explorer_Paste_File_Parm->CopyBackFum,&(Explorer_Paste_File_Parm->thrdCtrl));
			
			Fwl_Print(C3, M_EXPLORER, "ret:%d",ret);
			if (!ret)
			{
				Fwl_Print(C1, M_EXPLORER, "Exlporer Copy File Fail,return!!");
				Explorer_Paste_File_Parm->CopyState = eFS_COPY_Fail;
			}
			
            break;
			
		case M_EVT_USER_KEY:
		case M_EVT_TOUCH_SCREEN:
			
            if (eFS_COPY_ING == Explorer_Paste_File_Parm->CopyState)
            {
            	T_eBACK_STATE	ret;
				
                ret = MsgBox_Handler(&Explorer_Paste_File_Parm->msgbox, event, pEventParm);      
								
				if ((eNext == ret) || (eReturn == ret) || (eHome == ret))
				{
					Explorer_Paste_File_Parm->CopyStopFlg = AK_FALSE;
				} 
            }
            break;
			
		case SYS_EVT_USB_PLUG: // when USB/SD/MMC is in
		case SYS_EVT_SD_PLUG:	
			if ((pEventParm->c.Param8 == EVT_USB_PLUG_IN) ||(pEventParm->c.Param1 == EVT_SD_PLUG_IN))
			{
				Explorer_Paste_File_Parm->CopyStopFlg = AK_FALSE;
			}
			break;
			
		case VME_EVT_TIMER:
			break;
			
        default:
            break;
    }

	//退出本状态机
	if (eFS_COPY_ING != Explorer_Paste_File_Parm->CopyState \
		&& eFS_COPY_OtherState != Explorer_Paste_File_Parm->CopyState)
	{
		if (eFS_COPY_Success == Explorer_Paste_File_Parm->CopyState)
		{
			//当不是同一驱动盘时，实际采用方法是复制，当复制完时，需删除源目录
			if (Explorer_Paste_File_Parm->CutFlag  \
				&& (FileMgr_GetDriverId(Explorer_Paste_File_Parm->SrcPath) != FileMgr_GetDriverId(Explorer_Paste_File_Parm->DestPath)))
			{
		    	Fwl_FileDelete(Explorer_Paste_File_Parm->SrcPath);
			}

			MsgBox_InitAfx(&Explorer_Paste_File_Parm->msgbox, Explorer_Paste_File_Parm->MboxReturnLevel, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
		}
		else
		{
    		Fwl_FsGetFreeSize(Explorer_Paste_File_Parm->DestPath[0], &free_size);
					
		    if ((free_size.low < (Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize + SPACE_SIZE_KEEP)) \
					&& (free_size.high < 1))    //SPACE IS NOT ENOUGH
			{
				Fwl_Print(C1, M_EXPLORER, "Not Space:Need:%d,Free:%d",(Explorer_Paste_File_Parm->FileCurCopyInit.CurFileSize + SPACE_SIZE_KEEP),free_size.low);
				MsgBox_InitAfx(&Explorer_Paste_File_Parm->msgbox,Explorer_Paste_File_Parm->MboxReturnLevel, ctHINT, csEXPLORER_FREE_SIZE, MSGBOX_INFORMATION);
		    }
			else
			{
				MsgBox_InitAfx(&Explorer_Paste_File_Parm->msgbox, Explorer_Paste_File_Parm->MboxReturnLevel, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
			}
		}
		
		MsgBox_SetDelay(&Explorer_Paste_File_Parm->msgbox, MSGBOX_DELAY_1);
		m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&Explorer_Paste_File_Parm->msgbox);
	}
#endif
    return 0;
}

/*---------------------- END OF STATE s_explorer_paste_file ------------------------*/
#ifdef SUPPORT_EXPLORER

void resumepaste_file(void)
{
    m_triggerEvent(M_EVT_PASTE_EXIT, AK_NULL);
}

T_U32 get_msgbox_maxtcol(T_MSGBOX *mbox)
{
    T_U32 maxtcol;

    maxtcol = (mbox->res.MsgBkImgRct.width - mbox->res.IconImgRct.width - mbox->frameWidth*2) / g_Font.CWIDTH;
    return maxtcol;
}
#endif

