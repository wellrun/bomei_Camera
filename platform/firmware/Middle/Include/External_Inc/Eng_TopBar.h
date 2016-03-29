/******************************************
 * @File Name: Eng_TopBar.h
 * @brief Top Bar resource definition

 * Copyright (c) 2006, Anyka Co., Ltd. 
 * All rights reserved.

 * @author Ri'an Zeng
 * @date 2006-08-21
 * @version 1.0.0
 *****************************************/

 #ifndef _ENG_TOPBAR_H
 #define _ENG_TOPBAR_H


#include "anyka_types.h"
#include "Eng_Graph.h"

#define SHOW_TIME_INTERVAL          5 // 5S

#if (LCD_CONFIG_WIDTH==800)
#define TOP_BAR_HEIGHT               42
#else
#define TOP_BAR_HEIGHT               24
#endif

typedef enum {
    eTB_RES_BCKGRND = 0,        // Back ground
    eTB_RES_AUDIOSTATUS,        // Audio status
    eTB_RES_TITLE,              // Title
    eTB_RES_MENU,               // menu 
    eTB_RES_BATTERY,            // Battery
    eTB_RES_CANCEL,             //Cancel button
    eTB_RES_MAX_NUM
} T_eTB_RES_ID;                 //Top Bar resource ID



typedef struct _TB_RES_PUB_ITEM{
    T_POINT             position;
    T_U16               height;
    T_U16               width;
}T_TB_RES_PUB_ITEM;

/**Define top bar refresh control*/
#define TB_REFRESH_ALL                        0xff 
#define TB_REFRESH_AUDIO_STATUS     0x01
#define TB_REFRESH_TITLE                     0x02
#define TB_REFRESH_BATT                      0x04
#define TB_REFRESH_BKGRND                  0x08

/** @defgroup top bar interface 
 * @ingroup ENG
 */
/*@{*/
/**
 * @Brief   Init resource of top bar
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_BOOL: if the initialize successful, return AK_TRUE, otherwise return AK_FALSE. 
 */
T_BOOL TopBar_Init(T_VOID);


/**
 * @Brief   Enable show all the resource of top bar
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_EnableShow(T_VOID);


/**
 * @Brief   Disable show all the resource of top bar
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_DisableShow(T_VOID);


/**
 * @Brief   Set title
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   [IN]T_pSTR string: title string
 * @Return  T_BOOL: if the setting successful, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_SetTitle(T_pCWSTR string);


/**
 * @Brief   Enable Memu Button
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_EnableMenuButton(T_VOID);


/**
 * @Brief   Disable Memu Button
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_DisableMenuButton(T_VOID);

/**
 * @Brief   switch on/off  menu icon show
 * @Author  
 * @Data    
 * @Param  T_VOID
 * @Return  
 */
T_VOID TopBar_MenuIconShowSwitch(T_BOOL flag);

/**
 * @Brief   Set show resource
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   [IN]T_U16 refresh: refresh control
 * @Return  T_BOOL: if the Refresh successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_Show(T_U16 refresh);


/**
 * @Brief   Refresh Top Bar resource
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_Refresh(T_VOID);


/**
 * @Brief   Get Height of Top Bar 
 * @Author  Ri'an Zeng
 * @Data    2006-08-21
 * @Param   T_VOID 
 * @Return  T_U16: Height of Top Bar
 */
T_U16 TopBar_GetTopBarHeight(T_VOID);


/**
 * @Brief       Get bShowTime flag
 * @Author  Ri'an Zeng
 * @Data        2006-08-29
 * @Param   [IN] void
 * @Return  T_BOOL: show, return AK_TRUE, not show.
 */
T_BOOL TopBar_GetTimeShowFlag(T_VOID);


/**
 * @Brief       Set bShowTime flag
 * @Author  Ri'an Zeng
 * @Data        2006-08-29
 * @Param   [IN] flag
 * @Return  void 
 */
T_VOID TopBar_SetTimeShowFlag(T_BOOL flag);

T_VOID TopBar_ResetShowTimeCount(T_VOID);

T_VOID TopBar_ShowTimeDecrease(T_U32 millisecond);


/**
 * @Brief       Judge if enable show topbar
 * @Author  zhengwenbo
 * @Data        2006-08-29
 * @Param   [IN] void
 * @Return  AK_TRUE: enable AK_FALSE: disable
 */
T_BOOL TopBar_IsEnableShow(T_VOID);

/**
 * @Brief   Update battery icon
 * @Author  zhengwenbo
 * @Data    2006-09-7
 * @Param   T_VOID
 * @Return  T_VOID
 */
T_VOID TopBar_UpdateBattIcon(T_VOID);


/**
 * @Brief       scroll title
 * @Author  zhengwenbo
 * @Data        2006-09-08
 * @Param   T_TB_RES_ITEM *pTopBarItem: the pointer of topbar
 * @Return  T_BOOL: if  show successfully, return AK_TRUE, otherwise return AK_FALSE.
 */
T_BOOL TopBar_TitleTextScroll(T_VOID);

/**
 * @Brief   get battery's position
 * @Author  wangwei
 * @Data    2008-05-31
 * @Param   T_POS *pPosX: the pointer of pos x
 * @Param   T_POS *pPosY: the pointer of pos y 
 * @Return  T_BOOL
 * @Retval  if top bar has not be initialized, return AK_FALSE, otherwise return AK_TRUE.
 */
T_BOOL TopBar_GetBatteryPosition(T_POS *pPosX, T_POS *pPosY);

/**
 * @Brief   Get rect of Cancel Button
 * @Author  wangxuwen
 * @Data    2008-07-28
 * @Param   T_VOID
 * @Return  T_RECT:Rect of  Cancel Button
 */
T_RECT TopBar_GetRectofCancelButton(T_VOID);

/**
 * @Brief   Get rect of menu Button
 * @Author  
 * @Data    
 * @Param   T_VOID
 * @Return  T_RECT:Rect of  Cancel Button
 */
T_VOID TopBar_GetRectofMenuButton(T_pRECT pRect);
T_BOOL TopBar_GetMenuButtonState(T_VOID);
T_BOOL TopBar_GetMenuButtonShowState(T_VOID);
T_U8 TopBar_GetBattIconIndex(T_VOID);


/*@}*/

 #endif
 //end of Eng_TopBar.h
