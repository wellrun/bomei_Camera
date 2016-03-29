
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  *
  * @File name: Ctl_ToolBar.h
  * @Function: This head file is designed for decalaring data and function prototype \
              of the control Ctl_ToolBar.

  * @Author: Liuweijun
  * @Date: 2008-04-03
  * @Version: 1.0
  */

/**
  * TB  - ToolBar
  * BTN - BuTtoN
  */


#ifndef __UTL_TOOLBAR__
#define __UTL_TOOLBAR__

#include "anyka_types.h"
#include "Fwl_pfKeypad.h"
#include "ctl_global.h"
#include "gbl_macrodef.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef T_VOID (* T_fBUTTON_CLICK_CALLBACK_NORMAL)(T_U32 Value);

typedef T_VOID (* T_fBUTTON_CLICK_CALLBACK_EDIT)(T_eKEY_ID KeyId);

typedef T_VOID (* T_fBUTTON_SHOW_CALLBACK_EDIT)(T_RECT EditItemRect);

#define MAX_SHOW_TIME 		5


/////////////////////////////////////////////////////////////////////////////////////////////
//button 数据结构//
typedef enum
{
    BTN_STATE_NORMAL = 0,                           //< the button unfocused and unclicked
    BTN_STATE_FOCUS,                                //< the button focused but unclicked
    BTN_STATE_DOWN,                                 //< the button clicked
    BTN_STATE_DISABLED,                             //< the button disabled,and can't be shown.
    BTN_STATE_STATE_MAX,
    BTN_STATE_STATE_NONE
}T_BUTTON_STATE;


typedef enum
{
    BTN_TYPE_NORMAL = 0,                            //< normal button.每次点击都执行固定的调用
    BTN_TYPE_SUBMENU,                               //< this kind button has a sub-menu
    BTN_TYPE_SWITCH,                                //< a switch type button
    BTN_TYPE_EDIT,                                  //< can change the value,add/sub by step length.
    BTN_TYPE_OTHER
}T_BUTTON_TYPE;

//every button except the BTN_TYPE_EDIT type ones has several T_BUTTON_OPTION.
typedef struct _T_BUTTON_OPTION
{
    T_U32               Id;                         //< Option ID
    T_USTR_INFO         Text;                       //< button text to shown
    struct _T_BUTTON_OPTION *pPrev;                 //< previous option
    struct _T_BUTTON_OPTION *pNext;                 //< next option
}T_BUTTON_OPTION;


typedef struct _T_TB_BUTTON
{
    T_U32               Id;                         //< button id
    T_USTR_INFO         Name;                       //< button name, will be just when focused
    T_RECT              NameRect;                   //< title rect, if not focus,the nameRect is {0,0,0,0}

    T_BUTTON_TYPE       Type;                       //< button type
    T_eKEY_ID           Hotkey;                     //< hotkey.

    T_BUTTON_STATE      State;                      //< button state
    T_pDATA            StateIcon[BTN_STATE_STATE_MAX - 1];//< evevy state has a icon as content, but TB_BTN_DISABLED .

    T_U32               OptionNum;                  //< total option number of the button. the EDIT type button's OptionNum is 0
    T_RECT              OptionRect;                 //< option area rect, due to the OptionNum

    T_BUTTON_OPTION     *pHeadOption;               //< every button except T_BUTTON_EDIT type ones has options. Otherwise it's null.
    T_BUTTON_OPTION     *pFocusOption;
    T_BUTTON_OPTION     *pOldFocusOption;           //< 

    //< button who is not BTN_TYPE_EDIT type click callback function. otherwise use the EditClickCallback.
    T_fBUTTON_CLICK_CALLBACK_NORMAL NormalClickCallback; 

    //< just BTN_TYPE_EDIT type buttons have EditClickCallback and EditShowCallback. otherwise there are null!
    T_fBUTTON_CLICK_CALLBACK_EDIT   EditClickCallback;
    T_fBUTTON_SHOW_CALLBACK_EDIT    EditShowCallback;

    struct _T_TB_BUTTON *pPrev;                     //< point to previous button
    struct _T_TB_BUTTON *pNext;                     //< point to next button

}T_BUTTON, *T_pBUTTON;




//////////////////////////////////////////////////////////////////////////////////////////////
//toolbar 数据结构//

typedef T_pCDATA T_pSTATEICON_DATA[BTN_STATE_STATE_MAX - 1];

typedef enum
{
    TB_eTOP,
    TB_eBOTTOM,
    TB_eLEFT,
    TB_eRIGHT
}T_TB_DIRECTION;

//////////////////////////////////
typedef enum
{
    TB_eMODE_SHOWN_NORMAL = 0,                      //< normal shown
    TB_eMODE_SHOWN_ON_YUV = 1                       //< shownn on yuv data
}T_TB_SHOWN_MODE;


/** ToolBar move direction */
typedef enum {
    TB_MOVE_NEXT = 0,                               //< move the focus next
    TB_MOVE_PREV,                                   //< move the focus prev
    TB_MOVE_DIRECTION_NUM                           //< quantity of move focus direction
} T_TB_MOVE_DIRECTION;


typedef struct
{
    T_U8                *pY;
    T_U8                *pU;
    T_U8                *pV;
    T_U16               Width;
    T_U16               Height;
}T_YUV_BUF;

#if (LCD_CONFIG_WIDTH == 800)

#define RETURN_ICON_WIDTH  50 
#define RETURN_ICON_HEIGHT 36
#define RETURN_ICON_SPACE  6

#else
#if (LCD_CONFIG_WIDTH == 480)

#define RETURN_ICON_WIDTH  30 
#define RETURN_ICON_HEIGHT 20
#define RETURN_ICON_SPACE  5

#else
#if (LCD_CONFIG_WIDTH == 320)

#define RETURN_ICON_WIDTH  30 
#define RETURN_ICON_HEIGHT 20
#define RETURN_ICON_SPACE  5

#endif
#endif
#endif

////////////////////////////////////

typedef struct
{
    T_U16               ButtonWidth;                //< button width. all the buttons should be the same size.
    T_U16               ButtonHeight;               //< button height. all the buttons should be the same size.

    T_U16               Interval;                   //< button interval.
    T_TB_DIRECTION      Direction;                  //< direction and location of the toolbar

    T_RECT              BarRect;                    //< toolbar rect,not include the TitleRect and OptionRect.

    T_BUTTON            *pHeadBtn;                  //< the head button
    T_BUTTON            *pFirstBtn;                 //< the first shown button.Because the total button maybe more than the num can be shown
    T_BUTTON            *pFocusBtn;                 //< the focus button. it should always behind the pHeadBtn and the pFirstBtn.

    T_U32               ButtonTotalNum;
    T_U32               ButtonShownNum;             //< it shoule be calculated by the total num and button size

    T_TB_SHOWN_MODE     ShownMode;                  //< the toolbar to be shown is YUV data
    T_U8                Trans;                      //< transparence for showing the toolbar

    T_BOOL              SubMenuFlag;                //< If any BTN_TYPE_SUBMENU type button has been pressed down!

    T_COLOR             FontColor;                  //< font color
    T_COLOR             BkGrndColor;                //< back ground color of the toolbar area, including the button option area..

    //YUV bufs should be valid when the shown_mode is TB_eMODE_SHOWN_ON_YUV, otherwise there are null.
    T_YUV_BUF           BackYUV;                    //< the background yuv on which to show the toolbar.

    T_BOOL              ScrollPrev;                 //< whether need to show the prev scroll icon
    T_BOOL              ScrollBack;                 //< whether need to show the back scroll icon
    
    T_pDATA             pReturnIcon;                //<the return icon
    T_pDATA				back;						//<the back buf
    T_U8				showTimeCnt;				//<tools bar show time count
}T_TOOLBAR, *T_pTOOLBAR;


//////////////////////////////////////////////////////////////////////////////////////////////
//Interface functions//

/**
 * @brief      Init the ToolBar control
 *             After initialized, the toolbar has no button. You should add some buttons to the toolbar.
 *
 * @param[in]  pToolbar  The ToolBar to be initialized.User should malloc for the toolbar before call this function.
 * @param[in]  Rect      Toolbar rect,  coordinates relative to the upper-left corner of LCD
 * @param[in]  Direction
 * @param[in]  ButtonInterval   the interval of two buttons.
 * @param[in]  BtnWidth    Button width. All buttons added later should be the same width as this value.
 * @param[in]  BtnHeitght  Button height.All buttons added later should be the same height as this value.
 * @param[in]  ShownMode To be shown on YUV data or RGB data.
 * @param[in]  BkGrndColor ToolBar back ground color.
 * @param[in]  display transparence. the valid trans is 0~255 . If it's 0, it's not transparent and the background can't be shown.
 * @return T_BOOL
 * @retval    AK_FALSE   Create failed
 * @retval    AK_TRUE    Create success
 */
T_BOOL ToolBar_Init(T_pTOOLBAR pToolBar, T_TB_DIRECTION Direction, T_LEN windowWidth, T_LEN windowHeight, T_U16 ButtonInterval, \
                    T_U16 ButtonWidth, T_U16 ButtonHight, T_TB_SHOWN_MODE ShownMode, \
                    T_COLOR FontColor, T_COLOR BkGrndColor, T_U8 Trans);


/**
 * @brief      Add a button to a created ToolBar.
 *             You should add some buttons to a initiallized toolbar.
 *             All buttons should be the same size.
 *
 * @param[in]  pToolbar    The ToolBar to add button to.
 * @param[in]  ButtonId    Id of the button to be added.
 * @param[in]  Type        Switch type or option type.
 * @param[in]  Name        Button name.
 * @param[in]  pButtonIcon Icons for all button states to show but BTN_STATE_DISABLED.
 * @return     T_pBUTTON
 * @retval     AK_NULL     Add failed
 * @retval     Button ptr  Add success
 */
T_pBUTTON ToolBar_AddButton(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_BUTTON_TYPE Type, T_pCWSTR Name,
                         T_pSTATEICON_DATA pButtonIcon);


/**
 * @brief      Set Click callback function to a button whose type is not BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *
 * @param[in]  pToolbar    The ToolBar where the button in..
 * @param[in]  ButtonId    Id of the button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button be clicked, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetNormalButtonClickCB(T_pTOOLBAR pToolBar, T_U32 ButtonId, \
                                            T_fBUTTON_CLICK_CALLBACK_NORMAL ClickCallBack);

/**
 * @brief      Set Click callback function to a button whose type is just BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *
 * @param[in]  pToolbar    The ToolBar where the BTN_TYPE_EDIT button in..
 * @param[in]  ButtonId    Id of the BTN_TYPE_EDIT type button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button be clicked, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is not BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetEditButtonClickCB(T_pTOOLBAR pToolBar, T_U32 ButtonId, \
                                          T_fBUTTON_CLICK_CALLBACK_EDIT ClickCallBack);


/**
 * @brief      Set shown-callback function to a button whose type is just BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *             when need to show the content of the button, it will be called.
 *
 * @param[in]  pToolbar    The ToolBar where the BTN_TYPE_EDIT button in..
 * @param[in]  ButtonId    Id of the BTN_TYPE_EDIT type button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button need to show, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is not BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetEditButtonShowCB(T_pTOOLBAR pToolBar, T_U32 ButtonId, \
                                         T_fBUTTON_SHOW_CALLBACK_EDIT ShowCallBack);


/**
 * @brief      Set a button to diabled state.
 *             Then the button is existent but will not shown and can't be dealed in the toolbar..
 *             if a button is disabled, the total button number will decrease one.
 *
 * @param[in]  pToolbar  The ToolBar from which to delete a button.
 * @param[in]  ButtonId  Id of the button to be deleted.
 * @return     T_BOOL
 * @retval     AK_TRUE: Disabled the button successful. If the button has been the disabled state, return true too.
 * @retval     AK_FALSE:FAILED. Maybe the ptoolbar or the button is not exist.
 */
T_BOOL ToolBar_DisableButton(T_pTOOLBAR pToolBar, T_U32 ButtonId);


/**
 * @brief      Enable a diabled button to normal state.
 *             if a disabled button is enabled, the total button number will increase one.
 *
 * @param[in]  pToolbar  The ToolBar from which to delete a button.
 * @param[in]  ButtonId  Id of the button to be deleted.
 * @return     T_BOOL
 * @retval     AK_TRUE: enable the button successful.
 * @retval     AK_FALSE:FAILED. Maybe the ptoolbar or the button is not exist.
 */
T_VOID ToolBar_EnableButton(T_pTOOLBAR pToolBar, T_U32 ButtonId);

/**
 * @brief      Get a button from a toolbar bu ButtonId.
 *
 * @param[in]  pToolbar  The ToolBar from which to get a button.
 * @param[in]  ButtonId  Id of the button to get.
 * @return     T_pBUTTON
 * @retval     AK_NULL   no such a button
 * @retval     pointer to the button
 */
T_pBUTTON ToolBar_GetButtonById(T_pTOOLBAR pToolBar, T_U32 ButtonId);


/**
 * @brief      set the focus button by ButtonId.
 *
 * @param[in]  pToolbar  The ToolBar to set focus button.
 * @param[in]  ButtonId  Id of the button to be set as focus button.
 * @return     T_BOOL
 * @retval     AK_TRUE   Success!
 * @retval     AK_FALSE  Failed! the focus button not changed!
 */
T_BOOL ToolBar_SetFocusButtonById(T_pTOOLBAR pToolBar, T_U32 ButtonId);


/**
 * @brief      Add a option to a existent button whose type is not BTN_TYPE_EDIT.
 *             You shoule add serveral options for a button.
 *
 * @param[in]  pToolBar   the toolbar where the button is.
 * @param[in]  ButtonId   The button to which to add a option.
 * @param[in]  OptionId   option ID.
 * @param[in]  OptionText option text to show.
 * @return T_BOOL
 * @retval     AK_FALSE  Add failed
 * @retval     AK_TRUE   Add success
 */
T_BOOL ToolBar_AddOptionToButton(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_U32 OptionId, T_pCWSTR OptionText);


/**
 * @brief      Set the focus option of a button whose type is not BTN_TYPE_EDIT.
 *
 * @param[in]  pToolBar   the toolbar where the button is.
 * @param[in]  ButtonId   The button to which to add a option.
 * @param[in]  OptionId   the Option ID to be set as focus option.
 * @return T_BOOL
 * @retval     AK_FALSE  Add failed
 * @retval     AK_TRUE   Add success
 */
T_BOOL ToolBar_SetFocusOption(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_U32 OptionId);


/**
 * @brief      Set button hot key in a toolbar by button id.
 *             If you don't set any hot key, set kbNULL to it.
 *
 * @param[in]  pToolbar  The ToolBar.
 * @param[in]  ButtonId  button id of the button to be set hotkey.
 * @param[in]  key       the hot key value.
 * @return T_VOID
 */
T_VOID ToolBar_SetButtonHotKeyByID(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_eKEY_ID key);

/**
 * @brief      get the hotkey value of a button by button id.
 *
 * @param[in]  pToolbar  The ToolBar.
 * @param[in]  ButtonId  button id of the button to get hotkey.
 * @return T_eKEY_ID
 * @retval     kbNULL    this button hasn't a hotkey
 */
T_eKEY_ID ToolBar_GetButtonHotKeyByID(T_pTOOLBAR pToolBar, T_U32 ButtonId);


/**
 * @brief      the entry of show-module.
 *             the toolbar will be shown in different ways due to the T_TB_SHOWN_MODE.
 *
 * @param[in]  pToolbar  The ToolBar to be shown.
 * @retval     kbNULL    this button hasn't a hotkey
 */
T_VOID ToolBar_Show(T_pTOOLBAR pToolBar);


/**
 * @brief      The toolbar handle function.
 *             the toolbar will be shown in different ways due to the T_TB_SHOWN_MODE.
 *
 * @param[in]  pToolbar   The ToolBar to be handled.
 * @param[in]  event      event.
 * @param[in]  pParam     parameter imported.
 * @retval     T_eBACK_STATE
 */
T_eBACK_STATE ToolBar_Handler(T_pTOOLBAR pToolBar, T_EVT_CODE event, T_EVT_PARAM *pParam);


/**
 * @brief      to free all the resources allocted to the toolbar.
 *
 * @param[in]  pToolbar   The ToolBar to free.
 * @retval     T_VOID
 */
T_VOID ToolBar_Free(T_pTOOLBAR pToolBar);


/**
 * @brief      set background color for the toolbar.
 *             the option area will use the same background color.
 *
 * @param[in]  pToolbar  The ToolBar to free.
 * @param[in]  color     the background to be set.
 * @retval     T_VOID
 */
T_VOID ToolBar_SetBackColor(T_pTOOLBAR pToolBar, T_COLOR Color);


/**
 * @brief      To set the background YUV buf for the whole toolbar.
 *             This function is need to call just when the show mode is TB_eMODE_SHOWN_ON_YUV.
 *             For example, in cam preview module, the cam yuv buf should be set.
 *
 * @param[in]  pToolbar  The ToolBar .
 * @param[in]  pBackY    the background ybuf.
 * @param[in]  pBackU    the background ubuf.
 * @param[in]  pBackV    the background vbuf.
 * @param[in]  BackWidth the width of the background yuv buf.
 * @param[in]  BackHeight the height of the background yuv buf.
 * @retval     T_VOID
 */
T_VOID ToolBar_SetBackYUV(T_pTOOLBAR pToolBar, T_pCDATA pBackY, T_pCDATA pBackU, T_pCDATA pBackV, \
                          T_LEN BackWidth, T_LEN BackHeight);


#ifdef __cplusplus
}
#endif

#endif

