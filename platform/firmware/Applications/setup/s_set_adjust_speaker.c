
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Ctl_Dialog.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DIALOG        dialog;
    T_MSGBOX        msgbox;
} T_ADJUST_SPEAKER_PARM;

static T_ADJUST_SPEAKER_PARM *pAdjust_SpeakerParm;
#endif
/*---------------------- BEGIN OF STATE s_set_adjust_speaker ------------------------*/
void initset_adjust_speaker(void)
{
#ifdef SUPPORT_SYS_SET
    pAdjust_SpeakerParm = (T_ADJUST_SPEAKER_PARM *)Fwl_Malloc(sizeof(T_ADJUST_SPEAKER_PARM));
    AK_ASSERT_PTR_VOID(pAdjust_SpeakerParm, "initset_adjust_speaker(): malloc error");

    Dialog_Init(&pAdjust_SpeakerParm->dialog, gs.SpeakerGain, 0, 4, 1);
    Dialog_SetTitle(&pAdjust_SpeakerParm->dialog, GetCustomTitle(ctADJUSTSPEAKER));
#endif
}

void exitset_adjust_speaker(void)
{
#ifdef SUPPORT_SYS_SET
    Dialog_Free(&pAdjust_SpeakerParm->dialog);
    pAdjust_SpeakerParm = Fwl_Free(pAdjust_SpeakerParm);
#endif
}

void paintset_adjust_speaker(void)
{
#ifdef SUPPORT_SYS_SET
    Dialog_Show(&pAdjust_SpeakerParm->dialog, g_Graph.TtlFrCL);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_adjust_speaker(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   dialogRet;
    T_S16           CurValue;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    dialogRet = Dialog_Handler(&pAdjust_SpeakerParm->dialog, event, pEventParm);

    switch (dialogRet)
    {
    case eNext:
        CurValue = Dialog_GetCurValue(&pAdjust_SpeakerParm->dialog);
        if (CurValue != gs.SpeakerGain)
        {
            gs.SpeakerGain = (T_U8)CurValue;
        }
        MsgBox_InitAfx(&pAdjust_SpeakerParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pAdjust_SpeakerParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAdjust_SpeakerParm->msgbox);
        break;
    default:
        ReturnDefauleProc(dialogRet, pEventParm);
        break;
    }
#endif
    return 0;
}
