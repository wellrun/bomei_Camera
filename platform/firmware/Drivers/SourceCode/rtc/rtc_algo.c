
#include "anyka_types.h"
#include "drv_in_callback.h"


void my_memcopy(const T_U8 *from, T_U8 *to, T_U32 len){
	T_U32 i;

	if ( (!from) || (!to) )
	{
		if (AK_NULL != gb_drv_cb_fun.akprintf)
		{
			gb_drv_cb_fun.akprintf(C2, "RTC", "NULL pointer input.\n");
		}
		return;
	}

	for (i = 0; i < len; i++){
		*to++ = *from++;
	}
}

void mem_clr(T_U8 *mem, T_U32 len){
	int i;

	if (!mem)
		return;

	for (i = 0; i < len; i++) *mem++ = 0;
}

/* 
** Note: please see s35390a data sheet to understand bcd data format
*/
static const T_U8 b2h_table[8] = {80,40,20,10,8,4,2,1};
static const T_U8 h2b_table[10] = 
	{0x00,0x08,0x04,0x0c,0x02,0x0a,0x06,0x0e,0x01,0x09};
	
T_U8 hex2bcd(T_U8 hex){
	T_U8 bcd_hi,bcd_lo;

	bcd_lo = hex % 10;
	bcd_hi = hex / 10;
	return ( (h2b_table[bcd_lo] << 4) | h2b_table[bcd_hi]);
}

T_U8 bcd2hex(T_U8 bcd){
	unsigned char hex;
	int i;

	hex = 0;
	for (i = 0; i < 8; i++){
		if (bcd & ( 1 << i) ){
			hex += b2h_table[i];
		}
	}
	return hex;
}