#ifndef __RTC_H__
#define __RTC_H__

#include "anyka_types.h"
#include "arch_rtc.h"

//typedef T_VOID (*T_fRTC_CALLBACK)(T_VOID);/* Define RTC callback function type */

//
T_U32   external_rtc_get_alarmcount(T_VOID);


T_U32 external_rtc_get_RTCcount(T_VOID);

T_VOID external_rtc_set_RTCcount(T_U32 rtc_value);

T_VOID  external_rtc_set_alarmcount(T_U32 rtc_wakeup_value);

T_VOID 	external_rtc_init(T_VOID);

T_VOID 	external_rtc_set_callback(T_fRTC_CALLBACK cb);

T_BOOL  external_rtc_clear_alarm(T_VOID);

#endif