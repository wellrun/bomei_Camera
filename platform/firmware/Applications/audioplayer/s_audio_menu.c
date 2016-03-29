

#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Ctl_AudioPlayer.h"
#include "Fwl_oscom.h"
#include "Ctl_Msgbox.h"
#include "Fwl_Initialize.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_MSGBOX        msgbox;
    T_ICONEXPLORER  IconExplorer;
    T_USTR_FILE     FilePath;
    T_USTR_FILE     FileName;
}T_AUDIOMENU_PARM;

static T_AUDIOMENU_PARM *pAudioMenu = AK_NULL;

static T_VOID Mp32System(T_BOOL PowerMusic);
#endif

/*---------------------- BEGIN OF STATE s_audio_menu ------------------------*/
void initaudio_menu(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudioMenu = (T_AUDIOMENU_PARM *)Fwl_Malloc(sizeof(T_AUDIOMENU_PARM));
    AK_ASSERT_PTR_VOID(pAudioMenu, "initmp3_menu(): malloc error");

    MenuStructInit(&pAudioMenu->IconExplorer);
    GetMenuStructContent(&pAudioMenu->IconExplorer, mnMP3_MENU);

    TopBar_DisableMenuButton();

	gs.bPlayer2Saver = AK_FALSE;

#endif
}

void exitaudio_menu(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudioMenu->IconExplorer);
    pAudioMenu = Fwl_Free(pAudioMenu);
#endif
}

void paintaudio_menu(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudioMenu->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE IconExplorerRet;
//    T_FILELIST  *pFileList = AK_NULL;
    T_pCWSTR     pFilePath = AK_NULL;
    T_pCWSTR     pFileName = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioMenu->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_MENU)
    {
        IconExplorer_DelItem(&pAudioMenu->IconExplorer, 50);
        pFilePath = (T_pCWSTR)pEventParm->p.pParam1;
        pFileName = (T_pCWSTR)pEventParm->p.pParam2;

        if (pFilePath == AK_NULL || pFileName == AK_NULL)
        {
            IconExplorer_DelItem(&pAudioMenu->IconExplorer, 60);
            IconExplorer_DelItem(&pAudioMenu->IconExplorer, 70);
        }
        else
        {
            Utl_UStrCpyN(pAudioMenu->FilePath, (T_U16 *)pFilePath, sizeof(T_USTR_FILE)/2);
            Utl_UStrCpyN(pAudioMenu->FileName, (T_U16 *)pFileName, sizeof(T_USTR_FILE)/2);
        }

		// Delete Repeat Mode
		if (M_EVT_1 == (T_U32)pEventParm->p.pParam3)
		{
			IconExplorer_DelItem(&pAudioMenu->IconExplorer, 10);
        }
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioMenu->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pAudioMenu->IconExplorer))
        {
        case 10:                // Repeat Mode
            m_triggerEvent(M_EVT_1, pEventParm);
            break;
        case 20:                // Tone Mode
            m_triggerEvent(M_EVT_2, pEventParm);
            break;
        case 30:                // Prefade Listen
            m_triggerEvent(M_EVT_3, pEventParm);
            break;
        case 40:
            m_triggerEvent(M_EVT_4, pEventParm);
            break;
        case 50:
            break;
        case 60:                // Save to Boot Music
            Mp32System(AK_FALSE);
            break;
        case 70:
            Mp32System(AK_TRUE);
            break;
		case 90:				// Set Voice Change
            m_triggerEvent(M_EVT_6, pEventParm);
            break;
        default:
            break;
        }
        break;

#ifdef SUPPORT_VISUAL_AUDIO	
	case eOption:
        switch (IconExplorer_GetItemFocusId(&pAudioMenu->IconExplorer))
        {
        case 80:
			if(gs.bVisualAudio)
				MsgBox_InitAfx(&pAudioMenu->msgbox, 1, ctHINT, csVISUAL_AUDIO_ENABLE, MSGBOX_INFORMATION);
			else
				MsgBox_InitAfx(&pAudioMenu->msgbox, 1, ctHINT, csVISUAL_AUDIO_DISABLE, MSGBOX_INFORMATION);
				
			MsgBox_SetDelay(&pAudioMenu->msgbox, MSGBOX_DELAY_0);
			m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioMenu->msgbox);
       		break;
        default:
            break;
        }
        break;
#endif		

    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}



#ifdef SUPPORT_AUDIOPLAYER

/* PowerMusic:  AK_FALSE:save to power on picture
**              AK_TRUE :save to power off picture
*/
static T_VOID Mp32System(T_BOOL PowerMusic)
{
    T_USTR_FILE Content;
    T_pCWSTR    pFilePath = AK_NULL;
    T_pCWSTR    pFileName = AK_NULL;
    
    pFilePath = AudioPlayer_GetPlayAudioPath();
    pFileName = AudioPlayer_GetPlayAudioName();

    if ((pFilePath != AK_NULL) && (pFileName != AK_NULL))
    {
        Utl_UStrCpy(pAudioMenu->FilePath, (T_U16 *)pFilePath);
        Utl_UStrCpy(pAudioMenu->FileName, (T_U16 *)pFileName);
    }

    if(PowerMusic == AK_FALSE)
        Utl_UStrCpy(gs.PathPonAudio, pAudioMenu->FilePath);
    else
        Utl_UStrCpy(gs.PathPoffAudio, pAudioMenu->FilePath);

    Utl_UStrCpy(Content, pAudioMenu->FileName);
    Utl_UStrCat(Content, _T(" "));
    
    if(PowerMusic == AK_FALSE)
        Utl_UStrCat(Content, Res_GetStringByID(eRES_STR_MP3_SAVE_TO_POWERON_MUSIC));
    else
        Utl_UStrCat(Content, Res_GetStringByID(eRES_STR_MP3_SAVE_TO_POWEROFF_MUSIC));

    MsgBox_InitStr(&pAudioMenu->msgbox, 1, Res_GetStringByID(eRES_STR_PUB_SUCCESS), Content, MSGBOX_INFORMATION);
    MsgBox_SetDelay(&pAudioMenu->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioMenu->msgbox);

    return;
}

#endif
