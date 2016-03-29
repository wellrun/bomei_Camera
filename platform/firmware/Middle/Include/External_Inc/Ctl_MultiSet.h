
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  *
  * @File name: Ctl_MultiSet.h
  * @Function: This head file is designed for decalaring data and function prototype \
              of the control Ctl_MultiSet.

  * @Author: WangWei
  * @Date: 2008-05-04
  * @Version: 1.0
  */

#ifndef __CTL_MULTI_SET_H__
#define __CTL_MULTI_SET_H__

#include "ctl_global.h"
#include "Ctl_ScrollBar.h"
#include "Ctl_Title.h"
#include "eng_time.h"
#include "Ctl_IconExplorer.h"
#include "Eng_KeyMapping.h"

#ifdef __cplusplus
extern "C" {
#endif

//refresh flag
typedef enum
{
    MULTISET_REFRESH_NONE = 0, 
    MULTISET_REFRESH_TITLE,             
    MULTISET_REFRESH_ITEM,             
    MULTISET_REFRESH_FOCUS,        
    MULTISET_REFRESH_EDITAREA,      
    MULTISET_REFRESH_EDITTEXT,        
    MULTISET_REFRESH_OPTION,  
    MULTISET_REFRESH_ALL     
}T_MULTISET_REFRESH_FLAG;

typedef enum
{
    ICONSTATE_SETUP = 0,            /* be setup */
    ICONSTATE_SETUP_NOT             /* not be setup */
}T_ICON_SETUP_STATE;

typedef enum
{
    ICONSTATE_CHOOSE = 0,           /* be focus */
    ICONSTATE_CHOOSE_NOT            /* not be focus */
}T_ICON_CHOOSE_STATE;

/* control mode */
typedef enum
{
    MULTISET_MODE_NORMAL,           /* menu option mode */
    MULTISET_MODE_SETTING           /* edit mode */
}T_MULTISET_MODE;  

/* item type */
typedef enum
{
    ITEM_TYPE_ERROR = 0,            /* error  */
    ITEM_TYPE_NONE,      		    /* none */
    ITEM_TYPE_EDIT,           		/* edit   */
    ITEM_TYPE_RADIO,                /* radio button */      
    ITEM_TYPE_CHECK		            /* check box */
}T_ITEM_TYPE;

/* icon */
typedef enum
{
    ICON_LEFT = 0,              /* left  */
    ICON_RIGHT,                 /* right */
    ICON_UP,           		    /* up   */
    ICON_DOWN,                  /* down */      
}T_ICON_TYPE;

typedef struct{
    T_pDATA			pImgDt;        /* not focus background image data */
    T_pDATA			pFcsImgDt;     /* focus background image data */
} T_IMAGE_DATA;

typedef struct{
    T_IMAGE_DATA    optnBkDt;       /* option background image data */
    T_IMAGE_DATA    radioBtDt_Setup; /* option button image data when it have be setup*/
    T_IMAGE_DATA    radioBtDt;       /* option button image data when it have not be setup*/
    T_IMAGE_DATA    chkBxDt_Setup;  /* check box image data when it have be setup */
    T_IMAGE_DATA    chkBxDt;        /* check box image data when it have not be setup */
} T_OPTION_IMAGE;

typedef struct{
    T_IMAGE_DATA    itmBkDt;        /* item background image data */  
    T_IMAGE_DATA    attrbBkDt;      /* item attribute background image data */ 
} T_ITEM_IMAGE;

typedef struct{
    T_RECT			rect;           /* display rect */  
    T_COLOR			backColor;      /* display rect back color */ 
    T_pDATA			pBackData;      /* display rect background image data */ 
}T_DISP_BLOCK;

/* scroll bar strcut */
//typedef struct{
//    T_DISP_BLOCK         staticRect;    /* the static bar struct */
//    T_DISP_BLOCK         activeRect;	/* the active bar struct */
//    T_BOOL				 ScrBarIsValid;	/* scroll bar valid flag, AK_TRUE: is valid, AK_FALSE: is invalid */
//}T_SCROLL_BAR;

typedef struct{
    T_RECT			    rect;           /* text rect */     
    T_U16			    *pText;         /* text content */     
    T_U32			    textStyle;      /* text display style */
    T_COLOR		        textColor;      /* text color */
    T_COLOR		        backColor;      /* text rect back color */
    T_IMAGE_DATA        *pBackData;     /* test rect background image data */
    T_BOOL              bCursor;        /* edit cursor */
    T_RECT              cursor;         /* cursor rect */
    T_COLOR		        cursorColor;    /* cursor color */
}T_TEXT;

/* main title strcut */
typedef struct{
    T_RECT			    rect;           /* title rect */
    T_USTR_INFO			content;        /* title text */
    T_U32			    textStyle;      /* title display style */
    T_COLOR		        textColor;      /* title text color */
    T_COLOR		        backColor;      /* title background color, it is used when the image data is AK_NULL */
    T_pDATA             pBackData;      /* title background image data */
}T_MULTISET_TITLE;

/* item strcut */
typedef struct{
    T_RECT			    rect;           /* item rect */
    T_COLOR			    backColor;      /* item background color, it is used when the image data is AK_NULL */
    T_IMAGE_DATA        *pBackData;     /* item background image data */
    T_MULTISET_TITLE    title;          /* item title */  
    T_TEXT			    text;           /* item text struct */
}T_ITEM;

/* edit area strcut */
typedef struct{
    T_RECT			    rect;           /* edit area rect */
    T_COLOR			    backColor;      /* edit area background color, it is used when the image data is AK_NULL */
    T_pDATA             pBackData;      /* edit area background image data */
    T_MULTISET_TITLE    title;          /* edit area sub title */    
    T_TEXT			    text;           /* edit text struct */

    T_IMAGE_DATA        txtBckDt;       /* edit text background image */
    T_pDATA             pIcon[4];       /* edit icon image */
    T_pDATA             pButton;        /* edit validate button image */
    T_RECT              IconRct[4];     /* edit icon rect */
    T_RECT              buttonRct;      /* edit button rect */
}T_EDIT_AREA;

/* icon strcut */
typedef struct{
    T_RECT			    rect;           /* icon rect */
    T_COLOR			    backColor;      /* icon color, it is used when the image data is AK_NULL */
    T_IMAGE_DATA	    *pBackData[2];  /* icon image data */
} T_ICON;

/* option strcut */
typedef struct{
    T_RECT			    rect;           /* option rect */
    T_COLOR			    backColor;      /* option background color, it is used when the image data is AK_NULL */
    T_IMAGE_DATA        *pBkImgDt;      /* option background image data */
    T_MULTISET_TITLE    title;          /* sub title */
    T_BOOL			    choosed;        /* setup flag, AK_TRUE: be setup, AK_FALSE: not be setup */
    T_ICON		   	    icon;           /* icon struct */
}T_MULTISET_OPTION;

/* item option node */
typedef struct ITEM_OPTION 
{
    T_U32                   Id;         /* the option id */
    T_MULTISET_OPTION       option;     /* the option struct */
    struct ITEM_OPTION      *pPrevious; /* the option previous pointer */
    struct ITEM_OPTION     	*pNext;     /* the option next pointer */
} T_ITEM_OPTION_NODE;

/* hit button process call back function define */
typedef T_VOID (*T_MULTISET_HITBUTTON_CALLFUNC)(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y);

/* item key process call back function define */
typedef T_BOOL (*T_ITEM_KEYPROCESS_CALLFUNC)(T_MMI_KEYPAD *);

/* item show call back function define */
typedef T_BOOL (*T_ITEM_ShowCALLFUNC)(T_TEXT *);

/* item node struct*/
typedef struct ITEM_NODE
{
    T_U32                           id;                 /* item id */
    T_ITEM                          item;               /* item struct */
    T_ITEM_TYPE	                    itemType;           /* item option type */
    T_ITEM_OPTION_NODE	            *pOptionHead;       /* item option list head pointer */
    T_ITEM_OPTION_NODE	            *pOptionFocus;      /* item option focuss pointer */
    T_U32                           optionQty;          /* option quantity */
    T_MULTISET_HITBUTTON_CALLFUNC   fHitButtonCallFunc; /* item key process call back function */
    T_ITEM_KEYPROCESS_CALLFUNC      keyProcessCallFunc; /* item key process call back function */
    T_ITEM_ShowCALLFUNC             editShowCallFunc;   /* item show call back function */
    struct ITEM_NODE                *pPrevious;         /* previous item point */
    struct ITEM_NODE                *pNext;             /* next item point */
}T_ITEM_NODE;

/* multi set struct*/
typedef struct 
{
    T_MULTISET_MODE     ctlMode;            /* current control mode */
    T_ITEM_NODE         *pItemHead;         /* item list head pointer */
    T_ITEM_NODE         *pItemFocus;        /* item focus id */
    T_ITEM_NODE         *pFirstItem;        /* 当前屏幕显示的第一个item指针 */
    T_U32               itemQtyMax;         /* the max item quantity */
    T_U32               itemQty;            /* current item quantity */
    T_U32               itemQtyPerPage;     /* the item quantity per page of show */
    T_MULTISET_TITLE	mainTitle;          /* main title */
    T_DISP_BLOCK        backGround;         /* item menu background struct */        
    T_EDIT_AREA         editArea;           /* edit area struct */
    //T_SCROLL_BAR        scrollBar;          /* scroll bar */
    T_ITEM_IMAGE        itmImgDt;           /* item image data */
    T_OPTION_IMAGE      optnImgDt;          /* option image data */
    T_SCBAR             scrBar;             /* scroll bar */
    T_BOOL				ScrBarIsValid;	    /* scroll bar valid flag, AK_TRUE: is valid, AK_FALSE: is invalid */
    T_LEN               itmInterval;        /* option row interval*/
    T_LEN               optnInterval;       /* option row interval*/  
    T_S32               refreshFlag;        /* refresh flag */
}T_MULTISET;

/**
 * @brief   init T_MULTISET struct
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL MultiSet_Init(T_MULTISET *pMultiSet);

/**
 * @brief   free T_MULTISET struct
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_VOID
 */
T_VOID MultiSet_Free(T_MULTISET *pMultiSet);

/**
 * @brief   set control mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_MULTISET_MODE mode: control mode
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetCtlMode(T_MULTISET *pMultiSet, T_MULTISET_MODE mode);

/**
 * @brief   get current control mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_MULTISET_MODE
 * @retval  MULTISET_MODE_NORMAL: menu option mode, 
 * @retval  MULTISET_MODE_SETTING: edit mode, 
 */
T_MULTISET_MODE MultiSet_GetCtlMode(T_MULTISET *pMultiSet);

/**
 * @brief   set main title text
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   const T_U16 *pTitleText: main title text
 * @param   T_COLOR titleColor: main title text color
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetTitleText(T_MULTISET *pMultiSet, const T_U16 *pTitleText, T_COLOR titleColor);

/**
 * @brief   add item to multiSet struct 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   const T_U16 *pItemTitle: item title text
 * @param   const T_U16 *pItemText: item attribute text
 * @param   T_ITEM_TYPE itemType: item type
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_AddItemWithOption(T_MULTISET *pMultiSet, T_U32 itemId, 
                            const T_U16 *pItemTitle, const T_U16 *pItemText,
                            T_ITEM_TYPE itemType);

/**
 * @brief   add option to item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_U8 OptionId: option id
 * @param   const T_U16 *pOptionText: option conten
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_AddItemOption(T_MULTISET *pMultiSet, T_U32 ItemId, T_U8 OptionId, const T_U16 *pOptionText);

/**
 * @brief   set item key precess call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc: item key process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetItemKeyProcessCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc);

/**
 * @brief   set show edit call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc: item key process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetEditShowCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_ITEM_ShowCALLFUNC showEditCallBackFunc);

/**
 * @brief   get the focus item pointer
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_ITEM_NODE *
 * @retval  focus item pointer
 * @retval  AK_NULL
 */
 
T_ITEM_NODE *MultiSet_GetItemFocus(T_MULTISET *pMultiSet);

/**
 * @brief   set item focus by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemFocusId: the item id
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetItemFocus(T_MULTISET *pMultiSet, T_U32 itemFocusId);

/**
 * @brief   get item focus id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: item focus node pointer
 * @return  T_U32
 * @retval  item id
 */
T_U32 MultiSet_GetItemFocusId(T_ITEM_NODE *pItemFocus);

/**
 * @brief   根据item focus设置显示的第一个item指针
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetFirstItem(T_MULTISET *pMultiSet);
/**
 * @brief   设置复选框的设置状态
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @param   T_U8 optionId: the special option id
 * @param   T_BOOL setFlag:
            AK_TRUE: set 
            AK_FALSE: not set
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetCheckBoxState(T_MULTISET *pMultiSet, T_U32 itemId, 
                                 T_U32 optionId, T_BOOL setFlag);

/**
 * @brief   get the focus item's index 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_U32
 * @retval  0:error; other: success
 */
T_U32 MultiSet_GetFocusItemIndex(T_MULTISET *pMultiSet);

/**
 * @brief   get the focus item's option type 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: the special item node pointer
 * @return  T_ITEM_TYPE: 
 * @retval  ITEM_MODE_EDIT: edit;
 * @retval  ITEM_MODE_RADIO: option button
 * @retval  ITEM_MODE_CHECK: check box
 * @retval  error: pItemNode is AK_NULL
 */
T_ITEM_TYPE MultiSet_GetFocusItemType(T_ITEM_NODE *pItemNode);

/**
 * @brief   set the focus option 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @param   T_U8 optionId: the special option id
 * @return  T_BOOL
 * @retval  AK_TRUE  success, the special id option be setup
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetOptionFocus(T_MULTISET *pMultiSet, T_U32 itemId, T_U8 optionFocusId);

/**
 * @brief   get the focus option id 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: the special item node pointer
 * @return  T_U8: 
 * @retval  option id
 */
T_U32 MultiSet_GetOptionFocusId(T_ITEM_NODE *pItemFocus);

/**
 * @brief   get the focus option choose state 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the focus item node pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE : be chosen
 * @retval  AK_FALSE : not be chosen
 */
T_BOOL MultiSet_GetOptionFocusChooseState(T_ITEM_NODE *pItemFocus);

/**
 * @brief   multiSet handler
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @param   T_EVT_CODE Event: event
 * @param   T_EVT_PARAM *pPara: event parameter 
 * @return  T_eBACK_STATE: 
 * @retval  
 */
T_eBACK_STATE MultiSet_Handler(T_MULTISET *pMultiSet, T_EVT_CODE Event, T_EVT_PARAM *pParam);

/**
 * @brief   get item pointer by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @return  T_ITEM_NODE
 * @retval  item pionter
 * @retval  AK_NULL  fail
 */
T_ITEM_NODE *MultiSet_GetItemById(T_MULTISET *pMultiSet, T_U32 itemId);

/**
 * @brief   get option pointer by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: item node struct
 * @param   T_U32 itemId: the special option id
 * @return  T_ITEM_OPTION_NODE *
 * @retval  option node pionter
 * @retval  AK_NULL  fail
 */
T_ITEM_OPTION_NODE *MultiSet_GetOptionById(T_ITEM_NODE *pItemNode, T_U32 optionId);

/**
 * @brief   get text pos
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U16 *pText: display string pointer
 * @param   T_RECT *pRect: text display rect
 * @param   T_U32 textStyle: display style
 * @param   T_S16 *PosX:  save PosX
 * @param   T_S16 *PosY: save PosY
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_GetTextPos(T_U16 *pText, T_RECT *pRect, T_U32 textStyle, 
                            T_S16 *PosX, T_S16 *PosY);

/**
 * @brief   set refresh mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @param   T_MULTISET_REFRESH_FLAG refreshFlag: refresh flag
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetRefresh(T_MULTISET *pMultiSet, T_MULTISET_REFRESH_FLAG refreshFlag);

/**
 * @brief   show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_VOID
 */
T_VOID MultiSet_Show(T_MULTISET *pMultiSet);

/**
 * @brief   movie item focus to previous item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieItemFocusToPrevious(T_MULTISET *pMultiSet);

/**
 * @brief   movie item focus to next item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieItemFocusToNext(T_MULTISET *pMultiSet);

/**
 * @brief   check scroll bar is valid or not
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE:  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_CheckScrollBar(T_MULTISET *pMultiSet);

/**
 * @brief   movie option focus to previous option
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieOptionFocusToPrevious(T_ITEM_NODE *pItemFocus);

/**
 * @brief   movie option focus to next option
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieOptionFocusToNext(T_ITEM_NODE *pItemFocus);

/**
 * @brief   chang the option's choose state
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_ChangOptionChooseState(T_ITEM_NODE *pItemFocus);

/**
 * @brief   get the text rect width
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_LEN: 
 * @retval  the text rect width
 */
T_LEN  MultiSet_GetTextRectWidth(T_MULTISET *pMultiSet);

/**
 * @brief   load image data
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_IMAGE *pItemImage: T_ITEM_IMAGE struct pointer, save image data pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_LoadImageData(T_MULTISET *pMultiSet);

/**
 * @brief   set hit button precess call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_MULTISET_HITBUTTON_CALLFUNC hitButtonCallFunc: hit button process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetHitButtonCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_MULTISET_HITBUTTON_CALLFUNC fHitButtonCallFunc);


#ifdef __cplusplus
}
#endif

#endif


