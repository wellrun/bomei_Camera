/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: Eng_freetype.c
* Function: freetype vector font interface 
*
* Author: songmengxing
* Date: 2012-04-01
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gbl_macrodef.h"
#include "eng_string.h"
#include "eng_dataconvert.h"
#include "fwl_osmalloc.h"
#include "fwl_osfs.h"
#include "fwl_pfdisplay.h"
#include "fwl_display.h"
#include "imagelayer.h"
#include "eng_debug.h"
#include "lib_freetype_api.h"

#ifdef SUPPORT_VFONT

#define VF_IMG_WIDTH		MAIN_LCD_WIDTH
#define VF_IMG_HEIGHT		MAIN_LCD_HEIGHT
#define VFONT_CHAR_NUM		2000
#define UNICODE_CHAR_NUM	65536
#define LOAD_ID_ERR			0xffff

typedef struct {
    T_U16			showId;
	T_U16			unicode;
	FT_Glyph 		fontSlot;
} T_VFONT_INFO;


typedef struct {
    FT_Library		library;
	FT_Face			face;
	T_U8			image[VF_IMG_HEIGHT][VF_IMG_WIDTH];
	T_U16			loadId[UNICODE_CHAR_NUM];
	T_VFONT_INFO	inf[VFONT_CHAR_NUM];
	T_U16			tailId;
	T_U16			showId;
} T_VFONT_PARM;


static T_VFONT_PARM *pVfont = AK_NULL;



/**
* @brief VFont init
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Init(T_VOID)
{
	T_S32 error = -1;
	
	pVfont = (T_VFONT_PARM *)Fwl_Malloc(sizeof(T_VFONT_PARM));
    AK_ASSERT_PTR(pVfont, "VF_Init(): malloc error", error);
	memset(pVfont, 0, sizeof(T_VFONT_PARM));
	memset(pVfont->loadId, LOAD_ID_ERR, UNICODE_CHAR_NUM * sizeof(T_U16));

	error =  FT_Init_FreeType(&pVfont->library);
	return error;
}

/**
* @brief VFont destroy
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Destroy(T_VOID)
{
	T_S32 error = -1;
	
	AK_ASSERT_PTR(pVfont, "VF_Destroy(): pVfont error", error);
	AK_ASSERT_PTR(pVfont->library, "VF_Destroy(): pVfont->library error", error);
	
	error =  FT_Done_FreeType(pVfont->library);	
	pVfont = Fwl_Free(pVfont);
	
	return error;
}


/**
* @brief load a font
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_pCSTR path : font file path
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_New_Face(T_pCSTR path)
{
	T_S32 error = -1;
	
	AK_ASSERT_PTR(pVfont, "VF_New_Face(): pVfont error", error);
	AK_ASSERT_PTR(pVfont->library, "VF_New_Face(): pVfont->library error", error);
	AK_ASSERT_PTR(path, "VF_New_Face(): path error", error);
	
	error = FT_New_Face(pVfont->library, path, 0, &pVfont->face);
	return error;
}


/**
* @brief free the font
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Done_Face(T_VOID)
{
	T_S32 error = -1;
	T_U32 i = 0;
	
	AK_ASSERT_PTR(pVfont, "VF_Done_Face(): pVfont error", error);
	AK_ASSERT_PTR(pVfont->library, "VF_Done_Face(): pVfont->library error", error);
	AK_ASSERT_PTR(pVfont->face, "VF_Done_Face(): pVfont->face error", error);
	
	for (i=0; i<VFONT_CHAR_NUM; i++)
	{
		if (AK_NULL != pVfont->inf[i].fontSlot)
		{
			FT_Done_Glyph(pVfont->inf[i].fontSlot);
		}
	}

	memset(pVfont->loadId, LOAD_ID_ERR, UNICODE_CHAR_NUM * sizeof(T_U16));
	memset(pVfont->inf, 0, VFONT_CHAR_NUM * sizeof(T_VFONT_INFO));
	
	error = FT_Done_Face(pVfont->face);
	pVfont->face = AK_NULL;
	
	return error;
}


/**
* @brief set font size
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_U32 size : font size
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Set_Size(T_U32 size)
{
	T_S32 error = -1;
	T_U32 i = 0;
	
	AK_ASSERT_PTR(pVfont, "VF_Set_Size(): pVfont error", error);
	AK_ASSERT_PTR(pVfont->library, "VF_Set_Size(): pVfont->library error", error);
	AK_ASSERT_PTR(pVfont->face, "VF_Set_Size(): pVfont->face error", error);

	if (size != gs.VF_FontSize)
	{
		for (i=0; i<VFONT_CHAR_NUM; i++)
		{
			if (AK_NULL != pVfont->inf[i].fontSlot)
			{
				FT_Done_Glyph(pVfont->inf[i].fontSlot);
			}
		}

		memset(pVfont->loadId, LOAD_ID_ERR, UNICODE_CHAR_NUM * sizeof(T_U16));
		memset(pVfont->inf, 0, VFONT_CHAR_NUM * sizeof(T_VFONT_INFO));
	}
	
	error = FT_Set_Pixel_Sizes(pVfont->face, 0, size);
	return error;
}



/**
* @brief draw bitmap
*
* @author Songmengxing
* @date 2012-04-01
* @param in FT_Bitmap*  bitmap : bitmap
* @param in FT_Int x : x
* @param in FT_Int y : y
* @return T_BOOL
* @retval 
*/
static T_BOOL VF_DrawBitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;
	T_BOOL	ret = AK_FALSE;
	

	AK_ASSERT_PTR(pVfont, "VF_GetCharWidth(): pVfont error", ret);
	AK_ASSERT_PTR(bitmap, "VF_GetCharWidth(): bitmap error", ret);


	for ( i = x, p = 0; i < x_max; i++, p++ )
	{
		for ( j = y, q = 0; j < y_max; j++, q++ )
		{
			if ( i < 0 
				|| j < 0 
				|| i >= VF_IMG_WIDTH 
				|| j >= VF_IMG_HEIGHT )
			continue;

			pVfont->image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}

	return AK_TRUE;
}

/**
* @brief show image to a buffer
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_U8* pbuf : buffer to show
* @param in T_U32 imgW : buf width
* @param in T_U32 imgH : buf height
* @param in T_POS x :
* @param in T_POS x_max :
* @param in T_POS y :
* @param in T_POS y_max :
* @param in T_U32 color : color
* @return T_BOOL
* @retval 
*/
static T_BOOL VF_ShowImg(T_U8* pbuf, T_U32 imgW, T_U32 imgH, T_POS x, T_POS x_max, T_POS y, T_POS y_max, T_U32 color)
{
#ifdef OS_WIN32
  int  i, j;
	T_U8	*pBuffer = AK_NULL;
	T_U8	*pData = AK_NULL;
	float	alpha = 0;
	T_U8 r_src,g_src,b_src;
	T_BOOL	ret = AK_FALSE;

	AK_ASSERT_PTR(pVfont, "VF_ShowImg(): pVfont error", ret);


	if (((T_pImgLay)Fwl_hRGBLayer)->pData == pbuf)
	{
		pBuffer = Fwl_GetDispMemory() + (y * imgW + x)*3;
	}
	else
	{
		pBuffer = pbuf + (y * imgW + x)*3;
	}

	pData = (T_U8*)(&(pVfont->image[0][x]));
	
	for(i=y; (i<y_max)&&(i<(T_S32)imgH); i++)
    {
		for (j=x; (j<x_max)&&(j<(T_S32)imgW); j++)
		{
			if (0 == *pData)
			{
				pData++;
				pBuffer += 3;
			}
			else if (255 == *pData)
			{
				*pBuffer++ = (T_U8)(color >> 16);	
		    	*pBuffer++ = (T_U8)(color >> 8);
				*pBuffer++ = (T_U8)(color);
				pData++;
			}
			else
			{
				alpha = (float)*pData / 255;
				r_src = (T_U8)(color >> 16);	// R
		    	g_src = (T_U8)(color >> 8);		// G
		    	b_src = (T_U8)(color);			// B
				*pBuffer++ = (T_U8)((float)*pBuffer * (1 - alpha) + r_src * alpha);
				*pBuffer++ = (T_U8)((float)*pBuffer * (1 - alpha) + g_src * alpha);
				*pBuffer++ = (T_U8)((float)*pBuffer * (1 - alpha) + b_src * alpha);
				pData++;
			}
		}

		pBuffer += (imgW - (x_max - x)) * 3;
		pData += VF_IMG_WIDTH - (x_max - x);
    }
#else
	int  i, j;
	T_U8 r_src,g_src,b_src;
	T_U8 r_dst,g_dst,b_dst;
	T_U8 r, g, b;
	T_U8 *pBuffer = AK_NULL;
	T_U8 *pData = AK_NULL;
	float alpha = 0;
	T_U16 temp = 0;
	T_BOOL	ret = AK_FALSE;

	AK_ASSERT_PTR(pVfont, "VF_ShowImg(): pVfont error", ret);


	pBuffer = pbuf + (y * imgW + x)*2;
	pData = (T_U8*)(&(pVfont->image[0][x]));
	
	for(i=y; (i<y_max)&&(i<(T_S32)imgH); i++)
    {
		for (j=x; (j<x_max)&&(j<(T_S32)imgW); j++)
		{
			if (0 == *pData)
			{
				pData++;
				pBuffer += 2;
			}
			else if (255 == *pData)
			{
				r = (T_U8)(color >> 16);	// R
		    	g = (T_U8)(color >> 8);		// G
		    	b = (T_U8)(color);			// B

				*pBuffer++ = ((b & 0xf8) >> 3) | ((g & 0x1c) << 3);	// b, g
		    	*pBuffer++ = (r & 0xf8)  | ((g & 0xe0) >> 5);		// r, g  
				pData++;
			}
			else
			{
				alpha = (float)*pData / 255;
				r_src = (T_U8)(color >> 16);	// R
		    	g_src = (T_U8)(color >> 8);		// G
		    	b_src = (T_U8)(color);			// B

				temp = (*pBuffer) | (*(pBuffer+1)<<8);
				r_dst = (T_U8)((temp>>11)<<3);
				g_dst = (T_U8)((temp>>5)<<2);
				b_dst = (T_U8)(temp<<3);


				r = (T_U8)(r_dst * (1 - alpha) + r_src * alpha);
				g = (T_U8)(g_dst * (1 - alpha) + g_src * alpha);
				b = (T_U8)(b_dst * (1 - alpha) + b_src * alpha);
		    	
				*pBuffer++ = ((b & 0xf8) >> 3) | ((g & 0x1c) << 3);	// b, g
		    	*pBuffer++ = (r & 0xf8)  | ((g & 0xe0) >> 5);		// r, g  
				pData++;
			}
		}

		pBuffer += (imgW - (x_max - x)) * 2;
		pData += VF_IMG_WIDTH - (x_max - x);
    }
#endif
	return AK_TRUE;
}


/**
* @brief load a char
*
* @author Songmengxing
* @date 2012-04-01
* @param in FT_Face face : the face
* @param in const T_U16 chr : unicode id
* @return T_BOOL
* @retval 
*/
static T_BOOL VF_LoadChar(FT_Face face, const T_U16 chr)
{
	FT_GlyphSlot  slot ;
	FT_Error	  error;
	T_BOOL ret = AK_FALSE;
	T_U16 i = 0;

	AK_ASSERT_PTR(pVfont, "VF_LoadChar(): pVfont error", ret);
	AK_ASSERT_PTR(pVfont->library, "VF_LoadChar(): pVfont->library error", ret);
	AK_ASSERT_PTR(face, "VF_LoadChar(): face error", ret);

	slot = face->glyph;
	

	if (LOAD_ID_ERR == pVfont->loadId[chr])
	{			
		error = FT_Load_Char(face, chr, FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
		
		if (error)
		{
			Fwl_Print(C4, M_VFONT, "FT_Load_Char error\r\n");
			return AK_FALSE;
		}

		if (pVfont->tailId < VFONT_CHAR_NUM - 1)
		{
			error = FT_Get_Glyph(slot, &pVfont->inf[pVfont->tailId].fontSlot);

			if (error)
			{
				Fwl_Print(C2, M_VFONT, "FT_Get_Glyph error\r\n");
				return AK_FALSE;
			}

			error = FT_Glyph_To_Bitmap(&pVfont->inf[pVfont->tailId].fontSlot, FT_RENDER_MODE_NORMAL, NULL, 1);

			if (error)
			{
				Fwl_Print(C2, M_VFONT, "FT_Glyph_To_Bitmap error\r\n");
				FT_Done_Glyph(pVfont->inf[pVfont->tailId].fontSlot);
				return AK_FALSE;
			}
			
			pVfont->loadId[chr] = pVfont->tailId;
			pVfont->inf[pVfont->tailId].unicode = chr;
			pVfont->tailId++;
		}
		else
		{
			for (i=0; i<VFONT_CHAR_NUM; i++)
			{
				T_U32 tmp = 0;

				tmp = (65536 + pVfont->showId - pVfont->inf[i].showId) & 0xffff;
				
				if (tmp >= 1000)
				{
					break;
				}
			}

			if (VFONT_CHAR_NUM == i)
			{
				i = 0;
			}

			FT_Done_Glyph(pVfont->inf[i].fontSlot);
			pVfont->loadId[pVfont->inf[i].unicode] = LOAD_ID_ERR;

			error = FT_Get_Glyph(slot, &pVfont->inf[i].fontSlot);

			if (error)
			{
				Fwl_Print(C2, M_VFONT, "FT_Get_Glyph error\r\n");
				return AK_FALSE;
			}

			error = FT_Glyph_To_Bitmap(&pVfont->inf[i].fontSlot, FT_RENDER_MODE_NORMAL, NULL, 1);

			if (error)
			{
				Fwl_Print(C2, M_VFONT, "FT_Glyph_To_Bitmap error\r\n");
				FT_Done_Glyph(pVfont->inf[i].fontSlot);
				return AK_FALSE;
			}

			pVfont->loadId[chr] = i;
			pVfont->inf[i].unicode = chr;
		}
		
	}
	
	return AK_TRUE;
}


/**
* @brief get char width
*
* @author Songmengxing
* @date 2012-04-01
* @param in const T_U16 chr : unicode id
* @return T_U16
* @retval char width
*/
T_U16 VF_GetCharWidth(const T_U16 chr)
{
	T_U16 width = 0;
	T_U16 chrId = chr;

	AK_ASSERT_PTR(pVfont, "VF_GetCharWidth(): pVfont error", width);
	AK_ASSERT_PTR(pVfont->library, "VF_GetCharWidth(): pVfont->library error", width);
	AK_ASSERT_PTR(pVfont->face, "VF_GetCharWidth(): pVfont->face error", width);

	if (!VF_LoadChar(pVfont->face, chrId))
	{
		chrId = 0x25a1;
		
		if (!VF_LoadChar(pVfont->face, chrId))
		{
			return width;
		}
	}

	pVfont->showId++;
	pVfont->inf[pVfont->loadId[chrId]].showId = pVfont->showId;

	width = (T_U16)(((FT_BitmapGlyph)pVfont->inf[pVfont->loadId[chrId]].fontSlot)->root.advance.x >> 16);

	return width;
}


/**
* @brief show string to a buffer
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_U8* pbuf : buffer to show
* @param in T_U32 imgW : buf width
* @param in T_U32 imgH : buf height
* @param in T_U16* string : string to show
* @param in T_POS x :
* @param in T_POS y :
* @param in T_U32 color : color
* @param in T_U32 size : font size
* @return T_BOOL
* @retval 
*/
T_BOOL VF_DispStr(T_U8* pbuf, T_U32 imgW, T_U32 imgH, T_U16* string, T_POS x, T_POS y, T_COLOR color, T_U32 size)
{
	FT_Glyph   	glyphT;
	FT_Vector     pen;                    /* untransformed origin  */
	T_U16*         text;
	int           target_height;
	int           n, num_chars;
	T_POS			x_max = 0;
	T_POS			y_max = 0;
	FT_BitmapGlyph  bitGlyph ;
	T_BOOL			ret = AK_FALSE;
#ifdef OS_ANYKA
	T_USTR_FILE		path = {0};
#endif

	AK_ASSERT_PTR(pVfont, "VF_DispStr(): pVfont error", ret);
	AK_ASSERT_PTR(pVfont->library, "VF_DispStr(): pVfont->library error", ret);
	AK_ASSERT_PTR(pVfont->face, "VF_DispStr(): pVfont->face error", ret);
	AK_ASSERT_VAL(imgW >= VF_IMG_WIDTH, "VF_DispStr(): imgW error", ret);

#ifdef OS_ANYKA
	Eng_StrMbcs2Ucs(gs.VF_InstalledPath, path);
	
	if (!Fwl_CheckDriverIsValid(path))
    {
        return ret;
    }
#endif
	
	/* first argument     */
	text          = string;                           /* second argument    */
	num_chars     = Utl_UStrLen( string );
	target_height = VF_IMG_HEIGHT;
	
	pen.x = x;
	pen.y = size;
	
	memset(pVfont->image, 0, VF_IMG_WIDTH * VF_IMG_HEIGHT);
	
	for (n=0; n<num_chars; n++)
	{			
		if (!VF_LoadChar(pVfont->face, text[n]))
		{
			text[n] = 0x25a1;
			
			if (!VF_LoadChar(pVfont->face, text[n]))
			{
				continue;
			}
		}

		pVfont->showId++;
		pVfont->inf[pVfont->loadId[text[n]]].showId = pVfont->showId;

		glyphT = pVfont->inf[pVfont->loadId[text[n]]].fontSlot;
		
		/* now, draw to our target surface (convert position) */
		bitGlyph = (FT_BitmapGlyph)glyphT;
		
		VF_DrawBitmap( &bitGlyph->bitmap,
			pen.x + bitGlyph->left,
			pen.y - bitGlyph->top);
		
		
		if (pen.y - bitGlyph->top + bitGlyph->bitmap.rows > y_max)
		{
			y_max = pen.y - bitGlyph->top+ bitGlyph->bitmap.rows;
		}
		
		/* increment pen position */
		
		pen.x += bitGlyph->root.advance.x>>16;		
	}
  
	x_max = (T_POS)pen.x;
	  
	VF_ShowImg(pbuf, imgW, imgH, x, x_max, y, (T_POS)(y_max + y), color);

	  
	return AK_TRUE;
}


#endif

/* EOF */
