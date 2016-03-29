/**
 * @FILENAME: sys_delay.c
 * @BRIEF sys_delay driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"

#ifdef AKOS
#include "akos_api.h"
#endif

/**
 * @BRIEF minisecond delay
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_U32 minisecond: minisecond delay number
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID mini_delay(T_U32 minisecond)
{
    T_U32 i=0;
    T_U32 j=0;
    T_U32 k=0;
    T_U32 clocks;

#ifdef AKOS
    if (minisecond >= 10)
    {
        //akprintf(C3, M_DRVSYS, "mini_delay:%d, use sleep\n", minisecond);
        AK_Sleep(minisecond/5); //each tick 5ms
        return;
    }
#endif

    clocks = get_cpu_freq() / 5000 * minisecond;
    for ( i=0; i<clocks; i++ )
    {
        j++;
    }
    k = j;  //prevent optimiszed
}

/**
 * @BRIEF microsecond delay
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM T_U32 us: microsecond delay number
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID us_delay(T_U32 us)
{
    T_U32 i=0;
    T_U32 j=0;
    T_U32 k=0;
    T_U32 clocks;

    clocks = get_cpu_freq() / 1000000 * us / 5;
    for( i=0; i<clocks; i++ )
    {
        j++;
    }
    k = j;  //prevent optimiszed
}

