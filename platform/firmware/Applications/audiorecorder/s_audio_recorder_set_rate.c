
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
} T_AUDIO_RECORD_SET_RATE_PARM;

static T_AUDIO_RECORD_SET_RATE_PARM *pAudioRecordSetRateParm;
#endif

/*---------------------- BEGIN OF STATE s_set_lowbat_time ------------------------*/
void initaudio_recorder_set_rate(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRecordSetRateParm = (T_AUDIO_RECORD_SET_RATE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_RECORD_SET_RATE_PARM));
    AK_ASSERT_PTR_VOID(pAudioRecordSetRateParm, "initaudio_recorder_set_rate(): malloc error");

    MenuStructInit(&pAudioRecordSetRateParm->IconExplorer);
    GetMenuStructContent(&pAudioRecordSetRateParm->IconExplorer, mnAUDIO_RECORD_SET_RATE);

    if (gs.AudioRecordMode== eRECORD_MODE_AMR)
    {
    	gs.AudioRecordRate = 8000;
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 11025);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 12000);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 16000);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 22050);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 24000);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 32000);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 44100);
    	IconExplorer_DelItem(&pAudioRecordSetRateParm->IconExplorer, 48000);
    }
    
    IconExplorer_SetFocus(&pAudioRecordSetRateParm->IconExplorer, gs.AudioRecordRate);
#endif
}

void exitaudio_recorder_set_rate(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Free(&pAudioRecordSetRateParm->IconExplorer);
    pAudioRecordSetRateParm = Fwl_Free(pAudioRecordSetRateParm);
#endif
}

void paintaudio_recorder_set_rate(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Show(&pAudioRecordSetRateParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_recorder_set_rate(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE   IconExplorerRet;
    T_U32           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioRecordSetRateParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioRecordSetRateParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = IconExplorer_GetItemFocusId(&pAudioRecordSetRateParm->IconExplorer);
        if (focusID != gs.AudioRecordRate)
        {
            gs.AudioRecordRate = focusID;
        }
#ifdef SUPPORT_AUDIOREC_DENOICE
		if (8000 != gs.AudioRecordRate)
		{
			gs.bAudioRecDenoice = AK_FALSE;
		}
#endif
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
