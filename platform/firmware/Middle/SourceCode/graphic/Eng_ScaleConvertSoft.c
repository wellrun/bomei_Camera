#include "Eng_ScaleConvertSoft.h"
#include "Fwl_osMalloc.h"
#ifndef NULL
#define NULL			(void *)0
#endif


#define  GUI_ERROR_PARAMETER    2
#define  GUI_ERROR_TIMEOUT      1
#define  GUI_ERROR_OK           0


T_BOOL Rotate_FillRGB888(T_U8 *ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8 *obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90)
{
    int   srcPos, dstPos, srcStride, dstStride;
    int   i, j;
    unsigned char *psrc;
    unsigned char *pdst;

    if (NULL == ibuff || NULL == obuff)
    {
        //printf("input buff is NULL!\n");
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW == 0 || srcRectH == 0)
    {
        //printf("output buff is NULL!\n");
        return GUI_ERROR_PARAMETER;
    }

    srcPos = (srcRectY*srcWidth+srcRectX)*3;
    dstPos = (dstRectY*dstWidth+dstRectX)*3;

    if (rotate90)
    {
        pdst = obuff + dstPos;
        srcStride = (srcWidth-1)*3;
        dstStride = (dstWidth - srcRectH)*3;
        for(i=0; i<srcRectW; i++)
        {
            psrc = ibuff + srcPos;
            for(j=0; j<srcRectH; j++)
            {
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;

                psrc += srcStride;
            }
            pdst += dstStride;
            srcPos += 3;
        }
    }
    else
    {
        psrc = ibuff + srcPos;
        pdst = obuff + dstPos;
        srcStride = (srcWidth - srcRectW)*3;
        dstStride = (dstWidth - srcRectW)*3;
        for(i=0; i<srcRectH; i++)
        {
            for(j=0; j<srcRectW; j++)
            {
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;
            }

            psrc += srcStride;
            pdst += dstStride;
        }
    }

    return GUI_ERROR_OK;
}

T_BOOL Rotate_FillRGB565(T_U8 *ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8 *obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90)
{
    int   srcPos, dstPos, srcStride, dstStride;
    int   i, j;
    unsigned char *psrc;
    unsigned char *pdst;

    if (NULL == ibuff || NULL == obuff)
    {
        //printf("input buff is NULL!\n");
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW == 0 || srcRectH == 0)
    {
        //printf("output buff is NULL!\n");
        return GUI_ERROR_PARAMETER;
    }

    srcPos = (srcRectY*srcWidth+srcRectX)*2;
    dstPos = (dstRectY*dstWidth+dstRectX)*2;

    if (rotate90)
    {
        pdst = obuff + dstPos;
        srcStride = (srcWidth-1)*2;
        dstStride = (dstWidth - srcRectH)*2;
        for(i=0; i<srcRectW; i++)
        {
            psrc = ibuff + srcPos;
            for(j=0; j<srcRectH; j++)
            {
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;

                psrc += srcStride;
            }
            pdst += dstStride;
            srcPos += 2;
        }
    }
    else
    {
        psrc = ibuff + srcPos;
        pdst = obuff + dstPos;
        srcStride = (srcWidth - srcRectW)*2;
        dstStride = (dstWidth - srcRectW)*2;
        for(i=0; i<srcRectH; i++)
        {
            for(j=0; j<srcRectW; j++)
            {
                *pdst++ = *psrc++;
                *pdst++ = *psrc++;
            }

            psrc += srcStride;
            pdst += dstStride;
        }
    }

    return GUI_ERROR_OK;
}

#define  EPSILON    (0x2)
#define  FixedDiv(a, b)   (((a)<<8)/(b))
#define  FixedMul(a, b)   ((a)*(b)>>8)
#define  FixedFromInt(a)  ((a)<<8)
#define  IntFromFixed(a)  ((a)>>8)

typedef  int         T_Fixed;

T_U8 ScaleRGB888Soft(T_U8 *ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY,
        T_U16 srcRectW, T_U16 srcRectH, 
        T_U8 *obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
        T_U16 dstRectW, T_U16 dstRectH)
{
    T_Fixed ratioX, ratioY;
    T_Fixed x, y, u, v;
    int   i, j;
    T_U8  *srcPtr, *ptr;
    T_U8  ul, ur, dl, dr;
    T_U32 out;
    T_U32 dstStride;

    if (ibuff == NULL || obuff == NULL)
    {
        return GUI_ERROR_PARAMETER;
    }

    if ((srcRectW == dstRectW) && (srcRectH == dstRectH))
    {
        return Rotate_FillRGB888(ibuff, srcWidth, srcRectX, srcRectY,srcRectW, srcRectH,
                obuff, dstWidth, dstRectX, dstRectY, 0);
    }

    ratioX = FixedDiv(srcRectW, dstRectW);
    ratioY = FixedDiv(srcRectH, dstRectH);

    ibuff += (srcRectY * srcWidth + srcRectX) * 3;
    obuff += (dstRectY * dstWidth + dstRectX) * 3;

    dstStride = (dstWidth - dstRectW)*3;

    y = 0;
    for(i=0; i<dstRectH; i++)
    {
        y += ratioY;
        v = y & 0xff;
        ptr = ibuff + IntFromFixed(y) * srcWidth * 3;
        x = 0;
        for(j=0; j<dstRectW; j++)
        {
            x += ratioX;
            u = x & 0xff;
            srcPtr = ptr + IntFromFixed(x) * 3;
            //calculate RGB here
            ul = srcPtr[0];
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = srcPtr[3];
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = srcPtr[srcWidth*3];
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr = srcPtr[srcWidth*3+3];
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            *obuff++ = (T_U8)(out>>8);

            ul = srcPtr[0+1];
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = srcPtr[3+1];
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = srcPtr[srcWidth*3+1];
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr = srcPtr[srcWidth*3+3+1];
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            *obuff++ = (T_U8)(out>>8);

            ul = srcPtr[0+2];
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = srcPtr[3+2];
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = srcPtr[srcWidth*3+2];
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr = srcPtr[srcWidth*3+3+2];
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            *obuff++ = (T_U8)(out>>8);
        }
        obuff += dstStride;
    }

    return GUI_ERROR_OK;
}

T_U8 ScaleRGB565Soft(T_U8 *ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY,
        T_U16 srcRectW, T_U16 srcRectH, 
        T_U8 *obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
        T_U16 dstRectW, T_U16 dstRectH)
{
    T_Fixed ratioX, ratioY;
    T_Fixed x, y, u, v;
    int   i, j;
    T_U8  *srcPtr, *ptr;
    T_U8  ul, ur, dl, dr, R, G, B;
    T_U32 out;
    T_U32 dstStride;

    if (ibuff == NULL || obuff == NULL)
    {
        return GUI_ERROR_PARAMETER;
    }

    if ((srcRectW == dstRectW) && (srcRectH == dstRectH))
    {
        return Rotate_FillRGB565(ibuff, srcWidth, srcRectX, srcRectY,srcRectW, srcRectH,
                obuff, dstWidth, dstRectX, dstRectY, 0);
    }

    ratioX = FixedDiv(srcRectW-1, dstRectW-1);
    ratioY = FixedDiv(srcRectH-1, dstRectH-1);

    ibuff += (srcRectY * srcWidth + srcRectX) * 2;
    obuff += (dstRectY * dstWidth + dstRectX) * 2;

    dstStride = (dstWidth - dstRectW)*2;

    y = 0;
    for(i=0; i<dstRectH; i++)
    {
        
        v = y & 0xff;
        ptr = ibuff + IntFromFixed(y) * srcWidth * 2;
        x = 0;
        for(j=0; j<dstRectW; j++)
        {
            
            u = x & 0xff;
            srcPtr = ptr + IntFromFixed(x) * 2;
            //calculate RGB here
            ul = (srcPtr[1]&0xf8);
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = (srcPtr[3]&0xf8);
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = (srcPtr[srcWidth*2+1]&0xf8);
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr = (srcPtr[srcWidth*2+3]&0xf8);
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            R = (T_U8)(out>>8);

            ul = ((srcPtr[0]&0xe0)>>3)|((srcPtr[1]&0x07)<<5);
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = ((srcPtr[2]&0xe0)>>3)|((srcPtr[3]&0x07)<<5);
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = ((srcPtr[srcWidth*2+0]&0xe0)>>3)|((srcPtr[srcWidth*2+1]&0x07)<<5);
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr =((srcPtr[srcWidth*2+2]&0xe0)>>3)|((srcPtr[srcWidth*2+3]&0x07)<<5);
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            G = (T_U8)(out>>8);

            ul = (srcPtr[0]&0x1f)<<3;
            out = (ul<<8);
            if (u >= EPSILON)
            {
                ur = (srcPtr[2]&0x1f)<<3;
                out += (ur - ul) * u;
            }

            if (v >= EPSILON)
            {
                dl = (srcPtr[srcWidth*2]&0x1f)<<3;
                out += (dl - ul) * v;

                if (u >= EPSILON)
                {
                    dr = (srcPtr[srcWidth*2+2]&0x1f)<<3;
                    out += (ul + dr - ur - dl) * FixedMul(u, v);
                }
            }
            B = (T_U8)(out>>8);

            *obuff++ = ((G&0x1c)<<3)|((B&0xf8)>>3);
            *obuff++ = (R&0xf8)|((G&0xe0)>>5);
            x += ratioX;
        }
        y += ratioY;
        obuff += dstStride;
    }

    return GUI_ERROR_OK;
}

T_U8 Scale_ConvertSoft(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8 luminance_enabled, T_U8* luminance_table)
{
    if (format_in != format_out)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (NULL == ibuff || NULL == obuff)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW == 0 || srcRectH == 0
        || dstRectW == 0 || dstRectH == 0)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (format_in != FORMAT_YUV420 && nibuff != 1)
    {
        return GUI_ERROR_PARAMETER;
    }

    switch(format_in)
    {
    case FORMAT_RGB565:
        return ScaleRGB565Soft((T_U8*)ibuff[0], srcWidth, srcRectX, srcRectY, srcRectW, srcRectH, 
        (T_U8*)obuff[0], dstWidth, dstRectX, dstRectY, dstRectW, dstRectH);
        break;
	#if 0
    case FORMAT_RGB888:
        return ScaleRGB888Soft((T_U8*)ibuff[0], srcWidth, srcRectX, srcRectY, srcRectW, srcRectH, 
        (T_U8*)obuff[0], dstWidth, dstRectX, dstRectY, dstRectW, dstRectH);
        break;
	#endif
    case FORMAT_YUV420:
        return GUI_ERROR_TIMEOUT;
        break;
    default:
        return GUI_ERROR_PARAMETER;
        break;
    }
}

#define  ALPHA_BLEND(src, dst, alpha) (((src)*(alpha)+(dst)*(255-(alpha)))>>8)

T_U8 Scale_Convert2Soft(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color)
{
    T_U8 *sbuff, *dbuff;
    T_U16 sx, sy, sw, sh, swidth;
    T_U16 dx, dy, dw, dh, dwidth;
    T_S32 i, j, line_stride, bytes_per_pixel;
    T_U8 *ptr8, sR, sG, sB, dR, dG, dB;
    //T_U32 pixel_color;
    T_U8  ret;

    if (format_in != format_out)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (NULL == ibuff || NULL == obuff)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (srcRectW == 0 || srcRectH == 0
        || dstRectW == 0 || dstRectH == 0)
    {
        return GUI_ERROR_PARAMETER;
    }

    if (format_in != FORMAT_YUV420 && nibuff != 1)
    {
        return GUI_ERROR_PARAMETER;
    }

    switch(format_in)
    {
        case FORMAT_RGB565:
            bytes_per_pixel = 2;
            break;
#if 0
        case FORMAT_RGB888:
            bytes_per_pixel = 3;
            break;
#endif
        default:
            bytes_per_pixel = 0;
            break;
    }

    if (color_trans_enabled)
    {
        return GUI_ERROR_PARAMETER;
    }
#if 0
    if (color_trans_enabled)
    {
        sbuff = (T_U8 *)Fwl_Malloc(srcRectW*srcRectH*bytes_per_pixel);
        if (NULL == sbuff)
        {
            return GUI_ERROR_TIMEOUT;
        }

        sx = sy = 0;
        sw = swidth = srcRectW;
        sh = srcRectH;

        ptr8 = (T_U8*)ibuff[0] + (srcRectY*srcWidth+srcRectX)*bytes_per_pixel;
        line_stride = (srcWidth - srcRectW) * bytes_per_pixel;

		if (format_in == FORMAT_RGB888)
		{
			for(i=0; i<sh; i++)
			{
				for(j=0; j<sw; j++)
				{
					pixel_color = ptr8[0]|(ptr8[1]<<8)|(ptr8[2]<<16);
					if (color == pixel_color)
					{
						sbuff[(i*sw+j)*3+0] = 
							sbuff[(i*sw+j)*3+1] =
							sbuff[(i*sw+j)*3+2] = 0;
					}
					else
					{
						sbuff[(i*sw+j)*3+0] = ptr8[0];
						sbuff[(i*sw+j)*3+1] = ptr8[1];
						sbuff[(i*sw+j)*3+2] = ptr8[2];
					}
                    ptr8 += 3;
				}
				ptr8 += line_stride;
			}
		}
        else if (format_in == FORMAT_RGB565)
        {
            for(i=0; i<sh; i++)
            {
                for(j=0; j<sw; j++)
                {
                    pixel_color = ptr8[0]|(ptr8[1]<<8);
                    if (color == pixel_color)
                    {
                        sbuff[(i*sw+j)*2+0] =
                            sbuff[(i*sw+j)*2+1] = 0;
                    }
                    else
                    {
                        sbuff[(i*sw+j)*2+0] = ptr8[0];
                        sbuff[(i*sw+j)*2+1] = ptr8[1];
                    }
                    ptr8 += 2;
                }
                ptr8 += line_stride;
            }
        }
        else
        {
            Fwl_Free(sbuff);
            return GUI_ERROR_TIMEOUT;
        }
    }
    else
#endif
    {
        sbuff = (T_U8*)(ibuff[0]);

        sx = srcRectX;
        sy = srcRectY;
        sw = srcRectW;
        sh = srcRectH;
        swidth = srcWidth;
    }

    if (alpha_enabled)
    {
        dbuff = (T_U8*)Fwl_Malloc(dstRectW*dstRectH*bytes_per_pixel);
        if (NULL == dbuff)
        {
            if (sbuff != (T_U8*)(ibuff[0]))
            {
                Fwl_Free(sbuff);
                return GUI_ERROR_TIMEOUT;
            }
        }

        dx = dy = 0;
        dw = dwidth = dstRectW;
        dh = dstRectH;
    }
    else
    {
        dbuff = (T_U8*)obuff[0];

        dx = dstRectX;
        dy = dstRectY;
        dw = dstRectW;
        dh = dstRectH;
        dwidth = dstWidth;
    }

    switch(format_in)
    {
    case FORMAT_RGB565:
        ret = ScaleRGB565Soft(sbuff, swidth, sx, sy, sw, sh, 
        dbuff, dwidth, dx, dy, dw, dh);
        break;
#if 0
    case FORMAT_RGB888:
        ret = ScaleRGB888Soft(sbuff, swidth, sx, sy, sw, sh, 
        dbuff, dwidth, dx, dy, dw, dh);
        break;
#endif
    case FORMAT_YUV420:
        return GUI_ERROR_TIMEOUT;
        break;
    default:
        return GUI_ERROR_PARAMETER;
        break;
    }

    if (GUI_ERROR_OK != ret)
    {
        if (sbuff != (T_U8*)(ibuff[0]))
        {
            Fwl_Free(sbuff);
        }
        if (dbuff != (T_U8*)(obuff[0]))
        {
            Fwl_Free(dbuff);
        }

        return ret;
    }

    if (alpha_enabled)
    {
        ptr8 = (T_U8*)obuff[0] + (dstRectY*dstWidth+dstRectX)*bytes_per_pixel;
        line_stride = (dstWidth - dstRectW)*bytes_per_pixel;
		#if 0
		if (format_out == FORMAT_RGB888)
        {
            for(i=0; i<dh; i++)
            {
                for(j=0; j<dw; j++)
                {
                    ptr8[0] = ALPHA_BLEND(dbuff[(i*dw+j)*3+0], ptr8[0], alpha);
                    ptr8[1] = ALPHA_BLEND(dbuff[(i*dw+j)*3+1], ptr8[2], alpha);
                    ptr8[2] = ALPHA_BLEND(dbuff[(i*dw+j)*3+2], ptr8[2], alpha);
                    ptr8 += 3;
                }
                ptr8 += line_stride;
            }
        }
        else
		#endif
		if (format_out == FORMAT_RGB565)
        {
            for(i=0; i<dh; i++)
            {
                for(j=0; j<dw; j++)
                {
                    sR = dbuff[(i*dw+j)*2+1]&0xf8;
                    sG = ((dbuff[(i*dw+j)*2+0]>>3)&0x1c)|((dbuff[(i*dw+j)*2+1]<<5)&0xe0);
                    sB = (dbuff[(i*dw+j)*2+0]<<3)&0xf8;
                    dR = ptr8[1]&0xf8;
                    dG = ((ptr8[0]>>3)&0x1c)|((ptr8[1]<<5)&0xe0);
                    dB = (ptr8[0]<<3)&0xf8;

                    dR = ALPHA_BLEND(sR, dR, alpha);
                    dG = ALPHA_BLEND(sG, dG, alpha);
                    dB = ALPHA_BLEND(sB, dB, alpha);

                    ptr8[0] = ((dB>>3)&0x1f)|((dG<<3)&0xe0);
                    ptr8[1] = ((dR)&0xf8)|((dG>>5)&0x07);

                    ptr8 += 2;
                }
                ptr8 += line_stride;
            }
        }
    }

    if (sbuff != (T_U8*)ibuff[0])
    {
        Fwl_Free(sbuff);
    }

    if (dbuff != (T_U8*)obuff[0])
    {
        Fwl_Free(dbuff);
    }

    return ret;
}




