/**
 * @file Ctl_SlipMgr.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPMGR_H__
#define __CTL_SLIPMGR_H__


#include "akdefine.h"
#include "Gbl_macrodef.h"
#include "Fwl_vme.h"

#define SLIP_ITEM_HEIGHT		(27)

#define SLIP_ITEM_ICON_WIDTH	(SLIP_ITEM_HEIGHT - 2 * X_INTERVAL)
#define SLIP_ITEM_ICON_HEIGHT	(SLIP_ITEM_HEIGHT - 2 * Y_INTERVAL)


#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	ITEM_TYPE_LIST,		// ÁĞ±íÊ½µÄlist
	ITEM_TYPE_IMAGE,	// Í¼Æ¬ä¯ÀÀÄ£¿éËõÂÔÍ¼×¨ÓÃ
	ITEM_TYPE_NUM
	
}E_ITEMTYPE;

//iconMode
typedef enum{
	ICON_NONE,				// ÎŞÍ¼Æ¬
	ICON_LEFT_ONLY,			// Ö»ÓĞ×óÍ¼Æ¬
	ICON_RIGHT_ONLY,		// Ö»ÓĞÓÒÍ¼Æ¬
	ICON_ALL,				// ×óÓÒÍ¼Æ¬¶¼Ó

}SLIP_ITEM_ICON_MODE;


//textMode
typedef enum{
	TEXT_NONE, 				// ÎŞÎÄ×Ö
	TEXT_MAIN_ONLY,			// Ö»ÓĞÖ÷ÎÄ×Ö
	TEXT_MAIN_AND_DOWNLINE,	// Ö÷ÎÄ×ÖºÍÏÂĞĞÎÄ×Ö
	TEXT_MAIN_AND_RIGHT,	// Ö÷ÎÄ×ÖºÍÓÒ²àÎÄ×Ö
	TEXT_ALL,				// Ö÷ÎÄ×Ö¡¢ÏÂĞĞÎÄ×ÖºÍÓÒ²àÎÄ×Ö¶¼ÓĞ
	
}SLIP_ITEM_TEXT_MODE;

typedef enum{
	SLIPMSG_STA_STOP,		// Í£Ö¹×´Ì¬£¬ÓÃ»§ºÍÖÕ¶ËÃ»ÓĞÈÎºÎ½Ó´¥
	SLIPMSG_STA_DOWN,		// °´ÏÂ×´Ì¬£¬ÓÃ»§ÊÖ½Ó´¥ÖÕ¶ËÊ±£¬ÏìÓ¦downÊÂ¼ş
	SLIPMSG_STA_FOCUS,		// ¾Û½¹×´Ì¬£¬ÓÃ»§ÊÖ½Ó´¥ÖÕ¶Ë£¬Ëù¶ÔÓ¦²Ëµ¥³ÉÎª½¹µã
	SLIPMSG_STA_MOVE,		// ÒÆ¶¯×´Ì¬£¬ÓÃ»§ÊÖ½Ó´¥ÖÕ¶ËÒÆ¶¯
	SLIPMSG_STA_SLIP		// »¬¶¯×´Ì¬£¬ÓÃ»§ÊÖÃ»ÓĞ½Ó´¥ÖÕ¶Ë£¬²Ëµ¥¹ßĞÔ»¬¶¯
	
}E_SLIPMSG_STA;

typedef enum{
	SLIPMSG_NONE,
	SLIPMSG_UP,				// ´¥Ãş·Å¿ªÏûÏ¢
	SLIPMSG_DOWN,			// ´¥Ãş°´ÏÂÏûÏ¢
	SLIPMSG_MOVE,			// ´¥ÃşÒÆ¶¯ÏûÏ¢
	SLIPMSG_DOWNTIMER,		// down¼ÆÊ±Æ÷ÏûÏ¢
	SLIPMSG_REFRESHTIMER,	// Ë¢ĞÂ¼ÆÊ±Æ÷ÏûÏ¢
	SLIPMSG_SLIPOK			// ÒÆ¶¯Íê³ÉÏûÏ¢
	
}E_SLIPMSG_TYPE;

typedef enum{
	SLIPMSG_UNFOCUS_ITEM,	// unfocused item
	SLIPMSG_FOCUS_ITEM,		// focus item
	SLIPMSG_NOT_ITEM,		// not in item rect
	
}E_FOCUS_FLAG;

typedef  enum {
	SLIPCALC_MOVE_UP,		// ÏòÉÏÒÆ¶¯
	SLIPCALC_MOVE_DOWN,		// ÏòÏÂÒÆ¶¯
	SLIPCALC_MOVE_LEFT,		// Ïò×óÒÆ¶¯
	SLIPCALC_MOVE_RIGHT		// ÏòÓÒÒÆ¶¯
	
} E_MOVE_DIRECTION;


typedef struct _T_SLIP_TEXT{
	T_U16	*pText;			// ÎÄ×ÖÄÚÈİ
	T_RECT	rect;			// ÎÄ×ÖÇøÓò
	T_U32	alignMode;		// ÎÄ×ÖµÄ¶ÔÆë·½Ê½
	T_S32	scrlOffset;		// ¹ö¶¯Æ«ÒÆ×Ö·ûÊı
	T_U32	scrlOkCnt;		// ¹ö¶¯ÍêÕû¼ÆÊı
	
}T_SLIP_TEXT;


typedef struct _T_ICON{
	T_pDATA	pIcon;			// Í¼Æ¬Êı¾İ
	T_RECT	rect;			// Í¼Æ¬ÇøÓò
	T_U32	alignMode;		// Í¼Æ¬µÄ¶ÔÆë·½Ê½
	
}T_SLIP_ICON;

typedef struct _T_SLIP_ITEM{
	T_S32				id_of_item;		// ¸Ãitem¶ÔÓ¦Êµ¼ÊÁ´±íÖĞµÄitemµÄ±êÊ¶£¨ÓÉÓ¦ÓÃ¾ö¶¨´«Ê²Ã´¸øËü£©
	T_RECT				rect;			// ¸ÃitemµÄÔÚÆäËùÔÚÒ³bufferÀïµÄÇøÓò
	T_U8				*pbuf;			// ¸ÃitemµÄbuffer
	E_ITEMTYPE			type;			// itemÀàĞÍ£¬ÊÇÁĞ±íÊ½»¹ÊÇÍ¼Æ¬ä¯ÀÀËõÂÔÍ¼
	
	//bg
	T_COLOR				bgColor;		// ¸Ãitem·Ç½¹µãÊ±±³¾°ÑÕÉ«
	T_COLOR				focusBgColor;	// ¸Ãitem½¹µãÊ±±³¾°ÑÕÉ«
	T_pDATA				pBgImg;			// ¸Ãitem·Ç½¹µãÊ±±³¾°Í¼Æ¬
	T_pDATA				pFocusBgImg;	// ¸Ãitem½¹µãÊ±±³¾°Í¼Æ¬
	
	//text
	SLIP_ITEM_TEXT_MODE textMode;		// ¸ÃitemÎÄ×ÖÄ£Ê½
	T_SLIP_TEXT			*pTextMain;		// ¸ÃitemÖ÷ÎÄ×Ö½á¹¹Ö¸Õë
	T_SLIP_TEXT			*pTextDownLine;	// ¸ÃitemÏÂĞĞÎÄ×Ö½á¹¹Ö¸Õë
	T_SLIP_TEXT			*pTextRight;	// ¸ÃitemÓÒ²àÎÄ×Ö½á¹¹Ö¸Õë
	
	//icon
	SLIP_ITEM_ICON_MODE	iconMode;		// ¸ÃitemÍ¼Æ¬Ä£Ê½
	T_SLIP_ICON			*pIconLeft;		// ¸Ãitem×óÍ¼Æ¬½á¹¹Ö¸Õë
	T_SLIP_ICON			*pIconRight;	// ¸ÃitemÓÒÍ¼Æ¬½á¹¹Ö¸Õë

	struct _T_SLIP_ITEM	*pPrevious;		// ¸ÃitemÇ°Ò»¸ö½Úµã
    struct _T_SLIP_ITEM	*pNext;			// ¸ÃitemºóÒ»¸ö½Úµã
    
} T_SLIP_ITEM;

typedef struct _T_ITEM_MGR
{   
	T_U8			*pShow;			// Display Buffer
	T_U32			width;			// Buffer Width
	T_U32			height;			// Buffer Height
	T_U32			itemW;			// Item Width
	T_U32			itemH;			// Item Height
	
	E_ITEMTYPE		itemType;		// itemÀàĞÍ£¬ÊÇÁĞ±íÊ½»¹ÊÇÍ¼Æ¬ä¯ÀÀËõÂÔÍ¼
	T_U32			itemNumPerRow;	// Ã¿ĞĞµÄitemÊıÁ¿(ËõÂÔÍ¼ÓÃ)
	T_U32			itemNumPerCol;	// Ã¿ÁĞµÄitemÊıÁ¿(ËõÂÔÍ¼ÓÃ)
	
	T_pDATA			pBgImg;			// ±³¾°Í¼Æ¬
	T_COLOR			bgColor;		// ±³¾°ÑÕÉ«	

	T_U32			itemNum; 		// ·Ö¼¸¸öitem	
	T_U32			curItem;		// µ±Ç°ÏÔÊ¾µÄµÚÒ»¸öitem£¬Õë¶ÔitemNumÀ´Ëµ
	
	E_MOVETYPE		moveType; 		// »¬¶¯ÀàĞÍ£¬·ÖÉÏÏÂ»¬¶¯ºÍ×óÓÒ»¬¶¯
	T_U32			loadItemNum;	// µ±Ç°item»º³åµ½µÄ¸öÊı
	T_U32			totalItemNum;	// Êµ¼ÊÁ´±íµÄitem×Ü¸öÊı
	T_SLIP_ITEM 	**ppItemhead;	// itemµÄÁ´±íÍ·Ë«ÖØµÄÖ¸Õë
	
	T_S32			overLen;		// Ô½¹ı±ß½çµÄ³¤¶È
	T_U32			nextRemainLen;	// ºó·½Ê£Óà³¤¶È
	T_U32			preRemainLen;	// Ç°·½Ê£Óà³¤¶È
	
}T_ITEM_MGR;


/*
 * Slipping Message Management
 */
typedef struct _T_SLIP_MSG
{   
	E_SLIPMSG_STA	lastSta;	// ÉÏ´ÎµÄ×´Ì¬
	E_SLIPMSG_STA	curSta;		// µ±Ç°µÄ×´Ì¬
	T_TIMER			timer;		// ÏûÏ¢Ê±¼ä¼ÆÊ±Æ÷
	E_MOVETYPE		moveType; 	// »¬¶¯ÀàĞÍ£¬·ÖÉÏÏÂ»¬¶¯ºÍ×óÓÒ»¬¶¯
	T_POINT			downPoint;	// down×ø±ê
	E_FOCUS_FLAG	focusFlag;	// ÒÑ¾­ÊÇfocusµÄitem
	
}T_SLIP_MSG;


/*
 * Scrollbar Struct
 */
typedef struct _T_SLIP_SCRB
{
	T_RECT		rect;		// ¹ö¶¯ÌõµÄÇøÓò
	T_BOOL		visible;	// ¹ö¶¯ÌõÊÇ·ñ¿É¼û
	E_MOVETYPE	moveType;	// ÉÏÏÂ»¬¶¯»¹ÊÇ×óÓÒ»¬¶¯

	T_U32		maxSize;	// ×î´ó³¤¶È
	T_U32		size;		// »¬¿é´óĞ¡
	T_POS		pos;		// »¬¿éÎ»ÖÃ
}T_SLIP_SCRB;


/*
 * Slipping Speed Calculating
 */
typedef struct _T_SLIP_CALC
{   
	E_MOVETYPE			moveType;			// »¬¶¯ÀàĞÍ£¬·ÖÉÏÏÂ»¬¶¯ºÍ×óÓÒ»¬¶¯
	E_MOVE_DIRECTION	moveDirection;		// »¬¶¯·½Ïò
	
	T_POINT				downPoint;			// touch downµÄ×ø±ê
	T_POINT				movePointOld;		// touch move¾ÉµÄ×ø±ê
	T_POINT				movePointNew;		// touch moveĞÂµÄ×ø±ê
	T_POINT				upPoint;			// touch upµÄ×ø±ê
	
	T_U32				time;				// Ê±¼ä
	float				V0;					// ³õËÙ¶È
	float				Vt;					// µ±Ç°ËÙ¶È
	float				a;					// ¼ÓËÙ¶È
	T_S32				S; 					// µ±Ç°Î»ÒÆ
	
	T_S32				totalLen;			// °üÀ¨»Øµ¯µÄ×Ü»¬¶¯Â·³Ì
	T_S32				actualLen;			// Æğµãµ½ÖÕµãµÄÊµ¼ÊÎ»ÒÆ
	T_BOOL				bReboundFlag;		// »Øµ¯±ê¼Ç
	T_BOOL				bStillFlag;			// ¾²Ö¹±ê¼Ç
	
}T_SLIP_CALC;


typedef struct _T_SLIPMGR
{
	T_ITEM_MGR	*pItemMgr;		// Ïî¹ÜÀíÄ£¿é¾ä±ú
	T_SLIP_MSG	*pMsg;			// ÏûÏ¢¹ÜÀíÄ£¿é¾ä±ú
	T_SLIP_CALC	*pCalc;			// »¬¶¯Ëã·¨Ä£¿é¾ä±ú
	T_SLIP_SCRB	*pScrb;			// ¹ö¶¯ÌõÄ£¿é¾ä±ú

	T_RECT		rect;			// ÏÔÊ¾ÇøÓò
	T_TIMER		refreshTimer;	// Ë¢ĞÂtimer
	E_ITEMTYPE	itemType;		// itemÀàĞÍ£¬ÊÇÁĞ±íÊ½»¹ÊÇÍ¼Æ¬ä¯ÀÀËõÂÔÍ¼
	E_MOVETYPE	moveType; 		// »¬¶¯ÀàĞÍ£¬·ÖÉÏÏÂ»¬¶¯ºÍ×óÓÒ»¬¶¯
}T_SLIPMGR;

extern T_BOOL	gb_UserkeyValid;

/**
* @brief Creat a slip manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_ITEMTYPE itemtype:ITEM_TYPE_LIST or ITEM_TYPE_IMAGE
* @param in T_RECT rect: rect
* @param in T_U32 itemW: item width
* @param in T_U32 itemH: item height
* @param in T_U32 totalItemNum: total item num
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIPMGR * the SlipMgr handle
* @retval
*/
T_SLIPMGR *SlipMgr_Creat(E_ITEMTYPE itemtype, T_RECT rect, T_U32 itemW, T_U32 itemH, T_U32 totalItemNum, E_MOVETYPE movetype);

/**
* @brief Destroy a slip manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipMgr_Destroy(T_SLIPMGR *pSlipMgr);

/**
* @brief get Item index by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @return T_S32
* @retval >=0 : index; <0 : error
*/
T_S32 SlipMgr_GetIndexById(T_SLIPMGR *pSlipMgr, T_S32 id);

/**
* @brief get item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipMgr_GetItemNum(T_SLIPMGR *pSlipMgr);

/**
* @brief Set Item id, icon and text by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 index:the Item index in mgr
* @param in T_S32 id_of_item:the Item id
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_SetItem(T_SLIPMGR *pSlipMgr, T_U32 index, T_S32 id_of_item, 
							T_pCDATA pIconLeft, T_pCDATA pIconRight,
							const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight);

/**
* @brief Change Item right text  by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in const T_U16* pTextRight:the new right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ChangeTextRightById(T_SLIPMGR *pSlipMgr, T_S32 id, const T_U16* pTextRight);

/**
* @brief show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ShowItemById(T_SLIPMGR *pSlipMgr, T_S32 id, T_BOOL bFocus);

/**
* @brief scroll show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ScrollShowItemById(T_SLIPMGR *pSlipMgr, T_S32 id, T_BOOL bFocus);


/**
* @brief add loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 count: add count
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_AddLoadItemNum(T_SLIPMGR *pSlipMgr, T_S32 count);


/**
* @brief set total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: total num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetTotalItemNum(T_SLIPMGR *pSlipMgr, T_U32 totalnum);

/**
* @brief refresh
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_Refresh(T_SLIPMGR *pSlipMgr);

/**
* @brief handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_EVT_CODE event: event
* @param in T_EVT_PARAM *pEventParm: pEventParm
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @param in/out T_U32 *focusId: focus item id
* @param in T_U32 emptyNum: empty item num
* @return T_BOOL
* @retval 
*/
T_eBACK_STATE SlipMgr_Handle(T_SLIPMGR *pSlipMgr, T_EVT_CODE event, T_EVT_PARAM *pEventParm, 
									T_S32 *count, T_U32 *loadItemNum, T_U32 *focusId, T_U32 emptyNum);

/**
* @brief check focus item is in show rect or not
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 focusId: focus item id
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_CheckFocusItem(T_SLIPMGR *pSlipMgr, T_U32 focusId);

/**
* @brief prepare to show
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @param in T_S32 offset: offset
* @param in T_U32 emptyNum: empty item num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_PrepareToShow(T_SLIPMGR *pSlipMgr, T_S32 *count, T_U32 *loadItemNum, T_S32 offset, T_U32 emptyNum);

/**
* @brief clean offset
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: ¶ÔÓ¦Êµ¼ÊitemÁ´±íÒÑ¾­¼ÓÔØÉÏµÄitemÊıÁ¿
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_ClearOffset(T_SLIPMGR *pSlipMgr, T_U32 num);

/**
* @brief set loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetLoadItemNum(T_SLIPMGR *pSlipMgr, T_U32 num);

/**
* @brief Get Cur Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMgr_GetCurStatus(T_SLIPMGR *pSlipMgr);

/**
* @brief Set Item id by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 index:the Item index in mgr
* @param in T_S32 id_of_item:the Item id
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_SetItemId(T_SLIPMGR *pSlipMgr, T_U32 index, T_S32 id_of_item);

/**
* @brief get item buf by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param inT_U32 index: item index
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_GetItemBufByIndex(T_SLIPMGR *pSlipMgr, T_U32 index, T_U8 **pbuf, T_U32 *width, T_U32 *height);

/**
* @brief set background color
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_COLOR color: color
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetBgColor(T_SLIPMGR *pSlipMgr, T_COLOR color);

/**
* @brief get display buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_GetDisBuf(T_SLIPMGR *pSlipMgr, T_U8 **pbuf, T_U32 *width, T_U32 *height);



/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr:the SlipMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_LoadItemBgImg(T_SLIPMGR *pSlipMgr);

/**
* @brief clean Item focus
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr:the SlipMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_CleanItemFocus(T_SLIPMGR *pSlipMgr);


#ifdef __cplusplus
}
#endif

#endif
