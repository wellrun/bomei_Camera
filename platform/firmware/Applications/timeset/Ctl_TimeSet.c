
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: Ctl_TimeSet.c
  * @Function:  This file is a part of the state machine s_set_sysclock
  * @Author:    WangWei
  * @Date:      2008-05-04
  * @Version:   1.0
  */
  
#include "Ctl_TimeSet.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_osMalloc.h"
#include "Ctl_MsgBox.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_KeyMapping.h"
#include "Eng_Time.h"
#include "Eng_AkBmp.h"
#include "Eng_GblString.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "Ctl_MultiSet.h"
#include "Fwl_rtc.h"
#include "Lib_res_port.h"

static T_VOID TimeSet_SetDayOfFeb(T_SYSTIME *pSysTime);

/**
 * @brief   system clock setup init
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  TimeSet_Init(T_TIMESET *pTimeSet)
{
    MultiSet_Init(&pTimeSet->multiSet);
    pTimeSet->dayCursor = CURSOR_FIRST;
    pTimeSet->timeCursor = CURSOR_HOUR;

    return AK_TRUE;
}

/**
 * @brief   get resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  TimeSet_GetContent (T_TIMESET *pTimeSet)
{
    //T_U16 *pAttribText = AK_NULL;
  
    MultiSet_SetTitleText(&pTimeSet->multiSet, Res_GetStringByID(eRES_STR_CLK_SETUP), \
                            COLOR_BLACK);
    MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_TIME, Res_GetStringByID(eRES_STR_CLK_TIME),\
                            pTimeSet->timeBuf, ITEM_TYPE_EDIT);
    MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_DAY, Res_GetStringByID(eRES_STR_CLK_DAY), \
                            pTimeSet->dayBuf, ITEM_TYPE_EDIT);

    MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_DAYFORMAT, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_FORMAT), AK_NULL, ITEM_TYPE_RADIO);
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 0, Res_GetStringByID(eRES_STR_CLK_DAY_FORMAT1));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 1, Res_GetStringByID(eRES_STR_CLK_DAY_FORMAT2));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 2, Res_GetStringByID(eRES_STR_CLK_DAY_FORMAT3));
    switch(gs.DayFormat)
    {
        case DAY_FORMAT_YMD:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 0);
            break;
        case DAY_FORMAT_MDY:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 1);
            break;
        case DAY_FORMAT_DMY:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 2);
            break;
        default:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYFORMAT, 0);
            break;
    }

    MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR), AK_NULL, ITEM_TYPE_RADIO);
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 0, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR1));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 1, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 2, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR3));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 3, \
                            Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR4));
    switch (gs.DaySeparator)
    {
        case UNICODE_DOT:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 0);
            break;
        case UNICODE_COLON:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 1);
            break;
        case UNICODE_SOLIDUS:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 2);
            break;
        case UNICODE_BAR:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 3);
            break;
        default:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR, 1);
            break;                    
    }

	MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, \
                            Res_GetStringByID(eRES_STR_CLK_TIME_FORMAT), AK_NULL, ITEM_TYPE_RADIO);
	MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, 0, \
                           Res_GetStringByID(eRES_STR_CLK_TIME_FORMAT1));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, 1, \
                           Res_GetStringByID(eRES_STR_CLK_TIME_FORMAT2));
    switch(gs.TimeFormat)
    {
        case TIME_FORMAT_12:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, 0);
            break;
        case TIME_FORMAT_24:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, 1);
            break;
        default:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, 1);
            break;
    }
    
	MultiSet_AddItemWithOption(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, \
                            Res_GetStringByID(eRES_STR_CLK_TIME_SEPARATOR), AK_NULL, ITEM_TYPE_RADIO);
	MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, 0, \
                            Res_GetStringByID(eRES_STR_CLK_TIME_SEPARATOR1));
    MultiSet_AddItemOption(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, 1, \
                            Res_GetStringByID(eRES_STR_CLK_TIME_SEPARATOR2));
    switch (gs.TimeSeparator)
    {
        case UNICODE_DOT:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, 0);
            break;
        case UNICODE_COLON:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, 1);
            break;
        default:
            MultiSet_SetOptionFocus(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, 1);
            break;                    
    }

    MultiSet_CheckScrollBar(&pTimeSet->multiSet);
    
    MultiSet_SetRefresh(&pTimeSet->multiSet, MULTISET_REFRESH_ALL);
    
    return AK_TRUE;
}

/**
 * @brief   free system clock setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_VOID
 */
T_VOID  TimeSet_Free(T_TIMESET *pTimeSet)
{
    if (AK_NULL != pTimeSet)
    {
        MultiSet_Free(&pTimeSet->multiSet);
    }
}

/**
 * @brief   show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_VOID
 * @retval  
 */
T_VOID  TimeSet_Show(T_TIMESET *pTimeSet)
{
    if (AK_NULL == pTimeSet)
    {
        AK_DEBUG_OUTPUT("TimeSet_Show():pTimeSet is AK_NULL");
        return;
    }
    else
    {
        MultiSet_Show(&pTimeSet->multiSet);
        MultiSet_SetRefresh(&pTimeSet->multiSet, MULTISET_REFRESH_NONE);
    }
}

/**
 * @brief   set refresh flag
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @param   T_MULTISET_REFRESH_FLAG refreshFlag: refresh flag
 * @return  T_VOID
 */
T_VOID  TimeSet_SetRefresh(T_TIMESET *pTimeSet, T_MULTISET_REFRESH_FLAG refreshFlag)
{
    if (AK_NULL != pTimeSet)
    {
        MultiSet_SetRefresh(&pTimeSet->multiSet, refreshFlag);
    }
}

/**
 * @brief   synchronize system clock
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SyncSystemClock(T_TIMESET *pTimeSet)
{
    T_U16	strDate[30] = {0};
	T_U16	strTime[30] = {0};

    pTimeSet->sysTime = GetSysTime();
    if (pTimeSet->sysTime.year > MAX_YEAR)
    {
        pTimeSet->sysTime.year = MIN_YEAR;
        pTimeSet->sysTime.month = 1;
        pTimeSet->sysTime.day = 1;

        SetSysTime(&pTimeSet->sysTime); 
        AlarmRtcChange();
        pTimeSet->sysTime = GetSysTime();
    }
    ConvertTimeS2UcSByFormat(&pTimeSet->sysTime, strDate, strTime);

    pTimeSet->timeBuf[0] = UNICODE_END;
    Utl_UStrCpy(pTimeSet->timeBuf, strTime);

    pTimeSet->dayBuf[0] = UNICODE_END;
    Utl_UStrCpy(pTimeSet->dayBuf, strDate);

    MultiSet_SetRefresh(&pTimeSet->multiSet, MULTISET_REFRESH_ALL);

    return AK_TRUE;
}

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_MoveLeftTimeCursor(T_TIMESET *pTimeSet)
{
    if (CURSOR_HOUR == pTimeSet->timeCursor)
    {
        pTimeSet->timeCursor = CURSOR_SECOND;
    }
    else
    {
        pTimeSet->timeCursor--;
    }

	return AK_TRUE;
}

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_MoveRightTimeCursor(T_TIMESET *pTimeSet)
{
    if (CURSOR_SECOND == pTimeSet->timeCursor)
    {
        pTimeSet->timeCursor = CURSOR_HOUR;
    }
    else
    {
        pTimeSet->timeCursor++;
    }

	return AK_TRUE;
}

/**
 * @brief   incream the cursor num
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_IncreamTimeCursor(T_TIMESET *pTimeSet)
{
    switch(pTimeSet->timeCursor)
    {
        case CURSOR_HOUR:
            pTimeSet->sysTime.hour++;
            if (pTimeSet->sysTime.hour > 23)
            {
                pTimeSet->sysTime.hour = 0;
            }
            break;
            
        case CURSOR_MINUTE:
            pTimeSet->sysTime.minute++;
            if (pTimeSet->sysTime.minute > 59)
            {
                pTimeSet->sysTime.minute = 0;
            }            
            break;
            
        case CURSOR_SECOND:
            pTimeSet->sysTime.second++; 
            if (pTimeSet->sysTime.second > 59)
            {
                pTimeSet->sysTime.second = 0;
            }            
            break;
            
        default:
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   decrease the cursor num
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_DecreaseTimeCursor(T_TIMESET *pTimeSet)
{
    switch(pTimeSet->timeCursor)
    {
        case CURSOR_HOUR:
            if (0 == pTimeSet->sysTime.hour) 
            {
                pTimeSet->sysTime.hour = 23;
            }
			else
			{
				pTimeSet->sysTime.hour--;
			}
            break;
            
        case CURSOR_MINUTE:
            if (0 == pTimeSet->sysTime.minute)
            {
                pTimeSet->sysTime.minute = 59;
            }  
			else
			{
				pTimeSet->sysTime.minute--;
			}
            break;
            
        case CURSOR_SECOND:
            if (0 == pTimeSet->sysTime.second)
            {
                pTimeSet->sysTime.second = 59;
            }     
			else
			{
	            pTimeSet->sysTime.second--; 
			}
            break;
            
        default:
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   save edit time
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SaveEditTimeCursor(T_TIMESET *pTimeSet)
{
    T_SYSTIME   sysTime;

    sysTime = GetSysTime();

    sysTime.hour    = pTimeSet->sysTime.hour;
    sysTime.minute  = pTimeSet->sysTime.minute;
    sysTime.second  = pTimeSet->sysTime.second;
   
    SetSysTime(&sysTime); 
	return AK_TRUE;
}

/**
 * @brief   save edit date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SaveEditDateCursor(T_TIMESET *pTimeSet)
{
    T_SYSTIME   sysTime;

    sysTime = GetSysTime();

    sysTime.year    = pTimeSet->sysTime.year;
    sysTime.month   = pTimeSet->sysTime.month;
    sysTime.day     = pTimeSet->sysTime.day;
   
    SetSysTime(&sysTime); 
	return AK_TRUE;
}

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDate_MoveLeftCursor(T_TIMESET *pTimeSet)
{
    if (CURSOR_FIRST == pTimeSet->dayCursor)
    {
        pTimeSet->dayCursor = CURSOR_THIRD;
    }
    else
    {
        pTimeSet->dayCursor--;
    }

	return AK_TRUE;
}

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDate_MoveRightCursor(T_TIMESET *pTimeSet)
{
    if (CURSOR_THIRD == pTimeSet->dayCursor)
    {
        pTimeSet->dayCursor = CURSOR_FIRST;
    }
    else
    {
        pTimeSet->dayCursor++;
    }

	return AK_TRUE;
}


/**
 * @brief   set date of February
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_VOID
 * @info    29 days in Feb of leap year;
 * @info    28 days in Feb of normal year;
 */
static T_VOID TimeSet_SetDayOfFeb(T_SYSTIME *pSysTime)
{
    AK_ASSERT_PTR_VOID(pSysTime, "TimeSet_SetDayOfFeb(): pSysTime");

    if ((29 == pSysTime->day) && (2 == pSysTime->month) 
        && !Time_CheckIsLeapYear(pSysTime))
    {
        pSysTime->day = 28;    
    }
}


/**
 * @brief   incream cursor date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDay_IncreamCursor(T_TIMESET *pTimeSet)
{
    T_U8 month_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    switch (gs.DayFormat)
    {
        case DAY_FORMAT_MDY:        //month/day/year
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if (12 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.month++;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if ((!Time_CheckIsLeapYear(&pTimeSet->sysTime)) \
                        && (pTimeSet->sysTime.month == 2) && (pTimeSet->sysTime.day == 28))
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else if (pTimeSet->sysTime.day == month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.day++;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if (MAX_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MIN_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year++;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }        
            break;
            
        case DAY_FORMAT_DMY:        //day/month/year
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if ((!Time_CheckIsLeapYear(&pTimeSet->sysTime)) \
                        && (pTimeSet->sysTime.month == 2) && (pTimeSet->sysTime.day == 28))
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else if (pTimeSet->sysTime.day == month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.day++;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if (12 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.month++;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if (MAX_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MIN_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year++;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }
            break;
        
        default:                    //year/month/day
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if (MAX_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MIN_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year++;
                    }
                    
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if (12 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.month++;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if ((!Time_CheckIsLeapYear(&pTimeSet->sysTime)) \
                        && (pTimeSet->sysTime.month == 2) && (pTimeSet->sysTime.day == 28))
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else if (pTimeSet->sysTime.day == month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = 1;
                    }
                    else
                    {
                        pTimeSet->sysTime.day++;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   decrease cursor date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDay_DecreaseCursor(T_TIMESET *pTimeSet)
{
    T_U8 month_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    switch (gs.DayFormat)
    {
        case DAY_FORMAT_MDY:        //month/day/year
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if (1 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 12;
                    }
                    else
                    {
                        pTimeSet->sysTime.month--;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if (1 == pTimeSet->sysTime.day)
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    else
                    {
                        pTimeSet->sysTime.day--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if (MIN_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MAX_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }            break;
            
        case DAY_FORMAT_DMY:        //day/month/year
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if (1 == pTimeSet->sysTime.day)
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    else
                    {
                        pTimeSet->sysTime.day--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if (1 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 12;
                    }
                    else
                    {
                        pTimeSet->sysTime.month--;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if (MIN_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MAX_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }
            break;
        
        default:                    //year/month/day
            switch(pTimeSet->dayCursor)
            {
                case CURSOR_FIRST:
                    if (MIN_YEAR == pTimeSet->sysTime.year)
                    {
                        pTimeSet->sysTime.year = MAX_YEAR;
                    }
                    else
                    {
                        pTimeSet->sysTime.year--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_SECND:
                    if (1 == pTimeSet->sysTime.month)
                    {
                        pTimeSet->sysTime.month = 12;
                    }
                    else
                    {
                        pTimeSet->sysTime.month--;
                    }

                    if (pTimeSet->sysTime.day > month_day[pTimeSet->sysTime.month - 1])
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
                    
                case CURSOR_THIRD:
                    if (1 == pTimeSet->sysTime.day)
                    {
                        pTimeSet->sysTime.day = month_day[pTimeSet->sysTime.month - 1];
                    }
                    else
                    {
                        pTimeSet->sysTime.day--;
                    }
                    TimeSet_SetDayOfFeb(&pTimeSet->sysTime);
                    break;
            }
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   save date format
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: date format
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveDayFormat(T_U32 optionFocusId)
{
    gs.DayFormat = optionFocusId;
}

/**
 * @brief   save date separator
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: date separator
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveDaySeparator(T_U32 optionFocusId)
{
    switch (optionFocusId)
    {
        case 0:
            gs.DaySeparator = UNICODE_DOT;
            break;
        case 1:
            gs.DaySeparator = UNICODE_COLON;
            break;
        case 2:
            gs.DaySeparator = UNICODE_SOLIDUS;
            break;
        default:
            gs.DaySeparator = UNICODE_BAR;
            break;            
    }
}

/**
 * @brief   save time format
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: time format
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveTimeFormat(T_U32 optionFocusId)
{
    gs.TimeFormat = optionFocusId;
}

/**
 * @brief   save time separator
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: time separator
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveTimeSeparator(T_U32 optionFocusId)
{
    switch (optionFocusId)
    {
        case 0:
            gs.TimeSeparator = UNICODE_DOT;
            break;

        default:
            gs.TimeSeparator = UNICODE_COLON;
            break;            
    }
}
#endif

