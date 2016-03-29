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
} T_AUTOTEST_STOPRECORD_PARM;

extern T_BOOL autotest_recordflag;
extern T_BOOL autotest_testflag;
extern T_BOOL autotest_screen_saver_falg;





static T_AUTOTEST_STOPRECORD_PARM *pstoprecordParm;
#endif

/*---------------------- BEGIN OF STATE s_set_disp_version ------------------------*/
void initautotest_stoprecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST

    pstoprecordParm = (T_AUTOTEST_STOPRECORD_PARM *)Fwl_Malloc(sizeof(T_AUTOTEST_STOPRECORD_PARM));
    AK_ASSERT_PTR_VOID(pstoprecordParm, "initset_disp_version(): malloc error");

	MsgBox_InitAfx(&pstoprecordParm->msgbox, 2, ctHINT, csSETUP_AUTOTEST_RECODE_SAVEORNO, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitautotest_stoprecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST
    pstoprecordParm = Fwl_Free(pstoprecordParm);
    pstoprecordParm = NULL;

#endif
}

void paintautotest_stoprecord_menu(T_VOID)
{
#ifdef SUPPORT_AUTOTEST
    MsgBox_Show(&pstoprecordParm->msgbox);
    Fwl_RefreshDisplay();
#endif
}


unsigned char handleautotest_stoprecord_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUTOTEST
	T_eBACK_STATE   msgRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pstoprecordParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }
    msgRet = MsgBox_Handler(&pstoprecordParm->msgbox, event, pEventParm);
    switch(msgRet)
    {
	    case eNext:	
			MsgBox_InitStr(&pstoprecordParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
	        MsgBox_Show(&pstoprecordParm->msgbox);
	        Fwl_RefreshDisplay();
			
	        MsgBox_InitAfx(&pstoprecordParm->msgbox, 20, ctHINT, csSETUP_AUTOTEST_STOP_RECORD, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&pstoprecordParm->msgbox, MSGBOX_DELAY_0);
			AK_DEBUG_OUTPUT("save the record file\r\n");
			autotest_record_closefile(AK_TRUE);
			autotest_recordflag = AK_FALSE;
			AK_DEBUG_OUTPUT("enter screen saver\r\n");
			autotest_screen_saver_falg = AK_TRUE;
	        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pstoprecordParm->msgbox);	
			break;
			
		case eReturn:

			MsgBox_InitStr(&pstoprecordParm->msgbox, 0, GetCustomTitle(ctHINT), GetCustomString(csWAITING), MSGBOX_INFORMATION);
	        MsgBox_Show(&pstoprecordParm->msgbox);
	        Fwl_RefreshDisplay();
			
	        MsgBox_InitAfx(&pstoprecordParm->msgbox, 20, ctHINT, csSETUP_AUTOTEST_CANCEL_RECORD, MSGBOX_INFORMATION);
			MsgBox_SetDelay(&pstoprecordParm->msgbox, MSGBOX_DELAY_0);
			AK_DEBUG_OUTPUT("delt the record file\r\n");
			autotest_record_closefile(AK_FALSE);
			autotest_recordflag = AK_FALSE;
			AK_DEBUG_OUTPUT("enter screen saver\r\n");
			autotest_screen_saver_falg = AK_TRUE;
	        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pstoprecordParm->msgbox);		
			break;
			
	    default:
	        ReturnDefauleProc(msgRet, pEventParm);
	        break;
    }
#endif
    return 0;

}





























