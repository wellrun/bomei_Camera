/**
 * @file Eng_Time.h
 * @brief This header file is for time process and format transfer function prototype
 * 
 */

#ifndef __ENG_TIME_H__
#define __ENG_TIME_H__ 

#include "akdefine.h"
//#include "Gbl_Global.h"
#include "gbl_resource.h"
#include "anyka_types.h"

#define	INITIALYEAR			2000

T_VOID Time_AddMilliSeconds(T_SYSTIME *dtTime, T_U16 millisec);
T_VOID Time_AddSeconds(T_SYSTIME *dtTime, T_U16 seconds);
T_VOID Time_AddMinutes(T_SYSTIME *dtTime, T_U16 minutes);
T_VOID Time_SubMinutes(T_SYSTIME *dtTime, T_U16 minutes);
T_VOID Time_AddHours(T_SYSTIME *dtTime, T_U16 hours);
T_VOID Time_SubHours(T_SYSTIME *dtTime, T_U16 hours);
T_VOID Time_AddDays(T_SYSTIME *dtTime, T_U16 days);
T_VOID Time_AddMonths(T_SYSTIME *dtTime, T_U16 months);
T_VOID Time_AddYears(T_SYSTIME *dtTime, T_U16 years);
T_S16   CalculateWeek(T_SYSTIME *SysTime);
T_VOID  ConvertTimeS2C(T_SYSTIME *SysTime, T_pSTR strDate, T_pSTR strTime);
T_VOID ConvertTimeS2UcSByFormat(T_SYSTIME *SysTime, T_pWSTR strDate, T_pWSTR strTime);
T_VOID  ConvertTime2String( T_SYSTIME *SysTime, T_pSTR string );
T_BOOL  ConvertTimeC2S(T_pSTR strDate, T_pSTR strTime, T_SYSTIME *SysTime);
T_S8    *ConvertSecondsToTime1(T_S16 seconds, T_S8 format, T_pSTR time);


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
T_VOID Time_Init(T_SYSTIME *dtTime, T_U16 year);


/**
 * @convert system time to seconds counted from 1980-01-01 00:00:00
 * @author YiRuoxiang
 * @date 2006-02-17
 * @param T_SYSTIME SysTime: system time structure
 * @return T_U32: seconds counted from 1980-01-01 00:00:00
 */
T_U32 ConvertSysTimeToSeconds(T_SYSTIME *SysTime);

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
T_U8 ConvertSecondsToSysTime(T_U32 seconds, T_SYSTIME *SysTime);

T_pSTR ConvertSecondsToTimeStr(T_U16 seconds, T_S8 format, T_pSTR time);

T_VOID ConvertTimeS2C(T_SYSTIME *SysTime, T_pSTR strDate, T_pSTR strTime);

T_pSTR  ConvertSMSTime2Standard_Reverse(T_pSTR SMSTime, T_pSTR Standard);

T_pSTR  DecompressDate(T_pSTR strComp, T_pSTR strStd );
T_pSTR  DecompressTime(T_pSTR strComp, T_pSTR strStd );
T_BOOL  Time_CheckDateFormat(T_pSTR date);
T_BOOL  Time_CheckTimeFormat(T_pSTR time);
T_BOOL Time_CheckSystemTimeFormat(T_SYSTIME *time);
T_BOOL Time_CheckSystemDateFormat(T_SYSTIME *time);

T_pSTR  ConvertTimeByLang(const T_RES_LANGUAGE lang, T_pSTR sourTime, T_pSTR destTime);
T_pSTR  ConvertWeekByLang(const T_RES_LANGUAGE lang,  const T_S16 week, T_pSTR strWeek);

T_S16   GetDaysByDate(T_SYSTIME *dtTime);       
T_S16   GetTotalDaysByDate(T_SYSTIME *dtTime);  
T_S16   GetWeeksByDate(T_SYSTIME *dtTime);  
T_SYSTIME GetSysTime(T_VOID); 
T_VOID SetSysTime(T_SYSTIME *systime);

/**
 * get clock time as system time format
 * @author YiRuoxiang
 * @date 2006-03-29
 * @param T_eCLOCK clk: clock as T_eCLOCK list
 * @such as alarm clock, power on clock
 * @return T_SYSTIME
 * @retval system time structure
 */
T_SYSTIME GetAlarmTime(T_VOID);

/**
 * set system time format to alarm register of RTC
 * @author YiRuoxiang
 * @date 2006-03-29
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return T_VOID
 * @retval
 */
T_VOID SetAlarmTime(T_SYSTIME *systime);

/**
 * @brief convert seconds to data(year-month-day,Hour:Minute:second)
 * @author @b WuShanwei
 * @date 2008-12-14
 * @param const T_eLANGUAGE lang
 * @param  T_U32 seconds
 * @return T_SYSTIME
 * @retval 
 */
T_U32 convert_date_to_second(T_SYSTIME date);

/**
 * @brief convert seconds to data(year-month-day,Hour:Minute:second)
 * @author @b WuShanwei
 * @date 2008-12-14
 * @param const T_eLANGUAGE lang
 * @param  T_U32 seconds
 * @return T_SYSTIME
 * @retval 
 */
T_SYSTIME convert_second_to_date(T_U32 seconds);


T_BOOL Time_CheckIsLeapYear(T_SYSTIME *pSysTime);

T_U32 GetSysTimeSeconds(void);

T_VOID Date2Str(T_U32 datetime, T_S8 *string);


#endif

