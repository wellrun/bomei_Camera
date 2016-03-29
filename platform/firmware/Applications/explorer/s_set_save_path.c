

#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "fwl_keyhandler.h"
#include "Fwl_pfKeypad.h"
#include "Eng_FileManage.h"
#include "Eng_DataConvert.h"
#include "Lib_geshade.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST           DisplayList;
    T_MSGBOX                msgbox;
    T_DEFPATH_TYPE          type;
    T_USTR_FILE             setPath;
    T_BOOL                  MsgFlag;
} T_SET_SAVE_PATH_PARM;

static T_SET_SAVE_PATH_PARM *pSet_Save_Path_Parm = AK_NULL;

T_VOID SetPath_GetFilePath(T_U16 *pFilePath, const T_U16 *pCurPath, const T_U16 *pFolderName);

/*---------------------- BEGIN OF STATE s_set_save_path ------------------------*/
void initset_save_path(void)
{
    pSet_Save_Path_Parm = (T_SET_SAVE_PATH_PARM *)Fwl_Malloc(sizeof(T_SET_SAVE_PATH_PARM));
    AK_ASSERT_PTR_VOID(pSet_Save_Path_Parm, "initset_save_path(): malloc error");

    pSet_Save_Path_Parm->MsgFlag = AK_FALSE;
}

void exitset_save_path(void)
{
    DisplayList_Free(&pSet_Save_Path_Parm->DisplayList);
    pSet_Save_Path_Parm = Fwl_Free(pSet_Save_Path_Parm);

    TopBar_DisableMenuButton();
}

void paintset_save_path(void)
{
    if (pSet_Save_Path_Parm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pSet_Save_Path_Parm->msgbox);
    }
    else
    {
        DisplayList_SetTopBarMenuIconState(&pSet_Save_Path_Parm->DisplayList);
        DisplayList_Show(&pSet_Save_Path_Parm->DisplayList);
        GE_StartShade();
    }
    Fwl_RefreshDisplay();
}

unsigned char handleset_save_path(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE DisplayListRet;
    T_U16         *pCurPath = AK_NULL;
    T_U16         EmptyStr[2];
    T_USTR_FILE   Ustrbuf, Ustrbuf2;
    T_FILE_INFO   *pFileInfo = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pSet_Save_Path_Parm->DisplayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_SETPATH)
    {
        T_FILE_TYPE FileType[] = {FILE_TYPE_NONE};

        pSet_Save_Path_Parm->type = (T_DEFPATH_TYPE)pEventParm->w.Param1;
        DisplayList_init(&pSet_Save_Path_Parm->DisplayList, Fwl_GetDefPath(pSet_Save_Path_Parm->type), \
                Res_GetStringByID(eRES_STR_PATH_SET), FileType);
    }

    if (pSet_Save_Path_Parm->MsgFlag == AK_TRUE)
    {
        switch (MsgBox_Handler(&pSet_Save_Path_Parm->msgbox, event, pEventParm))
        {
        case eNext:
#ifdef CAMERA_SUPPORT
			if (!(0 == Utl_UStrCmp(pSet_Save_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH))
				&& (eVIDEOREC_PATH == pSet_Save_Path_Parm->type)))
#else
			if (!(0 == Utl_UStrCmp(pSet_Save_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH))))
#endif
			{
	            if (Fwl_SetDefPath(pSet_Save_Path_Parm->type, pSet_Save_Path_Parm->setPath))
	            {
	            	MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
	            }
				else
				{
					MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctHINT, csFAILURE_DONE, MSGBOX_INFORMATION);
				}
			
            	MsgBox_SetDelay(&pSet_Save_Path_Parm->msgbox, MSGBOX_DELAY_0);
            	m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pSet_Save_Path_Parm->msgbox);
			}
        case eReturn:
            pSet_Save_Path_Parm->MsgFlag = AK_FALSE;
            DisplayList_SetRefresh(&pSet_Save_Path_Parm->DisplayList, DISPLAYLIST_REFRESH_ALL);
            break;
			
        default:
            break;
        }
    }
    else
    {
        DisplayListRet = DisplayList_Handler(&pSet_Save_Path_Parm->DisplayList, event, pEventParm);
        switch (DisplayListRet)
        {
        case eNext:
            DisplayList_SetRefresh(&pSet_Save_Path_Parm->DisplayList, DISPLAYLIST_REFRESH_ALL);
            DisplayList_Operate(&pSet_Save_Path_Parm->DisplayList);
            break;
			
        case eMenu:
            pCurPath = DisplayList_GetCurFilePath(&pSet_Save_Path_Parm->DisplayList);
            pFileInfo = DisplayList_GetItemContentFocus(&pSet_Save_Path_Parm->DisplayList);
            if (pFileInfo != AK_NULL \
                && pCurPath != AK_NULL \
                && (pFileInfo->attrib & 0x10) == 0x10 \
                && Utl_UStrCmp(pFileInfo->name, _T("..")) != 0)
            {
                SetPath_GetFilePath(pSet_Save_Path_Parm->setPath, pCurPath, pFileInfo->name);

                if (Utl_UStrLen(pSet_Save_Path_Parm->setPath) > (MAX_FILENM_LEN -FILE_LEN_RESERVE1))
                {
                    MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctHINT, csFILENAME_LONG, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pSet_Save_Path_Parm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE,(T_EVT_PARAM *)&pSet_Save_Path_Parm->msgbox);
                    break;
                }

                if (!((Utl_UStrCmp(pFileInfo->name, _T(".")) == 0) \
                    && (pSet_Save_Path_Parm->DisplayList.subLevel == 1)))
                {
                    Utl_UStrCpy(Ustrbuf, pSet_Save_Path_Parm->setPath);
                    Ustrbuf[Utl_UStrLen(Ustrbuf) - 1] = 0;
                    if (!FileMgr_CheckFileIsExist(Ustrbuf))
                    {
                        MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctHINT, csFAILURE_DONE, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pSet_Save_Path_Parm->msgbox, MSGBOX_DELAY_1);
                        m_triggerEvent(M_EVT_MESSAGE,(T_EVT_PARAM *)&pSet_Save_Path_Parm->msgbox);
                        break;
                    }
                }

                pSet_Save_Path_Parm->MsgFlag = AK_TRUE;
                Utl_UStrCpy(Ustrbuf, GetCustomString(csSET_DEF_PATH_SET));
                Eng_StrMbcs2Ucs("\"", Ustrbuf2);
                Utl_UStrCat(Ustrbuf, Ustrbuf2);
                Utl_UStrCat(Ustrbuf, pSet_Save_Path_Parm->setPath);
                Utl_UStrCat(Ustrbuf, Ustrbuf2);
                EmptyStr[0] = 0;

                if (pSet_Save_Path_Parm->type == eAUDIOREC_PATH)
                {
                    MsgBox_InitStr(&pSet_Save_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), EmptyStr, MSGBOX_QUESTION | MSGBOX_YESNO);
                    Utl_UStrCat(Ustrbuf, Res_GetStringByID(eRES_STR_AS_AUDIOREC_DEF_PATH));
                    MsgBox_AddLine(&pSet_Save_Path_Parm->msgbox, Ustrbuf);
                }
#ifdef CAMERA_SUPPORT
                else if (pSet_Save_Path_Parm->type == eVIDEOREC_PATH)
                {
                	Printf_UC(pSet_Save_Path_Parm->setPath);
					Printf_UC(Fwl_GetDefPath(eRECIDX_PATH));
                	if (0 != Utl_UStrCmp(pSet_Save_Path_Parm->setPath,Fwl_GetDefPath(eRECIDX_PATH)))
                	{
	                    MsgBox_InitStr(&pSet_Save_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), EmptyStr, MSGBOX_QUESTION | MSGBOX_YESNO);
	                    Utl_UStrCat(Ustrbuf, Res_GetStringByID(eRES_STR_AS_VEDIOREC_DEF_PATH));
	                    MsgBox_AddLine(&pSet_Save_Path_Parm->msgbox, Ustrbuf);
                	}
					else
					{
						MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctHINT, csCAMERA_FOLDER_TABOO, MSGBOX_INFORMATION);
						MsgBox_SetDelay(&pSet_Save_Path_Parm->msgbox, MSGBOX_DELAY_0);
					}
                }
                else if (pSet_Save_Path_Parm->type == eIMAGEREC_PATH)
                {
                    MsgBox_InitStr(&pSet_Save_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), EmptyStr, MSGBOX_QUESTION | MSGBOX_YESNO);
                    Utl_UStrCat(Ustrbuf, Res_GetStringByID(eRES_STR_AS_CAPTURE_DEF_PATH));
                    MsgBox_AddLine(&pSet_Save_Path_Parm->msgbox, Ustrbuf);
                }
#endif

#ifdef SUPPORT_FM
                else if (pSet_Save_Path_Parm->type == eFMREC_PATH)
                {
                    MsgBox_InitStr(&pSet_Save_Path_Parm->msgbox, 0, GetCustomString(csSET_DEF_PATH_SET), EmptyStr, MSGBOX_QUESTION | MSGBOX_YESNO);
                    Utl_UStrCat(Ustrbuf, Res_GetStringByID(eRES_STR_AS_FMREC_DEF_PATH));
                    MsgBox_AddLine(&pSet_Save_Path_Parm->msgbox, Ustrbuf);
                }
#endif
                else
                {
                    MsgBox_InitAfx(&pSet_Save_Path_Parm->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pSet_Save_Path_Parm->msgbox, MSGBOX_DELAY_1);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pSet_Save_Path_Parm->msgbox);
                }
            }
            break;
			
        default:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
        }
    }

    return 0;
}
