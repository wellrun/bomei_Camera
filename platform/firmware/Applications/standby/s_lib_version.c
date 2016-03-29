
#include <time.h>
#include <string.h>
#include "Fwl_public.h"
#include "Fwl_pfKeypad.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Ctl_AVIPlayer.h"
#include "Ctl_ImgBrowse.h"
#include "Ctl_Ebook.h"
#include "Ctl_AudioPlayer.h"
#include "Fwl_Initialize.h"
#include "Eng_Alarm.h"
#include "Eng_String.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "Lib_state.h"
#include "Ctl_DisplayList.h"
#include "Ctl_ListBox.h"
#include "fwl_display.h"


#if (LCD_CONFIG_WIDTH==480)
#define     RGB_ITEM_NUM           12
#else
#define     RGB_ITEM_NUM           10
#endif


typedef struct _T_LIB_PARAM{
    T_LISTBOX               libListBox;
    T_MSGBOX                msgbox;    
} T_LIB_PARAM;

static T_LIB_PARAM *pLibParm = AK_NULL;

extern T_U8 *szLibNames[MAX_LIB_NUM];
extern _GetLibVersion GetLibVersions[MAX_LIB_NUM]; 
extern T_U8   szLibMacro[MAX_LIB_NUM][100];


static T_VOID lib_version_AddList(T_VOID);


static T_VOID lib_version_AddList(T_VOID)
{
    T_LIST_ELEMENT item;
    //T_USTR_INFO casInfo;
    //T_U16 ucasIDstr[25];
    //T_U8 casIDstr[25] = {0};
    //T_U8 casIDstrTmp[50];
    //T_USTR_INFO     wStrTmp;    

    T_S32 i = 0;
    T_U8 SzTmpStr[512];
    T_U16 utlTmpStr[512];
    
    memset(&item, 0, sizeof(T_LIST_ITEM));
    
    if(pLibParm)
    {
        for(i = 0; i < MAX_LIB_NUM; i++)
        {
            AK_DEBUG_OUTPUT("lib_version_AddList i= %d\n ", i);
            item.id = i;  
            Utl_StrCpy(SzTmpStr, szLibNames[i]);
            AK_DEBUG_OUTPUT("i = %d szLibNames[i]= %s\n" , i , SzTmpStr);

            if(AK_NULL != GetLibVersions[i])
            {
                Utl_StrCat(SzTmpStr, GetLibVersions[i]());
            }            
            else if (AK_NULL != szLibMacro[i])
            {
				Utl_StrCat(SzTmpStr, szLibMacro[i]);
            }

            AK_DEBUG_OUTPUT("i = %d GetLibVersions[i]= %s\n" , i , SzTmpStr);

            Eng_StrMbcs2Ucs(SzTmpStr, utlTmpStr);
            item.pText = utlTmpStr;
            ListBox_AppendItem(&pLibParm->libListBox, &item);
        }

    }

}


T_VOID initlib_version(T_VOID)
{
    T_LISTBOX_CFG my_cfg;
    T_U16 lcd_width, lcd_height;

    pLibParm = (T_LIB_PARAM *)Fwl_Malloc(sizeof(T_LIB_PARAM));
    AK_ASSERT_PTR_VOID(pLibParm, "initlib_version(): malloc error");

    if(pLibParm != AK_NULL)
    {
        TopBar_DisableShow();
        
        lcd_width =  Fwl_GetLcdWidth();
        lcd_height = Fwl_GetLcdHeight();    
        my_cfg.TitleVisuable = VISUABLE_NORMAL;
        my_cfg.ItemVisuable = VISUABLE_NORMAL;
        my_cfg.ScBarVisuable = VISUABLE_NORMAL;
        my_cfg.LeftVisuable = VISUABLE_NORMAL;
        my_cfg.BottomVisuable = VISUABLE_NORMAL;
        my_cfg.ExitVisuable = VISUABLE_NORMAL; //VISUABLE_NONE;
        my_cfg.pListItem = AK_NULL;
        my_cfg.PageItemNum = RGB_ITEM_NUM;
        my_cfg.ScBarMode = VERTICAL;
        my_cfg.ItemTextScroll = AK_TRUE;
        
        my_cfg.Rect.left = 0;
        my_cfg.Rect.top = 0;
        my_cfg.Rect.width = lcd_width;
        my_cfg.Rect.height = lcd_height;
#ifdef OS_ANYKA        
        ListBox_Init(&pLibParm->libListBox, &my_cfg);
        ListBox_SetTitleText(&pLibParm->libListBox, Res_GetStringByID(eRES_STR_LIB_VERSION));
        lib_version_AddList();
#endif
    }

    
}

T_VOID exitlib_version(T_VOID)
{
    if(pLibParm != AK_NULL)
    {
#ifdef OS_ANYKA
        ListBox_Free(&pLibParm->libListBox);
#endif
        Fwl_Free(pLibParm);
        pLibParm = AK_NULL;
    }
    TopBar_EnableShow();

}

T_VOID paintlib_version(T_VOID)
{    
    if(pLibParm != AK_NULL)
    {
#ifdef OS_ANYKA
        ListBox_Show(&pLibParm->libListBox, AK_FALSE);
#endif
    }

    Fwl_RefreshDisplay();
}


unsigned char handlelib_version(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_BOOL  bRet = AK_FALSE;

    if (IsPostProcessEvent(event))
    {
		#ifdef OS_ANYKA
        ListBox_SetRefresh(&pLibParm->libListBox, LISTBOX_REFRESH_ALL); 
		#endif
        return 1;
    }
    
    if(AK_NULL == pLibParm)
    {
        AK_ASSERT_PTR(pLibParm, "handlelib_version,pLibParm is NULL ", 0);
        return 1;
    }
    
    //if(M_EVT_1 == event)
    {
#ifdef OS_ANYKA
        bRet = ListBox_Handle(&pLibParm->libListBox, event, pEventParm);
#endif
        switch (bRet)
        {
            case eMenu:
                //ListBox_SetRefresh(&pLibParm->libListBox, LISTBOX_REFRESH_TITLE);
                //break;                
            case eNext:
                //ListBox_SetRefresh(&pLibParm->libListBox, LISTBOX_REFRESH_TITLE);
                //break;
            case eStay:
#ifdef OS_ANYKA
                ListBox_SetRefresh(&pLibParm->libListBox, LISTBOX_REFRESH_TITLE);
#endif
                break;
            
            default:
                ReturnDefauleProc(bRet, pEventParm);
                break;
        }               
    }

    return 0;
}






