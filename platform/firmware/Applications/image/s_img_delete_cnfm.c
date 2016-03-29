
#include "Fwl_public.h"

#ifdef SUPPORT_IMG_BROWSE
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Ctl_Audioplayer.h"
#include "Eng_DataConvert.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_BOOL              FromImgBrowse;
    T_DISPLAYLIST       *pDisplayList;
    T_MSGBOX            msgbox;
} T_IMG_DELETE_PARM;

static T_IMG_DELETE_PARM *pImg_Delete_Parm;
#endif
/*---------------------- BEGIN OF STATE s_img_delete_cnfm ------------------------*/
void initimg_delete_cnfm(void)
{
#ifdef SUPPORT_IMG_BROWSE

    pImg_Delete_Parm = (T_IMG_DELETE_PARM *)Fwl_Malloc(sizeof(T_IMG_DELETE_PARM));
    AK_ASSERT_PTR_VOID(pImg_Delete_Parm, "initimg_delete_cnfm(): malloc error");
#endif
}

void exitimg_delete_cnfm(void)
{
#ifdef SUPPORT_IMG_BROWSE

    pImg_Delete_Parm = Fwl_Free(pImg_Delete_Parm);
#endif
}

void paintimg_delete_cnfm(void)
{
#ifdef SUPPORT_IMG_BROWSE

    MsgBox_Show(&pImg_Delete_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleimg_delete_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE

    T_U32 tmp;
    T_eBACK_STATE menuRet;
    T_FILE_INFO     *FileInfo;
    T_U8            result;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pImg_Delete_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
    case M_EVT_1:
        pImg_Delete_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm->p.pParam1;
           tmp = (T_U32)pEventParm->p.pParam2;
           pImg_Delete_Parm->FromImgBrowse = (T_BOOL)tmp;

        FileInfo = DisplayList_GetItemContentFocus(pImg_Delete_Parm->pDisplayList);
        if (FileInfo != AK_NULL)
        {
            if((FileInfo != AK_NULL) && ((FileInfo->attrib & 0x10) == 0x10))
            {
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 0, ctHINT, csEXPLORER_DELETE_FOLDER, MSGBOX_QUESTION | MSGBOX_YESNO);
            }
            else
            {
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 0, ctHINT, csIMG_DELETE_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
            }
        }

        break;
    case M_EVT_DEL_EXIT:
        result = pEventParm->c.Param1;
        if (result && (result !=10))
        {
            DisplayList_DelFocusItem(pImg_Delete_Parm->pDisplayList);
            if (pImg_Delete_Parm->FromImgBrowse)
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 4, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            else
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        }
        else
        {
            //if (pImg_Delete_Parm->FromImgBrowse)
            //  MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 4, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            //else
            //  MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                if (result == 0) //delete file error
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 3, ctFAILURE, csFILE_DELFILE_FAILURE, MSGBOX_INFORMATION);
            else  //(result == 10),delete dir error
                MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 3, ctFAILURE, csFILE_DELDIR_FAILURE, MSGBOX_INFORMATION);
        }
        MsgBox_SetDelay(&pImg_Delete_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImg_Delete_Parm->msgbox);
        break;
    default:
        break;
    }

    menuRet = MsgBox_Handler(&pImg_Delete_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
    case eNext:
        FileInfo = DisplayList_GetItemContentFocus(pImg_Delete_Parm->pDisplayList);
        if ((FileInfo == AK_NULL) \
                || (((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER) \
                && ((Utl_UStrCmp(FileInfo->name, _T(".")) == 0)\
                || (Utl_UStrCmp(FileInfo->name, _T("..")) == 0))))
        {
            MsgBox_InitAfx(&pImg_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pImg_Delete_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImg_Delete_Parm->msgbox);
        }
        else
        {
            pEventParm->p.pParam1 = DisplayList_GetCurFilePath(pImg_Delete_Parm->pDisplayList);
            pEventParm->p.pParam2 = FileInfo;
            m_triggerEvent(M_EVT_NEXT, pEventParm);
        }
        break;
    default:
        ReturnDefauleProc(menuRet, pEventParm);
        break;
    }
#endif
    return 0;
}
