/**
 * @file
 * @brief ANYKA software
 * This file is for windows graphic operation
 *
 * @author Baoli.Miao ZouMai
 * @date 2001-4-20
 * @version 1.0
 */


#include "Gbl_Global.h"
#include <string.h>
#include "Eng_DataConvert.h"
#include "Eng_Font.h"
#include "Fwl_pfDisplay.h"
#include "Eng_String.h"
#include "Eng_String_UC.h"
#include "anyka_types.h"
#include "Eng_font.h"

const T_U8 gb_bitMask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

T_FONT_DIMENSION g_Font;
T_U8 CURRENT_FONT_SIZE = FONT_16;           /* Support font size */

#ifndef OS_ANYKA
extern T_S32 Eng_MultiByteToWideChar(T_RES_LANGUAGE lang, const T_S8 *src, T_U32 srcLen, T_U32 *readedBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);
#endif

T_VOID FontInit(T_VOID)
{
	FontResize(CURRENT_FONT_SIZE);
}

T_VOID FontResize(T_U8 size)
{
	switch (size)
	{
		case FONT_12:
			CURRENT_FONT_SIZE = FONT_12; 
			g_Font.CWIDTH = 7;
			g_Font.CHEIGHT = 12;
			g_Font.SCWIDTH = 7;
			g_Font.SCHEIGHT = 12;
			break;
		case FONT_16:
			CURRENT_FONT_SIZE = FONT_16; 
			/*Default font size is 16*/
			g_Font.CWIDTH = 9;
			g_Font.CHEIGHT = 16;
			g_Font.SCWIDTH = 9;
			g_Font.SCHEIGHT = 16;
			break;
	}
}
/* end of files */

