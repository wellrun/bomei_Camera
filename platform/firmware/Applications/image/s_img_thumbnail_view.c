#include "Fwl_public.h"
#ifdef SUPPORT_IMG_BROWSE
#include "fwl_vme.h"
#include "Lib_event.h"
#include "gbl_global.h"
#include "ctl_displaylist.h"
#include "eng_gblstring.h"
#include "eng_topbar.h"
#include "fwl_keyhandler.h"
#include "Lib_thumbsdb.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "ctl_msgbox.h"
#include "Ctl_ImgBrowse.h"
#include "eng_font.h"
#include "fwl_graphic.h"
#include "fwl_font.h"
#include "fwl_oscom.h"
#include "ctl_slipmgr.h"
#include "eng_screensave.h"
#include "Eng_KeyTranslate.h"
#include "Fwl_tscrcom.h"

#define     REFRESH_NONE				(0)
#define     REFRESH_FOCUSE_ITEM			(1)
#define     REFRESH_MOVE_BUF			(2)
#define     REFRESH_SETSLIDE			(4)
#define     REFRESH_ALL                 (0xff)

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
#define     THUMBS_NAME		         "thumbs565"
#else
#define     THUMBS_NAME		         "thumbs888"
#endif

#define     STAGE_COLOR_ONEROW			(0x00646464)
#define     STAGE_COLOR_FULLSCR			(0x00000000)

#define     FOCUS_COLOR_ONEROW			(0x00ffffff)
#define     FOCUS_COLOR_FULLSCR			(0x00ffffff)

#define     TOOLRECT_HEIGHT				(30)
#define     INFORECT_HEIGHT				(20)

#define     TOOL_PIC_WIDTH				(26)
#define     TOOL_PIC_INTERVAL_X			(10)
#define     TOOL_PIC_INTERVAL_Y			(2)
#define     SET_PIC_LEFT				(0)
#define     PLAY_PIC_LEFT				(MAIN_LCD_WIDTH/2-20)
#define     MODE_PIC_LEFT				(MAIN_LCD_WIDTH-40)

#define     SETSLIDE_HEIGHT				(30)
#define     SETSLIDE_TOP				(Fwl_GetLcdHeight() - TOOLRECT_HEIGHT - SETSLIDE_HEIGHT)
#define     SETSLIDEBAR_LEFT			(80)
#define     SETSLIDEBAR_WIDTH			(216)
#define     SETSLIDEBLOCK_WIDTH			(16)
#define     SETSLIDESTR_LEFT			(15)


typedef enum {
    THUMBNAIL_DIRECTION_UP = 0,
    THUMBNAIL_DIRECTION_DOWN,
    THUMBNAIL_DIRECTION_LEFT,
    THUMBNAIL_DIRECTION_RIGHT,
    THUMBNAIL_DIRECTION_NUM
} T_THUMBNAIL_DIRECTION;

typedef enum {
    THUMBNAIL_MODE_ONEROW = 0,
    THUMBNAIL_MODE_FULLSCR,
    THUMBNAIL_MODE_NUM
} T_THUMBNAIL_MODE;

typedef struct{
    T_RECT      rctPos;         //posintion
    T_S32       ItemIndex;      // the index of the item

}T_THUMBNAIL_ITEM;


typedef struct{
    T_S32   nItemHeight;
    T_S32   nItemWidth;
    T_S32   nItemX_Interval;
    T_S32   nItemY_Interval;
    T_COLOR ItemBgColor;

    T_S32   nThumbImgHeight;
    T_S32   nThumbImgWidth;

}T_UI_ITEM_ATTR;  // ui item attribution

typedef enum{
    NONE_ACTION,
    SETPOS_ACTION,
    ADD_ACTION,
    SUB_ACTION,
    MAX_ACTION
    
}T_PAGE_ACTION;

typedef struct tagResIcon
{	
	T_RECT		rect;
	T_pDATA		buf;
	T_pCDATA	pic;
	
}T_RES_ICON;

typedef struct{
    T_DISPLAYLIST *     pDisplayList;
	T_IMGBROWSE         ImgBrowse;
//stage property
    T_RECT              StageRect;                    //thumbnail image show area   
    T_S32               nPagesNum;
    T_S32               nCurPageIndex;

    T_S32               nTotalPicNum;           //pic numble

    T_COLOR             StageBgColor;

    //thumbnail item's property
    T_UI_ITEM_ATTR      stctUIItem;

    T_THUMBNAIL_ITEM    preItemFocused;
    T_THUMBNAIL_ITEM    CurItemFocused;
	T_THUMBNAIL_ITEM    SuspendItemFocused;

    T_S32               nRefreshFlag;

	THUMBSDB_HANDLE     hThumb;
	T_BOOL				bGoNext;
	T_MSGBOX            msgbox;
	T_U8				mode;
	T_U8				itemNumPreRow;
	T_U8				itemNumPreCol;

	T_RES_ICON			lArrow;
	T_RES_ICON			rArrow;

	T_RES_ICON			tool;		// Member pic Is NULL
	T_RES_ICON			setIcon;	// Member buf Is NULL
	T_RES_ICON			playIcon;	// Member buf Is NULL
	T_RES_ICON			modeIcon;	// Member buf Is NULL
	
	T_RES_ICON			info;	// Member pic Is NULL

	T_BOOL				bSetSlide;
	T_pCDATA			setSlideBkPic;
	T_pCDATA			BlueBarPic;
	T_pCDATA			GrayBarPic;
	T_pCDATA			BlockPic;
	T_POS				BlockCenter;

	T_BOOL				bGoSlide;
	T_pDATA				backupBuf;
	T_U32				showStep;
	T_SLIPMGR 			*pSlipMgr;
	T_U32				emptyItemNum;
	T_BOOL				bChangeFocus;
}T_THUMBNAIL_PARAM;

static T_THUMBNAIL_PARAM  *pThumbNail = AK_NULL;

static T_S32 ThumbNail_GetPicNum(T_DISPLAYLIST * pDisplayList);
static T_VOID ThumbNail_SetRefreshFlag(T_S32 nFlag);
static T_VOID ThumbNail_ClearRefreshFlag(T_THUMBNAIL_PARAM  *pThumbNail);
static T_VOID ThumbNail_MoveFocus(T_THUMBNAIL_PARAM  *pThumbNail, T_S32    nDirect);
static T_BOOL ThumbNail_IsPicFile(T_U16  *pFullName);
static T_BOOL ThumbNail_PicAcoordWithPageIndx(T_S32   nPicIndex, T_S32    nPageIndex);
static T_BOOL ThumbNail_ChangePages(T_THUMBNAIL_PARAM  *pThumbNail, T_PAGE_ACTION enmAction, T_S32 num);
static T_BOOL ThumbNail_ChangeFocus(T_THUMBNAIL_PARAM  *pThumbNail, T_S32 nPicIndex);
static T_VOID ThumbNail_RestFileList(T_THUMBNAIL_PARAM  *pThumbNail);
static T_BOOL ThumbNail_ResetData(T_THUMBNAIL_PARAM * pThumbNail);
static T_S32 ThumbNail_FreeData(T_THUMBNAIL_PARAM  *pThumbNail);
static T_BOOL ThumbNail_ReCalcuPos(T_THUMBNAIL_PARAM  *pThumbNail,T_THUMBNAIL_ITEM * pThumbItem);
static T_BOOL ThumbNail_Show(T_U32 step);
static T_BOOL ThumbNail_ShowSetSlide(T_VOID);
static T_VOID ThumbNail_InitRes(T_BOOL bReset);
static T_BOOL ThumbNail_CreatSlipMgr(T_VOID);
static T_BOOL ThumbNail_CheckSlipFocus(T_VOID);
//static T_BOOL ThumbNail_ChkPicIdxValid(T_S32 nPicIndex,  T_S32    nTotalPicNum,T_S32 nPageIndx, T_S32    nPagesNum);
//static T_VOID ThumbNail_LocatePos(T_THUMBNAIL_ITEM *pItem, T_S32 nPicIndex, T_THUMBNAIL_PARAM  *pThumbNail);


/*******************************************************************************************************/


static T_BOOL ThumbNail_ShowFocus(T_THUMBNAIL_PARAM  *pThumbNail)
{
	T_U32	focusId = 0;
	T_U32	i = 0;
	T_S32	index = 0;
	T_U8*	pbuf = AK_NULL;
	T_U32 	width = 0;
	T_U32 	height = 0;
	T_RECT 	rect;
	T_COLOR	focusColor = 0;
	T_COLOR	stageColor = 0;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
    
    if (pThumbNail->nTotalPicNum <= 0)
		return AK_FALSE;
         
    if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
    {
		focusColor = FOCUS_COLOR_ONEROW;
		stageColor = STAGE_COLOR_ONEROW;
	}
	else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
	{
		focusColor = FOCUS_COLOR_FULLSCR;
		stageColor = STAGE_COLOR_FULLSCR;
	}

	//clear focus
	for (i=0; i<SlipMgr_GetItemNum(pThumbNail->pSlipMgr); i++)
	{
		SlipMgr_GetItemBufByIndex(pThumbNail->pSlipMgr, i, &pbuf, &width, &height);

		RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
			(T_POS)(pThumbNail->stctUIItem.nItemY_Interval - 2), THUMBNAIL_WIDTH + 4, 2);
	    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, stageColor, RGB565);

		RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
			(T_POS)(pThumbNail->stctUIItem.nItemY_Interval), 2, THUMBNAIL_HEIGHT);
	    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, stageColor, RGB565);

		RectInit(&rect, (T_POS)(THUMBNAIL_WIDTH + pThumbNail->stctUIItem.nItemX_Interval), 
			(T_POS)(pThumbNail->stctUIItem.nItemY_Interval), 2, THUMBNAIL_HEIGHT);
	    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, stageColor, RGB565);

		RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
			(T_POS)(pThumbNail->stctUIItem.nItemY_Interval + THUMBNAIL_HEIGHT), THUMBNAIL_WIDTH + 4, 2);
	    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, stageColor, RGB565);
	}
	
    //show focus
	focusId = pThumbNail->CurItemFocused.ItemIndex;
    index = SlipMgr_GetIndexById(pThumbNail->pSlipMgr, focusId);

	if (-1 == index)
	{
		return AK_FALSE;
	}
	
	SlipMgr_GetItemBufByIndex(pThumbNail->pSlipMgr, index, &pbuf, &width, &height);

	RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
		(T_POS)(pThumbNail->stctUIItem.nItemY_Interval - 2), THUMBNAIL_WIDTH + 4, 2);
    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, focusColor, RGB565);

	RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
		(T_POS)(pThumbNail->stctUIItem.nItemY_Interval), 2, THUMBNAIL_HEIGHT);
    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, focusColor, RGB565);

	RectInit(&rect, (T_POS)(THUMBNAIL_WIDTH + pThumbNail->stctUIItem.nItemX_Interval), 
		(T_POS)(pThumbNail->stctUIItem.nItemY_Interval), 2, THUMBNAIL_HEIGHT);
    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, focusColor, RGB565);

	RectInit(&rect, (T_POS)(pThumbNail->stctUIItem.nItemX_Interval - 2), 
		(T_POS)(pThumbNail->stctUIItem.nItemY_Interval + THUMBNAIL_HEIGHT), THUMBNAIL_WIDTH + 4, 2);
    Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, focusColor, RGB565);
	
	return AK_TRUE;    
}


/*
change pages,
enmAction: 
            add     num(the step to add)
            sub     num( the step to sub)
            setpos  num(the specific page index ),
*/
static T_BOOL ThumbNail_ChangePages(T_THUMBNAIL_PARAM  *pThumbNail, T_PAGE_ACTION enmAction, T_S32 num)
{
    T_S32   nTotalPageNum = -1;
    T_S32   bFunRet = AK_FALSE;
    
    AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
    
    if (num < 0)
    {
        return AK_FALSE;
    }
    
    if (pThumbNail->nTotalPicNum < pThumbNail->itemNumPreRow * pThumbNail->itemNumPreCol)
    {
        nTotalPageNum = 1;
    }
    else
    {
        nTotalPageNum = (pThumbNail->nTotalPicNum + pThumbNail->itemNumPreRow - 1) /pThumbNail->itemNumPreRow - pThumbNail->itemNumPreCol + 1;
    }

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "ThumbNail_ChangePages enmAction=%d, num  =%d,pThumbNail->nCurPageIndex=%d, nTotalPageNum=%d",
        enmAction, num ,pThumbNail->nCurPageIndex, nTotalPageNum);
#endif
        
    switch(enmAction)
    {
    case ADD_ACTION:
        if (pThumbNail->nCurPageIndex < nTotalPageNum - 1)
        {
            if(pThumbNail->nCurPageIndex + num <= nTotalPageNum - 1)
            {
                pThumbNail->nCurPageIndex += num;
            }
            else
            {
                pThumbNail->nCurPageIndex = nTotalPageNum - 1;
            }
			
            bFunRet = AK_TRUE;                    
        }               
        break;
		
    case SUB_ACTION:
        if (pThumbNail->nCurPageIndex  > 0)
        {
            if(pThumbNail->nCurPageIndex - num >=0)
            {
                pThumbNail->nCurPageIndex -= num;
            }
            else
            {
                pThumbNail->nCurPageIndex = 0;
            }
			
            bFunRet = AK_TRUE;
        }
        break;
		
    case SETPOS_ACTION:
        if (pThumbNail->nCurPageIndex == num)
        {                                        
        }
        else if (num >= 0 && num < nTotalPageNum)
        {
            pThumbNail->nCurPageIndex = num;
            bFunRet = AK_TRUE;
        }
        break;
		
    default:
         break;
    }

    if (bFunRet)
    {
        ThumbNail_SetRefreshFlag(REFRESH_ALL);
	}

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "22ThumbNail_ChangePages enmAction=%d, num  =%d,pThumbNail->nCurPageIndex=%d",
    	enmAction, num ,pThumbNail->nCurPageIndex);
#endif
	
    return AK_TRUE;
}


static T_BOOL ThumbNail_ReCalcuPos(T_THUMBNAIL_PARAM  *pThumbNail,T_THUMBNAIL_ITEM * pThumbItem)
{
    T_S32   nRowIndex = -1;
    T_S32   nColumnIndex = -1;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pThumbItem, "pThumbItem Is Invalid", AK_FALSE);
	
    if (pThumbNail->nTotalPicNum > 0
        && pThumbItem->ItemIndex >= 0
        && pThumbNail->nTotalPicNum > pThumbItem->ItemIndex
        && ThumbNail_PicAcoordWithPageIndx(pThumbItem->ItemIndex, pThumbNail->nCurPageIndex))
    {
        nRowIndex = (pThumbItem->ItemIndex - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) / pThumbNail->itemNumPreRow;
        nColumnIndex = (pThumbItem->ItemIndex - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) % pThumbNail->itemNumPreRow;
            
        pThumbItem->rctPos.top = (T_POS)(pThumbNail->StageRect.top + nRowIndex * pThumbNail->stctUIItem.nItemHeight);
        pThumbItem->rctPos.left = (T_POS)(pThumbNail->StageRect.left + nColumnIndex * pThumbNail->stctUIItem.nItemWidth);
        pThumbItem->rctPos.height = (T_LEN)(pThumbNail->stctUIItem.nItemHeight);
        pThumbItem->rctPos.width = (T_LEN)(pThumbNail->stctUIItem.nItemWidth);                              
        ThumbNail_SetRefreshFlag(REFRESH_FOCUSE_ITEM);
    }
    	
    return AK_TRUE;
}


//T_S32 nPicIndex : new pic index to set
static T_BOOL ThumbNail_ChangeFocus(T_THUMBNAIL_PARAM  *pThumbNail, T_S32 nPicIndex )
{
    //the  row index and column index  relatively in current page 
    T_S32   nRowIndex = -1;
    T_S32   nColumnIndex = -1;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
        
    if (nPicIndex < 0 
		|| pThumbNail->CurItemFocused.ItemIndex == nPicIndex)
    {
        return AK_FALSE;
    }

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "ThumbNail_ChangeFocus nPicIndex=%d, pThumbNail->CurItemFocused.ItemIndex=%d, pThumbNail->nCurPageIndex=%d, pThumbNail->nTotalPicNum=%d", nPicIndex,pThumbNail->CurItemFocused.ItemIndex,pThumbNail->nCurPageIndex,pThumbNail->nTotalPicNum);
#endif    
    
    if (pThumbNail->nTotalPicNum > nPicIndex
		&& nPicIndex >= 0)
    {
        pThumbNail->CurItemFocused.ItemIndex = nPicIndex;

        if (ThumbNail_PicAcoordWithPageIndx(nPicIndex, pThumbNail->nCurPageIndex))
        {
			//pic focus in the same page, show focus
            nRowIndex = (pThumbNail->CurItemFocused.ItemIndex  - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) / pThumbNail->itemNumPreRow;
            nColumnIndex = (pThumbNail->CurItemFocused.ItemIndex  - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) % pThumbNail->itemNumPreRow;
                
            pThumbNail->CurItemFocused.rctPos.top = (T_POS)(pThumbNail->StageRect.top + nRowIndex * pThumbNail->stctUIItem.nItemHeight);
            pThumbNail->CurItemFocused.rctPos.left = (T_POS)(pThumbNail->StageRect.left + nColumnIndex * pThumbNail->stctUIItem.nItemWidth);
            pThumbNail->CurItemFocused.rctPos.height = (T_LEN)(pThumbNail->stctUIItem.nItemHeight);
            pThumbNail->CurItemFocused.rctPos.width = (T_LEN)(pThumbNail->stctUIItem.nItemWidth);                              
            ThumbNail_SetRefreshFlag(REFRESH_FOCUSE_ITEM);
        }
        else
        {
            //pic focus not in the same page , do nothing except show pages
        }
        
        //change pre focus position which need to clear
        if (ThumbNail_PicAcoordWithPageIndx(pThumbNail->preItemFocused.ItemIndex, pThumbNail->nCurPageIndex))
        {
            nRowIndex = (pThumbNail->preItemFocused.ItemIndex  - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) / pThumbNail->itemNumPreRow;
            nColumnIndex = (pThumbNail->preItemFocused.ItemIndex  - pThumbNail->nCurPageIndex * pThumbNail->itemNumPreRow ) % pThumbNail->itemNumPreRow;
                
            pThumbNail->preItemFocused.rctPos.top = (T_POS)(pThumbNail->StageRect.top + nRowIndex * pThumbNail->stctUIItem.nItemHeight);
            pThumbNail->preItemFocused.rctPos.left = (T_POS)(pThumbNail->StageRect.left + nColumnIndex * pThumbNail->stctUIItem.nItemWidth);
            pThumbNail->preItemFocused.rctPos.height = (T_LEN)(pThumbNail->stctUIItem.nItemHeight);
            pThumbNail->preItemFocused.rctPos.width = (T_LEN)(pThumbNail->stctUIItem.nItemWidth);                              
            ThumbNail_SetRefreshFlag(REFRESH_FOCUSE_ITEM);
        }

#ifdef THUMB_Debug
        Fwl_Print(C3, M_IMAGE, "222ThumbNail_ChangeFocus nPicIndex=%d, pThumbNail->CurItemFocused.ItemIndex=%d", nPicIndex,pThumbNail->CurItemFocused.ItemIndex);
#endif
    }
	
    return AK_TRUE;
}

//check whether the page index of  pic is the same with the nPageIndex
static T_BOOL ThumbNail_PicAcoordWithPageIndx(T_S32   nPicIndex, T_S32    nPageIndex)
{
    T_BOOL  bRet = AK_FALSE;
    T_S32   upBound = -1;
    T_S32   lowBound = -1;

    if (nPicIndex < 0 || nPageIndex < 0)
    {
        Fwl_Print(C3, M_IMAGE, "input param invalidate nPicIndex=%d,nPageIndex=%d",nPicIndex, nPageIndex);
        return bRet;
    }

    upBound = pThumbNail->itemNumPreRow * nPageIndex;
    lowBound = upBound +   pThumbNail->itemNumPreRow * pThumbNail->itemNumPreCol - 1;

    if (nPicIndex >= upBound && nPicIndex <= lowBound)
    {
        bRet = AK_TRUE;
    }
    else
    {
        bRet = AK_FALSE;
    }
    return bRet;  
}


static T_VOID ThumbNail_MoveFocus(T_THUMBNAIL_PARAM  *pThumbNail, T_S32    nDirect)
{
    //move focus and check whether the page need to change
    T_S32   nCurPicIndex = -1;   
    T_S32   nRowIndex = -1;  
    T_S32   nColumnIdx =-1;
    T_S32   nRowNum = 0;

	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
	
    if (pThumbNail->nTotalPicNum <= 0)
		return;
             
    nCurPicIndex 	= pThumbNail->CurItemFocused.ItemIndex;
    nRowIndex 		= (nCurPicIndex) / pThumbNail->itemNumPreRow; 
    nColumnIdx 		= pThumbNail->CurItemFocused.ItemIndex - nRowIndex * pThumbNail->itemNumPreRow;
    nRowNum 		= (pThumbNail->nTotalPicNum + pThumbNail->itemNumPreRow -1) / pThumbNail->itemNumPreRow;

#ifdef THUMB_Debug
	Fwl_Print(C3, M_IMAGE, "ThumbNail_MoveFocus nDirect=%d, pThumbNail->preItemFocused_id =%d, pThumbNail->CurItemFocused_id =%d,nRowIndex=%d, nColumnIdx=%d,curpage =%d,pages=%d, cuPageId =%d, totalPic =%d, nRowNum=%d",
	nDirect, pThumbNail->preItemFocused.ItemIndex, pThumbNail->CurItemFocused.ItemIndex,nRowIndex,nColumnIdx,pThumbNail->nCurPageIndex,
	pThumbNail->nPagesNum, pThumbNail->nCurPageIndex, pThumbNail->nTotalPicNum, nRowNum);
#endif
	pThumbNail->preItemFocused = pThumbNail->CurItemFocused;

	switch (nDirect)
	{
	case THUMBNAIL_DIRECTION_UP:
	    if(nRowIndex > 0)
	    {
	       	if (!ThumbNail_PicAcoordWithPageIndx(nCurPicIndex - pThumbNail->itemNumPreRow, pThumbNail->nCurPageIndex))
	       	{   
				//change the pages,move the focus       T_PAGE_ACTION
	            ThumbNail_ChangePages(pThumbNail,SUB_ACTION, 1);
	       	}
	       
	       	ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex - pThumbNail->itemNumPreRow);                       
	    }   
		else
		{
			if (nCurPicIndex + pThumbNail->itemNumPreRow * (nRowNum - 1) > pThumbNail->nTotalPicNum - 1)
			{
				ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, nRowNum - pThumbNail->itemNumPreCol);                
	    		ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex + pThumbNail->itemNumPreRow * (nRowNum - 2));
			}
			else
			{
				ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, nRowNum - pThumbNail->itemNumPreCol);                
	    		ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex + pThumbNail->itemNumPreRow * (nRowNum - 1));
			}
			
			pThumbNail->SuspendItemFocused = pThumbNail->preItemFocused;
			ThumbNail_CheckSlipFocus();
		}
	    break;

	case THUMBNAIL_DIRECTION_DOWN: 
	    if (nRowIndex < nRowNum - 1)
		{
			if (nCurPicIndex + pThumbNail->itemNumPreRow  <= pThumbNail->nTotalPicNum - 1)
	        {
	            if (!ThumbNail_PicAcoordWithPageIndx(nCurPicIndex + pThumbNail->itemNumPreRow, pThumbNail->nCurPageIndex))
	            {
					//change the pages,move the focus       T_PAGE_ACTION
	                 ThumbNail_ChangePages(pThumbNail, ADD_ACTION, 1);
	            }
	            
	            ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex + pThumbNail->itemNumPreRow);                       
	        }
			else
			{
				ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, 0);                
	    		ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex - pThumbNail->itemNumPreRow * (nRowNum - 2));

				pThumbNail->SuspendItemFocused = pThumbNail->preItemFocused;
				ThumbNail_CheckSlipFocus();
			}
	    }
		else
		{
			ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, 0);                
	    	ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex - pThumbNail->itemNumPreRow * (nRowNum - 1));

			pThumbNail->SuspendItemFocused = pThumbNail->preItemFocused;
			ThumbNail_CheckSlipFocus();
		}
	    break;

	case THUMBNAIL_DIRECTION_LEFT:
	    if (nCurPicIndex > 0)
	    {
	        if (!ThumbNail_PicAcoordWithPageIndx(nCurPicIndex - 1, pThumbNail->nCurPageIndex))
	        {
				//change the pages,move the focus       T_PAGE_ACTION
	          	ThumbNail_ChangePages(pThumbNail,SUB_ACTION, 1);
	        }
	         
	        ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex  - 1 );                                                 
	    }
		else
		{
			ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, nRowNum - pThumbNail->itemNumPreCol);                
			ThumbNail_ChangeFocus(pThumbNail, pThumbNail->nTotalPicNum - 1);

			pThumbNail->SuspendItemFocused = pThumbNail->preItemFocused;
			ThumbNail_CheckSlipFocus();
		}
	    break;
	
	case THUMBNAIL_DIRECTION_RIGHT:
	    if (nCurPicIndex < pThumbNail->nTotalPicNum - 1)
	    {
	        if (!ThumbNail_PicAcoordWithPageIndx(nCurPicIndex + 1, pThumbNail->nCurPageIndex))
	        {
				//change the pages,move the focus       T_PAGE_ACTION
	            ThumbNail_ChangePages(pThumbNail, ADD_ACTION, 1);
	        }
	        
	        ThumbNail_ChangeFocus(pThumbNail, nCurPicIndex  + 1 );                                                
	    }
		else
		{
			ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, 0);                
			ThumbNail_ChangeFocus(pThumbNail, 0);

			pThumbNail->SuspendItemFocused = pThumbNail->preItemFocused;
			ThumbNail_CheckSlipFocus();
		}
	    break;

	default:
	    break;
	}

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "222ThumbNail_MoveFocus nDirect=%d, pThumbNail->preItemFocused_id =%d, pThumbNail->CurItemFocused_id =%d,nRowIndex=%d, nColumnIdx=%d,curpage =%d,pages=%d, cuPageId =%d, totalPic =%d, nRowNum=%d",
    nDirect, pThumbNail->preItemFocused.ItemIndex, pThumbNail->CurItemFocused.ItemIndex,nRowIndex,nColumnIdx,pThumbNail->nCurPageIndex,
    pThumbNail->nPagesNum, pThumbNail->nCurPageIndex, pThumbNail->nTotalPicNum, nRowNum);
#endif     
}


static T_VOID ThumbNail_FreeRes(T_VOID)
{	
	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		if (AK_NULL != pThumbNail->lArrow.buf)
		{
			pThumbNail->lArrow.buf = Fwl_Free(pThumbNail->lArrow.buf);
		}
		
		if (AK_NULL != pThumbNail->rArrow.buf)
		{
			pThumbNail->rArrow.buf = Fwl_Free(pThumbNail->rArrow.buf);
		}
		
		if (AK_NULL != pThumbNail->tool.buf)
		{
			pThumbNail->tool.buf = Fwl_Free(pThumbNail->tool.buf);
		}
		
		if (AK_NULL != pThumbNail->info.buf)
		{
			pThumbNail->info.buf = Fwl_Free(pThumbNail->info.buf);
		}
		
		if (AK_NULL != pThumbNail->backupBuf)
		{
			pThumbNail->backupBuf = Fwl_Free(pThumbNail->backupBuf);
		}
	}
}

static T_BOOL ThumbNail_ChangeMode(T_THUMBNAIL_PARAM  *pThumbNail)
{
    AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		ThumbNail_FreeRes();
		ImgBrowse_Free(&pThumbNail->ImgBrowse);

		pThumbNail->mode = THUMBNAIL_MODE_FULLSCR;
		pThumbNail->bSetSlide = AK_FALSE;
		
		Fwl_Print(C3, M_THUMB, "Switch to FULL SCR\n");
	}
	else
	{
		pThumbNail->mode = THUMBNAIL_MODE_ONEROW;
		ImgBrowse_Init(&pThumbNail->ImgBrowse);
		ImgBrowse_SetDisMode(&pThumbNail->ImgBrowse,IMG_PREVIEW);
		ImgBrowse_Open(&pThumbNail->ImgBrowse, pThumbNail->pDisplayList);
		
		Fwl_Print(C3, M_THUMB, "Switch to ONE ROW\n");
	}
	
	ThumbNail_InitRes(AK_TRUE);
	ThumbNail_CreatSlipMgr();

	ThumbNail_ResetData(pThumbNail);
	ThumbNail_ReCalcuPos(pThumbNail, &pThumbNail->CurItemFocused);
    ThumbNail_ReCalcuPos(pThumbNail, &pThumbNail->preItemFocused);
	ThumbNail_SetRefreshFlag(REFRESH_ALL);

	return AK_TRUE;
}

static T_BOOL ThumbNail_SetSlideTime(T_U32 x)
{
	T_U32 i = 0;
	T_U32 level_width = SETSLIDEBAR_WIDTH / 7;
	T_S32 level = -1;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	
	if (THUMBNAIL_MODE_ONEROW != pThumbNail->mode
		|| !pThumbNail->bSetSlide)
    {
        return AK_FALSE;
    }

	for (i=0; i<8; i++)
	{
		if (x >= level_width * i
			&& x < level_width * (i+1))
		{
			level = i;
			break;
		}
	}

	if (-1 != level)
	{
		if (6 == level)
		{
			gs.ImgSlideInterval = 10;
		}
		else if (7 == level)
		{
			gs.ImgSlideInterval = 20;
		}
		else
		{
			gs.ImgSlideInterval = (T_U8)level;
		}

		return AK_TRUE;
	}

	return AK_FALSE;
}


static T_BOOL ThumbNail_SetSlideBlock(T_VOID)
{
	T_U32 level_width = SETSLIDEBAR_WIDTH / 7;
	T_U8 level;
	
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (10 == gs.ImgSlideInterval)
	{
		level = 6;
	}
	else if (20 == gs.ImgSlideInterval)
	{
		level = 7;
	}
	else
	{
		level = gs.ImgSlideInterval;
	}

	if (7 == level)
	{
		pThumbNail->BlockCenter = (T_POS)(SETSLIDEBAR_LEFT + SETSLIDEBAR_WIDTH);
	}
	else
	{
		pThumbNail->BlockCenter = (T_POS)(SETSLIDEBAR_LEFT + level * level_width);
	}

	return AK_FALSE;
}


//to deal with the touch action on the screen
static T_VOID ThumbNail_HitButton_Handle(T_THUMBNAIL_PARAM  *pThumbNail, T_TOUCHSCR_ACTION action, T_MMI_KEYPAD *pPhyKey, T_POS posX, T_POS posY)
{
    T_RECT  rect;

	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
	AK_ASSERT_PTR_VOID(pPhyKey, "pPhyKey Is Invalid");
	
	if (eTOUCHSCR_DOWN == action
		&& pThumbNail->bSetSlide 
		&& THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		RectInit(&rect, (T_POS)SETSLIDEBAR_LEFT, (T_POS)SETSLIDE_TOP, (T_LEN)SETSLIDEBAR_WIDTH, (T_LEN)SETSLIDE_HEIGHT);

		if (PointInRect(&rect, posX, posY))
		{
			pThumbNail->BlockCenter = posX;
			ThumbNail_SetSlideTime((T_U32)(pThumbNail->BlockCenter - SETSLIDEBAR_LEFT));
			ThumbNail_SetRefreshFlag(REFRESH_SETSLIDE);
		}
	}
	
	if (eTOUCHSCR_UP == action)
	{
		if (pThumbNail->bSetSlide && (THUMBNAIL_MODE_ONEROW == pThumbNail->mode))
		{
			RectInit(&rect, (T_POS)SETSLIDEBAR_LEFT, (T_POS)SETSLIDE_TOP, (T_LEN)SETSLIDEBAR_WIDTH, (T_LEN)SETSLIDE_HEIGHT);

			if(PointInRect(&rect, posX, posY))
			{
				pThumbNail->BlockCenter = posX;
				ThumbNail_SetSlideTime((T_U32)(pThumbNail->BlockCenter - SETSLIDEBAR_LEFT));
				ThumbNail_SetRefreshFlag(REFRESH_SETSLIDE);
			}
		}
		
		//get the rect of cancel button
	    rect = TopBar_GetRectofCancelButton();
		
	    if (PointInRect(&rect, posX, posY))
	    {
	        pPhyKey->keyID = kbCLEAR;
	        pPhyKey->pressType = PRESS_SHORT;
	        return;
	    }

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode
			&& SLIPMSG_STA_STOP != SlipMgr_GetCurStatus(pThumbNail->pSlipMgr))
		{
			return;
		}

		//get the rect of menu button
	    TopBar_GetRectofMenuButton(&rect);
		
	    if (PointInRect(&rect, posX, posY))
	    {
	        pPhyKey->keyID = kbMENU;
	        pPhyKey->pressType = PRESS_SHORT;
	        return;
	    }

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			rect = pThumbNail->modeIcon.rect;

			if (PointInRect(&rect, posX, posY))
			{
				pPhyKey->keyID = kbSWA;
				pPhyKey->pressType = PRESS_SHORT;
				return;
			}

			rect = pThumbNail->playIcon.rect;

			if (PointInRect(&rect, posX, posY))
			{
				pPhyKey->keyID = kbOK;
				pPhyKey->pressType = PRESS_LONG;
				return;
			}

			rect = pThumbNail->setIcon.rect;

			if (PointInRect(&rect, posX, posY))
			{
				pPhyKey->keyID = kbUP;
				pPhyKey->pressType = PRESS_SHORT;
				return;
			}

			//get the rect of lArrowPic button
		    rect = pThumbNail->lArrow.rect;
		    if (PointInRect(&rect, posX, posY)
				&& !pThumbNail->bSetSlide)
		    {
		        pPhyKey->keyID = kbLEFT;
		        pPhyKey->pressType = PRESS_SHORT;
				
		        return;
		    }

			//get the rect of RArrowPic button
		    rect = pThumbNail->rArrow.rect;
		    if (PointInRect(&rect, posX, posY)
				&& !pThumbNail->bSetSlide)
		    {
		        pPhyKey->keyID = kbRIGHT;
		        pPhyKey->pressType = PRESS_SHORT;
				
		        return;
		    }

			RectInit(&rect, 
				0, 
				(T_POS)(pThumbNail->info.rect.top + pThumbNail->info.rect.height), 
				Fwl_GetLcdWidth(), 
				(T_LEN)(pThumbNail->StageRect.top - (pThumbNail->info.rect.top + pThumbNail->info.rect.height)));

			if (PointInRect(&rect, posX, posY))
			{
				pPhyKey->keyID = kbOK;
				pPhyKey->pressType = PRESS_SHORT;
				return;
			}
		}
	}

}

//to deal with the user key action
static T_S32 ThumbNail_UserKey_Handle(T_THUMBNAIL_PARAM  *pThumbNail,T_MMI_KEYPAD * pPhyKey)
{
    T_eBACK_STATE   enmRet = eStay;

	AK_ASSERT_PTR(pPhyKey, "pThumbNail Is Invalid", eStay);
    
	if (SLIPMSG_STA_STOP != SlipMgr_GetCurStatus(pThumbNail->pSlipMgr)
		&& kbCLEAR != pPhyKey->keyID)
	{
		ThumbNail_SetRefreshFlag(REFRESH_MOVE_BUF);
		return enmRet;
	}
	
    // to complete
    switch(pPhyKey->keyID)
    {
    case kbOK:
        if (pPhyKey->pressType == PRESS_SHORT)
		{
			if (!pThumbNail->bSetSlide)
        	{
	            enmRet = eNext;
				pThumbNail->bGoSlide = AK_FALSE;
        	}
        }
		else
		{
			enmRet = eNext;
			pThumbNail->bGoSlide = AK_TRUE;
			pThumbNail->bSetSlide = AK_FALSE;
		}
        break;

    case kbCLEAR:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
            enmRet = eReturn;
        }
		else
		{
			enmRet = eHome;
		}
        break;
		
	case kbMENU:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
            enmRet = eMenu;
            pThumbNail->bSetSlide = AK_FALSE;
        }
        break;
		
     case kbUP:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
        	if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
        	{
            	ThumbNail_MoveFocus(pThumbNail , THUMBNAIL_DIRECTION_UP);
        	}
			else
			{
				if (!pThumbNail->bSetSlide)
				{
					pThumbNail->bSetSlide = AK_TRUE;
					ThumbNail_SetSlideBlock();
					ThumbNail_SetRefreshFlag(REFRESH_SETSLIDE);
				}
				else
				{
					pThumbNail->bSetSlide = AK_FALSE;
					ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
					ThumbNail_SetRefreshFlag(REFRESH_ALL);
				}
			}
        }
        break;

     case kbDOWN:
        if (pPhyKey->pressType == PRESS_SHORT
			&& THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
        {
            ThumbNail_MoveFocus(pThumbNail , THUMBNAIL_DIRECTION_DOWN);
        } 
		
        break;
		
     case kbLEFT:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
        	if (!pThumbNail->bSetSlide)
        	{
            	ThumbNail_MoveFocus(pThumbNail , THUMBNAIL_DIRECTION_LEFT);
        	}
			else
			{
				if (20 == gs.ImgSlideInterval)
				{
					gs.ImgSlideInterval = 10;
				}
				else if (10 == gs.ImgSlideInterval)
				{
					gs.ImgSlideInterval = 5;
				}
				else if ((gs.ImgSlideInterval <= 5) && (gs.ImgSlideInterval > 0))
				{
					gs.ImgSlideInterval--;
				}
				
				ThumbNail_SetSlideBlock();
				ThumbNail_SetRefreshFlag(REFRESH_SETSLIDE);
			}
        }            
        break;
		
     case kbRIGHT:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
        	if (!pThumbNail->bSetSlide)
        	{
            	ThumbNail_MoveFocus(pThumbNail , THUMBNAIL_DIRECTION_RIGHT);
        	}
			else
			{
				if (10 == gs.ImgSlideInterval)
				{
					gs.ImgSlideInterval = 20;
				}
				else if (5 == gs.ImgSlideInterval)
				{
					gs.ImgSlideInterval = 10;
				}
				else if (gs.ImgSlideInterval < 5)
				{
					gs.ImgSlideInterval++;
				}
				
				ThumbNail_SetSlideBlock();
				ThumbNail_SetRefreshFlag(REFRESH_SETSLIDE);
			}
        }
        break;
		
	case kbSWA:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
        	ThumbNail_RestFileList(pThumbNail);
            ThumbNail_ChangeMode(pThumbNail);	
        }
        break;
		
    default:
        break;            
    }

    return enmRet;

}

#if 0
static T_BOOL ThumbNail_ChkPicIdxValid(T_S32 nPicIndex,  T_S32    nTotalPicNum,T_S32 nPageIndx, T_S32    nPagesNum)
{
    T_S32      nTmpPages = 0;
    
    if (nTotalPicNum <= 0 || nPagesNum <= 0 || nPicIndex < 0 || nPageIndx < 0)
    {
        return AK_FALSE;
    }
    else
    {
        //check the relation between pic number and pages
        if (nTotalPicNum > pThumbNail->itemNumPreRow * pThumbNail->itemNumPreCol)
        {
            nTmpPages = (nTotalPicNum + pThumbNail->itemNumPreRow - 1 ) / pThumbNail->itemNumPreRow - pThumbNail->itemNumPreCol + 1;
        }
        else
        {
            nTmpPages = 1;
        }

        if (nTmpPages != nPagesNum)
        {
            return AK_FALSE;
        }

        // check the pic index and  the total pic num
        if (nPicIndex >= nTotalPicNum)
        {
            return AK_FALSE;
        }

        if (nPageIndx >= nPagesNum)
        {
            return AK_FALSE;
        }
        
    }

    return AK_TRUE;

}


// to set the location in the ui by the item index
static T_VOID ThumbNail_LocatePos(T_THUMBNAIL_ITEM *pItem, T_S32 nPicIndex , T_THUMBNAIL_PARAM  *pThumbNail)
{
    //to complete
    T_S32       nColumnIdx = -1;
    T_S32       nRowIndex = -1;    

	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
    
    if (!ThumbNail_ChkPicIdxValid(nPicIndex, pThumbNail->nTotalPicNum, pThumbNail->nCurPageIndex, pThumbNail->nPagesNum))
    {
        Fwl_Print(C3, M_IMAGE, "The index of pic is invalid %d, nPicIndex=%d, pagenum = %d", __LINE__, nPicIndex, pThumbNail->nPagesNum);
        nPicIndex = 0;
        return ;
    }
    else
    {
        pItem->ItemIndex = nPicIndex;

        //get the rowIndex acoording the itemIndex
        nRowIndex = (pItem->ItemIndex) / pThumbNail->itemNumPreRow;
        nColumnIdx = pItem->ItemIndex - nRowIndex * pThumbNail->itemNumPreRow;

        if (nRowIndex < pThumbNail->nCurPageIndex)
        {
            //roll up pages    
            pItem->rctPos.top = pThumbNail->StageRect.top;
            pItem->rctPos.left = (T_POS)(pThumbNail->StageRect.left + pThumbNail->stctUIItem.nItemWidth * nColumnIdx);                

            //need to reload pic
            ThumbNail_SetRefreshFlag(REFRESH_ALL);
        }
        else if (nRowIndex <= pThumbNail->nCurPageIndex + pThumbNail->itemNumPreCol -1)
        {
            //in one page,change focus
            pItem->rctPos.top = (T_POS)(pThumbNail->StageRect.top + (nRowIndex - pThumbNail->nCurPageIndex) * pThumbNail->stctUIItem.nItemHeight);                
            pItem->rctPos.left = (T_POS)(pThumbNail->StageRect.left + pThumbNail->stctUIItem.nItemWidth * nColumnIdx);                
            ThumbNail_SetRefreshFlag(REFRESH_FOCUSE_ITEM);                
        }
        else
        {
            //roll down pages
            pItem->rctPos.top = (T_POS)(pThumbNail->StageRect.top + pThumbNail->stctUIItem.nItemHeight * (pThumbNail->itemNumPreCol - 1));
            pItem->rctPos.left = (T_POS)(pThumbNail->StageRect.left + pThumbNail->stctUIItem.nItemWidth * nColumnIdx);

            //need to reload pic                
            ThumbNail_SetRefreshFlag(REFRESH_ALL);
        }
		
        pItem->rctPos.height = (T_LEN)(pThumbNail->stctUIItem.nItemHeight);
        pItem->rctPos.width =  (T_LEN)(pThumbNail->stctUIItem.nItemWidth); 
    }
}
#endif

static T_S32 ThumbNail_GetRefreshFlag(T_VOID)
{
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", REFRESH_NONE);
    
    return pThumbNail->nRefreshFlag;
}

static T_VOID ThumbNail_SetRefreshFlag(T_S32 nFlag)
{
	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
     
    pThumbNail->nRefreshFlag = nFlag;    
}

static T_VOID ThumbNail_ClearRefreshFlag(T_THUMBNAIL_PARAM  *pThumbNail)
{
    AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
    
    pThumbNail->nRefreshFlag = 0;     
}

//check whether it is a pic file or not
static T_BOOL ThumbNail_IsPicFile(T_U16  *pFullName)
{
    T_S32   nFileType = -1;
    T_BOOL  bRet = AK_FALSE;
//    T_FILE_TYPE 
    nFileType = Utl_GetFileType(pFullName);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "nFileType=%d", nFileType);
#endif 

    switch (nFileType)
    {
    case FILE_TYPE_BMP:
    case FILE_TYPE_JPG:
    case FILE_TYPE_JPEG:
    case FILE_TYPE_JPE:
    case FILE_TYPE_PNG:
    case FILE_TYPE_GIF:
        bRet = AK_TRUE;
        break;
		
    default:
        break;
    }
    return bRet ;

}

// get the total numble of the pic
static T_S32 ThumbNail_GetPicNum(T_DISPLAYLIST * pDisplayList)
{
    // to complete
    T_ICONEXPLORER_ITEM *   pItem = AK_NULL;
    T_S32                   nTotalPicNum = 0;
    T_FILE_INFO *           pFileInfo = AK_NULL;

#ifdef THUMB_Debug
    T_S8                    szStr[512];
    T_S32                   nCount = 0;
#endif 

	AK_ASSERT_PTR(pDisplayList, "pThumbNail Is Invalid", 0);
	AK_ASSERT_VAL(pDisplayList->IconExplorer.ItemQty > 0, "ItemQty Less Than 0", 0);
    
	pItem = pDisplayList->IconExplorer.pItemHead; 
	
    while (pItem != AK_NULL)
    {
#ifdef  THUMB_Debug
        nCount++;
#endif 
        if (pItem->pContent != AK_NULL )
        {
            pFileInfo = (T_FILE_INFO * )(pItem->pContent);

#ifdef THUMB_Debug
            Eng_StrUcs2Mbcs(pFileInfo->name, szStr);
            Fwl_Print(C3, M_IMAGE, "file name is %s , nCount=%d\n, pFileInfo->attrib=0x%x", szStr, nCount, pFileInfo->attrib);
#endif             
            if (!(pFileInfo->attrib & 0x10) && !(pFileInfo->attrib & 0x08) )
            {   
                if (ThumbNail_IsPicFile(pFileInfo->name))
                {
                    nTotalPicNum++;
                }                  
                else
                {
                    Fwl_Print(C3, M_IMAGE, "The file is not pic file");
                }
            }
        }
		
        pItem = pItem->pNext;
    }
	
    return nTotalPicNum;
}


static T_BOOL ThumbNail_CalcStageToLcd(T_U8*pbuf, T_U32 width, T_U32 height, T_U32 alpha, T_U32 key_alpha, T_U32 step)
{
    T_U8    *pBuffer = AK_NULL;
	T_U8    *pData = AK_NULL;
    T_S32	nDataOffset =  -1;
    T_S32	nBytesPerStageLine = -1;
    T_S32	nBytesPeLcdLine = -1;
    T_U32	i = 0;
	T_S32	j = 0;
	float	alpha_f = (float)alpha / 100;
	float	k_alpha_f = (float)key_alpha / 100;
	T_U8	key_color = (T_U8)STAGE_COLOR_ONEROW;
	T_U32	stepOffset = 0;
	T_U32	heightshow = 0;
#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
	T_U8	r_src,g_src,b_src;
	T_U8	r_dst,g_dst,b_dst;
	T_U8	k_r_src,k_g_src,k_b_src;
#endif

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (0 == step)
	{
		stepOffset = 0;
		heightshow = height;
	}
	else
	{
		stepOffset = (height + pThumbNail->tool.rect.height) / 2;
		heightshow = height - stepOffset + TOOLRECT_HEIGHT;
	}

	nDataOffset = ((pThumbNail->StageRect.top+stepOffset) * Fwl_GetLcdWidth() + pThumbNail->StageRect.left) * COLOR_SIZE;

    pBuffer = Fwl_GetDispMemory() + nDataOffset; 
    nBytesPerStageLine = width * COLOR_SIZE;
    nBytesPeLcdLine = Fwl_GetLcdWidth() * COLOR_SIZE;

	pData = pbuf;

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))

	k_r_src = key_color & 0xf8;
	k_g_src = key_color & 0xfc;
	k_b_src = key_color & 0xf8;

	for (i = 0; i < heightshow; i++)
    {
		for (j = 0; j < width; j++)
		{
			T_U16	temp = 0;
			
			temp = (*pData) | (*(pData+1)<<8);
			r_src = (T_U8)((temp>>11)<<3);
			g_src = (T_U8)((temp>>5)<<2);
			b_src = (T_U8)(temp<<3);

			temp = (*pBuffer) | (*(pBuffer+1)<<8);
			r_dst = (T_U8)((temp>>11)<<3);
			g_dst = (T_U8)((temp>>5)<<2);
			b_dst = (T_U8)(temp<<3);

			if ((k_r_src == r_src) && (k_g_src == g_src) && (k_b_src == b_src))
			{
				r_dst = (T_U8)(r_dst * (1 - k_alpha_f) + r_src * k_alpha_f);
				g_dst = (T_U8)(g_dst * (1 - k_alpha_f) + g_src * k_alpha_f);
				b_dst = (T_U8)(b_dst * (1 - k_alpha_f) + b_src * k_alpha_f);
			}
			else
			{
				r_dst = (T_U8)(r_dst * (1 - alpha_f) + r_src * alpha_f);
				g_dst = (T_U8)(g_dst * (1 - alpha_f) + g_src * alpha_f);
				b_dst = (T_U8)(b_dst * (1 - alpha_f) + b_src * alpha_f);
			}

			*pBuffer++ = (T_U8)(((b_dst & 0xf8) >> 3) | ((g_dst & 0x1c) << 3));	// b, g
	    	*pBuffer++ = (T_U8)((r_dst & 0xf8)  | ((g_dst & 0xe0) >> 5));	
			
			pData += 2;
		}

		pBuffer += nBytesPeLcdLine - nBytesPerStageLine;
    }
#else
    for (i = 0; i < heightshow; i++)
    {
		for (j = 0; j < nBytesPerStageLine; j++)
		{
			if (key_color == *pData)
			{
				*pBuffer = (T_U8)(*pBuffer * (1 - k_alpha_f) + *pData * k_alpha_f);
			}
			else
			{
				*pBuffer = (T_U8)(*pBuffer * (1 - alpha_f) + *pData * alpha_f);
			}
			
			pBuffer++;
			pData++;
		}

		pBuffer += nBytesPeLcdLine - nBytesPerStageLine;
    }
#endif
	return AK_TRUE;
}

static T_BOOL ThumbNail_CalcBufToLcd(T_pDATA buf, T_U32 alpha, T_U32 step)
{
    T_U8    *pBuffer = AK_NULL;
	T_U8    *pData = AK_NULL;
    T_S32	nDataOffset =  -1;
    T_S32	nBytesPerStageLine = -1;
    T_S32	nBytesPeLcdLine = -1;
	T_S16	height = 0;
    T_S32	i = 0;
	T_S32	j = 0;
	float	alpha_f = (float)alpha / 100;
	T_S16	stepOffset = 0;
	T_S16	heightoffset = 0;
	
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (2 == step)
	{
		return AK_TRUE;
	}
	else if (0 == step)
	{
		stepOffset = 0;
	}
	else
	{
		if (pThumbNail->info.buf == buf)
		{
			stepOffset = -TOP_BAR_HEIGHT;
		}
		else
		{
			stepOffset = (pThumbNail->StageRect.height + pThumbNail->tool.rect.height) / 2;
			heightoffset = stepOffset - TOOLRECT_HEIGHT;
		}
	}
	
	if (pThumbNail->lArrow.buf == buf)
	{
		nDataOffset = ((pThumbNail->lArrow.rect.top+stepOffset) * Fwl_GetLcdWidth() + pThumbNail->lArrow.rect.left) * COLOR_SIZE;

	    pBuffer = Fwl_GetDispMemory() + nDataOffset; 
	    nBytesPerStageLine = pThumbNail->lArrow.rect.width * COLOR_SIZE;
	    nBytesPeLcdLine = Fwl_GetLcdWidth() * COLOR_SIZE;
		height = pThumbNail->lArrow.rect.height - heightoffset;
		pData = pThumbNail->lArrow.buf;
	}
	else if (pThumbNail->rArrow.buf == buf)
	{
		nDataOffset = ((pThumbNail->rArrow.rect.top+stepOffset) * Fwl_GetLcdWidth() + pThumbNail->rArrow.rect.left) * COLOR_SIZE;

	    pBuffer = Fwl_GetDispMemory() + nDataOffset; 
	    nBytesPerStageLine = pThumbNail->rArrow.rect.width * COLOR_SIZE;
	    nBytesPeLcdLine = Fwl_GetLcdWidth() * COLOR_SIZE;
		height = pThumbNail->rArrow.rect.height - heightoffset;
		pData = pThumbNail->rArrow.buf;
	}
	else if (pThumbNail->tool.buf == buf)
	{
		nDataOffset = (pThumbNail->tool.rect.top * Fwl_GetLcdWidth() + pThumbNail->tool.rect.left) * COLOR_SIZE;

	    pBuffer = Fwl_GetDispMemory() + nDataOffset; 
	    nBytesPerStageLine = pThumbNail->tool.rect.width * COLOR_SIZE;
	    nBytesPeLcdLine = Fwl_GetLcdWidth() * COLOR_SIZE;
		height = pThumbNail->tool.rect.height;
		pData = pThumbNail->tool.buf;
	}
	else if (pThumbNail->info.buf == buf)
	{
		nDataOffset = ((pThumbNail->info.rect.top+stepOffset) * Fwl_GetLcdWidth() + pThumbNail->info.rect.left) * COLOR_SIZE;

	    pBuffer = Fwl_GetDispMemory() + nDataOffset; 
	    nBytesPerStageLine = pThumbNail->info.rect.width * COLOR_SIZE;
	    nBytesPeLcdLine = Fwl_GetLcdWidth() * COLOR_SIZE;
		height = pThumbNail->info.rect.height;
		pData = pThumbNail->info.buf;
	}
	else
	{
		return AK_FALSE;
	}

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
	for (i = 0; i < height; i++)
    {
		for (j = 0; j < nBytesPerStageLine / 2; j++)
		{
			T_U8 r_src,g_src,b_src;
			T_U8 r_dst,g_dst,b_dst;
			T_U16 temp = 0;

			temp = (*pData) | (*(pData+1)<<8);
			r_src = (T_U8)((temp>>11)<<3);
			g_src = (T_U8)((temp>>5)<<2);
			b_src = (T_U8)(temp<<3);

			temp = (*pBuffer) | (*(pBuffer+1)<<8);
			r_dst = (T_U8)((temp>>11)<<3);
			g_dst = (T_U8)((temp>>5)<<2);
			b_dst = (T_U8)(temp<<3);

			r_dst = (T_U8)(r_dst * (1 - alpha_f) + r_src * alpha_f);
			g_dst = (T_U8)(g_dst * (1 - alpha_f) + g_src * alpha_f);
			b_dst = (T_U8)(b_dst * (1 - alpha_f) + b_src * alpha_f);

			*pBuffer++ = (T_U8)(((b_dst & 0xf8) >> 3) | ((g_dst & 0x1c) << 3));	// b, g
	    	*pBuffer++ = (T_U8)((r_dst & 0xf8)  | ((g_dst & 0xe0) >> 5));	
			
			pData += 2;
		}

		pBuffer += nBytesPeLcdLine - nBytesPerStageLine;
    }
#else
    for (i = 0; i < height; i++)
    {
		for (j = 0; j < nBytesPerStageLine; j++)
		{
			*pBuffer = (T_U8)(*pBuffer * (1 - alpha_f) + *pData * alpha_f);
			pBuffer++;
			pData++;
		}

		pBuffer += nBytesPeLcdLine - nBytesPerStageLine;
    }
#endif
	return AK_TRUE;
}



static T_BOOL ThumbNail_ShowImgInfo(T_VOID)
{
	T_STR_INFO tmpstr;
	T_USTR_FILE Utmpstr;
	T_U32 strWidth = 0;
	T_RECT rect;
	T_U32 imgW = 0;
	T_U32 imgH = 0;
	
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pThumbNail->info.buf, "pThumbNail->infoBuf Is Invalid", AK_FALSE);

	RectInit(&rect, 0, 0, pThumbNail->info.rect.width, pThumbNail->info.rect.height);
	Fwl_FillSolidRectOnRGB(pThumbNail->info.buf, 
		pThumbNail->info.rect.width, 
		pThumbNail->info.rect.height, 
		&rect, pThumbNail->StageBgColor, RGB565);

	imgW = ImgBrowse_GetInImgW(&pThumbNail->ImgBrowse);
	imgH = ImgBrowse_GetInImgH(&pThumbNail->ImgBrowse);

	if ((imgW != 0) && (imgH != 0))
	{
		sprintf(tmpstr, "%ld X %ld", ImgBrowse_GetInImgW(&pThumbNail->ImgBrowse), ImgBrowse_GetInImgH(&pThumbNail->ImgBrowse));
		Eng_StrMbcs2Ucs(tmpstr, Utmpstr);
	}
	else
	{
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_AUDIOPLAYER_UNKNOW));
	}

	strWidth = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
	
	Fwl_UDispSpeciStringOnRGB(pThumbNail->info.buf,(pThumbNail->info.rect.width), (pThumbNail->info.rect.height),
				(T_POS)((pThumbNail->info.rect.width - strWidth) / 2), 0, Utmpstr, COLOR_WHITE, RGB565, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));

	return AK_TRUE;
}


static T_BOOL ThumbNail_ShowArrow(T_VOID)
{
	T_RECT rect;
	T_COLOR	bkcolor = 0;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (AK_NULL == pThumbNail->lArrow.buf
		|| AK_NULL == pThumbNail->rArrow.buf)
	{
		return AK_FALSE;
	}
	
	RectInit(&rect, 0, 0, pThumbNail->lArrow.rect.width, pThumbNail->lArrow.rect.height);
	Fwl_FillSolidRectOnRGB(pThumbNail->lArrow.buf, 
		pThumbNail->lArrow.rect.width, 
		pThumbNail->lArrow.rect.height, 
		&rect, pThumbNail->StageBgColor, RGB565);

	RectInit(&rect, 0, 0, pThumbNail->rArrow.rect.width, pThumbNail->rArrow.rect.height);
	Fwl_FillSolidRectOnRGB(pThumbNail->rArrow.buf, 
		pThumbNail->rArrow.rect.width, 
		pThumbNail->rArrow.rect.height, 
		&rect, pThumbNail->StageBgColor, RGB565);

	//if (pThumbNail->CurItemFocused.ItemIndex > 0)
	{
		Fwl_AkBmpDrawFromStringOnRGB(pThumbNail->lArrow.buf, 
						pThumbNail->lArrow.rect.width, pThumbNail->lArrow.rect.height, 
						0, 20, pThumbNail->lArrow.pic, 
                        &bkcolor, AK_FALSE, RGB565);
	}

	//if (pThumbNail->CurItemFocused.ItemIndex < pThumbNail->nTotalPicNum - 1)
	{
		Fwl_AkBmpDrawFromStringOnRGB(pThumbNail->rArrow.buf, 
						pThumbNail->rArrow.rect.width, pThumbNail->rArrow.rect.height, 
						0, 20, pThumbNail->rArrow.pic, 
                        &bkcolor, AK_FALSE, RGB565);
	}

	return AK_TRUE;
}


static T_BOOL ThumbNail_Showbottombuttons(T_VOID)
{
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pThumbNail->tool.buf, "pThumbNail->toolbuf Is Invalid", AK_FALSE);
		
	Fwl_AkBmpDrawFromStringOnRGB(pThumbNail->tool.buf, 
						pThumbNail->tool.rect.width, pThumbNail->tool.rect.height, 
						(T_POS)(pThumbNail->setIcon.rect.left+TOOL_PIC_INTERVAL_X), TOOL_PIC_INTERVAL_Y, pThumbNail->setIcon.pic, 
                        AK_NULL, AK_FALSE, RGB565);

	Fwl_AkBmpDrawFromStringOnRGB(pThumbNail->tool.buf, 
						pThumbNail->tool.rect.width, pThumbNail->tool.rect.height, 
						(T_POS)(pThumbNail->playIcon.rect.left+TOOL_PIC_INTERVAL_X), TOOL_PIC_INTERVAL_Y, pThumbNail->playIcon.pic, 
                        AK_NULL, AK_FALSE, RGB565);

	Fwl_AkBmpDrawFromStringOnRGB(pThumbNail->tool.buf, 
						pThumbNail->tool.rect.width, pThumbNail->tool.rect.height, 
						(T_POS)(pThumbNail->modeIcon.rect.left+TOOL_PIC_INTERVAL_X), TOOL_PIC_INTERVAL_Y, pThumbNail->modeIcon.pic, 
                        AK_NULL, AK_FALSE, RGB565);

	return AK_TRUE;
}

static T_BOOL ThumbNail_ShowSetSlide(T_VOID)
{
	T_RECT rect;
	T_USTR_FILE Utmpstr;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	
	if (THUMBNAIL_MODE_ONEROW != pThumbNail->mode
		|| !pThumbNail->bSetSlide)
	{
		return AK_FALSE;
	}

	Fwl_AkBmpDrawFromString(HRGB_LAYER, 
						0, (T_POS)SETSLIDE_TOP, 
						pThumbNail->setSlideBkPic,
				        AK_NULL, AK_FALSE);

	Fwl_AkBmpDrawFromString(HRGB_LAYER, 
						SETSLIDEBAR_LEFT, (T_POS)(SETSLIDE_TOP + 12), 
						pThumbNail->GrayBarPic,
				        &g_Graph.TransColor, AK_FALSE);

	RectInit(&rect, 0, 0, (T_LEN)(pThumbNail->BlockCenter - SETSLIDEBAR_LEFT), 6);

	Fwl_AkBmpDrawPartFromString(HRGB_LAYER, 
						SETSLIDEBAR_LEFT, (T_POS)(SETSLIDE_TOP + 12), 
						&rect,
						pThumbNail->BlueBarPic,
				        &g_Graph.TransColor, AK_FALSE);

	Fwl_AkBmpDrawFromString(HRGB_LAYER, 
						(T_POS)(pThumbNail->BlockCenter - SETSLIDEBLOCK_WIDTH / 2), 
						(T_POS)(SETSLIDE_TOP + 8), 
						pThumbNail->BlockPic,
				        &g_Graph.TransColor, AK_FALSE);

	if (0 == gs.ImgSlideInterval)
	{
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_PUB_CLOSE));
	}
	else if (10 == gs.ImgSlideInterval)
	{
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_TEN_SECONDS));
	}
	else if (20 == gs.ImgSlideInterval)
	{
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_TWENTY_SECONDS));
	}
	else if ((gs.ImgSlideInterval >= 1) && (gs.ImgSlideInterval <= 5))
	{
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_ONE_SECOND+gs.ImgSlideInterval-1));
	}
	
	Fwl_UDispSpeciString(HRGB_LAYER,
				SETSLIDESTR_LEFT, (T_POS)(SETSLIDE_TOP+8), Utmpstr, COLOR_WHITE, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));

	return AK_TRUE;
}

static T_BOOL ThumbNail_ShowOthers(T_U32 step)
{
	T_USTR_FILE Utmpstr;
	T_U32 width = 0;
	
	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	if (!ImgBrowse_Show(&pThumbNail->ImgBrowse))
	{
		memset(Fwl_GetDispMemory(),0,Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*COLOR_SIZE);
		
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_COM_FILE_INVALID));	
		width = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
		Fwl_UDispSpeciString(HRGB_LAYER,
				(T_POS)((Fwl_GetLcdWidth() - width)/2), (T_POS)(Fwl_GetLcdHeight()/3), Utmpstr, COLOR_WHITE, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
	}

	ThumbNail_ShowArrow();
	ThumbNail_CalcBufToLcd(pThumbNail->lArrow.buf, 50, step);
	ThumbNail_CalcBufToLcd(pThumbNail->rArrow.buf, 50, step);

	ThumbNail_ShowImgInfo();
	ThumbNail_CalcBufToLcd(pThumbNail->info.buf, 75, step);

	if (0 == step)
	{
		ThumbNail_CalcBufToLcd(pThumbNail->tool.buf, 75, step);
	}

	return AK_TRUE;
}

static T_BOOL ThumbNail_Show(T_U32 step)
{
	T_S32 nRefreshFlag = REFRESH_NONE;
	T_U8 *pbuf = AK_NULL;
	T_U32 width = 0;
	T_U32 height = 0;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	
	nRefreshFlag = ThumbNail_GetRefreshFlag();
	
	if (REFRESH_NONE == nRefreshFlag)
	{
		return AK_FALSE;
	}
	 	
	if ((REFRESH_ALL == nRefreshFlag) || (REFRESH_FOCUSE_ITEM == nRefreshFlag))
	{
		if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
		{
			ThumbNail_ShowFocus(pThumbNail);
			SlipMgr_Refresh(pThumbNail->pSlipMgr);
		}
		
		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{		
			ThumbNail_ShowOthers(step);
			memcpy(pThumbNail->backupBuf, Fwl_GetDispMemory(), MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * COLOR_SIZE);
			ThumbNail_ShowFocus(pThumbNail);
			SlipMgr_GetDisBuf(pThumbNail->pSlipMgr, &pbuf, &width, &height);
		}
	}
	else if (REFRESH_MOVE_BUF == nRefreshFlag)
	{
		if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
		{
			ThumbNail_ShowFocus(pThumbNail);
			SlipMgr_Refresh(pThumbNail->pSlipMgr);
		}
		
		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			if (0 == step)
			{
				memcpy(Fwl_GetDispMemory(), pThumbNail->backupBuf, MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * COLOR_SIZE);
			}
			else
			{
				ThumbNail_ShowOthers(step);
			}

			ThumbNail_ShowFocus(pThumbNail);
			SlipMgr_GetDisBuf(pThumbNail->pSlipMgr, &pbuf, &width, &height);
		}
	}
	else if (REFRESH_SETSLIDE == nRefreshFlag
		&& pThumbNail->bSetSlide)
	{
		ThumbNail_ShowSetSlide();
		Fwl_RefreshDisplay();
		
		return AK_TRUE;		
	}

	if (2 == step)
	{
		ThumbNail_ClearRefreshFlag(pThumbNail);
		Fwl_RefreshDisplay();	
		return AK_TRUE;
	}

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		ThumbNail_CalcStageToLcd(pbuf, width, height, 90, 50, step);
	}

	if (0 == step)
	{
		TopBar_Show(TB_REFRESH_ALL); 
	}

	if (pThumbNail->bSetSlide)
	{
		ThumbNail_ShowSetSlide();
	}

	ThumbNail_ClearRefreshFlag(pThumbNail);	
	Fwl_RefreshDisplay();	

	return AK_TRUE;
}

static T_S32 ThumbNail_FreeData(T_THUMBNAIL_PARAM  *pThumbNail)
{
    if (pThumbNail != AK_NULL
		&& THUMBSDB_INVALID_HANDLE != pThumbNail->hThumb)
    { 
    	ThumbsDB_DestroyHandle( pThumbNail->hThumb );
    	pThumbNail->hThumb = THUMBSDB_INVALID_HANDLE;
    }
	
    return AK_TRUE;
}

static T_BOOL ThumbNail_ResetData(T_THUMBNAIL_PARAM * pThumbNail)
{
    T_ICONEXPLORER_ITEM *pItem = AK_NULL;

    T_S32   nFocusPicId = 0;
    T_S32   nCount = 0;
    T_BOOL  bRet = AK_FALSE;
    T_FILE_INFO *   pFileInfo = AK_NULL;
    T_S32    nPageId = 0;

#ifdef THUMB_Debug
    T_U8        uzStr[512];
    T_U16       shStr[512];
#endif

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pThumbNail->pDisplayList, "pThumbNail->pDisplayList Is Invalid", AK_FALSE);
    
    pItem = pThumbNail->pDisplayList->IconExplorer.pItemHead;

    while (pItem != AK_NULL)
    {
        pFileInfo = (T_FILE_INFO * )(pItem->pContent);

        if ((pFileInfo->attrib & 0x10) != 0x10 )
        {
            bRet = ThumbNail_IsPicFile(pFileInfo->name);

            if (bRet)
            {
                nCount++;
            }
        }

        if (pItem == pThumbNail->pDisplayList->IconExplorer.pItemFocus)
        {
            if (nCount > 0)
            {
                nFocusPicId= nCount - 1;
            }
            else
            {
                nFocusPicId = 0;
            }
            
            break;
        }                             
        
        pItem = pItem->pNext;                
    }

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "nFocusPicId = %d", nFocusPicId);
#endif

    if (pThumbNail->nTotalPicNum > 0)
    {
        if (nFocusPicId >= pThumbNail->itemNumPreRow * pThumbNail->itemNumPreCol)
        {
            nPageId = nFocusPicId/pThumbNail->itemNumPreRow - pThumbNail->itemNumPreCol + 1;
        }
        else
        {
            nPageId = 0;
        }

#ifdef THUMB_Debug
        Fwl_Print(C3, M_IMAGE, "nPageId = %d", nPageId);
#endif
        ThumbNail_ChangePages(pThumbNail,  SETPOS_ACTION, nPageId);                
        ThumbNail_ChangeFocus(pThumbNail, nFocusPicId);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_IMAGE, "nPageindex = %d, picindex =%d", pThumbNail->nCurPageIndex, pThumbNail->CurItemFocused.ItemIndex);
#endif            
    }
    
    return AK_TRUE;
}


static T_VOID ThumbNail_RestFileList(T_THUMBNAIL_PARAM  *pThumbNail)
{
    T_ICONEXPLORER_ITEM *   pItem = AK_NULL;
    T_FILE_INFO *           pFileInfo = AK_NULL;
    T_S32                   nCount  = 0;

	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");
	
    if (0 <= pThumbNail->CurItemFocused.ItemIndex
		&& AK_NULL != pThumbNail->pDisplayList)
    {
        pItem = pThumbNail->pDisplayList->IconExplorer.pItemHead;

	    while (pItem != AK_NULL)
	    {
	        if (AK_NULL != (pFileInfo = (T_FILE_INFO*)(pItem->pContent))
			 	&& !(pFileInfo->attrib & 0x10) 
			 	&& !(pFileInfo->attrib & 0x08))
	        {
	            // a file 
	            if (ThumbNail_IsPicFile(pFileInfo->name)
					&& nCount++ == pThumbNail->CurItemFocused.ItemIndex)
	            {
	                IconExplorer_SetFocus(&pThumbNail->pDisplayList->IconExplorer, pItem->Id);

#ifdef THUMB_Debug
	                Fwl_Print(C3, M_IMAGE, "focus id = %d", pItem->Id);
#endif
	                break;                   	
	            }                  
	            else
	            {
	                Fwl_Print(C4, M_IMAGE, "The file is not pic file, or not focus pic");
	            }                        
	        }

	        pItem = pItem->pNext;
        }       
    }
}

static T_BOOL ThumbNail_SetSlipItem(T_U32 startImg, T_U32 startIndex, T_S32 count, T_U32 *emptyItemNum)
{
	T_U32 i = 0;
	T_U32 num = 0;
	T_U32 index = startIndex;
	T_U8 *pbuf = AK_NULL;
	T_U32 width = 0;
	T_U32 height = 0;
	T_ICONEXPLORER_ITEM *pItem = AK_NULL;
    T_S32 nPicNumCount = 0;
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_DISPLAYLIST *pDisplayList = AK_NULL;
    T_U16 UsFullPathName[512];
	T_U8 *pData = AK_NULL;
    T_U8 *pBuffer = AK_NULL;
    T_S32 nDataOffset =  -1;
    T_S32 nBytesPerStageLine = -1;
    T_S32 nBytesPerThumbLine = -1;
	T_RECT rect;
	T_U32 emptyNum = 0;
	T_S32 cnt = 0;

	if (AK_NULL == pThumbNail
		|| AK_NULL == pThumbNail->pSlipMgr
		|| pThumbNail->nTotalPicNum <= 0)
	{
		return AK_FALSE;
	}

	pDisplayList = pThumbNail->pDisplayList;

	num = count > 0 ? count : 0 - count;

	pItem = pDisplayList->IconExplorer.pItemHead; 

	while (pItem != AK_NULL)
	{
	    if (AK_NULL != (pFileInfo = (T_FILE_INFO*)(pItem->pContent))
			&& !(pFileInfo->attrib & 0x10) 
			&& !(pFileInfo->attrib & 0x08)
			&& ThumbNail_IsPicFile(pFileInfo->name))
	    {
	        // a file  	        
	        nPicNumCount++;

	        if (nPicNumCount > (T_S32)startImg
				&& nPicNumCount <= (T_S32)(startImg + num) 
				&& nPicNumCount <= pThumbNail->nTotalPicNum)
            {
                Utl_UStrCpy(UsFullPathName, pDisplayList->curPath);
                Utl_UStrCat(UsFullPathName, pFileInfo->name);

                pData = ThumbsDB_GetData(pThumbNail->hThumb, UsFullPathName);

				SlipMgr_SetItemId(pThumbNail->pSlipMgr, index, nPicNumCount - 1);
				SlipMgr_GetItemBufByIndex(pThumbNail->pSlipMgr, index, &pbuf, &width, &height);
				index++;

				RectInit(&rect, 0, 0, (T_LEN)width, (T_LEN)height);
				Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, pThumbNail->StageBgColor, RGB565);

                if (pData == AK_FALSE)
                {
                	T_USTR_FILE Utmpstr;
					T_U32 strWidth = 0;
					T_U32 x = 0;
					T_U32 y = 0;

					Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_AUDIOPLAYER_UNKNOW));
					strWidth = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
					
                    // the pic can not be decoded, show unknown string 
					y = pThumbNail->stctUIItem.nItemY_Interval;
					x = pThumbNail->stctUIItem.nItemX_Interval;

					Fwl_UDispSpeciStringOnRGB(pbuf,
                    	width, height,
						(T_POS)(x + (THUMBNAIL_WIDTH - strWidth) / 2), 
						(T_POS)(y + (THUMBNAIL_HEIGHT - GetFontHeight(CURRENT_FONT_SIZE)) / 2), Utmpstr, COLOR_WHITE, RGB565, 
						CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
                }   
                else
                {
                    nDataOffset =  (pThumbNail->stctUIItem.nItemY_Interval * width + pThumbNail->stctUIItem.nItemX_Interval) * COLOR_SIZE;
                    pBuffer = pbuf + nDataOffset; 
                    nBytesPerThumbLine = THUMBNAIL_WIDTH * COLOR_SIZE;
                    nBytesPerStageLine = width * COLOR_SIZE;

                    for(i = 0; i < THUMBNAIL_HEIGHT; i++)
                    {
                        memcpy(pBuffer, pData, nBytesPerThumbLine);
                        pData += nBytesPerThumbLine ;
                        pBuffer += nBytesPerStageLine;
                    }                     
                }                                                                
            }
			else if (nPicNumCount > (T_S32)(startImg + num))
			{
				break;
			}                           
        }

        pItem = pItem->pNext;
    }

	if (nPicNumCount < (T_S32)(startImg + num))
	{
		emptyNum = startImg + num - nPicNumCount;
	}

	if (0 == emptyNum)
	{
		if (0 == *emptyItemNum)
		{
			SlipMgr_AddLoadItemNum(pThumbNail->pSlipMgr, count);
		}
		else if (count < 0)
		{
			cnt = count + *emptyItemNum;
			*emptyItemNum = 0;
			
			SlipMgr_AddLoadItemNum(pThumbNail->pSlipMgr, cnt);
		}
	}
	else
	{
		while (nPicNumCount < (T_S32)(startImg + num))
		{
			SlipMgr_SetItemId(pThumbNail->pSlipMgr, index, -1);
			SlipMgr_GetItemBufByIndex(pThumbNail->pSlipMgr, index, &pbuf, &width, &height);
			index++;
			nPicNumCount++;
			
			RectInit(&rect, 0, 0, (T_LEN)width, (T_LEN)height);
			Fwl_FillSolidRectOnRGB(pbuf, width, height, &rect, pThumbNail->StageBgColor, RGB565);
		}

		if (count > 0)
		{
			count -= emptyNum;
		}
		else
		{
			count += emptyNum;
		}
		
		SlipMgr_AddLoadItemNum(pThumbNail->pSlipMgr, count);
		*emptyItemNum = emptyNum;
	}

	return AK_TRUE;
}

static T_BOOL ThumbNail_LoadSlipItem(T_S32 count, T_U32 loadItemNum)
{
	T_S32 startIndex = 0;
	T_U32 itemNum = 0;
	T_U32 startImg = 0;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	startImg = loadItemNum;
	itemNum = SlipMgr_GetItemNum(pThumbNail->pSlipMgr);

	if (count > 0)
	{
		startIndex = SlipMgr_GetIndexById(pThumbNail->pSlipMgr, loadItemNum - 1) + 1;
		
		if (startIndex >= (T_S32)itemNum)
		{
			startIndex -= itemNum;
		}
		
		ThumbNail_SetSlipItem(startImg, startIndex, count, &(pThumbNail->emptyItemNum));
	}
	else
	{
		startIndex = SlipMgr_GetIndexById(pThumbNail->pSlipMgr, loadItemNum - 1) + count + 1 + pThumbNail->emptyItemNum;

		if (startIndex < 0)
		{
			startIndex += itemNum;
		}

		startImg = loadItemNum + pThumbNail->emptyItemNum - itemNum + count;

		ThumbNail_SetSlipItem(startImg, startIndex, count, &(pThumbNail->emptyItemNum));
	}


	return AK_TRUE;
}


static T_BOOL ThumbNail_CheckSlipFocus(T_VOID)
{
	T_U32 focusId = 0;
	T_U32 index = 0;
	T_U32 slipItemNum = 0;
	T_U32 remainNum = 0;
	T_S32 count = 0;
	T_U32 loadItemNum = 0;
	T_S32 offset = 0;
	T_U32 num = 0;
	
    AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);

	SlipMgr_PrepareToShow(pThumbNail->pSlipMgr, &count, &loadItemNum, 0, 0);

    if (pThumbNail->SuspendItemFocused.ItemIndex != pThumbNail->CurItemFocused.ItemIndex)
	{
		focusId = pThumbNail->CurItemFocused.ItemIndex;
		
		if (SlipMgr_CheckFocusItem(pThumbNail->pSlipMgr, focusId))
		{
			ThumbNail_SetRefreshFlag(REFRESH_ALL);
			return AK_TRUE;
		}

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			index = focusId;
		}
		else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
		{
			index = focusId - focusId % pThumbNail->itemNumPreRow;
		}
				
		slipItemNum = SlipMgr_GetItemNum(pThumbNail->pSlipMgr);

		if ((T_S32)(index + slipItemNum) <= pThumbNail->nTotalPicNum)
		{
			SlipMgr_ClearOffset(pThumbNail->pSlipMgr, index);
			pThumbNail->emptyItemNum = 0;
			ThumbNail_SetSlipItem(index, 0, slipItemNum, &(pThumbNail->emptyItemNum));
			SlipMgr_SetLoadItemNum(pThumbNail->pSlipMgr, index + slipItemNum);
			SlipMgr_PrepareToShow(pThumbNail->pSlipMgr, &count, &loadItemNum, 0, 0);
		}
		else
		{
			if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
			{
				remainNum = index + slipItemNum - pThumbNail->nTotalPicNum;
				index -= remainNum;
			}
			else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
			{
				num = pThumbNail->nTotalPicNum % pThumbNail->itemNumPreRow;

				if (0 == num)
				{
					while ((T_S32)(index + slipItemNum) > pThumbNail->nTotalPicNum)
					{
						index -= pThumbNail->itemNumPreRow;
					}
				}
				else
				{
					while (index + slipItemNum > pThumbNail->nTotalPicNum - num + pThumbNail->itemNumPreRow)
					{
						index -= pThumbNail->itemNumPreRow;
					}
				}
			}
			
			SlipMgr_ClearOffset(pThumbNail->pSlipMgr, index);
			ThumbNail_SetSlipItem(index, 0, slipItemNum, &(pThumbNail->emptyItemNum));
			SlipMgr_SetLoadItemNum(pThumbNail->pSlipMgr, pThumbNail->nTotalPicNum);

			if (!SlipMgr_CheckFocusItem(pThumbNail->pSlipMgr, focusId))
			{
				if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
				{
					if (0 == slipItemNum % pThumbNail->itemNumPreCol)
					{
						offset = pThumbNail->StageRect.width - slipItemNum / pThumbNail->itemNumPreCol * pThumbNail->stctUIItem.nItemWidth;
					}
					else
					{
						offset = pThumbNail->StageRect.width - (slipItemNum / pThumbNail->itemNumPreCol + 1) * pThumbNail->stctUIItem.nItemWidth;
					}
				}
				else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
				{
					if (0 == slipItemNum % pThumbNail->itemNumPreRow)
					{
						offset = pThumbNail->StageRect.height - slipItemNum / pThumbNail->itemNumPreRow * pThumbNail->stctUIItem.nItemHeight;
					}
					else
					{
						offset = pThumbNail->StageRect.height - (slipItemNum / pThumbNail->itemNumPreRow + 1) * pThumbNail->stctUIItem.nItemHeight;
					}
				}
				
				SlipMgr_PrepareToShow(pThumbNail->pSlipMgr, &count, &loadItemNum, offset, 0);
			}
			else
			{
				SlipMgr_PrepareToShow(pThumbNail->pSlipMgr, &count, &loadItemNum, 0, 0);
			}
		}

		ThumbNail_SetRefreshFlag(REFRESH_ALL);
	}

    return AK_TRUE;
}


static T_BOOL ThumbNail_CreatSlipMgr(T_VOID)
{
	E_MOVETYPE movetype = MOVETYPE_Y;
	T_U32 slipItemNum = 0;

	AK_ASSERT_PTR(pThumbNail, "pThumbNail Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pThumbNail->pDisplayList, "pThumbNail->pDisplayList Is Invalid", AK_FALSE);
	
	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		movetype = MOVETYPE_X;
	}
	else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
	{
		movetype = MOVETYPE_Y;
	}

	if (AK_NULL != pThumbNail->pSlipMgr)
	{
		pThumbNail->pSlipMgr = SlipMgr_Destroy(pThumbNail->pSlipMgr);
	}

	pThumbNail->pSlipMgr = SlipMgr_Creat(ITEM_TYPE_IMAGE, pThumbNail->StageRect, 
		pThumbNail->stctUIItem.nItemWidth, pThumbNail->stctUIItem.nItemHeight,
		pThumbNail->nTotalPicNum, movetype);

	AK_ASSERT_PTR(pThumbNail->pSlipMgr, "pThumbNail Is Invalid", AK_FALSE);
		
	SlipMgr_SetBgColor(pThumbNail->pSlipMgr, pThumbNail->StageBgColor);

	slipItemNum = SlipMgr_GetItemNum(pThumbNail->pSlipMgr);
	pThumbNail->emptyItemNum = 0;
	pThumbNail->SuspendItemFocused.ItemIndex = 0;
	ThumbNail_SetSlipItem(0, 0, slipItemNum, &(pThumbNail->emptyItemNum));
	ThumbNail_CheckSlipFocus();
	
	return AK_TRUE;
}

static T_BOOL ThumbNail_InitIcon(T_RES_ICON *icon, T_U32 resID, T_COLOR color, T_BOOL loadRes)
{
	T_RECT rect;
	
	if (loadRes)
	{
		T_U32 len;
		
		icon->pic = Res_GetBinResByID(AK_NULL, AK_FALSE, resID, &len);
	}
		
	RectInit(&rect, 0, 0, icon->rect.width, icon->rect.height);
	icon->buf = Fwl_Malloc(icon->rect.width * icon->rect.height * COLOR_SIZE + 64);
	AK_ASSERT_PTR(icon->buf, "Malloc icon->buf Failure", AK_FALSE);
	
	Fwl_FillSolidRectOnRGB(icon->buf, icon->rect.width, icon->rect.height, &rect, color, RGB565);	
	
	return AK_TRUE;
}

static T_VOID ThumbNail_InitRes_PreviewMode(T_THUMBNAIL_PARAM *pThumb)
{
	T_U32 len;
	T_RECT	*pRect;
	
	AK_ASSERT_PTR_VOID(pThumb, "pThumbNail Is Invalid");

	pRect = &pThumb->StageRect;

	pRect->height = THUMBNAIL_HEIGHT + 10;
	pRect->top  = Fwl_GetLcdHeight() - pRect->height - TOOLRECT_HEIGHT;
	pRect->left = 40;
	pRect->width = Fwl_GetLcdWidth() - 2 * pRect->left;
	
	pThumb->itemNumPreRow = 3;
	pThumb->itemNumPreCol = 1;
	pThumb->StageBgColor = STAGE_COLOR_ONEROW;
	
	ThumbNail_SetSlideBlock();

	//left arrow
	RectInit(&(pThumb->lArrow.rect), 0, pRect->top, pRect->left, pRect->height);
	ThumbNail_InitIcon(&pThumb->lArrow, eRES_BMP_IMG_THUMB_LARROW, pThumb->StageBgColor, AK_TRUE);

	//right arrow
	RectInit(&(pThumb->rArrow.rect), (T_POS)(pRect->left + pRect->width), pRect->top, pRect->left, pRect->height);
	ThumbNail_InitIcon(&pThumb->rArrow, eRES_BMP_IMG_THUMB_RARROW, pThumb->StageBgColor, AK_TRUE);
	
	//tool rect
	RectInit(&(pThumb->tool.rect), 0, (T_POS)(Fwl_GetLcdHeight() - TOOLRECT_HEIGHT), MAIN_LCD_WIDTH, TOOLRECT_HEIGHT);
	ThumbNail_InitIcon(&pThumb->tool, 0, pThumb->StageBgColor, AK_FALSE);
	
	//three buttons
	RectInit(&(pThumb->setIcon.rect), SET_PIC_LEFT, (T_POS)(Fwl_GetLcdHeight() - TOOLRECT_HEIGHT), TOOL_PIC_WIDTH+2*TOOL_PIC_INTERVAL_X, TOOLRECT_HEIGHT);
	pThumb->setIcon.pic = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_SET, &len);
	RectInit(&(pThumb->playIcon.rect), PLAY_PIC_LEFT, (T_POS)(Fwl_GetLcdHeight() - TOOLRECT_HEIGHT), TOOL_PIC_WIDTH+2*TOOL_PIC_INTERVAL_X, TOOLRECT_HEIGHT);
	pThumb->playIcon.pic = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_PLAY, &len);
	RectInit(&(pThumb->modeIcon.rect), MODE_PIC_LEFT, (T_POS)(Fwl_GetLcdHeight() - TOOLRECT_HEIGHT), TOOL_PIC_WIDTH+2*TOOL_PIC_INTERVAL_X, TOOLRECT_HEIGHT);
	pThumb->modeIcon.pic = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_MODE, &len);

	ThumbNail_Showbottombuttons();
	
	//info rect
	RectInit(&(pThumb->info.rect), 0, TOP_BAR_HEIGHT, MAIN_LCD_WIDTH, INFORECT_HEIGHT);
	ThumbNail_InitIcon(&pThumb->info, 0, pThumb->StageBgColor, AK_FALSE);
	
	//set slide
	pThumb->setSlideBkPic 	= Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_SETSLIDE_BK, &len);
	pThumb->BlueBarPic 		= Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_VOLUME_PROGRESS, &len);
	pThumb->GrayBarPic 		= Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_VOLUME_PROGRESS_GRAY, &len);
	pThumb->BlockPic 		= Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_VOLUME_BLOCK, &len);

	//backup buf
	pThumb->backupBuf = Fwl_Malloc(MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * COLOR_SIZE + 64);
	memset(pThumb->backupBuf, 0, MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * COLOR_SIZE + 64);

}

static T_VOID ThumbNail_InitRes_FullMode(T_THUMBNAIL_PARAM *pThumb)
{
	T_RECT	*pRect;  

	AK_ASSERT_PTR_VOID(pThumb, "pThumbNail Is Invalid");
	
	 //the  thumbnail rect
	pRect = &pThumb->StageRect;	
		
	pRect->top  = g_Graph.TTLHEIGHT;
	pRect->height = Fwl_GetLcdHeight() - g_Graph.TTLHEIGHT;
	pRect->left = 0;
	pRect->width = Fwl_GetLcdWidth();
	
	pThumb->StageBgColor = STAGE_COLOR_FULLSCR;
	pThumb->itemNumPreRow = 4;
	pThumb->itemNumPreCol = 3;
}

static T_VOID ThumbNail_InitRes(T_BOOL bReset)
{
	AK_ASSERT_PTR_VOID(pThumbNail, "pThumbNail Is Invalid");

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		ThumbNail_InitRes_PreviewMode(pThumbNail);
	}
	else if (THUMBNAIL_MODE_FULLSCR == pThumbNail->mode)
	{
		ThumbNail_InitRes_FullMode(pThumbNail);	
	}
	
	//thumbnail items property
    pThumbNail->stctUIItem.nItemHeight = pThumbNail->StageRect.height / pThumbNail->itemNumPreCol;
    pThumbNail->stctUIItem.nItemWidth = pThumbNail->StageRect.width / pThumbNail->itemNumPreRow;
    pThumbNail->stctUIItem.nItemX_Interval = (pThumbNail->stctUIItem.nItemWidth - THUMBNAIL_WIDTH) / 2;
    pThumbNail->stctUIItem.nItemY_Interval = (pThumbNail->stctUIItem.nItemHeight - THUMBNAIL_HEIGHT) / 2;
    pThumbNail->stctUIItem.ItemBgColor = COLOR_GREEN;

    pThumbNail->stctUIItem.nThumbImgHeight = THUMBNAIL_HEIGHT;
    pThumbNail->stctUIItem.nThumbImgWidth = THUMBNAIL_WIDTH;
		
    if (!bReset)
	{
		pThumbNail->pDisplayList = AK_NULL;  
		
	    pThumbNail->nPagesNum =    0;
	    pThumbNail->nCurPageIndex = 0;
	    pThumbNail->nTotalPicNum = 0;     

	    //item focus       
	    pThumbNail->preItemFocused.ItemIndex  = -1;
	    pThumbNail->preItemFocused.rctPos.left = 0;
	    pThumbNail->preItemFocused.rctPos.height = 0;
	    pThumbNail->preItemFocused.rctPos.top = 0;
	    pThumbNail->preItemFocused.rctPos.width = 0;

	    pThumbNail->CurItemFocused.ItemIndex  = -1;
	    pThumbNail->CurItemFocused.rctPos.left = 0;
	    pThumbNail->CurItemFocused.rctPos.top = 0;
	    pThumbNail->CurItemFocused.rctPos.height = 0;
	    pThumbNail->CurItemFocused.rctPos.width = 0;  

		pThumbNail->SuspendItemFocused.ItemIndex = 0;
	    
	    pThumbNail->hThumb = THUMBSDB_INVALID_HANDLE;

	    ThumbNail_ClearRefreshFlag(pThumbNail);
	}
}

static T_BOOL ThumbNail_HandleInitEvt(T_THUMBNAIL_PARAM *thumbNail, T_EVT_PARAM *pEventParm)
{
	T_U16           ucPathStr[512];
    T_U16           ucTmpStr[512];
#ifdef THUMB_Debug
    T_S8            szStr[512];
 
    Fwl_Print(C3, M_IMAGE, "handleimg_thumbnail_view");   
#endif
 
	AK_ASSERT_PTR(thumbNail, "Parameter thumbNail Is Invalid", 0)
	
    thumbNail->pDisplayList = (T_DISPLAYLIST *)pEventParm;
	AK_ASSERT_PTR(thumbNail->pDisplayList, "Parameter pDisplayList Is Invalid", 0)

	if (THUMBNAIL_MODE_ONEROW == thumbNail->mode)
	{
		ImgBrowse_Open(&thumbNail->ImgBrowse, thumbNail->pDisplayList);
	}
	
    thumbNail->nTotalPicNum = ThumbNail_GetPicNum(thumbNail->pDisplayList);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "total pic number = %d , pThumbNail->pDisplayList=%d, level =%d", thumbNail->nTotalPicNum,thumbNail->pDisplayList, thumbNail->pDisplayList->subLevel);
#endif

    Utl_UStrCpy(ucPathStr, thumbNail->pDisplayList->curPath);

#ifdef THUMB_Debug
    Eng_StrUcs2Mbcs(thumbNail->pDisplayList->curPath, szStr);
    Fwl_Print(C3, M_IMAGE, "szStr =%s", szStr);
#endif
    Eng_StrMbcs2Ucs(THUMBS_NAME, ucTmpStr);
    Utl_UStrCat(ucPathStr, ucTmpStr);

#ifdef THUMB_Debug
    Eng_StrUcs2Mbcs(ucPathStr, szStr);
    Fwl_Print(C3, M_IMAGE, "szStr =%s", szStr);
#endif

    thumbNail->hThumb = ThumbsDB_CreateHandle(ucPathStr);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_IMAGE, "testing pThumbNail->hThumb = %d", thumbNail->hThumb);
#endif

    if (THUMBSDB_INVALID_HANDLE == thumbNail->hThumb)
    {
        Fwl_Print(C3, M_IMAGE, "failed to  create thumbnail handle");
		MsgBox_InitAfx(&thumbNail->msgbox, 2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&thumbNail->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&thumbNail->msgbox);
        return AK_FALSE;
    }

    ThumbNail_SetRefreshFlag(REFRESH_ALL);  

    if (thumbNail->nTotalPicNum > 0)
    {
        ThumbNail_ResetData(thumbNail);
		ThumbNail_CreatSlipMgr();
		thumbNail->nPagesNum = (thumbNail->nTotalPicNum + 2) / (thumbNail->itemNumPreRow * thumbNail->itemNumPreCol);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_IMAGE, "picNum = %d, curPicIndex =%d, pages = %d, curpageIdex = %d", thumbNail->nTotalPicNum,
        thumbNail->CurItemFocused.ItemIndex, 
        thumbNail->nPagesNum,
        thumbNail->nCurPageIndex);
#endif
    }
    else
    {
        thumbNail->nPagesNum = 0;
        //no pic
        return AK_FALSE;
    } 

	return AK_TRUE;
}

void suspendimg_thumbnail_view(void)
{
	Eng_SetDefKeyTranslate(); //restore key translate funciton to default

    // to complete
    if (pThumbNail->bGoNext && (THUMBNAIL_MODE_ONEROW == pThumbNail->mode))
	{
	    ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
	    ThumbNail_SetRefreshFlag(REFRESH_MOVE_BUF); 
		ThumbNail_Show(1);

		//ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
	    //ThumbNail_SetRefreshFlag(REFRESH_MOVE_BUF); 
		//ThumbNail_Show(2);

		ThumbNail_FreeRes();
		ImgBrowse_Free(&pThumbNail->ImgBrowse);
    }
	
	TopBar_DisableMenuButton();
}

void resumeimg_thumbnail_view(void)
{
	Eng_SetKeyTranslate(Eng_ImgThumbnailViewTranslate);//set key translate function

    // to complete
    TopBar_EnableMenuButton();
	
	if (pThumbNail->bGoNext)
	{
		pThumbNail->bGoNext = AK_FALSE;

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			ThumbNail_InitRes(AK_TRUE);
		}
		
		ThumbNail_ResetData(pThumbNail);
		ThumbNail_ReCalcuPos(pThumbNail, &pThumbNail->CurItemFocused);
        ThumbNail_ReCalcuPos(pThumbNail, &pThumbNail->preItemFocused);
		
		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			ImgBrowse_Init(&pThumbNail->ImgBrowse);
			ImgBrowse_SetDisMode(&pThumbNail->ImgBrowse,IMG_PREVIEW);
			ImgBrowse_Open(&pThumbNail->ImgBrowse, pThumbNail->pDisplayList);

			pThumbNail->showStep = 2;
		}
	}

	ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
    ThumbNail_SetRefreshFlag(REFRESH_ALL);  
    TopBar_SetTitle(GetCustomString(csImg_thumbnail));
    TopBar_Show(TB_REFRESH_ALL);  
//    TopBar_Refresh();
}

#endif
void initimg_thumbnail_view(void)
{
#ifdef SUPPORT_IMG_BROWSE

	Eng_SetKeyTranslate(Eng_ImgThumbnailViewTranslate);//set key translate function
	Standby_FreeUserBkImg();

    pThumbNail = (T_THUMBNAIL_PARAM*)Fwl_Malloc(sizeof(T_THUMBNAIL_PARAM));
    AK_ASSERT_PTR_VOID(pThumbNail, "initimg_thumbnail_view(): pThumbNail malloc error\n");
	memset(pThumbNail, 0, sizeof(T_THUMBNAIL_PARAM));
	
	pThumbNail->bGoNext = AK_FALSE;
	pThumbNail->mode = THUMBNAIL_MODE_ONEROW;

    m_regResumeFunc(resumeimg_thumbnail_view);
    m_regSuspendFunc(suspendimg_thumbnail_view);    
	
    ThumbNail_InitRes(AK_FALSE);

//    Reset TopBar's Title 
	TopBar_EnableMenuButton();
    TopBar_SetTitle(GetCustomString(csImg_thumbnail));
    //TopBar_Show(TB_REFRESH_TITLE);

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		ImgBrowse_Init(&pThumbNail->ImgBrowse);
		ImgBrowse_SetDisMode(&pThumbNail->ImgBrowse, IMG_PREVIEW);
	}
	
    /**start waitbox*/
    WaitBox_Start(WAITBOX_CLOCK, (T_pWSTR)GetCustomString(csLOADING));
	Fwl_Print(C3, M_IMAGE, "initimg_thumbnail_view OK!");  
#endif
}


void exitimg_thumbnail_view(void)
{
#ifdef SUPPORT_IMG_BROWSE
	Eng_SetDefKeyTranslate(); //restore key translate funciton to default

    /**stop waitbox*/
    WaitBox_Stop();

    if (pThumbNail != AK_NULL)
    {        
        ThumbNail_FreeData(pThumbNail);
		ThumbNail_FreeRes();

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
		{
			ImgBrowse_Free(&pThumbNail->ImgBrowse);
		}

		if (AK_NULL != pThumbNail->pSlipMgr)
		{
			pThumbNail->pSlipMgr = SlipMgr_Destroy(pThumbNail->pSlipMgr);
		}
		
        Fwl_Free(pThumbNail);
        pThumbNail = AK_NULL;
    }
	
	TopBar_DisableMenuButton();
	ScreenSaverEnable();
	Standby_LoadUserBkImg();
#endif
}


void paintimg_thumbnail_view(void)
{
#ifdef SUPPORT_IMG_BROWSE
	if (!pThumbNail->ImgBrowse.largeFlag 
		|| (pThumbNail->ImgBrowse.largeFlag && (ERROR_TIMER == pThumbNail->ImgBrowse.DecTimerId)))
	{
		/**stop waitbox*/
		WaitBox_Stop();
		
		ThumbNail_Show(pThumbNail->showStep);
		if (2 == pThumbNail->showStep)
		{
			ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
		    ThumbNail_SetRefreshFlag(REFRESH_MOVE_BUF); 
			ThumbNail_Show(1);

			ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
		    ThumbNail_SetRefreshFlag(REFRESH_ALL); 
			ThumbNail_Show(0);

			pThumbNail->showStep = 0;
		}
		
		ScreenSaverEnable();
	}
#endif
}

unsigned char handleimg_thumbnail_view(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE
    T_MMI_KEYPAD    phyKey;    
    T_eBACK_STATE   retState = eStay;
    T_POS           posX = 0;
    T_POS           posY = 0;
	T_U32			focusId = 0;
	T_S32			count = 0;
	T_U32			loadItemNum = 0;
    
    T_FILE_INFO *pFileInfo = AK_NULL;    

	if (IsPostProcessEvent(event))
    {
        return 1;
    }

    switch (event)
    {
    case M_EVT_3:
        //init the thumbnail image area
        if (!ThumbNail_HandleInitEvt(pThumbNail, pEventParm))
			return 0;
		
        break;
        
    case M_EVT_USER_KEY:
        phyKey.keyID = (T_eKEY_ID) pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL) pEventParm->c.Param2;

        retState = ThumbNail_UserKey_Handle(pThumbNail,&phyKey);          
		break;
		
    case M_EVT_TOUCH_SCREEN:
        phyKey.keyID = kbNULL;
        phyKey.pressType = PRESS_SHORT;
        posX = (T_POS)pEventParm->s.Param2;
        posY = (T_POS)pEventParm->s.Param3;

        switch(pEventParm->s.Param1)
        {
        case eTOUCHSCR_UP:
			Fwl_Print(C3, M_IMAGE, "handleimg_thumbnail_view eTOUCHSCR_UP"); 
			
			ThumbNail_HitButton_Handle(pThumbNail, eTOUCHSCR_UP, &phyKey, posX, posY);
            retState = ThumbNail_UserKey_Handle(pThumbNail,&phyKey);					

			if ((PRESS_SHORT == phyKey.pressType) 
				&& ((kbLEFT == phyKey.keyID) || (kbRIGHT == phyKey.keyID))
				&& (SLIPMSG_STA_STOP == SlipMgr_GetCurStatus(pThumbNail->pSlipMgr)))
			{
				event = M_EVT_USER_KEY;
				pEventParm->c.Param1 = (T_U8)phyKey.keyID;
				pEventParm->c.Param2 = (T_U8)phyKey.pressType;						
			}
					
            break;
			
        case eTOUCHSCR_DOWN:
			ThumbNail_HitButton_Handle(pThumbNail, eTOUCHSCR_DOWN, &phyKey, posX, posY);
            break;
			
        case eTOUCHSCR_MOVE:
            break;
			
        default:
            break;
        }
        break;
		
    default:
        break;               
    }

	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	{
		if (M_EVT_USER_KEY == event
			&& !pThumbNail->bSetSlide
			&& SLIPMSG_STA_STOP == SlipMgr_GetCurStatus(pThumbNail->pSlipMgr))
		{
			phyKey.keyID = (T_eKEY_ID) pEventParm->c.Param1;
	        phyKey.pressType = (T_BOOL) pEventParm->c.Param2;

			if (PRESS_SHORT == phyKey.pressType 
				&& (kbLEFT == phyKey.keyID || kbRIGHT == phyKey.keyID))
			{
				ImgBrowse_Handle(&pThumbNail->ImgBrowse, event, pEventParm);
			}
		}

		if (VME_EVT_TIMER == event 
			&& pEventParm->w.Param1 == (T_U32)pThumbNail->ImgBrowse.DecTimerId
			&& pThumbNail->ImgBrowse.largeFlag)
		{
			ImgBrowse_Handle(&pThumbNail->ImgBrowse, event, pEventParm);
		}
	}


	if (eStay == retState
		&& !pThumbNail->bSetSlide)
	{
		if (VME_EVT_TIMER == event
			&& pThumbNail->pSlipMgr->refreshTimer == (T_TIMER)pEventParm->w.Param1)
		{
			ThumbNail_SetRefreshFlag(REFRESH_MOVE_BUF);
		}
		
		focusId = pThumbNail->CurItemFocused.ItemIndex;
		
		retState = SlipMgr_Handle(pThumbNail->pSlipMgr, event, pEventParm, &count, &loadItemNum, &focusId, pThumbNail->emptyItemNum);

		if (pThumbNail->CurItemFocused.ItemIndex != (T_S32)focusId)
		{
			pThumbNail->preItemFocused = pThumbNail->CurItemFocused;   
			ThumbNail_ChangeFocus(pThumbNail, focusId);
			pThumbNail->bChangeFocus = AK_TRUE;
			ImgBrowse_SetRefresh(&pThumbNail->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
		}

		if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode
			&& SLIPMSG_STA_STOP == SlipMgr_GetCurStatus(pThumbNail->pSlipMgr)
			&& pThumbNail->bChangeFocus)
		{
			pThumbNail->bChangeFocus = AK_FALSE;
			ThumbNail_RestFileList(pThumbNail);                
            DisplayList_Operate(pThumbNail->pDisplayList);

			if (eNext != retState)
			{
				ImgBrowse_Change(&pThumbNail->ImgBrowse);
				ThumbNail_SetRefreshFlag(REFRESH_ALL);
			}
		}

		if ((0 != count) && (0 != loadItemNum))
		{
			ThumbNail_LoadSlipItem(count, loadItemNum);
		}

	}
	else if (eNext == retState)
	{
		pThumbNail->SuspendItemFocused.ItemIndex = pThumbNail->CurItemFocused.ItemIndex;
	}

	if ((M_EVT_EXIT == event) || ((event >= M_EVT_RETURN) && (event <= M_EVT_RETURN9)))
	{
		ThumbNail_CheckSlipFocus();
	}

    switch (retState)
    {
    case eMenu:
        pFileInfo = DisplayList_GetItemContentFocus(pThumbNail->pDisplayList);
        if (pFileInfo != AK_NULL)
        {
            pEventParm->p.pParam1 = (T_pVOID)pThumbNail->pDisplayList;
			
			if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode
				&& pThumbNail->ImgBrowse.bSupportPic)
			{
            	pEventParm->p.pParam2 = (T_pVOID)ImgBrowse_GetOutBuf(&pThumbNail->ImgBrowse);
			}
			else
			{
				pEventParm->p.pParam2 = AK_NULL;
			}
			
            m_triggerEvent(M_EVT_NEXT, pEventParm);
        }
    	break;
		
    case eNext:  
        if (pThumbNail != AK_NULL
			&& pThumbNail->nTotalPicNum != 0)
        {
            ThumbNail_RestFileList(pThumbNail);                
            Fwl_Print(C3, M_IMAGE, "next pThumbNail->pDisplayList=%d",pThumbNail->pDisplayList);
            pFileInfo = DisplayList_Operate(pThumbNail->pDisplayList);
			TopBar_DisableMenuButton();
			
            if (pFileInfo != AK_NULL)
            { 
                pEventParm = (T_EVT_PARAM *)(pThumbNail->pDisplayList);
				pThumbNail->bGoNext = AK_TRUE;

				if (pThumbNail->bGoSlide)
				{
					Fwl_Print(C3, M_IMAGE, "go to slide");
					m_triggerEvent(M_EVT_1, pEventParm);
					pThumbNail->bGoSlide = AK_FALSE;
				}
				else
				{
					Fwl_Print(C3, M_IMAGE, "go to browse");
                	m_triggerEvent(M_EVT_3, pEventParm);
				}
            }
        }
        break;
		
    case eReturn:
	 	if (pThumbNail != AK_NULL)
        {
            ThumbNail_RestFileList(pThumbNail);
        }
		
	 	if (THUMBNAIL_MODE_ONEROW == pThumbNail->mode)
	 	{
	        m_triggerEvent(M_EVT_EXIT, pEventParm); 
	 	}
		else
		{
			ThumbNail_ChangeMode(pThumbNail);	
		}
        break;
		
    default:
        ReturnDefauleProc(retState, pEventParm);
        break;
    } // end switch(ret)    
#endif
    return 0;
}

