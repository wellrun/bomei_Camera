/**
 * @file Eng_Time.c
 * @brief ANYKA software
 * Process time format
 * @author ZouMai
 * @date 2001-06-25
 * @version 1.0
 */

#include "Eng_Time.h"
#include "Eng_GblString.h"
#include "Eng_String.h"
#include "hal_timer.h"
#include "Eng_Alarm.h"
#include "Fwl_rtc.h"
#include "eng_debug.h"
#include "eng_dataconvert.h"


#ifdef OS_WIN32
#include <time.h>
#endif

static T_S16    GetDayNumFrom2000(T_SYSTIME *dtTime);

T_VOID Time_SubYear(T_SYSTIME *dtTime, T_U16 years);
T_VOID Time_SubMonth(T_SYSTIME *dtTime);
T_VOID Time_SubDay(T_SYSTIME *dtTime);

/**
 * @brief Time init
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25
 * @param T_SYSTIME *dtTime
 * @return T_VOID
 * @retval 
 */
T_VOID Time_Init(T_SYSTIME *dtTime, T_U16 year)
{
    AK_ASSERT_PTR_VOID(dtTime, "Time_Init(): dtTime");
    
    dtTime->year = year;
    dtTime->month = 1;
    dtTime->day = 1;
    dtTime->week = 1;
    dtTime->hour = 0;
    dtTime->minute = 0;
    dtTime->second = 0;

    return;
}


/**
 * @brief Add milliseconds to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 millisec
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMilliSeconds(T_SYSTIME *dtTime, T_U16 millisec)
{
    T_U16 curMilliSec;
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddSeconds(): dtTime");
    
    curMilliSec = dtTime->milli_second + millisec;
    dtTime->milli_second = (T_U16)(curMilliSec % 1000);
    if (curMilliSec >= 1000)
        Time_AddSeconds(dtTime, (T_S16)(curMilliSec / 1000));

    return;
}

/**
 * @brief Add seconds to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 seconds
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddSeconds(T_SYSTIME *dtTime, T_U16 seconds)
{
    T_U16 curSecond;
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddSeconds(): dtTime");
    
    curSecond = dtTime->second + seconds;
    dtTime->second = (T_U8)(curSecond % 60);
    if (curSecond >= 60)
        Time_AddMinutes(dtTime, (T_S16)(curSecond / 60));

    return;
}

/**
 * @brief Add minutes to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 minutes
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMinutes(T_SYSTIME *dtTime, T_U16 minutes)
{
    T_U16 curMinute;
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddMinutes(): dtTime");
    
    curMinute = dtTime->minute + minutes;
    dtTime->minute = (T_U8)(curMinute % 60);
    if (curMinute >= 60)
        Time_AddHours(dtTime, (T_S16)(curMinute / 60));

    return;
}

T_VOID Time_SubMinutes(T_SYSTIME *dtTime, T_U16 minutes)
{
    T_U16 Minute, Hours, curMinute;

    Hours = minutes / 60;
    Minute = minutes % 60;

    if( Minute > dtTime->minute )
    {
        Hours++;
        curMinute = dtTime->minute + 60 - Minute;
    }
    else
    {
        curMinute = dtTime->minute - Minute;
    }

    dtTime->minute = (T_U8)curMinute;
    Time_SubHours( dtTime, Hours );

    return;
}

/**
 * @brief Add hours to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 hours
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddHours(T_SYSTIME *dtTime, T_U16 hours)
{
    T_U16 curHour;
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddHours(): dtTime");
    
    curHour = dtTime->hour + hours;
    dtTime->hour = (T_U8)(curHour % 24);
    if (curHour >= 24)
        Time_AddDays(dtTime, (T_S16)(curHour / 24));

    return;
}

T_VOID Time_SubHours(T_SYSTIME *dtTime, T_U16 hours)
{
    T_S16 i;
    
    if( dtTime->hour >= hours )
    {
        dtTime->hour -= hours;
    }
    else
    {
        Time_SubDay( dtTime );
        i = hours - dtTime->hour;
        dtTime->hour = 24 - i;
    }

    return;
}
/**
 * @brief Add days to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 days
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddDays(T_SYSTIME *dtTime, T_U16 days)
{
    T_S16 daynum;
    T_S16 daynumsave;
    T_S16 i;
    T_S16 year;
    T_S16 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    AK_ASSERT_PTR_VOID(dtTime, "Time_AddDays(): dtTime");
    
    daynum = GetDayNumFrom2000(dtTime) + days;
    daynumsave = daynum;
    if (daynum <= 0)
        return;

    for (i = 2000; ; i++)
    {
        if (!(i%4 == 0 && i%100 != 0 || i%400 == 0))
             daynum -= 365;
        else
             daynum -= 366;

        if (daynum < 0)
        {
            daynum = daynumsave;
            break;
        }
        else
            daynumsave = daynum;
    }
    dtTime->year = (T_U16)i;
    year = dtTime->year;

    for (i = 1; i <= 12; i++)
    {
        if (i == 2 && (year%4 == 0 && year%100 != 0 || year%400 == 0))
             daynum -= 29;
        else
             daynum -= month_day[i-1];

        if (daynum < 0)
        {
            daynum = daynumsave;
            break;
        }
        else
            daynumsave = daynum;
    }
    dtTime->month = (T_U8)i;
    dtTime->day = (T_U8)(daynum + 1);
    
    return;
}


T_VOID Time_SubDay(T_SYSTIME *dtTime)
{
    T_S16 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if( dtTime->day == 1 )
    {
        Time_SubMonth( dtTime );
        if(dtTime->month == 2 && (dtTime->year%4 == 0 && dtTime->year%100 != 0 || dtTime->year%400 == 0))
        {
            dtTime->day = 29;
        }
        else
        {
            dtTime->day = (T_U8)month_day[dtTime->month-1];
        }
    }
    else
    {
        dtTime->day--;
    }
    return;
}

/**
 * @brief Add the months to the current time
 *  
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 months
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMonths(T_SYSTIME *dtTime, T_U16 months)
{
    T_U16 curMonth;
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddMonths(): dtTime");
    
    curMonth = dtTime->month + months;
    dtTime->month = (T_U8)((curMonth + 11) % 12 + 1);
    if (curMonth > 12)
        Time_AddYears(dtTime, (T_S16)((curMonth + 11) / 12));

    return;
}

T_VOID Time_SubMonth(T_SYSTIME *dtTime )
{
    if( dtTime->month == 1 )
    {
        dtTime->month = 13;
        Time_SubYear( dtTime, 1 );
    }
    dtTime->month--;
    return;
}
/**
 * @brief Add yesrs to the current time
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @param  T_U16 years
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddYears(T_SYSTIME *dtTime, T_U16 years)
{
    AK_ASSERT_PTR_VOID(dtTime, "Time_AddYears(): dtTime");
    dtTime->year = (T_U16)((dtTime->year + years) % 10000);

    return;
}

T_VOID Time_SubYear(T_SYSTIME *dtTime, T_U16 years)
{
    //AK_ASSERT_PTR_VOID(dtTime, "Time_AddYears(): dtTime");
    if( dtTime->year == 0 )
    {
        dtTime->year = 2000;
    }
    
    dtTime->year-=years;

    return;
}
/**
 * @brief Get the day number from 2000 years, if 2000-1-1, return 0 
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @return T_S16
 * @retval 
 */
static T_S16 GetDayNumFrom2000(T_SYSTIME *dtTime)
{
    T_S16 i;
    T_S16 year;
    T_S16 month;
    T_S16 day;
    T_S16 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_S16 daynum = -1;

    AK_ASSERT_PTR(dtTime, "GetDayNumFrom2000(): dtTime", 0);
    
    year = dtTime->year;
    month = dtTime->month;
    day = dtTime->day;
    if (year < 100)
        year = 2000 + year;
    else if (year >= 2000)
        year = 2000 + year % 2000;

    if (year < 2000 || month <= 0 || day <= 0)
        return 0;

    for (i = 2000; i < year; i++)
    {
        if (!(i%4 == 0 && i%100 != 0 || i%400 == 0))
             daynum += 365;
        else
             daynum += 366;
    }
    for (i = 1; i < month; i++)
    {
        if (i == 2 && (year%4 == 0 && year%100 != 0 || year%400 == 0))
             daynum += 29;
        else
             daynum += month_day[i-1];
    }
    return (daynum + day);
}

/**
 * @brief Calculate the week
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *SysTime
 * @return T_S16
 * @retval 
 */
T_S16 CalculateWeek(T_SYSTIME *SysTime)
{
#if 0
    T_S16       i;
    T_S16       dayFree[12]={0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

    if ((SysTime->year % 4 == 0 && SysTime->year % 100 != 0 || SysTime->year % 400 == 0))
    {
        for (i = 2; i < 12; i++)
            dayFree[i] = (dayFree[i] + 1) % 7;
    }

    SysTime->week = ((T_S16)((SysTime->year - 1) * 1.2425 + dayFree[SysTime->month-1] + SysTime->day + 0.000001)) % 7;
#endif

#if 0   
    if( SysTime == AK_NULL )
        return 0;
    SysTime->week = (T_U16)((GetDayNumFrom2000(SysTime) + 6) % 7);

    return SysTime->week;
#endif

#if 1   

    T_S32 y, s;
    T_S16 d;
    /* the table of days for clear year, if this year is leap, then add 1 from March(include) */
    T_S16 dayOfMonth[13] = {0, 31, 59, 90, 120, 151,181, 212, 243, 273, 304, 334, 365};

    if( SysTime == AK_NULL )
        return 0;
    y = SysTime->year;
    d = dayOfMonth[SysTime->month - 1] + SysTime->day;
    if (SysTime->month >= 3){    
        if ((y%4==0 && y%100 != 0) || y%400==0) //is leap
            d++;
    }
    s = y - 1 + ((y-1)/4) - ((y-1)/100)+ ((y-1)/400) + d;
    d = s % 7;

    SysTime->week = (T_U8)d;

    return (d);

#endif

}

/**
 * @brief Convert system time structure to string.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *SysTime : SYSTEMTIME structure
 * @param  T_pSTR strDate: standard format date "YYYY/MM/DD" for return
 * @param  T_pSTR strTime: standard format time "HH:MM:SS" for return
 * @return T_VOID
 * @retval 
 */
T_VOID ConvertTimeS2C(T_SYSTIME *SysTime, T_pSTR strDate, T_pSTR strTime)
{
    T_S8 strTemp[30];

    AK_FUNCTION_ENTER("ConvertTimeS2C");

    AK_ASSERT_PTR_VOID(SysTime, "ConvertTimeS2C(): SysTime");
    AK_ASSERT_PTR_VOID(strDate, "ConvertTimeS2C(): strDate");
    AK_ASSERT_PTR_VOID(strTime, "ConvertTimeS2C(): strTime");

    strDate[0] = '\0';
    Utl_Itoa(SysTime->year, strTemp, 10);       /* year */
    if (SysTime->year < 10)
    {
        Utl_StrCat(strDate, "0");
        Utl_StrCat(strDate, strTemp);
    }
    else
    {
        Utl_StrCpy(strDate, strTemp + Utl_StrLen(strTemp) - 4);
    }

    Utl_StrCat(strDate, "/");
    Utl_Itoa(SysTime->month,strTemp,10);        /* month */
    if(SysTime->month < 10)
        Utl_StrCat(strDate, "0");
    Utl_StrCat(strDate, strTemp);

    Utl_StrCat(strDate, "/");
    Utl_Itoa(SysTime->day,strTemp,10);          /* day */
    if(SysTime->day < 10)
        Utl_StrCat(strDate, "0");
    Utl_StrCat(strDate, strTemp);

    strTime[0] = '\0';
    Utl_Itoa((SysTime->hour),strTemp,10);   /* hour */
    if((SysTime->hour) < 10)
        Utl_StrCat(strTime, "0");
    Utl_StrCat(strTime, strTemp);

    Utl_StrCat(strTime, ":");
    Utl_Itoa(SysTime->minute,strTemp,10);       /* minute */
    if(SysTime->minute < 10)
        Utl_StrCat(strTime, "0");
    Utl_StrCat(strTime, strTemp);

    Utl_StrCat(strTime, ":");
    Utl_Itoa(SysTime->second,strTemp,10);       /* minute */
    if(SysTime->second < 10)
        Utl_StrCat(strTime, "0");
    Utl_StrCat(strTime, strTemp);

    AK_FUNCTION_LEAVE("ConvertTimeS2C");
    return;
}/* end ConvertTimeS2C(SYSTIME SysTime,T_pSTR strDate, T_pSTR strTime) */

T_VOID ConvertTimeS2UcSByFormat(T_SYSTIME *SysTime, T_pWSTR strDate, T_pWSTR strTime)
{
    T_U8    temp_hour;
	T_S8    strTemp[30];
    T_U16   tempBuf[30];
    T_U16   tmpStr[8] = {0};
    T_U16   *pUStr = AK_NULL;

    AK_FUNCTION_ENTER("ConvertTimeS2C");

    AK_ASSERT_PTR_VOID(SysTime, "ConvertTimeS2C(): SysTime");
    AK_ASSERT_PTR_VOID(strDate, "ConvertTimeS2C(): strDate");
    AK_ASSERT_PTR_VOID(strTime, "ConvertTimeS2C(): strTime");

    /*year/month/day*/
    strDate[0] = '\0';
    pUStr = strDate;
    switch (gs.DayFormat)
    {
        case DAY_FORMAT_MDY://月日年
        	sprintf(strTemp, "%02d", SysTime->month);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 2;
            
            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;

        	sprintf(strTemp, "%02d", SysTime->day);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 2;
            
            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;

        	sprintf(strTemp, "%04d", SysTime->year);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            break;
            
        case DAY_FORMAT_DMY://日月年
        	sprintf(strTemp, "%02d", SysTime->day);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 2;

            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;

        	sprintf(strTemp, "%02d", SysTime->month);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 2;

            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;

        	sprintf(strTemp, "%04d", SysTime->year);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            break;
            
        default://年月日
        	sprintf(strTemp, "%04d", SysTime->year);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 4;
    
            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;
            
        	sprintf(strTemp, "%02d", SysTime->month);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);
            pUStr = pUStr + 2;

            Utl_UStrCpy(pUStr, &gs.DaySeparator);
            pUStr = pUStr + 1;

        	sprintf(strTemp, "%02d", SysTime->day);
            Eng_StrMbcs2Ucs(strTemp, tempBuf);
            Utl_UStrCpy(pUStr, tempBuf);

            break;
    }


    /*hour/minute/second*/
    strTime[0] = '\0';
    pUStr = strTime;

    temp_hour = SysTime->hour;
    if (TIME_FORMAT_12 == gs.TimeFormat)
    {
		if ((SysTime->hour >= 12) && (SysTime->hour <= 23))
		{
		    if (SysTime->hour != 12)
	        {
                temp_hour = SysTime->hour - 12;
            }
            Eng_StrMbcs2Ucs(" PM", tmpStr);
		}
        else
        {
            if (SysTime->hour == 0)
            {
                temp_hour = 12;
            }
            Eng_StrMbcs2Ucs(" AM", tmpStr);
        }
    	sprintf(strTemp, "%02d", temp_hour);
    }
    else
    {
    	sprintf(strTemp, "%02d", temp_hour);
    }
    Eng_StrMbcs2Ucs(strTemp, tempBuf);
    Utl_UStrCpy(pUStr, tempBuf);
    pUStr = pUStr + 2;

    Utl_UStrCpy(pUStr, &gs.TimeSeparator);
    pUStr = pUStr + 1;    

	sprintf(strTemp, "%02d", SysTime->minute);
    Eng_StrMbcs2Ucs(strTemp, tempBuf);
    Utl_UStrCpy(pUStr, tempBuf);
    pUStr = pUStr + 2;

    Utl_UStrCpy(pUStr, &gs.TimeSeparator);
    pUStr = pUStr + 1;

	sprintf(strTemp, "%02d", SysTime->second);
    Eng_StrMbcs2Ucs(strTemp, tempBuf);
    Utl_UStrCpy(pUStr, tempBuf);
    pUStr = pUStr + 2;
    
    if (TIME_FORMAT_12 == gs.TimeFormat)
    {
        Utl_UStrCpy(pUStr, tmpStr);
    }
    
    AK_FUNCTION_LEAVE("ConvertTimeS2UnS");
    return;
}

T_VOID ConvertTime2String( T_SYSTIME *SysTime, T_pSTR string )
{
    T_S8 strTemp[10];

    string[0] = '\0';
    Utl_Itoa(SysTime->year, strTemp, 10);       /* year */
    if (SysTime->year < 10)
    {
        Utl_StrCat(string, "0");
        Utl_StrCat(string, strTemp);
    }
    else
    {
        Utl_StrCpy(string, strTemp + Utl_StrLen(strTemp) - 2);
    }

    Utl_Itoa(SysTime->month,strTemp,10);        /* month */
    if(SysTime->month < 10)
        Utl_StrCat(string, "0");
    Utl_StrCat(string, strTemp);

    Utl_Itoa(SysTime->day,strTemp,10);          /* day */
    if(SysTime->day < 10)
        Utl_StrCat(string, "0");
    Utl_StrCat(string, strTemp);

    Utl_Itoa((SysTime->hour)%24,strTemp,10);    /* hour */
    if((SysTime->hour)%24 < 10)
        Utl_StrCat(string, "0");
    Utl_StrCat(string, strTemp);

    Utl_Itoa(SysTime->minute,strTemp,10);       /* minute */
    if(SysTime->minute < 10)
        Utl_StrCat(string, "0");
    Utl_StrCat(string, strTemp);

    Utl_Itoa(SysTime->second,strTemp,10);       /* second */
    if(SysTime->second < 10)
        Utl_StrCat(string, "0");
    Utl_StrCat(string, strTemp);

    return;
}
/**
 * @brief Convert string date & time to system time sturcture.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_pSTR strDate: standard format date "YYYY/MM/DD" or "YY/MM/DD"
 * @param  T_pSTR strTime: standard format time "HH:MM:SS" or "YY/MM"
 * @param  T_SYSTIME *SysTime: SYSTEMTIME structure for return
 * @return T_BOOL
 * @retval 
 */
T_BOOL ConvertTimeC2S(T_pSTR strDate, T_pSTR strTime, T_SYSTIME *SysTime)
{
    T_S8    strTemp[10];
    T_S16   curLoc = 0;

    AK_FUNCTION_ENTER("ConvertTimeC2C");

    AK_ASSERT_PTR(SysTime, "ConvertTimeC2S(): SysTime", AK_FALSE);
    AK_ASSERT_PTR(strDate, "ConvertTimeC2S(): strDate", AK_FALSE);
    AK_ASSERT_PTR(strTime, "ConvertTimeC2S(): strTime", AK_FALSE);

    if (Utl_StrLen(strDate) == 10)
    {
        Utl_StrMid(strTemp, strDate, 0, 3);
        SysTime->year = (T_U16)Utl_Atoi(strTemp);
        curLoc = 5;
    }
    else if (Utl_StrLen(strDate) == 8)
    {
        Utl_StrMid(strTemp, strDate, 0, 1);
        SysTime->year = (T_U16)Utl_Atoi(strTemp);   /* Y2K */
        curLoc = 3;
    }
    else
        return AK_FALSE;

    if (Utl_StrLen(strTime) == 8)
    {
        Utl_StrMid(strTemp, strTime, 6, 7);
        SysTime->second = (T_U8)Utl_Atoi(strTemp);
    }
    else if (Utl_StrLen(strDate) == 5)
        SysTime->second = 0;
    else
        return AK_FALSE;

    Utl_StrMid(strTemp, strDate, curLoc, (T_S16)(curLoc+1));
    SysTime->month = (T_U8)Utl_Atoi(strTemp);

    Utl_StrMid(strTemp, strDate, (T_S16)(curLoc+3), (T_S16)(curLoc+4));
    SysTime->day = (T_U8)Utl_Atoi(strTemp);

    curLoc = 0;

    Utl_StrMid(strTemp, strTime, curLoc, (T_S16)(curLoc+1));
    SysTime->hour = (T_U8)Utl_Atoi(strTemp);
    curLoc += 3;

    Utl_StrMid(strTemp, strTime, curLoc, (T_S16)(curLoc+1));
    SysTime->minute = (T_U8)Utl_Atoi(strTemp);
    curLoc += 3;

    SysTime->milli_second = 0;

    AK_FUNCTION_LEAVE("ConvertTimeC2C");
    return AK_TRUE;
}/* end ConvertTimeC2C(T_pSTR strDate, T_pSTR strTime,SYSTEMTIME SysTime) */

/**
 * @convert system time to seconds counted from 1980-01-01 00:00:00
 * @author YiRuoxiang
 * @date 2006-02-17
 * @param T_SYSTIME SysTime: system time structure
 * @return T_U32: seconds counted from 1980-01-01 00:00:00
 */
T_U32 ConvertSysTimeToSeconds(T_SYSTIME *SysTime)
{
    T_U16 i;
    T_U16 year;
    T_U8 month;
    T_U8 day;
    T_U8 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U32 daynum = 0;

    if ((SysTime->year < 1980) || (SysTime->year > 2099) || (SysTime->month < 1) || (SysTime->month > 12) || \
       (SysTime->day < 1) || (SysTime->day > 31) || (SysTime->hour > 23) || (SysTime->minute > 59) || (SysTime->second > 59))
    {
        AK_DEBUG_OUTPUT("it is a wrong systime format\n");
        return 0;
    }
 
    year  = SysTime->year;
    month = SysTime->month;
    day   = SysTime->day - 1;

    for (i = 1980; i < year; i++)
    {
        if (!(i % 4 == 0 && i % 100 != 0 || i % 400 == 0))
             daynum += 365;
        else
             daynum += 366;
    }
    for (i = 1; i < month; i++)
    {
        if (i == 2 && (year % 4 == 0 && year % 100 != 0 || year % 400 == 0))
             daynum += 29;
        else
             daynum += month_day[i-1];
    }
    return ((daynum + day) * 86400 + SysTime->hour * 3600 + SysTime->minute * 60 + SysTime->second);
}

/**
 * @convert seconds counted from 1980-01-01 00:00:00 to system time
 * @author YiRuoxiang
 * @date 2006-02-16
 * @param  the seconds want to added, which are counted from 1980-01-01 00:00:00
 * @the seconds can't be more than 4102444799 (2099-12-31 23:59:59)
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return T_U8
 * @retval if system time is more than 2099-12-31 23:59:59, return AK_FALSE
 *          else return AK_TRUE;
 */
T_U8 ConvertSecondsToSysTime(T_U32 seconds, T_SYSTIME *SysTime)
{
    T_U32 second_intv, minute_intv, hour_intv, dayofw_intv, day_intv;
    T_U32 year_intv, day_total, year_leap;
    T_U8 month_std_day[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U8 month_leap_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U8 i;

    SysTime->year      = 1980;  //修改时间起点，要修改最大秒数和闰年数
    SysTime->month     = 1;
    SysTime->week      = 1; // 1980.1.1 is Tuesday
    SysTime->day       = 1;
    SysTime->hour      = 0;
    SysTime->minute    = 0;
    SysTime->second    = 0;

    if (seconds > ((T_U32)37869 * 100000 + 11999))  //最大秒数为3786911999，如修改时间起点，则该值也要修改 
    {
        AK_DEBUG_OUTPUT("the date is out of 2099-12-31 23:59:59\n");
        return AK_FALSE;
    }

    second_intv = seconds % 60;
    minute_intv = (seconds % 3600) / 60;
    hour_intv   = (seconds % 86400) / 3600;
    dayofw_intv = (seconds % 604800) / 86400;
    day_total   = seconds / 86400;  //day_total is still exact!!
    year_intv   = day_total / 365;  //难点是year_intv的计算
    year_leap   = (year_intv + 3) / 4;  //算的是肯定会经过的闰年数，如修改时间起点，如1970年改为1980年，括号中加的数为该年离即将来临的闰年的差减1，如1970后的为1972，为2-1=1；1980后的为1984，为4-1=3
    day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;
    if (day_intv > 366)
    {
        year_intv -= 1;
        year_leap -= 1;
        SysTime->year += (T_U16)year_intv;
        if ((SysTime->year % 4) == 0)
        {
            day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;
        }
        else
        {
            day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365 - 1;
        }
    }
    else
    {
        SysTime->year += (T_U16)year_intv;
    }
    if ((SysTime->second + second_intv) < 60)
    {
        SysTime->second += (T_U8)second_intv;
    }
    else
    {
        SysTime->second = SysTime->second + (T_U8)second_intv - 60;
    }

    if ((SysTime->minute + minute_intv) < 60)
    {
        SysTime->minute += (T_U8)minute_intv;
    }
    else
    {
        SysTime->minute += (T_U8)minute_intv - 60;
    }

    if ((SysTime->hour + hour_intv) < 24)
    {
        SysTime->hour += (T_U8)hour_intv;
    }
    else
    {
        SysTime->hour += (T_U8)hour_intv - 24;
    }
    
    if ((SysTime->week + dayofw_intv) < 7)
    {
        SysTime->week += (T_U8)dayofw_intv;
    }
    else
    {
        SysTime->week += (T_U8)dayofw_intv - 7;
    }
    
    if ((SysTime->year % 4) == 0)
    {
        for (i = 0; i <= 11; i++)
        {
            if (day_intv >= month_leap_day[i])
            {
                day_intv -= month_leap_day[i];
            }
            else
            {
                SysTime->month += i;
                SysTime->day   += (T_U8)day_intv;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i <= 11; i++)
        {
            if (day_intv >= month_std_day[i])
            {
                day_intv -= month_std_day[i];
            }
            else
            {
                SysTime->month += i;
                SysTime->day   += (T_U8)day_intv;
                break;
            }
        }
    }

    return AK_TRUE;
}

/**
 * @brief Convert seconds to time format.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_S16 seconds: second quantity
 * @param  T_S8 format: time for return
 * @param  T_pSTR time: time format.  0: SS, 1: MM:SS, 2: HH:MM:SS
 * @return T_S8
 * @retval 
 */
T_pSTR ConvertSecondsToTimeStr(T_U16 seconds, T_S8 format, T_pSTR time)
{
    T_S8 strTemp[10];

    AK_ASSERT_PTR(time, "ConvertSecondsToTimeStr(): time", AK_NULL);
    
    time[0] = 0;
    if (format == 1)        /* MM:SS */
    {
        Utl_Itoa((seconds % 3600) / 60, strTemp, 10);
        if ((seconds % 3600) / 60 < 10)
            Utl_StrCat(time, "0");
        Utl_StrCat(time, strTemp);
        Utl_StrCat(time, ":");

        Utl_Itoa(seconds % 60, strTemp, 10);
        if (seconds % 60 < 10)
            Utl_StrCat(time, "0");
        Utl_StrCat(time, strTemp);
    }
    else if (format == 2)   /* HH:MM:SS */
    {
        Utl_Itoa(seconds / 3600, strTemp, 10);
        if (seconds /3600 < 10)
            Utl_StrCat(time, "0");
        Utl_StrCat(time, strTemp);
        Utl_StrCat(time, ":");

        Utl_Itoa((seconds % 3600) / 60, strTemp, 10);
        if ((seconds % 3600) / 60 < 10)
            Utl_StrCat(time, "0");
        Utl_StrCat(time, strTemp);
        Utl_StrCat(time, ":");

        Utl_Itoa(seconds % 60, strTemp, 10);
        if (seconds % 60 < 10)
            Utl_StrCat(time, "0");
        Utl_StrCat(time, strTemp);
    }
    else                    /* SS */
        Utl_Itoa(seconds, time, 10);
    return time;
}

/** 
 * @brief Convert SMS format time to standard format time,but do not reverse
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_pSTR SMSTime: SMS format time: YYMMDDHHMMSS
 * @param  T_pSTR Standard: Standard format time: YY/MM/DD,HH:MM:SS
 * @return T_S8
 * @retval 
 */
T_pSTR ConvertSMSTime2Standard(T_pSTR SMSTime, T_pSTR Standard)
{
    T_S8    strTemp[20];

    AK_ASSERT_PTR(SMSTime, "ConvertSMSTime2Standard(): SMSTime", AK_NULL);
    AK_ASSERT_PTR(Standard, "ConvertSMSTime2Standard(): Standard", AK_NULL);
    
    Utl_StrCpy(strTemp, SMSTime);

    Standard[ 0] = strTemp[0];
    Standard[ 1] = strTemp[1];
    Standard[ 2] = '/';
    Standard[ 3] = strTemp[2];
    Standard[ 4] = strTemp[3];
    Standard[ 5] = '/';
    Standard[ 6] = strTemp[4];
    Standard[ 7] = strTemp[5];
    Standard[ 8] = ' ';
    Standard[ 9] = ' ';
    Standard[10] = strTemp[6];
    Standard[11] = strTemp[7];
    Standard[12] = ':';
    Standard[13] = strTemp[8];
    Standard[14] = strTemp[9];
    Standard[15] = ':';
    Standard[16] = strTemp[10];
    Standard[17] = strTemp[11];
    Standard[18] = 0;
    return Standard;
}/* end ConvertSMSTime2Standard(T_pSTR SMSTime, T_pSTR Standard) */


/**
 * @brief Decompress date string from YYMMDD to YY/MM/DD
 * @author Baoli.Miao
 */
T_pSTR DecompressDate(T_pSTR strComp, T_pSTR strStd )
{
    T_S16 i;
    T_pSTR p = AK_NULL;

    AK_ASSERT_PTR(strComp, "DecompressDate(): strComp", AK_NULL);
    AK_ASSERT_PTR(strStd, "DecompressDate(): strStd", AK_NULL);
    
    p = strStd;

    for( i=0; strComp[i] != 0 && i < 6; i++ )
    {
        if( i>0 && i%2 == 0 )
        {
            *p++ = '/';
        }
        *p++ = strComp[i];
    }
    *p = 0;

    return strStd;
}

/**
 * @brief Decompress time string from HHMMSS to HH:MM:SS
 * @author Baoli.Miao
 */
T_pSTR DecompressTime(T_pSTR strComp, T_pSTR strStd )
{
    T_S16 i;
    T_pSTR p = AK_NULL;

    AK_ASSERT_PTR(strComp, "DecompressTime(): strComp", AK_NULL);
    AK_ASSERT_PTR(strStd, "DecompressTime(): strStd", AK_NULL);
    
    p = strStd;

    for( i=0; strComp[i] != 0 && i < 6; i++ )
    {
        if( i>0 && i%2 == 0 )
        {
            *p++ = ':';
        }
        *p++ = strComp[i];
    }
    *p = 0;

    return strStd;
}


/**
 * @brief Judge date format is legal or ont
 * 
 * @author ZouMai
 * @date 2002-9-9
 * @param T_pSTR time: time in format YYMMDD or YYYYMMDD
 * @return T_BOOL
 * @retval AK_TRUE: success
 * @retval AK_FALSE: unsuccess
 */
T_BOOL Time_CheckDateFormat(T_pSTR date)
{
    T_S16 len;
    T_STR_TIME  strTemp;
    T_S16       year, month, day;
    T_S8        month_day[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    AK_FUNCTION_ENTER("Time_CheckDateFormat");

    AK_ASSERT_PTR(date, "Time_CheckDateFormat(): date", AK_FALSE);
    
    len = Utl_StrLen(date);
    if ((len != 6) && (len != 8))
        return AK_FALSE;

    if (len == 6)
    {
        year = Utl_Atoi(Utl_StrMid(strTemp, date, 0, 1)) + 2000;
        month = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, date, 2, 3));
        day = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, date, 4, 5));
    }
    else
    {
        year = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, date, 0, 3));
        month = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, date, 4, 5));
        day = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, date, 6, 7));
    }

    if (year%4==0 && year%100 !=0 || year%400==0)
         month_day[1]=29;

    if (month>12 || month < 1)
         return AK_FALSE;

    if (day > month_day[month-1])
         return AK_FALSE;

    return AK_TRUE;
}

T_BOOL Time_CheckSystemDateFormat(T_SYSTIME *time)
{
    T_S16       year, month, day;
    T_S8        month_day[12]={31,28,31,30,31,30,31,31,30,31,30,31};

    AK_FUNCTION_ENTER("Time_CheckDateFormat");
    
    year = time->year;
    month = time->month;
    day = time->day;

    if (year%4==0 && year%100 !=0 || year%400==0)
         month_day[1]=29;

    if (month>12 || month < 1)
         return AK_FALSE;

    if (day > month_day[month-1] || day <= 0)
         return AK_FALSE;

    return AK_TRUE;
}

/**
 * @brief Judge time format is legal or ont
 * 
 * @author @b ZouMai
 * 
 * @author ZouMai
 * @date 2002-9-9
 * @param T_pSTR time: time in format HHMMSS or HHMM
 * @return T_BOOL
 * @retval AK_TRUE: success
 * @retval AK_FALSE: unsuccess
 */
T_BOOL Time_CheckTimeFormat(T_pSTR time)
{
    T_S16 len;
    T_STR_TIME  strTemp;
    T_S16       hour, minute, second;

    AK_FUNCTION_ENTER("Time_CheckTimeFormat");

    AK_ASSERT_PTR(time, "Time_CheckTimeFormat(): time", AK_FALSE);
    
    len = Utl_StrLen(time);
    if ((len != 4) && (len != 6))
        return AK_FALSE;

    hour = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, time, 0, 1));
    minute = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, time, 2, 3));
    if (len == 4)
        second = 0;
    else
        second = (T_S16)Utl_Atoi(Utl_StrMid(strTemp, time, 4, 5));

    if (hour >= 24)
        return AK_FALSE;

    if (minute >= 60)
        return AK_FALSE;

    if (second >= 60)
        return AK_FALSE;

    AK_FUNCTION_LEAVE("Time_CheckTimeFormat");

    return AK_TRUE;
}

T_BOOL Time_CheckSystemTimeFormat(T_SYSTIME *time)
{
    T_S16       hour, minute, second;
    
    hour = time->hour;
    minute = time->minute;
    second = time->second;

    if (hour >= 24)
        return AK_FALSE;

    if (minute >= 60)
        return AK_FALSE;

    if (second >= 60)
        return AK_FALSE;

    AK_FUNCTION_LEAVE("Time_CheckTimeFormat");

    return AK_TRUE;
}


/**
 * @brief   Convert time format by current language
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-10-10
 * @param const T_RES_LANGUAGE lang
 * @param  T_pSTR sourTime  : source format time: "HH:MM:SS"
 * @param  T_pSTR destTime : converted format time
 *                         if gs.Lang == lgENGLISH, converted format: "HH:MM:SS"
 *                         if gs.Lang == lgSMPCHN, converted format: "HH时MM分SS秒"
 * @return T_S8
 * @retval 
 */
T_pSTR ConvertTimeByLang(const T_RES_LANGUAGE lang, T_pSTR sourTime, T_pSTR destTime)
{
    //T_S8 strTemp[13] = "  时  分  秒";

    AK_ASSERT_PTR(sourTime, "ConvertTimeByLang(): sourTime", AK_NULL);
    AK_ASSERT_PTR(destTime, "ConvertTimeByLang(): destTime", AK_NULL);

    //if (lang == lgENGLISH)
    {
        Utl_StrCpy(destTime, sourTime);
        return destTime;
    }
    /*strTemp[0] = sourTime[0];
    strTemp[1] = sourTime[1];
    strTemp[4] = sourTime[3];
    strTemp[5] = sourTime[4];
    strTemp[8] = sourTime[6];
    strTemp[9] = sourTime[7];
    Utl_StrCpy(destTime, strTemp);
    return destTime;*/
}/* end ConvertTimeByLang(const T_RES_LANGUAGE lang, T_pSTR sourTime, T_pSTR destTime, const T_S8 gs.Lang) */


/**
 * @brief Get week according to appointed date and current language
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2001-10-10
 * @param const T_RES_LANGUAGE lang
 * @param  const T_S16 week: appointed date
 * @param  T_pSTR strWeek: the returned week
 * @return T_S8
 * @retval 
 */
T_pSTR ConvertWeekByLang(const T_RES_LANGUAGE lang, const T_S16 week, T_pSTR strWeek)
{
    return strWeek;
} /* end ConvertWeekByLang(const T_RES_LANGUAGE lang, T_pSTR strDate, T_pSTR strWeek, const T_S8 gs.Lang) */

/**
 * @brief Get the total days of this month
 * 
 * @author @b Lin_Guijie
 * 
 * @author 
 * @date 2001-06-25 
 * @param T_SYSTIME *dtTime
 * @return T_S16
 * @retval 
 */
T_S16 GetDaysByDate(T_SYSTIME *dtTime)
{
    T_S16 i;
    T_S16 days;
    T_S16 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    days = month_day[dtTime->month-1];
    i = dtTime->year;
    if (dtTime->month == 2)  
    {
        if (i%4 == 0 && i%100 != 0 || i%400 == 0)
            days++;
    }
    return (days);
}

/**
 * @brief Get the total days to today of this year
 * 
 * @author @b Lin_Guijie
 * 
 * @author 
 * @date 2003-08-25 
 * @param T_SYSTIME *dtTime
 * @return T_S16
 * @retval 
 */
T_S16 GetTotalDaysByDate(T_SYSTIME *dtTime)
{
    T_S32 y;
    T_S16 d;
    /* the table of days for clear year, if this year is leap, then add 1 from March(include) */
    T_S16 dayOfMonth[13] = {0, 31, 59, 90, 120, 151,181, 212, 243, 273, 304, 334, 365};

    if( dtTime == AK_NULL )
        return 0;
    y = dtTime->year;
    d = dayOfMonth[dtTime->month - 1] + dtTime->day;
    if (dtTime->month >= 3){    
        if ((y%4==0 && y%100 != 0) || y%400==0) //is leap
            d++;
    }

    return (d);
}

/**
 * @brief Get the week number of this week of today
 * 
 * @author @b Lin_Guijie
 * 
 * @author 
 * @date 2003-08-25 
 * @param T_SYSTIME *dtTime
 * @return T_S16
 * @retval 
 */
T_S16 GetWeeksByDate(T_SYSTIME *dtTime)
{
    T_S32 y;
    T_S16 d, week;
    T_SYSTIME firstDay;
    /* the table of days for clear year, if this year is leap, then add 1 from March(include) */
    T_S16 dayOfMonth[13] = {0, 31, 59, 90, 120, 151,181, 212, 243, 273, 304, 334, 365};

    if( dtTime == AK_NULL )
        return 0;
    y = dtTime->year;
    d = dayOfMonth[dtTime->month - 1] + dtTime->day;
    if (dtTime->month >= 3){    
        if ((y%4==0 && y%100 != 0) || y%400==0) //is leap
            d++;
    }


    firstDay = *dtTime;
    firstDay.day = 1;
    firstDay.month = 1;
    firstDay.week = (T_U8)CalculateWeek(&firstDay);

    //sunday as the first day of week
    if ((d-1) <= (6-firstDay.week))
        week = 1;
    else
        week = ((d-1) - (6-firstDay.week) - 1)/7 + 2;

    return (week);
}

T_SYSTIME GetSysTime(T_VOID)
{
    T_SYSTIME ret;
    T_U32 tickcount;

    tickcount = Fwl_RTCGetCount();
    if (ConvertSecondsToSysTime(tickcount, &ret) == AK_FALSE)
    {
#ifdef OS_ANYKA 
        Fwl_RTCSetCount(0);  //reset system time to zero if out of 2099-12-31 23:59:59
#endif
    }
/*
#ifdef OS_WIN32
    {
        time_t ltime;
        struct tm *today;

        time( &ltime );
        today = localtime( &ltime );
        ret.day = today->tm_mday;
        ret.hour = today->tm_hour;
        ret.milli_second = 0;
        ret.minute = today->tm_min;
        ret.month = today->tm_mon + 1;
        ret.second = today->tm_sec;
        ret.week = today->tm_wday;
        ret.year = today->tm_year;
    }
#endif
*/
    gs.SysTimeYear = ret.year;

    return ret;
}

T_U32 GetSysTimeSeconds(void)
{
	T_SYSTIME       *pCurSysTime,CurSysTime;

    CurSysTime = GetSysTime();
    pCurSysTime = &CurSysTime;
    return ConvertSysTimeToSeconds(pCurSysTime);
}

T_VOID SetSysTime(T_SYSTIME *systime)
{
    T_U32 seconds;

    seconds = ConvertSysTimeToSeconds(systime);
#ifdef OS_ANYKA 
    Fwl_RTCSetCount(seconds);
#endif
}
 
/**
 * get clock time as system time format
 * @author YiRuoxiang
 * @date 2006-03-29
 * @param T_eCLOCK clk: clock as T_eCLOCK list
 * @such as alarm clock, power on clock
 * @return T_SYSTIME
 * @retval system time structure
 */
T_SYSTIME GetAlarmTime(T_VOID)
{
    T_SYSTIME ret;
#ifdef OS_ANYKA
    T_U32 tickcount;

    //tickcount = gs.AlarmTM;    
    	tickcount = GetAlarmDataMinValid(AK_FALSE);

    if (tickcount == 0)
        tickcount = Fwl_RTCGetCount();

    ConvertSecondsToSysTime(tickcount, &ret);
#endif
#ifdef OS_WIN32
    {
        time_t ltime;
        struct tm *today;

        time( &ltime );
        today = localtime( &ltime );
        ret.day = today->tm_mday;
        ret.hour = today->tm_hour;
        ret.milli_second = 0;
        ret.minute = today->tm_min;
        ret.month = today->tm_mon + 1;
        ret.second = today->tm_sec;
        ret.week = today->tm_wday;
        ret.year = today->tm_year;
    }
#endif
    return ret;
}

/**
 * set system time format to alarm register of RTC
 * @author YiRuoxiang
 * @date 2006-03-29
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return T_VOID
 * @retval
 */
T_VOID SetAlarmTime(T_SYSTIME *systime)
{
    T_U32 seconds;

    seconds = ConvertSysTimeToSeconds(systime);
#ifdef OS_ANYKA 
    Fwl_SetAlarmRtcCount(seconds);
#endif
}

/**
 * @brief convert seconds to data(year-month-day,Hour:Minute:second)
 * @author @b WuShanwei
 * @date 2008-12-14
 * @param const T_eLANGUAGE lang
 * @param  T_U32 seconds
 * @return T_SYSTIME
 * @retval 
 */
T_U32 convert_date_to_second(T_SYSTIME date)
{
    T_U32 current_time = 0;
    T_U16  year = 0;
    T_U16  month = 0;
    T_U16  day = 0;
    T_U16  hour = 0;
    T_U32  minute = 0;
    T_U32  second = 0;

    T_U16 month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U32 days = 0;
    T_U32 hours = 0;
    T_U32 minutes = 0;
    T_U16 i = 0;
    
    year = date.year;
    month = date.month;
    day = date.day;
    hour = date.hour;
    minute = date.minute;
    second = date.second;

    if (year < 100)
        year += INITIALYEAR;                        //default
    days = ((year - 1970) / 4) * (365 * 3 + 366);   //each 4 years have the same days 2000 / 400 = 5,so ignore it
    if (((year - 1970) % 4) == 1)
        days += 365;
    if (((year - 1970) % 4) == 2)
        days += 365 * 2;
    if (((year - 1970) % 4) == 3)
        days += 365 + 365 + 366;

    if ((month < 1) || (month > 12))
        month = 1;
    for (i = 1; i < month; i++)
        days += month_days[i - 1];

    if ((day < 1) || (day > 31))
        day = 1;
    days += (day - 1);

    if (((year % 4) == 0) && (month > 2))
        days += 1;                             //maybe the second month has 28 days

    if (hour > 24)
        hour = 0;
    hours = days * 24;
    hours += hour;

    if (minute > 60)
        minute = 0;
    minutes = hours * 60;
    minutes += minute;

    if (second > 60)
        second = 0;
    current_time = minutes * 60;
    current_time += second;
    
    return current_time;
}

/**
 * @brief convert seconds to data(year-month-day,Hour:Minute:second)
 * @author @b WuShanwei
 * @date 2008-12-14
 * @param const T_eLANGUAGE lang
 * @param  T_U32 seconds
 * @return T_SYSTIME
 * @retval 
 */
T_SYSTIME convert_second_to_date(T_U32 seconds)
{
    T_SYSTIME date;

    T_U16 year = 0;
    T_U8 month = 0;
    T_U8 day = 0;
    T_U8 hour = 0;
    T_U8 minute = 0;
    T_U8 second = 0;
	T_U32 secDiv,minDiv,hourDiv,yearYunDiv;
	T_U16 yearYunBase,dayYun,dayDiv,monthSum,monthSumLast;
	T_BOOL yearYunFlag;

    T_U16 month_days_1[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 month_days_2[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    Fwl_Print(C3, M_ENGINE, "convert_second_to_date : seconds = %d",seconds);

    second = (T_U8)(seconds % 60);
	secDiv = seconds / 60;

    minute = (T_U8)(secDiv % 60);
	minDiv = secDiv / 60;

    hour = (T_U8)(minDiv % 24);
	hourDiv = minDiv / 24;

	//1970,1971,1972(yun),1973, ,1970+(1*4+0),
	yearYunDiv = hourDiv/((365*3 + 366));
	yearYunBase = (T_U16)(yearYunDiv*4 + 1970);

	dayYun = (T_U16)(hourDiv % (365*3 + 366));
	
	if(dayYun < 365)
	{//1970
		year = yearYunBase + 0;
		dayDiv = dayYun;
		yearYunFlag = AK_FALSE;

	}
	else if(dayYun < 365*2)
	{//1971
		year = yearYunBase + 1;
		dayDiv = dayYun-365;
		yearYunFlag = AK_FALSE;
	}
	else if(dayYun < 365*2+366)
	{//1972
		year = yearYunBase + 2;
		dayDiv = dayYun-365*2;
		yearYunFlag = AK_TRUE;
	}
	else
	{//1973
		year = yearYunBase + 3;
		dayDiv = dayYun-365*2-366;
		yearYunFlag = AK_FALSE;
	}

	monthSum = 0;
	monthSumLast = 0;
	for(month=0; month<12; month++)
	{
		monthSumLast = monthSum;
		if(yearYunFlag)
		{
			monthSum = monthSum + month_days_2[month];
		}
		else
		{
			monthSum = monthSum + month_days_1[month];
		}

		if(dayDiv < monthSum)
		{
			month += 1;
			day = (T_U8)(dayDiv-monthSumLast+1);
			break;
		}
		else if(dayDiv == monthSum)
		{
			month += 2;
			day = 1;
			break;
		}

	}

    date.year = year;
    date.month = month;
    date.day = day;
    date.hour = hour;
    date.minute = minute;
    date.second = second;
    date.week = 0;
    date.milli_second = 0;
	Fwl_Print(C3, M_ENGINE, "convert_second_to_date:%d-%d-%d,%d:%d:%d",year,month,day,hour,minute,second);
    return date;
}


/**
 * @brief   check if is a leap year.
 * @author  
 * @date    2008-5-22
 * @param   T_SYSTIME *pSysTime
 * @return  T_BOOL
 * @retval  AK_TRUE  leap year
 * @retval  AK_FALSE not leap year
 */
T_BOOL Time_CheckIsLeapYear(T_SYSTIME *pSysTime)
{
    if (((pSysTime->year % 4 == 0) && (pSysTime->year % 100 != 0)) 
            || (pSysTime->year % 400 == 0))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_VOID Date2Str(T_U32 datetime, T_S8 *string)
{
    T_U16 date,time,tmp;

    date = (T_U16)(datetime >> 16);
    time = (T_U16)(datetime & 0xFFFF);
    //year
    tmp = (date>>9)&0x7f; //get year
    tmp += 1980;
    tmp = tmp%100;
    string[0] = (tmp/10)+'0';
    string[1] = (tmp%10)+'0';
    string[2] = '/';
    //month
    tmp = (date>>5)&0x0f;
    tmp = tmp%13;
    if(tmp==0)
    tmp = 1;
    string[3] = (tmp/10)+'0';
    string[4] = (tmp%10)+'0';
    string[5] = '/';
    //days
    tmp = (date)&0x1f;
    tmp = tmp%32;
    if(tmp==0)
    tmp = 1;
    string[6] = (tmp/10)+'0';
    string[7] = (tmp%10)+'0';
    string[8] = ' ';
    //hour
    tmp = (time>>11)&0x1f;
    tmp = tmp%24;
    string[9] = (tmp/10)+'0';
    string[10] = (tmp%10)+'0';
    string[11] = ':';

    //miniute
    tmp = (time>>5)&0x3f;
    tmp = tmp%60;
    string[12] = (tmp/10)+'0';
    string[13] = (tmp%10)+'0';
    string[14] = ':';

    //hour
    tmp = (time)&0x1f;
    tmp = tmp%30;
    tmp = tmp*2;
    string[15] = (tmp/10)+'0';
    string[16] = (tmp%10)+'0';
    string[17] = 0;
}

