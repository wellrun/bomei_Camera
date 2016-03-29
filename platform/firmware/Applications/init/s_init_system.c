
#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Eng_DynamicFont.h"
#include "Eng_ScreenSave.h"
#include "fwl_usb.h"
#include "Ctl_Fm.h"
#include "Fwl_pfCamera.h"
#include "Eng_Alarm.h"
#include "fwl_keyhandler.h"
#include "Eng_IdleThread.h"
#include "Log_MediaPlayer.h"
#include "log_media_recorder.h"
#include "log_uvc_cam.h"
#include "Fwl_pfaudio.h"
#include "Fwl_rtc.h"
#include "Lib_state_api.h"
#include "fwl_oscom.h"
#include "fwl_pfKeypad.h"
#include "fwl_tscrcom.h"
#include "Eng_PowerOnThread.h"

#if defined(CHIP_AK3771)
#define CHIP        "AK3771"
#elif defined(CHIP_AK3760)
#define CHIP        "AK3760"
#elif defined(CHIP_AK3753)
#define CHIP        "AK3753"
#elif defined(CHIP_AK3751)
#define CHIP        "AK3751"
#elif defined(CHIP_AK3750)
#define CHIP        "AK3750"
#endif


/* define GPIO register address */
/* Only when the bit 16 of register 0x20090020 is set to 1, bits [23:16] is valid */
#define GPIO_DIR                0x20090000      /* GPIO input/output select register */
#define GPIO_INTE               0x20090004      /* GPIO interrupt control register */
#define GPIO_INTP               0x20090008      /* GPIO interrupt input polarity select register */
#define GPIO_IN                 0x2009000C      /* GPIO port input real time data value */
#define GPIO_OUT                0x20090010      /* GPIO port output real time data value */
#define GPIO_EDGE               0x20091008      /* GPIO edge/level select register */

#ifdef SYS_UPDATE_SUPPORT
    #define UPDATE_DIR              DRI_D"update"
    #define UPDATE_FILE             UPDATE_DIR "/" "update.pack"
    #define MAX_PATH                (512)
#endif

T_VOID CloseAux(T_VOID);
static T_VOID VME_SystemInit(T_VOID);


#ifdef TOUCH_SCR
T_BOOL TscrIsCalibrated(T_VOID);
#endif

/*---------------------- BEGIN OF STATE s_init_system ------------------------*/
void initinit_system(void)
{
    Fwl_Print(C3, M_INIT, "Platform Version Sword_"CHIP"_"AK_VERSION_SOFTWARE"\n\n");
}

#ifdef SYS_UPDATE_SUPPORT
static void delete_update_file()
{
    T_WCHR wcs_update_file[MAX_PATH];
    T_WCHR wcs_update_dir[MAX_PATH];

    Eng_StrMbcs2Ucs(UPDATE_FILE, wcs_update_file);
    Eng_StrMbcs2Ucs(UPDATE_DIR, wcs_update_dir);
    Fwl_FileDelete (wcs_update_file);
    Fwl_FsRmDir(wcs_update_dir);
}
#endif

void exitinit_system(void)
{
    VME_SystemInit();
#ifdef SYS_UPDATE_SUPPORT    
    delete_update_file();
#endif
}

void paintinit_system(void)
{
}

unsigned char handleinit_system(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef TOUCH_SCR

	if (!TscrIsCalibrated())
	{
		Fwl_EnableTSCR();
		
		pEventParm->c.Param8 = (T_U8)AK_TRUE;
		m_triggerEvent(M_EVT_TSCR_CALIBRATE, pEventParm); 
		return 0;
	}
#endif

    m_triggerEvent(M_EVT_1, pEventParm);

    return 0;
}

static T_VOID VME_SystemInit(T_VOID)
{
    Fwl_Print(C3, M_INIT, "VME_SystemInit");
    MPlayer_Init();
#ifdef OS_ANYKA
	IdleThread_Create();

#ifdef CAMERA_SUPPORT
	MEnc_Init();
#ifdef SUPPORT_UVC
	UVCCam_Init();
#endif
#endif
#endif
    Fwl_AudioVolumeInit();
    Fwl_MiniDelay(10);

    Fwl_KeyOpen();//keypad_open();        

#ifdef OS_ANYKA
    Fwl_keypadEnableIntr();//以前扫描接口
#endif // OS_ANYKA
       
    Fwl_Print(C3, M_INIT, "keypad initialized");

    Fwl_RTCInit();


    UserCountDownReset();

	CPowerOnThread_New();  // create and run poweron thread

    //--------------close all never used module-------------
    // CloseAux();
    //--------------close all never used module-------------
    gb.PubMsgAllow = AK_TRUE;
}

T_VOID CloseAux()
{
    //------------close all outside aux---------------
    //close fm
#ifdef SUPPORT_FM
    Ctl_FmCheck();
#endif
#ifdef CAMERA_SUPPORT
    //使能camera进入默认低功耗状态
    Fwl_CameraPowerDown();
#endif
}

/* end of files */
