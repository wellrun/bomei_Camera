/**
 * @FILENAME ts.h
 * @BRIEF    touch screen driver
 * Copyright @ 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR   LianGenhui
 * VERSION   1.0
 * @REF 
 */    

#ifndef __TS_H__
#define __TS_H__

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "anyka_types.h"
#include "interrupt.h"

/*@{*/

//initial touch scheen interrupt
T_VOID ts_interrupt_init(T_U32 gpio, T_U8 level, T_INTR_HANDLER callback, T_U32 ts_filter);

//interrupt enable
T_VOID ts_interrupt_enable(T_VOID);

//check pendown now
T_U8 ts_check_pendown_now(T_VOID);

//power off TS
T_VOID ts_power_off(T_VOID);

//power on ts
T_VOID ts_power_on(T_VOID);



/*@}*/

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus
#endif // #ifndef __HTIMER_H__

