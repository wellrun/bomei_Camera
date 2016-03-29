
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_AUDIO_REPEAT_PARM;

static T_AUDIO_REPEAT_PARM *pAudio_RepeatParm;
#endif

/*---------------------- BEGIN OF STATE s_audio_list_repeat_mode ------------------------*/
void initaudio_list_repeat_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_RepeatParm = (T_AUDIO_REPEAT_PARM *)Fwl_Malloc(sizeof(T_AUDIO_REPEAT_PARM));
    AK_ASSERT_PTR_VOID(pAudio_RepeatParm, "initaudio_list_repeat_mode(): malloc error");

    MenuStructInit(&pAudio_RepeatParm->IconExplorer);
    GetMenuStructContent(&pAudio_RepeatParm->IconExplorer, mnMP3_REPEAT_MODE);
    IconExplorer_SetFocus(&pAudio_RepeatParm->IconExplorer, gs.AudioRepMode);
    TopBar_DisableMenuButton();
#endif
}

void exitaudio_list_repeat_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudio_RepeatParm->IconExplorer);
    pAudio_RepeatParm = Fwl_Free(pAudio_RepeatParm);
#endif
}

void paintaudio_list_repeat_mode(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudio_RepeatParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list_repeat_mode(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U32 focusID;
//    T_FILELIST *pFileList = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudio_RepeatParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudio_RepeatParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
    case eNext:
        focusID = IconExplorer_GetItemFocusId(&pAudio_RepeatParm->IconExplorer);
        if (focusID != gs.AudioRepMode)
        {
            gs.AudioRepMode = (T_U8)focusID;
//          SaveUserdata();
        }
        #if 0
        MsgBox_InitAfx(&pAudio_RepeatParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pAudio_RepeatParm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_RepeatParm->msgbox);
        #endif
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
