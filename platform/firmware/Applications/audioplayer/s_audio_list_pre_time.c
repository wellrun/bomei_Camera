
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Ctl_AudioPlayer.h"
#include "lib_sdfilter.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
    T_BOOL          bReturn2;   //return to s_audio_menu or s_audio_player
} T_AUDIO_PRE_PARM;

static T_AUDIO_PRE_PARM *pAudio_PreParm;
#endif

/*---------------------- BEGIN OF STATE s_audio_list_pre_time ------------------------*/
void initaudio_list_pre_time(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_PreParm = (T_AUDIO_PRE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_PRE_PARM));
    AK_ASSERT_PTR_VOID(pAudio_PreParm, "initaudio_list_pre_time(): malloc error");

    MenuStructInit(&pAudio_PreParm->IconExplorer);
    GetMenuStructContent(&pAudio_PreParm->IconExplorer, mnMP3_PRE_TIME);
    IconExplorer_SetFocus(&pAudio_PreParm->IconExplorer, gb.AudioPreTime);

    pAudio_PreParm->bReturn2 = AK_TRUE;
    TopBar_DisableMenuButton();
#endif
}

void exitaudio_list_pre_time(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudio_PreParm->IconExplorer);
    pAudio_PreParm = Fwl_Free(pAudio_PreParm);
#endif
}

void paintaudio_list_pre_time(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudio_PreParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list_pre_time(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U16 focusID;
    

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudio_PreParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_3)       //from s_audio_menu
    {
        pAudio_PreParm->bReturn2 = AK_TRUE;
    }
    else if (event == M_EVT_PRE_TIME)       //from s_audio_player
    {
        pAudio_PreParm->bReturn2 = AK_FALSE;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudio_PreParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pAudio_PreParm->IconExplorer);
        if (focusID != gb.AudioPreTime)
        {
            //gb.AudioPlaySpeed = _SD_WSOLA_1_0;
            //Fwl_AudioSetPlaySpeed(gb.AudioPlaySpeed);
            
            gb.AudioPreTime = (T_U16)focusID;
            if (gb.AudioPreTime)
            {
                AudioPlayer_StartAudition();
            }
            else
            {
                AudioPlayer_StopAudition();
            }
        }
      #if 0
        MsgBox_InitAfx(&pAudio_PreParm->msgbox, 3, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pAudio_PreParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_PreParm->msgbox);
      #endif

        if (pAudio_PreParm->bReturn2 == AK_TRUE)       //trigger from s_audio_menu
        {
          m_triggerEvent( M_EVT_RETURN2, pEventParm );
        }
        else  //trigger from s_audio_player
        {
          m_triggerEvent(M_EVT_EXIT, pEventParm);
        }
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

