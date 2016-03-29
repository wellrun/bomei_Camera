
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Ctl_DisplayList.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"



typedef struct {
    T_DISPLAYLIST       *pDisplayList;
    T_MSGBOX            msgbox;
} T_AUDIOREC_DELETE_ALL_PARM;

static T_AUDIOREC_DELETE_ALL_PARM *pAudioRec_Delete_All_Parm;
#endif

/*---------------------- BEGIN OF STATE s_audiorec_delete_all_cnfm ------------------------*/
void initaudiorec_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRec_Delete_All_Parm = (T_AUDIOREC_DELETE_ALL_PARM *)Fwl_Malloc(sizeof(T_AUDIOREC_DELETE_ALL_PARM));
    AK_ASSERT_PTR_VOID(pAudioRec_Delete_All_Parm, "initaudiorec_delete_all_cnfm(): malloc error");

    MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 0, ctHINT, csAUDIOREC_DELETE_ALL_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitaudiorec_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRec_Delete_All_Parm = Fwl_Free(pAudioRec_Delete_All_Parm);
#endif
}

void paintaudiorec_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOREC

    MsgBox_Show(&pAudioRec_Delete_All_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudiorec_delete_all_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE menuRet;
    T_FILE_INFO *pFileInfo;
    T_U8 result;
    T_U8 count;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudioRec_Delete_All_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
    case M_EVT_2:
        pAudioRec_Delete_All_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm->p.pParam1;
        break;
    case M_EVT_DEL_EXIT:
        result = pEventParm->c.Param1;
        if ((result > 0) && (result !=10))          //result=0: delete file error;  result=10:delete folder error!
        {
            DisplayList_DelFocusItem(pAudioRec_Delete_All_Parm->pDisplayList);
            count = 0;
            do {
                pFileInfo = DisplayList_GetItemContentFocus(pAudioRec_Delete_All_Parm->pDisplayList);
                if (pFileInfo == AK_NULL)
                    break;

                if (((pFileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER) \
                        && ((Utl_UStrCmp(pFileInfo->name, _T(".")) ==0) \
                        || (Utl_UStrCmp(pFileInfo->name, _T("..")) ==0)))
                {
                    DisplayList_MoveFocus(pAudioRec_Delete_All_Parm->pDisplayList, DISPLAYLIST_DIRECTION_DOWN);
                    count++;
                }
                else
                {
                    break;
                }
            } while (count < 3);

            if (result == 2)
                MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctHINT, csFILE_DELETE_CANCLE, MSGBOX_INFORMATION);
            else if ((pFileInfo != AK_NULL) && (count < 3))
            {
                pEventParm->p.pParam1 = DisplayList_GetCurFilePath(pAudioRec_Delete_All_Parm->pDisplayList);
                pEventParm->p.pParam2 = pFileInfo;
                m_triggerEvent(M_EVT_NEXT, pEventParm);
                break;
            }
            else
            {
                MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            }
        }
        else
            //MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
                {
                    if (result == 0) //delete file error
                    MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctFAILURE, csFILE_DELFILE_FAILURE, MSGBOX_INFORMATION);
                else  //(result == 10),delete dir error
                    MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctFAILURE, csFILE_DELDIR_FAILURE, MSGBOX_INFORMATION);
                }

        MsgBox_SetDelay(&pAudioRec_Delete_All_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRec_Delete_All_Parm->msgbox);
        break;
    default:
        break;
    }

    menuRet = MsgBox_Handler(&pAudioRec_Delete_All_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
    case eNext:
        count = 0;
        do {
            pFileInfo = DisplayList_GetItemContentFocus(pAudioRec_Delete_All_Parm->pDisplayList);
            if (pFileInfo == AK_NULL)
                break;

            if (((pFileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER) \
                    &&((Utl_UStrCmp(pFileInfo->name, _T(".")) ==0) \
                    || (Utl_UStrCmp(pFileInfo->name, _T("..")) ==0)))
            {
                DisplayList_MoveFocus(pAudioRec_Delete_All_Parm->pDisplayList, DISPLAYLIST_DIRECTION_DOWN);
                count++;
            }
            else
            {
                break;
            }
        } while (count < 3);

        pFileInfo = DisplayList_GetItemContentFocus(pAudioRec_Delete_All_Parm->pDisplayList);
        if (pFileInfo == AK_NULL)
        {
            MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudioRec_Delete_All_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRec_Delete_All_Parm->msgbox);
        }
        else if (count >= 3)
        {
            MsgBox_InitAfx(&pAudioRec_Delete_All_Parm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudioRec_Delete_All_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioRec_Delete_All_Parm->msgbox);
        }
        else
        {
            pEventParm->p.pParam1 = DisplayList_GetCurFilePath(pAudioRec_Delete_All_Parm->pDisplayList);
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

