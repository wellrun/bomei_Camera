/**
 * @FILENAME: autotest_record.c
 * @BRIEF atuotest record script file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR lixingjian
 * @DATE 2012-02-28
 * @VERSION 1.0
 * @REF
 */ 
#include "Fwl_public.h"
#include "Eng_DynamicFont.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Eng_DataConvert.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "autotest_record_func.h"

#ifdef SUPPORT_AUTOTEST

typedef struct
{
    T_MSGBOX        msgbox;
} T_AUTOTESTRECORD_PARM;

extern T_BOOL autotest_recordflag;
extern T_BOOL autotest_testflag;
extern T_BOOL autotest_screen_saver_falg;





static T_AUTOTESTRECORD_PARM *precordParm;
#endif

/*---------------------- BEGIN OF STATE s_set_disp_version ------------------------*/
void initautotest_startrecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST

    precordParm = (T_AUTOTESTRECORD_PARM *)Fwl_Malloc(sizeof(T_AUTOTESTRECORD_PARM));
    AK_ASSERT_PTR_VOID(precordParm, "initset_disp_version(): malloc error");

	MsgBox_InitAfx(&precordParm->msgbox, 20, ctHINT, csSETUP_AUTOTEST_RECORD, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitautotest_startrecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST
    precordParm = Fwl_Free(precordParm);
#endif
}

void paintautotest_startrecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST
    MsgBox_Show(&precordParm->msgbox);
    Fwl_RefreshDisplay();
#endif
}


unsigned char handleautotest_startrecord_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUTOTEST
	T_eBACK_STATE   msgRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&precordParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }
    msgRet = MsgBox_Handler(&precordParm->msgbox, event, pEventParm);
    switch(msgRet)
    {
    case eNext:	
		MsgBox_InitStr(&precordParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
        MsgBox_Show(&precordParm->msgbox);
        Fwl_RefreshDisplay();
		
		AK_DEBUG_OUTPUT("handleautotest_startrecord_menu0: %x\r\n",autotest_recordflag);
		if(autotest_recordflag == AK_TRUE)
		{
			AK_DEBUG_OUTPUT("**********************autotest recording********************** \r\n");
	        MsgBox_InitAfx(&precordParm->msgbox, 20, ctHINT, csSETUP_AUTOTEST_RECORDING, MSGBOX_INFORMATION);
		    MsgBox_SetDelay(&precordParm->msgbox, MSGBOX_DELAY_0);

			m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&precordParm->msgbox);
			break;
		}

		if(autotest_testflag == AK_TRUE)
		{
			AK_DEBUG_OUTPUT("**********************autotest testing********************** \r\n");
			MsgBox_InitAfx(&precordParm->msgbox, 20, ctHINT, csSETUP_AUTOTEST_TESTING_NOTTEST, MSGBOX_INFORMATION);
		    MsgBox_SetDelay(&precordParm->msgbox, MSGBOX_DELAY_0);
			m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&precordParm->msgbox);
			break;
		}

		autotest_screen_saver_falg = AK_FALSE;
        MsgBox_InitStr(&precordParm->msgbox, 20, GetCustomTitle(ctHINT), Res_GetStringByID(eRES_STR_START_AUTOTEST_RECORD_READY), MSGBOX_INFORMATION);
		autotest_recordflag = AK_TRUE;
		MsgBox_SetDelay(&precordParm->msgbox, MSGBOX_DELAY_0);
		
		AK_DEBUG_OUTPUT("handleautotest_startrecord_menu1 \r\n");
        autotest_record_openfile();
		AK_DEBUG_OUTPUT("handleautotest_startrecord_menu2 \r\n");
		

        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&precordParm->msgbox);
		
		break;
    default:
        ReturnDefauleProc(msgRet, pEventParm);
        break;
    }
#endif
    return 0;

}





























