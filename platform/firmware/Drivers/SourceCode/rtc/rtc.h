
#ifndef __RTC_H__
#define __RTC_H__

#include "drv_api.h"

typedef struct{
    T_U8   year;        /* 0~99, actually yesr is 2000 + 0~99 */
    T_U8    month;      /* 1-12 */
    T_U8    day;        /* 1-31 */
    T_U8    week;   	/* 0-6,  0: monday, 6: sunday*/
    T_U8    hour;		/* 0-23 for or 0~11. if status1 reg bit6 equal 0,
    						bit6 is am/pm flag: 0 means am, 1 means pm */
    T_U8    minute;     /* 0-59 */
    T_U8    second;     /* 0-59 */
}RTC_TIME;

/* others rtc address add here. */
/* 
** default 24 hours format, and no power on loop.
** return value: AK_TRUE means success, else error.
*/
int rtc_reset(T_U32 pin_scl, T_U32 pin_sda);

//int rtc_read_time(RTC_TIME *pt_rtc_time);
int rtc_read_systime(T_SYSTIME *pt_sys_time);

//int rtc_write_time(RTC_TIME *pt_rtc_time);
int rtc_write_systime(T_SYSTIME *pt_sys_time);

/*
** next some alarm functions.
*/
/*
int rtc_open_alarm(T_U8 alarm_num);

int rtc_close_alarm(T_U8 alarm_num);
*/

/*
** Note:
** For rtc_set_alarm() rtc_get_alarm() rtc_clear_alarm()
** valid argumence value for alarm_num only 0 and 1.
*/

/* 
** Reference rtc_clear_alarm().
** returan value: 0 means success, else error.
*/
int rtc_set_alarm(T_U8 alarm_num, T_SYSTIME * sys_time);

/*
** Note: should be call rtc_set_alarm() before call rtc_get_alarm(),
** 		and don't call rtc_get_alarm() after rtc_clear_alarm()
** returan value: 0 means success, else error.
*/
int rtc_get_alarm(T_U8 alarm_num, T_SYSTIME * sys_time);

/* 
** Note: when alarm come to, should clear the old alarm used rtc_clear_alarm(),
** and set a new alarm by rtc_set_alarm().
** returan value: 0 means success, else error.
*/
int rtc_clear_alarm(T_U8 alarm_num);

#endif