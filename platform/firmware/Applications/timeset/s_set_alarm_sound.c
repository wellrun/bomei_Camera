#include "Fwl_public.h"

#ifdef SUPPORT_SYS_SET
#include <time.h>
#include <string.h>
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_AVIPlayer.h"
#include "Fwl_Initialize.h"
#include "Eng_Alarm.h"
#include "Eng_String.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST           displayList;
    T_MSGBOX                msgbox;
    T_USTR_FILE     		filename;
	T_BOOL                  MsgFlag;
} T_ALARM_SOUND_PARM;

static T_ALARM_SOUND_PARM *pAlarmSoundParm = AK_NULL;

extern T_FILE_TYPE AudioFileType[];
#endif
/*---------------------- BEGIN OF STATE s_set_alarm_sound ------------------------*/
void initset_alarm_sound(void)
{
#ifdef SUPPORT_SYS_SET

    T_USTR_FILE pathstr, namestr;
/*    T_FILE_TYPE FileType[] = {
        FILE_TYPE_MP1, FILE_TYPE_MP2, FILE_TYPE_MP3,
        FILE_TYPE_AAC, FILE_TYPE_AMR, FILE_TYPE_WMA, FILE_TYPE_ASF, FILE_TYPE_MID,
        FILE_TYPE_MIDI, FILE_TYPE_ADPCM, FILE_TYPE_WAV, FILE_TYPE_M4A, FILE_TYPE_MP4,
        FILE_TYPE_FLAC_NATIVE, FILE_TYPE_FLAC_OGG,	FILE_TYPE_FLAC_OGA,
        FILE_TYPE_ADIF, FILE_TYPE_ADTS, FILE_TYPE_ALT, FILE_TYPE_APE, FILE_TYPE_NONE
    };
*/    
    pAlarmSoundParm = (T_ALARM_SOUND_PARM *)Fwl_Malloc(sizeof(T_ALARM_SOUND_PARM));
    AK_ASSERT_PTR_VOID(pAlarmSoundParm, "initset_alarm_sound error");

	pAlarmSoundParm->MsgFlag = AK_FALSE;

	if (0 != Utl_UStrLen(gs.AlarmClock.AlarmSoundPathName))
	{
    	Utl_USplitFilePath(gs.AlarmClock.AlarmSoundPathName, pathstr, namestr);
	}
	else
	{
		Utl_UStrCpy(pathstr, gs.DefPath[eAUDIO_PATH]);
	}
	
    DisplayList_init(&pAlarmSoundParm->displayList, pathstr, \
            Res_GetStringByID(eRES_STR_ALARM_SOND_SETUP), AudioFileType);
#endif
}

void exitset_alarm_sound(void)
{
#ifdef SUPPORT_SYS_SET

    TopBar_DisableMenuButton();
    DisplayList_Free(&pAlarmSoundParm->displayList);
    pAlarmSoundParm = Fwl_Free(pAlarmSoundParm);
#endif
}

void paintset_alarm_sound(void)
{
#ifdef SUPPORT_SYS_SET

    TopBar_EnableMenuButton();
    if (pAlarmSoundParm->MsgFlag == AK_TRUE)
    {
        MsgBox_Show(&pAlarmSoundParm->msgbox);
    }
    else
    {
    	DisplayList_Show(&pAlarmSoundParm->displayList);
    }
	
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_alarm_sound(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_U8 FileType = _SD_MEDIA_TYPE_UNKNOWN;
    T_USTR_FILE     FilePath;
	T_eBACK_STATE msgRet;
    //T_ALARM_TYPE *pAlarmClock = &(gs.AlarmClock);

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pAlarmSoundParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }


	if (pAlarmSoundParm->MsgFlag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pAlarmSoundParm->msgbox, event, pEventParm);
        switch (msgRet)
        {
        case eNext:
			gs.AlarmClock.AlarmSoundPathName[0] = 0;
            MsgBox_InitAfx(&pAlarmSoundParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAlarmSoundParm->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAlarmSoundParm->msgbox);
			break;
        case eReturn:
            pAlarmSoundParm->MsgFlag = AK_FALSE;
            DisplayList_SetRefresh(&pAlarmSoundParm->displayList, DISPLAYLIST_REFRESH_ALL);
            break;
        default:
            break;
        }
    }
    else
    {
	    DisplayListRet = DisplayList_Handler(&pAlarmSoundParm->displayList, event, pEventParm);
	    switch (DisplayListRet)
	    {
	        case eNext:
	            DisplayList_SetRefresh(&pAlarmSoundParm->displayList, DISPLAYLIST_REFRESH_ALL);
	            pFileInfo = DisplayList_Operate(&pAlarmSoundParm->displayList);
	            if (pFileInfo != AK_NULL)
	            {
	                // special deal with mp4 postfix, this file will be audio or movie file
	                FileType = Utl_GetFileType(pFileInfo->name);
	                AK_DEBUG_OUTPUT("FileType:%d\n", FileType);
	                if ((FileType > FILE_TYPE_MP4) && (FileType < FILE_TYPE_LRC) )
	                {
	                    //save pFileInfo->name and path to gs.AlarmSoundPathName
	                    Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(&pAlarmSoundParm->displayList));
	                    Utl_UStrCat(FilePath, pFileInfo->name);
	                    Utl_UStrCpy(gs.AlarmClock.AlarmSoundPathName, FilePath);
	                    gb.AlarmDataChanged = AK_TRUE;
	                    MsgBox_InitAfx(&pAlarmSoundParm->msgbox, 2, ctSUCCESS, csSELECT_OK, MSGBOX_INFORMATION);
	                }
	                else if( FileType==FILE_TYPE_MP4 )
	                {
	                    Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(&pAlarmSoundParm->displayList));
	                    Utl_UStrCat(FilePath, pFileInfo->name);
	                    if( AVIPlayer_CheckVideoFile(FilePath)==AVIPLAY_OPENERR ) //audio mp4 file
	                    {
	                        Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(&pAlarmSoundParm->displayList));
	                        Utl_UStrCat(FilePath, pFileInfo->name);
	                        Utl_UStrCpy(gs.AlarmClock.AlarmSoundPathName, FilePath);
	                        gb.AlarmDataChanged = AK_TRUE;
	                        MsgBox_InitAfx(&pAlarmSoundParm->msgbox, 2, ctSUCCESS, csSELECT_OK, MSGBOX_INFORMATION);
	                    }
	                    else
	                    {
	                        MsgBox_InitAfx(&pAlarmSoundParm->msgbox, 1, ctFAILURE, csALARM_SOUND_SEL, MSGBOX_INFORMATION);
	                    }
	                }
	                else
	                {
	                    MsgBox_InitAfx(&pAlarmSoundParm->msgbox, 1, ctFAILURE, csALARM_SOUND_SEL, MSGBOX_INFORMATION);
	                }
	                MsgBox_SetDelay(&pAlarmSoundParm->msgbox, MSGBOX_DELAY_1);
	                m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAlarmSoundParm->msgbox);
	            }
	            break;
			case eMenu:
				pAlarmSoundParm->MsgFlag = AK_TRUE;
				MsgBox_InitStr(&pAlarmSoundParm->msgbox, 0, GetCustomTitle(ctHINT), Res_GetStringByID(eRES_STR_RESTORE_DEF_ALM_SOUND), MSGBOX_QUESTION | MSGBOX_YESNO);
				break;
	        default:
	            ReturnDefauleProc(DisplayListRet, pEventParm);
	            break;
	    }
    }
#endif
    return 0;
}





