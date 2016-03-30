
/**
 * @file
 * @brief ANYKA software
 * this file is the entry of VME platform, init the hardware
 *
 * @author Pengyu Xue
 * @date    2003-04-18
 * @author
 */
#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "fwl_evtmailbox.h"
#include "drv_api.h"
#include "fwl_oscom.h"
#include "Fwl_osfs.h"
#include "Fwl_osMalloc.h"
#include "Fwl_pfAudio.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "fwl_usb.h"
#include "Eng_ScreenSave.h"
#include "Eng_MsgQueue.h"
#include "eng_queue.h"
#include "Eng_DataConvert.h"
#include "Ctl_Msgbox.h"
#include "Lib_state.h"
//#include "boot.h"
#include "ctl_ebook.h"
#include "Eng_topbar.h"
#include "Eng_ImgDec.h"
#include "Eng_DynamicFont.h"
#include "Eng_BatWarn.h"
#include "fwl_pfKeypad.h"
#include "eng_debug.h"
//#include "mount.h"
#include "gpio_config.h"
#include "misc.h"
#include "eng_debug.h"

/* For record */
#include "gbl_global.h"
#include "log_media_recorder.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "fwl_pfcamera.h"

#include "Fwl_display.h"
#include "Fwl_sys_detect.h"
#include "Fwl_power.h"
#include "hal_detector.h"
#include "fwl_net.h"
#include "Arch_sysctl.h"
#include "hw_spi.h"

#define PUB_TIMER_SECONDS       1


#ifdef SUPPORT_AUTOTEST
extern T_VOID creat_folderandfile(void);
#endif


T_GLOBAL_S  gs;     //global save parameter
T_GLOBAL    gb;     //global temp variable


#define TIMER_MAX_VALUE        0x3fffffff
T_U32 progtime = 0;
T_U32 transtime = 0;
T_U32 copyback_count=0;

//add by wgtbupt
T_INIT_DISP_ST   stInit_Display_info = {MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT};

extern T_VOID PowerOnOff_PrintChar(T_U32 charID); //in file : s_pub_switch_off.c


#ifdef OS_ANYKA
extern T_VOID store_all_int(T_VOID);
void CMain(unsigned long argc, void *argv)
{
    InitVariable();
    
    detector_init();
    
    VME_Main();
}
#endif // OS_ANYKA

extern T_BOOL TopBar_ShowBckGrnd(T_VOID);
extern T_VOID VME_Exception_Scan(T_U8 type);
extern T_U8 *Serial_GetSysNum(T_U8 **serial_buf, T_U32 *serial_len);

T_VOID VME_Main(T_VOID)
{
    T_SYS_MAILBOX mailbox;
    T_BOOL ret;

    T_U32 ad_value;
    T_U32 voltage;
    T_U32 uCount;
#ifndef SPIBOOT //nand/sd boot
    T_U32 serial_len = 0;
    T_U8  *serial_buf = AK_NULL;
#endif

#ifdef OS_WIN32
    Fwl_InitDisplay(&stInit_Display_info);
#endif

#ifdef OS_ANYKA
    exception_set_callback(VME_Exception_Scan);
#endif

    //=========================================
    //open console print dbg window : only for win32
    #ifdef OPEN_CONSOLE_PRINTER
    {
        KillHistroyDump(AK_TRUE);
        LoadDump();
    }
    #endif
    InitConsolePrinter(0, 1);
    ConsolePrint(0, "Begin Debug Dump And Auto Write Into File = zmjdump_%d.txt\n\n", 0);
    
    AK_DEBUG_OUTPUT("VME_Main\n");
    
//加入低电检测

    rtc_set_wpinLevel(1);
    
    for(ad_value=0,uCount=0; uCount<4; ++uCount)
    {
        ad_value += analog_getvalue_bat();
    }
    
    ad_value >>= 2;       // ad_value / 4
        
    voltage = BATTERY_VOL_FORMULA(ad_value);
    
    AK_DEBUG_OUTPUT("Bat_Value = %d\n",voltage);
    
    if(voltage <= 3420)  //3.42v
    {
        AK_DEBUG_OUTPUT("Bat_Value = %d, battery value too low,stop boot...\n",
            voltage);
        while(1)
        {            
            //gpio_set_pin_level(GPIO_POWER_OFF, GPIO_LEVEL_LOW);
            rtc_set_wpinLevel(0);
        }
    }
   
    gs.Lang = eRES_LANG_CHINESE_SIMPLE;
    Eng_FontManager_Init();
    
    GetDefUserdata();    
    
    //FreqMgr_InitModul();  //move to TMC_Task

    FontInit();
    MsgQu_Init();

    

    //init event queue
    MB_Init();
    MailBoxInit();

    Gbl_SetCallbackFuncs();

//	sys_mmc_detector_init();
    ret = Fwl_InitFs();

#ifdef OS_ANYKA
    /*如果系统正在使用SD卡把总线占住，这时按下RESET去重启
       因为没有断电，SD 卡会占用总线，LCD屏用总线去初始化会失败
       如果先初始化SD卡，把总线释放，再初始化LCD就可以了*/
    /*init lcd*/
    g_Graph.LCD_NUM = MAX_LCD_NUM;

    Fwl_InitDisplay(&stInit_Display_info);
    Fwl_DisplayOn();
#if (defined(CHIP_AK3760))
    Fwl_LcdRotate(LCD_90_DEGREE);
#elif  (defined(CHIP_AK3750))   
    Fwl_LcdRotate(LCD_270_DEGREE);
#endif
	//先填充背景，避免闪lcd 内存的画面
    Fwl_FillSolid(HRGB_LAYER, POWER_ON_BACK_COLOR);     
    Fwl_RefreshDisplay();
#endif

#ifdef OS_ANYKA
#ifndef SPIBOOT //nand/sd boot
    if (ret)
    {
        Serial_GetSysNum(AK_NULL, &serial_len);
        if (serial_len > 0)
        {
            serial_buf = (T_U8 *)Fwl_Malloc(serial_len);
            if (AK_NULL != serial_buf)
            {
                memset(serial_buf, 0, serial_len);
                AK_DEBUG_OUTPUT("## system serial number:%s ##\n",Serial_GetSysNum(&serial_buf, &serial_len));
                Fwl_Free(serial_buf);
            } 
        }        
    }
#endif
#endif

#ifdef SUPPORT_AUTOTEST
     creat_folderandfile();
#endif

    ReadUserdata();

    if (gs.sysbooterrstatus) //若是异常关机，则进行磁盘检查
    {
#ifndef SPIBOOT
        Fwl_ChkDsk();
#endif
    }

    CURRENT_FONT_SIZE = gs.FontSize;

    //this must be done after ReadUserdata();
    if (!Res_Init())
    {
        if (Res_ChkOpenBackFile())             //从备份盘恢复(NandBoot/SDBoot)
        {
            if (!Res_Init())            //again init
            {
                AK_DEBUG_OUTPUT("RES_INIT ERROR:2!");
                while (1);
            }
        }
        else
        {
            AK_DEBUG_OUTPUT("RES_INIT ERROR:1!");
            while (1);
        }
    }

    /**font lib init*/
    if (!Eng_FontLib_Init())
    {
        if (Eng_FontLib_ChkOpenBackFile())     //从备份盘恢复(NandBoot/SDBoot)
        {
            if (!Eng_FontLib_Init())    //again init
            {
                AK_ASSERT_VAL_VOID(0, "Dynamic font lib init fail:2!");
                while (1);
            }
        }
        else
        {
            AK_ASSERT_VAL_VOID(0, "Dynamic font lib init fail:1!");
            while (1);
        }
    }

    /**code page init*/
    if (!Eng_Codepage_Init())
    {
        if (Eng_Codepage_ChkOpenBackFile())    //从备份盘恢复(NandBoot/SDBoot)
        {
            if (!Eng_Codepage_Init())     //again init
            {
                AK_ASSERT_VAL_VOID(0, "Dynamic codepage init fail:2!");
                while (1);
            }
        }
        else
        {
            AK_ASSERT_VAL_VOID(0, "Dynamic codepage init fail:1!");
            while (1);
        }
    }
    
    ImgDec_SetImgLibCallBack();
    GraphInit();
    SetKeyLightCount(0);

    
    /**Top bar init*/
    TopBar_Init();

	
	/*SPI总线init*/
	Bomei_HwspiInit();

    /**Waiting box init*/
    WaitBox_Init();

    /**disable lowbattery warning before detect volage*/
    BatWarnDisable();

    //ImgDec_SetImgLibCallBack();

    /**display "开机中..."*/
    PowerOnOff_PrintChar(csSWITCH_ON);
    Fwl_DisplayBacklightOn( AK_TRUE);//开LCD背光灯
    Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
    
    PublicTimerStart();

    // do userware initialization here
    m_initStateHandler();
    
#ifdef OS_ANYKA
#ifdef CAMERA_SUPPORT
    Fwl_CameraFlashInit();
    Fwl_CameraFlashClose();
#endif
    // open speaker
    gpio_set_pin_dir(GPIO_SPEAKER_EN, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_SPEAKER_EN, GPIO_LEVEL_HIGH);
    
    sys_init_app();
    
#ifdef OS_ANYKA    
    Enable_2DModule();
    Reset_2DGraphic();
#endif

#endif // OS_ANYKA

#ifdef OS_WIN32
    // register callback for events
    VME_SetCallback(m_mainloop);
#endif

#ifdef SUPPORT_NETWORK
		Fwl_Lwip_Init();
		
		Fwl_Print(C2, M_NETWORK, "%s", Fwl_Lwip_GetVersion());
#endif


    /**start state machine*/
    mailbox.event = VME_EVT_SYSSTART;
    mailbox.param.c.Param1 = 0;                       
    m_mainloop((vT_EvtCode)mailbox.event, (vT_EvtParam *)(&mailbox.param));  


    AK_DEBUG_OUTPUT("Init OK!\n");

}

T_VOID SetKeyLightCount(T_U16 second)
{
#ifdef OS_ANYKA
    if (second>0)
        open_keypadlight();
    if (second==0)
        close_keypadlight();

    gb.KeyLightCount = second;
#endif
}

T_VOID KeyLightCountDecrease(T_U32 millisecond)
{
    T_U32 second = millisecond / 1000;
#ifdef OS_ANYKA
    if (gb.KeyLightCount==0)
        return;
    if (gb.KeyLightCount<=second){
        gb.KeyLightCount = 0;
        close_keypadlight();
    }
    else gb.KeyLightCount -= second;
#endif
}

T_VOID PublicTimerStart(T_VOID)
{
    PublicTimerStop();
    gb.s_public_timer_id = Fwl_SetTimerSecond(PUB_TIMER_SECONDS, AK_TRUE);
    AK_DEBUG_OUTPUT("public timer id = %d", gb.s_public_timer_id);
}

T_VOID PublicTimerStop(T_VOID)
{
    if (gb.s_public_timer_id != ERROR_TIMER)
    {
        Fwl_StopTimer(gb.s_public_timer_id);
        gb.s_public_timer_id = ERROR_TIMER;
    }
}

T_VOID VME_Reset(T_VOID)
{
#ifdef OS_ANYKA

  soft_reset();

#endif
}

T_VOID VME_Exception_Scan(T_U8 type)
{
    //dump audio decoder buffer if necessary
    Fwl_Print(C3,M_PUBLIC,"====beg dump audio data====\n");
    Fwl_Print(C3,M_PUBLIC,"====end dump audio data====\n");

    //memory scan
    Fwl_Print(C3,M_PUBLIC,"====beg scan memory status=====\n");
    Fwl_RamBeyondMonitor(250);
    Fwl_RamWilderMonitorGetbyTimer(250);
    Fwl_Print(C3,M_PUBLIC,"====pass memory scan=====\n");

    //thread enum
    #ifdef OS_ANYKA
    Fwl_Print(C3,M_PUBLIC,"====beg enum thread status=====\n");
    AK_List_Task();
    Fwl_Print(C3,M_PUBLIC,"====end enum thread status=====\n");
    #endif

    //other scan
}


