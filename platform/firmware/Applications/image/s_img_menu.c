
#include "Fwl_public.h"

#ifdef SUPPORT_IMG_BROWSE
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Eng_ImgConvert.h"
#include "Fwl_Image.h"
#include "lib_image_api.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "Eng_ImgDec.h"
#include "Lib_state_api.h"
#include "Fwl_pfdisplay.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST       *pDisplayList;
    T_ICONEXPLORER      IconExplorer;
    T_MSGBOX            msgbox;
    T_pDATA             pSaveImg;
    T_pDATA             pBmpBuf;
    T_U8                file_type;
    T_BOOL              FromImgBrowse;
	T_BOOL				bNoThumb;
} T_IMG_PARM;

static T_U8 SYS_PIC_HEAD[54] =
{
    0x42, 0x4D, 0x36, 0x84, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x84, 0x03, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static T_IMG_PARM *pImgParm;
static T_VOID ConvStdQBMP(T_U8 *buffer);

static T_VOID Image2System(T_U8 pic_type);
static T_BOOL ImgMenu_WriteBuf2File(T_U32 pic_type);

#ifdef UI_USE_ICONMENU
extern T_VOID StdbPicSetBuffer(T_VOID);
#endif
#endif
/*---------------------- BEGIN OF STATE s_img_menu ------------------------*/
void initimg_menu(void)
{
#ifdef SUPPORT_IMG_BROWSE

    pImgParm = (T_IMG_PARM *)Fwl_Malloc(sizeof(T_IMG_PARM));
    AK_ASSERT_PTR_VOID(pImgParm, "initimg_menu(): malloc error");

    pImgParm->pSaveImg = AK_NULL;
	pImgParm->bNoThumb = AK_FALSE;

    MenuStructInit(&pImgParm->IconExplorer);
    GetMenuStructContent(&pImgParm->IconExplorer, mnIMG_MENU);

    TopBar_DisableMenuButton();
#endif
}

void exitimg_menu(void)
{
#ifdef SUPPORT_IMG_BROWSE

    IconExplorer_Free(&pImgParm->IconExplorer);
    pImgParm = Fwl_Free(pImgParm);
#endif
}

void paintimg_menu(void)
{
#ifdef SUPPORT_IMG_BROWSE

    IconExplorer_Show(&pImgParm->IconExplorer);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleimg_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE

    T_eBACK_STATE IconExplorerRet;
    T_FILE_INFO *FileInfo;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pImgParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_NEXT || event == M_EVT_MENU)
    {
        pImgParm->pDisplayList = (T_DISPLAYLIST *)pEventParm->p.pParam1;
        if (event == M_EVT_NEXT)
        {
            pImgParm->pBmpBuf = (T_pDATA)pEventParm->p.pParam2;
            pImgParm->FromImgBrowse = AK_TRUE;
        }
        else
        {
            pImgParm->FromImgBrowse = AK_FALSE;

			if (AK_NULL == pEventParm->p.pParam3)
			{
				pImgParm->bNoThumb = AK_FALSE;
			}
			else
			{
				pImgParm->bNoThumb = AK_TRUE;
			}
        }
        FileInfo = DisplayList_GetItemContentFocus(pImgParm->pDisplayList);
        if (FileInfo != AK_NULL)
        {
        	T_hFILE		fp = _FOPEN_FAIL;
			T_U8		typeInfo[IMG_TYPE_INFO_SIZE];
			T_FILE_TYPE	FileType = FILE_TYPE_NONE;
			T_USTR_FILE file_path;
			
            IconExplorer_DelItem(&pImgParm->IconExplorer, 70);
			
			Utl_UStrCpy(file_path, DisplayList_GetCurFilePath(pImgParm->pDisplayList));
    		Utl_UStrCat(file_path, FileInfo->name);

			if (FILE_TYPE_MAP == Utl_GetFileType(FileInfo->name))
		    {
		        FileType = FILE_TYPE_MAP;
		    }
			else
			{
				fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ);
				
			    if (_FOPEN_FAIL != fp)
			    {
					Fwl_FileRead(fp, typeInfo, IMG_TYPE_INFO_SIZE);
					FileType = ImgDec_GetImgType(typeInfo);					
					Fwl_FileClose(fp);
			    }
			}

            pImgParm->file_type = FileType;
            
            if (((FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER) || (pImgParm->FromImgBrowse == AK_FALSE) \
                || ((pImgParm->file_type != FILE_TYPE_BMP) && (pImgParm->file_type != FILE_TYPE_JPG) && (pImgParm->file_type != FILE_TYPE_PNG) && (pImgParm->file_type != FILE_TYPE_GIF))
                || (AK_NULL == pImgParm->pBmpBuf))
            {
                IconExplorer_DelItem(&pImgParm->IconExplorer, 10);
                IconExplorer_DelItem(&pImgParm->IconExplorer, 20);
                IconExplorer_DelItem(&pImgParm->IconExplorer, 30);
#ifdef UI_USE_ICONMENU
                IconExplorer_DelItem(&pImgParm->IconExplorer, 80);
#endif
            }
            if ((FileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER)
            {
                //IconExplorer_DelItem(&pImgParm->IconExplorer, 70);
                 if (!pImgParm->FromImgBrowse || pImgParm->file_type == FILE_TYPE_GIF)
                 {
                    IconExplorer_DelItem(&pImgParm->IconExplorer, 30);
                    IconExplorer_DelItem(&pImgParm->IconExplorer, 80);
                 }
                 if (pImgParm->FromImgBrowse)
                 {
                    IconExplorer_DelItem(&pImgParm->IconExplorer, 60); //delete all item
                    IconExplorer_DelItem(&pImgParm->IconExplorer, 50); //delete current item
                 }
            }

            if (Utl_UStrCmp(FileInfo->name, _T(".")) == 0)
                IconExplorer_DelItem(&pImgParm->IconExplorer, 50);

            if (Utl_UStrCmp(FileInfo->name, _T("..")) == 0)
            {
                IconExplorer_DelItem(&pImgParm->IconExplorer, 50);
                IconExplorer_DelItem(&pImgParm->IconExplorer, 60);
                //IconExplorer_DelItem(&pImgParm->IconExplorer, 70);
            }
        }
    }

    IconExplorerRet = IconExplorer_Handler(&pImgParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pImgParm->IconExplorer))
        {
        case 10:                // Save to Boot Image
            Image2System(POWER_ON_PIC);
            if (pImgParm->file_type != FILE_TYPE_GIF)
                ImgMenu_WriteBuf2File(POWER_ON_PIC);
            break;
        case 20:                // Save to PowerOff Image
            Image2System(POWER_OFF_PIC);
            if (pImgParm->file_type != FILE_TYPE_GIF)
                ImgMenu_WriteBuf2File(POWER_OFF_PIC);
            break;
        case 30:                // Save to menu background Image
            Image2System(MENU_BK_PIC);
            if (pImgParm->file_type != FILE_TYPE_GIF)
                ImgMenu_WriteBuf2File(MENU_BK_PIC);

            Menu_LoadRes();
            IconExplorer_SetRefresh(&pImgParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
            break;
#ifdef UI_USE_ICONMENU
        case 80:                // Save to StandBy Image
            Image2System(STAND_BY_PIC);
            if (pImgParm->file_type != FILE_TYPE_GIF)
                ImgMenu_WriteBuf2File(STAND_BY_PIC);

            StdbPicSetBuffer();
            break;
#endif
        case 40:
            m_triggerEvent(M_EVT_3, pEventParm);
            break;
        case 50:                // Delete
            pEventParm->p.pParam1 = (T_pVOID)pImgParm->pDisplayList;
            pEventParm->p.pParam2 = (T_pVOID)pImgParm->FromImgBrowse;
            m_triggerEvent(M_EVT_1, pEventParm);
            break;
        case 60:                // Delete All
            pEventParm->p.pParam1 = (T_pVOID)pImgParm->pDisplayList;
            pEventParm->p.pParam2 = (T_pVOID)pImgParm->FromImgBrowse;
            m_triggerEvent(M_EVT_2, pEventParm);
            break;
        case 70:
            GE_ShadeInit();
            m_triggerEvent(M_EVT_SETPATH, (T_EVT_PARAM *)pImgParm->pDisplayList);
            break;            
        default:
            break;
        }
        break;
	case eReturn:       //return
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    case eHome:         //back to home
        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
        break;
    default:
        break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_IMG_BROWSE

static T_VOID Image2System(T_U8 pic_type)
{
    T_USTR_FILE file_path;
    T_FILE_INFO *FileInfo;

    FileInfo = DisplayList_GetItemContentFocus(pImgParm->pDisplayList);
    if (FileInfo == AK_NULL || (FileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER)
        return;
    Utl_UStrCpy(file_path, DisplayList_GetCurFilePath(pImgParm->pDisplayList));
    Utl_UStrCat(file_path, FileInfo->name);

    switch (pic_type)
    {
        case POWER_ON_PIC:
            Utl_UStrCpy(gs.PathPonPic, file_path);
            MsgBox_InitAfx(&pImgParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            break;
        case POWER_OFF_PIC:
            Utl_UStrCpy(gs.PathPoffPic, file_path);
            MsgBox_InitAfx(&pImgParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            break;
        case STAND_BY_PIC:
            Utl_UStrCpy(gs.PathStdbPic, _T(STDB_CACHE_PIC));
            MsgBox_InitAfx(&pImgParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            break;
        case MENU_BK_PIC:
            Utl_UStrCpy(gs.PathMenuPic, file_path);
            MsgBox_InitAfx(&pImgParm->msgbox, 1, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            break;
        default:
            MsgBox_InitAfx(&pImgParm->msgbox, 1, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
            break;
    }

    MsgBox_SetDelay(&pImgParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImgParm->msgbox);

    return;
}

/**
 * @brief: convert buffer to standard QVGA bmp sequence
 * @the upper lines(from line 0 to 119) overturn with the lower lines(from line 239 to 120)
 * @author: YiRuoxiang
 * @date 2006-04-18
 * @param: T_U8 *buffer: bmp buffer
 * @return T_VOID
 * @retval
 */
static T_VOID ConvStdQBMP(T_U8 *buffer)
{
    T_U32 x, y;
    T_U8 R,G,B;
    T_U8* pTemp;
    T_U8* pTemDst;
    T_U32 width = Fwl_GetLcdWidth();
    T_U32 height = Fwl_GetLcdHeight();

    for (y = 0; y <= (height / 2 -1); y++)
    {
        for (x = 0; x <= (width - 1); x++)
        {
            pTemp = buffer + (height - 1 - y) * width * 3 + x * 3;
            pTemDst = buffer + y * width * 3 + x * 3;

            R = *pTemp;
            G = *(pTemp + 1);
            B = *(pTemp + 2);

            *pTemp = *(pTemDst + 2);
            *(pTemp + 1) = *(pTemDst + 1);
            *(pTemp + 2) = *pTemDst;

            *pTemDst = B;
            *(pTemDst + 1) = G;
            *(pTemDst + 2) = R;

        }
    }
}

static T_BOOL ImgMenu_WriteBuf2File(T_U32 pic_type)
{
    T_STR_FILE FilePath;
    T_USTR_FILE UFilePath;
    T_pFILE fd;
    T_BOOL flag = AK_TRUE;
    T_U32 scale;

    if (pImgParm->file_type == FILE_TYPE_GIF)
        return AK_FALSE;

	Fwl_CreateDefPath();

    pImgParm->pSaveImg = Fwl_Malloc(Fwl_GetLcdWidth() * Fwl_GetLcdHeight() * 3 + 32);
    AK_ASSERT_PTR(pImgParm->pSaveImg, "ImgMenu_WriteBuf2File pSaveImg malloc error", AK_FALSE);

    /* save BMP data to a buffer */
    GetBMP2RGBBuf888(pImgParm->pBmpBuf, pImgParm->pSaveImg, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
            0, 0, 100, 0, &scale,COLOR_BLACK);
    ConvStdQBMP(pImgParm->pSaveImg);

    switch (pic_type)
    {
    case POWER_ON_PIC:
        strcpy(FilePath, PON_CACHE_PIC);
        break;
		
    case POWER_OFF_PIC:
        strcpy(FilePath, POFF_CACHE_PIC);
        break;
		
    case MENU_BK_PIC:
        strcpy(FilePath, MENU_CACHE_PIC);
        break;
		
    case STAND_BY_PIC:
        strcpy(FilePath, STDB_CACHE_PIC);
        break;
		
    default:
        flag = AK_FALSE;
        break;
    }

    if (flag == AK_FALSE)
    {
        pImgParm->pSaveImg = Fwl_Free(pImgParm->pSaveImg);
        return AK_FALSE;
    }

    Eng_StrMbcs2Ucs(FilePath, UFilePath);
    fd = Fwl_FileOpen(UFilePath, _FMODE_CREATE, _FMODE_CREATE);
    if (fd == _FOPEN_FAIL)
    {
        pImgParm->pSaveImg = Fwl_Free(pImgParm->pSaveImg);
        return AK_FALSE;
    }

    SYS_PIC_HEAD[0x12] = Fwl_GetLcdWidth() % 256;    //width
    SYS_PIC_HEAD[0x13] = Fwl_GetLcdWidth() / 256;
    SYS_PIC_HEAD[0x16] = Fwl_GetLcdHeight() % 256;   //height
    SYS_PIC_HEAD[0x17] = Fwl_GetLcdHeight() / 256;

    Fwl_FileWrite(fd, SYS_PIC_HEAD, 54);

    Fwl_FileWrite(fd, pImgParm->pSaveImg, Fwl_GetLcdWidth() * Fwl_GetLcdHeight() * 3);
    Fwl_FileClose(fd);

    pImgParm->pSaveImg = Fwl_Free(pImgParm->pSaveImg);

    return AK_TRUE;
}

#endif
