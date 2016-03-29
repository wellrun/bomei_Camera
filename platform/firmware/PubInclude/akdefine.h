/**
 * @file akdefine.h
 * @brief Type definition and some global definition
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-16
 * @version 1.0
 * @ref
 */



#ifndef __AK_DEFINE_H__
#define __AK_DEFINE_H__

#include "Anyka_types.h"

/* Define OS 
// OS_WIN32 OS_ANYKA 
#ifdef WIN32	
	#ifndef OS_WIN32
		#define OS_WIN32
	#endif
#else
	#ifndef OS_ANYKA
		#define OS_ANYKA
	#endif
#endif
*/
#define PONOFF_VIDEO_PLAY_TIME 	10
#define PONOFF_VIDEO_PLAY_SPEED    20  //ms
#define POWER_ON_BACK_COLOR     COLOR_ROYALBLUE1

#define ERROR_TIMER			  -1
#define MAX_LCD_NUM                 1

#define LCD_MODE_565			1

#ifdef OS_ANYKA
#define MAIN_LCD_WIDTH         LCD_CONFIG_WIDTH		// 800 //480//320
#define MAIN_LCD_HEIGHT        LCD_CONFIG_HEIGHT	// 480 //272//240
#else
#define MAIN_LCD_WIDTH         320
#define MAIN_LCD_HEIGHT        240
#endif

#ifdef OS_WIN32
#define SLAVE_LCD_WIDTH	    0	    //no slave lcd
#define SLAVE_LCD_HEIGHT	0
#endif

#ifdef CHRONTEL_7026_720_480 
#define GB_DISPBUF_LEN		(MAIN_LCD_WIDTH * MAIN_LCD_WIDTH  * 3)
#define GB_DISPBUF565_LEN 	(720*480*2)
#else
#define GB_DISPBUF_LEN		(MAIN_LCD_WIDTH * MAIN_LCD_WIDTH  * 3)
#define GB_DISPBUF565_LEN 	(720*288*2)
#endif



//akos//////////////////////////////////////////////////////////////////////////
typedef T_S32				T_hTask;
typedef T_S32				T_hQueue;
typedef T_S32				T_hSemaphore;
typedef T_S32				T_hHisr;
typedef T_U8					T_OPTION;
typedef T_S32				T_hMailbox;
typedef T_S32				T_hEventGroup;
typedef T_S32				T_hTimer;

typedef T_S32				T_ASIC_FREQ;


/* Define constants for use in service parameters.  */

#define         AK_AND                          2
#define         AK_AND_CONSUME                  3
#define         AK_DISABLE_TIMER                4
#define         AK_ENABLE_TIMER                 5
#define         AK_FIFO                         6
#define         AK_FIXED_SIZE                   7
#define         AK_NO_PREEMPT                   8
#define         AK_NO_START                     9
#define         AK_NO_SUSPEND                   0
#define         AK_OR                           0
#define         AK_OR_CONSUME                   1
#define         AK_PREEMPT                      10
#define         AK_PRIORITY                     11
#define         AK_START                        12
#define         AK_SUSPEND                      0xFFFFFFFFUL
#define         AK_VARIABLE_SIZE                13

/* Define task suspension constants.  */

#define         AK_DRIVER_SUSPEND               10
#define         AK_EVENT_SUSPEND                7
#define         AK_FINISHED                     11
#define         AK_MAILBOX_SUSPEND              3
#define         AK_MEMORY_SUSPEND               9
#define         AK_PARTITION_SUSPEND            8
#define         AK_PIPE_SUSPEND                 5
#define         AK_PURE_SUSPEND                 1
#define         AK_QUEUE_SUSPEND                4
#define         AK_READY                        0
#define         AK_SEMAPHORE_SUSPEND            6
#define         AK_SLEEP_SUSPEND                2
#define         AK_TERMINATED                   12

/* Define task status. */

#define         AK_DRIVER_SUSPEND               10
#define         AK_EVENT_SUSPEND                7
#define         AK_FINISHED                     11
#define         AK_MAILBOX_SUSPEND              3
#define         AK_MEMORY_SUSPEND               9
#define         AK_PARTITION_SUSPEND            8
#define         AK_PIPE_SUSPEND                 5
#define         AK_PURE_SUSPEND                 1
#define         AK_QUEUE_SUSPEND                4
#define         AK_READY                        0
#define         AK_SEMAPHORE_SUSPEND            6
#define         AK_SLEEP_SUSPEND                2
#define         AK_TERMINATED                   12

//lcd//////////////////////////////////////////////////////////////////////////
#define COLOR_BLACK			0x00000000			/* black: RGB(0, 0, 0) */
#define COLOR_RED			0x00FF0000			/* red: RGB(255, 0, 0) */
#define COLOR_GREEN			0x0000FF00			/* green: RGB(0, 255, 0) */
#define COLOR_BLUE			0x000000FF			/* blue: RGB(0, 0, 255) */
#define COLOR_YELLOW		0x00FFFF00			/* yellow: RGB(255, 255, 0) */
#define COLOR_CYAN			0x00FFFF00			/* cyan: RGB(0, 255, 255) */
#define COLOR_FUCHSIN		0x00FF00FF			/* fuchsin: RGB(255, 0, 255) */
#define COLOR_WHITE		    0x00FFFFFF			/* white: RGB(255, 255, 255) */
#define COLOR_SLATEGREY     0x708090            /* SlateGrey: RGB(112, 128, 144) */
#define COLOR_GREY31        0x4F4F4F            /* Grey31: RGB(79, 79, 79) */
#define COLOR_LIGHT_BLUE	0x00A6CAF0
#define COLOR_LIGHT_GREEN	0x00ABFABD
#define COLOR_LIGHT_RED		0x00F9A593


//color define for app/////////////////////////////////////////////////////////////////
#define COLOR_VIDEO_FILE_STAMP   0x004040        /*video file stamp color*/


//camera/////////////////////////////////////////////////////////////////////
/** @brief Camera Parameter Exposure definition
 *
 *	This structure define the value of parameter Exposure
 */

typedef enum {
	CAMERA_FOCUS_0 = 0,
	CAMERA_FOCUS_1,
	CAMERA_FOCUS_2,
	CAMERA_FOCUS_3,
	CAMERA_FOCUS_4,
	CAMERA_FOCUS_5,
	CAMERA_FOCUS_6,
	CAMERA_FOCUS_7,
	CAMERA_FOCUS_8,
	CAMERA_FOCUS_9,
	CAMERA_FOCUS_10,
	CAMERA_FOCUS_NUM
} T_CAMERA_FOCUS_LEVEL;

typedef enum {
    CAMERA_MODE_QXGA = 0,	// 2048 X 1536
	CAMERA_MODE_UXGA,		// 1600 X 1200
	CAMERA_MODE_SXGA,   	// 1280 X 1024
	CAMERA_MODE_XGA,		// 1024 X 768
	CAMERA_MODE_SVGA,		// 800  X 600
	CAMERA_MODE_VGA,    	// 640  X 480 
	CAMERA_MODE_QSVGA,  	// 400  X 300
	CAMERA_MODE_CIF,    	// 352  X 288
	CAMERA_MODE_QVGA,   	// 320  X 240
	CAMERA_MODE_QCIF,   	// 176  X 144
	CAMERA_MODE_QQVGA,		// 160  X 120
	CAMERA_MODE_PREV,		// 640  X 480
	CAMERA_MODE_REC,		// 352	X 288
	CAMERA_MODE_720P,		// 1280 X 720
	CAMERA_MODE_960,		// 1280 X 960
	CAMERA_MODE_D1,		    // 720 X 480
	CAMERA_MODE_NUM
} T_CAMERA_MODE;



//tscr/////////////////////////////////////////////////////////////////////////
/**	Use this value to set the touch panel to recognize charachter */
#define TSCR_SETMODE_RECO				0x00
/**	Use this value The user can only get up down move */
#define TSCR_SETMODE_GRAP				0x01


//USB//////////////////////////////////////////////////////////////////////////

#define USB_MODE_DMA			(1<<16)	  ///<use dma mode
#define USB_MODE_CPU			(1<<17)	  ///<use cpu mode

//kepad////////////////////////////////////////////////////////////////////////
typedef enum {
    KEYPAD_MATRIX_NORMAL = 0,   // normal matrix keypad
    KEYPAD_MATRIX_DIODE,        // diode matrix keypad
    KEYPAD_MATRIX_MIXED,        // mixed matrix keypad
    KEYPAD_KEY_PER_GPIO,         //one gpio = one key keypad
    KEYPAD_MATRIX_TIANXIN,            // keypad for tianxin board
    KEYPAD_ANALOG,            // keypad for analog keypad
    KEYPAD_TYPE_NUM
} T_KEYPAD_TYPE;

//alignMode
#define ALIGN_HORIZONTAL       (0x0f)
#define ALIGN_LEFT             (0x01)
#define ALIGN_RIGHT            (0x02)
#define ALIGN_HCENTER          (0x04)
#define ALIGN_VERTICAL         (0xf0)
#define ALIGN_UP               (0x10)
#define ALIGN_DOWN             (0x20)
#define ALIGN_VCENTER          (0x40)

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
#define COLOR_SIZE	2
#else
#define COLOR_SIZE	3
#endif


typedef enum{
	MOVETYPE_X,		//x方向移动消息
	MOVETYPE_Y,		//y方向移动消息
}E_MOVETYPE;



#endif

