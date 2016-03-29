/**
 * @file Ctl_Icon.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  2011-08-23
 * @version 1,0 
 */


#include "ctl_icon.h"
#include "akdefine.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_graph.h"
#include "eng_akbmp.h"
#include "fwl_graphic.h"


/**
 * @brief Creat a Icon control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_RECT rect:
 * @param in T_U32 alignmode:
 * @return T_SLIP_ICON * the Icon handle
 * @retval
 */
T_SLIP_ICON *Icon_Creat(T_RECT rect, T_U32 alignmode)
{
	T_SLIP_ICON *pSlipIcon = AK_NULL;

	pSlipIcon = (T_SLIP_ICON *)Fwl_Malloc(sizeof(T_SLIP_ICON));
	AK_ASSERT_PTR(pSlipIcon, "Icon_Creat(): pSlipIcon malloc error", AK_NULL);

	memset(pSlipIcon, 0, sizeof(T_SLIP_ICON));
	
	pSlipIcon->rect = rect;
	pSlipIcon->alignMode = alignmode;
	
	return pSlipIcon;
}

/**
 * @brief Destroy a Icon control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @return AK_NULL
 * @retval
 */
T_VOID *Icon_Destroy(T_SLIP_ICON *pSlipIcon)
{
	if (AK_NULL == pSlipIcon)
	{
		return AK_NULL;
	}

	pSlipIcon = Fwl_Free(pSlipIcon);
	
	return AK_NULL;
}

/**
 * @brief Set Icon data
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @param in const T_pDATA pIcon:the icon data
 * @return T_BOOL
 * @retval
 */
T_BOOL Icon_SetIcon(T_SLIP_ICON *pSlipIcon, T_pCDATA pIcon)
{	
	if (AK_NULL == pSlipIcon)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pIcon)
	{
		return AK_FALSE;
	}

	pSlipIcon->pIcon = (T_pDATA)pIcon;
	
	return AK_TRUE;
}


/**
 * @brief Show Icon data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_ICON *pSlipIcon:the Icon handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @return T_BOOL
 * @retval
 */
T_BOOL Icon_Show(T_SLIP_ICON *pSlipIcon, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight)
{
	T_RECT rect;
	T_POS left = 0;
	T_POS top = 0;
	T_U32 width = 0;
	T_U32 height = 0;
	T_AK_BMP AnykaBmp;
	
	if (AK_NULL == pSlipIcon)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pSlipIcon->pIcon)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pbuf)
	{
		return AK_FALSE;
	}

	AkBmpGetFromString(pSlipIcon->pIcon, &AnykaBmp);

	width = AnykaBmp.Width >= pSlipIcon->rect.width ? pSlipIcon->rect.width : AnykaBmp.Width;
	height = AnykaBmp.Height >= pSlipIcon->rect.height ? pSlipIcon->rect.height : AnykaBmp.Height;

	if (ALIGN_LEFT == (pSlipIcon->alignMode & ALIGN_HORIZONTAL))
	{
		left = pSlipIcon->rect.left;
	}
	else if (ALIGN_RIGHT == (pSlipIcon->alignMode & ALIGN_HORIZONTAL))
	{
		left = (T_POS)(pSlipIcon->rect.left + pSlipIcon->rect.width - width);
	}
	else
	{
		left = (T_POS)((pSlipIcon->rect.left + pSlipIcon->rect.width - width) / 2);
	}


	if (ALIGN_UP == (pSlipIcon->alignMode & ALIGN_VERTICAL))
	{
		top = pSlipIcon->rect.top;
	}
	else if (ALIGN_DOWN == (pSlipIcon->alignMode & ALIGN_VERTICAL))
	{
		top = (T_POS)(pSlipIcon->rect.top + pSlipIcon->rect.height - height);
	}
	else
	{
		top = (T_POS)(pSlipIcon->rect.top + (pSlipIcon->rect.height - height) / 2);
	}
	
	RectInit(&rect, 0, 0, (T_LEN)width, (T_LEN)height);
	Fwl_AkBmpDrawPartFromStringOnRGB(pbuf, imgwidth, imgheight, left, top,
		&rect, pSlipIcon->pIcon, &g_Graph.TransColor, AK_FALSE, RGB565);

	return AK_TRUE;
}


