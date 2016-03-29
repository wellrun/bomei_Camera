
/**
 * @file
 * @brief ANYKA software
 *
 * @author ZouMai
 * @date 2003-04-18
 * @author
 */

#include "Fwl_public.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Fwl_Image.h"
#include "Fwl_Initialize.h"
#include "Eng_AkBmp.h"
#include "Eng_ImgDec.h"
#include "Eng_DataConvert.h"
#include "Lib_geshade.h"
#include "eng_imgconvert.h"
#include "Lib_state_api.h"



/**
 * @brief ReturnDefauleProc
 *  one usful state machine return process function
 * @author ZouMai
 * @date 2002-9-9
 * @param T_eBACK_STATE event, T_EVT_PARAM *pEventParm
 */
T_VOID ReturnDefauleProc(T_eBACK_STATE state, T_EVT_PARAM *pEventParm)
{
    switch (state)
    {
    case eReturn:       //return
        GE_ShadeInit();
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    case eHome:         //back to home
        GE_ShadeInit();
        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
        break;
    default:
        break;
    }
}

T_BOOL IsPostProcessEvent(T_EVT_CODE event)
{
    if (event >= M_EVT_Z05COM_MSG || event == M_EVT_USB_IN || event == M_EVT_RTC)
        return AK_TRUE;
    else
        return AK_FALSE;
}

T_pDATA p_menu_bckgrnd = AK_NULL;
static T_VOID Menu_LoadImage(T_U8 **ppimage_buffer, T_pCWSTR filename, T_U16 dwidth, T_U16 dheight);

T_BOOL MenuStructInit(T_ICONEXPLORER *pIconExplorer)
{
    T_pCDATA    pTitleImg = AK_NULL;
    T_pCDATA    pItemBackImg = AK_NULL;
    T_RECT      TitleRect;
    T_RECT      ItemRect;
    T_U32       len = 0;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    TitleRect.left = 0;
    TitleRect.top = 0;
    TitleRect.width = Fwl_GetLcdWidth();
    TitleRect.height = TOP_BAR_HEIGHT;
    pTitleImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, &len);
    if (AK_NULL != pTitleImg)
    {
        AKBmpGetInfo(pTitleImg, &TitleRect.width, &TitleRect.height, AK_NULL);
    }
   
    if (p_menu_bckgrnd != AK_NULL)
    {
        pItemBackImg = p_menu_bckgrnd;
    }
    else
    {
        pItemBackImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_BACKGROUND, &len);
    }

    ItemRect.left = 0;
    ItemRect.top = TitleRect.height;
    ItemRect.width = Fwl_GetLcdWidth();
    ItemRect.height = Fwl_GetLcdHeight() - TitleRect.height;

    IconExplorer_Init(pIconExplorer, TitleRect, ItemRect, ICONEXPLORER_TITLE_ON | ICONEXPLORER_TITLE_TEXT_HCENTER | ICONEXPLORER_TITLE_TEXT_VCENTER | ICONEXPLORER_ITEM_FRAME);

	IconExplorer_SetTitleRect(pIconExplorer, TitleRect, COLOR_BLUE, pTitleImg);
    IconExplorer_SetTitleText(pIconExplorer, AK_NULL, COLOR_BLACK);
	
    IconExplorer_SetItemRect(pIconExplorer, ItemRect, COLOR_WHITE, pItemBackImg);
    IconExplorer_SetItemText(pIconExplorer, COLOR_WHITE, COLOR_SKYBLUE, COLOR_BLACK);
	
    IconExplorer_SetItemTransColor(pIconExplorer, g_Graph.TransColor);
	
    IconExplorer_SetItemIconStyle(pIconExplorer, ICONEXPLORER_NONEICON);
    IconExplorer_SetNoneIcon(pIconExplorer, ICONEXPLORER_ITEM_NONETEXT_HEIGHT, 1, 4);
	
//    IconExplorer_SetSmallIcon(pIconExplorer, smallIconW, smallIconH, 4, 1, 1);
//    IconExplorer_SetLargeIcon(pIconExplorer, bigIconW, bigIconH, 12, 1, 12, 8);
//    IconExplorer_SetSortIdCallBack(pIconExplorer, DisplayList_SortIdCallback);
//    IconExplorer_SetSortContentCallBack(pIconExplorer, DisplayList_SortContentCallback);
//    IconExplorer_SetListCallBack(pIconExplorer, DisplayList_ListCallback);
//    IconExplorer_SetSortMode(pIconExplorer, ICONEXPLORER_SORT_CONTENT);
    IconExplorer_SetScrollBarWidth(pIconExplorer, g_Graph.BScBarWidth);

    return AK_TRUE;
}

/**
 * @brief Load common resource for menu control
 *
 * @author @b zhengwenbo
 *
 * @author
 * @date 2006-09-6
 * @return  void
 */
T_VOID Menu_LoadRes(T_VOID)
{
    Menu_LoadImage(&p_menu_bckgrnd, _T(MENU_CACHE_PIC), Fwl_GetLcdWidth(), Fwl_GetLcdHeight());
}

T_VOID Menu_FreeRes(T_VOID)
{
    if (AK_NULL != p_menu_bckgrnd)
    {
        p_menu_bckgrnd = Fwl_Free(p_menu_bckgrnd);
        p_menu_bckgrnd = AK_NULL;
    }
}

/**
 * @brief Load image as menu's back ground picture
 *
 * @author @b zhengwenbo
 *
 * @author
 * @date 2006-09-6
 * @param [out]T_U8 *image_buffer: image data buffer
 * @param  [in]T_pCSTR filename: bmp file's path
 * @param  [in]T_U16 dwidth: width of the bmp picture
 * @param  [in]T_U16 dheight: height of the bmp picture
 * @return  void
 */
static T_VOID Menu_LoadImage(T_U8 **ppimage_buffer, T_pCWSTR filename, T_U16 dwidth, T_U16 dheight)
{
    T_U32 scale;
    T_USTR_FILE file_path;
    T_U8 *bmp_buf = AK_NULL;
    T_BOOL img_support = AK_FALSE;
    T_U32 num;
    T_U32   imgLen;
    T_pDATA pData = AK_NULL;


    Utl_UStrCpy(file_path, (T_U16 *)filename);

    if (gs.MenuPic == AK_TRUE)
    {
        if (AK_NULL == *ppimage_buffer)
	    {
	        *ppimage_buffer = (T_pDATA)Fwl_Malloc(FULL_BMP_SIZE);
	        AK_ASSERT_PTR_VOID(*ppimage_buffer, "Menu_LoadImage(): malloc error");

	        memset(*ppimage_buffer, 0, FULL_BMP_SIZE);
	    }

        bmp_buf = ImgDec_GetImageData(file_path);
        if (bmp_buf != AK_NULL)
            img_support = AK_TRUE;
    }

    if (img_support)
    {
        num = FillAkBmpHead(*ppimage_buffer, dwidth, dheight);
        GetBMP2RGBBuf(bmp_buf, (*ppimage_buffer)+num, dwidth, dheight, \
                0, 0, 100, 0, &scale,COLOR_BLACK);
    }
    else
    {
        /**Set default background picture*/
        if (AK_NULL != *ppimage_buffer)
        {
        	Fwl_Free(*ppimage_buffer);
        }
        pData = Res_StaticLoad(AK_NULL, eRES_BMP_MENU_BACKGROUND, &imgLen); 
        *ppimage_buffer = pData;
    }

    ImgDec_FreeImageData(bmp_buf);
}
