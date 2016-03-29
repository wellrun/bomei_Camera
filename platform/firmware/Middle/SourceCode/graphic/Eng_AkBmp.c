/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name£ºEng_AkBmp.c
 * Function£ºThe file define some function for processing AK bmp picture.
 
 *
 * Author£ºZou Mai
 * Date£º2002-09-14
 * Version£º1.0          
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/

#include "Eng_AkBmp.h"
#include "Eng_Graph.h"
#include "Fwl_pfDisplay.h"
#include "Eng_Debug.h"

#include "Fwl_display.h"
#include "ImageLayer.h"
#include "Fwl_gui.h"


extern const T_U8 gb_bitMask[];
/**
 * @brief Dram BMP with frame. !!!The function can only support one-color deep bmp.
 * 
 * @author @b MiaoBaoli
 * 
 * @author 
 * @date 2002-09-03
 * @param T_POS x
 * @param  T_POS y
 * @param  const T_AK_BMP *AkBmp
 * @return T_BOOL
 */
T_BOOL AkBmpDrawWithFrame( HLAYER  hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp, T_COLOR color, T_COLOR frameColor )
{
    T_U8    bitcolor[ 512 ];
    T_U16    i, j, k;
    T_POS    x0, y0;
    T_U8*    pData;
    T_U16    bPatch;

    if( AkBmp->Deep != 1 )
    {
        return AK_FALSE;
    }

    //bPatch = AkBmp->Width % 8;
    bPatch = (AkBmp->Width & 0x07);

    
    for( i=0; i<AkBmp->Width; ++i )
    {
        bitcolor[ i ] = 0;
    }

    //MemSet(bitcolor, 0x00,  (T_U32)(AkBmp->Width));
    
    pData = AkBmp->BmpData;
    y0 = y;

    for( j=0; j<AkBmp->Height; ++j, ++y0 )
    {
        x0 = x+1;
        for( i=1, k=1; i<AkBmp->Width; ++i, ++k, ++x0 )
        {
            if( k==8 )
            {
                ++pData;
                k = 0;
            }
            if( *pData & gb_bitMask[ k ] )
            {
                if( !bitcolor[ i-1 ] )
                {    
                    Fwl_SetPixel( hLayer, (T_POS)(x0-1), y0, frameColor );
                }
                if( !bitcolor[ i ] )
                {
                    Fwl_SetPixel( hLayer, x0, (T_POS)(y0-1), frameColor );
                }
                Fwl_SetPixel( hLayer, x0, y0, color );
                bitcolor[ i ] = 1;
            }
            else
            {
                if( bitcolor[ i-1 ] || bitcolor[ i ] )
                {
                    Fwl_SetPixel( hLayer, x0, y0, frameColor );
                }
                bitcolor[ i ] = 0;
            }
        }
        if( bPatch )
        {
            ++pData;
        }
    }
    return AK_TRUE;
}


/**
 * @brief Draw background bmp from bmp string
 *
 * @author @b LiaoJianhua
 * @date 2005-12-27
 * @param HLAYER hLayer : handle of layer
 * @param T_POS x: the x position
 * @param T_POS y: the y position
 * @param T_pCDATA BmpString:
 * @return T_BOOL: drawn flag
 * @note: AKBmpDrawBackXXX functions are the most quick function in AKBmpDrawXXX functions
 * @retval NOT AK_NULL: softkey control creatted
 */
T_BOOL AkBmpDrawBackFromString(HLAYER hLayer, T_POS x, T_POS y, T_pCDATA BmpString)
{
    T_AK_BMP    AnykaBmp;

    AK_ASSERT_PTR(BmpString, "AkBmpDrawBackFromString(): BmpString", AK_FALSE);
    
    AkBmpGetFromString(BmpString, &AnykaBmp);
    return AkBmpDrawBack(hLayer, x, y, &AnykaBmp);
}



T_BOOL AkBmpDraw(HLAYER hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse)
{
    return Fwl_AkBmpDrawPart(hLayer, x, y, AK_NULL, AkBmp, bkColor, Reverse);
}

T_BOOL    AkBmpDrawBack(HLAYER hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp)
{
    return Fwl_AkBmpDrawPart(hLayer, x, y, AK_NULL, AkBmp, AK_NULL, AK_FALSE);
}

/**
 * @brief Get BMP data from string
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-09-03
 * @param T_pDATA BmpString
 * @param  T_AK_BMP *AnykaBmp
 * @return T_AK_BMP
 * @retval 
 */
T_AK_BMP *AkBmpGetFromString(T_pCDATA BmpString, T_AK_BMP *AnykaBmp)
{
    T_pDATA     bmpData = ( T_pDATA)BmpString;

    AK_ASSERT_PTR(AnykaBmp, "AkBmpGetFromString(): BmpString", AnykaBmp);

    if (BmpString == AK_NULL)
    {
        AnykaBmp->Width = 0;
        AnykaBmp->Height = 0;
        AnykaBmp->Deep = 0;
        AnykaBmp->BmpData = AK_NULL;

        return AnykaBmp;
    }

#if 0
    AnykaBmp->Frame = AKBmpGetInfo(BmpString, &AnykaBmp->Width, &AnykaBmp->Height, &AnykaBmp->Deep);

    if (AnykaBmp->Frame == 0 || AnykaBmp->Frame == 2)   /* width and height have 2 bytes */
    {
        AnykaBmp->BmpData = (T_pDATA)(BmpString + 6);
    }
    else                        /* normal mode */
    {
        AnykaBmp->BmpData = (T_pDATA)(BmpString + 4);
    }
#endif

    AnykaBmp->Frame = *bmpData; 

    if (AnykaBmp->Frame == 0 || AnykaBmp->Frame == 2)   /* width and height have 2 bytes */ 
    { 
        //fixed by ljh, don't convert a T_U8 pointer to T_U16 pointer!!!!!!!!!!!! 

        AnykaBmp->Width = (T_LEN)(bmpData[1] | bmpData[2]<<8); 
        AnykaBmp->Height = (T_LEN)(bmpData[3] | bmpData[4]<<8); 
        //AnykaBmp->Width = (T_LEN)(*(T_U16 *)(bmpData + 1)); 
        //AnykaBmp->Height = (T_LEN)(*(T_U16 *)(bmpData + 3)); 
        AnykaBmp->Deep = *(bmpData + 5); 
        AnykaBmp->BmpData = (T_pDATA)(bmpData + 6); 
    } 
    else      /* normal mode */ 
    { 
        //Modified by Tommy for BMP generated by Image Lib, 2008-01-15 
        //NOTICE: The standard BMP's width & height are signed 32bits! 
        AnykaBmp->Width = (T_LEN)(*(T_S32 *)(bmpData + 0x12)); 
        AnykaBmp->Height = (T_LEN)(*(T_S32 *)(bmpData + 0x16)); 
        //NOTICE: The standard BMP's depth is unsigned 16bits! 
        AnykaBmp->Deep = (T_U8)(*(T_U16 *)(bmpData + 0x1C)); 
        AnykaBmp->BmpData = (T_pDATA)(bmpData + 0x36); 
        //End of modification 

    } 

    return AnykaBmp;
}

/**
* @brief get the info of the bmp data string
* 
* @author: Zhuobin Li
* @date 2005-12-16
* @param T_pCDATA BmpString: The bmp string.
* @param  T_LEN *pWidth: The pointer for return the width of the bmp. 
if the pointer is AK_NULL, don't return the value of the width.
* @param  T_LEN *pHeight: The pointer for return the Height of the bmp. 
if the pointer is AK_NULL, don't return the value of the Height.
* @param  T_U8 *pDeep: The pointer for return the Deep of the bmp. 
if the pointer is AK_NULL, don't return the value of the Deep.
* @return T_U8 : return the frame of the bmp(by the frame, width and height have 2 bytes or 1 byte).
* @retval 
*/
T_U8 AKBmpGetInfo(T_pCDATA BmpString, T_LEN *pWidth, T_LEN *pHeight, T_U8 *pDeep)
{
    T_U8        frame;
    T_U16        tmpLow;
    T_U16        tmpHght;
    T_LEN       tmp;
    
    AK_ASSERT_PTR(BmpString, "AKBmpGetInfo(): BmpString", 3);
    
    if (BmpString == AK_NULL)
    {
        return 0;
    }
    frame = *BmpString;
    
    if (frame == 0 || frame == 2)    /* width and height have 2 bytes */
    {
        if (pWidth != AK_NULL)
        {
            // if the pointer is odd number, we can not use T_U16 or T_U32 to
            // get the number which the pointer point to, so use the way to get the T_U16 data
            //*pWidth = (T_LEN)(*(T_U16 *)(BmpString + 1));
            tmpLow = (T_U16)(*(BmpString + 1));
            tmpHght = (T_U16)(*(BmpString + 2));
            tmp = (T_LEN)((tmpHght << 8) + tmpLow);
            *pWidth = tmp;
        }
        if (pHeight != AK_NULL)
        {
            //*pHeight = (T_LEN)(*(T_U16 *)(BmpString + 3));
            tmpLow = (T_U16)(*(BmpString + 3));
            tmpHght = (T_U16)(*(BmpString + 4));
            tmp = (T_LEN)((tmpHght << 8) + tmpLow);
            *pHeight = tmp;
        }
        if (pDeep != AK_NULL)
        {
            *pDeep = *(BmpString + 5);
        }
    }
    else                        /* normal mode */
    {
        if (pWidth != AK_NULL)
        {
            *pWidth = (T_LEN)(*(T_S32 *)(BmpString + 0x12));
        }
        if (pHeight != AK_NULL)
        {
            *pHeight = (T_LEN)(*(T_S32 *)(BmpString + 0x16));
        }
        if (pDeep != AK_NULL)
        {
            *pDeep = (T_U8)(*(T_U16 *)(BmpString + 0x1C));
        }
    }
    
    return frame;
    
}

/*
    get standard windows-bmp info.
*/
T_VOID StdBmpGetInfo(T_pCDATA StdBmpString, T_LEN *pWidth, T_LEN *pHeight, T_U8 *pDeep)
{
    T_pDATA     bmpData = (T_pDATA)StdBmpString;

    AK_ASSERT_PTR_VOID(StdBmpString, "StdBmpGetInfo: the bmp stream is null!");

    if (pWidth != AK_NULL)
    {
        *pWidth = (T_LEN)(*(T_S32 *)(bmpData + 0x12)); 
    }

    if (pHeight != AK_NULL)
    {
        *pHeight = (T_LEN)(*(T_S32 *)(bmpData + 0x16)); 
    }
    
    //NOTICE: The standard BMP's depth is unsigned 16bits! 
    if (pDeep != AK_NULL)
    {
        *pDeep = (T_U8)(*(T_U16 *)(bmpData + 0x1C)); 
    }
}



/**
 * @brief Draw AKBmp format bitmap partly
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param T_eLCD LCD: LCD ID
 * @param T_POS x: x position the AKBmp would be drawn 
 * @param T_POS y: y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param T_RECT *range: the part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param T_AK_BMP AkBmp: the source AKbmp would be drawn
 * @param T_COLOR *bkColor: the transparent color
 * @param T_BOOL Reverse: if AK_TRUE, reverse draw the source pixel
 * @return T_VOID
 * @note if range, akcolor, reverse are AK_NULL, AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */

 T_BOOL	 Fwl_AkBmpDrawPart(HLAYER layer, T_POS x, T_POS y, T_RECT *range,
				 const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse)
 {
	 T_POINT point;
 
	 point.x = x;
	 point.y = y;
 
 
	 return  ImgLay_AkBmpDrawPart(layer,  point, range,  AkBmp, bkColor,  Reverse);
	 
 }



 /**
  * @brief Draw partial BMP from ANYKA BMP string to the screen
  * 
  * @author ZouMai
  * @date 2002-09-03
  * @param T_POS x	  show rect left-top-point.x of LCD 
  * @param T_POS y	  show rect left-top-point.y of LCD 
  * @param T_RECT *rang: range of the partial BMP(Big BMP map left-top-point as (0,0) )
  * @param T_pCDATA bmpStream: ANYKA BMP string
  * @param T_COLOR bkColor: transparent color
  * @param T_BOOL Reverse: reverse or not
  * @return T_BOOL
  * @retval 
  */
 T_BOOL  Fwl_AkBmpDrawPartFromString(HLAYER layer, T_POS x, T_POS y, T_RECT *range,
				 T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse)
 {
	 T_POINT point;
 
	 point.x = x;
	 point.y = y;
 
	 return  ImgLay_AkBmpDrawPartFromString(layer, point, range,
				  BmpStream, bkColor,  Reverse);
 
 }

 /**
  * @brief Dram BMP from BMP data string 
  * 
  * @author @b ZouMai
  * 
  * @author 
  * @date  2002-09-03
  * @param T_POS x
  * @param	T_POS y
  * @param	T_pDATA BmpString
  * @return T_BOOL
  * @retval 
  */

 T_BOOL  Fwl_AkBmpDrawFromString(HLAYER layer, T_POS x, T_POS y, T_pCDATA BmpStream,
				 T_COLOR *bkColor, T_BOOL Reverse)
 {
	 T_POINT point;
 
	 point.x = x;
	 point.y = y;
 
	 return  ImgLay_AkBmpDrawFromString( layer,  point,  BmpStream,
				 bkColor,  Reverse);
 }

 T_BOOL  Fwl_AkBmpDrawAlphaFromString(HLAYER layer, T_POS x, T_POS y, T_pCDATA BmpStream,
				 T_COLOR *bkColor, T_BOOL Reverse)
 {
	 T_POINT point;
 
	 point.x = x;
	 point.y = y;
 
	 return  ImgLay_AkBmpDrawAlphaFromString( layer,  point,  BmpStream,
				 bkColor,  Reverse);
 }

 
 
 
 T_BOOL Fwl_AKBmpAlphaShow(T_U8 *srcBuf, T_U32 srcBufW, T_RECT srcRect,
						   T_U8 *dstBuf, T_U32 dstBufW, T_RECT dstRect,T_U8 alpha)
 {
	T_U32		 inbuf[1];
	T_U32		 obuff[1];
	T_AK_BMP	 AnykaBmp;

	if ((AK_NULL == srcBuf) || (AK_NULL == dstBuf))
	{
		 Fwl_Print(C3, M_DISPLAY, "Fwl_AKBmpAlphaShow AK_NULL == buf!");
		 return AK_FALSE;
	}
 
	AkBmpGetFromString(srcBuf, &AnykaBmp);
	inbuf[0] = (T_U32)AnykaBmp.BmpData;
	obuff[0] = (T_U32)dstBuf;

	if (Fwl_ScaleConvertEx(inbuf, 1, (T_U16)srcBufW, &srcRect, FORMAT_RGB565,					   
					   obuff, 1, (T_U16)dstBufW, &dstRect, FORMAT_RGB565,
					   AK_TRUE, alpha, AK_TRUE, g_Graph.TransColor))
    {
       	return AK_FALSE;
    }

	return AK_TRUE;
 }



