
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Initialize.h"
#include "Fwl_pfKeypad.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_DISPLAYLIST       displayList;
} T_AUDIO_SET_DEFAULT_PATH_PARM;

static T_AUDIO_SET_DEFAULT_PATH_PARM *pAudioSetDefaultPathParm;
#endif


/*---------------------- BEGIN OF STATE s_audio_set_default_path ------------------------*/
void initaudio_set_default_path(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_MP1, FILE_TYPE_MP2, FILE_TYPE_MP3,
        FILE_TYPE_AAC, FILE_TYPE_AC3, FILE_TYPE_AMR, FILE_TYPE_WMA, FILE_TYPE_ASF, FILE_TYPE_MID,
        FILE_TYPE_MIDI, FILE_TYPE_ADPCM, FILE_TYPE_WAV, FILE_TYPE_M4A, FILE_TYPE_MP4,
        FILE_TYPE_ADIF, FILE_TYPE_ADTS, FILE_TYPE_NONE
    };
    
    pAudioSetDefaultPathParm = (T_AUDIO_SET_DEFAULT_PATH_PARM *)Fwl_Malloc(sizeof(T_AUDIO_SET_DEFAULT_PATH_PARM));
    AK_ASSERT_PTR_VOID(pAudioSetDefaultPathParm, "initaudio_set_default_path error");

    DisplayList_init(&pAudioSetDefaultPathParm->displayList, Fwl_GetDefPath(eAUDIO_PATH), \
            GetCustomTitle(ctSET_DEF_PATH), FileType);
#endif
}

void exitaudio_set_default_path(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    DisplayList_Free(&pAudioSetDefaultPathParm->displayList);
    pAudioSetDefaultPathParm = Fwl_Free(pAudioSetDefaultPathParm);
#endif
}

void paintaudio_set_default_path(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    DisplayList_Show(&pAudioSetDefaultPathParm->displayList);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_set_default_path(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE DisplayListRet;
    T_MMI_KEYPAD phyKey;
    T_FILE_INFO *FileInfo;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pAudioSetDefaultPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

    DisplayListRet = DisplayList_Handler(&pAudioSetDefaultPathParm->displayList, event, pEventParm);
    switch (DisplayListRet)
    {
        case eNext:
            FileInfo = DisplayList_Operate(&pAudioSetDefaultPathParm->displayList);
            break;
            
        case eStay:
            switch (event)
            {
                case M_EVT_USER_KEY:
                    phyKey.keyID = pEventParm->c.Param1;
                    phyKey.pressType = pEventParm->c.Param2;
                    if (phyKey.pressType == PRESS_SHORT)
                    {
                        switch(phyKey.keyID)
                        {
                            case kbMENU:
                                FileInfo = DisplayList_GetItemContentFocus(&pAudioSetDefaultPathParm->displayList);
                                if (FileInfo != AK_NULL && ((FileInfo->attrib & 0x10) == 0x10) \
                                    && (Utl_UStrCmp(FileInfo->name, _T("..")) != 0) \
                                    && (!((DisplayList_GetSubLevel(&pAudioSetDefaultPathParm->displayList) == 1) \
                                    && (Utl_UStrCmp(FileInfo->name, _T(".")) == 0))))
                                {
                                    DisplayList_SetRefresh(&pAudioSetDefaultPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
                                    m_triggerEvent(M_EVT_SETPATH, (T_EVT_PARAM *)(&pAudioSetDefaultPathParm->displayList));
                                }
                                break;
                                
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
            
        default:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
    }
#endif
    return 0;
}
