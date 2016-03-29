
#include "Fwl_public.h"
#ifdef SUPPORT_EXPLORER

#include "Ctl_DisplayList.h"
#include "Fwl_Initialize.h"
#include "Eng_ImgConvert.h"
#include "Ctl_Msgbox.h"
#include "Eng_FileManage.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfkeypad.h"
#include "Ctl_AVIPlayer.h"
#include "Eng_DataConvert.h"
#include "fwl_oscom.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"
#include "Eng_Math.h"
#include "Ctl_APlayerList.h"
#include "Eng_time.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_DISPLAYLIST   *pDisplayList;
    T_MSGBOX        msgbox;
    T_BOOL          MsgFlag;
    T_BOOL          SameFileFlag;
    T_BOOL          ListRefreshFlag;
} T_EXPLORER_MENU_PARM;

static T_EXPLORER_MENU_PARM *pExplorerMenuParm;

extern T_BOOL AVIPlayer_IsSupportFileType(T_pCWSTR pFileName);
#ifdef SUPPORT_VIDEOPLAYER
extern T_BOOL AVIList_Add(T_pCWSTR pFilePath, T_BOOL SearchSub);
#endif
//extern T_FILELIST_RET Mp3_FileListAdd(T_STR_FILE FilePath, T_FILE_INFO FileInfo, T_BOOL SubFolder);
static T_BOOL ExplorerMenu_ShowFileInfo(T_DISPLAYLIST   *pDisplayList);
T_VOID Date2Str(T_U32 datetime, T_S8 *string);
#endif
/*---------------------- BEGIN OF STATE s_set_explorer_menu ------------------------*/
void initset_explorer_menu(void)
{
#ifdef SUPPORT_EXPLORER
    pExplorerMenuParm = (T_EXPLORER_MENU_PARM *)Fwl_Malloc(sizeof(T_EXPLORER_MENU_PARM));
    AK_ASSERT_PTR_VOID(pExplorerMenuParm, "initset_explorer_menu(): malloc error");

    MenuStructInit(&pExplorerMenuParm->IconExplorer);
    GetMenuStructContent(&pExplorerMenuParm->IconExplorer, mnEXPLORER_MENU);

    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    pExplorerMenuParm->MsgFlag = AK_FALSE;
    pExplorerMenuParm->ListRefreshFlag = AK_FALSE;
#endif
}

void exitset_explorer_menu(void)
{
#ifdef SUPPORT_EXPLORER
    if ((pExplorerMenuParm->pDisplayList != AK_NULL) && (AK_TRUE == pExplorerMenuParm->ListRefreshFlag))
        DisplayList_ListRefresh(pExplorerMenuParm->pDisplayList);
    IconExplorer_Free(&pExplorerMenuParm->IconExplorer);
    pExplorerMenuParm = Fwl_Free(pExplorerMenuParm);
#endif
}

void paintset_explorer_menu(void)
{
#ifdef SUPPORT_EXPLORER
    if (pExplorerMenuParm->MsgFlag == AK_TRUE)
        MsgBox_Show(&pExplorerMenuParm->msgbox);
    else
    {
        IconExplorer_Show(&pExplorerMenuParm->IconExplorer);
        GE_StartShade();
    }
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_explorer_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_EXPLORER

    T_eBACK_STATE IconExplorerRet, msgRet;
    T_FILE_INFO *FileInfo;
    T_BOOL SubFolder = AK_FALSE;
    T_BOOL addRet = AK_FALSE;

    T_USTR_FILE FilePath;
    T_USTR_FILE FilePathTmp;
    T_U32       strlen;
    T_USTR_FILE src_path,dest_path;
    T_USTR_FILE file_name;
    T_U16       menu_id;
    T_FILE_MANAGE_RETURN_VALUE  RET_VALUE;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pExplorerMenuParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_NEXT)
    {
        pExplorerMenuParm->pDisplayList = (T_DISPLAYLIST *)pEventParm;
        FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);
        if (FileInfo != AK_NULL)
        {
            Utl_UStrCpy(FilePathTmp, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
            Utl_UStrCat(FilePathTmp, FileInfo->name);
            if ((FileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER)
            {
                IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 50);
                IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 80);
                
                if(AudioPlayer_IsSupportFile(FilePathTmp))//audio
                {
                    if( Utl_GetFileType(FilePathTmp)==FILE_TYPE_MP4 )
                    {

                        if( AVIPlayer_CheckVideoFile(FilePathTmp)==AVIPLAY_OK )
                        {
                            Fwl_Print(C3, M_EXPLORER, "VIDEO MP4 FILE!!\n");
                            IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 10);
                        }
                        else
                        {
                            Fwl_Print(C3, M_EXPLORER, "AUDIO MP4 FILE!!\n");
                            IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 20);
                        }
                    }
                    else
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 20);
                }
                else if(AVIPlayer_IsSupportFileType(FilePathTmp))//video
                {
                    IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 10);
                }
                else//other
                {
                    IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 10);
                    IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 20);
                }            
            }
            else
            {
                if (Utl_UStrCmp(FileInfo->name, _T(".")) == 0)
                {
                    IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 30);
                    if (DisplayList_GetSubLevel(pExplorerMenuParm->pDisplayList) == 1 )
                    {
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 10);
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 20);
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 40);
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 50);
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 60);
                        IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 70);
                    }
                }
                IconExplorer_DelItem(&pExplorerMenuParm->IconExplorer, 90);
            }
        }
        pExplorerMenuParm->MsgFlag = AK_FALSE;

    }

    if  ((event == M_EVT_RETURN3) || (event == M_EVT_RETURN2))
    {
		if (AK_NULL != pExplorerMenuParm)
		{
			IconExplorer_Show(&pExplorerMenuParm->IconExplorer);
       		Fwl_RefreshDisplay();
		}
		m_triggerEvent(M_EVT_EXIT, pEventParm);
        return 0;
    }

    if (pExplorerMenuParm->MsgFlag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pExplorerMenuParm->msgbox, event, pEventParm);
        switch (msgRet)
        {
        case eNext:
            //deal with the msgbox !
            if(pExplorerMenuParm->SameFileFlag == AK_TRUE)
            {
                pExplorerMenuParm->ListRefreshFlag = AK_TRUE;

                if(FileMgr_GetFolderFlag() == AK_TRUE)
               	{
                    m_triggerEvent(M_EVT_PASTE_FOLDER, pEventParm);
                }
                else
                {
                	pEventParm->p.pParam3 = (vPVOID)pExplorerMenuParm->SameFileFlag;
                    m_triggerEvent(M_EVT_PASTE_FILE, pEventParm);
               	}

                pExplorerMenuParm->SameFileFlag = AK_FALSE;
                pExplorerMenuParm->MsgFlag = AK_FALSE;
                break;
            }
            else
            {
                SubFolder = AK_TRUE;
            }
            
        case eReturn:
            if(pExplorerMenuParm->SameFileFlag != AK_TRUE)
            {
                pExplorerMenuParm->MsgFlag = AK_FALSE;

                if ((M_EVT_USER_KEY == event ) && ((T_eKEY_ID)pEventParm->c.Param1 == kbCLEAR))
                {
                    m_triggerEvent(M_EVT_EXIT, pEventParm);
                    break;
                }

                MsgBox_InitStr(&pExplorerMenuParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
                MsgBox_Show(&pExplorerMenuParm->msgbox);
                Fwl_RefreshDisplay();

                if (M_EVT_TOUCH_SCREEN == event)
                {
                    event = M_EVT_USER_KEY;
                    //触摸屏下按消息框的'否',应该实现 添加当前目录但不加子目录的功能。
                    if (pEventParm->c.Param1 == kbCLEAR)
                    {
                        pEventParm->c.Param1 = kbOK;
                    }
                }

                FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);
                if (FileInfo != AK_NULL)
                {
                    Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
                    
                    IconExplorerRet = IconExplorer_Handler(&pExplorerMenuParm->IconExplorer, event, pEventParm);
                    switch (IconExplorerRet)
                    {
                        case eNext:
                            menu_id = (T_U16)IconExplorer_GetItemFocusId(&pExplorerMenuParm->IconExplorer);
                            switch (menu_id)
                            {
                            case 10:
                                Utl_UStrCat(FilePath, FileInfo->name);
                                addRet = AudioPlayer_Add(FilePath, SubFolder);
                                break;
#ifdef SUPPORT_VIDEOPLAYER
                            case 20:
                                Utl_UStrCat(FilePath, FileInfo->name);
                                addRet = AVIList_Add(FilePath, SubFolder);
                                break;
#endif
                            }
                        break;

                        default:
                        break;
                    }
                }

                if (addRet)
                {
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_0);
                }
                else
                {
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                }
                m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                break;
            }
            else
            {
                pExplorerMenuParm->MsgFlag = AK_FALSE;
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                break;
            }
        default:
            break;
        }
    }
    else
    {
        IconExplorerRet = IconExplorer_Handler(&pExplorerMenuParm->IconExplorer, event, pEventParm);
        switch (IconExplorerRet)
        {
        case eNext:
            menu_id = (T_U16)IconExplorer_GetItemFocusId(&pExplorerMenuParm->IconExplorer);
            switch (menu_id)
            {
            case 10:
                FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);
                Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
                Utl_UStrCat(FilePath, FileInfo->name);

                if ((FileInfo == AK_NULL) || (!(FileMgr_CheckFileIsExist(FilePath))))
                {
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                    break;
                }

                if ((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER)
                {
                    pExplorerMenuParm->MsgFlag = AK_TRUE;
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
                }
                else
                {
                    MsgBox_InitStr(&pExplorerMenuParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
                    MsgBox_Show(&pExplorerMenuParm->msgbox);
                    Fwl_RefreshDisplay();

                    addRet = AudioPlayer_Add(FilePath, AK_FALSE);

                    if (addRet)
                    {
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_0);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                    }
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                }
                break;
#ifdef SUPPORT_VIDEOPLAYER
            case 20:
                FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);
                Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
                Utl_UStrCat(FilePath, FileInfo->name);

                if ((FileInfo == AK_NULL) || (!(FileMgr_CheckFileIsExist(FilePath))))
                {
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                    break;
                }
                if ((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER)
                {
                    pExplorerMenuParm->MsgFlag = AK_TRUE;
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 0, ctHINT, csMP3_ADD_LIST_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
                }
                else
                {
                    MsgBox_InitStr(&pExplorerMenuParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
                    MsgBox_Show(&pExplorerMenuParm->msgbox);
                    Fwl_RefreshDisplay();

                    addRet = AVIList_Add(FilePath, AK_FALSE);

                    if (addRet)
                    {
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_0);
                    }
                    else
                    {
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                    }
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                }
                break;
#endif
            case 30:
                m_triggerEvent(M_EVT_1, (T_EVT_PARAM *)pExplorerMenuParm->pDisplayList);
                break;
            case 40:
                m_triggerEvent(M_EVT_2, (T_EVT_PARAM *)pExplorerMenuParm->pDisplayList);
                break;
            case 50:
                m_triggerEvent(M_EVT_SETPATH, (T_EVT_PARAM *)pExplorerMenuParm->pDisplayList);
                break;
            case 60:
            case 70:
                FileMgr_InitFileInfo();
                FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);

                Utl_UStrCpy(src_path, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
                Utl_UStrCat(src_path, FileInfo->name);
                if (menu_id == 60)
                    FileMgr_SaveSrcFileInfo(src_path,AK_FALSE);
                else
                    FileMgr_SaveSrcFileInfo(src_path,AK_TRUE);

                m_triggerEvent(M_EVT_EXIT, pEventParm);
                break;

            case 80:
            //after "paste menu be pressed"
                FileInfo = DisplayList_GetItemContentFocus(pExplorerMenuParm->pDisplayList);

                Utl_UStrCpy(dest_path, DisplayList_GetCurFilePath(pExplorerMenuParm->pDisplayList));
                FileMgr_GetSrcPath(src_path);
                FileMgr_GetFileName(file_name);

                if (Utl_UStrCmp(FileInfo->name, _T(".")) != 0)
                {
                    Utl_UStrCat(dest_path, FileInfo->name);
                    strlen = Utl_UStrLen(dest_path);
                    dest_path[strlen] = UNICODE_SOLIDUS;
                    dest_path[strlen + 1] = 0;
                }

                if (Utl_UStrLen(dest_path) + Utl_UStrLen(file_name)< MAX_FILENM_LEN - FILE_LEN_RESERVE2)
                {
                    Utl_UStrCat(dest_path, file_name);
                }
                else
                {
                    MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csFILENAME_LONG, MSGBOX_INFORMATION | MSGBOX_OK);
                    MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                    return 0;
                }

                FileMgr_SaveDestFileInfo(dest_path);

                //pre_handle before starting paste ,and get a return value;
                RET_VALUE = FileMgr_Paste_Pre_Handle(src_path,dest_path);

                switch (RET_VALUE)
                {
                    case PASTE_FOLDER:
                        pExplorerMenuParm->ListRefreshFlag = AK_TRUE;
                        m_triggerEvent(M_EVT_PASTE_FOLDER, pEventParm);
                        break;
                    case PASTE_FILE:
                        pExplorerMenuParm->ListRefreshFlag = AK_TRUE;
                        pEventParm->p.pParam3 = (vPVOID)pExplorerMenuParm->SameFileFlag;
                        m_triggerEvent(M_EVT_PASTE_FILE, pEventParm);
                        break;
                    case MOVE_FILE:
                        pExplorerMenuParm->ListRefreshFlag = AK_TRUE;
                        pEventParm->p.pParam3 = (vPVOID)pExplorerMenuParm->SameFileFlag;
                        m_triggerEvent(M_EVT_PASTE_FILE, pEventParm);
                        break;
                    case PASTE_BOARD_NULL:
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csEXPLORER_NOFILE, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                        break;
                    case LACK_DISKSPACE:
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csEXPLORER_FREE_SIZE, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                        break;
                    case FILE_SAME_NAME:
                        pExplorerMenuParm->MsgFlag = AK_TRUE;
                        pExplorerMenuParm->SameFileFlag = AK_TRUE;
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox, 0, ctHINT, csEXPLORER_SAMEFILE, MSGBOX_QUESTION | MSGBOX_YESNO);
                        break;
                    case FILE_ERROR:
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                       break;
                    case FILE_BUSY:         //提示信息需要变化一下：文件播放中
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                        break;
                    case FILE_SAME_ADDR:
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csEXPLORER_SAME_PATH, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                        break;
                    case FILE_LONG_NAME:
                        MsgBox_InitAfx(&pExplorerMenuParm->msgbox,2, ctHINT, csFILENAME_LONG, MSGBOX_INFORMATION | MSGBOX_OK);
                        MsgBox_SetDelay(&pExplorerMenuParm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
                    default:
                        //m_triggerEvent, return the pre_state;
                        break;
                }
                break;
                case 90:
                ExplorerMenu_ShowFileInfo(pExplorerMenuParm->pDisplayList);
                break;
            default:
                break;
            }
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
        }
    }
#endif
    return 0;
}



#ifdef SUPPORT_EXPLORER
static T_BOOL ExplorerMenu_ShowFileInfo(T_DISPLAYLIST   *pDisplayList)
{
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_STR_INFO  tmpstr, timestr;
    T_USTR_FILE Utmpstr, Utmpstr2;

    AK_ASSERT_PTR(pDisplayList, "ExplorerMenu_ShowFileInfo(): pDisplayList", AK_FALSE);

    pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
    if (AK_NULL != pFileInfo && 0x10 != (pFileInfo->attrib&0x10))
    {
        memset(Utmpstr, 0, sizeof(T_USTR_FILE));
		memset(Utmpstr2, 0, sizeof(T_USTR_FILE));
		
        MsgBox_InitStr(&pExplorerMenuParm->msgbox, 1, GetCustomString(csEXPLORER_FILE_INFO), Utmpstr, MSGBOX_INFORMATION | MSGBOX_OK);

        Utl_UStrCpy(Utmpstr, DisplayList_GetCurFilePath(pDisplayList));
        Utl_UStrCat(Utmpstr, pFileInfo->name);

        Utl_UStrCpy(Utmpstr2, GetCustomString(csEXPLORER_PATH));
        Utl_UStrCat(Utmpstr2, Utmpstr);
        MsgBox_AddLine(&pExplorerMenuParm->msgbox, Utmpstr2);

		memset(tmpstr, 0, sizeof(T_STR_INFO));
		memset(Utmpstr, 0, sizeof(T_USTR_FILE));
		memset(Utmpstr2, 0, sizeof(T_USTR_FILE));

		Utl_UStrCpy(Utmpstr2, GetCustomString(csEXPLORER_SIZE));
		//sprintf(tmpstr, "%lu ", pFileInfo->size.low); 
		U64_Int2Str(tmpstr, pFileInfo->size.high, pFileInfo->size.low);
		Eng_StrMbcs2Ucs(tmpstr, Utmpstr);
		Utl_UStrCat(Utmpstr2, Utmpstr);
		Utl_UStrCat(Utmpstr2, GetCustomString(csEXPLORER_UNIT));
        MsgBox_AddLine(&pExplorerMenuParm->msgbox, Utmpstr2);


		memset(timestr, 0, sizeof(T_STR_INFO));
		memset(Utmpstr, 0, sizeof(T_USTR_FILE));
		memset(Utmpstr2, 0, sizeof(T_USTR_FILE));
		
        Date2Str(pFileInfo->time_write, timestr);

        Utl_UStrCpy(Utmpstr2, GetCustomString(csEXPLORER_WRITETIME));
        Eng_StrMbcs2Ucs(timestr, Utmpstr);
        Utl_UStrCat(Utmpstr2, Utmpstr);
        MsgBox_AddLine(&pExplorerMenuParm->msgbox, Utmpstr2);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorerMenuParm->msgbox);
    }

    return AK_TRUE;
}


#endif
