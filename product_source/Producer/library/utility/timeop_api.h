/**
 * @file timeop_api.h
 * @brief This header file is for time process and format transfer function prototype
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 */

/** @defgroup TimeOperation Time operation interface
 *	@ingroup UTLLib
 */

#ifndef __TIMEOP_API_H__
#define __TIMEOP_API_H__ 

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
 * @brief Convert system time structure to string.
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] SysTime		SYSTEMTIME structure
 * @param[out] strDate		standard format date "YY/MM/DD" for return
 * @param[out] strTime		standard format time "HH:MM:SS" for return
 * @return T_VOID
 * @retval 
 */
T_VOID	ConvertTimeS2C(T_SYSTIME *SysTime, T_pSTR strDate, T_pSTR strTime);

/**
 * @brief Convert string date & time to system time structure.
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] strDate		standard format date "YYYY/MM/DD" or "YY/MM/DD"
 * @param[in] strTime		standard format time "HH:MM:SS" or "YY/MM"
 * @param[out] SysTime		SYSTEMTIME structure for return
 * @return success or not
 * @retval 
 */
T_BOOL	ConvertTimeC2S(T_pSTR strDate, T_pSTR strTime, T_SYSTIME *SysTime);

/**
 * @brief Convert system time structure to string.
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] SysTime		SYSTEMTIME structure
 * @param[out] string		"YYMMDDHHMMSS" format
 * @return T_VOID
 * @retval 
 */
T_VOID	ConvertTime2String( T_SYSTIME *SysTime, T_pSTR string );

/**
 * @brief Convert seconds to time format.
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] seconds		second quantity
 * @param[in] format		time format.  0: SS, 1: MM:SS, 2: HH:MM:SS
 * @param[out] time			time string for return 
 * @return time stirng for return 
 * @retval 
 */
T_S8	*ConvertSecondsToTime(T_S16 seconds, T_S8 format, T_pSTR time);

/** 
 * @brief Convert SMS format time to standard format time,but do not reverse
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] SMSTime		SMS format time: YYMMDDHHMMSS
 * @param[out] Standard		Standard format time: YY/MM/DD,HH:MM:SS
 * @return standard format time
 * @retval 
 */
T_S8	*ConvertSMSTime2Standard(T_pSTR SMSTime, T_pSTR Standard);

/** 
 * @brief Convert SMS format time to standard format time, but reverse .
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] SMSTime		SMS format time: YYMMDDHHMMSS
 * @param[out] Standard		Standard format time: YY/MM/DD,HH:MM:SS
 * @return standard format time
 * @retval 
 */
T_pSTR	ConvertSMSTime2Standard_Reverse(T_pSTR SMSTime, T_pSTR Standard);

/**
 * @brief Compress standard mode date or time.
 * 
 * @author \b ZouMai
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] stdMode		standard mode  : YY/MM/DD or HH:MM:SS
 * @param[out] compMode		compressed mode: YYMMDD or HHMMSS
 * @return date or time string of compressed mode
 * @retval 
 */
T_S8	*CompressStdDateTime(T_pSTR stdMode, T_pSTR compMode);

/**
 * @brief Decompress date string from YYMMDD to YY/MM/DD
 * 
 * @author \b Baoli.Miao
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] strComp		compressed mode: YYMMDD 
 * @param[out] strStd		standard mode  : YY/MM/DD
 * @return date string of standard mode
 * @retval 
 */
T_pSTR	DecompressDate(T_pSTR strComp, T_pSTR strStd );

/**
 * @brief Decompress time string from HHMMSS to HH:MM:SS
 * 
 * @author \b Baoli.Miao
 * 
 * @author 
 * @date 2001-06-25 
 * @param[in] strComp		compressed mode: HHMMSS 
 * @param[out] strStd		standard mode  : HH:MM:SS
 * @return time string of standard mode
 * @retval 
 */
T_pSTR	DecompressTime(T_pSTR strComp, T_pSTR strStd );

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
 * @brief Judge date format is legal or not
 * 
 * @author \b ZouMai
 * @date 2002-9-9
 * @param[in] date	date in format YYMMDD or YYYYMMDD
 * @return T_BOOL
 * @retval AK_TRUE		success
 * @retval AK_FALSE		failed
 */
T_BOOL	Time_CheckDateFormat(T_pSTR date);

/**
 * @brief Judge time format is legal or not
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
T_BOOL	Time_CheckTimeFormat(T_pSTR time);

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


