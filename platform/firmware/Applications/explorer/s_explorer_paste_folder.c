
#include "Gbl_Global.h"
#ifdef SUPPORT_EXPLORER
#include "Fwl_public.h"
#include "Ctl_FileList.h"
#include "Ctl_Msgbox.h"
#include "Eng_FileManage.h"
#include "Eng_ScreenSave.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_DataConvert.h"
#include <string.h>
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

#define PASTE_FOLDER_INIT(mutex)        ((mutex) = AK_Create_Semaphore(1, AK_PRIORITY))
#define PASTE_FOLDER_LOCK(mutex)        AK_Obtain_Semaphore((mutex), AK_SUSPEND)
#define PASTE_FOLDER_UNLOCK(mutex)      AK_Release_Semaphore((mutex))
#define PASTE_FOLDER_DEINIT(mutex)      {\
		AK_Delete_Semaphore(mutex);\
		(mutex) = -1;\
}

typedef struct{
    T_MSGBOX    		msgbox;
    T_BOOL      		DeleteFlag;
    T_BOOL      		CutFlag;
    T_USTR_FILE 		SrcPath;
    T_USTR_FILE 		DestPath;
	T_USTR_FILE 		CurDelFileName;
    T_BOOL      		SameFileFlag;
	T_FS_DEL_CALLBACK  	DelCallBack;
	T_PCOPY_CTRL  		thrdCtrl;
	T_U32         		result;//0失败，1成功，2用户取消
	volatile e_FILE_COPY_STATE   States;
	volatile T_BOOL     DelStopFlg;
}T_EXPLORER_PASTE_FOLDER_PARM;

static T_EXPLORER_PASTE_FOLDER_PARM *Explorer_Paste_Folder_Parm = AK_NULL;
static T_USTR_FILE  destfile_path;
static T_hSemaphore hPasteFolderLock = -1;


T_BOOL FileMgr_CheckFileType(T_pCWSTR  pFilePath);

/*------------- BEGIN OF STATE s_explorer_paste_folder --------------------------------*/
/*------------- This statemachine deal with both pasting and deleting a folder --------*/

static T_BOOL Explorer_FileDelCallBack(T_VOID *pData, T_U16 *FileName)
{
	PASTE_FOLDER_LOCK(hPasteFolderLock);
	if (AK_NULL != FileName)
	{
		Utl_UStrCpyN(Explorer_Paste_Folder_Parm->CurDelFileName,FileName,FS_MAX_PATH_LEN);
	}
	PASTE_FOLDER_UNLOCK(hPasteFolderLock);
	
	return Explorer_Paste_Folder_Parm->DelStopFlg;
}

static T_VOID Explorer_FileDelState(e_FILE_COPY_STATE State)
{	
	PASTE_FOLDER_LOCK(hPasteFolderLock);
	Explorer_Paste_Folder_Parm->States = State;
	PASTE_FOLDER_UNLOCK(hPasteFolderLock);
	
	Fwl_Print(C3, M_EXPLORER, "@@Del State:%d",State);
}
#endif
void initexplorer_paste_folder(void)
{
#ifdef SUPPORT_EXPLORER
    Explorer_Paste_Folder_Parm = (T_EXPLORER_PASTE_FOLDER_PARM *)Fwl_Malloc(sizeof(T_EXPLORER_PASTE_FOLDER_PARM));
    AK_ASSERT_PTR_VOID(Explorer_Paste_Folder_Parm, "initexplorer_paste_folder(): malloc error");
	memset(Explorer_Paste_Folder_Parm,0,sizeof(T_EXPLORER_PASTE_FOLDER_PARM));

    ScreenSaverDisable();

	PASTE_FOLDER_INIT(hPasteFolderLock);
	
	Explorer_Paste_Folder_Parm->DelCallBack.Callback = (F_DelCallback)Explorer_FileDelCallBack;
	Explorer_Paste_Folder_Parm->DelCallBack.SetState = (F_DelSetState)Explorer_FileDelState;
	Explorer_Paste_Folder_Parm->DelStopFlg = AK_TRUE;
    Explorer_Paste_Folder_Parm->SameFileFlag = AK_FALSE;
	Explorer_Paste_Folder_Parm->thrdCtrl = AK_NULL;

	Explorer_Paste_Folder_Parm->result = 1;
	Explorer_Paste_Folder_Parm->States = eFS_COPY_OtherState;//非0，1，2
#endif
}

void exitexplorer_paste_folder(void)
{
#ifdef SUPPORT_EXPLORER
	Explorer_Paste_Folder_Parm->DelStopFlg = AK_FALSE;
	
	if ((AK_TRUE == Explorer_Paste_Folder_Parm->DeleteFlag) && (eFS_COPY_ING == Explorer_Paste_Folder_Parm->States))
	{
		/*SW10A00002224 如果没有这个等待，后台线程函数还没有退出就去KILL线程，会死机*/
		while(1) 
		{
			AK_Sleep(40);
			if (eFS_COPY_ING != Explorer_Paste_Folder_Parm->States)
				break;
		}
	}
	
	if (Explorer_Paste_Folder_Parm->CutFlag == AK_TRUE)
        FileMgr_SetManageState(FILE_NULL);

    ScreenSaverEnable();

	PASTE_FOLDER_LOCK(hPasteFolderLock);
	if (AK_NULL != Explorer_Paste_Folder_Parm->thrdCtrl)
	{
		Fwl_KillCopyThead(&Explorer_Paste_Folder_Parm->thrdCtrl);
	}
	PASTE_FOLDER_UNLOCK(hPasteFolderLock);

	Fwl_Free(Explorer_Paste_Folder_Parm);
	Explorer_Paste_Folder_Parm = AK_NULL;
	
	PASTE_FOLDER_DEINIT(hPasteFolderLock);
#endif
}

void paintexplorer_paste_folder(void)
{
#ifdef SUPPORT_EXPLORER
	T_USTR_FILE     utmpstr;

	PASTE_FOLDER_LOCK(hPasteFolderLock);
	Utl_UStrCpy(utmpstr, GetCustomString(csFILE_DELETING));
	Utl_UStrCat(utmpstr, _T(" "));
	Utl_UStrCat(utmpstr, Explorer_Paste_Folder_Parm->CurDelFileName);
	MsgBox_InitStr(&Explorer_Paste_Folder_Parm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_INFORMATION|MSGBOX_EXIT);
	MsgBox_Show(&Explorer_Paste_Folder_Parm->msgbox);
	PASTE_FOLDER_UNLOCK(hPasteFolderLock);
	
	Fwl_RefreshDisplay();
#endif
}

unsigned char handleexplorer_paste_folder(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_EXPLORER

	T_hFILESTAT     find;
	T_USTR_FILE     tmppath,foucspath;
    T_USTR_FILE     subpath;
	T_USTR_FILE		FindPath;
    T_eBACK_STATE               msgRet;
    T_pRECT         pMBoxRct = AK_NULL;
	T_BOOL          ret = AK_FALSE;

    subpath[0] = 0;
	
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    switch(event)
    {
        case M_EVT_PASTE_FOLDER:                //start folder paste
        case M_EVT_DELETE_FOLDER:               //start delete a folder

            MsgBox_InitAfx(&Explorer_Paste_Folder_Parm->msgbox,0, ctHINT, csWAITING, MSGBOX_INFORMATION);
            MsgBox_Show(&Explorer_Paste_Folder_Parm->msgbox);
            
            Fwl_InvalidateRect( pMBoxRct->left, pMBoxRct->top, pMBoxRct->width, pMBoxRct->height);
			
            /***取得拷贝或删除的文件相关信息***/
            if (event == M_EVT_DELETE_FOLDER)
            {
				FileMgr_GetDeletePath(Explorer_Paste_Folder_Parm->SrcPath);
                Explorer_Paste_Folder_Parm->DeleteFlag = AK_TRUE;
                Explorer_Paste_Folder_Parm->CutFlag = AK_FALSE;
            }
            else
            {
                FileMgr_GetSrcPath(Explorer_Paste_Folder_Parm->SrcPath);
                FileMgr_GetDestPath(Explorer_Paste_Folder_Parm->DestPath);
                Explorer_Paste_Folder_Parm->CutFlag = FileMgr_GetCutFlag();
                Explorer_Paste_Folder_Parm->DeleteFlag = AK_FALSE;
            }

			if(Explorer_Paste_Folder_Parm->DeleteFlag)
			{
				ret = Fwl_FileDel_CallBack(Explorer_Paste_Folder_Parm->SrcPath,Explorer_Paste_Folder_Parm->DelCallBack,\
					AK_NULL,&Explorer_Paste_Folder_Parm->thrdCtrl);

				Fwl_Print(C3, M_EXPLORER, "ret:%d",ret);
				if (!ret)
				{
					Fwl_Print(C1, M_EXPLORER, "Exlporer Del File Fail,return!!");

					Explorer_Paste_Folder_Parm->result = 0;
					Explorer_Paste_Folder_Parm->States = eFS_COPY_Fail; //设置状态，退出本状态机
				}
			}
        	else //若为拷贝目录的操作，则在目标目录创建好第一个空目录
            {
                Utl_UStrCpy(destfile_path, Explorer_Paste_Folder_Parm->DestPath);
                Utl_UStrCpy(foucspath, Explorer_Paste_Folder_Parm->SrcPath);
                if(Fwl_FsIsDir(foucspath))
                {
                    FileMgr_GetSubPath(Explorer_Paste_Folder_Parm->SrcPath, foucspath, subpath);
                    Utl_UStrCpy(tmppath,subpath);
                    if (Utl_UStrLen(destfile_path) + Utl_UStrLen(subpath) > (MAX_FILENM_LEN - FILE_LEN_RESERVE2))
                    {
                        MsgBox_InitAfx(&Explorer_Paste_Folder_Parm->msgbox,3, ctHINT, csFILENAME_LONG, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&Explorer_Paste_Folder_Parm->msgbox, MSGBOX_DELAY_0);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&Explorer_Paste_Folder_Parm->msgbox);
                        break;
                    }

                    Utl_UStrCat(destfile_path, tmppath);
                    if (!FileMgr_CheckFileIsExist(destfile_path))
                    {
						if (Fwl_FsMkDir(destfile_path) == AK_FALSE)
	                    {
	                        MsgBox_InitAfx(&Explorer_Paste_Folder_Parm->msgbox,3, ctHINT, csFILE_MKDIR_FAILURE, MSGBOX_INFORMATION);
	                        MsgBox_SetDelay(&Explorer_Paste_Folder_Parm->msgbox, MSGBOX_DELAY_0);
	                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&Explorer_Paste_Folder_Parm->msgbox);
	                        break;
	                    }
                    }
					else //folder is exist,will to set same file.
					{
						Explorer_Paste_Folder_Parm->SameFileFlag = AK_TRUE;
					}
					
					Utl_UStrCpy(FindPath,Explorer_Paste_Folder_Parm->SrcPath);
					Utl_UStrCat(FindPath, _T("/*.*"));
					
					if (FS_INVALID_STATHANDLE != (find=Fwl_FsFindFirst(FindPath)))
					{
						Fwl_FsFindClose(find);						
						pEventParm->p.pParam1 = Explorer_Paste_Folder_Parm->SrcPath;
						pEventParm->p.pParam2 = Explorer_Paste_Folder_Parm->DestPath;
						pEventParm->p.pParam3 = (vPVOID)Explorer_Paste_Folder_Parm->SameFileFlag;
						m_triggerEvent(M_EVT_PASTE_FILE, pEventParm);
					}
					else
					{
						Fwl_FsFindClose(find);						
						if (Explorer_Paste_Folder_Parm->CutFlag)
							Fwl_FileDelete(Explorer_Paste_Folder_Parm->SrcPath);
						
						MsgBox_InitAfx(&Explorer_Paste_Folder_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
										
						MsgBox_SetDelay(&Explorer_Paste_Folder_Parm->msgbox, MSGBOX_DELAY_1);
						m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&Explorer_Paste_Folder_Parm->msgbox);
					}
                }
            }
			
            break;
			
		case M_EVT_USER_KEY:
		case M_EVT_TOUCH_SCREEN:
			
			msgRet = MsgBox_Handler(&Explorer_Paste_Folder_Parm->msgbox, event, pEventParm);
	        switch(msgRet)
	        {
	            case eNext:
				case eHome:
				case eReturn:
					Explorer_Paste_Folder_Parm->result = 2;
					Explorer_Paste_Folder_Parm->DelStopFlg = AK_FALSE;
	                break;
	            default:
	                break;
	        }
			break;
			
		case SYS_EVT_USB_PLUG: // when USB/SD/MMC is in
		case SYS_EVT_SD_PLUG:	
			if ((pEventParm->c.Param8 == EVT_USB_PLUG_IN) ||(pEventParm->c.Param1 == EVT_SD_PLUG_IN))
			{
				Explorer_Paste_Folder_Parm->DelStopFlg = AK_FALSE;
			}
			break;
			
		case VME_EVT_TIMER:
			break;

        default:
            break;
    }

	//一定要保证是删除回调退出后才调下面的。
	if (AK_TRUE == Explorer_Paste_Folder_Parm->DeleteFlag \
		&& eFS_COPY_ING != Explorer_Paste_Folder_Parm->States \
		&& eFS_COPY_OtherState != Explorer_Paste_Folder_Parm->States)
	{		
		if(eFS_COPY_Fail == Explorer_Paste_Folder_Parm->States)
			Explorer_Paste_Folder_Parm->result = 0;
		
		pEventParm->c.Param1 = (T_U8)Explorer_Paste_Folder_Parm->result;
		m_triggerEvent(M_EVT_DEL_EXIT,pEventParm);
	}
#endif
    return 0;
}



