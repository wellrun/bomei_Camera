#ifndef __ENG_CALENDAR_H__
#define __ENG_CALENDAR_H__

#include "anyka_types.h"

#define START_YEAR  1901
#define END_YEAR    2050

extern T_S16 iLunarYear, iLunarMonth, iLunarDay;

//判断iYear是不是闰年
T_BOOL IsLeapYear(T_S16 iYear);

//计算iYear,iMonth,iDay对应是星期几 1年1月1日 --- 65535年12月31日
T_S16 WeekDay(T_S16 iYear, T_S16 iMonth, T_S16 iDay);

//返回iYear年iMonth月的天数 1年1月 --- 65535年12月
T_S16 MonthDays(T_S16 iYear, T_S16 iMonth);

//返回阴历iLunarYer年阴历iLunarMonth月的天数，如果iLunarMonth为闰月，
//高字为第二个iLunarMonth月的天数，否则高字为0 
// 1901年1月---2050年12月
T_S32 LunarMonthDays(T_S16 iLunarYear, T_S16 iLunarMon);

//返回阴历iLunarYear年的总天数
// 1901年1月---2050年12月
T_S16 LunarYearDays(T_S16 iLunarYear);

//返回阴历iLunarYear年的闰月月份，如没有返回0
// 1901年1月---2050年12月
T_S16 GetLeapMonth(T_S16 iLunarYear);

//把iYear年格式化成天干记年法表示的字符串
void FormatLunarYear(T_S16  iYear, char *pBuffer);

//把iMonth格式化成中文字符串
void FormatMonth(T_S16 iMonth, T_U16 *pBuffer, T_BOOL bLunar);

//把iDay格式化成中文字符串
void FormatLunarDay(T_S16  iDay, T_U16 *pBuffer);

//计算公历两个日期间相差的天数  1年1月1日 --- 65535年12月31日
T_S32 CalcDateDiff(T_S16 iEndYear, T_S16 iEndMonth, T_S16 iEndDay,
                  T_S16 iStartYear, 
                  T_S16 iStartMonth, T_S16 iStartDay);

//计算公历iYear年iMonth月iDay日对应的阴历日期,返回对应的阴历节气 0-24
//1901年1月1日---2050年12月31日
T_S16 GetLunarDate(T_S16 iYear, T_S16 iMonth, T_S16 iDay);

void  l_CalcLunarDate(T_S32 iSpanDays);

T_S16  l_GetLunarHolDay(T_S16 iYear, T_S16 iMonth, T_S16 iDay);

#endif

