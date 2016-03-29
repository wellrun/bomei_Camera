#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOREC
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Eng_DataConvert.h"
#include "Eng_Debug.h"
#include "Fwl_Initialize.h"
#include "Eng_ImgConvert.h"
#include "Ctl_DisplayList.h"
#include "Fwl_pfKeypad.h"
#include "Eng_ScreenSave.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_DISPLAYLIST   displayList;
    T_MSGBOX        msgbox;
} T_AUDIO_RECORD_LIST;

static T_AUDIO_RECORD_LIST *pAudio_Record_List_Parm;

extern T_VOID explorer_send2audio(T_DISPLAYLIST *pDisplayList);
#endif
/*---------------------- BEGIN OF STATE s_gam_nes_list ------------------------*/
void initaudio_recorder_list(void)
{
#ifdef SUPPORT_AUDIOREC

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_AMR,
        FILE_TYPE_WAV,
		FILE_TYPE_MP3,
		FILE_TYPE_AAC,
        FILE_TYPE_NONE
    };
    
    pAudio_Record_List_Parm = (T_AUDIO_RECORD_LIST *)Fwl_Malloc(sizeof(T_AUDIO_RECORD_LIST));
    AK_ASSERT_PTR_VOID(pAudio_Record_List_Parm, "initgam_nes_list(): malloc error");
    DisplayList_init(&pAudio_Record_List_Parm->displayList, Fwl_GetDefPath(eAUDIOREC_PATH), \
            GetCustomString(csAUDIOREC_LIST_TITLE), FileType);

    //TopBar_EnableMenuButton();
    gb.bInExplorer = AK_TRUE;
#endif
}


void exitaudio_recorder_list(void)
{
#ifdef SUPPORT_AUDIOREC

    gb.bInExplorer = AK_FALSE;
    TopBar_DisableMenuButton();
    DisplayList_Free(&pAudio_Record_List_Parm->displayList);
    pAudio_Record_List_Parm = Fwl_Free(pAudio_Record_List_Parm);
#endif
}

void paintaudio_recorder_list(void)
{
#ifdef SUPPORT_AUDIOREC

    DisplayList_SetTopBarMenuIconState(&pAudio_Record_List_Parm->displayList);
    DisplayList_Show(&pAudio_Record_List_Parm->displayList);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_recorder_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOREC

    T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_USTR_FILE  file;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pAudio_Record_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

    DisplayListRet = DisplayList_Handler(&pAudio_Record_List_Parm->displayList, event, pEventParm);
    switch (DisplayListRet)
    {
        case eNext:
            DisplayList_SetRefresh(&pAudio_Record_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
            pFileInfo = DisplayList_Operate(&pAudio_Record_List_Parm->displayList);
    		
            if (pFileInfo != AK_NULL)
            {
            
            Utl_UStrCpy(file, Fwl_GetDefPath(eAUDIOREC_PATH));
    		Utl_UStrCat(file, pFileInfo->name);
    		
#ifdef SUPPORT_AUDIOPLAYER
                if (AudioPlayer_IsSupportFile(file))    //audio
                {
                    AudioPlayer_Stop();
                    explorer_send2audio(&pAudio_Record_List_Parm->displayList);    //¼ÓÈëÒôÀÖ²¥·Å
                    m_triggerEvent(M_EVT_1, pEventParm);
                }
#endif
            }
            break;

        case eMenu:
            pFileInfo = DisplayList_GetItemContentFocus(&pAudio_Record_List_Parm->displayList);
            if (pFileInfo != AK_NULL)
            {
                /**not into menu if focus path is B:\ or ..*/
                //if(!(((pFileInfo->attrib & EXPLORER_ISFOLDER) ==  EXPLORER_ISFOLDER)
                    //&& ((0 == Utl_UStrCmp(pFileInfo->name, _T("."))) 
                      //|| (0 == Utl_UStrCmp(pFileInfo->name, _T(".."))))))
                if (!(((pFileInfo->attrib&0x10) == 0x10) \
                    && (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)))
                {
                    m_triggerEvent(M_EVT_MENU, (T_EVT_PARAM *)&pAudio_Record_List_Parm->displayList);
                    DisplayList_SetRefresh(&pAudio_Record_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
            }
            break;
            
        default:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
    }
#endif
    return 0;
}

