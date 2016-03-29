/**
 * @brief audio recorder menu
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liu weijun
 * @date    2006-08-28
 */

#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Fwl_osMalloc.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_AUDIO_RECORD_MENU;

static T_AUDIO_RECORD_MENU *pAudioRecordMenu;
#endif

/*---------------------- BEGIN OF STATE s_av_menu ------------------------*/
T_VOID audio_recorder_menu_resume(T_VOID)
{
#ifdef SUPPORT_AUDIOREC_DENOICE

	if (8000 == gs.AudioRecordRate)
	{
		IconExplorer_AddItemWithOption(&pAudioRecordMenu->IconExplorer, 60, AK_NULL, 0, Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_SET),
				AK_NULL, AK_NULL, ICONEXPLORER_OPTION_LIST, &gs.bAudioRecDenoice);			  // 录音情景模式选择
		IconExplorer_AddItemOption(&pAudioRecordMenu->IconExplorer, 60, AK_FALSE, Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_DIS)); // 会议模式(不加降噪)
		IconExplorer_AddItemOption(&pAudioRecordMenu->IconExplorer, 60, AK_TRUE,  Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_EN));  // 听写模式(加降噪)
	}
	else
	{
		IconExplorer_DelItem(&pAudioRecordMenu->IconExplorer, 60);
	}
#endif
}

void initaudio_recorder_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    pAudioRecordMenu = (T_AUDIO_RECORD_MENU *)Fwl_Malloc(sizeof(T_AUDIO_RECORD_MENU));
    AK_ASSERT_PTR_VOID(pAudioRecordMenu, "initaudio_recorder_menu(): malloc error");

    MenuStructInit(&pAudioRecordMenu->IconExplorer);
    GetMenuStructContent(&pAudioRecordMenu->IconExplorer, mnAUDIORECORDER_MENU);

#ifdef SUPPORT_AUDIOREC_DENOICE
	if (8000 == gs.AudioRecordRate)
	{     				
        IconExplorer_AddItemWithOption(&pAudioRecordMenu->IconExplorer, 60, AK_NULL, 0, Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_SET),
   				AK_NULL, AK_NULL, ICONEXPLORER_OPTION_LIST, &gs.bAudioRecDenoice);            // 录音情景模式选择
   		IconExplorer_AddItemOption(&pAudioRecordMenu->IconExplorer, 60, AK_FALSE, Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_DIS)); // 会议模式(不加降噪)
        IconExplorer_AddItemOption(&pAudioRecordMenu->IconExplorer, 60, AK_TRUE,  Res_GetStringByID(eRES_STR_AUDIO_REC_DENOICE_EN));  // 听写模式(加降噪)
	}
#endif

	m_regResumeFunc(audio_recorder_menu_resume);
#endif
}

void exitaudio_recorder_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Free(&pAudioRecordMenu->IconExplorer);
    pAudioRecordMenu = Fwl_Free(pAudioRecordMenu);
#endif
}

void paintaudio_recorder_menu(void)
{
#ifdef SUPPORT_AUDIOREC

    IconExplorer_Show(&pAudioRecordMenu->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_recorder_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE   IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioRecordMenu->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioRecordMenu->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pAudioRecordMenu->IconExplorer))
        {
        case 10:                // 开始录音
            GE_ShadeInit();
            m_triggerEvent(M_EVT_1,pEventParm);
            break;
        case 20:                // 设置录音格式/模式
            GE_ShadeInit();
            m_triggerEvent(M_EVT_2,pEventParm);
            break;
        case 30:                // 显示录音文件，可点播
            GE_ShadeInit();
            m_triggerEvent(M_EVT_3,pEventParm);
            break;
        case 40:                // 设置采样率
            GE_ShadeInit();
            m_triggerEvent(M_EVT_4,pEventParm);
            break;
        case 50:				// 设置存储路径			
            GE_ShadeInit();
            pEventParm->w.Param1 = eAUDIOREC_PATH;
            m_triggerEvent(M_EVT_SETPATH, pEventParm);
            break;
        default:
            GE_ShadeInit();
            m_triggerEvent(M_EVT_EXIT,pEventParm);
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
