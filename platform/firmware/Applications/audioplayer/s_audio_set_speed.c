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
} T_AUDIO_SPEED_PARM;

static T_AUDIO_SPEED_PARM *pAudio_SpeedParm;

extern T_AUDIOPLAYER *p_audio_player;

#endif

/*---------------------- BEGIN OF STATE s_audio_set_speed ------------------------*/
void initaudio_set_speed(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_SpeedParm = (T_AUDIO_SPEED_PARM *)Fwl_Malloc(sizeof(T_AUDIO_SPEED_PARM));
    AK_ASSERT_PTR_VOID(pAudio_SpeedParm, "initaudio_set_speed(): malloc error");

    MenuStructInit(&pAudio_SpeedParm->IconExplorer);
    GetMenuStructContent(&pAudio_SpeedParm->IconExplorer, mnMP3_SPEED);
    IconExplorer_SetFocus(&pAudio_SpeedParm->IconExplorer, gb.AudioPlaySpeed);
    TopBar_DisableMenuButton();
#endif
}

void exitaudio_set_speed(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudio_SpeedParm->IconExplorer);
    pAudio_SpeedParm = Fwl_Free(pAudio_SpeedParm);
#endif
}

void paintaudio_set_speed(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudio_SpeedParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_set_speed(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U16 focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudio_SpeedParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudio_SpeedParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pAudio_SpeedParm->IconExplorer);
        if (focusID != gb.AudioPlaySpeed)
        {
            gb.AudioPlaySpeed = focusID;
                        
            Fwl_AudioSetPlaySpeed(gb.AudioPlaySpeed);            
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

