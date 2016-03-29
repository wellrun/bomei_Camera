/**
 * @FILENAME: autotest.c
 * @BRIEF atuotest 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR lixingjian
 * @DATE 2012-02-28
 * @VERSION 1.0
 * @REF
 */ 


#include "Fwl_public.h"
#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


 #ifdef SUPPORT_AUTOTEST
 typedef struct 
 {
	 T_ICONEXPLORER  IconExplorer;
	 T_MSGBOX		 msgbox;
 } T_AUTOTEST_PARM;
 
 static T_AUTOTEST_PARM *pAutoTestParm;
 #endif
 
 /*---------------------- BEGIN OF STATE s_autotest_menu ------------------------*/
 void initautotest_menu(void)
 {
 #ifdef SUPPORT_AUTOTEST
	 pAutoTestParm = (T_AUTOTEST_PARM *)Fwl_Malloc(sizeof(T_AUTOTEST_PARM));
	 AK_ASSERT_PTR_VOID(pAutoTestParm, "initautotest_menu(): malloc error");
 
	 MenuStructInit(&pAutoTestParm->IconExplorer);
	 GetMenuStructContent(&pAutoTestParm->IconExplorer, mnAUTOTEST_MENU);
#endif	 
 }
 
 void exitautotest_menu(void)
 {
 #ifdef SUPPORT_AUTOTEST
	 IconExplorer_Free(&pAutoTestParm->IconExplorer);
	 pAutoTestParm = Fwl_Free(pAutoTestParm);
#endif
 }
 
 void paintautotest_menu(void)
 {
 #ifdef SUPPORT_AUTOTEST
	 IconExplorer_Show(&pAutoTestParm->IconExplorer);
	 GE_StartShade();
	 Fwl_RefreshDisplay();
#endif

 }
 
 unsigned char handleautotest_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
 {
 #ifdef SUPPORT_AUTOTEST
	 T_eBACK_STATE IconExplorerRet;
 
	 if (IsPostProcessEvent(event))
	 {
		 IconExplorer_SetRefresh(&pAutoTestParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
		 return 1;
	 }
 
	 IconExplorerRet = IconExplorer_Handler(&pAutoTestParm->IconExplorer, event, pEventParm);
	 switch (IconExplorerRet)
	 {
	 case eNext:
		 switch (IconExplorer_GetItemFocusId(&pAutoTestParm->IconExplorer))
		 {
		 case 0: // 启动录制
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_1, pEventParm);
			 break;
		 case 1: //正常功能测试
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_2, pEventParm);
			 break;
		 case 2: // 交叉组合测蔗
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_3, pEventParm);
			 break;
	     case 3: // 兼容性测试
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_4, pEventParm);
			 break;
		 case 4: // 压力测试
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_5, pEventParm);
			 break;
		 case 5: // 性能测试
			 GE_ShadeInit();
			 m_triggerEvent(M_EVT_6, pEventParm);
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



























