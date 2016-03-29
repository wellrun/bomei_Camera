/**
 * @file timeop_api.h
 * @brief This header file is for time process and format transfer function prototype
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 */

/** @defgroup TimeOperation Time operation interface
 *	@ingroup UTLLib
 */

#ifndef __UTIMEOP_API_H__
#define __UTIMEOP_API_H__ 


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
T_VOID ConvertTimeS2C_U(T_SYSTIME *SysTime, T_pWSTR strDate, T_pWSTR strTime);

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
T_BOOL ConvertTimeC2S_U(T_pWSTR strDate, T_pWSTR strTime, T_SYSTIME *SysTime);

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
T_VOID ConvertTime2String_U( T_SYSTIME *SysTime, T_pWSTR string );

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
T_pWSTR ConvertSecondsToTime_U(T_S16 seconds, T_S8 format, T_pWSTR time);

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
T_pWSTR ConvertSMSTime2Standard_U(T_pWSTR SMSTime, T_pWSTR Standard);

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
T_pWSTR ConvertSMSTime2Standard_Reverse_U(T_pWSTR SMSTime, T_pWSTR Standard);

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
T_pWSTR CompressStdDateTime_U(T_pWSTR stdMode, T_pWSTR compMode);

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
T_pWSTR DecompressDate_U(T_pWSTR strComp, T_pWSTR strStd );

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
T_pWSTR DecompressTime_U(T_pWSTR strComp, T_pWSTR strStd );

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
T_BOOL Time_CheckDateFormat_U(T_pWSTR date);

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
T_BOOL Time_CheckTimeFormat_U(T_pWSTR time);

/** @} */

#endif


