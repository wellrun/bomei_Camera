/**
 * @file tscr_input.h
 * @brief handwriting  driver header
 *
 * This driver handle the handwriting process . It include setting recognize mode ,setting recognize range,
 setting text show rect,setting work mode and so on.
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LuoXiaoqing
 * @date 2006-04-26
 * @version 1.0
 */


#ifndef __H_TSCR_INPUT__
#define __H_TSCR_INPUT__
#include "Gbl_Global.h"
#include "tscr_command.h"
#include "Fwl_tscrcom.h"
#include "drv_api.h"


/** @defgroup TSINPUT touch panel handwriting driver 
 *	@ingroup TS
 */
/*@{*/


/** @{@name point status
	These value define the point status.when the ADC driver fix a point by call back function.
 */
/**	inking coordinates in writing */ 
#define ePH_RET_INKING_COOR			2

/** The current coordinates is over the write area */ 
#define ePH_RET_COOR_OVER_WRITEAREA		14

/**recognize over  */
#define ePH_RET_RECOG_ARRAY			4

/** pen up  */
#define ePH_RET_PEN_UP				7

/** pen down */
#define ePH_RET_PEN_DOWN			8

/** pen move */
#define ePH_RET_PEN_MOVE			9

/** stroke over */
#define ePH_RET_STROKE_OVER			11

/** word over */
#define ePH_RET_WORD_OVER			12
/** @} */

/**  line num  when drawing  a line in writing */
#define TSCR_DRAW_LINE_NUM		3

/**
 * @brief TSCRGET callback define
 *	define TSCRGET callback function type
 */
typedef T_VOID (*T_fTSCRGET_CALLBACK)(T_TOUCHSCR_ACTION act ,T_POINT pt);

 /** 
 * @brief touch panel hardware init
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return none
 * @remark This function only need call one times when the system boot.
 */
T_BOOL tscr_HW_init(T_VOID);
 /** 
 * @brief touch panel hardware free .Inclouding Touch Screen, ADC timer. 
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return none
 * @remark This function only need call one times when the system power off.
 */
T_VOID tscr_HW_free(T_VOID);
  /** 
 * @brief touch panel module software init.
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark This function will called when user init a handwriting editor. 
 */
T_BOOL tscr_SW_init( T_VOID);

 /** 
 * @brief free the WT recognize LIB
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
T_BOOL tscr_SW_free(T_VOID);


 /** 
 * @brief set write Area when handwriting
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[in] rect  the write Area rect .
  * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
T_BOOL tscr_SetWriteArea(const T_RECT *rect);

 /** 
 * @brief set the text show rect when handwriting
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[in] rect  the text show rect .
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
 T_BOOL tscr_SetTextRect(const T_RECT *rect);

 /** 
 * @brief set the touch panel work mode 
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[in] mode  TSCR_SETMODE_GECO for recogniton mode, TSCR_SETMODE_GRAP for graphic mode
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark  Default is Recognition mode. Graphic mode transmission coordinates
		are similar to Recognition mode except recognition is not processed
		under Graphic mode.
 */
T_BOOL tscr_SetMode(T_U8 mode);
 
 /** 
 * @brief set the recognize range  
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[out] range set the flags of range to set recognize range  
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark the range flags is  defined in  Recognize Range value
 */
T_BOOL tscr_SetRecogMode(T_U32 rangeInput);
  	

 /** 
 * @brief Set call back function for send event to state machine
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return none
 * @remark This function only need call one times when the system boot.
 */
T_BOOL tscr_set_event_callback(T_fTSCRGET_CALLBACK callback_func);

 
 /** 
 * @brief get the current LCD point . 
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[out] pt store the current LCD point 
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark  the point value is:  width 0 to 239 ,  height 0 to 319
 */
T_BOOL tscr_GetPoint(T_POINT *pt);


  /** 
 * @brief Get the Current ADC point
 * @author \b LuoXiaoqing
 * @date 2006-04-26
  * @param[out] ADpt  the current ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
 */  
 T_U8   tscr_getCurADPt(T_pTSPOINT   ADpt);

    /** 
 * @brief Get the Last down ADC point
  * @param[out] ADpt  the Last down  ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
 */  
 T_U8   tscr_getLastDownADPt(T_pTSPOINT   ADpt);
	
 /** 
 * @brief  Get recognized result
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[out] dest  The string which  recognized result copy to . 
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
T_BOOL tscr_GETWORDS(T_pTSTR dest);

/** 
* @brief  Get user mode
* @author 
* @date 
* @return T_U8 : the user mode
*/
T_U8   tscr_getUserMode(T_VOID);
/** 
* @brief  show current line
* @author 
* @date 
* @T_POINT endpt : the line which line draw to
* @return T_U8 : the user mode
*/
T_VOID tscr_showCurLine(T_POINT endpt);
/**
*@brief set matrix
*@return T_VOID
*/
T_VOID tscr_SetMatrixToDef(T_VOID);
/**
*@brief get init status
*@return AK_TRUE if init is ready;otherwise AK_FALSE 
*/
T_BOOL tscr_GetWTInitStatus(T_VOID);


// Load handwrite dictionary
T_S16 HWRE_LoadDict(T_VOID);

// Free handwrite dictionary
T_VOID HWRE_FreeDict(T_VOID);


/*@}*/
#endif //__H_TSCR_INPUT__





