/**
 * @file Ctl_SlipScrb.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#include "ctl_slipScrb.h"
#include "fwl_display.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_graph.h"
#include "fwl_graphic.h"



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
T_SLIP_SCRB *SlipScrb_Creat(T_RECT rect, E_MOVETYPE movetype)
{
	T_SLIP_SCRB *pSlipScrb = AK_NULL;

	if (movetype > MOVETYPE_Y)
	{
		return AK_NULL;
	}

	pSlipScrb = (T_SLIP_SCRB *)Fwl_Malloc(sizeof(T_SLIP_SCRB));

	AK_ASSERT_PTR(pSlipScrb, "SlipScrb_Creat(): pSlipScrb malloc error", AK_NULL);
	memset(pSlipScrb, 0, sizeof(T_SLIP_SCRB));

	pSlipScrb->rect = rect;
	pSlipScrb->moveType = movetype;

	return pSlipScrb;
}

/**
* @brief Destroy a slip scrollbar control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_SCRB *pSlipScrb:the scrollbar handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipScrb_Destroy(T_SLIP_SCRB *pSlipScrb)
{
	if (AK_NULL == pSlipScrb)
	{
		return AK_NULL;
	}

	pSlipScrb = Fwl_Free(pSlipScrb);
	return AK_NULL;
}

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
T_BOOL SlipScrb_Show(T_SLIP_SCRB *pSlipScrb, T_U8* buf, T_U32 imgWidth, T_U32 imgHeight)
{
	T_RECT rect;
	
	if (AK_NULL == pSlipScrb)
	{
		return AK_FALSE;
	}

	if (AK_NULL == buf)
	{
		return AK_FALSE;
	}

	if (MOVETYPE_Y == pSlipScrb->moveType)
	{
		if (pSlipScrb->maxSize <= (T_U32)pSlipScrb->rect.height)
		{
			pSlipScrb->visible = AK_FALSE;
		}
		else
		{
			pSlipScrb->visible = AK_TRUE;
			pSlipScrb->size = pSlipScrb->rect.height * pSlipScrb->rect.height / pSlipScrb->maxSize;
			
			if (pSlipScrb->size < SCROLL_BLOCK_MIN_VSIZE)
			{
				pSlipScrb->pos = (T_POS)(pSlipScrb->pos * (pSlipScrb->rect.height - (SCROLL_BLOCK_MIN_VSIZE - pSlipScrb->size)) / pSlipScrb->rect.height);
				pSlipScrb->size = SCROLL_BLOCK_MIN_VSIZE;
			}
		}

		if (pSlipScrb->visible)
		{
			RectInit(&rect, pSlipScrb->rect.left, pSlipScrb->pos, pSlipScrb->rect.width, (T_LEN)pSlipScrb->size);
			Fwl_FillSolidRectOnRGB(buf, imgWidth, imgHeight, &rect, COLOR_BLUE, RGB565);
		}
	}
	else if (MOVETYPE_X == pSlipScrb->moveType)
	{
		if (pSlipScrb->maxSize <= (T_U32)pSlipScrb->rect.width)
		{
			pSlipScrb->visible = AK_FALSE;
		}
		else
		{
			pSlipScrb->visible = AK_TRUE;
			pSlipScrb->size = pSlipScrb->rect.width * pSlipScrb->rect.width / pSlipScrb->maxSize;
			
			if (pSlipScrb->size < SCROLL_BLOCK_MIN_VSIZE)
			{
				pSlipScrb->pos = (T_POS)(pSlipScrb->pos * (pSlipScrb->rect.width - (SCROLL_BLOCK_MIN_VSIZE - pSlipScrb->size)) / pSlipScrb->rect.width);
				pSlipScrb->size = SCROLL_BLOCK_MIN_VSIZE;
			}
		}

		if (pSlipScrb->visible)
		{
			RectInit(&rect, pSlipScrb->pos, pSlipScrb->rect.top, (T_LEN)pSlipScrb->size, pSlipScrb->rect.height);
			Fwl_FillSolidRectOnRGB(buf, imgWidth, imgHeight, &rect, COLOR_BLUE, RGB565);
		}
	}

	return AK_TRUE;
}


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
T_BOOL SlipScrb_SetCurPos(T_SLIP_SCRB *pSlipScrb, T_POS curPos)
{
	if (AK_NULL == pSlipScrb)
	{
		return AK_FALSE;
	}

	pSlipScrb->pos = curPos;

	return AK_TRUE;
}

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
T_BOOL SlipScrb_SetMaxSize(T_SLIP_SCRB *pSlipScrb, T_U32 MaxSize)
{
	if (AK_NULL == pSlipScrb)
	{
		return AK_FALSE;
	}

	pSlipScrb->maxSize = MaxSize;

	return AK_TRUE;
}

