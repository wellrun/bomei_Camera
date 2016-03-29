/**
 * @file Ctl_Text.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  2011-08-23
 * @version 1,0 
 */


#include "ctl_text.h"
#include "akdefine.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_graph.h"
#include "eng_font.h"
#include "fwl_font.h"
#include "eng_dynamicfont.h"
#include "eng_string_uc.h"


#define	SCRL_OK_COUNT_MAX	2

/**
 * @brief Creat a Text control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_RECT rect:
 * @param in T_U32 alignmode:
 * @return T_SLIP_TEXT * the Text handle
 * @retval
 */
T_SLIP_TEXT *Text_Creat(T_RECT rect, T_U32 alignmode)
{
	T_SLIP_TEXT *pSlipText = AK_NULL;

	pSlipText = (T_SLIP_TEXT *)Fwl_Malloc(sizeof(T_SLIP_TEXT));
	AK_ASSERT_PTR(pSlipText, "Text_Creat(): pSlipText malloc error", AK_NULL);

	memset(pSlipText, 0, sizeof(T_SLIP_TEXT));

	pSlipText->rect = rect;
	pSlipText->alignMode = alignmode;

	return pSlipText;
}

/**
 * @brief Destroy a Text control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @return AK_NULL
 * @retval
 */
T_VOID *Text_Destroy(T_SLIP_TEXT *pSlipText)
{
	if (AK_NULL == pSlipText)
	{
		return AK_NULL;
	}

	if (AK_NULL != pSlipText->pText)
    {
		pSlipText->pText = Fwl_Free(pSlipText->pText);
    }

	pSlipText = Fwl_Free(pSlipText);
	
	return AK_NULL;
}

/**
 * @brief Set Text data
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in const T_U16* pText:the text data
 * @return T_BOOL
 * @retval
 */
T_BOOL Text_SetText(T_SLIP_TEXT *pSlipText, const T_U16* pText)
{
	T_U32 len = 0;
	
	if (AK_NULL == pSlipText)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pText)
	{
		return AK_FALSE;
	}

	if (AK_NULL != pSlipText->pText)
    {
		pSlipText->pText = Fwl_Free(pSlipText->pText);
    }

	len = Utl_UStrLen(pText);

    /*malloc new text*/
    pSlipText->pText = (T_U16*)Fwl_Malloc((len + 1) << 1);
    if (AK_NULL == pSlipText->pText)
    {
        AK_DEBUG_OUTPUT("Text_SetText(): pSlipText->pText malloc fail\n");
        return AK_FALSE;
    }

    Utl_UStrCpy(pSlipText->pText, pText);

	return AK_TRUE;
}


/**
 * @brief Show Text data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @param in  T_COLOR color:color
 * @return T_BOOL
 * @retval
 */
T_BOOL Text_Show(T_SLIP_TEXT *pSlipText, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight, T_COLOR color)
{
	T_RECT rect;
	T_POS left = 0;
	T_POS top = 0;
	T_U32 width = 0;
	T_S32 i = 0;
	T_U32 len = 0;
	T_U32 displen = 0;
	
	if (AK_NULL == pSlipText)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pSlipText->pText)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pbuf)
	{
		return AK_FALSE;
	}

	pSlipText->scrlOffset = 0;
	pSlipText->scrlOkCnt = 0;
	
	RectInit(&rect, 0, 0, pSlipText->rect.width, pSlipText->rect.height);
	len = Utl_UStrLen(pSlipText->pText);
	width = UGetSpeciStringWidth(pSlipText->pText, CURRENT_FONT_SIZE, (T_U16)len);

	//当字串的宽度大于显示区域时，截断
	if (width > (T_U32)pSlipText->rect.width)
	{	
		for (i=len-1; i>=0; i--)
		{
			width = UGetSpeciStringWidth(pSlipText->pText, CURRENT_FONT_SIZE, (T_U16)i);
			if (width <= (T_U32)pSlipText->rect.width)
			{
				displen = (T_U32)i;
				break;
			}
		}	
	}
	else
	{
		displen = len;
	}
	
	if (ALIGN_LEFT == (pSlipText->alignMode & ALIGN_HORIZONTAL))
	{
		left = pSlipText->rect.left;
	}
	else if (ALIGN_RIGHT == (pSlipText->alignMode & ALIGN_HORIZONTAL))
	{
		left = (T_POS)(pSlipText->rect.left + pSlipText->rect.width - width);
	}
	else
	{
		left = (T_POS)(pSlipText->rect.left + (pSlipText->rect.width - width) / 2);
	}


	if (ALIGN_UP == (pSlipText->alignMode & ALIGN_VERTICAL))
	{
		top = pSlipText->rect.top;
	}
	else if (ALIGN_DOWN == (pSlipText->alignMode & ALIGN_VERTICAL))
	{
		top = (T_POS)(pSlipText->rect.top + pSlipText->rect.height - GetFontHeight(CURRENT_FONT_SIZE));
	}
	else
	{
		top = (T_POS)(pSlipText->rect.top + (pSlipText->rect.height - GetFontHeight(CURRENT_FONT_SIZE)) / 2);
	}
	
	Fwl_UDispSpeciStringOnRGB(pbuf, imgwidth, imgheight, left, top, 
							pSlipText->pText, color, RGB565, CURRENT_FONT_SIZE, (T_U16)displen);

	return AK_TRUE;
}

/**
 * @brief scroll Show Text data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @param in  T_COLOR color:color
 * @return T_BOOL
 * @retval
 */
T_BOOL Text_ScrollShow(T_SLIP_TEXT *pSlipText, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight, T_COLOR color)
{
	T_RECT rect;
	T_POS left = 0;
	T_POS top = 0;
	T_U32 width = 0;
	T_S32 i = 0;
	T_U32 len = 0;
	T_U32 displen = 0;
	T_BOOL	ret = AK_FALSE;
	
	if (AK_NULL == pSlipText)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pSlipText->pText)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pbuf)
	{
		return AK_FALSE;
	}
	
	RectInit(&rect, 0, 0, pSlipText->rect.width, pSlipText->rect.height);
	len = Utl_UStrLen(pSlipText->pText);
	width = UGetSpeciStringWidth(pSlipText->pText, CURRENT_FONT_SIZE, (T_U16)len);

	//当字串的宽度大于显示区域
	if (width > (T_U32)pSlipText->rect.width)
	{	
		if (pSlipText->scrlOffset < (T_S32)Utl_UStrLen(pSlipText->pText))
		{
			pSlipText->scrlOffset++;
		}
		
		for (i=len-pSlipText->scrlOffset; i>=0; i--)
		{
			width = UGetSpeciStringWidth(pSlipText->pText + pSlipText->scrlOffset, CURRENT_FONT_SIZE, (T_U16)i);
			if (width <= (T_U32)pSlipText->rect.width)
			{
				displen = (T_U32)i;
				break;
			}
		}	

		ret = AK_TRUE;
	}
	else
	{
		pSlipText->scrlOffset = 0;
		displen = len;
		return ret;
	}
	
	left = pSlipText->rect.left;

	if (ALIGN_UP == (pSlipText->alignMode & ALIGN_VERTICAL))
	{
		top = pSlipText->rect.top;
	}
	else if (ALIGN_DOWN == (pSlipText->alignMode & ALIGN_VERTICAL))
	{
		top = (T_POS)(pSlipText->rect.top + pSlipText->rect.height - GetFontHeight(CURRENT_FONT_SIZE));
	}
	else
	{
		top = (T_POS)(pSlipText->rect.top + (pSlipText->rect.height - GetFontHeight(CURRENT_FONT_SIZE)) / 2);
	}
	
	Fwl_UDispSpeciStringOnRGB(pbuf, imgwidth, imgheight, left, top, 
							pSlipText->pText + pSlipText->scrlOffset, 
							color, RGB565, CURRENT_FONT_SIZE, (T_U16)displen);

	if (i == (T_S32)(len - pSlipText->scrlOffset))
	{
		if (pSlipText->scrlOkCnt < SCRL_OK_COUNT_MAX)
		{
			pSlipText->scrlOkCnt++;
		}
		else
		{
			pSlipText->scrlOkCnt = 0;
			pSlipText->scrlOffset = -1;
		}
	}

	return ret;
}

