
#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Eng_FileManage.h"
#include "Eng_DataConvert.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST           *pDisplayList;
    T_MSGBOX                msgbox;
    T_ICONEXPLORER          IconExplorer;
    T_USTR_FILE             setPath;
    T_BOOL                  MsgFlag;
} T_SET_PATH_PARM;

static T_SET_PATH_PARM *pSet_Path_Parm;

T_VOID SetPath_GetFilePath(T_U16 *pFilePath, const T_U16 *pCurPath, const T_U16 *pFolderName);

/*---------------------- BEGIN OF STATE s_set_default_path ------------------------*/
void initset_default_path(void)
{
    pSet_Path_Parm = (T_SET_PATH_PARM *)Fwl_Malloc(sizeof(T_SET_PATH_PARM));
    AK_ASSERT_PTR_VOID(pSet_Path_Parm, "initset_default_path(): malloc error");

    MenuStructInit(&pSet_Path_Parm->IconExplorer);
    GetMenuStructContent(&pSet_Path_Parm->IconExplorer, mnSET_PATH_MENU);

    pSet_Path_Parm->MsgFlag = AK_FALSE;
}

void exitset_default_path(void)
{
    IconExplorer_Free(&pSet_Path_Parm->IconExplorer);
    pSet_Path_Parm = Fwl_Free(pSet_Path_Parm);
}


void paintset_default_path(void)
{
    if (pSet_Path_Parm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pSet_Path_Parm->msgbox);
    }
    else
    {
        IconExplorer_Show(&pSet_Path_Parm->IconExplorer);
        GE_StartShade();
    }
    
    Fwl_RefreshDisplay();
}

unsigned char handleset_default_path(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE   IconExplorerRet, msgRet;
    T_U16           *pCurPath = AK_NULL;
    T_USTR_INFO     utmpstr, ustr_null;
    T_FILE_INFO     *pFileInfo = AK_NULL;
    T_DEFPATH_TYPE  PathType;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pSet_Path_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_SETPATH)
    {
        pSet_Path_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm;
        IconExplorer_DelItem(&pSet_Path_Parm->IconExplorer, 60);
    }

    if (pSet_Path_Parm->MsgFlag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pSet_Path_Parm->msgbox, event, pEventParm);
        switch (msgRet)
        {
        case eNext:			
			PathType = (T_DEFPATH_TYPE)IconExplorer_GetItemFocusId(&pSet_Path_Parm->IconExplorer);
#ifdef CAMERA_SUPPORT
			if (!(0 == Utl_UStrCmp(pSet_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH))
				 && (eVIDEOREC_PATH == PathType)))
#else
			if (!(0 == Utl_UStrCmp(pSet_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH))))
#endif
			{  
	            if (Fwl_SetDefPath(PathType, pSet_Path_Parm->setPath))
	            {
		            MsgBox_InitAfx(&pSet_Path_Parm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
	            }
				else
				{
					MsgBox_InitAfx(&pSet_Path_Parm->msgbox, 2, ctHINT, csFAILURE_DONE, MSGBOX_INFORMATION);
				}
				
	            MsgBox_SetDelay(&pSet_Path_Parm->msgbox, MSGBOX_DELAY_0);
	            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pSet_Path_Parm->msgbox);
			}
        case eReturn:
            pSet_Path_Parm->MsgFlag = AK_FALSE;
            IconExplorer_SetRefresh(&pSet_Path_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
            break;
        default:
            break;
        }
    }
    else
    {
        IconExplorerRet = IconExplorer_Handler(&pSet_Path_Parm->IconExplorer, event, pEventParm);
        switch (IconExplorerRet)
        {
        case eNext:
            pCurPath = DisplayList_GetCurFilePath(pSet_Path_Parm->pDisplayList);
            pFileInfo = DisplayList_GetItemContentFocus(pSet_Path_Parm->pDisplayList);
            if (pCurPath && pFileInfo && (pFileInfo->attrib&0x10) == 0x10)
            {
                SetPath_GetFilePath(pSet_Path_Parm->setPath, pCurPath, pFileInfo->name);

                if (Utl_UStrLen(pSet_Path_Parm->setPath) > (MAX_FILENM_LEN - FILE_LEN_RESERVE1))
                {
                    MsgBox_InitAfx(&pSet_Path_Parm->msgbox, 3, ctHINT, csFILENAME_LONG, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pSet_Path_Parm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE,(T_EVT_PARAM *)&pSet_Path_Parm->msgbox);
                    break;
                }

                Utl_UStrCpy(utmpstr,pSet_Path_Parm->setPath);
                utmpstr[Utl_UStrLen(utmpstr) - 1] = 0;
                if (!FileMgr_CheckFileIsExist(utmpstr))
                {
                    MsgBox_InitAfx(&pSet_Path_Parm->msgbox, 3, ctHINT, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pSet_Path_Parm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE,(T_EVT_PARAM *)&pSet_Path_Parm->msgbox);
                    break;
                }

                pSet_Path_Parm->MsgFlag = AK_TRUE;

                Utl_UStrCpy(utmpstr, GetCustomString(csSET_DEF_PATH_SET));
                Utl_UStrCat(utmpstr, _T("\""));
                
                Utl_UStrCat(utmpstr, pSet_Path_Parm->setPath);

                ustr_null[0] = 0;
                switch (IconExplorer_GetItemFocusId(&pSet_Path_Parm->IconExplorer))
                {
#ifdef SUPPORT_VIDEOPLAYER
                    case eVIDEO_PATH:
                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
                        Utl_UStrCat(utmpstr, _T("\""));
                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_MOVIE_DEF_PATH));
                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
                        break;
#endif
#ifdef SUPPORT_AUDIOPLAYER
                    case eAUDIO_PATH:
                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
                        Utl_UStrCat(utmpstr, _T("\""));
                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_MUSIC_DEF_PATH));
                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
                        break;
#endif
#ifdef SUPPORT_IMG_BROWSE
                    case eIMAGE_PATH:
                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
                        Utl_UStrCat(utmpstr, _T("\""));
                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_PICTURE_DEF_PATH));
                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
                        break;
#endif
#ifdef CAMERA_SUPPORT
                    case eIMAGEREC_PATH:
                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
                        Utl_UStrCat(utmpstr, _T("\""));
                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_CAPTURE_DEF_PATH));
                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
                        break;
                    case eVIDEOREC_PATH:
						Printf_UC(pSet_Path_Parm->setPath);
						Printf_UC(Fwl_GetDefPath(eRECIDX_PATH));
						if (0 != Utl_UStrCmp(pSet_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH)))
                        {
	                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
	                        Utl_UStrCat(utmpstr, _T("\""));
	                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_VEDIOREC_DEF_PATH));
	                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
						}
						else
						{
							MsgBox_InitAfx(&pSet_Path_Parm->msgbox, 2, ctHINT, csCAMERA_FOLDER_TABOO, MSGBOX_INFORMATION);
							MsgBox_SetDelay(&pSet_Path_Parm->msgbox, MSGBOX_DELAY_0);
						}
                        break;
#endif
#ifdef SUPPORT_AUDIOREC
                    case eAUDIOREC_PATH:
                        MsgBox_InitStr(&pSet_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), ustr_null, MSGBOX_QUESTION | MSGBOX_YESNO);
                        Utl_UStrCat(utmpstr, _T("\""));
                        Utl_UStrCat(utmpstr, Res_GetStringByID(eRES_STR_AS_AUDIOREC_DEF_PATH));
                        MsgBox_AddLine(&pSet_Path_Parm->msgbox, utmpstr);
                        break;
#endif

                    default:
                        break;
                }
            }
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
        }
    }
    return 0;
}

T_VOID SetPath_GetFilePath(T_U16 *pFilePath, const T_U16 *pCurPath, const T_U16 *pFolderName)
{
	T_S32   count;
    T_S32   i;
	T_BOOL  flag = AK_FALSE;

    Utl_UStrCpy(pFilePath, (T_U16 *)pCurPath);

    if (Utl_UStrCmp((T_U16 *)pFolderName, _T("..")) == 0)
    {
        count = Utl_UStrLen(pFilePath) - 1;
        for (i=count; i>=0; i--)
        {
            if ((pFilePath[i] == UNICODE_SOLIDUS) || (pFilePath[i] == UNICODE_RES_SOLIDUS))
            {
                if (flag == AK_TRUE)
                    break;
                else
                    flag = AK_TRUE;
            }
            pFilePath[i] = 0;
        }
    }
    else if (Utl_UStrCmp((T_U16 *)pFolderName, _T(".")) != 0)
	{
        Utl_UStrCat(pFilePath, pFolderName);
        Utl_UStrCat(pFilePath, _T("/"));
    }
}
