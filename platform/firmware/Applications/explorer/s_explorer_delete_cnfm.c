
#include "Fwl_public.h"
#ifdef SUPPORT_EXPLORER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Eng_FileManage.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_DataConvert.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST   *pDisplayList;
    T_MSGBOX        msgbox;
	T_BOOL			fromFontOpt;
} T_EXPLORER_DELETE_PARM;

static T_EXPLORER_DELETE_PARM *pExplorer_Delete_Parm;

T_BOOL AudioPlayer_IsPlayingFile(T_pCWSTR Filepath);
#endif
/*---------------------- BEGIN OF STATE s_explorer_delete_cnfm ------------------------*/
void initexplorer_delete_cnfm(void)
{
#ifdef SUPPORT_EXPLORER
    pExplorer_Delete_Parm = (T_EXPLORER_DELETE_PARM *)Fwl_Malloc(sizeof(T_EXPLORER_DELETE_PARM));
    AK_ASSERT_PTR_VOID(pExplorer_Delete_Parm, "initexplorer_delete_cnfm(): malloc error");
	
    MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, 0, ctHINT, csEXPLORER_DELETE_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
	pExplorer_Delete_Parm->fromFontOpt = AK_FALSE;
#endif
}

void exitexplorer_delete_cnfm(void)
{
#ifdef SUPPORT_EXPLORER
    pExplorer_Delete_Parm = Fwl_Free(pExplorer_Delete_Parm);
#endif
}

void paintexplorer_delete_cnfm(void)
{
#ifdef SUPPORT_EXPLORER
    MsgBox_Show(&pExplorer_Delete_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleexplorer_delete_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_EXPLORER
    T_eBACK_STATE   menuRet;
    T_U8            result;
    T_FILE_INFO     *FileInfo;
    T_pWSTR         Path;
    T_USTR_FILE     FilePath;
    T_USTR_FILE     SystemPath;
	T_U16 			retLevel;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pExplorer_Delete_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
    case M_EVT_1:
        pExplorer_Delete_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm;
        break;
    case M_EVT_DEL_EXIT:
		if (pExplorer_Delete_Parm->fromFontOpt)
		{
			retLevel = 2;
		}
		else
		{
			retLevel = 2;
		}
		
        result = pEventParm->c.Param1;
        if (result)
        {

            DisplayList_DelFocusItem(pExplorer_Delete_Parm->pDisplayList);
            if (result == 2)
                MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, retLevel, ctHINT, csFILE_DELETE_CANCLE, MSGBOX_INFORMATION);
            else
                MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, retLevel, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        }
        else
        {
            MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, retLevel, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        }
        MsgBox_SetDelay(&pExplorer_Delete_Parm->msgbox, MSGBOX_DELAY_0);
        DisplayList_ListRefresh(pExplorer_Delete_Parm->pDisplayList);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorer_Delete_Parm->msgbox);
        break;
    default:
        break;
    }

    menuRet = MsgBox_Handler(&pExplorer_Delete_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
    case eNext:
        FileInfo = DisplayList_GetItemContentFocus(pExplorer_Delete_Parm->pDisplayList);
        Path = DisplayList_GetCurFilePath(pExplorer_Delete_Parm->pDisplayList);
        Utl_UStrCpy(FilePath,Path);
        Utl_UStrCpy(SystemPath,Path);
        Utl_UStrCat(FilePath, FileInfo->name);

        if (Fwl_FsIsDir(FilePath))
        {
        	Utl_UStrCat(SystemPath, FileInfo->name);
        	Utl_UStrCat(SystemPath, _T("/"));
        }

        if ((FileInfo == AK_NULL) \
                || (((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER) \
                && ((Utl_UStrCmp(FileInfo->name, _T(".")) == 0) \
                || (Utl_UStrCmp(FileInfo->name, _T("..")) == 0))) || \
                (AudioPlayer_IsPlayingFile(FilePath) == AK_TRUE))
        {
            MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pExplorer_Delete_Parm->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorer_Delete_Parm->msgbox);
        }
        else if ((Utl_UStrCmp(SystemPath, _T(SYSTEM_PATH)) == 0))
        {
        	MsgBox_InitAfx(&pExplorer_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pExplorer_Delete_Parm->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pExplorer_Delete_Parm->msgbox);
        }
        else
        {
            if (!Fwl_FsIsDir(FilePath))
            {
                pEventParm->p.pParam1 = DisplayList_GetCurFilePath(pExplorer_Delete_Parm->pDisplayList);;
                pEventParm->p.pParam2 = FileInfo;
                m_triggerEvent(M_EVT_NEXT, pEventParm);
            }
            else
            {
                //FileMgr_InitFileInfo();
                //FileMgr_SaveSrcFileInfo(FilePath,AK_FALSE);
                if (FILE_NULL == FileMgr_GetManageState())
                    FileMgr_InitFileInfo();
                FileMgr_SetDeletetPath(FilePath);
                m_triggerEvent(M_EVT_DELETE_FOLDER, pEventParm);
            }
        }
        break;
    default:
        ReturnDefauleProc(menuRet, pEventParm);
        break;
    }
#endif
    return 0;
}
