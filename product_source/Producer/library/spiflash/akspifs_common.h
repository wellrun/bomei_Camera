
#ifndef _SPIFS_COMMON_H_
#define _SPIFS_COMMON_H_
#include "anyka_types.h"

#ifdef WIN32
#define akprintf _CCprintf
#endif

#define DEBUG

#ifdef DEBUG
#define AKPRINT akprintf

#else
#define AKPRINT

#endif

struct T_SYSTIME {
	T_U16 wYear;			/* 4 byte*/
	T_U16 wMonth;			/* 1-12 */
	T_U16 wDayOfWeek;		/* 0-6 */
	T_U16 wDay;				/* 1-31 */
	T_U16 wHour;			/* 0-23 */
	T_U16 wMinute;			/* 0-59 */
	T_U16 wSecond;			/* 0-59 */
	T_U16 wMilliseconds;	/* 0-999 */		
};	/* system time structure */  
//查找第一个不为0的位
#define find_first_one_bit(addr, size) \
        find_next_one_bit((addr), (size), 0)

T_VOID memory_set( T_VOID * s, T_S32 c, T_U32 count );
T_S32 memory_compare( const T_VOID * s1,const T_VOID * s2, T_U32 count );
T_U32 string_len( const T_S8 * s );
T_VOID string_copy( T_S8 * dest, const T_S8 * src );
T_VOID memory_copy( T_S8 * dest,const T_S8 * src, T_U32 count );
T_VOID* memory_move(T_VOID *dst, const T_VOID *src, T_S32 count);
T_BOOL include_str( const T_S8 * s1, const T_S8 * s2 );
T_S8 * string_scan( const T_S8 * str, T_S8 c );
T_VOID set_bit( T_S32 nr,  T_VOID *addr );
T_S32 test_bit( T_S32 nr,  T_VOID * addr );
T_VOID clear_bit( T_S32 nr, T_VOID *addr );
T_U32 find_next_one_bit( T_VOID * addr, T_U32 size, T_U32 offset );
T_U32 find_next_databit( T_VOID * addr, T_U32 size);


#endif






