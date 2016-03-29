/**@file anyka_types.h
 * @brief Define the various common data types.
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author Justin.Zhao
 * @date 2010-05-26
 * @version 2.0
 */


#ifndef _ANYKA_TYPES_H_
#define _ANYKA_TYPES_H_

/* preliminary type definition for global area */
typedef	unsigned char           T_U8;		/* unsigned 8 bit integer */
typedef char					T_CHR;		/* char */
typedef	unsigned short			T_U16;		/* unsigned 16 bit integer */
typedef	unsigned long			T_U32;		/* unsigned 32 bit integer */
#ifdef OS_ANYKA
typedef	unsigned long long		T_U64;		/* unsigned 64 bit integer */
#else
typedef unsigned __int64 		T_U64;		/* unsigned 64 bit integer */
#endif
typedef	signed char			 	T_S8;		/* signed 8 bit integer */
typedef	signed short			T_S16;		/* signed 16 bit integer */
typedef	signed long 			T_S32;		/* signed 32 bit integer */
#ifdef OS_ANYKA
typedef	signed long long 		T_S64;		/* signed 64 bit integer */
#else
typedef	__int64 				T_S64;		/* signed 64 bit integer */
#endif
typedef void					T_VOID;		/* void */
typedef volatile unsigned int 	V_UINT32;

#ifdef OS_ANYKA
#define _VOLATILE               volatile
#else
#define _VOLATILE           
#endif

#define    T_U8_MAX				((T_U8)0xff)                    // maximum T_U8 value
#define    T_U16_MAX			((T_U16)0xffff)                    // maximum T_U16 value
#define    T_U32_MAX			((T_U32)0xffffffff)                // maximum T_U32 value
#define    T_U64_MAX			((T_U32)0xffffffffffffffff)                // maximum T_U64 value
#define    T_S8_MIN				((T_S8)(-127-1))                // minimum T_S8 value
#define    T_S8_MAX				((T_S8)127)                        // maximum T_S8 value
#define    T_S16_MIN			((T_S16)(-32767L-1L))        // minimum T_S16 value
#define    T_S16_MAX			((T_S16)(32767L))            // maximum T_S16 value
#define    T_S32_MIN			((T_S32)(-2147483647L-1L))    // minimum T_S32 value
#define    T_S32_MAX			((T_S32)(2147483647L))        // maximum T_S32 value
#define    T_S64_MIN			((T_S32)(-9223372036854775807L-1L))    // minimum T_S64 value
#define    T_S64_MAX			((T_S32)(9223372036854775807L))        // maximum T_S64 value

/* basal type definition for global area */
typedef T_U8					T_BOOL;		/* BOOL type */

typedef T_VOID *				T_pVOID;	/* pointer of void data */
typedef const T_VOID *			T_pCVOID;	/* const pointer of void data */


typedef T_CHR *					T_pSTR;		/* pointer of string */
typedef const T_CHR *			T_pCSTR;	/* const pointer of string */


typedef T_U16					T_WCHR;		/**< unicode char */
typedef T_U16 *					T_pWSTR;	/* pointer of unicode string */
typedef const T_U16 *			T_pCWSTR;	/* const pointer of unicode string */


typedef T_U8 *					T_pDATA;	/* pointer of data */
typedef const T_U8 *			T_pCDATA;	/* const pointer of data */

typedef T_U32					T_COLOR;		/* color value */

typedef T_U32					T_HANDLE;			/* a handle */

#ifndef _UNICODE
	#define T_TCHR				T_CHR		/**< character type that is portable for ANSI */
	#define T_pTSTR				T_pSTR		/**< string pointer type that is portable for ANSI */
	#define T_pCTSTR			T_pCSTR		/**< constant string pointer type that is portable for ANSI */
#else
	#define T_TCHR				T_WCHR		/**< character type that is portable for Unicode */
	#define T_pTSTR				T_pWSTR		/**< string pointer type that is portable for Unicode */
	#define T_pCTSTR			T_pCWSTR	/**< constant string pointer type that is portable for Unicode */
#endif

typedef T_S16					T_LEN;		/**< length type: unsigned short */
typedef T_S16					T_POS;		/**< position type: short */
typedef T_S32					T_TIMER;	/**< timer type: int */
typedef	T_pVOID					T_LPTHREAD_START;	/**< handle for thread */

typedef T_U32					T_eLANGUAGE;

#define		AK_FALSE			0
#define		AK_TRUE				1
#define 	AK_NULL				((T_pVOID)(0))

#define		AK_EMPTY

/* universal structure definition for global area*/
typedef struct {
	T_POS		x;
	T_POS		y;
}T_POINT, *T_pPOINT;

typedef struct {
	T_POS	left;
	T_POS	top;
	T_LEN	width;
	T_LEN	height;
} T_RECT, *T_pRECT;

typedef struct{
    T_U32 low;
    T_U32 high;
} T_U64_INT;

typedef T_U8 T_FONT;


/**
 * @brief system time struction
 
 *   define system time
 */
typedef struct tagSYSTIME{
    T_U16   year;               ///< 4 byte: 1980-2099
    T_U8    month;              ///< 1-12 
    T_U8    day;                ///< 1-31 
    T_U8    hour;               ///< 0-23 
    T_U8    minute;             ///< 0-59 
    T_U8    second;             ///< 0-59 
    T_U16   milli_second;       ///< 0-999 
    T_U8    week;               ///< 0-6,  0: monday, 6: sunday
} T_SYSTIME, *T_pSYSTIME;

//for debug
#define HJH_DEBUG AK_DEBUG_OUTPUT
//#define HJH_DEBUG



#endif	//  _ANYKA_TYPES_H_

