
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_FileList.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "svc_medialist.h"



typedef struct {
    T_MSGBOX            msgbox;
} T_AUDIO_UPDATE_PARM;

static T_AUDIO_UPDATE_PARM *pAudio_Update_Parm;
extern T_BOOL fUpdateAudioLib;
#endif

/*---------------------- BEGIN OF STATE s_audio_delete_cnfm ------------------------*/
void initaudio_update(void)
{
#ifdef SUPPORT_AUDIOPLAYER
    pAudio_Update_Parm = (T_AUDIO_UPDATE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_UPDATE_PARM));
    AK_ASSERT_PTR_VOID(pAudio_Update_Parm, "initaudio_delete_cnfm(): malloc error");
#endif
}

void exitaudio_update(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Update_Parm = Fwl_Free(pAudio_Update_Parm);
#endif
}

void paintaudio_update(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    MsgBox_Show(&pAudio_Update_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_update(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   menuRet;
    
    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudio_Update_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }


    {
        if (M_EVT_UPDATE == event)
        {
            MsgBox_InitAfx(&pAudio_Update_Parm->msgbox, 0, ctHINT, csREFRESH_AUDIO_LIST, MSGBOX_QUESTION | MSGBOX_YESNO);
        }
            
        menuRet = MsgBox_Handler(&pAudio_Update_Parm->msgbox, event, pEventParm);
        switch (menuRet)
        {
        case eNext:
        	MsgBox_InitStr(&pAudio_Update_Parm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
            MsgBox_Show(&pAudio_Update_Parm->msgbox);
            Fwl_RefreshDisplay();
			/*Stop Play BackGround audio and delete CurPlay List*/
			AudioPlayer_Stop();
			Fwl_FileDelete(_T(AUDIOLIST_CURTPLY_FILE));

            MList_AddItem(Fwl_GetDefPath(eAUDIO_PATH), AK_TRUE, AK_FALSE, eMEDIA_LIST_AUDIO);

			fUpdateAudioLib = AK_TRUE;

            MsgBox_InitStr(&pAudio_Update_Parm->msgbox, 2, GetCustomTitle(ctHINT), Res_GetStringByID(eRES_STR_COMMAND_SENT), MSGBOX_INFORMATION);

            
            MsgBox_SetDelay(&pAudio_Update_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Update_Parm->msgbox);
            break;
			
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
        }
    }    
#endif
    return 0;
}



