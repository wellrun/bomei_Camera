
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_Time.h"
#include "Eng_Calendar.h"
#include "Eng_TopBar.h"
#include "Eng_AkBmp.h"
#include "Eng_Calendar.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "lib_image_api.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Fwl_tscrcom.h"


typedef struct
{
    T_pCDATA        GreNumIcon[10];
    T_pCDATA        LunNumIcon[13];
}T_DATE;

typedef enum
{
    SEASON_SPRING = 0,
    SEASON_SUMMER,
    SEASON_AUTUMN,
    SEASON_WINTER,
}T_SEASON;

typedef struct {
    T_pCDATA    pBckImg[4];     //background image
    T_pCDATA    pWkImg[4][2];   //week bar image
    T_pCDATA    pFcsImg;        //focus block image
    T_pCDATA    pRetImg[2];     //return button image
    T_pCDATA    pLftImg[2];     //left button image
    T_pCDATA    pRghtImg[2];    //right button image
    T_pCDATA    pFcsBtnImg;     //focus button image
    T_RECT      BckImgRect;     //background image rect
    T_RECT      WkImgRect;      //week bar image rect
    T_RECT      FcsImgRect;     //focus block image rect
    T_RECT      RetImgRect;     //return button image rect
    T_RECT      LftImgRect;     //left button image rect
    T_RECT      RghtImgRect;    //right button image rect
    T_RECT      FcsBtnImgRect;  //right button image rect
    T_DATE      date;
}T_CALENDAR_RES;

typedef struct {
    T_CALENDAR_RES  res;
    T_SYSTIME       curTime;
    T_POS           fcsPosX;
    T_POS           fcsPosY;
    T_LEN           wkWd;           //week width
    T_LEN           lftFrmW;        //left frame width    
    T_LEN           rowH;           //row height
    T_MSGBOX        msgbox;
    T_BOOL          bFcsBtn;
    T_BOOL          refreshflag;
}T_CALENDAR_PARM;

T_CALENDAR_PARM *pCalendar_Parm = AK_NULL;

static T_VOID Calendar_GetRes(T_CALENDAR_RES *pRes);
static T_VOID Calendar_InitResRect(T_CALENDAR_RES *pRes);
static T_SEASON Calendar_CheckCurSeason(T_U8 month);
static T_U8 Calendar_CheckCurLanguage(T_VOID);
static T_VOID Calendar_GetCurFcsRect(T_pRECT pFcsRest);
static T_VOID Calendar_GetSpecDateRect(T_pRECT pDateRest, T_pSYSTIME pTime);
static T_VOID Calender_UserKey_Handle(T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm);
static T_VOID Calender_HitButton_Handle(T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm);

static T_S16        weekdays, monthdays;

/*---------------------- BEGIN OF STATE s_func_calendar ------------------------*/


static T_VOID Calendar_ResumeFunc(T_VOID)
{
   Calendar_GetRes(&pCalendar_Parm->res);    
}
#endif
void initfunc_calendar(void)
{
#ifdef SUPPORT_SYS_SET

    TopBar_DisableShow();

    pCalendar_Parm = (T_CALENDAR_PARM *)Fwl_Malloc(sizeof(T_CALENDAR_PARM));
    AK_ASSERT_PTR_VOID(pCalendar_Parm, "initfunc_calendar(): malloc error");

    pCalendar_Parm->curTime = GetSysTime();
    Calendar_GetRes(&pCalendar_Parm->res);
    Calendar_InitResRect(&pCalendar_Parm->res);
    pCalendar_Parm->fcsPosX = 0;
    pCalendar_Parm->fcsPosY = 0;
    //设置星期列宽度、星期日列离背景左边距、行高。
    pCalendar_Parm->wkWd = Fwl_GetLcdWidth() / 7;
#if (LCD_CONFIG_WIDTH==800)
	pCalendar_Parm->rowH = 50;
#else
    pCalendar_Parm->rowH = 25;
#endif
    pCalendar_Parm->lftFrmW = (Fwl_GetLcdWidth() % 7) / 2;
    pCalendar_Parm->bFcsBtn = AK_FALSE;
    pCalendar_Parm->refreshflag = AK_TRUE;

    m_regResumeFunc(Calendar_ResumeFunc);
#endif
}

void exitfunc_calendar(void)
{
#ifdef SUPPORT_SYS_SET

    pCalendar_Parm = Fwl_Free(pCalendar_Parm);

    TopBar_EnableShow();
#endif
}

void paintfunc_calendar(void)
{
#ifdef SUPPORT_SYS_SET

    T_CALENDAR_RES  *pCalendarRes = AK_NULL;
    T_U16           strMonth[6] = {0};
    T_U16           strDay[8] = {0};
    T_USTR_INFO     uText;
    T_S8            tmpstr[100];
    T_U8            i, StartRow;
    T_pRECT         pWeekRect = AK_NULL;
    T_pRECT         pBkImgRect = AK_NULL;
    T_POS           posX = 0;
    T_POS           posY = 0;
    T_LEN           numIconWidth = 9;
    T_LEN           numIconHeight = 9;
    T_U16           UStrLen = 0;
    T_pSYSTIME      pCurTime = AK_NULL;
    T_SEASON        season = SEASON_SPRING;
    T_U8            retLan = 0;

    if (AK_TRUE != pCalendar_Parm->refreshflag)
    {
        return;
    }

    pCurTime = &pCalendar_Parm->curTime;
    if (pCurTime->year > END_YEAR)
    {
        return;
    }
    
    GetLunarDate(pCurTime->year, pCurTime->month, pCurTime->day);
    monthdays = MonthDays(pCurTime->year, pCurTime->month);
    weekdays = WeekDay(pCurTime->year, pCurTime->month, pCurTime->day);

    //画标题和背景。
    pCalendarRes = &pCalendar_Parm->res;
    pWeekRect = &pCalendarRes->WkImgRect;
    pBkImgRect = &pCalendarRes->BckImgRect;

    season = Calendar_CheckCurSeason(pCurTime->month);
    retLan = Calendar_CheckCurLanguage();

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pWeekRect->left, pWeekRect->top, pCalendarRes->pWkImg[season][retLan], AK_NULL, AK_FALSE);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pBkImgRect->left, pBkImgRect->top, pCalendarRes->pBckImg[season], AK_NULL, AK_FALSE);
    
    //获取数字图片宽高，所有数字图片尺寸一样。
    if (AK_NULL != pCalendarRes->date.GreNumIcon[0])
    {
        AKBmpGetInfo(pCalendarRes->date.GreNumIcon[0], &numIconWidth, &numIconHeight, AK_NULL);
    }

    //计算当前focus块位置，并显示focus块。
    if (WeekDay(pCurTime->year, pCurTime->month, 1) == 0)
    {
        StartRow = (6 - weekdays + pCurTime->day)/7 - 1;
    }
    else
    {
        StartRow = (6 - weekdays + pCurTime->day)/7;
    }
   
    pCalendar_Parm->fcsPosX = pCalendar_Parm->lftFrmW + weekdays * pCalendar_Parm->wkWd\
                            + (pCalendar_Parm->wkWd - pCalendar_Parm->res.FcsImgRect.width)/2;
    pCalendar_Parm->fcsPosY = pBkImgRect->top + (StartRow + 1) * pCalendar_Parm->rowH\
                            + (pCalendar_Parm->rowH - pCalendar_Parm->res.FcsImgRect.width)/2;
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pCalendar_Parm->fcsPosX, pCalendar_Parm->fcsPosY,\
        &pCalendarRes->FcsImgRect, pCalendarRes->pFcsImg, AK_NULL, AK_FALSE);

    //计算日期坐标，并显示。
    for (StartRow=0,i=1; i <= monthdays; i ++)
    {
        weekdays = WeekDay(pCurTime->year,pCurTime->month,i);
        if (0 == weekdays)
        {
            posX = pCalendar_Parm->lftFrmW + (pCalendar_Parm->wkWd - numIconWidth) / 2;
        }
        else
        {
            posX = pCalendar_Parm->lftFrmW + weekdays * pCalendar_Parm->wkWd + (pCalendar_Parm->wkWd - numIconWidth) / 2;
        }
        posY = pBkImgRect->top + (StartRow + 1) * pCalendar_Parm->rowH + (pCalendar_Parm->rowH - numIconHeight)/2;

        if(i/10 == 0)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, posX, posY, pCalendarRes->date.GreNumIcon[i%10], \
                                &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            posX = posX - numIconWidth/2;
            Fwl_AkBmpDrawFromString(HRGB_LAYER, posX, posY, pCalendarRes->date.GreNumIcon[i/10], \
                                &g_Graph.TransColor, AK_FALSE);
            posX = posX + numIconWidth;
            Fwl_AkBmpDrawFromString(HRGB_LAYER, posX, posY, pCalendarRes->date.GreNumIcon[i%10], \
                                &g_Graph.TransColor, AK_FALSE);
        }

        if(6 == weekdays)
        {
            StartRow ++ ;
        }
    }

    memset(uText, 0, sizeof(uText));
    if ((gs.Lang == eRES_LANG_CHINESE_BIG5) \
        || (gs.Lang == eRES_LANG_CHINESE_SIMPLE) \
        || (gs.Lang == eRES_LANG_CHINESE_TRADITION))
    {
        FormatMonth(iLunarMonth, strMonth, AK_TRUE);
        FormatLunarDay(iLunarDay, strDay);
        sprintf(tmpstr, "%4d.%02d  ", pCurTime->year, pCurTime->month);
        Eng_StrMbcs2Ucs(tmpstr, uText);
        Utl_UStrCat(uText, Res_GetStringByID(eRES_STR_CAL_LUNAR));
        Utl_UStrCat(uText, strMonth);
        Utl_UStrCat(uText, strDay);
    }
    else
    {
        sprintf(tmpstr, "%4d . %02d", pCurTime->year, pCurTime->month);
        Eng_StrMbcs2Ucs(tmpstr, uText);
    }

    UStrLen = (T_U16)Utl_UStrLen(uText);
    posX = (T_POS)(Fwl_GetLcdWidth() - UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, UStrLen)) / 2;
    posY = pBkImgRect->top + (pCalendar_Parm->rowH - GetFontHeight(CURRENT_FONT_SIZE))/2;

    Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, uText, COLOR_BLACK , CURRENT_FONT_SIZE, UStrLen);

    //show button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pCalendarRes->RetImgRect.left, pCalendarRes->RetImgRect.top, \
            pCalendarRes->pRetImg[0], &g_Graph.TransColor, AK_FALSE);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pCalendarRes->LftImgRect.left, pCalendarRes->LftImgRect.top, \
            pCalendarRes->pLftImg[0], &g_Graph.TransColor, AK_FALSE);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pCalendarRes->RghtImgRect.left, pCalendarRes->RghtImgRect.top, \
            pCalendarRes->pRghtImg[0], &g_Graph.TransColor, AK_FALSE);

    if (AK_TRUE == pCalendar_Parm->bFcsBtn)
    {
        if (AK_NULL != pCalendarRes->pRetImg[1])
        {
            pCalendarRes->FcsBtnImgRect.left = pCalendarRes->RetImgRect.left + (pCalendarRes->RetImgRect.width - pCalendarRes->FcsBtnImgRect.width) / 2;
            pCalendarRes->FcsBtnImgRect.top = pCalendarRes->RetImgRect.top + (pCalendarRes->RetImgRect.height - pCalendarRes->FcsBtnImgRect.height) / 2;
        }
        else if (AK_NULL != pCalendarRes->pLftImg[1])
        {
            pCalendarRes->FcsBtnImgRect.left = pCalendarRes->LftImgRect.left + (pCalendarRes->LftImgRect.width - pCalendarRes->FcsBtnImgRect.width) / 2;
            pCalendarRes->FcsBtnImgRect.top = pCalendarRes->LftImgRect.top + (pCalendarRes->LftImgRect.height - pCalendarRes->FcsBtnImgRect.height) / 2;
        }
        else
        {
            pCalendarRes->FcsBtnImgRect.left = pCalendarRes->RghtImgRect.left + (pCalendarRes->RghtImgRect.width - pCalendarRes->FcsBtnImgRect.width) / 2;
            pCalendarRes->FcsBtnImgRect.top = pCalendarRes->RghtImgRect.top + (pCalendarRes->RghtImgRect.height - pCalendarRes->FcsBtnImgRect.height) / 2;
        }
        
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pCalendarRes->FcsBtnImgRect.left, pCalendarRes->FcsBtnImgRect.top, \
                pCalendarRes->pFcsBtnImg, &g_Graph.TransColor, AK_FALSE);
    }

    pCalendar_Parm->refreshflag = AK_FALSE;

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlefunc_calendar(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_MMI_KEYPAD    phyKey;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    if (pCalendar_Parm->curTime.year > END_YEAR)
    {
        MsgBox_InitStr(&pCalendar_Parm->msgbox, 2, GetCustomTitle(ctWARNING), GetCustomString(csWARNING_EXCEED_MAX_YEAR), MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pCalendar_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pCalendar_Parm->msgbox);
        return 1;
    }

    /*don't delete, otherwise will die(Spr_S00000388)*/
    AK_DEBUG_OUTPUT("debug token: event = %d", event);

    switch (event)
    {
        case M_EVT_USER_KEY:
            phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
            phyKey.pressType = (T_PRESS_TYPE)pEventParm->c.Param2;

            pCalendar_Parm->refreshflag = AK_TRUE;
            Calender_UserKey_Handle(&phyKey, pEventParm);
           
        case M_EVT_TOUCH_SCREEN:
            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pEventParm->s.Param1) 
            {
                case eTOUCHSCR_UP:
                    pCalendar_Parm->res.pRetImg[1] = AK_NULL;
                    pCalendar_Parm->res.pLftImg[1] = AK_NULL;
                    pCalendar_Parm->res.pRghtImg[1] = AK_NULL;
                    pCalendar_Parm->bFcsBtn = AK_FALSE;
                    pCalendar_Parm->refreshflag = AK_TRUE;
                    break;

                case eTOUCHSCR_DOWN:
                    Calender_HitButton_Handle(&phyKey, pEventParm);
                    Calender_UserKey_Handle(&phyKey, pEventParm);
                    break;

                case eTOUCHSCR_MOVE:
                     break;

                default:
                     break;
            }
            break;

        default:
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_SYS_SET

static T_VOID Calender_UserKey_Handle(T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm)
{
    T_pSYSTIME  pCurTime = AK_NULL;
    
    if ((AK_NULL == pPhyKey) || (AK_NULL == pEventParm))
    {
        return;
    }

    pCurTime = &pCalendar_Parm->curTime;
        
    switch (pPhyKey->keyID)
    {
        case kbUP:
            if(pPhyKey->pressType == PRESS_LONG)      //长按UP,按月递减
            {
                if(pCurTime->month == 1)
                {
                    if(pCurTime->year - 1 < 1901)
                            break;
                    pCurTime->year -= 1;
                    pCurTime->month = 12;
                }
                else
                {
                    pCurTime->month -= 1;
                    monthdays = MonthDays(pCurTime->year,pCurTime->month);
                    if(pCurTime->day > (T_U8)monthdays)
                        pCurTime->day = (T_U8)monthdays;
                }
            }
            else
            {
                if(pCurTime->day <= 7)
                {
                    if(pCurTime->month == 1)
                    {
                        if(pCurTime->year - 1 < 1901)
                            break;
                        pCurTime->year -= 1;
                        pCurTime->month = 12;
                        pCurTime->day = 31 - (7 - pCurTime->day);
                    }
                    else
                    {
                        pCurTime->month -= 1;
                        monthdays = MonthDays((T_S16)(pCurTime->year+1900),(T_S16)(pCurTime->month));
                        pCurTime->day = monthdays - (7 - pCurTime->day);
                    }
                }
                else
                {
                    pCurTime->day -= 7;
                }
            }
            break;
            
        case kbDOWN:
            if(pPhyKey->pressType == PRESS_LONG)      //长按DOWN,按月递加
            {
                if(pCurTime->month == 12)
                {
                    if(pCurTime->year  + 1 > 2050)
                            break;
                    pCurTime->year += 1;
                    pCurTime->month = 1;
                }
                else
                {
                    pCurTime->month += 1;
                    monthdays = MonthDays(pCurTime->year,pCurTime->month);
                    if(pCurTime->day > (T_U8)monthdays)
                        pCurTime->day = (T_U8)monthdays;
                }
            }
            else
            {
                monthdays = MonthDays(pCurTime->year,pCurTime->month);
                if(monthdays - pCurTime->day < 7)
                {
                    if(pCurTime->month == 12)
                    {
                        if(pCurTime->year  + 1 > 2050)
                            break;
                        pCurTime->year += 1;
                        pCurTime->month = 1;
                        pCurTime->day = 7-(31 - pCurTime->day);
                    }
                    else
                    {
                        pCurTime->month += 1;
                        pCurTime->day = 7-(monthdays - pCurTime->day);
                    }
                }
                else
                {
                    pCurTime->day += 7;
                }
            }
            break;
            
        case kbLEFT:
            if(pCurTime->day == 1)
            {
                if(pCurTime->month == 1)
                {
                    if(pCurTime->year  - 1 < 1901)
                        break;
                    pCurTime->month = 12;
                    pCurTime->year -= 1;
                }
                else
                {
                    pCurTime->month -= 1;
                }
                pCurTime->day = (T_U8)MonthDays(pCurTime->year,pCurTime->month);
            }
            else
            {
                pCurTime->day -= 1;
            }
            break;
            
        case kbRIGHT:
            if(pCurTime->day == MonthDays(pCurTime->year,pCurTime->month))
            {
                if(pCurTime->month == 12)
                {
                    if(pCurTime->year  + 1 > 2050)
                        break;
                    pCurTime->month = 1;
                    pCurTime->year += 1;
                }
                else
                {
                    pCurTime->month += 1;
                }
                pCurTime->day = 1;
            }
            else
                pCurTime->day += 1;
            break;
            
        case kbCLEAR:               // Cancel
            GE_ShadeInit();
            if(PRESS_LONG == pPhyKey->pressType)   
            {
                m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
            }
            else
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
            }

            break;
            
        default:
            break;
    }
}

static T_VOID Calender_HitButton_Handle(T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm)
{
    T_POS       posX = 0;
    T_POS       posY = 0;  
    T_RECT      rest;
    T_SYSTIME   tmpTime;
    T_S16       monthDays = 0;

    pPhyKey->keyID = kbNULL;
    pPhyKey->pressType = PRESS_SHORT;

    posX = (T_POS)pEventParm->s.Param2;
    posY = (T_POS)pEventParm->s.Param3;

    //return button
    if (PointInRect(&pCalendar_Parm->res.RetImgRect, posX, posY))
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
        pCalendar_Parm->res.pRetImg[1] = (T_pCDATA)pCalendar_Parm->res.pFcsBtnImg[1];
        pCalendar_Parm->bFcsBtn = AK_TRUE;
        pCalendar_Parm->refreshflag = AK_TRUE;
        return;
    }

    //left button: prior page
    if (PointInRect(&pCalendar_Parm->res.LftImgRect, posX, posY))
    {
        pPhyKey->keyID = kbUP;
        pPhyKey->pressType = PRESS_LONG;
        pCalendar_Parm->res.pLftImg[1] = (T_pCDATA)pCalendar_Parm->res.pFcsBtnImg[1];
        pCalendar_Parm->bFcsBtn = AK_TRUE;
        pCalendar_Parm->refreshflag = AK_TRUE;
        return;
    }

    //right button: next page
    if (PointInRect(&pCalendar_Parm->res.RghtImgRect, posX, posY))
    {
        pPhyKey->keyID = kbDOWN;
        pPhyKey->pressType = PRESS_LONG;
        pCalendar_Parm->res.pRghtImg[1] = (T_pCDATA)pCalendar_Parm->res.pFcsBtnImg[1];
        pCalendar_Parm->bFcsBtn = AK_TRUE;
        pCalendar_Parm->refreshflag = AK_TRUE;
        return;
    }

    //focus day
    Calendar_GetCurFcsRect(&rest);
    if (PointInRect(&rest, posX, posY))
    {
        return;
    }

    tmpTime.year = pCalendar_Parm->curTime.year;
    tmpTime.month= pCalendar_Parm->curTime.month;

    if ((posY < rest.top) || ((posX < rest.left) && (posY < rest.top + rest.height))) 
    {
        tmpTime.day= 1;
        while (tmpTime.day < pCalendar_Parm->curTime.day)
        {
            Calendar_GetSpecDateRect(&rest, &tmpTime);
            if (PointInRect(&rest, posX, posY))
            {
                pCalendar_Parm->curTime.day = tmpTime.day;
                pCalendar_Parm->refreshflag = AK_TRUE;
                return;
            }
            tmpTime.day++;
        }
    }
    else
    {
        tmpTime.day= pCalendar_Parm->curTime.day;
        monthDays = MonthDays(tmpTime.year, tmpTime.month);

        while (tmpTime.day <= monthDays)
        {
            Calendar_GetSpecDateRect(&rest, &tmpTime);
            if (PointInRect(&rest, posX, posY))
            {
                pCalendar_Parm->curTime.day = tmpTime.day;
                pCalendar_Parm->refreshflag = AK_TRUE;
                return;
            }
            tmpTime.day++;
        }
    }
}

static T_VOID Calendar_GetRes(T_CALENDAR_RES *pRes)
{
    T_U32       imgLen = 0;
    T_pSYSTIME  pCurTime = AK_NULL;
    T_U8        i = 0;

    pCurTime = &pCalendar_Parm->curTime;

    GetLunarDate(pCurTime->year, pCurTime->month, pCurTime->day);

    if (pCurTime->month > 12 || pCurTime->month == 0)
    {
        return;
    }

    for (i = 0; i < 4; i++)
    {
        pRes->pBckImg[SEASON_SPRING + i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CALENDAR_SPRING + i, &imgLen);
        pRes->pWkImg[SEASON_SPRING + i][0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CALENDAR_SPRING_WEEK_EN + i, &imgLen);
        pRes->pWkImg[SEASON_SPRING + i][1] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CALENDAR_SPRING_WEEK + i, &imgLen);
    }

    for ( i = 0; i < 10; i++)
    {
        pRes->date.GreNumIcon[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CALENDAR_NUMBER0 + i, &imgLen);
    }

    pRes->pFcsImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CALENDAR_FOCUS, &imgLen);

    //new
    pRes->pRetImg[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CALENDAR_BUTTON_RETURN, AK_NULL);
	AKBmpGetInfo(pRes->pRetImg[0], &pRes->RetImgRect.width, &pRes->RetImgRect.height, AK_NULL);
    pRes->pRetImg[1] = AK_NULL;

    pRes->pLftImg[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CALENDAR_BUTTON_LEFT, AK_NULL);
	AKBmpGetInfo(pRes->pLftImg[0], &pRes->LftImgRect.width, &pRes->LftImgRect.height, AK_NULL);
    pRes->pLftImg[1] = AK_NULL;

    pRes->pRghtImg[0]= Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CALENDAR_BUTTON_RIGHT, AK_NULL);
	AKBmpGetInfo(pRes->pRghtImg[0], &pRes->RghtImgRect.width, &pRes->RghtImgRect.height, AK_NULL);
    pRes->pRghtImg[1] = AK_NULL;

    pRes->pFcsBtnImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CALENDAR_BUTTON_FOCUS, AK_NULL);
	AKBmpGetInfo(pRes->pFcsBtnImg, &pRes->FcsBtnImgRect.width, &pRes->FcsBtnImgRect.height, AK_NULL);
}

static T_VOID Calendar_InitResRect(T_CALENDAR_RES *pRes)
{
    T_pRECT pBkImgRect;
    T_pRECT pWkImgRect;
    T_pRECT pFcsImgRect;

    pBkImgRect = &pRes->BckImgRect;
    pWkImgRect = &pRes->WkImgRect;
    pFcsImgRect = &pRes->FcsImgRect;

    AKBmpGetInfo(pRes->pWkImg[0][0], &pWkImgRect->width, &pWkImgRect->height, AK_NULL);
    pWkImgRect->left = 0;
    pWkImgRect->top = 0;

    AKBmpGetInfo(pRes->pBckImg[0], &pBkImgRect->width, &pBkImgRect->height, AK_NULL);
    pBkImgRect->left = 0;
    pBkImgRect->top = pWkImgRect->top + pWkImgRect->height;

    AKBmpGetInfo(pRes->pFcsImg, &pFcsImgRect->width, &pFcsImgRect->height, AK_NULL);
    pFcsImgRect->left = 0;
    pFcsImgRect->top = 0;

    pRes->RetImgRect.left = (Fwl_GetLcdWidth() - pRes->RetImgRect.width) / 2;
    pRes->RetImgRect.top = Fwl_GetLcdHeight() - pRes->RetImgRect.height;

    pRes->LftImgRect.left = (pRes->FcsBtnImgRect.width - pRes->LftImgRect.width) / 2;
    pRes->LftImgRect.top = pRes->RetImgRect.top + (pRes->RetImgRect.height - pRes->LftImgRect.height) / 2;

    pRes->RghtImgRect.left = Fwl_GetLcdWidth() \
                - (pRes->FcsBtnImgRect.width - pRes->RghtImgRect.width) / 2 \
                - pRes->RghtImgRect.width;
    pRes->RghtImgRect.top = pRes->LftImgRect.top;

    pRes->FcsBtnImgRect.left = pRes->LftImgRect.left;
    pRes->FcsBtnImgRect.top = pRes->LftImgRect.top;
}

static T_VOID Calendar_GetCurFcsRect(T_pRECT pFcsRest)
{
    T_CALENDAR_RES  *pRes = AK_NULL;

    if ((AK_NULL == pCalendar_Parm) || (AK_NULL == pFcsRest))
    {
        return;
    }

    pRes = &pCalendar_Parm->res;

    pFcsRest->left  = pCalendar_Parm->fcsPosX;
    pFcsRest->top  = pCalendar_Parm->fcsPosY;
    pFcsRest->width = pRes->FcsImgRect.width;
    pFcsRest->height= pRes->FcsImgRect.height;
}

//get the specifically date rect
static T_VOID Calendar_GetSpecDateRect(T_pRECT pDateRest, T_pSYSTIME pTime)
{
    T_U8            StartRow = 0;
    T_CALENDAR_RES  *pRes = AK_NULL;
    T_S16           tmpWkDays = 0;
    
    if ((AK_NULL == pCalendar_Parm) || (AK_NULL == pDateRest) || (AK_NULL == pTime))
    {
        return;
    }

    pRes = &pCalendar_Parm->res;
    tmpWkDays = WeekDay(pTime->year, pTime->month, pTime->day);

    if (WeekDay(pTime->year, pTime->month, 1) == 0)
    {
        StartRow = (6 - tmpWkDays + pTime->day) / 7 - 1;
    }
    else
    {
        StartRow = (6 - tmpWkDays + pTime->day) / 7;
    }
   
    pDateRest->left = pCalendar_Parm->lftFrmW \
                    + tmpWkDays * pCalendar_Parm->wkWd \
                    + (pCalendar_Parm->wkWd - pRes->FcsImgRect.width) / 2;
    
    pDateRest->top = pRes->BckImgRect.top \
                    + (StartRow + 1) * pCalendar_Parm->rowH \
                    + (pCalendar_Parm->rowH - pRes->FcsImgRect.width) / 2;

    pDateRest->width = pRes->FcsImgRect.width;
    pDateRest->height= pRes->FcsImgRect.height;
}

static T_SEASON Calendar_CheckCurSeason(T_U8 month)
{
    if ((month > 1) && (month <= 4))
    {
        return SEASON_SPRING;
    }
    else if ((month > 4) && (month <= 7))
    {
        return SEASON_SUMMER;
    }
    else if ((month > 7) && (month <= 10))
    {
        return SEASON_AUTUMN;
    }
    else 
    {
        return SEASON_WINTER;
    }
}

static T_U8 Calendar_CheckCurLanguage(T_VOID)
{
    if (gs.Lang == eRES_LANG_ENGLISH)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
#endif
