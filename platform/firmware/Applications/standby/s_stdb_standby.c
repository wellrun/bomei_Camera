
#include "Eng_ScreenSave.h"
#include "Eng_BatWarn.h"
#include "Fwl_Image.h"
#include "Eng_ImgConvert.h"
#include "Eng_KeyMapping.h"
#include "Eng_MsgQueue.h"
#include "Ctl_MsgBox.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_Time.h"
#include "Fwl_osFS.h"
#include "Eng_ImgDec.h"
#include "Eng_topbar.h"
#include "Lib_state.h"
#include "Ctl_AudioPlayer.h"
#include "eng_math.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_AkBmp.h"
#include "fwl_usb.h"
#include "Ctl_Fm.h"
#include "fwl_sys_detect.h"
#include "Eng_DataConvert.h"
#include "Lib_geshade.h"
#include "fwl_power.h"
#include "fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "eng_imgconvert.h"
#include "fwl_display.h"
#include "fwl_oscom.h"
#include "fwl_usb_host.h"
#include "Eng_KeyTranslate.h"



#ifdef TOUCH_SCR
#include "fwl_tscrcom.h"
#endif

#ifdef UI_USE_ICONMENU
#define STATE_BAR_WIDTH             (Fwl_GetLcdWidth()-29)
#define STATE_BAR_HEIGHT            20
#define STATE_BAR_LEFT              0
#define STATE_BAR_TOP               0

#define BATTERY_SHOW_WIDTH          29
#define BATTERY_SHOW_HEIGHT         24
#define BATTERY_SHOW_LEFT           STATE_BAR_WIDTH
#define BATTERY_SHOW_TOP            STATE_BAR_TOP

#define ICON_SHOW_WIDTH             Fwl_GetLcdWidth()
#define ICON_SHOW_HEIGHT            Fwl_GetLcdHeight()-STATE_BAR_HEIGHT
#define ICON_SHOW_LEFT              0
#define ICON_SHOW_TOP               STATE_BAR_HEIGHT

#define ICON_WIDTH                  147
#define OTHER_REFRESH_NONE          0x00
#define OTHER_REFRESH_ALL           0xff
#define OTHER_REFRESH_MP3_STATU     0x01
#define OTHER_REFRESH_SND           0x02
#define OTHER_REFRESH_VOLUME        0x04
#define OTHER_REFRESH_VOLTAGE       0x08
#define OTHER_REFRESH_TIME          0x10
#define OTHER_REFRESH_TIMEDOT       0x20
#define OTHER_REFRESH_BKGRND        0x40
#define OTHER_REFRESH_FRAME         0x80

#define BATTERY_ICON_QTY            0x05

#ifdef ICONMENU_VERTICAL_ICON
#define STAT_ICON_DISTANCE          3
#define FIRST_STAT_ICON_X_POS       230
#else
#define STAT_ICON_DISTANCE          0x03
#define FIRST_STAT_ICON_POS         0x28
#endif // ICONMENU_VERTICAL_ICON

#define STAT_FRAME_DISTANCE         5
#define FIRST_STAT_ICON_Y_POS       4

typedef struct {
    T_RECT      IconAttachtRect;
    T_RECT      TopBarRect;
    T_RECT      iconRect;
    T_RECT      batteryRect;
    T_RECT      timeRect;
    T_pCDATA    pIconAttachBkImg;
    T_RECT      scBarRect;
    T_RECT      scrBarTopRect;      // under battery icon and on the top of scrbar
    T_pCDATA    pOtherBkImg;
    //T_pCDATA    pDefBkImg;
    T_pCDATA    pBatteryImg[BATTERY_ICON_QTY];
} T_STDB_RES_PARM;
#endif

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
#ifdef UI_USE_ICONMENU
    T_ICONMENU      IconMenu;
    T_U8            *pUserBkImgBuf;
    T_STDB_RES_PARM stdbRes;
    T_BOOL          imgLoadFlag;
    T_U8            otherRefreshFlag;
    T_BOOL          timeDotFlag;
    T_BOOL          bEnterFirst;
#endif
} T_STDB_PARM;

static T_STDB_PARM *pStdbParm = AK_NULL;
static T_RES_LANGUAGE StdbyLang;
static T_U8 KeyHoldNum = 0;


T_VOID StandbyFree(T_VOID);
static T_VOID Standby_ResumeFunc(T_VOID);
static T_VOID Standby_SuspendFunc(T_VOID);
#ifdef UI_USE_ICONMENU
T_VOID StdbPicSetBuffer(T_VOID);
static T_VOID StdbShwBattery(const T_STDB_RES_PARM *pStdbRes);
static T_VOID Standy_GetRes(T_STDB_RES_PARM *pStdbRes);
static T_BOOL initStdbResRect(T_STDB_RES_PARM *pStdbRes);
static T_BOOL Standby_SetTimeStrRect(T_STDB_RES_PARM *pStdbRes);
static T_VOID stdbShwOther(T_VOID);
//static T_VOID stdbShwOtherBckgrnd(T_pCDATA pBckgrnd, T_POS left, T_POS top);
static T_VOID stdbRefreshTime(const T_STDB_RES_PARM *pStdbRes, T_BOOL drawedBckgrnd);
static T_VOID stdbRefreshTimeDot(const T_STDB_RES_PARM *pStdbRes, T_BOOL drawFlag, T_BOOL drawedBckgrnd);
static T_VOID stdbSetOtherRefresh(const T_U8 freshFlag);
static T_U8 stdbGetOtherRefresh(T_VOID);
T_VOID Standby_LoadUserBkImg(T_VOID);
T_VOID Standby_FreeUserBkImg(T_VOID);


static T_VOID Standby_SetBkImg(T_U8 *pImage);

#endif


#ifdef SUPPORT_AUTOTEST
extern T_BOOL autotest_screen_saver_falg;
extern T_BOOL autotest_recordflag;
extern T_BOOL autotest_testflag;


static T_VOID Autotest_setfocus(T_VOID)
{
	if(autotest_screen_saver_falg == AK_TRUE)
	{
		IconMenu_SetItemFocus(&pStdbParm->IconMenu, 8);
		autotest_screen_saver_falg = AK_FALSE;
	}
}
#endif



static T_VOID Standby_EnterFirst(T_EVT_CODE event, T_EVT_PARAM *pEventParm);


/*---------------------- BEGIN OF STATE s_stdb_standby ------------------------*/
void initstdb_standby(void)
{
    AK_DEBUG_OUTPUT("standby state\n");
	Eng_SetKeyTranslate(Eng_StandbyTranslate);//set key translate function

    // AK_DEBUG_OUTPUT("initstdb_standby remain=0x%x\n", Fwl_GetRemainRamSize());
    pStdbParm = (T_STDB_PARM *)Fwl_Malloc(sizeof(T_STDB_PARM));
    AK_ASSERT_PTR_VOID(pStdbParm, "initstdb_standby(): malloc error");

    pStdbParm->pUserBkImgBuf = AK_NULL;
    pStdbParm->bEnterFirst = AK_TRUE;

    /**Get menu's common resource*/
    Menu_LoadRes();

    StdbyLang = gs.Lang;

#ifdef UI_USE_ICONMENU
    TopBar_DisableShow();

    Standy_GetRes(&pStdbParm->stdbRes);
    if (initStdbResRect(&pStdbParm->stdbRes) == AK_FALSE)
        return;

    pStdbParm->imgLoadFlag = AK_FALSE;
    stdbSetOtherRefresh(OTHER_REFRESH_ALL);
    pStdbParm->timeDotFlag = AK_TRUE;

    IconMenu_Init(&pStdbParm->IconMenu, pStdbParm->stdbRes.IconAttachtRect, pStdbParm->stdbRes.iconRect, pStdbParm->stdbRes.scBarRect);
    IconMenu_SetIconAttachStyle(&pStdbParm->IconMenu, ICONMENU_ALIGN_HCENTER | ICONMENU_ALIGN_VCENTER, COLOR_BLACK, COLOR_BLUE, AK_NULL, AK_NULL);
    
    IconMenu_SetOtherShowCallBack(&pStdbParm->IconMenu, stdbShwOther);
    IconMenu_AddOtherRect(&pStdbParm->IconMenu, pStdbParm->stdbRes.TopBarRect);
   
    
    Standby_LoadUserBkImg();

    
        
#ifdef ICONMENU_VERTICAL_ICON
    IconMenu_SetItemMatrix(&pStdbParm->IconMenu, 12, 1);
    IconMenu_SetItemFocus(&pStdbParm->IconMenu, 2);
#else

    IconMenu_SetItemMatrix(&pStdbParm->IconMenu, 3, 4);

    GetMainMenuContent(mnMAIN_MENU, &pStdbParm->IconMenu);

	if (pStdbParm->IconMenu.ItemRow > pStdbParm->IconMenu.pageRow)
	{
		pStdbParm->IconMenu.IconRect.width = Fwl_GetLcdWidth() - g_Graph.BScBarWidth;
	    IconMenu_AddOtherRect(&pStdbParm->IconMenu, pStdbParm->stdbRes.scBarRect);
	    IconMenu_AddOtherRect(&pStdbParm->IconMenu, pStdbParm->stdbRes.scrBarTopRect);
	}

    IconMenu_SetItemFocus(&pStdbParm->IconMenu, 4);

#endif // ICONMENU_VERTICAL_ICON

#else   /* else of UI_USE_ICONMENU */
    MenuStructInit(&pStdbParm->IconExplorer);
    GetMenuStructContent(&pStdbParm->IconExplorer, mnMAIN_MENU);
#endif  /* end of UI_USE_ICONMENU */

    m_regResumeFunc(Standby_ResumeFunc);
    m_regSuspendFunc(Standby_SuspendFunc);
    UserCountDownReset();

    TopBar_ResetShowTimeCount();


    /**usb detect regiset must be after standby state init, otherwise standby state will lost usb plug in event when power on*/
#ifdef OS_ANYKA
	Fwl_Usb_ConnectDisk();
#ifndef  DEBUG_OUTPUT_USB
    sys_usb_detector_init();
    sys_powerkey_detector_init();
#endif	
#endif // OS_ANYKA

#ifdef UI_USE_ICONMENU
	IconMenu_SetIconShowFlag(&pStdbParm->IconMenu, AK_TRUE);
#endif

    AK_DEBUG_OUTPUT("exit initstdb_standby\r\n");
}

void exitstdb_standby(void)
{
	Eng_SetDefKeyTranslate(); //restore key translate funciton to default
    StandbyFree();
}

void paintstdb_standby(void)
{
#ifdef UI_USE_ICONMENU
    if (pStdbParm->imgLoadFlag == AK_TRUE)
    {
        Standby_LoadUserBkImg();

        pStdbParm->imgLoadFlag = AK_FALSE;
    }

    if (gb.MainMenuRefresh)
    {
        ResetMainMenuText(mnMAIN_MENU, &pStdbParm->IconMenu);
        gb.MainMenuRefresh = AK_FALSE;
    }

    IconMenu_Show(&pStdbParm->IconMenu);
#else   /* else of UI_USE_ICONMENU */
    IconExplorer_Show(&pStdbParm->IconExplorer);
#endif  /* end of UI_USE_ICONMENU */

    GE_StartShade();
    Fwl_RefreshDisplay();	
}

unsigned char handlestdb_standby(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_MMI_KEYPAD phyKey;
#ifdef UI_USE_ICONMENU
    T_U8        FocusId;
    T_eBACK_STATE IconMenuRet;
#else   /* else of UI_USE_ICONMENU */
    T_eBACK_STATE IconExplorerRet;
#endif  /* end of UI_USE_ICONMENU */


#ifdef SUPPORT_AUTOTEST
	Autotest_setfocus();
#endif

    /*the following code used for power off if hold key was locked when power on*/
    if (KeyHoldNum < 2)
    {
        if (KeyHoldNum == 0)
        {
            KeyHoldNum++;
            return 0;
        }
        else if(KeyHoldNum == 1)
        {
            KeyHoldNum++;
        }
    }

    if (IsPostProcessEvent(event))
    {
        if (event != M_EVT_Z09COM_SYS_RESET)
        {
#ifdef UI_USE_ICONMENU
            IconMenu_SetRefresh(&pStdbParm->IconMenu, ICONMENU_REFRESH_ALL);
            stdbSetOtherRefresh(OTHER_REFRESH_ALL);
#else   /* else of UI_USE_ICONMENU */
            IconExplorer_SetRefresh(&pStdbParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
#endif  /* end of UI_USE_ICONMENU */
            return 1;
        }
    }

    Standby_EnterFirst(event, pEventParm);

    //alarm arrived

     if((M_EVT_Z09COM_SYS_RESET== event || M_EVT_RETURN == event) && (EVT_RTC_TRIGGER == pEventParm->c.Param8))
     {
        AK_DEBUG_OUTPUT("go to alarm");


        m_triggerEvent(M_EVT_ALARM, pEventParm);      
        return 0;
     }

#ifdef TOUCH_SCR	// to calibrate touch screen after reset factory setting
    if (0 == gs.matrixPtr.X[0] && 0 == gs.matrixPtr.X[1] && 0 == gs.matrixPtr.X[2] )
    {
		Fwl_EnableTSCR();
	
        pEventParm->c.Param8 = (T_U8)AK_TRUE;
        m_triggerEvent(M_EVT_TSCR_CALIBRATE, pEventParm); 
        return 0;
    }
#endif


	m_triggerEvent(M_EVT_3, pEventParm);





#ifdef UI_USE_ICONMENU
    //power off when press clear key for a long time in anystate
    if (event == M_EVT_USER_KEY)
    {
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;
        if (phyKey.keyID == kbVOICE_UP || phyKey.keyID == kbVOICE_DOWN)
        {
            IconMenu_SetRefresh(&pStdbParm->IconMenu, ICONMENU_REFRESH_OTHER);
            stdbSetOtherRefresh(OTHER_REFRESH_VOLUME | OTHER_REFRESH_SND);
            return 0;
        }
        else if (MappingStdbyKey(phyKey)==fkSTDBY_POWEROFF)
        {
			
#ifdef SUPPORT_AUTOTEST
			//为了自动化测试工具设置在待机介面上长按一个返回键时不关机
			if((autotest_recordflag == AK_TRUE) || (autotest_testflag == AK_TRUE))
			{
				return 0;  
			}
#endif
            VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_Z99COM_POWEROFF, (T_U32)AK_NULL);
            return 0;
        }
    }


    IconMenuRet = IconMenu_Handler(&pStdbParm->IconMenu, event, pEventParm);
    switch (IconMenuRet) {
        case eNext:
            IconMenu_SetRefresh(&pStdbParm->IconMenu, ICONMENU_REFRESH_ALL);
            stdbSetOtherRefresh(OTHER_REFRESH_ALL);
            FocusId = IconMenu_GetFocusId(&pStdbParm->IconMenu);

            switch (FocusId) {
#ifdef SUPPORT_AUDIOPLAYER
                case eMAIN_ICON_AUDIO_PLAYER: //audio player
                    GE_ShadeInit();
                    pEventParm = (T_EVT_PARAM *)((T_U32)(AK_TRUE));
                    m_triggerEvent(M_EVT_1, pEventParm);
                    break;
#endif
#ifdef SUPPORT_FM
                case eMAIN_ICON_FM:     //FM
                    m_triggerEvent(M_EVT_2, pEventParm);
                    break;
#endif
#ifdef CAMERA_SUPPORT
                case eMAIN_ICON_CAMERA: // camera
                        m_triggerEvent(M_EVT_3, pEventParm);
                    
                    break;
#endif
#ifdef SUPPORT_AUDIOREC
                case eMAIN_ICON_AUDIO_RECORDER: //audio recorder
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_11, pEventParm);
                    break;
#endif
#ifdef SUPPORT_VIDEOPLAYER
                case eMAIN_ICON_VIDEO_PLAYER: // video player
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_6, pEventParm);
                    break;
#endif
#ifdef SUPPORT_IMG_BROWSE
                case eMAIN_ICON_IMG_BROWSE: //image browser
                    //GE_ShadeInit();
                    m_triggerEvent(M_EVT_4, pEventParm);
                    break;
#endif
#ifdef SUPPORT_EBK
                case eMAIN_ICON_EBK: //ebook
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_7, pEventParm);
                    break;
#endif
#ifdef INSTALL_GAME
                case eMAIN_ICON_GAME: //game
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_5, pEventParm);
                    break;
#endif
#ifdef SUPPORT_TOOLBOX
                case eMAIN_ICON_TOOL_BOX: //tool box
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_8, pEventParm);
                    break;
#endif
#ifdef SUPPORT_EXPLORER
                case eMAIN_ICON_EXPLORER: //explorer
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_10, pEventParm);
                    break;
#endif
#ifdef SUPPORT_SYS_SET
                case eMAIN_ICON_SYSTEM_SET: // system setup
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_9, pEventParm);
                    break;
#endif
#if 0
                case eMAIN_ICON_TIME_SET: // time set
                    m_triggerEvent(M_EVT_12, pEventParm);
                    break;
#endif                   

#ifdef SUPPORT_FLASH
			   case eMAIN_ICON_FLASH_PLAYER: // flash player
                    m_triggerEvent(M_EVT_14, pEventParm);
                    break;
#endif
#ifdef SUPPORT_NETWORK
				case eMAIN_ICON_NETWORK: // network
					m_triggerEvent(M_EVT_NETWORK, pEventParm);
					break;
#endif

                default:
                    break;
            }
            break;
        case eReturn:
            break;
        case eHome:
            break;
        default:
            ReturnDefauleProc(IconMenuRet, pEventParm);
            break;
    }

    if (event == M_EVT_PUB_TIMER)
    {
        if (pStdbParm->timeDotFlag)
        {
            pStdbParm->timeDotFlag = AK_FALSE;
        }
        else
        {
            pStdbParm->timeDotFlag =  AK_TRUE;
        }

        if (IconMenu_GetIconShowFlag(&pStdbParm->IconMenu))
        {
            IconMenu_SetRefresh(&pStdbParm->IconMenu, ICONMENU_REFRESH_OTHER);
            stdbSetOtherRefresh(OTHER_REFRESH_TIMEDOT | OTHER_REFRESH_TIME | \
                    OTHER_REFRESH_VOLTAGE | OTHER_REFRESH_MP3_STATU);
        }
    }
    else if (event != M_EVT_PUB_TIMER)
    {
        if ((IconMenu_GetRefreshFlag(&pStdbParm->IconMenu) & ICONMENU_REFRESH_OTHER) == ICONMENU_REFRESH_OTHER)
        {
            stdbSetOtherRefresh(OTHER_REFRESH_ALL);
        }
    }
#else   /* else of UI_USE_ICONMENU */
    //power off when press clear key for a long time in anystate
    if (event == M_EVT_USER_KEY)
    {
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;
        if (MappingStdbyKey(phyKey)==fkSTDBY_POWEROFF)
        {
            VME_ReTriggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
            return 0;
        }
    }


    IconExplorerRet = IconExplorer_Handler(&pStdbParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
        case eNext:
            switch (IconExplorer_GetItemFocusId(&pStdbParm->IconExplorer))
            {
#ifdef SUPPORT_AUDIOPLAYER
                case eMAIN_ICON_AUDIO_PLAYER: //audio player
                    pEventParm = (T_EVT_PARAM *)((T_U32)(AK_TRUE));
                    m_triggerEvent(M_EVT_1, pEventParm);
                    break;
#endif
#ifdef SUPPORT_FM
                case eMAIN_ICON_FM:     //FM
                    m_triggerEvent(M_EVT_2, pEventParm);
                    break;
#endif
#ifdef CAMERA_SUPPORT
                case eMAIN_ICON_CAMERA: // camera
                    m_triggerEvent(M_EVT_3, pEventParm);
                    break;
#endif
#ifdef SUPPORT_AUDIOREC
                case eMAIN_ICON_AUDIO_RECORDER: //audio recorder
                    m_triggerEvent(M_EVT_11, pEventParm);
                    break;
#endif
#ifdef SUPPORT_VIDEOPLAYER
                case eMAIN_ICON_VIDEO_PLAYER: // video player
                    m_triggerEvent(M_EVT_6, pEventParm);
                    break;
#endif
#ifdef SUPPORT_IMG_BROWSE
                case eMAIN_ICON_IMG_BROWSE: //image browser
                    m_triggerEvent(M_EVT_4, pEventParm);
                    break;
#endif
#ifdef SUPPORT_EBK
                case eMAIN_ICON_EBK: //ebook
                    m_triggerEvent(M_EVT_7, pEventParm);
                    break;
#endif
#ifdef INSTALL_GAME
                case eMAIN_ICON_GAME: //game
                    m_triggerEvent(M_EVT_5, pEventParm);
                    break;
#endif
#ifdef SUPPORT_TOOLBOX
                case eMAIN_ICON_TOOL_BOX: //tool box
                    m_triggerEvent(M_EVT_8, pEventParm);
                    break;
#endif
#ifdef SUPPORT_EXPLORER
                case eMAIN_ICON_EXPLORER: //explorer
                    m_triggerEvent(M_EVT_10, pEventParm);
                    break;
#endif
#ifdef SUPPORT_SYS_SET
                case eMAIN_ICON_SYSTEM_SET: // system setup
                    m_triggerEvent(M_EVT_9, pEventParm);
                    break;
#endif
                case eMAIN_ICON_TIME_SET: // time set
                    m_triggerEvent(M_EVT_12, pEventParm);
                    break;
#ifdef SUPPORT_FLASH
			   case eMAIN_ICON_FLASH_PLAYER: // flash player
                    m_triggerEvent(M_EVT_14, pEventParm);
                    break;
#endif
                default:
                    break;
            }
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif  /* end of UI_USE_ICONMENU */

    return 0;
}

T_VOID StandbyFree(T_VOID)
{
#ifdef UI_USE_ICONMENU
    IconMenu_Free(&pStdbParm->IconMenu);
#else   /* else of UI_USE_ICONMENU */
    IconExplorer_Free(&pStdbParm->IconExplorer);
#endif  /* end of UI_USE_ICONMENU */
    Menu_FreeRes();

    Standby_FreeUserBkImg();

    pStdbParm = Fwl_Free(pStdbParm);
}

static T_VOID Standby_EnterFirst(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_BOOL  bTouched = AK_FALSE;
#ifdef  TOUCH_SCR   
   // if (TvOut_IsOpen())
		
    if(M_EVT_TOUCH_SCREEN == event && eTOUCHSCR_DOWN == pEventParm->s.Param1)
    {
        bTouched = AK_TRUE;
    }
#endif

    

    if (AK_TRUE == pStdbParm->bEnterFirst)
    {
        if (M_EVT_USER_KEY == event || bTouched)
        {
            pStdbParm->bEnterFirst = AK_FALSE;
            
            /**stop audio*/
            //Fwl_AudioDisableDA();
            TopBar_Show(TB_REFRESH_AUDIO_STATUS);
        }
    }
}

static T_VOID Standby_ResumeFunc(T_VOID)
{
	Eng_SetKeyTranslate(Eng_StandbyTranslate);//set key translate function

#ifdef UI_USE_ICONMENU
    TopBar_DisableShow();
    Menu_LoadRes();
    Standy_GetRes(&pStdbParm->stdbRes);

    GetMainMenuContent(mnMAIN_MENU, &pStdbParm->IconMenu);
    
    if (AK_NULL == pStdbParm->pUserBkImgBuf)
    {
        Standby_LoadUserBkImg();
    }

    Standby_SetTimeStrRect(&pStdbParm->stdbRes);
	
#ifdef SUPPORT_AUTOTEST
	Autotest_setfocus();
#endif

#else   /* else of UI_USE_ICONMENU */
    if (StdbyLang != gs.Lang) // If change language
    {
        IconExplorer_Free(&pStdbParm->IconExplorer);

        MenuStructInit(&pStdbParm->IconExplorer);
        GetMenuStructContent(&pStdbParm->IconExplorer, mnMAIN_MENU);

        StdbyLang = gs.Lang;
    }
#endif  /* end of UI_USE_ICONMENU */
}

static T_VOID Standby_SuspendFunc(T_VOID)
{
	Eng_SetDefKeyTranslate(); //restore key translate funciton to default

#ifdef UI_USE_ICONMENU
    IconMenu_SetRefresh(&pStdbParm->IconMenu, ICONMENU_REFRESH_ALL);
    stdbSetOtherRefresh(OTHER_REFRESH_ALL);
#else   
    IconExplorer_SetRefresh(&pStdbParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
#endif  

#ifdef UI_USE_ICONMENU
    TopBar_DisableShow();
#else   /* else of UI_USE_ICONMENU */
#endif  /* end of UI_USE_ICONMENU */
}

#ifdef UI_USE_ICONMENU
T_VOID StdbPicSetBuffer(T_VOID)
{
    pStdbParm->imgLoadFlag = AK_TRUE;
}

/**
 * @brief Load image as standby's back ground picture
 *
 * @author @b zhengwenbo
 *
 * @author
 * @date 2006-09-6
 * @param [out]T_U8 *image_buffer: image data buffer
 * @param  [in]T_pCSTR filename: bmp file's path
 * @param  [in]T_U16 dwidth: width of the bmp picture
 * @param  [in]T_U16 dheight: height of the bmp picture
 * @return  void
 */
static T_VOID Standby_LoadImage(T_U8 **ppimage_buffer, T_pCWSTR filename, T_U16 dwidth, T_U16 dheight)
{
    T_U32 scale;
    T_USTR_FILE file_path;
    T_U8 *bmp_buf = AK_NULL;
    T_BOOL img_support = AK_FALSE;
    T_U32 num;
    T_U32   imgLen;
    T_pDATA pData = AK_NULL;

    Utl_UStrCpy(file_path, (T_U16 *)filename);

    if (AK_TRUE == gs.UserStdbyBkImg)
    {
		if (AK_NULL == *ppimage_buffer)
		{
			*ppimage_buffer = (T_pDATA)Fwl_Malloc(FULL_BMP_SIZE);
			AK_ASSERT_PTR_VOID(*ppimage_buffer, "Standby_LoadUserBkImg(): malloc error");

			memset(*ppimage_buffer, 0, FULL_BMP_SIZE);
		}

        bmp_buf = ImgDec_GetImageData(file_path);
        if (bmp_buf != AK_NULL)
            img_support = AK_TRUE;
    }

    if (img_support)
    {
        num = FillAkBmpHead(*ppimage_buffer, dwidth, dheight);
        GetBMP2RGBBuf(bmp_buf, (*ppimage_buffer)+num, dwidth, dheight, \
                0, 0, 100, 0, &scale,COLOR_BLACK);
    }
    else
    {
        /**Set default background picture*/
        if (AK_NULL != *ppimage_buffer)
        {
        	Fwl_Free(*ppimage_buffer);
        }
        pData = Res_StaticLoad(AK_NULL, eRES_BMP_MAIN_BACKGROUND, &imgLen); 
        *ppimage_buffer = pData;
    }

    ImgDec_FreeImageData(bmp_buf);
}

T_VOID Standby_LoadUserBkImg(T_VOID)
{
	AK_ASSERT_PTR_VOID(pStdbParm, "pStdbParm error");

    Standby_LoadImage(&pStdbParm->pUserBkImgBuf, _T(STDB_CACHE_PIC), Fwl_GetLcdWidth(), Fwl_GetLcdHeight());
    Standby_SetBkImg(pStdbParm->pUserBkImgBuf);
}


/**
 * @brief free user background image buffer
 *
 * @author zhengwenbo
 * @date 2008-5-27
 * @param  T_VOID
 * @return T_VOID
 * @retval  
 */
T_VOID Standby_FreeUserBkImg(T_VOID)
{
	AK_ASSERT_PTR_VOID(pStdbParm, "pStdbParm error");
	
    if (AK_NULL != pStdbParm->pUserBkImgBuf)
    {
        pStdbParm->pUserBkImgBuf = Fwl_Free(pStdbParm->pUserBkImgBuf);
        pStdbParm->pUserBkImgBuf = AK_NULL;
    }
}


/**
 * @brief set background image to iconmenu
 *
 * @author zhengwenbo
 * @date 2008-5-27
 * @param[in]  T_U8 *pImage
 * @return T_VOID
 * @retval  
 */
static T_VOID Standby_SetBkImg(T_U8 *pImage)
{
    AK_ASSERT_PTR_VOID(pImage, "Standby_SetBkImg(): pImage NULL\n");
    
#ifdef ICONMENU_VERTICAL_ICON
    IconMenu_SetIconStyle(&pStdbParm->IconMenu, 0, 15, COLOR_FUCHSIN, COLOR_BLUE, 0, (T_pCDATA)pImage, &pStdbParm->stdbRes.iconRect);
#else
    IconMenu_SetIconStyle(&pStdbParm->IconMenu, 15, 5, COLOR_FUCHSIN, COLOR_BLUE, 0, (T_pCDATA)pImage, &pStdbParm->stdbRes.iconRect);
#endif 
}




///static T_U8 gBatteryShow = 0;

T_S32 BatteryGetValue(T_VOID);

static T_VOID StdbShwBattery(const T_STDB_RES_PARM *pStdbRes)
{
    T_S32 batShowNo;
    T_pCDATA pBatImg;

    batShowNo = TopBar_GetBattIconIndex();//BatteryGetShowNo();
    pBatImg = pStdbRes->pBatteryImg[batShowNo];
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pStdbRes->batteryRect.left, pStdbRes->batteryRect.top, pBatImg, &g_Graph.TransColor, AK_FALSE);
}

/*********** 因为与Eng_TopBar.c中的TopBar_GetBattIconIndex函数定义重复
static T_S32 BatteryGetShowNo(T_VOID)
{
#ifdef OS_ANYKA
    T_S32 value;

    if ((Fwl_UseExternCharge() == AK_TRUE) && (Fwl_ChargeVoltageFull() == AK_FALSE))
    {
        gBatteryShow = (gBatteryShow +1 ) % BATTERY_ICON_QTY;
    }
    else
    {
        value = Fwl_GetBatteryVoltage();

        if (value < BATTERY_VALUE_WARN)
            gBatteryShow = 0;
        else if (value >= BATTERY_VALUE_MAX)
            gBatteryShow = BATTERY_ICON_QTY-1;
        else
            gBatteryShow = (value-BATTERY_VALUE_WARN)*BATTERY_ICON_QTY/(BATTERY_VALUE_MAX-BATTERY_VALUE_WARN);
    }
#else
    gBatteryShow = BATTERY_ICON_QTY-1;
#endif // _DEBUG

    return gBatteryShow;
}
**********/

static T_VOID Standy_GetRes(T_STDB_RES_PARM *pStdbRes)
{
    T_U32  imgLen;
    T_U32  i;

    //pStdbRes->pDefBkImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MAIN_BACKGROUND, &imgLen);
    for(i=0; i<BATTERY_ICON_QTY; i++)
        pStdbRes->pBatteryImg[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, (eRES_BMP_MAIN_BAT_EMPTY + i), &imgLen);
    
    return;
}

/*
Main Menu refresh partition map

VMENU
-----------------------------
|
                |topbar                    |
|                   |________________|
|                   |                            |
|                   |                            |
|       icon      |  attach icon           |
|                   |                            |
|                   |                            |   
|                   |                            |
|                   |                            |
|___________|_______________ |
                    |
                    |-->ICON_WITDH

九宫格
-----------------------------
|
                topbar                    |
_____________________________
|                                            |  |-->topscrbar
|                                            |_|
|       icon                               |  |
|                                            |  |
|                                            |  | -->scrbar  
|                                            |  |   
|                                            |  |   
|_________________________|_|

*/
static T_BOOL initStdbResRect(T_STDB_RES_PARM *pStdbRes)
{
//    T_USTR_FILE ustr;
//    T_LEN   fontHeight;
    T_LEN   batteryHght;
    T_LEN   batteryWdth;
    
   /* if (pStdbRes->pDefBkImg == AK_NULL)
    {
        return AK_FALSE;
    }*/

#ifdef ICONMENU_VERTICAL_ICON
    // scBarRect is not use
    pStdbRes->scBarRect.top = 0;
    pStdbRes->scBarRect.height = 0;
    pStdbRes->scBarRect.width = 0;
    pStdbRes->scBarRect.left = 0;

    //pStdbRes->voltageRect
    AKBmpGetInfo(pStdbRes->pBatteryImg[0], &batteryWdth, &batteryHght, AK_NULL);
    fontHeight = GetFontHeight(CURRENT_FONT_SIZE);

    if (AK_TRUE != TopBar_GetBatteryPosition(&pStdbRes->batteryRect.left, &pStdbRes->batteryRect.top))
    {
        pStdbRes->batteryRect.left = Fwl_GetLcdWidth() - batteryWdth - STAT_FRAME_DISTANCE;
        pStdbRes->batteryRect.top = pStdbRes->TopBarRect.top + FIRST_STAT_ICON_Y_POS;
    }
    pStdbRes->batteryRect.width = batteryWdth;
    pStdbRes->batteryRect.height = batteryHght;

    pStdbRes->TopBarRect.height = pStdbRes->batteryRect.top + pStdbRes->batteryRect.height;
    pStdbRes->TopBarRect.width = Fwl_GetLcdWidth() - ICON_WIDTH;
    pStdbRes->TopBarRect.left = ICON_WIDTH;
    pStdbRes->TopBarRect.top = 0;
    
    pStdbRes->iconRect.left = 0;
    pStdbRes->iconRect.height = Fwl_GetLcdHeight();
    pStdbRes->iconRect.top = 0;
    pStdbRes->iconRect.width = ICON_WIDTH;

    /**Attach rect*/ 
    pStdbRes->IconAttachtRect.left = pStdbRes->iconRect.width;
    pStdbRes->IconAttachtRect.top  = pStdbRes->TopBarRect.height;
    pStdbRes->IconAttachtRect.height = pStdbRes->iconRect.height - pStdbRes->TopBarRect.height;
    pStdbRes->IconAttachtRect.width = Fwl_GetLcdWidth() - ICON_WIDTH;

    Standby_SetTimeStrRect(pStdbRes);

#else       // 九宫格
    pStdbRes->IconAttachtRect.left = 0;
    pStdbRes->IconAttachtRect.top  = 0;
    pStdbRes->IconAttachtRect.height = 0;
    pStdbRes->IconAttachtRect.width = 0;
    
    AKBmpGetInfo(pStdbRes->pBatteryImg[0], &batteryWdth, &batteryHght, AK_NULL);
    if (AK_TRUE != TopBar_GetBatteryPosition(&pStdbRes->batteryRect.left, &pStdbRes->batteryRect.top))
    {
        pStdbRes->batteryRect.left = Fwl_GetLcdWidth() - batteryWdth - STAT_FRAME_DISTANCE;
        pStdbRes->batteryRect.top = FIRST_STAT_ICON_Y_POS;
    }
    pStdbRes->batteryRect.width = batteryWdth;
    pStdbRes->batteryRect.height = batteryHght;
    
    pStdbRes->TopBarRect.height = pStdbRes->batteryRect.top + pStdbRes->batteryRect.height;
    pStdbRes->TopBarRect.width = Fwl_GetLcdWidth();
    pStdbRes->TopBarRect.left = 0;
    pStdbRes->TopBarRect.top = 0;
    
    Standby_SetTimeStrRect(pStdbRes);
    
    pStdbRes->iconRect.left = 0;
    pStdbRes->iconRect.height = Fwl_GetLcdHeight() - pStdbRes->TopBarRect.height;
    pStdbRes->iconRect.top = pStdbRes->TopBarRect.height;
    pStdbRes->iconRect.width = Fwl_GetLcdWidth();// - g_Graph.BScBarWidth;

    pStdbRes->scrBarTopRect.left = Fwl_GetLcdWidth() - g_Graph.BScBarWidth;
    pStdbRes->scrBarTopRect.top = pStdbRes->TopBarRect.height;
    pStdbRes->scrBarTopRect.height = 10;
    pStdbRes->scrBarTopRect.width = g_Graph.BScBarWidth;
    
    pStdbRes->scBarRect.left = Fwl_GetLcdWidth() - g_Graph.BScBarWidth;
    pStdbRes->scBarRect.top = pStdbRes->iconRect.top + pStdbRes->scrBarTopRect.height;
    pStdbRes->scBarRect.height = pStdbRes->iconRect.height;
    pStdbRes->scBarRect.width = g_Graph.BScBarWidth;
#endif

    return AK_TRUE;
}

static T_BOOL Standby_SetTimeStrRect(T_STDB_RES_PARM *pStdbRes)
{
    T_SYSTIME   curTime;
    T_U16       strDate[30] = {0};
    T_U16       strTime[30] = {0};
    T_U16       TmpStr[30] = {0};
    T_U16       *pUDStr = AK_NULL;
    T_U16       *pUStr = AK_NULL;
    T_U32       strTmWidth = 0;     

    if (AK_NULL == pStdbRes)
    {
        AK_DEBUG_OUTPUT("Standby_SetTimeStrRect():AK_NULL == pStdbRes");
        return AK_FALSE;
    }

    curTime = GetSysTime();
    ConvertTimeS2UcSByFormat(&curTime, strDate, TmpStr);
    Utl_UStrCpyN(strTime, TmpStr, 5);

    if (TIME_FORMAT_12 == gs.TimeFormat)
    {
        pUDStr = strTime + 5;
        pUStr = TmpStr + 8;
        Utl_UStrCpy(pUDStr, pUStr);
    }
    
    strTmWidth = UGetSpeciStringWidth(strTime, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(strTime));

    pStdbRes->timeRect.left = (T_S16)(pStdbRes->batteryRect.left - strTmWidth - STAT_ICON_DISTANCE);
    pStdbRes->timeRect.top = pStdbRes->batteryRect.top;
    pStdbRes->timeRect.width = (T_U16)strTmWidth;
    pStdbRes->timeRect.height = GetFontHeight(CURRENT_FONT_SIZE);

    return AK_TRUE;
}

static T_VOID stdbShwOther(T_VOID)
{
    T_STDB_RES_PARM *pStdbRes;
    T_POS otherLeft;
    T_POS otherTop;
    T_U8  freshFlag;
    T_BOOL drawedBckgrnd = AK_FALSE;

    pStdbRes = &pStdbParm->stdbRes;
    otherLeft = pStdbRes->TopBarRect.left;
    otherTop  = pStdbRes->TopBarRect.top;
    freshFlag = stdbGetOtherRefresh() ;

    if ((freshFlag & OTHER_REFRESH_BKGRND) == OTHER_REFRESH_BKGRND)
    {
        drawedBckgrnd = AK_TRUE;
    }

    if ((freshFlag & OTHER_REFRESH_VOLTAGE) == OTHER_REFRESH_VOLTAGE)
        StdbShwBattery(pStdbRes);

    if ((freshFlag & OTHER_REFRESH_TIME) == OTHER_REFRESH_TIME)
        stdbRefreshTime(pStdbRes, drawedBckgrnd);

    if ((freshFlag & OTHER_REFRESH_TIMEDOT) == OTHER_REFRESH_TIMEDOT)
        stdbRefreshTimeDot(pStdbRes, pStdbParm->timeDotFlag, drawedBckgrnd);

    stdbSetOtherRefresh(OTHER_REFRESH_NONE);
}


static T_VOID stdbRefreshTime(const T_STDB_RES_PARM *pStdbRes, T_BOOL drawedBckgrnd)
{
    T_SYSTIME   curTime;
    T_U16	    strDate[30] = {0};
    T_U16       strTime[30] = {0};
    T_U16       TmpStr[30] = {0};
    T_U16       *pUDStr = AK_NULL;
    T_U16       *pUStr = AK_NULL;
    T_pCDATA    pBckgrnd;
    T_U16       strLen = 0;
    T_U16       strWidth = 0;
    T_POS       strLeft = 0;
    T_POS       strTop = 0;
    T_RECT      dispRect;

    curTime = GetSysTime();

    ConvertTimeS2UcSByFormat(&curTime, strDate, TmpStr);
    Utl_UStrCpyN(strTime, TmpStr, 5);

    if (TIME_FORMAT_12 == gs.TimeFormat)
    {
        pUDStr = strTime + 5;
        pUStr = TmpStr + 8;
        Utl_UStrCpy(pUDStr, pUStr);
    }

    strLen = (T_U16)Utl_UStrLen(strTime);
    strWidth = (T_U16)UGetSpeciStringWidth(strTime, CURRENT_FONT_SIZE, strLen);
    strLeft = pStdbRes->timeRect.left;
    strTop = pStdbRes->timeRect.top;
    
    dispRect.left = strLeft;
    dispRect.top = strTop;
    dispRect.width = strWidth;
    dispRect.height = g_Font.SCHEIGHT;
    

    pBckgrnd = pStdbParm->pUserBkImgBuf; 
    

    /**if background has been drawn, don't draw here again*/
    if (drawedBckgrnd != AK_TRUE)
    {
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, strLeft, strTop, &dispRect, pBckgrnd, AK_NULL, AK_FALSE);
    }

    /**Display time string*/
    Fwl_UDispSpeciString(HRGB_LAYER, strLeft, strTop, strTime, COLOR_WHITE, CURRENT_FONT_SIZE, strLen);
}

static T_VOID stdbRefreshTimeDot(const T_STDB_RES_PARM *pStdbRes, T_BOOL drawFlag, T_BOOL drawedBckgrnd)
{
    T_pCDATA    pBckgrnd;
    T_RECT      bmpRange;
    T_SYSTIME   curTime;
    T_U16	    strDate[30] = {0};
    T_U16       TmpStr[30] = {0};
    T_U16       *pUDStr = AK_NULL;    
    T_U16       dispStr[16] = {0};
    T_U16       offset = 0;
    T_POS       strLeft = 0;
    T_POS       strTop = 0;
    T_U16       strWidth = 0;
    T_U16       strLen = 0;

    curTime = GetSysTime();

    ConvertTimeS2UcSByFormat(&curTime, strDate, TmpStr);
    offset = (T_U16)UGetSpeciStringWidth(TmpStr, CURRENT_FONT_SIZE, 2);
    strLeft = pStdbRes->timeRect.left + offset;
    strTop = pStdbRes->timeRect.top; 
    pUDStr = TmpStr + 2;
    Utl_UStrCpyN(dispStr, pUDStr, 1);

    strLen = (T_U16)Utl_UStrLen(dispStr);
    strWidth = (T_U16)UGetSpeciStringWidth(dispStr, CURRENT_FONT_SIZE, strLen);

    bmpRange.left = strLeft;
    bmpRange.top  = strTop;
    bmpRange.width = strWidth;
    bmpRange.height = g_Font.SCHEIGHT;

    pBckgrnd = pStdbParm->pUserBkImgBuf; 


    /**if background has been drawn, don't draw here again*/
    if (drawedBckgrnd != AK_TRUE)
    {
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bmpRange.left, bmpRange.top, &bmpRange, pBckgrnd, AK_NULL, AK_FALSE);
    }
    
    if (drawFlag)
    {
        Fwl_UDispSpeciString(HRGB_LAYER, strLeft, strTop, dispStr, COLOR_WHITE, CURRENT_FONT_SIZE, strLen);
    }
}

/*
static T_VOID stdbShwOtherBckgrnd(T_pCDATA pBckgrnd, T_POS left, T_POS top)
{
    Fwl_AkBmpDrawFromString(HRGB_LAYER, left, top, pBckgrnd, &g_Graph.TransColor, AK_FALSE);
}
*/
static T_VOID stdbSetOtherRefresh(const T_U8 freshFlag)
{
    if (freshFlag != OTHER_REFRESH_NONE)
        pStdbParm->otherRefreshFlag |= freshFlag;
    else
        pStdbParm->otherRefreshFlag = OTHER_REFRESH_NONE;
}

static T_U8 stdbGetOtherRefresh(T_VOID)
{
    if( AUDIOPLAYER_STATE_NONE == AudioPlayer_GetCurState() ) 
        pStdbParm->otherRefreshFlag |= 0x01;//ShiYueKun add to resolve music icon error in stop state in main
    
    return pStdbParm->otherRefreshFlag;
}
#endif  /* end of UI_USE_ICONMENU */

/* end of file */

