
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Ctl_AudioPlayer.h"
#include "eng_gblstring.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_pfAudio.h"
#include "Ctl_Fm.h"
#include "Ctl_APlayerList.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;       /*for s_audio_root state machine*/
    T_ICONEXPLORER  *pGIconExplorer;    /*pGIconExplorer:valid in audio player module, copy audio list to pGIconExplorer 
                                        when you play some song in the audio list*/
} T_AUDIO_ROOT_PARM;
static T_AUDIO_ROOT_PARM    *pAudioRootParm;

static T_U32 LastFocusID = 0;
#endif

/*---------------------- BEGIN OF STATE s_audio_root ------------------------*/
void initaudio_root(void)
{
#ifdef SUPPORT_AUDIOPLAYER

	Standby_FreeUserBkImg();

    pAudioRootParm = (T_AUDIO_ROOT_PARM *)Fwl_Malloc(sizeof(T_AUDIO_ROOT_PARM));
    AK_ASSERT_PTR_VOID(pAudioRootParm, "initaudio_root(): pAudioRootParm malloc error");
    MenuStructInit(&pAudioRootParm->IconExplorer);
    GetMenuStructContent(&pAudioRootParm->IconExplorer, mnAUDIO_ROOT);
    IconExplorer_SetFocus(&pAudioRootParm->IconExplorer, LastFocusID);
    IconExplorer_SetRefresh(&pAudioRootParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);

    if (AUDIOPLAYER_STATE_BACKGROUNDPLAY == AudioPlayer_GetCurState())
    {
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
        pAudioRootParm->pGIconExplorer = AudioPlayer_GetIconExplorer();
    }
    else
    {
        pAudioRootParm->pGIconExplorer = (T_ICONEXPLORER *)Fwl_Malloc(sizeof(T_ICONEXPLORER));
        AK_ASSERT_PTR_VOID(pAudioRootParm->pGIconExplorer, "initaudio_root(): pGIconExplorer malloc error");

        MenuStructInit(pAudioRootParm->pGIconExplorer);
		IconExplorer_SetItemQtyMax(pAudioRootParm->pGIconExplorer, AUDIOPLAYER_MAX_ITEM_QTY);
        // Fwl_AudioEnableDA();
    }
#endif
}

void exitaudio_root(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    WaitBox_Stop();
    if (AudioPlayer_GetCurState() == AUDIOPLAYER_STATE_PLAY)
    {
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_BACKGROUNDPLAY);
    }
    else
    {
        if (AK_NULL != AudioPlayer_GetIconExplorer())
        {
            AudioPlayer_Destroy();
            AudioPlayer_Free();
        }
        else
        {
            IconExplorer_Free(pAudioRootParm->pGIconExplorer);
            pAudioRootParm->pGIconExplorer = Fwl_Free(pAudioRootParm->pGIconExplorer);
        }

		{
			AK_DEBUG_OUTPUT("exit audio root, Close DA.\n");
	        Fwl_AudioDisableDA();
		}

		MList_Close(eMEDIA_LIST_AUDIO);
    }

    IconExplorer_Free(&pAudioRootParm->IconExplorer);
    pAudioRootParm = Fwl_Free(pAudioRootParm);

	Standby_LoadUserBkImg();
#endif
}

void paintaudio_root(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudioRootParm->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_root(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;
    T_U32 focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioRootParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (VME_EVT_PAINT == event)
    {
        IconExplorer_SetRefresh(&pAudioRootParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 0;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioRootParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
    case eNext:
        focusID = IconExplorer_GetItemFocusId(&pAudioRootParm->IconExplorer);
        LastFocusID = focusID; 
        pEventParm->p.pParam1 = pAudioRootParm->pGIconExplorer;

        switch (focusID)
        {
        case 1: // 当前播放
            GE_ShadeInit();
            m_triggerEvent(M_EVT_CURPLAY, pEventParm);
            break;
			
        case 2: // 最常播放
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_OFTENPLAY;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
			
        case 3: // 最近播放
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_RECEPLAY;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
			
        case 4: // 最近增加
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_RECEAPPEND;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
			
        case 5: // 歌曲库
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_TITLE;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
            
        case 6: // 流派
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_GENRE;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
			
        case 7: // 艺术家
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_ARTIST;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break; 
			
        case 8: // 专辑
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_ALBUM;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
			
        case 9: // 作曲家
            GE_ShadeInit();
            pEventParm->p.pParam3 = (T_pVOID)LTP_COMPOSER;
            m_triggerEvent(M_EVT_LIST, pEventParm);
            break;
            
        case 10:
            GE_ShadeInit();
            m_triggerEvent(M_EVT_FAVORITE, pEventParm);
            break;
			
        case 11:			
            GE_ShadeInit();
            m_triggerEvent(M_EVT_FETCH_SONG, pEventParm);
            break;
			
        case 12:			
            m_triggerEvent(M_EVT_UPDATE, pEventParm);
            break;
			
        default:
            break;
        }
        break;
        
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
