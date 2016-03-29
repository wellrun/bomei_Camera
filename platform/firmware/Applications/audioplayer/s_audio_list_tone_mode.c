
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_AUDIO_TONE_PARM;

static T_AUDIO_TONE_PARM *pAudio_ToneParm;
extern T_AUDIOPLAYER *p_audio_player;

#endif

/*---------------------- BEGIN OF STATE s_audio_list_tone_mode ------------------------*/
void initaudio_list_tone_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_ToneParm = (T_AUDIO_TONE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_TONE_PARM));
    AK_ASSERT_PTR_VOID(pAudio_ToneParm, "initmp3_list_repeat_mode(): malloc error");

    MenuStructInit(&pAudio_ToneParm->IconExplorer);
    GetMenuStructContent(&pAudio_ToneParm->IconExplorer, mnMP3_TONE_MODE);
    IconExplorer_SetFocus(&pAudio_ToneParm->IconExplorer, gs.AudioToneMode);
    TopBar_DisableMenuButton();
#endif
}

void exitaudio_list_tone_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudio_ToneParm->IconExplorer);
    pAudio_ToneParm = Fwl_Free(pAudio_ToneParm);
#endif
}

void paintaudio_list_tone_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudio_ToneParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list_tone_mode(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{

#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U16 focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudio_ToneParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudio_ToneParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pAudio_ToneParm->IconExplorer);

        if (focusID != gs.AudioToneMode)
        {
            gs.AudioToneMode = (T_U8)focusID;            
            Fwl_AudioSetEQMode(gs.AudioToneMode);            
        }

        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
