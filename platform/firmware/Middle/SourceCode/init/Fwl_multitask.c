/**
 * @file apl_multitask.c
 * @brief Multitask OS entrance
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liufei
 * @date 2007-07-19
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
 
#include "akos_api.h"
#include "drv_api.h"
#include "Fwl_osMalloc.h"

#include "fwl_evtmailbox.h"

#include "Eng_string.h"
//Eric+ 2007-08-13
#include "akMMIapp.h"
#include "akmsgdispatch.h"
#include "fwl_pfdisplay.h"
#include "AKWnd.h"
#include "akos_api.h"
#include "AKAppMgr.h"
#include "fwl_oscom.h"
#include "AKMediaBGApp.h"
#include "AKPublicBGApp.h"
#include "arch_uart.h"
#include "hal_print.h"
#include "fwl_keyhandler.h"
#include "Fwl_Initialize.h"
#include "akoslay_init.h"
#include "raminit.h"
#include "Fwl_display.h"
#include "arch_init.h"
#include "l2_cache.h"
#include "fwl_rtc.h"
#include "arch_rtc.h"
#include "fwl_display.h"
#include "arch_lcd.h"
#include "Eng_IdleThread.h"


#ifdef TOUCH_SCR
#include "fwl_tscrcom.h"
#endif



#ifdef OS_ANYKA
/*======================================================================*/
//#define  TMC_STACK_SIZE    (100*1024)
#ifdef CHRONTEL
extern     T_U8 LCD_CHRONTEL_DEVICE_ID;
T_U8 i2c_temp;

#define CH7024ID  0x45
#define CH7025ID  0x55
#define CH7026ID  0x54
#define GPIO_CHRONTEL_ONOFF 27
#endif

/*======================================================================*/
//Task handle defination.
T_hTask    hTMC; //task manager controller, the first task run on akos.

//Task stack defination.
//T_U8       TMC_Task_Stack[TMC_STACK_SIZE];


/*=============================================================*/
extern T_VOID lcd_bufer_init(T_VOID);

T_VOID AK_WatchDog_CB(THREAD_LOCALE *thread_value);



#ifdef OS_ANYKA
// Clear MMIThread Queue Event
T_VOID Reset_MMIThread_Queue(T_VOID)
{
    AK_Reset_Queue(IThread_GetQueue(CMMI_GetThread()));
}
#endif

/*********************************************************************************/
/*********************************************************************************/
static T_S32 Ak_AppFrame_Init( T_VOID )
{    
    IAppMgr     *pIAppMgr;
    IMsgDispatch *pIMsgDispatch;
    IApp         *pIApp;
    T_S32         lRet = 0;
    T_AppEntry     AppEntry;
    
    
    Fwl_Print(C3, M_INIT, "--Ak_AppFrame_Init.");

    // Create AppMgr Thread
    CAppMgr_New(&pIAppMgr);
    IThread_Run((IThread *)pIAppMgr); 

    // Create MMI Thread
    CMMI_New(&pIApp);
    IAkApp_SetEvtMsk(pIApp, SYS_EVT_COMM_MSK|SYS_EVT_VATC_MSK|SYS_EVT_AUDIO_MSK|SYS_EVT_MMI_MSK);

    // Create MsgDispatch HISR
    CMsgDispatch_New(&pIMsgDispatch);
    
    AppEntry.wAppCls  = AKAPP_CLSID_MMI;
    AppEntry.pIThread = (IThread*)pIApp;

    // Register MMI Thread
    lRet = IAppMgr_AddEntry(pIAppMgr, &AppEntry, AK_TRUE);    
    IMsgDispatch_RegisterThread(pIMsgDispatch, (T_HANDLE)pIApp);
    
    //start up audio thread
    lRet = CMediaBGApp_New(AK_NULL);
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C1, M_INIT, "Create Media task failed");
        return AK_FALSE;
    }

    // start public background thread
    lRet = CPublicBGApp_New(AK_NULL);    
    if (AK_IS_FAILURE(lRet))
    {
        Fwl_Print(C1, M_INIT, "Create public task failed");
        return AK_FALSE;
    }
    

#ifdef SUPPORT_GPS    
    GpsHandle_init();
#endif   
    return AK_SUCCESS;
}

static T_VOID TMC_Task(T_U32 argc, T_VOID *argv)
{     
//wgtbupt    
#ifdef OS_ANYKA    
    T_DRIVE_INITINFO drv_info;
	T_U32 vref;
    //should change

#if (defined(CHIP_AK3771))
        drv_info.chip = CHIP_3771;
#elif (defined(CHIP_AK3753))
        drv_info.chip = CHIP_3753;
#elif (defined(CHIP_AK3750))
        drv_info.chip = CHIP_3750;
#elif (defined(CHIP_AK3760))
        drv_info.chip = CHIP_3760;
#else
#error "must define CHIP_AK37XX"
#endif

    drv_info.fRamAlloc = Fwl_MallocAndTrace;
    drv_info.fRamFree = Fwl_FreeAndTrace;
#endif

    Fwl_MallocInit();
    Fwl_MallocSupportMultithread();

#ifdef OS_ANYKA        
    drv_init(&drv_info);
//    pmu_set_ldo33(LDO33_30V);
#endif

#ifdef DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT_USB
    console_init(uiUART1, CONSOLE_USB, UART_BAUD_115200);
#else
    console_init(uiUART0, CONSOLE_UART, UART_BAUD_115200);
#endif
    console_setcallback(Fwl_Print); 
#else
    console_init(uiUART1, CONSOLE_NULL, UART_BAUD_115200);
#endif    	
    AKOSLay_init(); //gpio init must be placed after console_init for pin config
    AK_Set_WD_Callback((T_WD_CB)AK_WatchDog_CB);
    AK_Feed_Watchdog(0);

    Fwl_InitTimer();
    
#ifdef OS_ANYKA        
    FreqMgr_Init(280*1000*1000, 20*1000*1000, Idle_GetCpuUsage);
#endif
    
	vref = Efuse_Rd();
    Fwl_Print(C3, M_FWL, "vref value=[%x]",vref);
 	if (vref !=0)
 	{
 		if (0x80 == (vref & 0x80))
	        Fwl_Print(C2, M_FWL, "the vref value don't be burned into,adjusting the VREF1V5  failed!");
		else	        
	        Fwl_Print(C3, M_FWL, "adjusting the VREF1V5  succeed!");
    }
	else
        Fwl_Print(C2, M_FWL, "read vref value faild,adjusting the VREF1V5  failed!");
	

    //wgtbupt    
    rtc_init(SYSTEM_DEFAULT_YEAR);    //for file operation

    Ak_AppFrame_Init(); 

#ifdef TOUCH_SCR
    // Disable TSCR For Avoid Abnormal Event Before Calibrating
    // Enable TSCR On Standby SM
    Fwl_DisableTSCR();

    Fwl_Tscr_HW_init();
    Fwl_tscr_SetMode(TSCR_SETMODE_GRAP);
#endif

}


/********************************************************************************
                                    Main Entrance
*********************************************************************************/

void    Application_Initialize(void *first_available_memory)
{
    /* first initialize */
    Fwl_InitMMU();
    
    AK_System_Init((T_VOID *)TMC_Task, "TMCtask");

    return;
}

T_VOID AK_WatchDog_CB(THREAD_LOCALE *thread_value)
{
    T_S32        i;
    /*---------------------------> Begin to report informations <-------------------------*/
    if (thread_value->tc_current_sp != 0)
    {
        Fwl_Print(C2, M_AKFRAME, "======================== Watch Dog Died! =======================");
        Fwl_Print(C2, M_AKFRAME, "Task Name: %s",thread_value->tc_name);
        Fwl_Print(C2, M_AKFRAME, "PC Value: 0x%x",thread_value->tc_pc_value);
        Fwl_Print(C2, M_AKFRAME, "The lastest stack values:");
        for (i = 0; i < 20; i++)
        {
            Fwl_Print(C2, M_AKFRAME, "\t 0x%x" ,thread_value->stack_current_value[i]);
        }
    }
    else
    {
        Fwl_Print(C2, M_AKFRAME, "======================== Watch Dog Died! =======================");
        Fwl_Print(C2, M_AKFRAME, "HISR Name: %s",thread_value->tc_name);
        Fwl_Print(C2, M_AKFRAME, "PC Value: 0x%x",thread_value->tc_pc_value);
    }
    /*--------------------------->       ......END......        <-------------------------*/
}

#else  //for Win32


T_S32 Ak_AppFrame_Init( T_VOID )
{    
    IAppMgr *pIAppMgr;
    IMsgDispatch *pIMsgDispatch;
    IApp *pIApp;
    T_S32 lRet = 0;
    T_AppEntry AppEntry;
    

    Fwl_Print(C3, M_INIT, "--Ak_AppFrame_Init.");

    CAppMgr_New(&pIAppMgr);
    IThread_Run((IThread *)pIAppMgr);    
     CMMI_New(&pIApp);
    IAkApp_SetEvtMsk(pIApp,0xffffffff);
    CMsgDispatch_New(&pIMsgDispatch);
    AppEntry.wAppCls  = AKAPP_CLSID_MMI;
    AppEntry.pIThread = (IThread*)pIApp;
    IMsgDispatch_RegisterThread(pIMsgDispatch, (T_HANDLE)pIApp);    
    lRet = IAppMgr_AddEntry(pIAppMgr, &AppEntry, AK_TRUE);    

    //VisualTimerHandle_init();

    return AK_SUCCESS;
}

#endif

