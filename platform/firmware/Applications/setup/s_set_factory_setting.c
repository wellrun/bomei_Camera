
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Ctl_Msgbox.h"
#include "Fwl_Initialize.h"
#include "Eng_ScreenSave.h"
#include "Fwl_pfDisplay.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "ctl_ebook.h"
#include "Ctl_AudioPlayer.h"
#include "eng_dataconvert.h"
#include "fwl_osfs.h"
#include "Ctl_Fm.h"
#include "lib_sdfilter.h"
#include "Fwl_rtc.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_MSGBOX        msgbox;
} T_SET_FACTORY_PARM;

static T_SET_FACTORY_PARM *pSet_Factory_Parm;

#ifdef UI_USE_ICONMENU
extern T_VOID StdbPicSetBuffer(T_VOID);
#endif

#ifdef	SUPPORT_TVOUT
extern T_VOID DisplaySwitch(DISPLAY_TYPE_DEV dstDisplayType);
#endif

extern T_U16  gbFmVolValue;
#endif

/*---------------------- BEGIN OF STATE s_set_factory_setting ------------------------*/
void initset_factory_setting(void)
{

#ifdef SUPPORT_SYS_SET



    pSet_Factory_Parm = (T_SET_FACTORY_PARM *)Fwl_Malloc(sizeof(T_SET_FACTORY_PARM));
    AK_ASSERT_PTR_VOID(pSet_Factory_Parm, "initset_factory_setting(): malloc error");

#if (SDRAM_MODE == 8)
    MsgBox_InitAfx(&pSet_Factory_Parm->msgbox, 0, ctHINT, csSYSTEM_REBOOT, MSGBOX_QUESTION | MSGBOX_YESNO);
    MsgBox_AddLine(&pSet_Factory_Parm->msgbox, GetCustomString(csSETUP_FACTORY));
#else
    MsgBox_InitAfx(&pSet_Factory_Parm->msgbox, 0, ctHINT, csSETUP_FACTORY, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
#endif
}

void exitset_factory_setting(void)
{
#ifdef SUPPORT_SYS_SET
    pSet_Factory_Parm = Fwl_Free(pSet_Factory_Parm);
#endif
}

void paintset_factory_setting(void)
{
#ifdef SUPPORT_SYS_SET
    MsgBox_Show(&pSet_Factory_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_factory_setting(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   msgRet;
    T_RES_LANGUAGE  language;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pSet_Factory_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    msgRet = MsgBox_Handler(&pSet_Factory_Parm->msgbox, event, pEventParm);
    switch(msgRet)
    {
    case eNext:
        language = gs.Lang;
        AudioPlayer_Stop();

        //Fwl_ResumeBatteryRate();
        GetDefUserdata();   //in this function, gs.Lang is set to eRES_LANG_CHINESE_SIMPLE !

         gs.bSetRTCcount = AK_TRUE;
          
        gb.AudioPlaySpeed = _SD_WSOLA_1_0;
        //Fwl_AudioSetPlaySpeed(gb.AudioPlaySpeed);

        /**close alarm*/
        Fwl_SetAlarmRtcCount(gs.AlarmClock.DayAlarm);
        gb.RtcType = RTC_ALARM;

        UserCountDownReset();
        
        //Fwl_FileDelete(_T(AUDIO_META_INFO_FILE));
        //Fwl_FileDelete(_T(AUDIOLIST_CURTPLY_FILE));        
        Fwl_FileDelete(_T(PON_CACHE_PIC));
        Fwl_FileDelete(_T(POFF_CACHE_PIC));
        Fwl_FileDelete(_T(MENU_CACHE_PIC));
        Fwl_FileDelete(_T(STDB_CACHE_PIC));

        //AVIPlayer_UpdateVideoList(); //SW10A00002690
        //APList_Update();

#ifdef UI_USE_ICONMENU
        StdbPicSetBuffer();
#endif
        
#if (SDRAM_MODE == 8)
        VME_ReTriggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
        gb.ResetAfterPowerOff = AK_TRUE;
#else
        /**Reload menu's backgound picture*/
        Menu_LoadRes();
        
#ifdef	SUPPORT_TVOUT
		DisplaySwitch(DISPLAY_LCD_0);
#endif
        gb.MainMenuRefresh = AK_TRUE;
        MsgBox_InitAfx(&pSet_Factory_Parm->msgbox, RETURN_TO_HOME, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pSet_Factory_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pSet_Factory_Parm->msgbox);
#endif
        SaveUserdata(); //manual save user config at here, in case of unnormal shut down system 
        //if(!Fwl_TvoutIsOpen())
        {
	        Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
        }

		CURRENT_FONT_SIZE = gs.FontSize;
        FontInit();
		DynamicFont_Codepage_Free();
		DynamicFont_FontLib_Free();
		Eng_FontLib_Init();
		Eng_Codepage_Init();
	    Res_SetLanguage(gs.Lang);
		
        break;
    default:
        ReturnDefauleProc(msgRet, pEventParm);
        break;
    }
#endif
    return 0;
}
