
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  *
  * @File name: Ctl_contextMenu.h
  * @Function: This head file is designed for decalaring data and function prototype \
              of the control Ctl_contextMenu.

  * @Author:WangWei
  * @Date: 2008-08-26
  * @Version: 1.0
  */

#ifndef __UTL_TOOLBAR__
#define __UTL_TOOLBAR__

#include "anyka_types.h"
#include "Fwl_pfKeypad.h"
#include "ctl_global.h"
#include "Ctl_ScrollBar.h"
#include "gbl_macrodef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMENU_REFRESH_ALL       0xff
#define CMENU_REFRESH_NONE      0x0
#define CMENU_REFRESH_FOCUS     0x1

//option 数据结构体
typedef struct OPTION_NODE{
    T_U32                   id;             	// option id
    T_U16			        neme[256];    
    struct OPTION_NODE      *pPrevious;     	// previous item point
    struct OPTION_NODE      *pNext;         	// next item point

    T_U32                   level;

    T_RECT			        rect;
    struct OPTION_NODE      *pFather;     	    // previous item point
    struct OPTION_NODE      *pChild;         	// next item point
    struct OPTION_NODE      *pFcsCh;         	// next item point
    struct OPTION_NODE      *pFrstCh;         	// next item point
    T_U32	                MaxChQty;     
    T_U32	                CurChQty;        
    T_U32		            QtyPerPg; 


}T_OPTION_NODE;

//控件管理结构体
typedef struct 
{
    T_OPTION_NODE		ListRoot;  

    T_U32               CurLevel;

    T_COLOR			    frameColor;   
    T_COLOR			    backColor;   
    T_COLOR			    focusColor; 
    T_COLOR			    textColor;   
    
    T_SCBAR             scrBar;
    
    T_U8                refreshFlag;    
} T_CONTEXTMENU;

//Interface functions//
/**
 * @brief   init T_CONTEXTMENU struct
 * @author  WangWei
 * @date    2008-08-26
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @param   T_POS posX: contextMenu rect x coordinate
 * @param   T_POS posY: contextMenu rect y coordinate
 * @param   T_COLOR frmCl: frame color
 * @param   T_COLOR bkCl: background color
 * @param   T_COLOR FcsCl: focus color
 * @param   T_COLOR TxtCl: test color
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Init(T_CONTEXTMENU *pCMenu, T_POS posX, T_POS posY, 
    T_COLOR frmCl, T_COLOR bkCl, T_COLOR FcsCl, T_COLOR TxtCl);

/**
 * @brief   add option to the List
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: the father node of the option list
 * @param   const T_U16 *pOptionName: option neme
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_AddOption(T_OPTION_NODE *pListRoot, const T_U16 *pOptionName);

/**
 * @brief   Get option pointer by the id:(0~n)
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pointer
 * @param   T_U32 id: the option id
 * @return  T_OPTION_NODE *
 * @retval  T_OPTION_NODE struct pointer
 */
T_OPTION_NODE *ContextMenu_GetOptionById(T_OPTION_NODE *pListRoot, T_U32 id);

/**
 * @brief   set contextMenu rect
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_CheckLocaRect(T_OPTION_NODE *pListRoot);


/**
 * @brief   set scroll bar of contextMenu
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_SetScrBar(T_CONTEXTMENU *pCMenu);

/**
 * @brief   show contextMenu
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Show(T_CONTEXTMENU *pCMenu);

/**
 * @brief   free contextMenu 
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Free(T_CONTEXTMENU *pCMenu);

/**
 * @brief   set list focus by the option id
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pionter
 * @param   T_U32 id: option id
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_SetFocusById(T_OPTION_NODE *pListRoot, T_U32 id);


/**
 * @brief   set list refresh flag
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct pionter
 * @param   T_S8 refresh: refresh flag
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_SetRefresh(T_CONTEXTMENU *pCMenu, T_U8 refresh);

/**
 * @brief   contextMenu handler
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct pointer
 * @param   T_EVT_CODE Event: event
 * @param   T_EVT_PARAM *pParam: event parameter 
 * @return  T_eBACK_STATE: 
 * @retval  
 */
T_eBACK_STATE ContextMenu_Handler(T_CONTEXTMENU *pCMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);

#ifdef __cplusplus
}
#endif

#endif

