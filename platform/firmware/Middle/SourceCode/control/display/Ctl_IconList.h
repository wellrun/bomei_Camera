#ifndef _CTL_ICONLIST_H_
#define _CTL_ICONLIST_H_

#include "Anyka_Types.h"
#include "Gbl_Macrodef.h"
#include "Fwl_KeyHandler.h"
#include "Fwl_vme.h"
#include "Ctl_IconExplorer.h"

T_BOOL IconList_MoveItemOptionFocus(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_ReShowItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem);
T_BOOL IconList_CheckShowFocus(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_CheckScrollBar(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_SmallMoveUp(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_SmallMoveDown(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_LargeMoveLeft(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_LargeMoveRight(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_LargeMoveUp(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_LargeMoveDown(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_CheckRect(T_RECT *Rect);
T_BOOL IconList_SwapItemContent(T_ICONEXPLORER_ITEM *pItem1, T_ICONEXPLORER_ITEM *pItem2);
T_BOOL IconList_Item_DelAllOption(T_ICONEXPLORER_ITEM *pItem);
T_BOOL IconList_CheckSlipFocus(T_ICONEXPLORER *pIconExplorer);
T_eBACK_STATE IconList_UserKey_Handler(T_ICONEXPLORER *pIconExplorer, T_MMI_KEYPAD *pPhyKey);
T_VOID IconList_HitButton_Handler(T_ICONEXPLORER *pIconExplorer, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam);
T_U32 IconExplorer_GetItemOldFocusId(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_CreatSlipMgr(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconList_InsertTailItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *p);
T_BOOL IconList_LoadSlipItem(T_ICONEXPLORER *pIconExplorer, T_S32 count, T_U32 loadItemNum);


#endif	// _CTL_ICONLIST_H_