/**
 * @file Ctl_MsgBox.c
 * @brief ANYKA software
 * Message box Control
 * @author ZouMai
 * @date 2002-06-24
 * @version 1.0
 */

#include "eng_string.h"
#include "eng_string_uc.h"
#include "Ctl_MsgBox.h"
#include "Ctl_Progress.h"
#include "Eng_GblString.h"
#include "Eng_KeyMapping.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Lib_res_port.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "eng_debug.h"
#include "fwl_pfKeypad.h"
#include "lib_image_api.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_tscrcom.h"
#include "fwl_display.h"

extern T_pDATA p_menu_bckgrnd;

static T_VOID MsgBox_SetButtonNum(T_MSGBOX *pMsgBox, T_U16 mode);
//static T_U8 MsgBox_GetButtonNum(T_MSGBOX *pMsgBox);
static T_VOID MsgBox_SetIconBmpId(T_MSGBOX *pMsgBox, T_U16 mode);

static T_VOID MsgBox_LoadRes(T_MSGBOX *mbox);
static T_VOID MsgBox_InitRect(T_MSGBOX *pMsgBox, T_LEN MBWidth, T_LEN MBHeight);


static T_eBACK_STATE MsgBox_UserKey_Handler(T_MSGBOX *mbox, T_MMI_KEYPAD phyKey);
static T_VOID MsgBox_HitButton_Handler(T_MSGBOX *mbox, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam);
static T_VOID   MsgBox_ShowContent(T_MSGBOX *mbox);


/**
 * @brief Initialize the variables in T_MSGBOX structure.
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param  T_LEN width
 * @param  T_LEN height: if height is 0, stands for automatically adapted
 * @param  T_U16 mode: type of the message box
 * @return T_BOOL
 * @retval
 */
T_BOOL  MsgBox_Init(T_MSGBOX *mbox, T_LEN width, T_LEN height, T_U16 mode)
{
    T_MSGBOX_RES    *pRes = AK_NULL;
    T_POS       tempLeft = 0;

    AK_FUNCTION_ENTER("MsgBox_Init");
    AK_ASSERT_PTR(mbox, "MsgBox_Init(): mbox", AK_FALSE);
    AK_ASSERT_VAL(width <= Fwl_GetLcdWidth(), "MsgBox_Init(): width", AK_FALSE);

    if (g_Font.CWIDTH <= 0 || g_Font.CWIDTH > 20)       /* maybe g_Font.CWIDTH not be assigned */
        return AK_FALSE;

    pRes = &mbox->res;

    mbox->initFlag   = INITIALIZED_FLAG;

    MsgBox_SetIconBmpId(mbox, mode);
    MsgBox_SetButtonNum(mbox, mode);
    
    mbox->horMode    = (T_U16)(mode & MSGBOX_ALLHOR_MODE);
    mbox->verMode    = (T_U16)(mode & MSGBOX_ALLVER_MODE);


    mbox->Info[0]       = 0;
    mbox->pInfo         = AK_NULL;
    mbox->delayTime     = -1;               /* default delay time: always display */
    //mbox->btnHeight     = 0;
    mbox->frameWidth    = MSGBOX_FRAME_WIDTH;
    mbox->rIntvl        = 1;
    mbox->scrbarWidth   = 0;

    mbox->MaxRowQty     = MAX_STR_LINE;
    mbox->CurRowQty     = 0;
    mbox->CurRowID      = 0;
    mbox->CurChrQty     = 0;
    mbox->ReturnLevel   = 0;

    Utl_UStrCarveInit(&mbox->UCarvedStr);

    //load image
    MsgBox_LoadRes(mbox);
    //calc image rect
    MsgBox_InitRect(mbox, width, height);
    mbox->buttonFocus = 0;

    mbox->MaxPgRow  = (T_U16)((pRes->contentRct.height + mbox->rIntvl) / (mbox->rIntvl + g_Font.CHEIGHT));

    //scroll bar
    tempLeft = pRes->MsgBkImgRct.left + pRes->MsgBkImgRct.width - g_Graph.BScBarWidth - mbox->frameWidth;
    ScBar_Init(&mbox->scBar, tempLeft, pRes->contentRct.top, g_Graph.LScBarWidth, pRes->contentRct.height, -10, SCBAR_VERTICAL);

    MsgBox_SetRefresh(mbox, CTL_REFRESH_ALL);

    AK_FUNCTION_LEAVE("MsgBox_Init");

    return AK_TRUE;
}

/**
 * @brief Initialize general message box
 *
 * @author ZouMai
 * @date 2001-9-16
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param T_U16 retLevel
 * @param T_S16 tID
 * @param T_S16 sID
 * @param T_U16 mode
 * @return T_VOID
 * @retval
 */
T_VOID  MsgBox_InitAfx(T_MSGBOX *mbox, T_U16 retLevel, T_S16 tID, T_S16 sID, T_U16 mode)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_InitAfx(): mbox");

    if (!MsgBox_Init(mbox, MSGBOX_WIDTH_DEF, MSGBOX_HEIGHT_DEF, mode))
    {
        Fwl_Print(C3, M_CTRL, "MsgBox_Init error\n");
        return;
    }

    MsgBox_AddLine(mbox, GetCustomString(sID));

    mbox->ReturnLevel = retLevel;

    return;
}

/**
 * @brief Initialize string message box
 *
 * @author ZouMai
 * @date 2001-9-16
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param T_U16 retLevel
 * @param T_pCSTR title
 * @param T_pCSTR content
 * @param T_U16 mode
 * @return T_VOID
 * @retval
 */
T_VOID  MsgBox_InitStr(T_MSGBOX *mbox, T_U16 retLevel, T_pCWSTR title, T_pCWSTR content, T_U16 mode)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_InitStr(): mbox");

    if (!MsgBox_Init(mbox, MSGBOX_WIDTH_DEF, MSGBOX_HEIGHT_DEF, mode))
    {
        Fwl_Print(C3, M_CTRL, "MsgBox_Init Error");
        return;
    }

    if (!Utl_UStrIsEmpty(content))
        MsgBox_AddLine(mbox, content);

    mbox->ReturnLevel = retLevel;

    return;
}

/**
 * @brief Show  message box button
 *
 * @author ZhuSiZhe
 * @date 2001-9-16
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param T_BOOL chaBtn
 * @return T_VOID
 * @retval
 */
T_VOID MsgBox_ShowButton(T_MSGBOX *mbox)  /* chaBtn be use in change the button style*/
{
    T_USTR_INFO     uText;
    T_MSGBOX_RES    *pRes = AK_NULL;
    T_pCDATA        pButton1Img = AK_NULL;
    T_pCDATA        pButton2Img = AK_NULL;
    T_COLOR         bt1TxtClr = COLOR_BLACK;
    T_COLOR         bt2TxtClr = COLOR_BLACK;
    T_POS           bt1Left = 0;
    T_POS           bt1Top = 0;
    T_POS           bt2Left = 0;
    T_POS           bt2Top = 0;
    T_LEN           btWidth = 0;
    T_LEN           btHeight = 0;
    
    pRes = &mbox->res;

    btWidth = pRes->BtnImgRct1.width;
    btHeight = pRes->BtnImgRct1.height;
       
    AK_ASSERT_VAL_VOID((0 == mbox->buttonNum) || (pRes->BtnImgRct1.height >= g_Font.CHEIGHT), "MsgBox_ShowButton() btHeight < g_Font.CHEIGHT");

    /* show button */
    switch (mbox->buttonNum)
    {
        case 1:
            bt1Left = pRes->BtnImgRct1.left;
            bt1Top = pRes->BtnImgRct1.top;
            pButton1Img = pRes->pBtnImg[0];
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bt1Left, bt1Top, pButton1Img, &g_Graph.TransColor, AK_FALSE);
            break;
            
        case 2:
            bt1Left = pRes->BtnImgRct1.left;
            bt1Top = pRes->BtnImgRct1.top;
            bt2Left = pRes->BtnImgRct2.left;
            bt2Top = bt1Top;
            pButton1Img = pRes->pBtnImg[0];
            pButton2Img = pRes->pBtnImg[1];
            
            if (mbox->buttonFocus == 1)
            {
                bt1TxtClr = COLOR_BLACK;
                bt2TxtClr = COLOR_WHITE;
                Fwl_AkBmpDrawFromString(HRGB_LAYER, bt1Left, bt1Top, pButton1Img, &g_Graph.TransColor, AK_FALSE);
                Fwl_AkBmpDrawFromString(HRGB_LAYER, bt2Left, bt2Top, pButton2Img, &g_Graph.TransColor, AK_FALSE);
            }
            else
            {
                bt1TxtClr = COLOR_WHITE;
                bt2TxtClr = COLOR_BLACK;
                Fwl_AkBmpDrawFromString(HRGB_LAYER, bt1Left, bt1Top, pButton2Img, &g_Graph.TransColor, AK_FALSE);
                Fwl_AkBmpDrawFromString(HRGB_LAYER, bt2Left, bt2Top, pButton1Img, &g_Graph.TransColor, AK_FALSE);
            }
            break;
            
        default:
            break;
    }

    if (mbox->buttonNum == 1 || mbox->buttonNum == 2)
    {
        /* show caption */
        switch (mbox->buttonMode)
        {
        case MSGBOX_OK:
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_OK));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt1Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt1Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), COLOR_WHITE, CURRENT_FONT_SIZE);
            break;

        case MSGBOX_EXIT:
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_RETURN));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt1Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt1Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), COLOR_WHITE, CURRENT_FONT_SIZE);
            break;
            
        case MSGBOX_YESNO:
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_YES));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt1Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt1Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt1TxtClr, CURRENT_FONT_SIZE);
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_NO));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt2Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt2Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt2TxtClr, CURRENT_FONT_SIZE);
            break;
            
        case MSGBOX_OKCANCEL:
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_OK));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt1Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt1Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt1TxtClr, CURRENT_FONT_SIZE);
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_CANCEL));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt2Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt2Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt2TxtClr, CURRENT_FONT_SIZE);
            break;
            
        case MSGBOX_RETRYCANCEL:
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_RETRY));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt1Left+((btWidth-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt1Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt1TxtClr, CURRENT_FONT_SIZE);
            Utl_UStrCpy(uText, GetCustomString(csMBOX_BUTTON_CANCEL));
            Fwl_UDispString(HRGB_LAYER, (T_POS)(bt2Left+((42-UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText)))/2)), \
                (T_POS)(bt2Top+(btHeight-g_Font.CHEIGHT)/2), uText, (T_U16)Utl_UStrLen(uText), bt2TxtClr, CURRENT_FONT_SIZE);
            break;
            
        default:
            break;
        }
    }
    return;
}

/**
 * @brief Reset message box
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_VOID
 * @retval
 */
T_VOID  MsgBox_Reset(T_MSGBOX *mbox)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_Reset(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized"); /* check mbox contorl has been initialized or not */

    mbox->Info[0]   = 0;
    mbox->pInfo     = AK_NULL;

    mbox->CurRowQty = 0;
    mbox->CurRowID  = 0;
    mbox->CurChrQty = 0;

    MsgBox_SetRefresh(mbox, CTL_REFRESH_ALL);

    return;
}

/**
 * @brief clear content and the pointer of current line is reserved
 *
 * @author 
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_VOID
 * @retval
 */
T_VOID  MsgBox_ClearContent_EX(T_MSGBOX *mbox)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_Reset(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized"); /* check mbox contorl has been initialized or not */

    mbox->Info[0]       = 0;
    mbox->pInfo         = AK_NULL;
    mbox->CurRowQty     = 0;
//    mbox->CurRowID      = 0;
    mbox->CurChrQty     = 0;

    MsgBox_SetRefresh(mbox, CTL_REFRESH_CONTENT);

    return;
}



/**
 * @brief set message box delay time
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox
 * @param T_U16 maxRow
 * @return T_VOID
 * @retval
 */
T_VOID MsgBox_SetDelay(T_MSGBOX *mbox, T_S16 delayTime)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_SetDelay(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized"); /* check mbox contorl has been initialized or not */

    mbox->delayTime = delayTime;

    return;
}

T_VOID  MsgBox_SetData(T_MSGBOX *mbox, T_U16 *pInfo)
{
    mbox->pInfo = pInfo;
    return;
}

/**
 * @brief Get the height of the messagebox
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox
 * @param T_U16 maxRow
 * @return T_VOID
 * @retval
 */
T_VOID MsgBox_SetMaxRow(T_MSGBOX *mbox, T_U16 maxRow)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_SetMaxRow(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_SetMaxRow():mbox not initialized"); /* check mbox contorl has been initialized or not */

    if (maxRow <= MAX_STR_LINE)
        mbox->MaxRowQty = maxRow;
    else
        mbox->MaxRowQty = MAX_STR_LINE;

    if (mbox->CurRowQty > maxRow)
    {
        mbox->CurRowQty = maxRow;
        if (mbox->CurRowID > maxRow)
            mbox->CurRowID = maxRow;
    }

    return;
}

/**
 * @brief Set return level
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-28
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param  T_U16 retLevel
 * @return T_VOID
 * @retval
 */
T_VOID  MsgBox_SetReturnLevel(T_MSGBOX *mbox, T_U16 retLevel)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_SetReturnLevel(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_SetReturnLevel():mbox not initialized"); /* check mbox contorl has been initialized or not */

    mbox->ReturnLevel = retLevel;

    return;
}

/**
 * @brief Set refresh flag
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-02
 * @param T_MSGBOX *menu
 * @param  T_S16 refresh
 * @return T_VOID
 * @retval
 */
T_VOID MsgBox_SetRefresh(T_MSGBOX *mbox, T_S16 refresh)
{
    AK_ASSERT_PTR_VOID(mbox, "MsgBox_SetRefresh(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_SetRefresh():mbox not initialized"); /* check mbox contorl has been initialized or not */

    if (refresh == CTL_REFRESH_NONE || refresh == CTL_REFRESH_ALL)
    {
        mbox->RefreshFlag = refresh;
    }
    else if (refresh == CTL_REFRESH_FOCUS)
    {
        if (mbox->RefreshFlag == CTL_REFRESH_NONE)
        {
            mbox->RefreshFlag = refresh;
        }
    }
    else if (refresh == CTL_REFRESH_CONTENT)
    {
        if (mbox->RefreshFlag != CTL_REFRESH_ALL)
        {
            mbox->RefreshFlag = refresh;
        }
    }
    else
    {
        if (mbox->RefreshFlag != CTL_REFRESH_ALL && mbox->RefreshFlag != CTL_REFRESH_CONTENT)
        {
            mbox->RefreshFlag = refresh;
        }
    }

    return;
}

/**
 * @brief Add string to structure T_MSGBOX and use new line.
 *        Before display the information, user must add relevant strings.
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-28
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param  T_pSTR text: information for display
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgBox_AddLine(T_MSGBOX *mbox, T_pCWSTR uText)
{
    T_U16           len;
    T_U16           uString1[5];          /*unicode text*/
    T_STR_FILE      msgstr1 = "\r\n";

    Eng_StrMbcs2Ucs(msgstr1, uString1);

    AK_ASSERT_PTR(mbox, "MsgBox_AddLine(): mbox", AK_FALSE);
    AK_ASSERT_PTR(uText, "MsgBox_AddLine(): text", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_AddLine():mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    len = (T_U16)Utl_UStrLen(uText);
    if (len == 0)   /* only add a return */
    {
        if (mbox->CurChrQty + 1 > MSGBOX_LEN_MAX)
            return AK_FALSE;
        Utl_UStrCat(mbox->Info, uString1);
        mbox->CurChrQty++;
    }
    else
    {
        if ((mbox->CurChrQty > 0) && (mbox->Info[mbox->CurChrQty-1] != UNICODE_N))
        {
            len++;      /* need add '\n' before the string */
        }
        
        if (mbox->CurChrQty + len > MSGBOX_LEN_MAX)
        {
            return AK_FALSE;
        }
        
        if ((mbox->CurChrQty > 0) && (mbox->Info[mbox->CurChrQty-1] != UNICODE_N))
        {
            Utl_UStrCat(mbox->Info, uString1);
        }
        
        Utl_UStrCat(mbox->Info, uText);
        mbox->CurChrQty = (T_U16)(mbox->CurChrQty + len);
    }

    return AK_TRUE;
}

/**
 * @brief Add information to structure T_MSGBOX.
 *        Before display the information, user must add relevant strings.
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-28
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param  T_pSTR text: information for display
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgBox_CatText(T_MSGBOX *mbox, T_pCWSTR text)
{
    T_U16           len;
    T_USTR_INFO uText;

    AK_ASSERT_PTR(mbox, "MsgBox_CatText(): mbox", AK_FALSE);
    AK_ASSERT_PTR(text, "MsgBox_CatText(): text", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    Utl_UStrCpy(uText, text);
    len = (T_U16)Utl_UStrLen(uText);

    if (mbox->CurChrQty + len > MSGBOX_LEN_MAX)
        return AK_FALSE;

    Utl_UStrCat(mbox->Info, uText);
    mbox->CurChrQty = (T_U16)(mbox->CurChrQty + len);

    return AK_TRUE;
}/* end MsgBox_CatText(T_MSGBOX *mbox, T_pSTR text) */

/**
 * @brief Get return level
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-28
 * @param const T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_BOOL
 * @retval
 */
T_U16   MsgBox_GetReturnLevel(const T_MSGBOX *mbox)
{
    AK_ASSERT_PTR(mbox, "MsgBox_GetReturnLevel(): mbox", 0);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_GetReturnLevel():mbox not initialized", 0);  

    return mbox->ReturnLevel;
}

/**
 * @brief Get the content of the message box
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-28
 * @param const T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_pSTR
 * @retval
 */
T_pWSTR  MsgBox_GetText(const T_MSGBOX *mbox)
{
    AK_ASSERT_PTR(mbox, "MsgBox_GetText(): mbox", AK_NULL);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_GetText():mbox not initialized", 0); 

    return (T_pWSTR)mbox->Info;
}

T_U8    MsgBox_GetFocusButtonID(const T_MSGBOX *mbox)
{
    AK_ASSERT_PTR(mbox, "MsgBox_GetFocusButtonID(): mbox", (T_U8)(-1));
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_GetFocusButtonID():mbox not initialized", 0);   /* check mbox contorl has been initialized or not */

    return mbox->buttonFocus;
}

/**
 * @brief Show the message box
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox
 * @return T_VOID
 * @retval
 */
T_VOID MsgBox_Show(T_MSGBOX *mbox)
{
    T_MSGBOX_RES    *pRes = AK_NULL;

    AK_ASSERT_PTR_VOID(mbox, "MsgBox_Show(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "MsgBox_Show():mbox not initialized"); 

    if (CTL_REFRESH_NONE == mbox->RefreshFlag)
    {
        MsgBox_ShowButton(mbox);
        return;
    }

    pRes = &mbox->res;

    if (AK_NULL == pRes->pMsgBkImg)
    {
        Fwl_FillSolidRect(HRGB_LAYER, pRes->MsgBkImgRct.left, pRes->MsgBkImgRct.top, \
                        pRes->MsgBkImgRct.width, pRes->MsgBkImgRct.height, 0x2FAAD4);
    }
    else
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->MsgBkImgRct.left, pRes->MsgBkImgRct.top,\
                        pRes->pMsgBkImg, &g_Graph.TransColor, AK_FALSE);
    }


    //show message content
    MsgBox_ShowContent(mbox);

    MsgBox_ShowButton(mbox);


    MsgBox_SetRefresh(mbox, CTL_REFRESH_NONE);

    return;
}

/**
 * @brief Message box control
 *
 * @author ZouMai
 * @date 2001-9-16
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @param  T_S16 DelayTime: Delay time
 * @return T_eBACK_STATE
 * @retval BACK_OFF_1: return to the previous menu
 */
T_eBACK_STATE   MsgBox_Handler(T_MSGBOX *mbox, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD    phyKey;
    T_eBACK_STATE   ret = eStay;

    AK_ASSERT_PTR(mbox, "MsgBox_Handler(): mbox", eStay);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", eStay);   /* check mbox contorl has been initialized or not */

    switch (Event) 
    {
        case M_EVT_USER_KEY:
            AK_ASSERT_PTR(pParam, "MsgBox_Handler(): pParam", eStay);
            phyKey.keyID = (T_eKEY_ID)pParam->c.Param1;
            phyKey.pressType = (T_BOOL)pParam->c.Param2;

            ret = MsgBox_UserKey_Handler(mbox, phyKey);
            break;

        case M_EVT_TOUCH_SCREEN:
            AK_ASSERT_PTR(pParam, "MsgBox_Handler(): pParam", eStay);
            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pParam->s.Param1) 
            {
                case eTOUCHSCR_DOWN:
                    break;

                case eTOUCHSCR_UP:
                    MsgBox_HitButton_Handler(mbox, &phyKey, pParam);

                    pParam->c.Param1 = (T_U8)phyKey.keyID;
                    pParam->c.Param2 = (T_U8)phyKey.pressType;
                    
                    ret = MsgBox_UserKey_Handler(mbox, phyKey);
                    break;

                case eTOUCHSCR_MOVE:
                     break;

                default:
                     break;
            }
            break;
            
        case M_EVT_PUB_TIMER:
            if (mbox->delayTime >= 0)
            {
                mbox->delayTime -= (T_S16)(pParam->w.Param2 / 1000);
                
                if (mbox->delayTime <= 0)
                {
                    MsgBox_SetRefresh(mbox, CTL_REFRESH_ALL);
                    Utl_UStrCarveFree(&mbox->UCarvedStr);
                    ret = eNext;
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

static T_eBACK_STATE MsgBox_UserKey_Handler(T_MSGBOX *mbox, T_MMI_KEYPAD phyKey)
{
    T_eBACK_STATE ret = eStay;
    T_eFUNC_KEY funcKey = fkNULL;

    if (AK_NULL == mbox)
    {
        return ret;
    }

    funcKey = MappingMsgboxKey(phyKey);
    switch (funcKey) 
    {
        case fkMBOX_COMPLETE:
            if ((mbox->buttonMode == MSGBOX_YESNO) \
                || (mbox->buttonMode == MSGBOX_OKCANCEL) \
                || (mbox->buttonMode == MSGBOX_RETRYCANCEL))
            {
                if (mbox->buttonFocus == 0)
                {
                    ret = eNext;
                }
                else
                {
                    ret = eReturn;
                }
            }
            else
            {
                ret = eNext;
            }
            Utl_UStrCarveFree(&mbox->UCarvedStr);
            break;
            
        case fkMBOX_EXIT:
            Utl_UStrCarveFree(&mbox->UCarvedStr);
            ret = eReturn;
            break;
            
        case fkMBOX_HOME:
            MsgBox_SetRefresh(mbox, CTL_REFRESH_ALL);
            Utl_UStrCarveFree(&mbox->UCarvedStr);
            ret = eHome;
            break;
            
        case fkMBOX_UP_LINE:
            MsgBox_ScrollUpLine(mbox);
            break;
            
        case fkMBOX_DOWN_LINE:
            MsgBox_ScrollDnLine(mbox);
            break;
            
        case fkMBOX_UP_PAGE:
            Fwl_Print(C3, M_CTRL, "page up\r\n");
            break;
            
        case fkMBOX_DOWN_PAGE:
            Fwl_Print(C3, M_CTRL, "page down\r\n");
            break;
            
        case fkMBOX_BUTTON_LEFT:
            if (mbox->buttonNum > 0)
            {
                mbox->buttonFocus = (mbox->buttonFocus + mbox->buttonNum - 1) % mbox->buttonNum;
            }
            else
            {
                MsgBox_ScrollUpPage(mbox);
            }
            break;
            
        case fkMBOX_BUTTON_RIGHT:
            if (mbox->buttonNum > 0)
            {
                mbox->buttonFocus = (mbox->buttonFocus + 1) % mbox->buttonNum;
            }
            else
            {
                MsgBox_ScrollDnPage(mbox);
            }
            break;

        default:
            if (phyKey.pressType == PRESS_LONG)
                Fwl_KeyStop();
            break;
    }
    return ret;
}

static T_VOID MsgBox_HitButton_Handler(T_MSGBOX *mbox, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam)
{
    T_MSGBOX_RES *pRes = AK_NULL;
    T_RECT  rect, rect1;
    T_POS   posX = 0;
    T_POS   posY = 0;
 

    if ((AK_NULL == mbox) || (AK_NULL == pPhyKey) || (AK_NULL == pParam))
    {
        return;
    }

    pRes = &mbox->res;

    posX = (T_POS)pParam->s.Param2;
    posY = (T_POS)pParam->s.Param3;
    
    if (!PointInRect(&pRes->MsgBkImgRct, posX, posY))
    {
        return;    
    }
    
    if (1 == mbox->buttonNum)
    {
        if (PointInRect(&pRes->BtnImgRct1, posX, posY))
        {
            pPhyKey->keyID = kbOK;
            pPhyKey->pressType = PRESS_SHORT;
            return;    
        }
    }
    else if (2 == mbox->buttonNum)
    {
        if (PointInRect(&pRes->BtnImgRct1, posX, posY))
        {
            if (0 == mbox->buttonFocus)
            {
                pPhyKey->keyID = kbOK;
                pPhyKey->pressType = PRESS_SHORT;
            }
            else
            {
                pPhyKey->keyID = kbLEFT;
                pPhyKey->pressType = PRESS_SHORT;
            }
            return;    
        }

        if (PointInRect(&pRes->BtnImgRct2, posX, posY))
        {
            if (0 == mbox->buttonFocus)
            {
                pPhyKey->keyID = kbRIGHT;
                pPhyKey->pressType = PRESS_SHORT;
            }
            else
            {
                pPhyKey->keyID = kbOK;
                pPhyKey->pressType = PRESS_SHORT;
            }
            return;    
        }
    }

    //scroll bar up icon
    ScBar_GetUpIconRect(&rect, &mbox->scBar);
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbUP;
        pPhyKey->pressType = PRESS_SHORT;
        return;    
    }

    //scroll bar down icon
    ScBar_GetDownIconRect(&rect, &mbox->scBar);
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbDOWN;
        pPhyKey->pressType = PRESS_SHORT;
        return;    
    }

    //scroll bar 
    rect.left = mbox->scBar.Left;
    rect.top = mbox->scBar.Top;
    rect.width = mbox->scBar.Width;
    rect.height = mbox->scBar.Height;
    
    if (PointInRect(&rect, posX, posY))
    {
        ScBar_GetLocaRect(&rect1, &mbox->scBar);

        if (posY < rect1.top)
        {
            pPhyKey->keyID = kbUP;
            pPhyKey->pressType = PRESS_LONG;
        }
        else if (posY > rect1.top + rect1.height)
        {
            pPhyKey->keyID = kbDOWN;
            pPhyKey->pressType = PRESS_LONG;
        }
        return;    
    }
}

/**
 * @brief Get the height of the messagebox
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox
 * @return T_VOID
 * @retval
 */
#if 0
static T_VOID MsgBox_CalcuHeight(T_MSGBOX *mbox)
{
    T_LEN           btHeight;

    AK_ASSERT_PTR_VOID(mbox, "MsgBox_CalcuHeight(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized"); /* check mbox contorl has been initialized or not */


    AKBmpGetInfo(gb_BmpButton1, AK_NULL, &btHeight, AK_NULL);
    if (mbox->buttonNum > 0)        /* The button is between the message content and the message box bottom */
        mbox->btnHeight = btHeight + MSGBOX_BUTTON_BOTTOM;  /* MSGBOX_BUTTON_BOTTOM pixel between message bottom */
    else
        mbox->btnHeight = 0;

    if (mbox->Height > g_Graph.LCDMSHEI[DISPLAY_LCD_0])
        mbox->Height = g_Graph.LCDMSHEI[DISPLAY_LCD_0];

    if (mbox->Height < MSGBOX_MIN_HEIGHT)
        mbox->Height = MSGBOX_MIN_HEIGHT;

    mbox->Top = g_Graph.LCDTBHEI[DISPLAY_LCD_0] + (g_Graph.LCDMSHEI[DISPLAY_LCD_0] - mbox->Height) / 2;
    mbox->cTop = mbox->Top + mbox->frameWidth;

    mbox->cHeight   = (T_U16)(mbox->Top + mbox->Height - mbox->cTop - mbox->btnHeight - mbox->frameWidth);
    mbox->MaxPgRow  = (T_U16)((mbox->cHeight + mbox->rIntvl) / (mbox->rIntvl + g_Font.CHEIGHT));

    return;
}
#endif

/**
 * @brief Browse the information.
 *        If the quantity of page greater than 1, user can browse the information.
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-27
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgBox_ScrollUpLine(T_MSGBOX *mbox)
{
    AK_ASSERT_PTR(mbox, "MsgBox_ScrollUpLine(): mbox", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    if (0 == mbox->CurRowID)        
    {  
        return AK_FALSE;
    }

    mbox->CurRowID--;
    ScBar_SetCurValue(&mbox->scBar, mbox->CurRowID);
    MsgBox_SetRefresh(mbox, CTL_REFRESH_CONTENT);

    return AK_TRUE;
}/* end MsgBox_Up(T_MSGBOX *mbox) */

T_BOOL  MsgBox_ScrollUpPage(T_MSGBOX *mbox)
{
    T_U32 i = 0;
    AK_ASSERT_PTR(mbox, "MsgBox_ScrollUpLine(): mbox", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    while(i < mbox->MaxPgRow)
    {
        if(!MsgBox_ScrollUpLine(mbox))
        {
            return AK_FALSE;
        }
        i++;
    }
    return AK_TRUE;
}/* end MsgBox_Up(T_MSGBOX *mbox) */

/**
 * @brief Get the next line of the message box
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-24
 * @param T_MSGBOX *mbox
 * @return T_BOOL
 * @retval
 */
T_BOOL MsgBox_ScrollDnLine(T_MSGBOX *mbox)
{
    AK_ASSERT_PTR(mbox, "MsgBox_ScrollDnLine(): mbox", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    if (mbox->CurRowQty <= mbox->MaxPgRow)
        return AK_FALSE;

    if (mbox->CurRowID >= mbox->CurRowQty-mbox->MaxPgRow)
        return AK_FALSE;

    mbox->CurRowID++;
    ScBar_SetCurValue(&mbox->scBar, mbox->CurRowID);
    MsgBox_SetRefresh(mbox, CTL_REFRESH_CONTENT);

    return AK_TRUE;
}
T_BOOL  MsgBox_ScrollDnPage(T_MSGBOX *mbox)
{
    T_U32 i = 0;

    AK_ASSERT_PTR(mbox, "MsgBox_ScrollDnLine(): mbox", AK_FALSE);
    AK_ASSERT_VAL(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized", AK_FALSE);    /* check mbox contorl has been initialized or not */

    while(i < mbox->MaxPgRow)
    {
        if(!MsgBox_ScrollDnLine(mbox))
        {
            return AK_FALSE;
        }
        i++;
    }
    return AK_TRUE;
}

/**
 * @brief  Show the current page information.
 *         mbox->verMode, mbox->horMode affect the location of information.
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-06-25
 * @param T_MSGBOX *mbox: T_MSGBOX structure
 * @return T_VOID
 * @retval
 */
static T_VOID MsgBox_ShowContent(T_MSGBOX *mbox)
{
    T_MSGBOX_RES    *pRes = AK_NULL;
    T_S16       i, lnNum;
    T_POS       curLeft, curTop;
    T_S32       LineLimit = 0;
//    T_U32       len = 0;
//    T_U16       txtW = 0;
//    T_U16       tmpNum = 0;
    T_POS       iconLeft = 0;
    T_POS       iconTop = 0;
    T_S16       *pInfo = AK_NULL;

    AK_ASSERT_PTR_VOID(mbox, "MsgBox_ShowContent(): mbox");
    AK_ASSERT_VAL_VOID(mbox->initFlag == INITIALIZED_FLAG, "mbox not initialized"); /* check mbox contorl has been initialized or not */

    if (CTL_REFRESH_NONE == mbox->RefreshFlag)
    {
        return;
    }


    pRes = &mbox->res;
    
    if (AK_NULL == mbox->pInfo)
    {
        pInfo = mbox->Info;
    }
    else
    {
        pInfo = mbox->pInfo;
    }

    //分割消息字符串，并计算字符串的行数。
    if ((mbox->UCarvedStr.LineNum == 0) && (mbox->UCarvedStr.MaxLen == 0))
    {
        if (0 == pRes->IconImgRct.width)   
        {   
            mbox->UCarvedStr.MaxLen = (T_U16)(pRes->MsgBkImgRct.width - 4 * mbox->frameWidth);
        }
        else
        {
            mbox->UCarvedStr.MaxLen = (T_U16)(pRes->MsgBkImgRct.width - pRes->IconImgRct.width - 2 * mbox->frameWidth - MSGBOX_INTVL_TEXT_IMG);
        }
        Utl_UStrCarve(pInfo, mbox->UCarvedStr.MaxLen, AK_NULL, &mbox->UCarvedStr, (T_U16)(mbox->UCarvedStr.MaxLen / g_Font.SCWIDTH));

        if (mbox->UCarvedStr.LineNum > mbox->MaxPgRow)   /* more than one page */
        {
            mbox->scrbarWidth = g_Graph.LScBarWidth;
            Utl_UStrCarveFree(&mbox->UCarvedStr);
            
            if (0 == pRes->IconImgRct.width)
            {
                mbox->UCarvedStr.MaxLen = (T_U16)(pRes->MsgBkImgRct.width - 4 * mbox->frameWidth - mbox->scrbarWidth - MSGBOX_INTVL_TEXT_IMG);
            }
            else
            {
                mbox->UCarvedStr.MaxLen = (T_U16)(pRes->MsgBkImgRct.width - pRes->IconImgRct.width - mbox->scrbarWidth - 2 * mbox->frameWidth - 2 * MSGBOX_INTVL_TEXT_IMG);
            }

            Utl_UStrCarve(pInfo, mbox->UCarvedStr.MaxLen, AK_NULL, &mbox->UCarvedStr, (T_U16)(mbox->UCarvedStr.MaxLen/g_Font.SCWIDTH));
        }
    }

    //检查消息行数是否超过最大行数: 没超过，消息行数值不变；超过了，将最大行数数值赋给消息行数变量。
    mbox->CurRowQty = (T_U16)mbox->UCarvedStr.LineNum;
    if (mbox->CurRowQty > mbox->MaxRowQty)
    {
        mbox->CurRowQty = mbox->MaxRowQty;
    }

    //当消息行数超过一屏显示行数时显示滚动条，并且消息内容顶格显示。
    if (mbox->CurRowQty > mbox->MaxPgRow)
    {
        //设置滚动条
        ScBar_SetValue(&mbox->scBar, mbox->CurRowID, (T_U16)(mbox->CurRowQty), mbox->MaxPgRow);

        //消息顶格显示
        curTop = pRes->contentRct.top;                    /* Top */
        lnNum = mbox->CurRowQty > mbox->MaxPgRow ? mbox->MaxPgRow : mbox->CurRowQty;
    }
    else
    {
        //消息显示区域Y坐标:top
        curTop = pRes->contentRct.top;                    /* Top */
        lnNum = mbox->CurRowQty > mbox->MaxPgRow ? mbox->MaxPgRow : mbox->CurRowQty;
    
        if (mbox->verMode == MSGBOX_VMIDDLE)            /* Middle */
        {
            curTop += (pRes->contentRct.height - lnNum * (mbox->rIntvl + g_Font.CHEIGHT) + mbox->rIntvl) / 2;
        }
        else if (mbox->verMode == MSGBOX_VBOTTOM)       /* Bottom */
        {
            curTop += pRes->contentRct.height - (lnNum * (mbox->rIntvl + g_Font.CHEIGHT) - mbox->rIntvl);
        }
    }

    //如果存在icon图片，获取并显示icon,icon的纵坐标依赖mbox->cTop
    if (AK_NULL != pRes->pIconImg)
    {
        iconLeft = (T_POS)(pRes->MsgBkImgRct.left + mbox->frameWidth);
        if (pRes->IconImgRct.width > g_Font.CHEIGHT)
        {
            iconTop = (T_POS)(curTop - ((pRes->IconImgRct.width - g_Font.CHEIGHT) / 2));
        }
        else
        {
            iconTop = curTop;
        }
        Fwl_AkBmpDrawFromString(HRGB_LAYER, iconLeft, iconTop, pRes->pIconImg, &g_Graph.TransColor, AK_FALSE);
    }
    else
    {
        Fwl_Print(C3, M_CTRL, "MsgBox_ShowContent():get icon image fail!!!");
    }

    if(lnNum+mbox->CurRowID <= mbox->CurRowQty)
    {
        LineLimit = lnNum+mbox->CurRowID;
    }
    else
    {
        LineLimit = mbox->CurRowQty;
    }
    
    for (i = mbox->CurRowID; i < LineLimit; i++)
    {
#if 1
        if (0 == pRes->IconImgRct.width)
        {
            curLeft = pRes->MsgBkImgRct.left + 2 * mbox->frameWidth;
        }
        else
        {
            curLeft = pRes->MsgBkImgRct.left + mbox->frameWidth + pRes->IconImgRct.width + MSGBOX_INTVL_TEXT_IMG;
        }
#else
        txtW = (T_U16)UGetSpeciStringWidth(mbox->UCarvedStr.String[i], CURRENT_FONT_SIZE, Utl_UStrLen(mbox->UCarvedStr.String[i]));

        if (mbox->horMode == MSGBOX_HLEFT)          /* Left */
        {
            if (mbox->scrbarWidth == 0)                      /* no scroll bar */
                curLeft = mbox->Left + mbox->iconWidth + mbox->scrbarWidth/2 + mbox->frameWidth*2;
            else
                curLeft = mbox->Left + mbox->iconWidth + mbox->frameWidth*2;
        }
        else if (mbox->horMode == MSGBOX_HRIGHT)        /* Right */
        {
            if (mbox->scrbarWidth == 0)
                curLeft = mbox->Left + mbox->Width - txtW - (mbox->scrbarWidth >> 1) - mbox->frameWidth*2;
            else
                curLeft = mbox->Left + mbox->Width - txtW - mbox->scrbarWidth - mbox->frameWidth*2;
        }
        else                                            /* Middle */
        {
            curLeft = mbox->Left+mbox->iconWidth+((mbox->Width-mbox->iconWidth-txtW-mbox->scrbarWidth) >> 1);
        }
#endif
        Fwl_UDispSpeciString(HRGB_LAYER, curLeft, curTop, mbox->UCarvedStr.String[i],
                         COLOR_WHITE, CURRENT_FONT_SIZE, mbox->UCarvedStr.UnicodeNum[i]);

        curTop += mbox->rIntvl + g_Font.CHEIGHT;
    }

    Utl_UStrCarveFree(&mbox->UCarvedStr);

    if (mbox->scrbarWidth > 0)
    {
        ScBar_Show(&mbox->scBar);
    }

    return;
}

T_BOOL MsgBox_GetRect(T_MSGBOX *mbox, T_pRECT pMsgRect)
{
    T_BOOL ret = AK_FALSE;

    if (mbox && (mbox->initFlag == INITIALIZED_FLAG) && (pMsgRect))
    {
        pMsgRect->left = mbox->res.MsgBkImgRct.left;
        pMsgRect->top = mbox->res.MsgBkImgRct.top;
        pMsgRect->width = mbox->res.MsgBkImgRct.width;
        pMsgRect->height = mbox->res.MsgBkImgRct.height;

        ret = AK_TRUE;
    }

    return ret;
}

static T_VOID MsgBox_LoadRes(T_MSGBOX *pMsgBox)
{
    T_MSGBOX_RES    *pRes = AK_NULL;
    
    if (AK_NULL == pMsgBox)
    {
        return;
    }

    pRes = &pMsgBox->res;

	//background image
    pRes->pMsgBkImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MSG_FRAME, AK_NULL);

    //button:focus and not focus
    if (1 == pMsgBox->buttonNum)
    {
        pRes->pBtnImg[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MSG_BUTTON0, AK_NULL);
        pRes->pBtnImg[1] = AK_NULL;
    }
    else if (2 == pMsgBox->buttonNum)
    {
        pRes->pBtnImg[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MSG_BUTTON0, AK_NULL);
        pRes->pBtnImg[1] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MSG_BUTTON1, AK_NULL);
    }
    else
    {
        pRes->pBtnImg[0] = AK_NULL;
        pRes->pBtnImg[1] = AK_NULL;
    }

    if ((eRES_BMP_MSG_ICONEXCL == pMsgBox->iconBmpId) \
        || (eRES_BMP_MSG_ICONINFO == pMsgBox->iconBmpId) \
        || (eRES_BMP_MSG_ICONQUES == pMsgBox->iconBmpId))
    {
        pRes->pIconImg = Res_GetBinResByID(AK_NULL, AK_TRUE, pMsgBox->iconBmpId, AK_NULL);
    }
    else
    {
        pRes->pIconImg = AK_NULL;
    }
}

static T_VOID MsgBox_InitRect(T_MSGBOX *pMsgBox, T_LEN MBWidth, T_LEN MBHeight)
{
    T_MSGBOX_RES    *pRes = AK_NULL;
    T_LEN           tmpW = 0;
    
    if (AK_NULL == pMsgBox)
    {
        return;
    }

    pRes = &pMsgBox->res;

    //background image rect
    if (AK_NULL != pRes->pMsgBkImg)
    {
        AKBmpGetInfo(pRes->pMsgBkImg, &pRes->MsgBkImgRct.width, &pRes->MsgBkImgRct.height, AK_NULL);
    }
    else
    {
        pRes->MsgBkImgRct.width = MBWidth;
        pRes->MsgBkImgRct.height = MBHeight;

        if (pRes->MsgBkImgRct.width <= 0 || pRes->MsgBkImgRct.width > Fwl_GetLcdWidth())
        {
            pRes->MsgBkImgRct.width = MSGBOX_WIDTH_DEF;
        }

        if (pRes->MsgBkImgRct.height < MSGBOX_MIN_HEIGHT)
        {
            pRes->MsgBkImgRct.height = MSGBOX_MIN_HEIGHT;
        }
    }
    pRes->MsgBkImgRct.left = (Fwl_GetLcdWidth() - pRes->MsgBkImgRct.width) / 2;
    pRes->MsgBkImgRct.top = (Fwl_GetLcdHeight() - pRes->MsgBkImgRct.height) / 2;

    //button rect
    if (1 == pMsgBox->buttonNum)
    {
        AKBmpGetInfo(pRes->pBtnImg[0], &pRes->BtnImgRct1.width, &pRes->BtnImgRct1.height, AK_NULL);

        tmpW = (pRes->MsgBkImgRct.width - pRes->BtnImgRct1.width) / 2;
        pRes->BtnImgRct1.left = pRes->MsgBkImgRct.left + tmpW;
        pRes->BtnImgRct1.top = pRes->MsgBkImgRct.top + pRes->MsgBkImgRct.height \
                                - pRes->BtnImgRct1.height - pMsgBox->frameWidth;

    }
    else if (2 == pMsgBox->buttonNum)
    {
        AKBmpGetInfo(pRes->pBtnImg[0], &pRes->BtnImgRct1.width, &pRes->BtnImgRct1.height, AK_NULL);
        pRes->BtnImgRct2.width = pRes->BtnImgRct1.width;
        pRes->BtnImgRct2.height = pRes->BtnImgRct1.height;

        tmpW =  (pRes->MsgBkImgRct.width / 2 - pRes->BtnImgRct1.width) / 2;

        pRes->BtnImgRct1.left = pRes->MsgBkImgRct.left + tmpW;
        pRes->BtnImgRct1.top = pRes->MsgBkImgRct.top + pRes->MsgBkImgRct.height \
                                - pRes->BtnImgRct1.height - pMsgBox->frameWidth;

        pRes->BtnImgRct2.left = pRes->MsgBkImgRct.left + pRes->MsgBkImgRct.width \
                                - tmpW - pRes->BtnImgRct2.width;
        pRes->BtnImgRct2.top= pRes->BtnImgRct1.top;
    }
    else
    {
        RectInit(&pRes->BtnImgRct1, 0, 0, 0, 0);
        RectInit(&pRes->BtnImgRct2, 0, 0, 0, 0);
    }

    //icon
    if (AK_NULL != pRes->pIconImg)
    {
        AKBmpGetInfo(pRes->pIconImg, &pRes->IconImgRct.width, &pRes->IconImgRct.height, AK_NULL);
    }
    else
    {
        RectInit(&pRes->IconImgRct, 0, 0, 0, 0);
    }

    //content rect
    if (0 != pRes->IconImgRct.width)
    {
        if (pRes->IconImgRct.width > g_Font.CHEIGHT)
        {
            pRes->contentRct.top = pRes->MsgBkImgRct.top \
                        + pMsgBox->frameWidth \
                        + ((pRes->IconImgRct.width - g_Font.CHEIGHT) / 2);
        }
        else
        {
            pRes->contentRct.top = pRes->MsgBkImgRct.top + pMsgBox->frameWidth;
        }

        pRes->contentRct.height   = (T_U16)(pRes->MsgBkImgRct.top + pRes->MsgBkImgRct.height \
                        - pRes->contentRct.top \
                        - pRes->BtnImgRct1.height\
                        - 2 * pMsgBox->frameWidth);
    }
    else
    {
        pRes->contentRct.top = pRes->MsgBkImgRct.top + pMsgBox->frameWidth;
        pRes->contentRct.height   = (T_U16)(pRes->MsgBkImgRct.top + pRes->MsgBkImgRct.height \
                        - pRes->contentRct.top \
                        - pRes->BtnImgRct1.height\
                        - 2 * pMsgBox->frameWidth);

    }
}

static T_VOID MsgBox_SetButtonNum(T_MSGBOX *pMsgBox, T_U16 mode)
{
    if (AK_NULL == pMsgBox)
    {
        return;
    }

    pMsgBox->buttonMode = (T_U16)(mode & MSGBOX_ALLBUTTON_MODE);

    switch (pMsgBox->buttonMode)
    {
        case MSGBOX_OK:
        case MSGBOX_EXIT:
            pMsgBox->buttonNum = 1;
            break;
            
        case MSGBOX_YESNO:
        case MSGBOX_OKCANCEL:
        case MSGBOX_RETRYCANCEL:
            pMsgBox->buttonNum = 2;
            break;
            
        default:
            pMsgBox->buttonNum = 0;
            break;
    }
}

static T_VOID MsgBox_SetIconBmpId(T_MSGBOX *pMsgBox, T_U16 mode)
{
    if (AK_NULL == pMsgBox)
    {
        return;
    }

    pMsgBox->iconMode   = (T_U16)(mode & MSGBOX_ALLICON_MODE);

    switch (pMsgBox->iconMode)
    {
        case MSGBOX_EXCLAMATION:
            pMsgBox->iconBmpId = eRES_BMP_MSG_ICONEXCL;
            break;
            
        case MSGBOX_INFORMATION:
            pMsgBox->iconBmpId = eRES_BMP_MSG_ICONINFO;
            break;
            
        case MSGBOX_QUESTION:
            pMsgBox->iconBmpId = eRES_BMP_MSG_ICONQUES;
            break;
            
        default:
            pMsgBox->iconBmpId = eRES_BINARY_NUM;
            break;
    }
}


