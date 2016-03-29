/****************************************************************************************
File: std_Types.h

Description: standard Types definations 

Component: std

Module: std_Types

Description: Contains all types used globally 

Author: A.Osama

Date: 26-Mar-07'

Comment: Types may be modified or updated during development , please make sure of the
		 Version of this file before using it to make sure of compliance.

Version: 0.1
		 0.2 -Added sint8_t ,sint8 types
		 0.3 -Changed file version to 6 to align with driver version6
****************************************************************************************/

#ifndef STD_TYPES
#define STD_TYPES
//Comment: Current file version should reflect the file version mentioned above
#define STD_TYPES_VERSION 0

//Standard Types
typedef unsigned char	uint8_t;
typedef unsigned short  uint16_t;
typedef signed short    int16_t;
typedef unsigned long   uint32_t;
typedef signed long     int32_t;
typedef unsigned char	bool_t;
typedef uint32_t   Bool;

typedef unsigned char	uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
typedef signed char     sint8_t;
typedef signed char     sint8;
typedef signed short    sint16;
typedef signed long     sint32;
typedef char *			tString;
//Bool Type constants
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef NULL
	#define NULL (uint32_t)0x00
#endif
#define tHANDLE	void *
//#define True TRUE
//#define False FALSE


typedef struct
{
	uint8_t* paArray;
	uint32_t u32Length;
}tstrArray;

#endif