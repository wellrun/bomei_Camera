/*******************************************************************
 * @File Name: Eng_TopBar.c
 * @brief The file define some functions for handling the displaying
          operation of icon in top bar.

 * Copyright (c) 2006, Anyka Co., Ltd.
 * All rights reserved.

 * @author Ri'an Zeng
 * @date 2006-08-21
 * @version 1.0.0
 *******************************************************************/
#include "Eng_TopBar.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Fwl_pfAudio.h"
#include "Eng_Time.h"
#include "Fwl_public.h"
#include "Fwl_pfDisplay.h"
#include "Lib_res_port.h"
#include "Eng_AkBmp.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "ctl_global.h"
#include "Ctl_AudioPlayer.h"
#include "hal_timer.h"
#include "eng_dataconvert.h"
#include "eng_debug.h"
#include "fwl_power.h"
#include "Lib_state_api.h"
#include "fwl_usb_host.h"
#include "fwl_display.h"




#define BATTERY_ZONE_X_LEFT_DIS     3
#define BATTERY_ZONE_X_RIGHT_DIS    5
#define BATTERY_ZONE_Y_UP_DIS       3
#define BATTERY_ZONE_Y_DOWN_DIS     3

#define BATTERY_INTERVAL_NUM        4

#define TOPBAR_BCKGND_X             0
#define TOPBAR_BCKGND_Y             0
#define FIRST_STATUS_ICON_POS           5

#define AUDIO_STATUS_ICON_QTY           5
#define BATTERY_ICON_QTY                5
#define STATUS_ICON_DISTANCE            3

#define AUDIO_STATUS_STOP        0
#define AUDIO_STATUS_PLAY        1
#define AUDIO_STATUS_PAUSE      2
#define AUDIO_STATUS_FF            3
#define AUDIO_STATUS_FR            4

#define TB_SPACE_WIDTH(width)       (width >> 3)        // width/8
#define TB_SPACE_TAIL_LEN          6

typedef struct {
    T_U16                   initFlag;               /* identify the control is initialized or not */
    T_BOOL                  bScroll;                /**AK_TRUE: enable scroll text */
    T_USTR_INFO             uText;  /*unicode text*/
    T_U16                   uTextLen;               /*unicode text length*/
    T_pDATA                 bmpData;                /* bmp data in anyka format */
    T_POINT                 point;                  /**diplaying position*/
    T_RECT                  range;                  /* location of title */
    T_U16                   TitleTextOffset;        /* Title text scroll offset */
    T_U16                   DispTextMax;            /* maximum char number displayed*/
    T_S16                   TmCount;                /* time count for scroll the text automatically */
} T_TB_TITLE;

typedef struct {
    T_BOOL              enableShowTopBar;
    T_TB_TITLE          title;
    T_pCDATA            bckGrndIconData;
    T_pCDATA            audioStatusIconData[AUDIO_STATUS_ICON_QTY];
    T_pCDATA            batteryStatusIconData[BATTERY_ICON_QTY];
    T_U8                BattIconIndex;                                               // the index of battery icon
    T_pCDATA            CancelButton;
    T_pCDATA            menuButton;
    T_BOOL              MenuIsValid;
    T_BOOL              bMenuIconShow;                  // 1: allow show  0:forbid show
    
    T_TB_RES_PUB_ITEM   pubItem[eTB_RES_MAX_NUM];
    T_BOOL              bShowTime;                     // AK_TRUE:show time   AK_FALSE:show title
}T_TB_RES_ITEM;



static T_TB_RES_ITEM *pTopBarItem = AK_NULL;
static T_U32 gb_TbShowTimeCount = SHOW_TIME_INTERVAL;

static T_BOOL TopBar_ShowBckGrnd(T_VOID);
static T_BOOL TopBar_ShowAudioStatus(T_VOID);
static T_BOOL TopBar_ShowTitle(T_VOID);
static T_BOOL TopBar_ShowBattery(T_VOID);
static T_BOOL TopBar_ShowCancelButton(T_VOID);
static T_BOOL TopBar_ShowMenuButton(T_VOID);
static T_BOOL TopBar_InitTitle(T_TB_RES_ITEM *pTopBarItem);
static T_BOOL TopBar_TitleTextOffset(T_TB_RES_ITEM *pTopBarItem);
static T_BOOL TopBar_IsCurrentSMNeedMenu(T_VOID);
extern T_BOOL     Img_list_ShowBtns(T_VOID);

/**
 * @Brief   Init resource of top bar
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_BOOL: if initialize successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_Init(T_VOID)
{
    T_U32 imgLen = 0;
    //T_U8 i = 0;


    T_TB_RES_PUB_ITEM *tempItem = (T_TB_RES_PUB_ITEM *)Fwl_Malloc(sizeof(T_TB_RES_PUB_ITEM));

    pTopBarItem = (T_TB_RES_ITEM *)Fwl_Malloc(sizeof(T_TB_RES_ITEM));


//  AK_ASSERT_VAL(pTopBarItem->enableShowTopBar, "Top bar is disable show!", AK_FALSE);

    pTopBarItem->enableShowTopBar = AK_FALSE;
    pTopBarItem->BattIconIndex= 0;

    pTopBarItem->MenuIsValid = AK_FALSE;
    pTopBarItem->bMenuIconShow = AK_TRUE;

    pTopBarItem->bShowTime = AK_FALSE; // show title
    pTopBarItem->bckGrndIconData =  Res_GetBinResByID(&pTopBarItem->bckGrndIconData, AK_FALSE, eRES_BMP_PUB_TITLE, &imgLen);
//  AK_ASSERT_PTR_VOID(pTopBarItem->bckGrndIconData, "Get the icon data of back ground faild");
    AKBmpGetInfo(pTopBarItem->bckGrndIconData, &(tempItem->width), &(tempItem->height), AK_NULL);
    pTopBarItem->pubItem[eTB_RES_BCKGRND].height = tempItem->height;
    pTopBarItem->pubItem[eTB_RES_BCKGRND].width = tempItem->width;


    pTopBarItem->audioStatusIconData[AUDIO_STATUS_STOP] =
        Res_GetBinResByID(&pTopBarItem->audioStatusIconData[AUDIO_STATUS_STOP], AK_FALSE, (eRES_BMP_MAIN_MP3_STOP), &imgLen);
    pTopBarItem->audioStatusIconData[AUDIO_STATUS_PLAY] =
        Res_GetBinResByID(&pTopBarItem->audioStatusIconData[AUDIO_STATUS_PLAY], AK_FALSE, (eRES_BMP_MAIN_MP3_PLAY), &imgLen);
    pTopBarItem->audioStatusIconData[AUDIO_STATUS_PAUSE] =
        Res_GetBinResByID(&pTopBarItem->audioStatusIconData[AUDIO_STATUS_PAUSE], AK_FALSE, (eRES_BMP_MAIN_MP3_PAUSE), &imgLen);
    pTopBarItem->audioStatusIconData[AUDIO_STATUS_FF] =
        Res_GetBinResByID(&pTopBarItem->audioStatusIconData[AUDIO_STATUS_FF], AK_FALSE, (eRES_BMP_MAIN_MP3_FORWARD), &imgLen);
    pTopBarItem->audioStatusIconData[AUDIO_STATUS_FR] =
        Res_GetBinResByID(&pTopBarItem->audioStatusIconData[AUDIO_STATUS_FR], AK_FALSE, (eRES_BMP_MAIN_MP3_BACKWARD), &imgLen);
    AKBmpGetInfo(pTopBarItem->audioStatusIconData[0], &(tempItem->width), &(tempItem->height), AK_NULL);
    pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].height = tempItem->height;
    pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].width = tempItem->width;


    pTopBarItem->batteryStatusIconData[0] = Res_GetBinResByID(&pTopBarItem->batteryStatusIconData[0], AK_FALSE, eRES_BMP_MAIN_BAT_EMPTY, &imgLen);
    pTopBarItem->batteryStatusIconData[1] = Res_GetBinResByID(&pTopBarItem->batteryStatusIconData[1], AK_FALSE, eRES_BMP_MAIN_BAT_MIDDLE1, &imgLen);
    pTopBarItem->batteryStatusIconData[2] = Res_GetBinResByID(&pTopBarItem->batteryStatusIconData[2], AK_FALSE, eRES_BMP_MAIN_BAT_MIDDLE2, &imgLen);
    pTopBarItem->batteryStatusIconData[3] = Res_GetBinResByID(&pTopBarItem->batteryStatusIconData[3], AK_FALSE, eRES_BMP_MAIN_BAT_MIDDLE3, &imgLen);
    pTopBarItem->batteryStatusIconData[4] = Res_GetBinResByID(&pTopBarItem->batteryStatusIconData[4], AK_FALSE, eRES_BMP_MAIN_BAT_FULL, &imgLen);

    AKBmpGetInfo(pTopBarItem->batteryStatusIconData[0], &(tempItem->width), &(tempItem->height), AK_NULL);
    pTopBarItem->pubItem[eTB_RES_BATTERY].height = tempItem->height;
    pTopBarItem->pubItem[eTB_RES_BATTERY].width = tempItem->width;

    //Cancel Button
    pTopBarItem->CancelButton = Res_GetBinResByID(&pTopBarItem->CancelButton, AK_FALSE, eRES_BMP_MAIN_CANCEL_BUTTON, &imgLen);
    AKBmpGetInfo(pTopBarItem->CancelButton, &(tempItem->width), &(tempItem->height), AK_NULL);
    pTopBarItem->pubItem[eTB_RES_CANCEL].height = tempItem->height;
    pTopBarItem->pubItem[eTB_RES_CANCEL].width = tempItem->width;

    //menu button
    pTopBarItem->menuButton = Res_GetBinResByID(&pTopBarItem->menuButton, AK_FALSE, eRES_BMP_MAIN_MENU_BUTTON, AK_NULL);
    AKBmpGetInfo(pTopBarItem->menuButton, &(tempItem->width), &(tempItem->height), AK_NULL);
    pTopBarItem->pubItem[eTB_RES_MENU].height = tempItem->height;
    pTopBarItem->pubItem[eTB_RES_MENU].width = tempItem->width;

    if ((pTopBarItem->pubItem[eTB_RES_BCKGRND].width) >= Fwl_GetLcdWidth())
    {
        pTopBarItem->pubItem[eTB_RES_BCKGRND].position.x = TOPBAR_BCKGND_X;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_BCKGRND].position.x = (Fwl_GetLcdWidth()
            - (pTopBarItem->pubItem[eTB_RES_BCKGRND].width)) >> 2;
    }
    pTopBarItem->pubItem[eTB_RES_BCKGRND].position.y = TOPBAR_BCKGND_Y;

    pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.x = FIRST_STATUS_ICON_POS;

    if (pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].height < pTopBarItem->pubItem[eTB_RES_BCKGRND].height)
    {
        pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.y
            = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].height)/2;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.y = TOPBAR_BCKGND_Y;
    }


    pTopBarItem->pubItem[eTB_RES_TITLE].height = g_Font.CHEIGHT;


    if (pTopBarItem->pubItem[eTB_RES_TITLE].height < pTopBarItem->pubItem[eTB_RES_BCKGRND].height)
    {
        pTopBarItem->pubItem[eTB_RES_TITLE].position.y
            = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - pTopBarItem->pubItem[eTB_RES_TITLE].height) >> 1;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_TITLE].position.y = TOPBAR_BCKGND_Y;
    }

    pTopBarItem->pubItem[eTB_RES_BATTERY].position.x = Fwl_GetLcdWidth() - (2 * FIRST_STATUS_ICON_POS) 
                                                                               - pTopBarItem->pubItem[eTB_RES_BATTERY].width
                                                                               - pTopBarItem->pubItem[eTB_RES_CANCEL].width;
    if (pTopBarItem->pubItem[eTB_RES_BATTERY].height < pTopBarItem->pubItem[eTB_RES_BCKGRND].height)
    {
        pTopBarItem->pubItem[eTB_RES_BATTERY].position.y
            = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - pTopBarItem->pubItem[eTB_RES_BATTERY].height)/2;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_BATTERY].position.y = TOPBAR_BCKGND_Y;
    }

    //the position of Cancel button
    pTopBarItem->pubItem[eTB_RES_CANCEL].position.x = Fwl_GetLcdWidth() - FIRST_STATUS_ICON_POS - pTopBarItem->pubItem[eTB_RES_CANCEL].width;                                                        

    if (pTopBarItem->pubItem[eTB_RES_CANCEL].height < pTopBarItem->pubItem[eTB_RES_BCKGRND].height)
    {
        pTopBarItem->pubItem[eTB_RES_CANCEL].position.y
            = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - pTopBarItem->pubItem[eTB_RES_CANCEL].height)/2;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_CANCEL].position.y = TOPBAR_BCKGND_Y;
    }

    //the position of menu button
    pTopBarItem->pubItem[eTB_RES_MENU].position.x = Fwl_GetLcdWidth() - (3 * FIRST_STATUS_ICON_POS) 
                                                                               - pTopBarItem->pubItem[eTB_RES_BATTERY].width
                                                                               - pTopBarItem->pubItem[eTB_RES_CANCEL].width
                                                                               - pTopBarItem->pubItem[eTB_RES_MENU].width;

    if (pTopBarItem->pubItem[eTB_RES_MENU].height < pTopBarItem->pubItem[eTB_RES_BCKGRND].height)
    {
        pTopBarItem->pubItem[eTB_RES_MENU].position.y
            = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - pTopBarItem->pubItem[eTB_RES_MENU].height)/2;
    }
    else
    {
        pTopBarItem->pubItem[eTB_RES_MENU].position.y = TOPBAR_BCKGND_Y;
    }

    TopBar_InitTitle(pTopBarItem);

    /**Get battery's index*/
    TopBar_GetBattIconIndex();

    Fwl_Free(tempItem);

    return AK_TRUE;
}//end of TopBar_Init


/**
 * @Brief       Enable show all the resource of top bar
 * @Author  Ri'an Zeng
 * @Data        2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_EnableShow(T_VOID)
{
    /**Update battery icon index*/
    if (AK_FALSE == pTopBarItem->enableShowTopBar)
    {
        TopBar_GetBattIconIndex();

        pTopBarItem->enableShowTopBar = AK_TRUE;
    }
}//end of TopBar_EnableShow


/**
 * @Brief       Disable show all the resource of top bar
 * @Author  Ri'an Zeng
 * @Data        2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_DisableShow(T_VOID)
{
    pTopBarItem->enableShowTopBar = AK_FALSE;

    return ;

}//end of TopBar_DisableShow

/**
 * @Brief   Set title
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   [IN]T_pSTR string: title string
 * @Return  T_BOOL: if the setting successful, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_SetTitle(T_pCWSTR string)
{
    T_U32 width = 0;
#ifdef SUPPORT_VFONT
	T_BOOL		bUseVFont = AK_FALSE;
#endif
    //T_U16 ch = 0x3000; //blank char

    AK_ASSERT_PTR(string, "Title string is null!", AK_FALSE);

#ifdef SUPPORT_VFONT
	bUseVFont = gb.bIsUseVFont;
	gb.bIsUseVFont = AK_FALSE;
#endif


    //pTopBarItem->title.uTextLen = (T_U16)MultiByteToWideChar(gs.Lang, string, strlen(string), AK_NULL, pTopBarItem->title.uText, strlen(string)+1, &ch);
    Utl_UStrCpy(pTopBarItem->title.uText, string);
    pTopBarItem->title.uTextLen = (T_U16)Utl_UStrLen(string);

    width = UGetSpeciStringWidth(pTopBarItem->title.uText, CURRENT_FONT_SIZE, pTopBarItem->title.uTextLen);
//  string[pTopBarItem->title.uTextLen] = string;

    /**Judge if scroll text*/
    if (width > (T_U32)pTopBarItem->title.range.width) // need scroll
    {
        pTopBarItem->title.bScroll = AK_TRUE;
        pTopBarItem->title.point.x = pTopBarItem->title.range.left;
        pTopBarItem->title.point.y = pTopBarItem->title.range.top;
    }
    else
    {
        pTopBarItem->title.bScroll = AK_FALSE;
        pTopBarItem->title.point.x = (T_S16)(pTopBarItem->title.range.left
                           + ((pTopBarItem->title.range.width - width) >> 1)); // middle of range
        pTopBarItem->title.point.y = pTopBarItem->title.range.top;
    }

#ifdef SUPPORT_VFONT
	gb.bIsUseVFont = bUseVFont;
#endif

    return AK_TRUE;
}//end of TopBar_SetTitle


/**
 * @Brief   Enable Memu Button
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_EnableMenuButton(T_VOID)
{
    if (pTopBarItem->MenuIsValid == AK_FALSE)
    {
        pTopBarItem->MenuIsValid = AK_TRUE;
    }
}

/**
 * @Brief   Disable Memu Button
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_DisableMenuButton(T_VOID)
{
    if (pTopBarItem->MenuIsValid == AK_TRUE)
    {
       pTopBarItem->MenuIsValid = AK_FALSE;
    }
}

/**
 * @Brief   switch on/off menu icon show
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_MenuIconShowSwitch(T_BOOL flag)
{
    pTopBarItem->bMenuIconShow = flag;
}

/**
 * @Brief       Set show resource
 * @Author  Ri'an Zeng
 * @Data        2006-08-21
 * @Param   [IN]T_U16 refresh: the refresh control
 * @Return  T_BOOL: if the Refresh successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_Show(T_U16 refresh)
{
    T_BOOL ret = AK_TRUE;

    if (pTopBarItem->enableShowTopBar == AK_FALSE)
    {
        return AK_TRUE;
    }

	if (eM_s_pub_pre_message == SM_GetCurrentSM())
	{
		return AK_TRUE;
	}

    switch (refresh)
    {
        case TB_REFRESH_AUDIO_STATUS:
            if (AK_FALSE == TopBar_ShowAudioStatus())
            {
                ret = AK_FALSE;
            }
            break;

        case TB_REFRESH_TITLE:
            if (AK_FALSE == TopBar_ShowTitle())
            {
                ret = AK_FALSE;
            }
            break;

        case TB_REFRESH_BATT:
            if (AK_FALSE == TopBar_ShowBattery())
            {
                ret = AK_FALSE;
            }
            break;

        case TB_REFRESH_BKGRND:
        case TB_REFRESH_ALL:
            if ((AK_FALSE == TopBar_ShowBckGrnd()) || (AK_FALSE == TopBar_ShowAudioStatus()) || (AK_FALSE == TopBar_ShowTitle())
                  || (AK_FALSE == TopBar_ShowBattery()) || (AK_FALSE == TopBar_ShowCancelButton()))
            {
                ret = AK_FALSE;
            }

            if ((AK_TRUE == pTopBarItem->MenuIsValid) && (AK_FALSE == TopBar_ShowMenuButton()))
            {
               ret = AK_FALSE; 
            }
            break;

        default :
            Fwl_Print(C3, M_CTRL, "TopBar_Show(): refresh control error\n");
            break;
    }
    
#ifdef SUPPORT_IMG_BROWSE
	if (eM_s_img_list == SM_GetCurrentSM())
	{
		Img_list_ShowBtns();
	}
#endif
    return ret;
}//end of TopBar_Show

T_VOID TopBar_Refresh(T_VOID)
{
    if (pTopBarItem->enableShowTopBar == AK_FALSE)
    {
        return;
    }

	Fwl_RefreshDisplay();
}//end of TopBar_Refresh


/**
 * @Brief   Get Height of Top Bar
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_U16: Height of Top Bar
 */
T_U16 TopBar_GetTopBarHeight(T_VOID)
{
    T_U16 tempHeight = 0;

//  AK_ASSERT_PTR_VOID(pTopBarItem->bckGrndIconData, "Refresh back ground failed, Get the icon data of back ground faild");

    tempHeight = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height);

    return tempHeight;

}
//end of TBar_GetTopBarHeight


/**
 * @Brief       show back gound
 * @Author  Ri'an Zeng
 * @Data        2006-08-21
 * @Param   T_VOID
 * @Return  T_BOOL: if Show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
static T_BOOL TopBar_ShowBckGrnd(T_VOID)
{
    Fwl_AkBmpDrawFromString(HRGB_LAYER,
                            pTopBarItem->pubItem[eTB_RES_BCKGRND].position.x,
                            pTopBarItem->pubItem[eTB_RES_BCKGRND].position.y, pTopBarItem->bckGrndIconData,
                            &g_Graph.TransColor, AK_FALSE);

    return AK_TRUE;
}//end lf TopBar_ShowBckGrnd



/**
 * @Brief   show audio status
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_BOOL: if show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
static T_BOOL TopBar_ShowAudioStatus(T_VOID)
{
    T_U32 AudioStatus;
    T_AUDIOPLAYER_STATE audioPlayStatus = AUDIOPLAYER_STATE_NONE;
    T_RECT   range;

    audioPlayStatus = AudioPlayer_GetCurState();
    if (MPLAYER_PLAY == MPlayer_GetStatus()
		&& AUDIOPLAYER_STATE_NONE == audioPlayStatus)
    {
        audioPlayStatus = AUDIOPLAYER_STATE_PLAY;
    }

    switch (audioPlayStatus)
    {
        case AUDIOPLAYER_STATE_STOP:
            AudioStatus = AUDIO_STATUS_STOP;
            break;
        case  AUDIOPLAYER_STATE_PLAY:
        case AUDIOPLAYER_STATE_BACKGROUNDPLAY:
        case AUDIOPLAYER_STATE_AB_PLAY:
        case AUDIOPLAYER_STATE_AUDITION:
            if (AudioPlayer_GetSuspendFlag() == AK_TRUE)
            {
                AudioStatus = AUDIO_STATUS_PAUSE;
            }
            else
            {
                AudioStatus = AUDIO_STATUS_PLAY;
            }
            break;
        case AUDIOPLAYER_STATE_PAUSE:
            AudioStatus = AUDIO_STATUS_PAUSE;
            break;
        case AUDIOPLAYER_STATE_FORWARD:
            AudioStatus = AUDIO_STATUS_FF;
            break;
        case AUDIOPLAYER_STATE_BACKWARD:
            AudioStatus = AUDIO_STATUS_FR;
            break;
        default:
            AudioStatus = AUDIO_STATUS_STOP;
            break;
    }

    range.left = pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.x;
    range.top = pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.y;
    range.width = pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].width;
    range.height = pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].height;

    /**Draw relevant part of back ground*/
    if (AK_FALSE == Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
                                pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.x,
                                pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.y, &range, pTopBarItem->bckGrndIconData,
                                AK_NULL, AK_FALSE))
    {
        return AK_FALSE;
    }

    /**Draw audio play status icon*/
    if (AK_FALSE == Fwl_AkBmpDrawFromString(HRGB_LAYER,
                                pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.x,
                                pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].position.y, pTopBarItem->audioStatusIconData[AudioStatus],
                                &g_Graph.TransColor, AK_FALSE))
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}//end of TopBar_ShowAudioStatus


/**
 * @Brief       show title
 * @Author  Ri'an Zeng
 * @Data        2006-08-21
 * @Param   T_VOID
 * @Return  T_BOOL: if  show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
static T_BOOL TopBar_ShowTitle(T_VOID)
{
    T_SYSTIME tempSystem;
    //T_U8    tempHour    = 0;
    //T_U8    tempMinute  = 0;
    T_U16   tempString[30] = {0};
    T_POINT timePos;
    T_U16   timeLen     = 0;
    T_U8    timeWidth   = 0;
    T_RECT range;
    T_TB_TITLE *title = AK_NULL;
    T_U16	    strDate[30] = {0};
	T_U16	    strTime[30] = {0};
    T_U16       *pUstr = AK_NULL;
#ifdef SUPPORT_VFONT
	T_BOOL		bUseVFont = AK_FALSE;
#endif


    /**Set the range of displaying title string or time string*/
    /*Top Bar
    _______________________________________________________
    |  |status| space |     title/time      |space  |menu|  |batt|  |cancel | |
    _______________________________________________________
                  |-----------MidWidth---------|
    */
    range = pTopBarItem->title.range;

    /**Draw relevant part of background*/
    if (AK_FALSE == Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
                                            range.left,
                                            range.top,
                                            &range, pTopBarItem->bckGrndIconData, AK_NULL, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowTitle(): Draw part of back ground fail\n");
        return AK_FALSE;
    }

#ifdef SUPPORT_VFONT
	bUseVFont = gb.bIsUseVFont;
	gb.bIsUseVFont = AK_FALSE;
#endif

    if (AK_TRUE == pTopBarItem->bShowTime)
    {
        tempSystem = GetSysTime();
        ConvertTimeS2UcSByFormat(&tempSystem, strDate, strTime);
        Utl_UStrCpyN(tempString, strTime, 5);
        pUstr = strTime + 8;
        Utl_UStrCat(tempString, pUstr);

        timeLen = (T_U16)Utl_UStrLen(tempString);
        timeWidth = (T_U8)UGetSpeciStringWidth(tempString, CURRENT_FONT_SIZE, timeLen);

        timePos.x = (Fwl_GetLcdWidth() - timeWidth) >> 1;
        timePos.y = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - g_Font.SCHEIGHT) >> 1;

        /**Display time string*/
        Fwl_UDispSpeciString(HRGB_LAYER, timePos.x, timePos.y, tempString, COLOR_BLACK, CURRENT_FONT_SIZE, timeLen);
    }
    else 
    {
        title = &pTopBarItem->title;

        if (pTopBarItem->title.bScroll)
        {
            Fwl_UScrollDispString(HRGB_LAYER, title->uText, pTopBarItem->title.point.x, pTopBarItem->title.point.y,
                                (T_U16)Utl_UStrLen((T_U16 *)title->uText), title->TitleTextOffset, (T_U16)title->range.width, COLOR_BLACK, CURRENT_FONT_SIZE);
        }
        else
        {
            /**Display title string*/
            Fwl_UDispSpeciString(HRGB_LAYER, pTopBarItem->title.point.x, pTopBarItem->title.point.y, title->uText, COLOR_BLACK, CURRENT_FONT_SIZE, title->uTextLen);

        }
    }

#ifdef SUPPORT_VFONT
	gb.bIsUseVFont = bUseVFont;
#endif

    return AK_TRUE;
}//end of TopBar_ShowTitle


T_U8 TopBar_GetBattIconIndex(T_VOID)
{
    T_U32 Voltage = 0;

    Voltage = Fwl_GetBatteryVoltage();
#ifdef USB_HOST
	if ((eM_s_usb_host!=SM_GetCurrentSM() && !Fwl_UsbHostIsConnect()) //exclude usb host mode
			&& (Fwl_UseExternCharge() == AK_TRUE) && (Fwl_ChargeVoltageFull() == AK_FALSE))
#else
	if ((Fwl_UseExternCharge() == AK_TRUE) && (Fwl_ChargeVoltageFull() == AK_FALSE))
#endif
    {
        pTopBarItem->BattIconIndex = (pTopBarItem->BattIconIndex + 1)%BATTERY_ICON_QTY;
    }
    else
    {
        if (Voltage < BATTERY_VALUE_WARN)
            pTopBarItem->BattIconIndex = 0;
        else if (Voltage >= BATTERY_VALUE_MAX)
            pTopBarItem->BattIconIndex = BATTERY_ICON_QTY-1;
        else
            pTopBarItem->BattIconIndex = (T_U8)((Voltage - BATTERY_VALUE_WARN)*BATTERY_ICON_QTY/(BATTERY_VALUE_MAX-BATTERY_VALUE_WARN))%BATTERY_ICON_QTY;
    }

    return pTopBarItem->BattIconIndex;
}//end of TopBar_BatteryGetShowNo

/**
 * @Brief   Update battery icon
 * @Author  zhengwenbo
 * @Data    2006-09-7
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_UpdateBattIcon(T_VOID)
{
    static T_U8 BattIconIndex = 0xff;
    T_U8 index;

    if (AK_FALSE == pTopBarItem->enableShowTopBar)
    {
        return;
    }

    /**Show battery*/
    index = TopBar_GetBattIconIndex();
    if (index != BattIconIndex)
    {
        TopBar_Show(TB_REFRESH_BATT);
        BattIconIndex = index;
        TopBar_Refresh();
    }
}

/**
 * @Brief   show battery status
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
static T_BOOL TopBar_ShowBattery(T_VOID)
{
    T_RECT range;

    range.left = pTopBarItem->pubItem[eTB_RES_BATTERY].position.x;
    range.top = pTopBarItem->pubItem[eTB_RES_BATTERY].position.y;
    range.width = pTopBarItem->pubItem[eTB_RES_BATTERY].width;
    range.height = pTopBarItem->pubItem[eTB_RES_BATTERY].height;

    /**Draw relevant part of back ground*/
    if (AK_FALSE == Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
                                            pTopBarItem->pubItem[eTB_RES_BATTERY].position.x,
                                            pTopBarItem->pubItem[eTB_RES_BATTERY].position.y,
                                            &range, pTopBarItem->bckGrndIconData, AK_NULL, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowBattery(): Draw part of back ground fail\n");
        return AK_FALSE;
    }

    /**Draw battery icon*/
    if (AK_FALSE == Fwl_AkBmpDrawFromString(HRGB_LAYER,
                                            pTopBarItem->pubItem[eTB_RES_BATTERY].position.x,
                                            pTopBarItem->pubItem[eTB_RES_BATTERY].position.y,
                                            pTopBarItem->batteryStatusIconData[pTopBarItem->BattIconIndex],
                                            &g_Graph.TransColor, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowBattery(): Draw battery icon fail\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}//end of TopBar_ShowBattery

/**
 * @Brief       Get bShowTime flag
 * @Author  zhengwenbo
 * @Data        2006-08-29
 * @Param   [IN] void
 * @Return  T_BOOL: show, return AK_TRUE, not show.
 */
T_BOOL TopBar_GetTimeShowFlag(T_VOID)
{
    return pTopBarItem->bShowTime;
}

/**
 * @Brief       Set bShowTime flag
 * @Author  zhengwenbo
 * @Data        2006-08-29
 * @Param   [IN] flag
 * @Return  void
 */
T_VOID TopBar_SetTimeShowFlag(T_BOOL flag)
{
    pTopBarItem->bShowTime = flag;
}


T_VOID TopBar_ResetShowTimeCount(T_VOID)
{
    gb_TbShowTimeCount = SHOW_TIME_INTERVAL;
}

T_VOID TopBar_ShowTimeDecrease(T_U32 millisecond)
{
    T_U32 second = millisecond / 1000;
    
    if (0 == gb_TbShowTimeCount)
    {
        return;
    }

    if (gb_TbShowTimeCount > second)
    {
        gb_TbShowTimeCount -= second;
    }
    else
    {
        gb_TbShowTimeCount = 0;
    }

    if (0 == gb_TbShowTimeCount)
    {
        TopBar_SetTimeShowFlag(AK_TRUE); // Show time in top bar

        TopBar_Show(TB_REFRESH_TITLE);
        TopBar_Refresh();
    }
}


/**
 * @Brief       Judge if enable show topbar
 * @Author  zhengwenbo
 * @Data        2006-08-29
 * @Param   [IN] void
 * @Return  AK_TRUE: enable AK_FALSE: disable
 */
T_BOOL TopBar_IsEnableShow(T_VOID)
{
    return pTopBarItem->enableShowTopBar;
}

/**
 * @Brief       Intialize title
     caution: must be called after top bar's picture resource is loaded
 * @Author  zhengwenbo
 * @Data        2006-09-8
 * @Param   [IN] pTitle: the point of title
 * @Return  AK_TRUE: init success AK_FALSE: init fail
 */
 static T_BOOL TopBar_InitTitle(T_TB_RES_ITEM *pTopBarItem)
{
    T_LEN MidWidth = 0;

    AK_ASSERT_PTR(pTopBarItem, "(): Input parameter error\n", AK_FALSE);

    /**Set the rangee of displaying title string or time string*/
    /*Top Bar
    _______________________________________________________
    |  |status| space |     title/time      |space    |menu| |batt|  |cancel   |
    _______________________________________________________
                  |-----------MidWidth---------|
    */

    pTopBarItem->title.initFlag = AK_TRUE;          /* identify the control is initialized or not */
    pTopBarItem->title.bScroll = AK_FALSE;
    //pTopBarItem->title.text = ;                            /* display 3 lines at most */
    pTopBarItem->title.bmpData = AK_NULL;           /* bmp data in anyka format */

    MidWidth = pTopBarItem->pubItem[eTB_RES_BCKGRND].width - pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].width
                                                           - pTopBarItem->pubItem[eTB_RES_BATTERY].width 
                                                           - pTopBarItem->pubItem[eTB_RES_CANCEL].width
                                                           - pTopBarItem->pubItem[eTB_RES_MENU].width
                                                           - 4*FIRST_STATUS_ICON_POS;

    pTopBarItem->title.range.left = pTopBarItem->pubItem[eTB_RES_BCKGRND].position.x + FIRST_STATUS_ICON_POS
                        +  pTopBarItem->pubItem[eTB_RES_AUDIOSTATUS].width + TB_SPACE_WIDTH(MidWidth);
    pTopBarItem->title.range.top = (pTopBarItem->pubItem[eTB_RES_BCKGRND].height - g_Font.SCHEIGHT) >> 1;
    pTopBarItem->title.range.width = MidWidth - TB_SPACE_WIDTH(MidWidth);
    pTopBarItem->title.range.height = g_Font.SCHEIGHT;

    pTopBarItem->title.TitleTextOffset = 0;                        /* The first shown character's ID, this variable for scroll title */
    pTopBarItem->title.DispTextMax =(( pTopBarItem->title.range.width / g_Font.SCWIDTH) >> 1) << 1;     /* maximum char number displayed, even number*/
    pTopBarItem->title.TmCount = 0;                     /* time count for scroll the text automatically */

    return AK_TRUE;
}

/**
 * @Brief       scroll title
 * @Author  zhengwenbo
 * @Data        2006-09-08
 * @Param   T_TB_RES_ITEM *pTopBarItem: the pointer of topbar
 * @Return  T_BOOL: if  show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
 static T_BOOL TopBar_TitleTextOffset(T_TB_RES_ITEM *pTopBarItem)
{
    T_TB_TITLE *title = AK_NULL;

    AK_ASSERT_PTR(pTopBarItem, "TopBar_ScrollTitleText():Input parameter error\n", AK_FALSE);

    title = &pTopBarItem->title;

    AK_ASSERT_PTR(title, "TopBar_ScrollTitleText():title point is NULL\n", AK_FALSE);

    title->TitleTextOffset++;

    if (title->TitleTextOffset + TB_SPACE_TAIL_LEN >= title->uTextLen)
    {
        title->TitleTextOffset = 0;
    }

    return AK_TRUE;
}

/**
 * @Brief       scroll title
 * @Author  zhengwenbo
 * @Data        2006-09-08
 * @Param   T_TB_RES_ITEM *pTopBarItem: the pointer of topbar
 * @Return  T_BOOL: if  show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_TitleTextScroll(T_VOID)
{
    T_TB_TITLE *title;

    if (AK_FALSE == pTopBarItem->enableShowTopBar)
    {
        return AK_FALSE;
    }

    if ((pTopBarItem->title.bScroll) && (AK_FALSE == pTopBarItem->bShowTime))
    {
        title = &pTopBarItem->title;
        AK_ASSERT_PTR(title, "TopBar_TitleTextScroll(): title pointer is NULL\n", AK_FALSE);

        TopBar_TitleTextOffset(pTopBarItem);
        TopBar_ShowTitle();
        TopBar_Refresh();
    }
    else
    {
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @Brief   get battery's position
 * @Author  wangwei
 * @Data    2008-05-31
 * @Param   T_POS *pPosX: the pointer of pos x
 * @Param   T_POS *pPosY: the pointer of pos y 
 * @Return  T_BOOL
 * @Retval  if top bar has not be initialized, return AK_FALSE, otherwise return AK_TRUE.
 */
T_BOOL TopBar_GetBatteryPosition(T_POS *pPosX, T_POS *pPosY)
{
    if (AK_NULL == pTopBarItem)
    {
        return AK_FALSE;
    }

    *pPosX = pTopBarItem->pubItem[eTB_RES_BATTERY].position.x;
    *pPosY = pTopBarItem->pubItem[eTB_RES_BATTERY].position.y;

    return AK_TRUE;
}


/**
 * @Brief   show cancel button
 * @Author  wangxuwen
 * @Data    2008-07-28
 * @Param   T_VOID
 * @Return  T_VOID
 */
static T_BOOL TopBar_ShowCancelButton(T_VOID)
{
    T_RECT range;

    range.left = pTopBarItem->pubItem[eTB_RES_CANCEL].position.x;
    range.top = pTopBarItem->pubItem[eTB_RES_CANCEL].position.y;
    range.width = pTopBarItem->pubItem[eTB_RES_CANCEL].width;
    range.height = pTopBarItem->pubItem[eTB_RES_CANCEL].height;

    /**Draw relevant part of back ground*/
    if (AK_FALSE == Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
                                            pTopBarItem->pubItem[eTB_RES_CANCEL].position.x,
                                            pTopBarItem->pubItem[eTB_RES_CANCEL].position.y,
                                            &range, pTopBarItem->bckGrndIconData, AK_NULL, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowCancelButton(): Draw part of back ground fail\n");
        return AK_FALSE;
    }

    /**Draw Cancel Button*/
    if (AK_FALSE == Fwl_AkBmpDrawFromString(HRGB_LAYER,
                                        pTopBarItem->pubItem[eTB_RES_CANCEL].position.x,
                                        pTopBarItem->pubItem[eTB_RES_CANCEL].position.y,
                                        pTopBarItem->CancelButton,
                                        &g_Graph.TransColor, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowCancelButton(): Draw Cancel Button fail\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}//end of TopBar_ShowBattery

/**
 * @Brief   Judge current SM whether need menu iron
 * @Author  
 * @Data    
 * @Param   T_VOID
 * @Return  T_VOID
 */
 static T_BOOL TopBar_IsCurrentSMNeedMenu(T_VOID)
{
    M_STATES curState;
    T_BOOL   ret = AK_TRUE;

    curState = SM_GetCurrentSM();

    if (
         curState == eM_s_set_alarm
        || curState == eM_s_set_menu
        || curState == eM_s_tool_menu
        || curState == eM_s_video_read_list
        || curState == eM_s_video_save_list
        || curState == eM_s_video_menu)
    {
        ret = AK_FALSE;
    }

    return ret;
}

/**
 * @Brief   show menu button
 * @Author  
 * @Data    
 * @Param   T_VOID
 * @Return  T_VOID
 */
static T_BOOL TopBar_ShowMenuButton(T_VOID)
{
    T_RECT range;

    if (AK_FALSE == pTopBarItem->bMenuIconShow || !(TopBar_IsCurrentSMNeedMenu()))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowMenuButton(): Menu icon show not allow");
        return AK_FALSE;
    }

    range.left = pTopBarItem->pubItem[eTB_RES_MENU].position.x;
    range.top = pTopBarItem->pubItem[eTB_RES_MENU].position.y;
    range.width = pTopBarItem->pubItem[eTB_RES_MENU].width;
    range.height = pTopBarItem->pubItem[eTB_RES_MENU].height;

    /**Draw relevant part of back ground*/
    if (AK_FALSE == Fwl_AkBmpDrawPartFromString(HRGB_LAYER,
                                            pTopBarItem->pubItem[eTB_RES_MENU].position.x,
                                            pTopBarItem->pubItem[eTB_RES_MENU].position.y,
                                            &range, pTopBarItem->bckGrndIconData, AK_NULL, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowMenuButton(): Draw part of back ground fail");
        return AK_FALSE;
    }

    /**Draw Menu Button*/
    if (AK_FALSE == Fwl_AkBmpDrawFromString(HRGB_LAYER,
                                        pTopBarItem->pubItem[eTB_RES_MENU].position.x,
                                        pTopBarItem->pubItem[eTB_RES_MENU].position.y,
                                        pTopBarItem->menuButton,
                                        &g_Graph.TransColor, AK_FALSE))
    {
        Fwl_Print(C3, M_CTRL, "TopBar_ShowMenuButton(): Draw Menu Button fail");
        return AK_FALSE;
    }

    return AK_TRUE;
}//end of TopBar_ShowMenuButton


/**
 * @Brief   Get rect of Cancel Button
 * @Author  wangxuwen
 * @Data    2008-07-28
 * @Param   T_VOID
 * @Return  T_RECT:Rect of  Cancel Button
 */
T_RECT TopBar_GetRectofCancelButton(T_VOID)
{
  T_RECT rect;

  rect.left = pTopBarItem->pubItem[eTB_RES_CANCEL].position.x;
  rect.top = pTopBarItem->pubItem[eTB_RES_CANCEL].position.y;
  rect.width = pTopBarItem->pubItem[eTB_RES_CANCEL].width;
  rect.height = pTopBarItem->pubItem[eTB_RES_CANCEL].height;

  return rect;
}

/**
 * @Brief   Get rect of menu Button
 * @Author  
 * @Data   
 * @Param   T_VOID
 * @Return  T_RECT:Rect of  Cancel Button
 */
T_VOID TopBar_GetRectofMenuButton(T_pRECT pRect)
{
  pRect->left = pTopBarItem->pubItem[eTB_RES_MENU].position.x;
  pRect->top = pTopBarItem->pubItem[eTB_RES_MENU].position.y;
  pRect->width = pTopBarItem->pubItem[eTB_RES_MENU].width;
  pRect->height = pTopBarItem->pubItem[eTB_RES_MENU].height;
}

T_BOOL TopBar_GetMenuButtonState(T_VOID)
{
    return pTopBarItem->MenuIsValid;
}

T_BOOL TopBar_GetMenuButtonShowState(T_VOID)
{
	return pTopBarItem->bMenuIconShow;
}

//end of Eng_TopBar.c
