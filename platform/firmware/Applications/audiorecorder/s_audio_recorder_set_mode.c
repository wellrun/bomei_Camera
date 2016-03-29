
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"



typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_AUDIO_RECORD_SET_MODE_PARM;

static T_AUDIO_RECORD_SET_MODE_PARM *pAudioRecordSetModeParm;
#endif

/*---------------------- BEGIN OF STATE s_set_lowbat_time ------------------------*/
void initaudio_recorder_set_mode(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRecordSetModeParm = (T_AUDIO_RECORD_SET_MODE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_RECORD_SET_MODE_PARM));
    AK_ASSERT_PTR_VOID(pAudioRecordSetModeParm, "initaudio_recorder_set_mode(): malloc error");

    MenuStructInit(&pAudioRecordSetModeParm->IconExplorer);
    GetMenuStructContent(&pAudioRecordSetModeParm->IconExplorer, mnAUDIO_RECORD_SET_MODE);
    IconExplorer_SetFocus(&pAudioRecordSetModeParm->IconExplorer, gs.AudioRecordMode);
#endif
}

void exitaudio_recorder_set_mode(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Free(&pAudioRecordSetModeParm->IconExplorer);
    pAudioRecordSetModeParm = Fwl_Free(pAudioRecordSetModeParm);
#endif
}

void paintaudio_recorder_set_mode(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Show(&pAudioRecordSetModeParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_recorder_set_mode(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE   IconExplorerRet;
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioRecordSetModeParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioRecordSetModeParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pAudioRecordSetModeParm->IconExplorer);
        if (focusID != gs.AudioRecordMode)
        {
            gs.AudioRecordMode = focusID;

            if (gs.AudioRecordMode == eRECORD_MODE_AMR)
            {
            	gs.AudioRecordRate = 8000;
            }
        }
        GE_ShadeInit();
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
