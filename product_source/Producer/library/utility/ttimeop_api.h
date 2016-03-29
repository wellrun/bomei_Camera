/**
 * @file timeop_api.h
 * @brief This header file is for time process and format transfer function prototype
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 */

/** @defgroup TimeOperation Time operation interface
 *	@ingroup UTLLib
 */

#ifndef __TTIMEOP_API_H__
#define __TTIMEOP_API_H__ 

#ifndef _UNICODE
#include "timeop_api.h"
#define ConvertTimeS2C_T					ConvertTimeS2C
#define ConvertTimeC2S_T					ConvertTimeC2S
#define ConvertTime2String_T				ConvertTime2String
#define ConvertSecondsToTime_T				ConvertSecondsToTime
#define ConvertSMSTime2Standard_T			ConvertSMSTime2Standard
#define ConvertSMSTime2Standard_Reverse_T	ConvertSMSTime2Standard_Reverse
#define CompressStdDateTime_T				CompressStdDateTime
#define DecompressDate_T					DecompressDate
#define DecompressTime_T					DecompressTime
#define Time_CheckDateFormat_T				Time_CheckDateFormat
#define Time_CheckTimeFormat_T				Time_CheckTimeFormat

#else

#include "utimeop_api.h"

#define ConvertTimeS2C_T					ConvertTimeS2C_U
#define ConvertTimeC2S_T					ConvertTimeC2S_U
#define ConvertTime2String_T				ConvertTime2String_U
#define ConvertSecondsToTime_T				ConvertSecondsToTime_U
#define ConvertSMSTime2Standard_T			ConvertSMSTime2Standard_U
#define ConvertSMSTime2Standard_Reverse_T	ConvertSMSTime2Standard_Reverse_U
#define CompressStdDateTime_T				CompressStdDateTime_U
#define DecompressDate_T					DecompressDate_U
#define DecompressTime_T					DecompressTime_U
#define Time_CheckDateFormat_T				Time_CheckDateFormat_U
#define Time_CheckTimeFormat_T				Time_CheckTimeFormat_U

#endif

/**
 * @brief Time init
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25
 * @param[in] dtTime	system time
 * @param[in] year		initial year
 * @return void
 * @retval 
 */
T_VOID Time_Init(T_SYSTIME *dtTime, T_U16 year);

/**
 * @brief Add milliseconds to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] millisec	milli-seconds to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMilliSeconds(T_SYSTIME *dtTime, T_U16 millisec);

/**
 * @brief Add seconds to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] seconds	seconds to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddSeconds(T_SYSTIME *dtTime, T_U16 seconds);

/**
 * @brief Add minutes to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] minutes   minutes to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMinutes(T_SYSTIME *dtTime, T_U16 minutes);

/**
 * @brief Subtract minutes to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] minutes   minutes to be subtracted
 * @return T_VOID
 * @retval 
 */
T_VOID Time_SubMinutes(T_SYSTIME *dtTime, T_U16 minutes);

/**
 * @brief Add hours to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] hours		hours to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddHours(T_SYSTIME *dtTime, T_U16 hours);

/**
 * @brief Subtract hours to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] hours		hours to be subtracted
 * @return T_VOID
 * @retval 
 */
T_VOID Time_SubHours(T_SYSTIME *dtTime, T_U16 hours);

/**
 * @brief Add days to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] days		days to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddDays(T_SYSTIME *dtTime, T_U16 days);

T_VOID Time_SubDay(T_SYSTIME *dtTime);

/**
 * @brief Add months to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] months	months to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddMonths(T_SYSTIME *dtTime, T_U16 months);

T_VOID Time_SubMonth(T_SYSTIME *dtTime );

/**
 * @brief Add years to the current time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @param[in] years		years to be added
 * @return T_VOID
 * @retval 
 */
T_VOID Time_AddYears(T_SYSTIME *dtTime, T_U16 years);

T_VOID Time_SubYear(T_SYSTIME *dtTime, T_U16 years);

/**
 * @brief Calculate the week
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] SysTime	current system time
 * @return the week
 * @retval 
 */
T_S16	CalculateWeek(T_SYSTIME *SysTime);

/**
 * @brief Compare time
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] ast1		time to be compared
 * @param[in] ast2		time to be compared
 * @param[in] type		time type
 * @return result of comparison
 * @retval 
 */
T_S16	Time_Compare(T_SYSTIME* ast1, T_SYSTIME* ast2, T_U16 type);

/**
 * @brief Compare time for alarm type
 * 
 * @author \b GuoJinpin
 * 
 * @author 
 * @date 2005-01-20 
 * @param[in] ast1		time to be compared
 * @param[in] ast2		time to be compared
 * @param[in] type		time type for alarm
 * @return result of comparison
 * @retval 
 */
T_S16	Time_CompareAlarmType(T_SYSTIME* ast1, T_SYSTIME* ast2, T_U8 type);  // add by 

/**
 * @brief Judge system time format is legal or not
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2002-9-9
 * @param[in] time	time in format HHMMSS or HHMM
 * @return T_BOOL
 * @retval AK_TRUE	success
 * @retval AK_FALSE	failed
 */
T_BOOL Time_CheckSystemTimeFormat(T_SYSTIME *time);

/**
 * @brief Judge system date format is legal or not
 * 
 * @author \b ZouMai
 * @date 2002-9-9
 * @param[in] time	date in format YYMMDD or YYYYMMDD
 * @return T_BOOL
 * @retval AK_TRUE		success
 * @retval AK_FALSE		failed
 */
T_BOOL Time_CheckSystemDateFormat(T_SYSTIME *time);

/**
 * @brief Get the total days of this month
 * 
 * @author \b Lin_Guijie
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] dtTime	current system time
 * @return the total days of this month
 * @retval 
 */
T_S16	GetDaysByDate(T_SYSTIME *dtTime);

/**
 * @brief Get the total days to today of this year
 * 
 * @author \b Lin_Guijie
 * 
 * @author 
 * @date 2003-08-25 
 * @param [in] dtTime	current system time
 * @return the total days to today of this year
 * @retval 
 */		
T_S16	GetTotalDaysByDate(T_SYSTIME *dtTime);

/**
 * @brief Get the week number of this week of today
 * 
 * @author \b Lin_Guijie
 * 
 * @author 
 * @date 2003-08-25 
 * @param dtTime	current system time
 * @return the week number of this week of today
 * @retval 
 */	
T_S16	GetWeeksByDate(T_SYSTIME *dtTime);		

/** @} */

#endif


