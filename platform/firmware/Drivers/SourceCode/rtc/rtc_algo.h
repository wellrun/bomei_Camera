
#ifndef __RTC_ALGO_H__
#define __RTC_ALGO_H__

#include "anyka_types.h"

void my_memcopy(const T_U8 *from, T_U8 *to, T_U32 len);

void mem_clr(T_U8 *mem, T_U32 len);

T_U8 hex2bcd(unsigned char hex);

T_U8 bcd2hex(T_U8 bcd);

#endif