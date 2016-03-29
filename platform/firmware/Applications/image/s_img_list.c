
/*
    change log:
            for resolve the screen display dither when change the freq to the max one or from the max one ,  move the 
            freq  manage model  at the time that enter the image list mode.

*/

#include "Fwl_public.h"

#ifdef SUPPORT_IMG_BROWSE
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Eng_DataConvert.h"
#include "Eng_Debug.h"
#include "Fwl_Initialize.h"
#include "Ctl_Ebook.h"
#include "Eng_ImgConvert.h"
#include "Ctl_DisplayList.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Eng_akbmp.h"
#include "eng_font.h"
#include "Eng_KeyTranslate.h"
#include "Fwl_tscrcom.h"


#define BTN_TOP				(4)
#define BTN_TOTAL_LEFT		(25)

typedef enum {
    IMGLIST_MODE_TOTAL = 0,
    IMGLIST_MODE_DIR,
    IMGLIST_MODE_NUM
} T_IMGLIST_MODE;



typedef struct {
    T_DISPLAYLIST           displayList;
    T_MSGBOX                msgbox;

	T_IMGLIST_MODE			mode;
	T_pDATA					btn_pressdown;
	T_pDATA					btn_normal;
	T_RECT					btn_dir_rect;
	T_RECT					btn_total_rect;
} T_IMAGE_LIST_PARM;

static T_IMAGE_LIST_PARM *pImage_List_Parm;

static T_VOID resume_img_list(T_VOID);
static T_VOID suspend_img_list(T_VOID);

T_VOID     Img_list_InitRes(T_VOID)
{
	T_U32 len;
	
	if (AK_NULL == pImage_List_Parm)
	{
		return;
	}
	
	pImage_List_Parm->btn_pressdown = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_LISTMODE_BTN_PRESSDOWN, &len);
	pImage_List_Parm->btn_normal = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_LISTMODE_BTN_NORMAL, &len);
	AKBmpGetInfo(pImage_List_Parm->btn_pressdown, &pImage_List_Parm->btn_dir_rect.width, &pImage_List_Parm->btn_dir_rect.height, AK_NULL);
	pImage_List_Parm->btn_total_rect.width = pImage_List_Parm->btn_dir_rect.width;
	pImage_List_Parm->btn_total_rect.height = pImage_List_Parm->btn_dir_rect.height;


	pImage_List_Parm->btn_total_rect.left = BTN_TOTAL_LEFT;
	pImage_List_Parm->btn_total_rect.top = BTN_TOP;
	pImage_List_Parm->btn_dir_rect.left = pImage_List_Parm->btn_total_rect.left + 2 + pImage_List_Parm->btn_dir_rect.width;
	pImage_List_Parm->btn_dir_rect.top = BTN_TOP;

}


T_BOOL     Img_list_ShowBtns(T_VOID)
{
	T_USTR_FILE Utmpstr;
	T_POS left = 0;
	T_POS top = 0;
	T_U32 width = 0;
	
	if (AK_NULL == pImage_List_Parm)
	{
		return AK_FALSE;
	}

	if (IMGLIST_MODE_TOTAL == pImage_List_Parm->mode)
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, pImage_List_Parm->btn_total_rect.left,
                        pImage_List_Parm->btn_total_rect.top, pImage_List_Parm->btn_pressdown,
                        AK_NULL, AK_FALSE);

		Fwl_AkBmpDrawFromString(HRGB_LAYER, pImage_List_Parm->btn_dir_rect.left,
                        pImage_List_Parm->btn_dir_rect.top, pImage_List_Parm->btn_normal,
                        AK_NULL, AK_FALSE);		
	}
	else
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, pImage_List_Parm->btn_total_rect.left,
                        pImage_List_Parm->btn_total_rect.top, pImage_List_Parm->btn_normal,
                        AK_NULL, AK_FALSE);
		Fwl_AkBmpDrawFromString(HRGB_LAYER, pImage_List_Parm->btn_dir_rect.left,
                        pImage_List_Parm->btn_dir_rect.top, pImage_List_Parm->btn_pressdown,
                        AK_NULL, AK_FALSE);
	}

	Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_ALL));

	width = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
	left = (T_POS)(pImage_List_Parm->btn_total_rect.left + (pImage_List_Parm->btn_total_rect.width - width) / 2);
	top = (T_POS)(pImage_List_Parm->btn_total_rect.top + (pImage_List_Parm->btn_total_rect.height - GetFontHeight(CURRENT_FONT_SIZE)) / 2);
	
	Fwl_UDispSpeciString(HRGB_LAYER,
				left, top, Utmpstr, COLOR_BLACK, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));

	Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_DIR));

	width = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
	left = (T_POS)(pImage_List_Parm->btn_dir_rect.left + (pImage_List_Parm->btn_dir_rect.width - width) / 2);
	top = (T_POS)(pImage_List_Parm->btn_dir_rect.top + (pImage_List_Parm->btn_dir_rect.height - GetFontHeight(CURRENT_FONT_SIZE)) / 2);
	
	Fwl_UDispSpeciString(HRGB_LAYER,
				left, top, Utmpstr, COLOR_BLACK, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
		
	return AK_TRUE;
}

T_BOOL     Img_list_ChangeMode(T_IMGLIST_MODE mode)
{
	T_USTR_FILE Utmpstr;
	
	if (AK_NULL == pImage_List_Parm)
	{
		return AK_FALSE;
	}

	if (pImage_List_Parm->mode == mode)
	{
		return AK_FALSE;
	}

	if ((IMGLIST_MODE_TOTAL == pImage_List_Parm->mode) && (IMGLIST_MODE_DIR == mode))
	{
		pImage_List_Parm->mode = IMGLIST_MODE_DIR;
		DisplayList_SetFindSubFolder(&pImage_List_Parm->displayList, DISPLAYLIST_NOT_FIND_SUBFOLDER);
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_DIR_LIST));
		IconExplorer_SetTitleText(&pImage_List_Parm->displayList.IconExplorer, Utmpstr, COLOR_BLACK);
	}
	else
	{
		pImage_List_Parm->mode = IMGLIST_MODE_TOTAL;
		DisplayList_SetFindSubFolder(&pImage_List_Parm->displayList, DISPLAYLIST_FIND_SUBFOLDER);
		Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_ALL_IMG));
		IconExplorer_SetTitleText(&pImage_List_Parm->displayList.IconExplorer, Utmpstr, COLOR_BLACK);
	}

	IconExplorer_SetListFlag(&pImage_List_Parm->displayList.IconExplorer);

    DisplayList_CheckItemList(&pImage_List_Parm->displayList);
	
	return AK_TRUE;
}

T_BOOL Img_list_ModeHandle(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
	if (AK_NULL == pImage_List_Parm)
	{
		return AK_FALSE;
	}
	
	if (M_EVT_TOUCH_SCREEN == event)
	{
		T_RECT rect;
		T_POS posX;
		T_POS posY;

		if (eTOUCHSCR_UP == pEventParm->s.Param1)
		{
			posX = (T_POS)pEventParm->s.Param2;
	        posY = (T_POS)pEventParm->s.Param3;

			rect = pImage_List_Parm->btn_total_rect;
			if (PointInRect(&rect, posX, posY))
			{
				Img_list_ChangeMode(IMGLIST_MODE_TOTAL);
				return AK_TRUE;
			}

			rect = pImage_List_Parm->btn_dir_rect;
			if (PointInRect(&rect, posX, posY))
			{
				Img_list_ChangeMode(IMGLIST_MODE_DIR);
				return AK_TRUE;
			}
		}
	}

	if (M_EVT_USER_KEY == event)
	{
		if (kbSWA == pEventParm->c.Param1)
		{
			if (IMGLIST_MODE_TOTAL == pImage_List_Parm->mode)
			{
				Img_list_ChangeMode(IMGLIST_MODE_DIR);
			}
			else
			{
				Img_list_ChangeMode(IMGLIST_MODE_TOTAL);
			}

			return AK_TRUE;
		}
	}

	return AK_FALSE;
}

#endif
/*---------------------- BEGIN OF STATE s_ebk_list ------------------------*/
void initimg_list(void)
{
#ifdef SUPPORT_IMG_BROWSE

    T_FILE_TYPE FileType[] = {
        FILE_TYPE_BMP,
        FILE_TYPE_GIF,
        FILE_TYPE_JPG,
        FILE_TYPE_JPEG,
        FILE_TYPE_JPE,
        FILE_TYPE_PNG,        
        FILE_TYPE_NONE
    };
    
	Eng_SetKeyTranslate(Eng_ImgListTranslate);//set key translate function

    //gs.ImgSlideInterval = (T_U8)gb.nImgSlideMode;
    //Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    //Fwl_RefreshDisplay();  

    //FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_HIGH);
    
    pImage_List_Parm = (T_IMAGE_LIST_PARM *)Fwl_Malloc(sizeof(T_IMAGE_LIST_PARM));
    AK_ASSERT_PTR_VOID(pImage_List_Parm, "initimg_list(): malloc error");
	memset(pImage_List_Parm, 0, sizeof(T_IMAGE_LIST_PARM));

	Img_list_InitRes();

    DisplayList_init(&pImage_List_Parm->displayList, Fwl_GetDefPath(eIMAGE_PATH), \
                      Res_GetStringByID(eRES_STR_ALL_IMG), FileType);
	DisplayList_SetFindSubFolder(&pImage_List_Parm->displayList, DISPLAYLIST_FIND_SUBFOLDER);

    m_regResumeFunc(resume_img_list);
    m_regSuspendFunc(suspend_img_list);
#endif
}

void exitimg_list(void)
{
#ifdef SUPPORT_IMG_BROWSE

	Eng_SetDefKeyTranslate(); //restore key translate funciton to default

    //gb.nImgSlideMode  =     gs.ImgSlideInterval;
    TopBar_DisableMenuButton();
    DisplayList_Free(&pImage_List_Parm->displayList);
    pImage_List_Parm = Fwl_Free(pImage_List_Parm);

    //Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
    //Fwl_RefreshDisplay();  
    GE_ShadeCancel();
#endif
}

void paintimg_list(void)
{
#ifdef SUPPORT_IMG_BROWSE

    DisplayList_SetTopBarMenuIconState(&pImage_List_Parm->displayList);
    DisplayList_Show(&pImage_List_Parm->displayList);

	Img_list_ShowBtns();
	
 	GE_StartShade(); 
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleimg_list(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE

    T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pImage_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

	if (Img_list_ModeHandle(event, pEventParm))
	{
		return 0;
	}

	if (pImage_List_Parm->displayList.bPathTooDeep)
	{
		MsgBox_InitAfx(&pImage_List_Parm->msgbox, 1, ctFAILURE, csOUT_PATH_DEEP, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pImage_List_Parm->msgbox, MSGBOX_DELAY_1);
		pImage_List_Parm->displayList.bPathTooDeep = AK_FALSE;
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImage_List_Parm->msgbox);
	}

	if (pImage_List_Parm->displayList.bNameTooLong)
	{
		MsgBox_InitAfx(&pImage_List_Parm->msgbox, 1, ctFAILURE, csFILENAME_LONG, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pImage_List_Parm->msgbox, MSGBOX_DELAY_1);
		pImage_List_Parm->displayList.bNameTooLong = AK_FALSE;
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImage_List_Parm->msgbox);
	}
	
    DisplayListRet = DisplayList_Handler(&pImage_List_Parm->displayList, event, pEventParm);
    switch (DisplayListRet)
    {
        case eNext:
            DisplayList_SetRefresh(&pImage_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
            pFileInfo = DisplayList_Operate(&pImage_List_Parm->displayList);
            if (pFileInfo != AK_NULL)
            {
                Fwl_Print(C3, M_IMAGE, "go to thumbnail view");
                pEventParm = (T_EVT_PARAM *)(&pImage_List_Parm->displayList);
                m_triggerEvent(M_EVT_3, pEventParm);
            }
            break;

        case eMenu:
            pFileInfo = DisplayList_GetItemContentFocus(&pImage_List_Parm->displayList);
            if (pFileInfo != AK_NULL)
            {
                if (!(((pFileInfo->attrib & 0x10) == 0x10) \
                    && (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)))
                {
                    pEventParm->p.pParam1 = (T_pVOID)(&pImage_List_Parm->displayList);
                    pEventParm->p.pParam2 = AK_NULL;
					if (IconExplorer_GetItemQty(&pImage_List_Parm->displayList.IconExplorer) <= 2)
					{
						pEventParm->p.pParam3 = (T_pVOID)1;
					}
					else
					{
						pEventParm->p.pParam3 =(T_pVOID)0;
					}
                    GE_ShadeInit();
                    m_triggerEvent(M_EVT_MENU, pEventParm);
                    DisplayList_SetRefresh(&pImage_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
                }
            }
            break;
            
        case eReturn:
        case eHome:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
        default:
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_IMG_BROWSE

static T_VOID resume_img_list(T_VOID)
{
	Eng_SetKeyTranslate(Eng_ImgListTranslate);//set key translate function

    TopBar_EnableMenuButton();
	DisplayList_SetRefresh(&pImage_List_Parm->displayList, DISPLAYLIST_REFRESH_ALL);
}

static T_VOID suspend_img_list(T_VOID)
{
	Eng_SetDefKeyTranslate(); //restore key translate funciton to default
	return;
}
#endif
