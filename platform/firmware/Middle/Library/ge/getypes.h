/**
* @FILENAME getypes.h
* @BRIEF Graphic Effects Library Type Defines
* Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR Tommy Lau (Liu Huaiguang)
* @DATE 2007-09-20
* @VERSION 1.3
* @REF No reference
*/

#ifndef __GETYPES_H__
#define __GETYPES_H__

#ifdef __cplusplus
extern "C" {
#endif



// -------------------------------------------------------------------------- //
//                                                                            //
//                              Compiler Support                              //
//                                                                            //
// -------------------------------------------------------------------------- //

//	Would be good to have a test for platform.  If the platform isn't Windows
//	we should define __cdecl to be nothing.  We can't just use _MSC_VER though
//	as there are other compilers for the Windows platforms.
#if defined (_MSC_VER)
#	define GE_MSVC					1
#	ifdef __STDC__
	//	when compiling as ANSI, parameter-passing specifications aren't allowed
#		define GE_API
#		define GEINLINE
#		define GEFORCEINLINE
#		define GE_DWORD_ALIGN
#	else
#		define GE_API					__cdecl
#		define GEINLINE					_inline
#		define GEFORCEINLINE			__forceinline
#		define GE_INLINING_SUPPORTED	1
#		define GE_DWORD_ALIGN			__declspec(align(4))
#	endif
#elif defined (__GNUC__)
#	define GE_GNUC					1
#	define GE_API
#	define GEINLINE					inline
#	define GEFORCEINLINE			inline
#	define GE_INLINING_SUPPORTED	1
//#	define GE_DWORD_ALIGN			__declspec(align(4))
//	Modified by Tommy, use our GNU C's alignment
//	Our GNU C's version was too old, use the old method
#	define GE_DWORD_ALIGN			__attribute__((aligned(4)))
#elif defined (__arm)
#	define GE_ADS					1
#	define GE_API    
#	define GEINLINE					inline
#	define GEFORCEINLINE			inline
#	define GE_INLINING_SUPPORTED	1
#	define GE_DWORD_ALIGN			__declspec(align(4))
#else
#	error Unknown compiler -- please supply a compiler or check out the getypes.h.
#	define GE_API
#endif

// -------------------------------------------------------------------------- //
//                                                                            //
//                               Type Definition                              //
//                                                                            //
// -------------------------------------------------------------------------- //

																		/* 8-bit addressing    16 bit addressing*/
typedef void					GE_VOID;
typedef unsigned char			GE_BYTE;								/* 1 byte              1 byte */
typedef char					GE_CHAR;								/* 1 byte              1 byte */
typedef unsigned short			GE_WORD, GE_WCHAR;						/* 2 bytes             1 byte */
typedef short					GE_SHORT;								/* 2 bytes             1 byte */
typedef float					GE_FLOAT;
typedef unsigned int			GE_UINT, GE_BOOL;						/* 4 bytes             2 bytes */
typedef int						GE_INT;									/* 4 bytes             2 bytes */
typedef long					GE_LONG, GE_RESULT;						/* 4 bytes             2 bytes */
typedef unsigned long			GE_DWORD, GE_ULONG;						/* 4 bytes             2 bytes */
typedef unsigned short			GE_LANGID;
#if defined (_MSC_VER)
typedef __int64					GE_INT64;
typedef unsigned __int64		GE_UINT64;
#elif defined (__GNUC__)
typedef long long int			GE_INT64;
typedef unsigned long long int	GE_UINT64;
#endif

// -------------------------------------------------------------------------- //
//                                                                            //
//                              Error Definition                              //
//                                                                            //
// -------------------------------------------------------------------------- //

#define GE_FAILED(Status)		((GE_RESULT)(Status) < 0)
#define GE_SUCCEEDED(Status)	((GE_RESULT)(Status) >= 0)

//	Generic success return value
#define GE_SUCCESS				((GE_RESULT)0x00000000L)
#define GE_S_FALSE				((GE_RESULT)0x00000001L)
//#define GE_S_WITHOUT_ALPHA		((GE_RESULT)0x00000002L)

//	Fail return codes
//	Standard return codes copied from winerror.h
#define GE_E_FAIL						((GE_RESULT)0x80004005L)
#define GE_E_INVALIDARG					((GE_RESULT)0x80070057L)
#define GE_E_OUTOFMEMORY				((GE_RESULT)0x80000002L)
#define GE_E_FILENOTFOUND				((GE_RESULT)0x80030002L)
#define GE_E_FILEOPEN					((GE_RESULT)0x8003006EL)    
#define GE_E_BUFFERTOOSMALL				((GE_RESULT)0x8007007AL)
#define GE_E_NOTIMPL					((GE_RESULT)0x80004001L)
#define GE_E_NOMORE						((GE_RESULT)0x80070103L)
#define GE_E_PARAMETERS_MISMATCHED		((GE_RESULT)0xC00D272FL)

#define GE_SEVERITY_SUCCESS				0uL
#define GE_SEVERITY_ERROR				1uL
#define GE_FACILITY_GE					4uL
#define GE_S_BASECODE					0xC000
#define GE_E_BASECODE					0xC000

#define MAKE_GE_RESULT(sev, fac, code) \
((GE_RESULT)(((GE_ULONG)(sev)<<31)|((GE_ULONG)(fac)<<16)|((GE_ULONG)(code))))

// -------------------------------------------------------------------------- //
//                                                                            //
//                              Other Definition                              //
//                                                                            //
// -------------------------------------------------------------------------- //

#ifndef	NULL
#	define	NULL	((GE_VOID *)0)
#endif

#ifndef TRUE
#	define	TRUE	1
#endif

#ifndef FALSE
#	define	FALSE	0
#endif



#ifdef __cplusplus
}
#endif

#endif	//	__GETYPES_H__
