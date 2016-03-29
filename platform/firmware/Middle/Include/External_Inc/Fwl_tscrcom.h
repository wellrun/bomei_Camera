/** @file
 * @brief touch screen  handle write interface header file
 *
 * This file provides functions of init touch screen ,set recognize mode ,recognize range  and so on!
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LuoXiaoQing
 * @date 2006-04-27
 * @version 1.0
 */

#ifndef __FWL_TSCR_COM_H__
#define __FWL_TSCR_COM_H__

#include "akdefine.h"
#include "fwl_calibrate.h"

 /** @{@name Touch Panel Work Mode value
 *	These value set the touch panel recognize off / on
 *	
 */
 #if 0
/**	Use this value to set the touch panel to recognize charachter */
#define TSCR_SETMODE_RECO				0x00
/**	Use this value The user can only get up down move */
#define TSCR_SETMODE_GRAP				0x01
#endif

 /** @{@name Recognize Range value
 *	These flags can set the recognize range !
 *	Use bit 0 to 16 bit length range value
 */
/**	Use this flag to choose Simple  Chinese I */
#define TSCR_SETRECOGMODE_CHI_ST		0x0001	   
/**	Use this flag to choose Simple  Chinese II */
#define TSCR_SETRECOGMODE_CHII_ST		0x0002
/**	Use this flag to choose  Tran.  Chinese  */
#define TSCR_SETRECOGMODE_CH_T		0x0004
/**	Use this flag to choose  digiter  */
#define TSCR_SETRECOGMODE_DIG			0x0020
/**	Use this flag to choose  lower case charachter  */
#define TSCR_SETRECOGMODE_LOWER		0x0040	
/**	Use this flag to choose  upper case charachter  */
#define TSCR_SETRECOGMODE_UPPER		0x0080
/**	Use this flag to choose    interpunction*/
#define TSCR_SETRECOGMODE_CHAR		0x0100
/**	Use this flag to choose    Library setting */
#define TSCR_SETRECOGMODE_LABRA		0X4000
/**	Use this flag to choose    User specific */
#define TSCR_SETRECOGMODE_USER_SPECIFIC	0X8000

/**
*@action event by using touch panel
*/
typedef enum
{
	eTOUCHSCR_UP,
	eTOUCHSCR_DOWN,
	eTOUCHSCR_MOVE,
	eTOUCHSCR_GETWORDS,
	eTOUCHSCR_REFRESH_LINE
}T_TOUCHSCR_ACTION;

/** @defgroup FWL_TSCR Touch panel  interface
    @ingroup FWL
 */
/*@{*/

 /** 
 * @brief touch panel hardware init .  Inclouding Touch Screen, ADC timer. 
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return none
 * @remark This function only need call one times when the system boot.
 */
T_BOOL Fwl_Tscr_HW_init(T_VOID);
 /** 
 * @brief touch panel hardware free .Inclouding Touch Screen, ADC timer. 
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @return none
 * @remark This function only need call one times when the system power off.
 */
T_VOID Fwl_Tscr_HW_free(T_VOID);

 /** 
 * @brief  Get recognized result
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[out] dest  The string which  recognized result copy to . 
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
T_BOOL Fwl_TSCR_GetWords(T_pTSTR dest);


  /** 
 * @brief set the recognize range  
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[out] range set the flags of range to set recognize range  
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark the range flags is  defined in  Recognize Range value
 */
T_BOOL Fwl_tscr_SetRecogMode(T_U32 range);


  /** 
 * @brief set write Area when handwriting
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[in] rect  the write Area rect .
  * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 * @remark the range flags is  defined in  Recognize Range value
 */
T_BOOL Fwl_tscr_SetWriteArea(const T_RECT *rect);

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
T_BOOL Fwl_tscr_SetMode(T_U8 mode);
 
 /** 
 * @brief set the text show rect when handwriting
 * @author \b LuoXiaoqing
 * @date 2006-04-26
 * @param[in] rect  the text show rect .
 * @return AK_TRUE if the function is successful; otherwise is AK_FALSE
 */
 T_BOOL Fwl_tscr_SetTextRect(const T_RECT *rect);

#if 0
/** 
 * @brief Get the Current ADC point
 * @author \b LuoXiaoqing
 * @date 2006-04-26
  * @param[out] ADpt  the current ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
*/  
 T_U8   Fwl_tscr_getCurADPt(T_pTSPOINT   ADpt);
/** 
 * @brief Get the Last down ADC point
  * @param[out] ADpt  the Last down ADC Sample point the (x,y) is from 0--1024
 * @return T_U8
 */  
 T_U8   Fwl_tscr_getLastDownADPt(T_pTSPOINT   ADpt); 
#endif
  
 T_U8	 Fwl_tscr_getCurADPt_point(T_pPOINT   ADpt);


 /** 
  * @brief Get user's Mode
   * @param[out] user's touch screen Mode 
  * @return T_U8
  */ 
 T_U8	Fwl_tscr_getUserMode(T_VOID);
 
  /** 
  * @brief test whether the touch panel need to response
   * @param[out]  
  * @return AK_TRUE if screan need response;Otherwise is AK_FALSE 
  */ 
T_BOOL Fwl_TSCR_IsNeedRespond(T_VOID);
 /** 
  * @brief set status of touch panel
  * @param[in]   T_BOOL status : AK_TRUE if open the touch panel;otherwise AK_FALSE
  * @return T_VOID
  */ 
T_VOID Fwl_TSCR_SetTouchStatus(T_BOOL status);

    /** 
 * @brief  enable the touch screen
 * @author 
 * @date 
  * @param[out] none
 * @return  none
 */   
T_VOID Fwl_EnableTSCR(T_VOID);


  /** 
 * @brief  disable the touch screen
 * @author 
 * @date 
  * @param[out] 
 * @return 
 */   
T_VOID Fwl_DisableTSCR(T_VOID);
    /** 
 * @brief   check that whether the touch screen is validate
 * @author 
 * @date 
  * @param[out] 
 * @return 
 */   
T_BOOL Fwl_IsTSCRValid(T_VOID) ;


#ifdef SUPPORT_AUTOTEST
/** 
 * @brief   auotest  send tscr info callback
 * @author  lixingjian
 * @date 
  * @param[out] 
 * @return 
 */   

T_VOID Autotest_TSCR_SendEvent_Callback(T_TOUCHSCR_ACTION act ,T_POS pointx, T_POS pointy);	
#endif
 /*@}*/
#endif


