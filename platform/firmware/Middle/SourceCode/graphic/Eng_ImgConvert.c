/**
 * @file convert.c
 * @brief image convert APIs.
 * This file provides image convert APIs: image zoom in or out,
 * cut image, convert image mirror.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-10-29
 * @version 1.0
 * @ref AK3210 technical manual.
 */

#include "Fwl_public.h"
#include "Eng_ImgConvert.h"
#include "Fwl_Image.h"
#include "eng_debug.h"
#include "fwl_pfdisplay.h"
#include "fwl_graphic.h"
#include "Lib_ThumbsDB.h"

extern T_BOOL				  Flag_ImgBrowse_ClrBuf;

/**
 * @brief RGB image convert function
 * Convert the image's width & height to height & width.
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U8 *RGB: input & output source RGB image data
 * @param T_U32 srcW: source RGB image's width
 * @param T_U32 srcH: source RGB image's height
 * @param T_BOOL Xmirror: image X mirror(AK_TRUE) or not(AK_FALSE)
 * @param T_BOOL Ymirror: image Y mirror(AK_TRUE) or not(AK_FALSE)
 * @param T_BOOL reverse: image force reverse(AK_TRUE) or auto reverse(AK_FALSE)
 * @param T_BOOL XCenter: X center align
 * @param T_BOOL YCenter: Y center align
 * @param T_BOOL clean: clean LCD display
 * @param T_BOOL swap: swap red & blue color
 * @return T_VOID
 * @retval
 */
#ifdef OS_WIN32
T_VOID ConvertRGB2LCDBuf(T_U8 *RGB, T_U32 width, T_U32 height, \
                   T_BOOL Xmirror, T_BOOL Ymirror, T_BOOL reverse, \
                   T_BOOL XCenter, T_BOOL YCenter, \
                   T_BOOL clean, T_BOOL swap)
{
    T_U32 x, y, srcY, dstY, srcPos, dstPos,srcWidth;
    T_U8 *disp;

    //FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);

    srcWidth = width;

    disp = Fwl_GetDispMemory();

    if (clean)
        memset((void *)disp, 0x00, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
    if ((width <= (T_U32)(Fwl_GetLcdWidth())) && !reverse)
    {
        if (width > (T_U32)(Fwl_GetLcdWidth()))
            width = (T_U32)(Fwl_GetLcdWidth());
        if (height > (T_U32)(Fwl_GetLcdHeight()))
            height = (T_U32)(Fwl_GetLcdHeight());

        for (y=0; y<height; y++)
        {
            srcY = y*srcWidth;
            dstY = ((Ymirror?(height-y-1):y)+(YCenter?((Fwl_GetLcdHeight()-height)/2):0))*Fwl_GetLcdWidth();
            for (x=0; x<width; x++)
            {
                srcPos = (srcY+x)*3;
                dstPos = (dstY+(Xmirror?(width-x-1):x)+(XCenter?((Fwl_GetLcdWidth()-width)/2):0))*3;
                if (swap)
                {
                    disp[dstPos] = RGB[srcPos+2];
                    disp[dstPos+1] = RGB[srcPos+1];
                    disp[dstPos+2] = RGB[srcPos];
                }
                else 
                {
                    memcpy((void *)(disp+dstPos), (void *)(RGB+srcPos), 3);
                }
            }
        }
    }
    else
    {
        if (height > (T_U32)(Fwl_GetLcdWidth()))
            height = (T_U32)(Fwl_GetLcdWidth());
        if (width > (T_U32)(Fwl_GetLcdHeight()))
            width = (T_U32)(Fwl_GetLcdHeight());

        for (y=0; y<height; y++)
        {
            srcY = y*srcWidth;
            dstY = (Ymirror?y:(height-y-1))+(XCenter?((Fwl_GetLcdWidth()-height)/2):0);
            for (x=0; x<width; x++)
            {
                srcPos = (srcY+x)*3;
                dstPos = (((Xmirror?(width-x-1):x)+(YCenter?((Fwl_GetLcdHeight()-width)/2):0))*Fwl_GetLcdWidth()+dstY)*3;
                if (swap)
                {
                    disp[dstPos] = RGB[srcPos+2];
                    disp[dstPos+1] = RGB[srcPos+1];
                    disp[dstPos+2] = RGB[srcPos];
                }
                else
                {
                    memcpy((void *)(disp+dstPos), (void *)(RGB+srcPos), 3);
                }
            }
        }
    }

    //FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}
#endif
#ifdef OS_ANYKA
T_VOID ConvertRGB2LCDBuf(T_U8 *RGB, T_U32 width, T_U32 height, \
                   T_BOOL Xmirror, T_BOOL Ymirror, T_BOOL reverse, \
                   T_BOOL XCenter, T_BOOL YCenter, \
                   T_BOOL clean, T_BOOL swap)
{
    T_U32 x, y, srcY, dstY, srcPos, dstPos,srcWidth;
    T_U8 *disp;

//    FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);

    srcWidth = width;

    disp = Fwl_GetDispMemory();

    if (clean)
        memset((void *)disp, 0x00, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
    if ((width <= (T_U32)(Fwl_GetLcdWidth())) && !reverse)
    {
        if (width > (T_U32)(Fwl_GetLcdWidth()))
            width = (T_U32)(Fwl_GetLcdWidth());
        if (height > (T_U32)(Fwl_GetLcdHeight()))
            height = (T_U32)(Fwl_GetLcdHeight());

        for (y=0; y<height; y++)
        {
            srcY = y*srcWidth;
            dstY = ((Ymirror?(height-y-1):y)+(YCenter?((Fwl_GetLcdHeight()-height)/2):0))*Fwl_GetLcdWidth();
            for (x=0; x<width; x++)
            {
                srcPos = (srcY+x)*3;
                dstPos = (dstY+(Xmirror?(width-x-1):x)+(XCenter?((Fwl_GetLcdWidth()-width)/2):0))*2;
                if (swap)
                {
                    disp[dstPos] = (T_U8)(((RGB[srcPos+1]&0x1c)<<3) | ((RGB[srcPos]&0xf8)>>3));
                    disp[dstPos+1] = (T_U8)((RGB[srcPos+2]&0xf8) | ((RGB[srcPos+1]&0xe0)>>5));
                }
                else
                {
                    disp[dstPos] = (T_U8)(((RGB[srcPos+1]&0x1c)<<3) | ((RGB[srcPos+2]&0xf8)>>3));
                    disp[dstPos+1] = (T_U8)((RGB[srcPos]&0xf8) | ((RGB[srcPos+1]&0xe0)>>5));
                }
            }
        }
    }
    else
    {
        if (height > (T_U32)(Fwl_GetLcdWidth()))
            height = (T_U32)(Fwl_GetLcdWidth());
        if (width > (T_U32)(Fwl_GetLcdHeight()))
            width = (T_U32)(Fwl_GetLcdHeight());

        for (y=0; y<height; y++)
        {
            srcY = y*srcWidth;
            dstY = (Ymirror?y:(height-y-1))+(XCenter?((Fwl_GetLcdWidth()-height)/2):0);
            for (x=0; x<width; x++)
            {
                srcPos = (srcY+x)*3;
                dstPos = (((Xmirror?(width-x-1):x)+(YCenter?((Fwl_GetLcdHeight()-width)/2):0))*Fwl_GetLcdWidth()+dstY)*2;
                if (swap)
                {
                    disp[dstPos] = (T_U8)(((RGB[srcPos+1]&0x1c)<<3) | ((RGB[srcPos]&0xf8)>>3));
                    disp[dstPos+1] = (T_U8)((RGB[srcPos+2]&0xf8) | ((RGB[srcPos+1]&0xe0)>>5));
                }
                else
                {
                    disp[dstPos] = (T_U8)(((RGB[srcPos+1]&0x1c)<<3) | ((RGB[srcPos+2]&0xf8)>>3));
                    disp[dstPos+1] = (T_U8)((RGB[srcPos]&0xf8) | ((RGB[srcPos+1]&0xe0)>>5));
                }
            }
        }
    }

//    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}
#endif

T_VOID GetRGB2RGBBuf(T_U8 *srcRGB, T_U32 srcW, T_U32 srcH, \
                     T_U8 *dstRGB, T_U32 dstW, T_U32 dstH, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U32 *scale)
{
    T_U32 x, y, srcY, dstY;
    T_U32 dispW, dispH, tmpW, tmpH;
    T_U32 zoom_max;

//    FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);

    /* Check zoom */
    if (100*srcW/dstW >= 100*srcH/dstH)
        zoom_max = 100*srcW/dstW;
    else
        zoom_max = 100*srcH/dstH;

    memset((void *)dstRGB, 0x00, dstW*dstH*3);
    if (zoom_max < 100)
    {
        zoom = 100;
        dispW = srcW;
        dispH = srcH;

        /* display image align center */
        for (y=0; y<dispH; y++)
        {
            srcY = y*dispW*3;
            dstY = (y+((dstH-dispH)>>1))*dstW*3;
            for (x=0; x<dispW; x++)
                memcpy((void *)(dstRGB+dstY+(x+((dstW-dispW)>>1))*3), (void *)(srcRGB+srcY+x*3), 3);
        }
    }
    else
    {
        if (zoom < 100)
            zoom = 100;
        else if (zoom > zoom_max)
            zoom = zoom_max;

        if ((srcW*dstH) >= (srcH*dstW))
        {
            tmpH = srcH*dstW/srcW;
            tmpW = dstW;
        }
        else
        {
            tmpW = srcW*dstH/srcH;
            tmpH = dstH;
        }

        if (dstW < srcW*tmpW/srcW*zoom/100)
            dispW = dstW;
        else
            dispW = srcW*tmpW/srcW*zoom/100;
        if (dstH < srcH*tmpH/srcH*zoom/100)
            dispH = dstH;
        else
            dispH = srcH*tmpH/srcH*zoom/100;

        if (offsetX > srcW-dispW*srcW/tmpW*100/zoom)
            offsetX = srcW-dispW*srcW/tmpW*100/zoom;
        if (offsetY > srcH-dispH*srcH/tmpH*100/zoom)
            offsetY = srcH-dispH*srcH/tmpH*100/zoom;

        for (y=0; y<dispH; y++)
        {
            srcY = (y*srcH/tmpH*100/zoom+offsetY)*srcW*3;
            dstY = (y+((dstH-dispH)>>1))*dstW*3;
            for (x=0; x<dispW; x++)
                memcpy((void *)(dstRGB+dstY+(x+((dstW-dispW)>>1))*3), (void *)(srcRGB+srcY+(x*srcW/tmpW*100/zoom+offsetX)*3), 3);
        }
    }

    *scale = zoom*dispW/srcW;
//    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}

T_U32 PaletteNum(T_U16 deep)
{
    T_U32 i, num;

    num = 1;
    for (i=0; i<deep; i++)
        num *= 2;

    return num;
}

T_VOID ConvertRGB2BGR(T_U8 *buffer, T_U32 width, T_U32 height)
{
    T_U32 x, y, pos;
    T_U8 tmp;

//    FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);

    for (y=0; y<height; y++)
    {
        for (x=0; x<width; x++)
        {
            pos = (y*width+x)*3;
            tmp = buffer[pos];
            buffer[pos] = buffer[pos+2];
            buffer[pos+2] = tmp;
        }
    }

//    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}

// return AKBMP head length
T_U32 FillAkBmpHead(T_U8 *img_buff, T_U16 width, T_U16 height)
{
    T_U32 num = 0;
#ifdef LCD_MODE_565
	T_U8 bitsize = 16;
#else
	T_U8 bitsize = 24;
#endif

    if (1)
    {
        *(img_buff + (num++)) = 0;
        *(img_buff + (num++)) = width & 0xFF;
        *(img_buff + (num++)) = (width >> 8) & 0xFF;
        *(img_buff + (num++)) = height & 0xFF;
        *(img_buff + (num++)) = (height >> 8) & 0xFF;
    }
    else
    {
        *(img_buff + (num++)) = 1;
        *(img_buff + (num++)) = (T_U8)width;
        *(img_buff + (num++)) = (T_U8)height;
    }

    *(img_buff + (num++)) = bitsize;

    return num;
}

T_BOOL GetBMP2RGBBuf565(T_U8 *srcBMP, T_U8 *dstRGB, \
                     T_U32 dstW, T_U32 dstH, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U16 rotate, T_U32 *scale, T_COLOR bColor)
{
    T_U32 x, y, srcY, dstY, srcPos, dstPos;
    T_U32 srcW, srcH, dispW, dispH, tmpW, tmpH;
    T_U32 tmp1, tmp2;
    T_U32 zoom_max;
    T_U8 *RGB;
    T_U16 deep;
    T_U32 color, compression, maskR, maskG, maskB;
    T_U8 moveR, moveG, moveB;
    T_U8 palette[256][4];
    T_BOOL bLargeImg;
    T_U32 bioffset,colorused;
    T_RECT rect;

//    FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);
	if (AK_NULL == srcBMP)
	{
		return AK_FALSE;
	}

    if (rotate == 0 || rotate == 180)
    {
        memcpy((void *)&srcW, (void *)&srcBMP[0x12], 4);
        memcpy((void *)&srcH, (void *)&srcBMP[0x16], 4);
    }
    else
    {
        memcpy((void *)&srcH, (void *)&srcBMP[0x12], 4);
        memcpy((void *)&srcW, (void *)&srcBMP[0x16], 4);
    }
	
	if ((0 == srcW) || (0 == srcH))
	{
		return AK_FALSE;
	}

    memcpy((void *)&deep, (void *)&srcBMP[0x1c], 2);

	if ((1 != deep) && (4 != deep) && (8 != deep) && (16 != deep) && (24 != deep) &&(32 != deep))
	{
		return AK_FALSE;
	}

	
    memcpy((void *)&compression, (void *)&srcBMP[0x1e], 4);
    
    memcpy((void *)&bioffset, (void *)&srcBMP[0x0a], 4);
    colorused = (bioffset - 54) >> 2; //(bioffset-54)/4;
    //get palette
    if (deep < 24)
    {
        if (deep == 16)
        {
            if (compression == 0)
            {
                maskR = 0x7c00; moveR = 7;
                maskG = 0x03e0; moveG = 2;
                maskB = 0x001f; moveB = 3;
                RGB = srcBMP + BMP_HEAD_SIZE;
            }
            else if (compression == 3)
            {
                memcpy((void *)&maskR, (void *)(srcBMP+BMP_HEAD_SIZE+4*0), 4);
                memcpy((void *)&maskG, (void *)(srcBMP+BMP_HEAD_SIZE+4*1), 4);
                memcpy((void *)&maskB, (void *)(srcBMP+BMP_HEAD_SIZE+4*2), 4);
                if (maskR == 0x7c00)
                {
                    moveR = 7;
                }
                else if (maskR == 0xF800)
                {
                    moveR = 8;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                if (maskG == 0x03e0)
                {
                    moveG = 2;
                }
                else if (maskG == 0x07e0)
                {
                    moveG = 3;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                if (maskB == 0x001f)
                {
                    moveB = 3;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                RGB = srcBMP + BMP_HEAD_SIZE + 3*4;
            }
            else
            {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                return AK_FALSE;
            }
        }
        else
        {   
            memcpy((void *)palette, (void *)(srcBMP+BMP_HEAD_SIZE), colorused*4);
            RGB = srcBMP + bioffset;
        }
    }
    else
    {
        RGB = srcBMP + BMP_HEAD_SIZE;
    }

    /* Check zoom */
    tmp1 = 100*srcW/dstW;
    tmp2 = 100*srcH/dstH;
    if (tmp1 >= tmp2)
        zoom_max = tmp1;
    else
        zoom_max = tmp2;

	//modify by wgtbupt
	//memset((void *)dstRGB, 0x00, dstW*dstH*3);
	//memset((void *)dstRGB, 0x00, dstW*dstH*2);

	rect.left   = 0;
	rect.top    = 0;
	rect.width  = (T_LEN)dstW;
	rect.height = (T_LEN)dstH;

	Fwl_Clean(dstRGB, dstW, dstH, &rect, bColor, RGB565);

    if (zoom_max < 100)
    {
        bLargeImg = AK_FALSE;
        zoom = 100;
        dispW = srcW;
        dispH = srcH;
    }
    else
    {
        bLargeImg = AK_TRUE;
        if (zoom < 100)
            zoom = 100;
        else if (zoom > zoom_max)
            zoom = zoom_max;

        if ((srcW*dstH) >= (srcH*dstW))
        {
            tmpH = srcH*dstW/srcW;
            tmpW = dstW;
        }
        else
        {
            tmpW = srcW*dstH/srcH;
            tmpH = dstH;
        }

		if (0 == tmpW)
		{
			tmpW = 1;
		}
		
		if (0 == tmpH)
		{
			tmpH = 1;
		}

        tmp1 = (srcW*tmpW*zoom)/(srcW*100);
        tmp2 = (srcH*tmpH*zoom)/(srcH*100);
        if (dstW < tmp1) 
            dispW = dstW;
        else
            dispW = tmp1; 
        if (dstH < tmp2) 
            dispH = dstH;
        else
            dispH = tmp2;

        tmp1 = srcW-(dispW*srcW*100)/(tmpW*zoom);
        tmp2 = srcH-(dispH*srcH*100)/(tmpH*zoom);
        if (offsetX > tmp1)  
            offsetX = tmp1;  
        if (offsetY > tmp2) 
            offsetY = tmp2;  
    }

    /* display image align center */
    for (y=0; y<dispH; y++)
    {
        if (!bLargeImg)
        {
            switch(rotate)
            {
                case 0:
                    srcY = y*(((dispW*deep+31)>>5)<<2);
                    break;
                case 90:
                    srcY = ((dispH - 1 - y)*deep)>>3;  //(dispH - 1 - y)*deep/8;
                    break;
                case 180:
                    srcY = (dispH - 1 - y)*(((dispW*deep+31)>>5)<<2);
                    break;
                case 270:
                    srcY = (y*deep)>>3;  //y*deep/8;
                    break;
            }
        }
        else
        {
            switch(rotate)
            {
            case 0:
                srcY = (((y*srcH*100)/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));   //((y*srcH/tmpH*100/zoom+offsetY)*(((srcW*deep+31)>>5)<<2));
                break;
            case 90:
                srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
                break;
            case 180:
                srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));
                break;
            case 270:
                srcY = ((y*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
                break;
            }
        }

		//wgtbupt
        //dstY = (((dstH+dispH)>>1)-y-1)*dstW*3;
        dstY = (((dstH+dispH)>>1)-y-1)*dstW*2;
        for (x=0; x<dispW; x++)
        {
            if (!bLargeImg)
            {
                switch(rotate)
                {
                case 0:
                    srcPos = srcY+((x*deep)>>3);  //srcY+x*deep/8;
                    break;
                case 90:
                    srcPos = x*(((dispH*deep+31)>>5)<<2)+srcY;
                    break;
                case 180:
                    srcPos = srcY+(((dispW - 1 - x)*deep)>>3);    //srcY+(dispW - 1 - x)*deep/8;
                    break;
                case 270:
                    srcPos = (dispW - 1 - x)*(((dispH*deep+31)>>5)<<2)+srcY;
                    break;
                }
            }
            else
            {
                switch(rotate)
                {
                case 0:
                    srcPos = srcY+(((x*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
                    break;
                case 90:
                    srcPos = srcY+((x*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2));
                    break;
                case 180:
                    srcPos = srcY+((((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
                    break;
                case 270:
                    srcPos = (((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2))+srcY;
                    break;
                }
            }

			//wgtbupt
            //dstPos = dstY+(x+((dstW-dispW)>>1))*3;
            dstPos = dstY+(x+((dstW-dispW)>>1))*2;

            color = 0;
            if (deep <= 8)
                memcpy((void *)&color, (void *)&RGB[srcPos], 1);
            else if (deep == 16)
                memcpy((void *)&color, (void *)&RGB[srcPos], 2);

            if (deep == 1)
            {
             //   dstRGB[dstPos] = palette[(color>>(7-x%8))&0x01][2];
             //   dstRGB[dstPos+1] = palette[(color>>(7-x%8))&0x01][1];
             //   dstRGB[dstPos+2] = palette[(color>>(7-x%8))&0x01][0];

			    dstRGB[dstPos+1] = (palette[(color>>(7-x%8))&0x01][2] & 0x0F8) |
				    (palette[(color>>(7-x%8))&0x01][1] >> 5);                
                dstRGB[dstPos] = (palette[(color>>(7-x%8))&0x01][0]>>3) |
					((palette[(color>>(7-x%8))&0x01][1] << 3) & 0x0C0 );
            }
            else if (deep == 4)
            {
                dstRGB[dstPos+1] = (palette[(color>>(4*(1-x%2)))&0x0f][2] & 0x0F8) |
					(palette[(color>>(4*(1-x%2)))&0x0f][1] >> 5);                
                dstRGB[dstPos] = (palette[(color>>(4*(1-x%2)))&0x0f][0]>>3) |
					((palette[(color>>(4*(1-x%2)))&0x0f][1] << 3) & 0x0C0);
            }
            else if (deep == 8)
            {
                dstRGB[dstPos+1] = (palette[color][2] & 0x0F8) |
					(palette[color][1] >> 5);                
                dstRGB[dstPos] = (palette[color][0] >> 3) |
					((palette[color][1] << 3) & 0x0C0);
            }
            else if (deep == 16)
            {
                dstRGB[dstPos+1] = ((T_U8)((color&maskR)>>moveR) & 0x0F8) |
					(((T_U8)((color&maskG)>>moveG) >> 5));                
                dstRGB[dstPos] = ((T_U8)((color&maskB)<<moveB) >> 3) |
					(((T_U8)((color&maskG)>>moveG) << 3) & 0x0C0);
            }
            else if (deep >= 24)
            {
                dstRGB[dstPos+1] = (RGB[srcPos+2] & 0x0F8) |
					(RGB[srcPos+1] >> 5);                
                dstRGB[dstPos] = (RGB[srcPos] >> 3) |
					((RGB[srcPos+1] << 3) & 0x0C0);
            }
        }
    }

    *scale = zoom*dispW/srcW;

	return AK_TRUE;
//    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}
#if 0
T_VOID GetBMP2RGBBuf888_Rect(T_U8 *srcBMP, T_U8 *dstRGB, \
                     T_U32 dstW, T_U32 dstH, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U16 rotate, T_U32 *scale, T_RECT *pstRectOfBuf)
{
	T_U32 x, y, srcY, dstY, srcPos, dstPos;
	T_U32 srcW, srcH, dispW, dispH, tmpW, tmpH;
	T_U32 tmp1, tmp2;
	T_U32 zoom_max;
	T_U8 *RGB;
	T_U16 deep;
	T_U32 color, compression, maskR, maskG, maskB;
	T_U8 moveR, moveG, moveB;
	T_U8 palette[256][4];
	T_BOOL bLargeImg;
	T_U32 bioffset,colorused;

	if (AK_NULL == srcBMP)
	{
		return;
	}

//	  FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);

	if (rotate == 0 || rotate == 180)
	{
		memcpy((void *)&srcW, (void *)&srcBMP[0x12], 4);
		memcpy((void *)&srcH, (void *)&srcBMP[0x16], 4);
	}
	else
	{
		memcpy((void *)&srcH, (void *)&srcBMP[0x12], 4);
		memcpy((void *)&srcW, (void *)&srcBMP[0x16], 4);
	}
	memcpy((void *)&deep, (void *)&srcBMP[0x1c], 2);

	if ((1 != deep) && (4 != deep) && (8 != deep) && (16 != deep) && (24 != deep) &&(32 != deep))
	{
		return;
	}
	
	memcpy((void *)&compression, (void *)&srcBMP[0x1e], 4);
	
	memcpy((void *)&bioffset, (void *)&srcBMP[0x0a], 4);
	colorused = (bioffset - 54) >> 2; //(bioffset-54)/4;

	//get palette
	if (deep < 24)
	{
		if (deep == 16)
		{
			if (compression == 0)
			{
				maskR = 0x7c00; moveR = 7;
				maskG = 0x03e0; moveG = 2;
				maskB = 0x001f; moveB = 3;
				RGB = srcBMP + BMP_HEAD_SIZE;
			}
			else if (compression == 3)
			{
				memcpy((void *)&maskR, (void *)(srcBMP+BMP_HEAD_SIZE+4*0), 4);
				memcpy((void *)&maskG, (void *)(srcBMP+BMP_HEAD_SIZE+4*1), 4);
				memcpy((void *)&maskB, (void *)(srcBMP+BMP_HEAD_SIZE+4*2), 4);
				if (maskR == 0x7c00)
				{
					moveR = 7;
				}
				else if (maskR == 0xF800)
				{
					moveR = 8;
				}
				else
				{
//					  FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
					return;
				}
				if (maskG == 0x03e0)
				{
					moveG = 2;
				}
				else if (maskG == 0x07e0)
				{
					moveG = 3;
				}
				else
				{
//					  FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
					return;
				}
				if (maskB == 0x001f)
				{
					moveB = 3;
				}
				else
				{
//					  FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
					return;
				}
				RGB = srcBMP + BMP_HEAD_SIZE + 3*4;
			}
			else
			{
//					  FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
				return;
			}
		}
		else
		{	
			memcpy((void *)palette, (void *)(srcBMP+BMP_HEAD_SIZE), colorused*4);
			RGB = srcBMP + bioffset;
		}
	}
	else
	{
		RGB = srcBMP + BMP_HEAD_SIZE;
	}

	/* Check zoom */
	tmp1 = 100*srcW/dstW;
	tmp2 = 100*srcH/dstH;
	if (tmp1 >= tmp2)
		zoom_max = tmp1;
	else
		zoom_max = tmp2;

	if(Flag_ImgBrowse_ClrBuf)
	{
		memset((void *)dstRGB, 0x00, pstRectOfBuf->width * pstRectOfBuf->height * 3);
	}
	else
	{
		Flag_ImgBrowse_ClrBuf = AK_TRUE;
	}

	if (zoom_max < 100)
	{
		bLargeImg = AK_FALSE;
		zoom = 100;
		dispW = srcW;
		dispH = srcH;
	}
	else
	{
		bLargeImg = AK_TRUE;
		if (zoom < 100)
			zoom = 100;
		else if (zoom > zoom_max)
			zoom = zoom_max;

		if ((srcW*dstH) >= (srcH*dstW))
		{
			tmpH = srcH*dstW/srcW;
			tmpW = dstW;
		}
		else
		{
			tmpW = srcW*dstH/srcH;
			tmpH = dstH;
		}

		if (0 == tmpW)
		{
			tmpW = 1;
		}
		
		if (0 == tmpH)
		{
			tmpH = 1;
		}

		tmp1 = (srcW*tmpW*zoom)/(srcW*100);
		tmp2 = (srcH*tmpH*zoom)/(srcH*100);
		if (dstW < tmp1) 
			dispW = dstW;
		else
			dispW = tmp1; 
		if (dstH < tmp2) 
			dispH = dstH;
		else
			dispH = tmp2;

		tmp1 = srcW-(dispW*srcW*100)/(tmpW*zoom);
		tmp2 = srcH-(dispH*srcH*100)/(tmpH*zoom);
		if (offsetX > tmp1)  
			offsetX = tmp1;  
		if (offsetY > tmp2) 
			offsetY = tmp2;  
	}

	/* display image align center */
	for (y=0; y<dispH; y++)
	{
		if (!bLargeImg)
		{
			switch(rotate)
			{
				case 0:
					srcY = y*(((dispW*deep+31)>>5)<<2);
					break;
				case 90:
					srcY = ((dispH - 1 - y)*deep)>>3;  //(dispH - 1 - y)*deep/8;
					break;
				case 180:
					srcY = (dispH - 1 - y)*(((dispW*deep+31)>>5)<<2);
					break;
				case 270:
					srcY = (y*deep)>>3;  //y*deep/8;
					break;
			}
		}
		else
		{
			switch(rotate)
			{
			case 0:
				srcY = (((y*srcH*100)/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));	//((y*srcH/tmpH*100/zoom+offsetY)*(((srcW*deep+31)>>5)<<2));
				break;
			case 90:
				srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
				break;
			case 180:
				srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));
				break;
			case 270:
				srcY = ((y*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
				break;
			}
		}

		dstY = (((pstRectOfBuf->height + dispH)>>1)-y-1)* pstRectOfBuf->width *3;
		
		for (x=0; x<dispW; x++)
		{
			if (!bLargeImg)
			{
				switch(rotate)
				{
				case 0:
					srcPos = srcY+((x*deep)>>3);  //srcY+x*deep/8;
					break;
				case 90:
					srcPos = x*(((dispH*deep+31)>>5)<<2)+srcY;
					break;
				case 180:
					srcPos = srcY+(((dispW - 1 - x)*deep)>>3);	  //srcY+(dispW - 1 - x)*deep/8;
					break;
				case 270:
					srcPos = (dispW - 1 - x)*(((dispH*deep+31)>>5)<<2)+srcY;
					break;
				}
			}
			else
			{
				switch(rotate)
				{
				case 0:
					srcPos = srcY+(((x*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
					break;
				case 90:
					srcPos = srcY+((x*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2));
					break;
				case 180:
					srcPos = srcY+((((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
					break;
				case 270:
					srcPos = (((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2))+srcY;
					break;
				}
			}
			
			dstPos = dstY+(x+((pstRectOfBuf->width - dispW)>>1))*3;

			color = 0;
			if (deep <= 8)
				memcpy((void *)&color, (void *)&RGB[srcPos], 1);
			else if (deep == 16)
				memcpy((void *)&color, (void *)&RGB[srcPos], 2);

			if (deep == 1)
			{
				dstRGB[dstPos] = palette[(color>>(7-x%8))&0x01][2];
				dstRGB[dstPos+1] = palette[(color>>(7-x%8))&0x01][1];
				dstRGB[dstPos+2] = palette[(color>>(7-x%8))&0x01][0];
			}
			else if (deep == 4)
			{
				dstRGB[dstPos] = palette[(color>>(4*(1-x%2)))&0x0f][2];
				dstRGB[dstPos+1] = palette[(color>>(4*(1-x%2)))&0x0f][1];
				dstRGB[dstPos+2] = palette[(color>>(4*(1-x%2)))&0x0f][0];
			}
			else if (deep == 8)
			{
				dstRGB[dstPos] = palette[color][2];
				dstRGB[dstPos+1] = palette[color][1];
				dstRGB[dstPos+2] = palette[color][0];
			}
			else if (deep == 16)
			{
				dstRGB[dstPos] = (T_U8)((color&maskR)>>moveR);
				dstRGB[dstPos+1] = (T_U8)((color&maskG)>>moveG);
				dstRGB[dstPos+2] = (T_U8)((color&maskB)<<moveB);
			}
			else if (deep >= 24)
			{
				dstRGB[dstPos] = RGB[srcPos+2];
				dstRGB[dstPos+1] = RGB[srcPos+1];
				dstRGB[dstPos+2] = RGB[srcPos];
			}
		}
	}

	*scale = zoom*dispW/srcW;
//	  FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}

#endif


T_BOOL GetBMP2RGBBuf888(T_U8 *srcBMP, T_U8 *dstRGB, \
                     T_U32 dstW, T_U32 dstH, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U16 rotate, T_U32 *scale, T_COLOR bColor)
{
    T_U32 x, y, srcY, dstY, srcPos, dstPos;
    T_U32 srcW, srcH, dispW, dispH, tmpW, tmpH;
    T_U32 tmp1, tmp2;
    T_U32 zoom_max;
    T_U8 *RGB;
    T_U16 deep;
    T_U32 color, compression, maskR, maskG, maskB;
    T_U8 moveR, moveG, moveB;
    T_U8 palette[256][4];
    T_BOOL bLargeImg;
    T_U32 bioffset,colorused;
    T_RECT rect;

//    FreqMgr_StateCheckIn(FREQ_FACTOR_IMAGE, FREQ_PRIOR_NULL);
	if (AK_NULL == srcBMP)
	{
		return AK_FALSE;
	}

    if (rotate == 0 || rotate == 180)
    {
        memcpy((void *)&srcW, (void *)&srcBMP[0x12], 4);
        memcpy((void *)&srcH, (void *)&srcBMP[0x16], 4);
    }
    else
    {
        memcpy((void *)&srcH, (void *)&srcBMP[0x12], 4);
        memcpy((void *)&srcW, (void *)&srcBMP[0x16], 4);
    }

	if ((0 == srcW) || (0 == srcH))
	{
		return AK_FALSE;
	}
	
    memcpy((void *)&deep, (void *)&srcBMP[0x1c], 2);

	if ((1 != deep) && (4 != deep) && (8 != deep) && (16 != deep) && (24 != deep) &&(32 != deep))
	{
		return AK_FALSE;
	}
	
    memcpy((void *)&compression, (void *)&srcBMP[0x1e], 4);
    
    memcpy((void *)&bioffset, (void *)&srcBMP[0x0a], 4);
    colorused = (bioffset - 54) >> 2; //(bioffset-54)/4;

    //get palette
    if (deep < 24)
    {
        if (deep == 16)
        {
            if (compression == 0)
            {
                maskR = 0x7c00; moveR = 7;
                maskG = 0x03e0; moveG = 2;
                maskB = 0x001f; moveB = 3;
                RGB = srcBMP + BMP_HEAD_SIZE;
            }
            else if (compression == 3)
            {
                memcpy((void *)&maskR, (void *)(srcBMP+BMP_HEAD_SIZE+4*0), 4);
                memcpy((void *)&maskG, (void *)(srcBMP+BMP_HEAD_SIZE+4*1), 4);
                memcpy((void *)&maskB, (void *)(srcBMP+BMP_HEAD_SIZE+4*2), 4);
                if (maskR == 0x7c00)
                {
                    moveR = 7;
                }
                else if (maskR == 0xF800)
                {
                    moveR = 8;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                if (maskG == 0x03e0)
                {
                    moveG = 2;
                }
                else if (maskG == 0x07e0)
                {
                    moveG = 3;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                if (maskB == 0x001f)
                {
                    moveB = 3;
                }
                else
                {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                    return AK_FALSE;
                }
                RGB = srcBMP + BMP_HEAD_SIZE + 3*4;
            }
            else
            {
//                    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
                return AK_FALSE;
            }
        }
        else
        {   
            memcpy((void *)palette, (void *)(srcBMP+BMP_HEAD_SIZE), colorused*4);
            RGB = srcBMP + bioffset;
        }
    }
    else
    {
        RGB = srcBMP + BMP_HEAD_SIZE;
    }

    /* Check zoom */
    tmp1 = 100*srcW/dstW;
    tmp2 = 100*srcH/dstH;
    if (tmp1 >= tmp2)
        zoom_max = tmp1;
    else
        zoom_max = tmp2;


	//memset((void *)dstRGB, 0x00, dstW*dstH*3);
	rect.left   = 0;
	rect.top    = 0;
	rect.width  = (T_LEN)dstW;
	rect.height = (T_LEN)dstH;

	Fwl_Clean(dstRGB, dstW, dstH, &rect, bColor, RGB888);

    if (zoom_max < 100)
    {
        bLargeImg = AK_FALSE;
        zoom = 100;
        dispW = srcW;
        dispH = srcH;
    }
    else
    {
        bLargeImg = AK_TRUE;
        if (zoom < 100)
            zoom = 100;
        else if (zoom > zoom_max)
            zoom = zoom_max;

        if ((srcW*dstH) >= (srcH*dstW))
        {
            tmpH = srcH*dstW/srcW;
            tmpW = dstW;
        }
        else
        {
            tmpW = srcW*dstH/srcH;
            tmpH = dstH;
        }

		if (0 == tmpW)
		{
			tmpW = 1;
		}
		
		if (0 == tmpH)
		{
			tmpH = 1;
		}

        tmp1 = (srcW*tmpW*zoom)/(srcW*100);
        tmp2 = (srcH*tmpH*zoom)/(srcH*100);
        if (dstW < tmp1) 
            dispW = dstW;
        else
            dispW = tmp1; 
        if (dstH < tmp2) 
            dispH = dstH;
        else
            dispH = tmp2;

        tmp1 = srcW-(dispW*srcW*100)/(tmpW*zoom);
        tmp2 = srcH-(dispH*srcH*100)/(tmpH*zoom);
        if (offsetX > tmp1)  
            offsetX = tmp1;  
        if (offsetY > tmp2) 
            offsetY = tmp2;  
    }

    /* display image align center */
    for (y=0; y<dispH; y++)
    {
        if (!bLargeImg)
        {
            switch(rotate)
            {
                case 0:
                    srcY = y*(((dispW*deep+31)>>5)<<2);
                    break;
                case 90:
                    srcY = ((dispH - 1 - y)*deep)>>3;  //(dispH - 1 - y)*deep/8;
                    break;
                case 180:
                    srcY = (dispH - 1 - y)*(((dispW*deep+31)>>5)<<2);
                    break;
                case 270:
                    srcY = (y*deep)>>3;  //y*deep/8;
                    break;
            }
        }
        else
        {
            switch(rotate)
            {
            case 0:
                srcY = (((y*srcH*100)/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));   //((y*srcH/tmpH*100/zoom+offsetY)*(((srcW*deep+31)>>5)<<2));
                break;
            case 90:
                srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
                break;
            case 180:
                srcY = (((dispH - 1 - y)*srcH*100/(tmpH*zoom)+offsetY)*(((srcW*deep+31)>>5)<<2));
                break;
            case 270:
                srcY = ((y*srcH*100/(tmpH*zoom)+offsetY)*deep)>>3;
                break;
            }
        }

        dstY = (((dstH+dispH)>>1)-y-1)*dstW*3;
        for (x=0; x<dispW; x++)
        {
            if (!bLargeImg)
            {
                switch(rotate)
                {
                case 0:
                    srcPos = srcY+((x*deep)>>3);  //srcY+x*deep/8;
                    break;
                case 90:
                    srcPos = x*(((dispH*deep+31)>>5)<<2)+srcY;
                    break;
                case 180:
                    srcPos = srcY+(((dispW - 1 - x)*deep)>>3);    //srcY+(dispW - 1 - x)*deep/8;
                    break;
                case 270:
                    srcPos = (dispW - 1 - x)*(((dispH*deep+31)>>5)<<2)+srcY;
                    break;
                }
            }
            else
            {
                switch(rotate)
                {
                case 0:
                    srcPos = srcY+(((x*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
                    break;
                case 90:
                    srcPos = srcY+((x*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2));
                    break;
                case 180:
                    srcPos = srcY+((((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*deep)>>3);
                    break;
                case 270:
                    srcPos = (((dispW - 1 - x)*srcW*100/(tmpW*zoom)+offsetX)*(((srcH*deep+31)>>5)<<2))+srcY;
                    break;
                }
            }
            
            dstPos = dstY+(x+((dstW-dispW)>>1))*3;

            color = 0;
            if (deep <= 8)
                memcpy((void *)&color, (void *)&RGB[srcPos], 1);
            else if (deep == 16)
                memcpy((void *)&color, (void *)&RGB[srcPos], 2);

            if (deep == 1)
            {
                dstRGB[dstPos] = palette[(color>>(7-x%8))&0x01][2];
                dstRGB[dstPos+1] = palette[(color>>(7-x%8))&0x01][1];
                dstRGB[dstPos+2] = palette[(color>>(7-x%8))&0x01][0];
            }
            else if (deep == 4)
            {
                dstRGB[dstPos] = palette[(color>>(4*(1-x%2)))&0x0f][2];
                dstRGB[dstPos+1] = palette[(color>>(4*(1-x%2)))&0x0f][1];
                dstRGB[dstPos+2] = palette[(color>>(4*(1-x%2)))&0x0f][0];
            }
            else if (deep == 8)
            {
                dstRGB[dstPos] = palette[color][2];
                dstRGB[dstPos+1] = palette[color][1];
                dstRGB[dstPos+2] = palette[color][0];
            }
            else if (deep == 16)
            {
                dstRGB[dstPos] = (T_U8)((color&maskR)>>moveR);
                dstRGB[dstPos+1] = (T_U8)((color&maskG)>>moveG);
                dstRGB[dstPos+2] = (T_U8)((color&maskB)<<moveB);
            }
            else if (deep >= 24)
            {
                dstRGB[dstPos] = RGB[srcPos+2];
                dstRGB[dstPos+1] = RGB[srcPos+1];
                dstRGB[dstPos+2] = RGB[srcPos];
            }
        }
    }

    *scale = zoom*dispW/srcW;

	return AK_TRUE;
//    FreqMgr_StateCheckOut(FREQ_FACTOR_IMAGE);
}

/*
T_U16   Eng_Get_UVBufPositon_YUV420(T_POS row, T_POS col, T_LEN imgwidth)
{
    T_U32 temp;

    temp = (T_U32)imgwidth * (row - row%2) + col;
    return (T_U16)((temp / (imgwidth << 1)) * (imgwidth >> 1) + ((temp % (imgwidth << 1)) >> 1));
}
*/

T_VOID  Eng_Bmp2Yuv(T_U8 * bmp, T_U8 * yuv)
{
    T_S32 colorNum;
    T_S32 width;
    T_S32 height;
    T_S32 i,j;
    T_S32 Y=0,U=0,V=0;
    T_S32 R,G,B;
    T_S32 line_width;
    T_U8 *Ybuf,*Ubuf,*Vbuf;
    T_U8 *colorPallette;
    T_U8 *bmpData, *bmpPtr;
    T_S32 bitdeep;

    width = bmp[18]|(bmp[19]<<8)|(bmp[20]<<16)|(bmp[21]<<24);
    height = bmp[22]|(bmp[23]<<8)|(bmp[24]<<16)|(bmp[25]<<24);
    bitdeep = bmp[0x1C]|(bmp[0x1D] << 8);

    if(bitdeep == 8)
        line_width = (width+3)/4*4;     //四字节对齐
    else if (bitdeep == 16)
        line_width = width + (width % 2);
    else if (bitdeep == 24)
        line_width = (width+3)/4*4;
    else
        line_width = width;

    colorNum = bmp[46]|(bmp[47]<<8)|(bmp[48]<<16)|(bmp[49]<<24);

    colorPallette = bmp+54;
    bmpData = colorPallette + colorNum*4;

    Ybuf = yuv;
    Ubuf = yuv+width*height;
#if (defined(CHIP_AK3631) || defined(CHIP_AK322L) || defined(CHIP_AK3224))
    Vbuf = yuv+width*height*3/2;        //yuv format is 4:2:2.
#else
    Vbuf = yuv+width*height*5/4;        //ak78xx, yuv format is 4:2:0.
#endif

    for(i = 0; i<height; i++)
    {
        bmpPtr = bmpData + (height-i-1) * line_width * (bitdeep >> 3);

        for(j = 0; j<width; j++)
        {
            if (bitdeep == 8)
            {
                R = colorPallette[*bmpPtr*4+2];
                G = colorPallette[*bmpPtr*4+1];
                B = colorPallette[*bmpPtr*4];
            }
            else if (bitdeep == 16)
            {
                //16位bmp的rgb排列分5:6:5 和5:5:5两种，需要掩码。暂留空
                B = 255;
                G = 255;
                R = 255;
            }
            else if (bitdeep == 24)
            {
                B = *bmpPtr++;
                G = *bmpPtr++;
                R = *bmpPtr++;
            }
            else
            {
                B = *bmpPtr++;
                G = *bmpPtr++;
                R = *bmpPtr++;
            }
            
            Y = (77*R+150*G+29*B)>>8;

            if(Y>255)
                Y=255;
            Ybuf[i*width+j] = (T_U8) Y;

#if (defined(CHIP_AK3631) || defined(CHIP_AK322L) || defined(CHIP_AK3224))
            if(j&0x01)                  //yuv format is 4:2:2
            {
                U += ((-43*R-85*G+128*B)>>8)+128;
                U = U>>1;
                if(U>255)
                    U=255;
                else if(U<0)
                    U=0;
                Ubuf[(i*width+j)>>1] = (T_U8)U;
                V += ((128*R-107*G-21*B)>>8)+128;
                V = V>>1;
                if(V>255)
                    V=255;
                else if(V<0)
                    V=0;
                Vbuf[(i*width+j)>>1] = (T_U8)V;
            }
            else
            {
                U = ((-43*R-85*G+128*B)>>8)+128;
                V = ((128*R-107*G-21*B)>>8)+128;
            }

#else
            //ak78xx, yuv format is 4:2:0. Four pixels that share a U & V are a 2x2 matrix.
        if (i % 2 == 0)
        {
            if ((j % 2) == 0)
            {
                T_U16 uvpos;

                uvpos = Eng_Get_UVBufPositon_YUV420((T_POS)i, (T_POS)j, (T_LEN)width);
				
                U = ((-43*R-85*G+128*B)>>8)+128;

                if(U>255)
                    U=255;
                else if(U<0)
                    U=0;

                Ubuf[uvpos] = (T_U8)U;

            }
        }
        else
        {
            if ((j % 2) == 1)
            {
                T_U16 uvpos;

                uvpos = Eng_Get_UVBufPositon_YUV420((T_POS)i, (T_POS)j, (T_LEN)width);

                V = ((128*R-107*G-21*B)>>8)+128;

                if(V>255)
                    V=255;
                else if(V<0)
                    V=0;

                Vbuf[uvpos] = (T_U8)V;
            }
        }
#endif

            if ((bitdeep == 8) || (bitdeep == 32))
                bmpPtr++;
            else
            {
                //do nothing. 16bit暂留空。24bit bmp不需要再移动指针。
            }
        }
    }
}

T_VOID SetBmpInfoHeader(BITMAPINFOHEADER *bi, T_U16 width, T_U16 height, T_U8 bitDeep, T_U32 imageSize)
{
	AK_ASSERT_PTR_VOID(bi, "bi Is Invalid");
	
	bi->biSize 		= 40;
    bi->biWidth 	= width;
    bi->biHeight 	= height;
    bi->biPlanes 	= 1;
    bi->biBitCount 	= 16;
    bi->biCompression = 3;
    bi->biClrUsed 	= 0;
    bi->biClrImportant = 0;
    bi->biSizeImage = imageSize;
}

/**
 * @brief Convert BMP Image to BMP 16Bit Deep Image(RGB565)
 *
 * @author 
 * @date	March 9, 2012
 * @param	bmp_src		[in]	Source BMP Image
 * @param	size			[in/out]	Source/Destination BMP Image Size
 * @param	dst_w		[in]	Destination BMP Image Width
 * @param	dst_w		[in]	Destination BMP Image Height
 * @return 	T_pDATA
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pDATA Eng_BMPSubSample(T_U8* bmp_src, T_U32 *size, T_U16 dst_w, T_U16 dst_h)
{
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    T_U16 i, j;
    T_U32 line_src, line_dst, width, height;
    T_S32 ratio_w, ratio_h, offset_x, offset_y;
    T_U8  *p_dst, *p_src, *bmp_dst;	
	T_U16 delta_x, delta_y, bits;	
    T_U8  pallete[256*4];
    T_U8  index_color;
    T_U32 mask565RGB[3] = {0xf800, 0x07e0, 0x001f};
	T_U32 mask555RGB[3] = {0x7C00, 0x03e0, 0x001f};
	T_BOOL bRGB555 = AK_FALSE;

	AK_ASSERT_PTR(bmp_src, "bmp_src Is Invalid", AK_NULL)

    if (!size || *size < 54)
	{
        AK_DEBUG_OUTPUT("error: src BMP size is wrong\n");
        return AK_NULL;
    }

    memcpy(&bf, bmp_src, 14);
    memcpy(&bi, bmp_src+14, 40);

    if (bf.bfType != 0x4d42)
	{
        AK_DEBUG_OUTPUT("error:NOT BMP\n");
        return AK_NULL;
    }

    width 	= bi.biWidth;
    height 	= bi.biHeight;
    bits 	= bi.biBitCount;
    
    switch (bits)
	{
    case 4:
    case 8:
        //get pallete
        if (bi.biBitCount <= 8)
        {
            if (bi.biClrUsed == 0)
            {
                bi.biClrUsed = 1<<bi.biBitCount;
            }
            memcpy(pallete, &bmp_src[54], bi.biClrUsed*4);
        }
        line_src = ((width*bits+31)/32)*4;
        break;
		
    case 16:
    	if (0 == bi.biCompression || 0 == memcmp(bmp_src+54, mask555RGB, 12))
    	{
			bRGB555 = AK_TRUE;
    	}

		line_src = ((width*(bits/8)+3)>>2)<<2;
		break;
    case 24:
    case 32:
        line_src = ((width*(bits/8)+3)>>2)<<2;
        break;
		
    default:
        AK_DEBUG_OUTPUT("error:unsupport BMP color depth\n");
        return NULL;
    }
    
    bmp_src += bf.bfOffBits;

    if (*size < 54+line_src*height)
	{
        AK_DEBUG_OUTPUT("error:BMP require size %d bytes,but input size %d bytes\n", 54+line_src*height, *size);
        return AK_NULL;
    }

    if (dst_w < 2)
		dst_w = 2;
	
    if (dst_h < 2)
		dst_h = 2;

    line_dst 	= ((dst_w*2+3)>>2)<<2;
	
    bmp_dst 	= (T_U8*)Fwl_Malloc(66+line_dst*dst_h);    
	AK_ASSERT_PTR(bmp_dst, "bmp_dst Malloc Failure", AK_NULL);    
	
    *size = 66 + line_dst*dst_h;

	SetBmpInfoHeader(&bi, dst_w, dst_h, 16, line_dst*dst_h);
    
    bf.bfType 		= 0x4d42;
    bf.bfSize 		= 66 + bi.biSizeImage;
    bf.bfOffBits 	= 66;

    p_dst = bmp_dst;
    memcpy(p_dst, &bf, 14);
    p_dst += 14;
    memcpy(p_dst, &bi, 40);
    p_dst += 40;	
    memcpy(p_dst, mask565RGB, 12);
    p_dst += 12;

    ratio_w 	= width*(1 << 16)/(dst_w - 1);
    ratio_h 	= height*(1 << 16)/(dst_h - 1);
    offset_x 	= 0;
    offset_y 	= 0;

    for (i=0; i<dst_h; i++)
	{
        offset_y 	= (i*ratio_h)>>16;
		delta_y 	= (i*ratio_h)&0xffff;
        p_src 		= bmp_src + offset_y * line_src;
		
        for (j=0; j<dst_w; j++)
		{
            offset_x	= (j * ratio_w)>>16;
			delta_x 		= (j*ratio_w)&0xffff;
			
            switch (bits)
			{			
			case 4:
				index_color = (p_src[(offset_x>>1)]>>(4*(1-(offset_x%2))))&0xf;
				p_dst[j*2+0] = ((pallete[index_color*4+0]>>3)&0x1f) |
								((pallete[index_color*4+1]&0x1c)<<3);
				p_dst[j*2+1] = ((pallete[index_color*4+1]>>5)&0x07) |
								(pallete[index_color*4+2]&0xf8);
				break;
				
			case 8:
				index_color = p_src[offset_x]&0xff;
				p_dst[j*2+0] = ((pallete[index_color*4+0]>>3)&0x1f) |
								((pallete[index_color*4+1]&0x1c)<<3);
				p_dst[j*2+1] = ((pallete[index_color*4+1]>>5)&0x07) |
								(pallete[index_color*4+2]&0xf8);
				break;
				
            case 16:
				if (bRGB555)
				{// 555 --> 565
					p_dst[j*2+0] = ((p_src[offset_x*2+0] & 0xe0) << 1) | (p_src[offset_x*2+0] & 0x1f);
					p_dst[j*2+1] = (p_src[offset_x*2+1] << 1) | (p_src[offset_x*2+0] >> 7);
				}
				else
				{
					p_dst[j*2+0] = p_src[offset_x*2+0];
					p_dst[j*2+1] = p_src[offset_x*2+1];
				}
				break;
				
            case 24:				
                p_dst[j*2+0] = (p_src[(offset_x)*3+0]>>3)&0x1f |
                                (p_src[(offset_x)*3+1]&0x1c)<<3;
                p_dst[j*2+1] = (p_src[(offset_x)*3+1]>>5)&0x07 |
                                p_src[(offset_x)*3+2]&0xf8;
                break;
				
            case 32:
                p_dst[j*2+0] = (p_src[(offset_x)*4+0]>>3)&0x1f |
                                (p_src[(offset_x)*4+1]&0x1c)<<3;
                p_dst[j*2+1] = (p_src[(offset_x)*4+1]>>5)&0x07 |
                                p_src[(offset_x)*4+2]&0xf8;
                break;
				
            default:
                break;
            }
        }
		
        p_dst += line_dst;
    }

    return bmp_dst;
}

/* end of file */

