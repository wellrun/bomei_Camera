/**
 * @file fwl_gm.c
 * @brief Game framework layer
 *
 * @author ZouMai
 * @date 2004-01-05
 * @version 1.0
 */

#include "fwl_gm.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_Topbar.h"
#include "Ctl_Fm.h"
#include "fwl_oscom.h"
#include "arch_lcd.h"
#include "fwl_display.h"

#ifdef OS_BREW
#else   /* not define OS_BREW */
    #include <string.h>
    #include "Fwl_pfAudio.h"
    #include "Fwl_pfDisplay.h"
    #include "Eng_AkBmp.h"
    #include "Eng_DataConvert.h"
    #include "Eng_Graph.h"
    #include "Eng_DynamicFont.h"
    #include "Eng_Font.h"
    #include "Eng_GblString.h"

    static const unsigned char gb_bitMask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
#endif  /* end of #ifdef OS_BREW */

typedef struct _T_GAME_FWL {
    T_U8            VDspMem[GM_SCREEN_WIDTH*GM_SCREEN_HEIGHT*2+6];  /* Virtual diaplay memory */
    T_GM_COLOR      textColor;              /* text color */
    T_GM_COLOR      winBackColor;           /* application window background color */
    T_GM_RECT       winRect;                /* application window rangle */
    T_POS           gameLeft;               /* game window left */
    T_POS           gameTop;                /* game window top */
    T_U32           timerMS;                /* millisecond of the last timer */
    T_S8            volume;                 /* volume */
//    T_MPlayer    *pMPlayer;             /*audio play handle*/
} T_GAME_FWL;

typedef struct {
    T_U16           dot21;
    T_U16           block;
    T_U16           boat;
    T_U16           egg;
    T_U16           race;
    T_U16           rect;
    T_U16           hero;
    T_U16           bead;
} T_GAME_SCORE;
T_VOID * Display_Temp_Buffer;
/*********************************** PUBLIC ***********************************/
/**
 * @brief Initialize framework layer handle
 * User should call this function before call any other framework layer functions, else unpredictable mistake may arise.
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL *phGmFwl: Pointer to the address of framework layer handle
 * @param void *pApp: Pointer to the game application layer structure
 * @return T_BOOL
 * @retval
 */


T_BOOL GmFwl_Init(T_GMFWL_HDL *phGmFwl, T_VOID *pApp)
{
    T_GMFWL_HDL hGmFwl;

#ifdef OS_BREW
#else   /* not define OS_BREW */
//  GameData data;

    if (phGmFwl == AK_NULL)
        return AK_FALSE;

    *phGmFwl = (T_GMFWL_HDL)MALLOC(sizeof(T_GAME_FWL));
    if (*phGmFwl == AK_NULL)
    {
        return AK_FALSE;
    }

    hGmFwl = *phGmFwl;
    MEMSET(hGmFwl, 0, sizeof(T_GAME_FWL));

    hGmFwl->VDspMem[0] = 0;
    hGmFwl->VDspMem[1] = 64;
    hGmFwl->VDspMem[2] = 1;
    hGmFwl->VDspMem[3] = 240;
    hGmFwl->VDspMem[4] = 0;
    hGmFwl->VDspMem[5] = 16;
    hGmFwl->textColor = RGB2AkColor(0x00, 0x00, 0x00, LCD0_COLOR);//LcdGetColor(DISPLAY_LCD_0));
    hGmFwl->winBackColor = LCD0_COLOR;//GetWinBkColor(DISPLAY_LCD_0);
    hGmFwl->winRect.left = 0;
    hGmFwl->winRect.top = 0;//LCD0_TOPBAR_HEIGHT;//GetTBHeight(DISPLAY_LCD_0);
    hGmFwl->winRect.width = Fwl_GetLcdWidth();
    hGmFwl->winRect.height = Fwl_GetLcdHeight();//GetMsHeight(DISPLAY_LCD_0)-16;
    hGmFwl->gameLeft = (T_POS)((GM_MAX(hGmFwl->winRect.width,GM_SCREEN_WIDTH)-GM_SCREEN_WIDTH)/2);
    hGmFwl->gameTop = (T_POS)((GM_MAX(hGmFwl->winRect.height,GM_SCREEN_HEIGHT)-GM_SCREEN_HEIGHT)/2);
    hGmFwl->timerMS = 0;
//  memapi_getGameData(data);
//  hGmFwl->volume = (T_S8)(data[8]);
    hGmFwl->volume = 1;
#if 0
    hGmFwl->pMPlayer = (T_MPlayer*)Fwl_Malloc(sizeof(T_MPlayer));
    AK_ASSERT_PTR(hGmFwl->pMPlayer, "hGmFwl->pMPlayer  malloc failed\n",AK_FALSE);
#endif

#endif  /* end of #ifdef OS_BREW */

    return AK_TRUE;
}

/**
 * @brief Free framework layer handle
 * User should call this function when exit mini karaoke application.
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL *phGmFwl: Pointer to the address of framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_Free(T_GMFWL_HDL *phGmFwl)
{
    T_GMFWL_HDL hGmFwl;

    if (phGmFwl == AK_NULL || *phGmFwl == AK_NULL)
        return;
    hGmFwl = *phGmFwl;
	
#if 0	
    FREE(hGmFwl->pMPlayer);
    hGmFwl->pMPlayer = AK_NULL;
#endif

    FREE(hGmFwl);
    *phGmFwl = AK_NULL;
}


T_VOID GmFwlAudioResume(T_VOID)
{
    TopBar_DisableShow();
}

/*********************************** KEY ***********************************/
T_U8 GmFwl_GetGameKey(T_U8 gameID, T_U32 userKey)
{
#ifdef OS_BREW
#else   /* not define #ifdef OS_BREW */
#if 0 
    switch (gameID) {
    case GM_ID_BOAT:
        switch (userKey) {
        case AP_KEY_UP:         return GMBOAT_KEY_UP;
        case AP_KEY_DOWN:       return GMBOAT_KEY_DOWN;
        case AP_KEY_LEFT:       return GMBOAT_KEY_LEFT;
        case AP_KEY_RIGHT:      return GMBOAT_KEY_RIGHT;
        case AP_KEY_SELECT:     return GMBOAT_KEY_OK;
        case AP_KEY_CLEAR:
        case AP_KEY_ONHOOK:     return GMBOAT_KEY_CANCEL;
        default:                return GMBOAT_KEY_OTHER;
        }
        break;
    case GM_ID_EGG:
        switch (userKey) {
        case AP_KEY_UP:         return GMEGG_KEY_UP;
        case AP_KEY_DOWN:       return GMEGG_KEY_DOWN;
        case AP_KEY_SELECT:     return GMEGG_KEY_OK;
        case AP_KEY_CLEAR:
        case AP_KEY_ONHOOK:     return GMEGG_KEY_CANCEL;
        default:                return GMEGG_KEY_OTHER;
        }
        break;
    default:
        break;
    }
#endif
#endif  /* end of #ifdef OS_BREW */
    return 0;
}

/*********************************** DISPLAY ***********************************/
/**
 * @brief Get application window range
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_GM_RECT *winRect
 * @return T_VOID
 * @retval
 */
T_VOID  GmFwl_GetWinRect(T_GMFWL_HDL hGmFwl, T_GM_RECT *winRect)
{
    if (hGmFwl == AK_NULL || winRect == AK_NULL)
        return;

    *winRect = hGmFwl->winRect;
}

/**
 * @brief Get mini karaoke text height
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_LEN: text height
 * @retval
 */
T_LEN   GmFwl_GetTextHeight(T_GMFWL_HDL hGmFwl)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    if (hGmFwl == AK_NULL)
        return 0;

    return g_Font.SCHEIGHT;
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Get mini karaoke text width
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param const T_U16 *text: text in unicode format
 * @param T_U16 textLen: text length. If textLen is 0, return 0.
 * @return T_LEN: text width
 * @retval
 */
T_LEN   GmFwl_GetTextWidth(T_GMFWL_HDL hGmFwl, const T_U16 *text, T_U16 textLen)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    if (hGmFwl == AK_NULL || text == AK_NULL || textLen == 0)
        return 0;

    return (T_LEN)(Utl_UByteCount(text,textLen)* g_Font.SCWIDTH);
    //return UniCodes2GB((T_U16 *)text, textLen, AK_NULL) * g_Font.SCWIDTH;
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Clean application window
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_CleanWinBackground(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    Fwl_FillSolidRect(HRGB_LAYER,
        hGmFwl->winRect.left,   hGmFwl->winRect.top,
        hGmFwl->winRect.width, hGmFwl->winRect.height,
        hGmFwl->winBackColor);

#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Clean display memory.
 * 
 * @author ZouMai
 * @date 2003-08-09
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param const T_GM_RECT *destRect
 * @return T_VOID
 * @retval 
 */
T_VOID  GmFwl_VDspCleanRect(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_COLOR color)
{
    T_POS   i= 0;
    T_POS   j = 0;
    T_POS   right = 0;
    T_POS   bottom = 0;
    T_U16   *pDest = AK_NULL;

    if (hGmFwl == AK_NULL || destRect == AK_NULL)
        return;

    right = (T_POS)(destRect->left + destRect->width);
    bottom = (T_POS)(destRect->top + destRect->height);
    if (right < 0 || bottom < 0 || destRect->left >= GM_SCREEN_WIDTH || destRect->top >= GM_SCREEN_HEIGHT)
        return;

    if (right > GM_SCREEN_WIDTH)
        right = GM_SCREEN_WIDTH;
    if (bottom > GM_SCREEN_HEIGHT)
        bottom = GM_SCREEN_HEIGHT;

    for (i = (T_POS)GM_MAX(destRect->top, 0); i < bottom; i++)
    {
        j = (T_POS)GM_MAX(destRect->left, 0);
        pDest = (T_U16 *)(hGmFwl->VDspMem + 6 + i * GM_SCREEN_WIDTH * 2 + j * 2);
        for (; j < right; j++, pDest++)
        {
            *pDest = (T_U16)((color>>19<<11) | (((T_U8)(color>>8))>>2<<5) | (((T_U8)color>>3)));
        }
    }
}

/**
 * @brief Draw image data to display memory
 * 
 * @author ZouMai
 * @date 2003-08-09
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param const T_GM_RECT *destRect
 * @param T_pCDATA imageData
 * @param T_POS sourX
 * @param T_POS sourY
 * @param T_pCDATA trans
 * @return T_VOID
 * @retval 
 */
T_VOID  GmFwl_VDspDrawImage(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_pCDATA imageData, T_POS sourX, T_POS sourY, T_pCDATA trans)
{
    T_POS       i, j;
    T_POS       right;
    T_POS       bottom;
    T_pDATA     pDest;
    T_pDATA     pSour;
    T_LEN       imageWidth;
    T_LEN       imageHight;

    if (hGmFwl == AK_NULL || destRect == AK_NULL)
        return;

    AKBmpGetInfo(imageData, &imageWidth, &imageHight, AK_NULL);
    right = (T_POS)(destRect->left + GM_MIN(destRect->width, imageWidth));
    bottom = (T_POS)(destRect->top + GM_MIN(destRect->height, imageHight));
    if (right < 0 || bottom < 0 || destRect->left >= GM_SCREEN_WIDTH || destRect->top >= GM_SCREEN_HEIGHT)
        return;

    if (right > GM_SCREEN_WIDTH)
        right = GM_SCREEN_WIDTH;
    if (bottom > GM_SCREEN_HEIGHT)
        bottom = GM_SCREEN_HEIGHT;

    for (i = (T_POS)GM_MAX(destRect->top, 0); i < bottom; i++)
    {
        j = (T_POS)GM_MAX(destRect->left, 0);
        pDest = (T_pDATA)(hGmFwl->VDspMem+6+i*GM_SCREEN_WIDTH*2+j*2);
        pSour = (T_pDATA)(imageData+6+((i-destRect->top+sourY)*imageWidth+(j-destRect->left+sourX))*2);
        for (; j < right; j++)
        {
            if (!(trans != AK_NULL && *(pSour) == trans[0] && *(pSour+1) == trans[1]))
            {
                *(pDest) = *(pSour);
                *(pDest+1) = *(pSour+1);
            }
            pSour += 2;
            pDest += 2;
        }
    }
    return;
}

T_VOID  GmFwl_DisplayVDsp(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect)
{
    if (hGmFwl == AK_NULL || destRect == AK_NULL)
        return;

    GmFwl_DisplayImage(hGmFwl, destRect, (const T_pDATA)hGmFwl->VDspMem, 0, 0);
}

T_VOID  GmFwl_DisplayImage(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_pCDATA imageData, T_POS sourX, T_POS sourY)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    if (hGmFwl == AK_NULL || destRect == AK_NULL || imageData == AK_NULL)
    {
        return;
    }

    Fwl_AkBmpDrawFromString(HRGB_LAYER,
        (T_POS)(hGmFwl->winRect.left+hGmFwl->gameLeft+destRect->left),
        (T_POS)(hGmFwl->winRect.top+hGmFwl->gameTop+destRect->top),
        imageData, AK_NULL, AK_FALSE);
#endif  /* end of #ifdef OS_BREW */
}

T_VOID  GmFwl_DrawImage(HLAYER hLayer, T_POS x, T_POS y, T_pCDATA BmpString, T_COLOR *bkColor, T_BOOL Reverse)
{
    T_LEN   w = 0;
    T_LEN   h = 0;
    T_RECT  rect;
    

    if ((x >= 0) && (y >= 0))
    {
        Fwl_AkBmpDrawFromString(hLayer, x, y, BmpString, bkColor, Reverse);
    }
    else
    {
        rect.left = 0;
        rect.top = 0;

        if (x < 0)
        {
            rect.left = -x;
            x = 0;
        }
        if (y < 0)
        {
            rect.top = -y;
            y = 0;
        }
        
        AKBmpGetInfo(BmpString, &w, &h, AK_NULL);

        rect.width = w - rect.left;
        rect.height= h - rect.top;
        
        Fwl_AkBmpDrawPartFromString(hLayer, x, y, &rect, BmpString, bkColor, AK_FALSE);
    }
}

/**
 * @brief Display text
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_POS destX: x-coordinate of the begining of the text
 * @param T_POS destY: y-coordinate of the begining of the text
 * @param const T_U16 *text: text in unicode format
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_DisplayText(DISPLAY_TYPE_DEV LCD, T_POS destX, T_POS destY, const T_U8 *text)
{
    T_USTR_INFO   uText;    /*unicode text*/
           
    Eng_StrMbcs2Ucs((T_S8 *)text, uText);
    
#ifdef OS_BREW
#else       /* not define OS_BREW */
    
    Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(destX), (T_POS)(destY),
        uText, RGB2AkColor(255, 255, 255, LCD0_COLOR), CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText));
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Update the screen
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param const T_GM_RECT *destRect
 * @return T_VOID
 * @retval
 */
T_VOID  GmFwl_InvalidateRect(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect)
{
    if (hGmFwl == AK_NULL)
        return;
#ifdef OS_BREW
#else   /* not define OS_BREW */
    Fwl_RefreshDisplay();
#endif  /* end of #ifdef OS_BREW */
}

/*********************************** SOUND ***********************************/
/**
 * @brief Get karaoke sound volume
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_S8
 * @retval
 */
T_S8 GmFwl_GetVolume(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return -1;

    return hGmFwl->volume;
}

/**
 * @brief Set karaoke sound volume
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param volume
 * @return T_BOOL 
 * @retval
 */
T_BOOL  GmFwl_SetVolume(T_GMFWL_HDL hGmFwl, T_S8 volume)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */

    if (hGmFwl == AK_NULL)
        return AK_FALSE;

    hGmFwl->volume = volume;
#endif  /* end of #ifdef OS_BREW */
    return AK_TRUE;
}

T_VOID GmFwl_SoundEndCallBack(T_END_TYPE endType)
{
	MPlayer_Close();
}
/**
 * @brief Play karaoke sound
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_pCDATA soundData: sound data
 * @param T_U8 loop: loop
 * @param T_U32 size: sound data size
 * @return T_BOOL
 * @retval
 */
T_BOOL GmFwl_SoundPlay(T_GMFWL_HDL hGmFwl, const T_pDATA soundData, T_U8 loop, T_U32 size)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    T_S32   volume;
    T_U8 MidiFlag[] = {0x4d,0x54,0x68,0x64};
    T_U8 AudioType;
  

    if (hGmFwl == AK_NULL || soundData == AK_NULL || loop == 0 || size == 0)
        return AK_FALSE;
    if (GmFwl_GetVolume(hGmFwl) <= 0)
        return AK_FALSE;

    volume = 3;

    if (memcmp((void *)MidiFlag, (void *)soundData, sizeof(MidiFlag)) == 0)
        AudioType = _SD_MEDIA_TYPE_MIDI;
    else
        AudioType = _SD_MEDIA_TYPE_MP3;
    if(Fwl_MP3AudioOpenBuffer(soundData, size))
    {
		return MP3Player_Play(0);
    }
#endif  /* end of #ifdef OS_BREW */

    return AK_TRUE;
}

/**
 * @brief Stop karaoke sound
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_SoundStop(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    Fwl_AudioStop(T_END_TYPE_USER);
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Set key touch tone always ON
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID  GmFwl_KeyToneAlwaysOff(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */

#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Resume key touch tone state
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID  GmFwl_KeyToneResume(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */
#endif  /* end of #ifdef OS_BREW */
}

/*********************************** VIBRATOR ***********************************/
T_VOID  GmFwl_VibraPlay(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;
}

T_VOID  GmFwl_VibraStop(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;
}

/*********************************** BACKLIGHT ***********************************/
/**
 * @brief Set BACKLIGHT always ON
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_BacklightAlwaysOn(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Resume BACKLIGHT state
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_BacklightResume(T_GMFWL_HDL hGmFwl)
{
    if (hGmFwl == AK_NULL)
        return;

#ifdef OS_BREW
#else   /* not define OS_BREW */
//  na_sms_light_off();
#endif  /* end of #ifdef OS_BREW */
}

/*********************************** TIMER ***********************************/
//static T_VOID GmFwl_HandleBoatTimer(void *pi);
//static T_VOID GmFwl_HandleEggTimer(void *pi);

/**
 * @brief Set timer
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_U32 milliSeconds: milliseconds
 * @return T_GM_TIMER
 * @retval
 */
T_GM_TIMER  GmFwl_SetTimer(T_GMFWL_HDL hGmFwl, T_U32 milliSeconds, T_U8 gameID)
{
    if (hGmFwl == AK_NULL)
        return GM_ERROR_TIMER;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    return Fwl_SetTimerMilliSecond(milliSeconds, AK_TRUE);
#endif  /* end of #ifdef OS_BREW */
    return GM_ERROR_TIMER;
}

/**
 * @brief Stop timer
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_GM_TIMER timerHandle:
 * @return T_VOID
 * @retval
 */
T_VOID GmFwl_StopTimer(T_GMFWL_HDL hGmFwl, T_GM_TIMER timerHandle)
{
    if (hGmFwl == AK_NULL || timerHandle == GM_ERROR_TIMER)
        return;
#ifdef OS_BREW
#else   /* not define OS_BREW */
    Fwl_StopTimer(timerHandle);
    return;
#endif  /* end of #ifdef OS_BREW */
}

/*********************************** STRING AND MEMORY ***********************************/

/*********************************** FILE SYSTEM ***********************************/
T_pGM_FILE  GmFwl_FileOpen(T_GMFWL_HDL hGmFwl, const T_pSTR path, T_GM_FILE_MODE mode)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    T_USTR_FILE Ustr;

    if (path == AK_NULL)
        return GM_FOPEN_FAIL;

    Eng_StrMbcs2Ucs(path, Ustr);

    if (mode == GM_FILE_READ)
        return Fwl_FileOpen(Ustr, mode, mode);
    else if (mode == GM_FILE_TRUNCATE)
        return Fwl_FileOpen(Ustr, mode, mode);
    else
        return Fwl_FileOpen(Ustr, mode, mode);
#endif  /* end of #ifdef OS_BREW */
}

T_U32   GmFwl_FileRead(T_pGM_FILE pFile, T_pDATA buffer, T_U32 count)
{
    T_U32   readed = 0;
    if (pFile== GM_FOPEN_FAIL)
        return 0;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    readed = Fwl_FileRead(pFile, buffer, count);
    if(readed == 0)
        return 0;
    return readed;
#endif  /* end of #ifdef OS_BREW */
}

T_U32   GmFwl_FileWrite(T_pGM_FILE pFile, T_pCDATA buffer, T_U32 count)
{
    T_U32   writed = 0;
    if (pFile== GM_FOPEN_FAIL)
        return 0;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    writed = Fwl_FileWrite(pFile, buffer, count);
    if(writed == 0)
        return 0;
    return writed;
#endif  /* end of #ifdef OS_BREW */
}

T_BOOL  GmFwl_FileClose(T_pGM_FILE pFile)
{
    if (pFile== GM_FOPEN_FAIL)
        return AK_FALSE;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    return Fwl_FileClose(pFile);
#endif  /* end of #ifdef OS_BREW */
}

T_S32   GmFwl_FileSeek(T_pGM_FILE pFile, T_U32 offset, T_U16 origin)
{
    if (pFile== GM_FOPEN_FAIL)
        return 0;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    return Fwl_FileSeek(pFile, offset, origin);
#endif  /* end of #ifdef OS_BREW */
}

T_BOOL  GmFwl_FileDelete(T_GMFWL_HDL hGmFwl, const T_pWSTR path)
{
    if (hGmFwl == AK_NULL || path == AK_NULL)
        return AK_FALSE;

#ifdef OS_BREW
#else   /* not define OS_BREW */
    return Fwl_FileDelete(path);
#endif  /* end of #ifdef OS_BREW */
}

T_U16 GmScore_GetDataSize(T_VOID)
{
    return sizeof(T_GAME_SCORE);
}

/**
 * @brief Read game reocrd
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_U8 gameID:
 * @return T_U16: game record
 * @retval
 */
T_U16  GmFwl_ReadRecord(T_GMFWL_HDL hGmFwl, T_U8 gameID)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    T_pGM_FILE  pFile;
    T_U16       record;

    switch (gameID) {
    case GM_ID_BOAT:
        pFile = GmFwl_FileOpen(hGmFwl, GMBOAT_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_EGG:
        pFile = GmFwl_FileOpen(hGmFwl, GMEGG_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_21:
        pFile = GmFwl_FileOpen(hGmFwl, GM21_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_BLOCK:
        pFile = GmFwl_FileOpen(hGmFwl, GMBLOCK_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_RACE:
        pFile = GmFwl_FileOpen(hGmFwl, GMRACE_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_RECT:
        pFile = GmFwl_FileOpen(hGmFwl, GMRECT_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_HERO:
        pFile = GmFwl_FileOpen(hGmFwl, GMHERO_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    case GM_ID_BEAD:
        pFile = GmFwl_FileOpen(hGmFwl, GMBEAD_RECORD_FILE_NAME, GM_FILE_READ);
        break;
    default:
        pFile = GM_FOPEN_FAIL;
        break;
    }
    if (pFile == GM_FOPEN_FAIL)
        return 0;

    if (GmFwl_FileRead(pFile, (T_pDATA)&record, 2) != 2)
    {
        record = 0;
    }
    GmFwl_FileClose(pFile);

    return record;
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Save game reocrd
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_U8 gameID:
 * @param T_U16 record: record
 * @return T_BOOL
 * @retval
 */
T_BOOL GmFwl_SaveRecord(T_GMFWL_HDL hGmFwl, T_U8 gameID, T_U16 record)
{
#ifdef OS_BREW
#else   /* not define OS_BREW */
    T_pGM_FILE      pFile;

    if (hGmFwl == AK_NULL)
        return AK_FALSE;
    
    switch (gameID) {
    case GM_ID_BOAT:
        pFile = GmFwl_FileOpen(hGmFwl, GMBOAT_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_EGG:
        pFile = GmFwl_FileOpen(hGmFwl, GMEGG_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_21:
        pFile = GmFwl_FileOpen(hGmFwl, GM21_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_BLOCK:
        pFile = GmFwl_FileOpen(hGmFwl, GMBLOCK_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_RACE:
        pFile = GmFwl_FileOpen(hGmFwl, GMRACE_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_RECT:
        pFile = GmFwl_FileOpen(hGmFwl, GMRECT_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_HERO:
        pFile = GmFwl_FileOpen(hGmFwl, GMHERO_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    case GM_ID_BEAD:
        pFile = GmFwl_FileOpen(hGmFwl, GMBEAD_RECORD_FILE_NAME, GM_FILE_CREATE);
        break;
    default:
        pFile = GM_FOPEN_FAIL;
        break;
    }
    if (pFile == GM_FOPEN_FAIL)
        return AK_FALSE;

    if (GmFwl_FileWrite(pFile, (T_pDATA)&record, 2) != 2)
    {
        GmFwl_FileClose(pFile);
        return AK_FALSE;
    }
    GmFwl_FileClose(pFile);

    return AK_TRUE;
#endif  /* end of #ifdef OS_BREW */
}

/**
 * @brief Process game over score
 * 
 * @author ZouMai
 * @date 2004-01-12
 * @param T_GMFWL_HDL hGmFwl: Pointer to the framework layer handle
 * @param T_U8 gameID:
 * @param T_U16 record: record
 * @return T_BOOL
 * @retval
 */
T_BOOL  GmFwl_GameOverScore(T_GMFWL_HDL hGmFwl, T_U8 gameID, T_U16 score)
{
    if (hGmFwl == AK_NULL)
        return AK_FALSE;

    if (score > GmFwl_ReadRecord(hGmFwl, gameID))
    {
        return GmFwl_SaveRecord(hGmFwl, gameID, score);
    }
    return AK_TRUE;
}



#if 0

T_VOID GmFwl_double_Img_BitBlt(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
					 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
					 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat)
{

#ifdef OS_ANYKA

	T_U16 WidthTemp,HeightTemp;
	WidthTemp=scaleWidth/2;
	HeightTemp=scaleHeight/2;


    if( (WidthTemp > srcWidth) || (HeightTemp > srcHeight) )
    {
       if(AK_NULL==Display_Temp_Buffer)
	   {
	       Display_Temp_Buffer=Fwl_Malloc(WidthTemp*HeightTemp*2);
		   if(AK_NULL==Display_Temp_Buffer)
		   {
		       return;
		   }
	   }
	
       Img_BitBlt(Display_Temp_Buffer, WidthTemp, HeightTemp, 0, 0, WidthTemp,
			srcBuf, srcWidth, srcHeight, srcFormat);
	
       Img_BitBlt(dstBuf, scaleWidth, scaleHeight, dstPosX, dstPosY, dstWidth,
			Display_Temp_Buffer, WidthTemp, HeightTemp, 1);

    }
	else
	{
	   Img_BitBlt(dstBuf, scaleWidth, scaleHeight, dstPosX, dstPosY, dstWidth,
			srcBuf, srcWidth, srcHeight, srcFormat);
	}

#endif
}

#endif

T_VOID GmFwl_double_Img_BitBlt_Free(T_VOID)
{    
#ifdef OS_ANYKA

	if(Display_Temp_Buffer)
	{
	    Fwl_Free(Display_Temp_Buffer);
	}
	Display_Temp_Buffer=AK_NULL;
#endif	
}


/*********************************** IFILE ***********************************/
#ifdef GM_DEFINE_IFILE_API
T_VOID  IFile_Init(T_IFILE *piFile, T_U32 maxlen)
{
    if (piFile == AK_NULL)
        return;

    piFile->maxLength = maxlen;
    IFile_Seek(piFile, IFILE_SEEK_SET, 0);
}

T_U32   IFile_Read(T_IFILE *piFile, T_pDATA buffer, T_U32 count)
{
    if (piFile == AK_NULL || buffer == AK_NULL)
        return 0;
    
    if (piFile->from == IFILE_FROM_BUFF)
    {
        T_U32       i;
        T_pDATA     tmpData;

        if (piFile->curOffset + count > piFile->maxLength)
            count = piFile->maxLength - piFile->curOffset;

        tmpData = piFile->fd.pData + piFile->curOffset;
        for (i = 0; i < count; i++)
        {
            *buffer = *tmpData;
            buffer++;
            tmpData++;
        }
        piFile->curOffset += count;
        return count;
    }
    else if (piFile->from == IFILE_FROM_FILE_PTR)
    {
        return IFILE_READ(piFile->fd.pFile, buffer, count);
    }
    return 0;
}

T_U32 IFile_ReadChr(T_IFILE *piFile, T_U8 *chr)
{
    if (piFile == AK_NULL || chr == AK_NULL)
        return 0;
    
    if (piFile->from == IFILE_FROM_BUFF)
    {
        if (piFile->curOffset + 1 > piFile->maxLength)
            return 0;

        *chr = *(piFile->fd.pData + piFile->curOffset);
        piFile->curOffset++;
        return 1;
    }
    else if (piFile->from == IFILE_FROM_FILE_PTR)
    {
        return IFILE_READ(piFile->fd.pFile, (T_pDATA)chr, 1);
    }
    return 0;
}

T_pDATA IFile_ReadAddr(T_IFILE *piFile, T_U32 count)
{
    if (piFile == AK_NULL)
        return AK_NULL;
    
    if (piFile->from == IFILE_FROM_BUFF)
    {
        T_pDATA     tmpData;

        if (piFile->curOffset + count > piFile->maxLength)
            count = piFile->maxLength - piFile->curOffset;

        tmpData = piFile->fd.pData + piFile->curOffset;
        piFile->curOffset += count;
        return tmpData;
    }
    else
    {
        return AK_NULL;
    }
}

T_U32   IFile_Write(T_IFILE *piFile, T_pCDATA buffer, T_U32 count)
{
    if (piFile == AK_NULL || buffer == AK_NULL)
        return 0;
    
    if (piFile->from == IFILE_FROM_BUFF)
    {
        T_U32       i;
        T_pDATA     tmpData;

        if (piFile->curOffset + count > piFile->maxLength)
            count = piFile->maxLength - piFile->curOffset;

        tmpData = piFile->fd.pData + piFile->curOffset;
        for (i = 0; i < count; i++)
        {
            *tmpData = *buffer;
            buffer++;
            tmpData++;
        }
        piFile->curOffset += count;
        return count;
    }
    else if (piFile->from == IFILE_FROM_FILE_PTR)
    {
        return IFILE_WRITE(piFile->fd.pFile, buffer, count);
    }
    return 0;
}

T_U32   IFile_WriteChr(T_IFILE *piFile, T_U8 chr)
{
    if (piFile == AK_NULL)
        return 0;
    
    if (piFile->from == IFILE_FROM_BUFF)
    {
        if (piFile->curOffset + 1 > piFile->maxLength)
            return 0;

        *(piFile->fd.pData + piFile->curOffset) = chr;
        piFile->curOffset++;
        return 1;
    }
    else if (piFile->from == IFILE_FROM_FILE_PTR)
    {
        return IFILE_WRITE(piFile->fd.pFile, (const T_pDATA)&chr, 1);
    }
    return 0;
}

T_S32   IFile_Seek(T_IFILE *piFile, T_U16 origin, T_U32 offset)
{
    if (piFile == AK_NULL)
        return 0;

    if (piFile->from == IFILE_FROM_BUFF)
    {
        if (origin == IFILE_SEEK_END)
            piFile->curOffset = piFile->maxLength-1;
        else if (origin == IFILE_SEEK_SET)
            piFile->curOffset = 0;
        piFile->curOffset += offset;
        if (piFile->curOffset > piFile->maxLength)
            piFile->curOffset = piFile->maxLength;
        return piFile->curOffset;
    }
    else if (piFile->from == IFILE_FROM_FILE_PTR)
    {
        return IFILE_SEEK(piFile->fd.pFile, offset, origin);
    }
    return 0;
}

/**
 * @brief Read a unit
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_IFILE *piFile
 * @param T_U8 bytes
 * @return T_U32
 * @retval
 */
T_U32 IFile_ReadUnit(T_IFILE *piFile, T_U8 bytes)
{
    T_U32       ret = 0;
    T_U8        i;
    T_U8        buf;
    
    if (piFile == AK_NULL)
        return ret;

    for (i = 0; i < bytes; i++)
    {
        if (IFile_ReadChr(piFile, &buf) != 1)
            return 0;
        ret = (ret<<8) + buf;
    }

    return ret;
}

/**
 * @brief Read a unit in reversed mode
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_IFILE *piFile
 * @param T_U8 bytes
 * @return T_U32
 * @retval
 */
T_U32 IFile_ReadUnitR(T_IFILE *piFile, T_U8 bytes)
{
    T_U32       ret = 0;
    T_U8        i;
    T_U8        buf;
    
    if (piFile == AK_NULL)
        return ret;

    for (i = 0; i < bytes; i++)
    {
        if (IFile_ReadChr(piFile, &buf) != 1)
            return 0;
        ret += (buf << (8 * i));
    }

    return ret;
}

/**
 * @brief Read multiple U16 data
 *
 * @author ZouMai
 * @date 2004-01-12
 * @param T_IFILE *piFile
 * @param T_U16 *retData
 * @param T_U16 count
 * @return T_BOOL
 * @retval
 */
T_BOOL IFile_ReadMultiU16(T_IFILE *piFile, T_U16 *retData, T_U16 count)
{
    T_U8        i;
    T_U8        buf[2];
    
    if (piFile == AK_NULL)
        return AK_FALSE;

    for (i = 0; i < count; i++)
    {
        if (IFile_Read(piFile, buf, 2) != 2)
            return AK_FALSE;
        retData[i] = (T_U16)((buf[0] << 8) + buf[1]);
    }

    return AK_TRUE;
}
#endif

/*********************************** OTHERS ***********************************/

