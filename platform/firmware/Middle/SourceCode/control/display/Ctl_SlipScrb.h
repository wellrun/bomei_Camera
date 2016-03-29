/**
 * @file Ctl_SlipScrb.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPSCRB_H__
#define __CTL_SLIPSCRB_H__

#include "akdefine.h"
#include "Ctl_SlipMgr.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	SCROLL_BLOCK_MIN_VSIZE	(20)
#define	SCROLL_WIDTH			(3)



/**
* @brief Creat a slip scrollbar control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_RECT rect:rect
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIP_SCRB * the scrollbar handle
* @retval
*/
T_SLIP_SCRB *SlipScrb_Creat(T_RECT rect, E_MOVETYPE movetype);

/**
* @brief Destroy a slip scrollbar control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_SCRB *pSlipScrb:the scrollbar handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipScrb_Destroy(T_SLIP_SCRB *pSlipScrb);



/**
* @brief Set cur pos
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_SCRB *pSlipScrb:the scrollbar handle
* @param in T_POS curPos:the pos to show
* @return T_BOOL
* @retval
*/
T_BOOL SlipScrb_SetCurPos(T_SLIP_SCRB *pSlipScrb, T_POS curPos);

/**
* @brief Show a slip scrollbar control to a buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_SCRB *pSlipScrb:the scrollbar handle
* @param in T_U8* buf:the buf to show
* @param in T_U32 imgWidth:the width of the buf
* @param in T_U32 imgHeight:the height of the buf
* @return T_BOOL
* @retval
*/
T_BOOL SlipScrb_Show(T_SLIP_SCRB *pSlipScrb, T_U8* buf, T_U32 imgWidth, T_U32 imgHeight);

/**
* @brief Set max size
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_SCRB *pSlipScrb:the scrollbar handle
* @param in T_U32 MaxSize:the max size of the view
* @return T_BOOL
* @retval
*/
T_BOOL SlipScrb_SetMaxSize(T_SLIP_SCRB *pSlipScrb, T_U32 MaxSize);





#ifdef __cplusplus
}
#endif

#endif
