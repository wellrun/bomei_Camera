/**
 * @FILENAME: fwl_sys_detect.c
 * @BRIEF system usb charger earphone detect
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "gbl_global.h"
#include "fwl_power.h"
#include "fwl_sys_detect.h"
#include "fwl_evtmailbox.h"
#include "Fwl_sysevent.h"
#include "Lib_event.h"
#include "Lib_state_api.h"
#include "Eng_Debug.h"
#include "Eng_ScreenSave.h"
#include "ctl_fm.h"
#include "hal_timer.h"
#include "gpio_config.h"
#include "hal_gpio.h"
#include "hal_usb_s_state.h"
#include "fwl_pfaudio.h"
#include "hal_ts.h"
#include "fwl_usb.h"
#include "drv_gpio.h"
#include "arch_mmc_sd.h"
#include "fs.h"
#include "Fwl_vme.h"
#include "fwl_sd.h"
#include "Fwl_usb_host.h"
#include "Fwl_public.h"
#include "Fwl_pfdisplay.h"

T_pCSTR usb_detect_name     = "UDISK";
T_pCSTR hp_detect_name      = "HP";
T_pCSTR sd_detect_name      = "SD";
T_pCSTR mmc_detect_name     = "MMC";
T_pCSTR charger_detect_name = "DC";
T_pCSTR powerkey_detect_name = "POWERKEY";

static T_BOOL m_bMmcDetectOpen=AK_FALSE;



T_BOOL charger_is_conneted(T_VOID)
{   
    T_BOOL connect_state;    

    return AK_FALSE;

    if((detector_get_state(charger_detect_name, &connect_state)) &&
        (connect_state))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    } 
}

/**
 * @BRIEF charger handler
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] T_U32 pin: gpio pin ID
 * @PARAM[in] T_U8 polarity: 1 means active high level. 0 means active low level.
 * @RETURN T_VOID
 * @RETVAL
 */
 
static T_VOID sys_charger_handler(T_BOOL connect_state)
{
  
}

/**
 * @BRIEF system init gpio charger callback function
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID sys_charger_detector_init(T_VOID)
{
#ifdef OS_ANYKA
    if(!detector_set_callback(charger_detect_name, sys_charger_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", charger_detect_name);
	}
#endif
}

T_VOID usb_enable_detect(T_BOOL benable)
{
#ifdef OS_ANYKA
	detector_enable(usb_detect_name, benable);
#endif
}


T_BOOL usb_is_connected(T_VOID)
{
#ifdef OS_ANYKA
    T_BOOL connect_state;    
   
    if((detector_get_state(usb_detect_name, &connect_state)) &&
        (connect_state))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }      
#else
	return AK_FALSE;
#endif
}


/**
 * @BRIEF system usb detect
 * @AUTHOR 
 * @DATE 
 * @PARAM T_VOID
 * @RETURN T_BOOL
 * @RETVAL  1: usb disk 0:not usbdisk
 */ 
T_BOOL sys_usb_detect(T_VOID)
{
#ifdef OS_ANYKA
    if(!usb_is_connected())
    {
        return AK_FALSE;
    }
	
    return usb_detect();
#else
	return AK_TRUE;
#endif
}


/**
 * @BRIEF usb detect handler
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] T_U32 pin: gpio pin ID
 * @PARAM[in] T_U8 polarity: 1 means active high level. 0 means active low level.
 * @RETURN T_VOID
 * @RETVAL
 */
static T_VOID sys_usb_detect_handler(T_BOOL connect_state)
{	
	T_SYS_MAILBOX mailbox;		

	Fwl_Print(C3,M_USB,"sys_usb_detect_handler is vorked, USB mode=%d,proc=%d\n"
		,Fwl_UsbGetMode(),Fwl_UsbGetDealProc());

//usbhost正在处理时，不处理		
	if (USB_DEAL_BEGIN == Fwl_UsbGetDealProc() 
			&& USB_DISK_MODE!= Fwl_UsbGetMode() && USB_CAMERA_MODE!=Fwl_UsbGetMode()) //有别的设备未处理完
		return ;


	// SW10A00002894
	if (AK_FALSE == connect_state && \
		eM_s_pub_usb != SM_GetCurrentSM() && \
		eM_s_usb_camera != SM_GetCurrentSM())
	{
		return;
	}

	if (AK_TRUE == ScreenSaverIsOn())
    {        
        /**exit screen saver*/
        VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_NULL);
    }

	if (connect_state)
	{
		pmu_set_ldo12(LDO12_135V);
		//pmu_set_ldopll(LDOPLL_135V);
	}

	if (connect_state)
	{
	//连接时，只需要考虑u盘
		Fwl_UsbSetDealProc(USB_DEAL_BEGIN);
		Fwl_UsbSetMode(USB_DISK_MODE);
	}
	
    mailbox.event = SYS_EVT_USB_DETECT;
    mailbox.param.c.Param1 = connect_state ? EVT_USB_PLUG_IN : EVT_USB_PLUG_OUT;
    
    AK_DEBUG_OUTPUT("usb device  %s\n\r",connect_state ? "IN" : "OUT" );
    AK_PostUniqueEvent( &mailbox, AK_NULL);
    AK_DEBUG_OUTPUT("usb device  %s,Post Unique Event End\n\r",connect_state ? "IN" : "OUT" );
}


/**
 * @BRIEF system init gpio usb callback function
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */    
T_VOID sys_usb_detector_init(T_VOID)
{
#ifdef OS_ANYKA
	if(!detector_set_callback(usb_detect_name, sys_usb_detect_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", usb_detect_name);
	}
#endif
}


/**
 * @BRIEF system init gpio flip callback function
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */ 
#ifdef SUPPORT_FLIP
T_VOID sys_init_gpio_flip(T_VOID)
{
}
#endif







T_BOOL headset_is_conneted(T_VOID)
{
#ifdef OS_ANYKA
    T_BOOL connect_state = AK_FALSE;    

    detector_get_state(hp_detect_name, &connect_state);

	return connect_state;
#endif	// OS_ANYKA
    return AK_TRUE;
}



/**
 * @BRIEF       headset detect handler
 * @AUTHOR      wangguotian
 * @DATE        2007-04-23
 * @PARAM[in]   connect_state
 * @RETURN      T_VOID
 * @RETVAL
 */
static T_VOID headset_detector_handler(T_BOOL connect_state)
{
    T_SYS_MAILBOX mailbox;

    if(connect_state)
    {
        
        //earphone in
        Fwl_Print(C3, M_FWL, "HP IS PLUG IN!!!!\n");
        mailbox.event = SYS_EVT_PINIO;
        mailbox.param.w.Param1 = PINIO_EARPHONE_EVENT;
        mailbox.param.w.Param2 = 1;
        AK_PostEvent( &mailbox);
        gb.bEarphoneStatus = AK_TRUE;

#ifdef OS_ANYKA		
        //hp connect!!The DA is open then hp in ,set connect .
        //connect or disconnect operation has delay ,it will influenced media play
        //if(DaIsEnable())
        //  analog_setsignal(INPUT_DAC, OUTPUT_HP, SIGNAL_CONNECT);
#endif
    }
    else
    {
        //earphone out
        T_SYS_MAILBOX mailbox;


        mailbox.event = SYS_EVT_PINIO;
        mailbox.param.w.Param1 = PINIO_EARPHONE_EVENT;
        mailbox.param.w.Param2 = 0;
        AK_PostEvent( &mailbox);
        gb.bEarphoneStatus = AK_FALSE;

        Fwl_Print(C3, M_FWL, "HP IS PLUG OUT!!!!\n");
    }
}

/**
 * @BRIEF system init gpio headset callback function
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_VOID
 * @RETURN T_VOID
 * @RETVAL
 */ 
T_VOID sys_headset_detector_init(T_VOID)
{
	AK_DEBUG_OUTPUT("sys_init_gpio_headset\r\n");
#ifdef OS_ANYKA    
	if(! detector_set_callback(hp_detect_name, headset_detector_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", hp_detect_name);
	}
#endif
} 


T_VOID sd_enable_detect(T_BOOL benable)
{
#ifdef OS_ANYKA
    detector_enable(sd_detect_name, benable);
#endif
}


T_BOOL sd_is_connected(T_VOID)
{
#ifdef OS_ANYKA
    T_BOOL connect_state = AK_FALSE;    

    detector_get_state(sd_detect_name, &connect_state);

	return connect_state;
#else
	return AK_FALSE;
#endif
}





/*
 * sd card interrupt state handler
 * deliver message to upper state machine
 */
static T_VOID sd_detector_handler(T_BOOL connect_state)
{
    T_SYS_MAILBOX mailbox;
	T_BOOL  isNeedPostMsg = AK_FALSE;
    
    if (connect_state)
    {		
		if (!Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO))
		{
			isNeedPostMsg = AK_TRUE;
		}
		mailbox.param.c.Param1 = EVT_SD_PLUG_IN;	 
    }
    else
    {   
		if (Fwl_Sd_GetInitState(eSD_INTERFACE_SDIO))
		{
			isNeedPostMsg = AK_TRUE;
		}
		mailbox.param.c.Param1 = EVT_SD_PLUG_OUT;
    }
	mailbox.param.c.Param2 = eSD_INTERFACE_SDIO;

	if (isNeedPostMsg)
	{
		mailbox.event = SYS_EVT_SDIO_PLUG;
		AK_PostUniqueEvent( &mailbox, AK_NULL);
	}
}


T_VOID sys_sd_detector_init(T_VOID)
{
#ifdef OS_ANYKA
	if(!detector_set_callback(sd_detect_name, sd_detector_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", sd_detect_name);
	}
#endif
}




T_VOID mmc_enable_detect(T_BOOL benable)
{
#ifdef OS_ANYKA
//    card_detect_enable(INTERFACE_SDMMC8, benable);
    detector_enable(mmc_detect_name, benable);

#endif
}


T_BOOL mmc_is_connected(T_VOID)
{
#ifdef OS_ANYKA
/*
    T_BOOL connect_state = AK_FALSE;    

	card_detector_get_state(INTERFACE_SDMMC8, &connect_state);
	return connect_state;
*/
    T_BOOL connect_state = AK_FALSE;    

    detector_get_state(mmc_detect_name, &connect_state);

	return connect_state;

#else
	return AK_FALSE;
#endif
}




/*
 * tf card interrupt state handler
 * deliver message to upper state machine
 */
static T_VOID mmc_detector_handler(T_BOOL connect_state)
{
    T_SYS_MAILBOX mailbox;
	T_BOOL isNeedPostMsg = AK_FALSE;
/*
	if (!m_bMmcDetectOpen)
		return ;
*/
    if (connect_state)
    {
		if (!Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC))
		{
			isNeedPostMsg = AK_TRUE;
		}
		mailbox.param.c.Param1 = EVT_SD_PLUG_IN;	 
    }
    else
    {   
		if (Fwl_Sd_GetInitState(eSD_INTERFACE_SDMMC))
		{
			isNeedPostMsg = AK_TRUE;
		}
		mailbox.param.c.Param1 = EVT_SD_PLUG_OUT;
    }
    
	mailbox.param.c.Param2 = eSD_INTERFACE_SDMMC;

	if (isNeedPostMsg)
	{
		mailbox.event = SYS_EVT_SDMMC_PLUG;
		AK_PostUniqueEvent( &mailbox, AK_NULL);
	}
}


T_VOID sys_mmc_detector_init(T_VOID)
{
#ifdef OS_ANYKA
/*
	card_detect_reg(INTERFACE_SDMMC8 , GPIO_MMC_DETECT, 
                      AK_TRUE, mmc_detector_handler );
*/                      
	if(!detector_set_callback(mmc_detect_name, mmc_detector_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", mmc_detect_name);
	}
	
#endif
}
static T_VOID powerkey_detector_handler(T_BOOL connect_state)
{
    T_SYS_MAILBOX   mailbox;
    T_MMI_KEYPAD mmikey;

   AK_DEBUG_OUTPUT("powerkey_detector_handler: connect state=%d\n",connect_state);

	if (connect_state)
	{
		//在充电画面下，直接重启
        if (gb.PowerOffStatus == AK_TRUE)
        {
        	AkDebugOutput("begin reset ...\n");
            Fwl_DisplayBacklightOff();
            Fwl_DisplayOff();
            
            VME_Reset();
    
            return;
        }
        
		mmikey.keyID = kbCLEAR;
		mmikey.pressType = PRESS_LONG;
		
        mailbox.event = SYS_EVT_USER_KEY;
        mailbox.param.c.Param1 = (T_U8)mmikey.keyID;
        mailbox.param.c.Param2 = (T_U8)mmikey.pressType;


        AK_PostEventEx(&mailbox, AK_NULL,AK_TRUE, AK_FALSE,AK_TRUE);
		
	}
	
}

T_VOID sys_powerkey_detector_init(T_VOID)
{
#ifdef OS_ANYKA
	if(!detector_set_callback(powerkey_detect_name, powerkey_detector_handler))
	{
        AK_DEBUG_OUTPUT("%s detector_set_callback fail\n", powerkey_detect_name);
	}
	
#endif
}
T_VOID sys_mmc_dector_open()
{
	m_bMmcDetectOpen = AK_TRUE;
}
T_VOID sys_init_app(T_VOID)
{
//    sys_headset_detector_init();
//    sys_charger_detector_init();
//    sys_sd_detector_init();
	sys_mmc_detector_init();
//	sys_mmc_dector_open();
//	Fwl_Usb_ConnectDisk();
}

