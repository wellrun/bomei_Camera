
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC

#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Fwl_osFS.h"
#include "Ctl_Audioplayer.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_DISPLAYLIST       *pDisplayList;
    T_MSGBOX            msgbox;
} T_AUDIOREC_DELETE_PARM;

static T_AUDIOREC_DELETE_PARM *pAudioRec_Delete_Parm;
#endif

/*---------------------- BEGIN OF STATE s_audiorec_delete_cnfm ------------------------*/
void initaudiorec_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRec_Delete_Parm = (T_AUDIOREC_DELETE_PARM *)Fwl_Malloc(sizeof(T_AUDIOREC_DELETE_PARM));
    AK_ASSERT_PTR_VOID(pAudioRec_Delete_Parm, "initaudiorec_delete_cnfm(): malloc error");
#endif
}

void exitaudiorec_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRec_Delete_Parm = Fwl_Free(pAudioRec_Delete_Parm);
#endif
}

void paintaudiorec_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    MsgBox_Show(&pAudioRec_Delete_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudiorec_delete_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE menuRet;
    T_FILE_INFO *pFileInfo;
    T_U8 result;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudioRec_Delete_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
    case M_EVT_1:
        pAudioRec_Delete_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm->p.pParam1;

        pFileInfo = DisplayList_GetItemContentFocus(pAudioRec_Delete_Parm->pDisplayList);
        if (pFileInfo != AK_NULL)
        {
            if((pFileInfo != AK_NULL) && ((pFileInfo->attrib & 0x10) == 0x10))
            {
                MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 0, ctHINT, csEXPLORER_DELETE_FOLDER, MSGBOX_QUESTION | MSGBOX_YESNO);
            }
            else
            {
                MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 0, ctHINT, csAUDIOREC_DELETE_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
            }
        }
        break;
    case M_EVT_DEL_EXIT:
        result = pEventParm->c.Param1;
        if (result && (result != 10))
        {
            DisplayList_DelFocusItem(pAudioRec_Delete_Parm->pDisplayList);
            MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        }
        else
        {
            //MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                if (result == 0) //delete file error
                MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 3, ctFAILURE, csFILE_DELFILE_FAILURE, MSGBOX_INFORMATION);
            else  //(result == 10),delete dir error
                MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 3, ctFAILURE, csFILE_DELDIR_FAILURE, MSGBOX_INFORMATION);
        }
        MsgBox_SetDelay(&pAudioRec_Delete_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRec_Delete_Parm->msgbox);
        break;
    default:
        break;
    }

    menuRet = MsgBox_Handler(&pAudioRec_Delete_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
    case eNext:
        pFileInfo = DisplayList_GetItemContentFocus(pAudioRec_Delete_Parm->pDisplayList);
        if ((pFileInfo == AK_NULL) \
            || (((pFileInfo->attrib & 0x10) == 0x10) \
            && ((Utl_UStrCmp(pFileInfo->name, _T(".")) == 0) \
            || (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0))))
        {
            MsgBox_InitAfx(&pAudioRec_Delete_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudioRec_Delete_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRec_Delete_Parm->msgbox);
        }
        else
        {
            pEventParm->p.pParam1 = DisplayList_GetCurFilePath(pAudioRec_Delete_Parm->pDisplayList);
            pEventParm->p.pParam2 = pFileInfo;
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
