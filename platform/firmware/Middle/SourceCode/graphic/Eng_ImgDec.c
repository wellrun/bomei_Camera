
#include "Ctl_ImgBrowse.h"
#include "Eng_ImgConvert.h"
#include "Fwl_pfKeypad.h"
#include "Eng_DynamicFont.h"
#include "Eng_ScreenSave.h"
#include "Fwl_Image.h"
#include "Eng_Jpeg2Bmp.h"
#include "Eng_ImgDec.h"
#include <string.h>
#include "eng_debug.h"
#include "fwl_osmalloc.h"
#include "fwl_osfs.h"
#include "Eng_Math.h"
#include "drv_api.h"
#include "Fwl_ImageLib.h"


#define  DIGITAL_ZOOM   100

#if (SDRAM_MODE == 8)
#define  SUPPORT_MAX_IMG_SIZE	320
#else
#define  SUPPORT_MAX_IMG_SIZE	1024
#endif


static T_pVOID ImgDec_MemMalloc(T_U32 size, char *filename, T_U32 line);
static T_pVOID ImgDec_MemFree(T_pVOID ptr);


T_VOID ImgDec_GetDstWH(T_U32 ImgW, T_U32 ImgH, T_U32 MaxDisW, T_U32 MaxDisH, T_U32 *pDstW, T_U32 *pDstH)
{
    if (ImgW * MaxDisH > ImgH * MaxDisW)
    {
        *pDstW = MaxDisW;
        *pDstH = ImgH * MaxDisW / ImgW;
    }
    else
    {
        *pDstW = ImgW * MaxDisH / ImgH;
        *pDstH = MaxDisH;
    }
}

T_BOOL ImgDec_GifDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    //T_hGIFENUM gifHandle = IMG_INVALID_HANDLE;
    T_U8            bitsPerPix;
    T_S32           delay;
    T_pDATA         pGifBuf;
    T_U32           gifBufSize;
    T_U8            bitCount;
    T_U16           gifW = 0;
    T_U16           gifH = 0;
    T_BOOL          ret = AK_FALSE;

    pGifBuf = pImgDecAttrib->pInImgBuf;
    gifBufSize = pImgDecAttrib->InImgSize;
    Img_GIFInfo(pGifBuf, &gifW, &gifH, &bitCount);
    pImgDecAttrib->InImgW = gifW;
    pImgDecAttrib->InImgH = gifH;
     pImgDecAttrib->OutImgW = gifW;
    pImgDecAttrib->OutImgH = gifH;
    pImgDecAttrib->GifHandle = GIFEnum_New(pGifBuf, gifBufSize);

    if (IMG_INVALID_HANDLE != pImgDecAttrib->GifHandle)
    {
        GIFEnum_FirstFrame(pImgDecAttrib->GifHandle);
        pImgDecAttrib->pOutImgBuf = (T_pDATA)GIFEnum_GetCurBMP(pImgDecAttrib->GifHandle, &pImgDecAttrib->GifCurFrameSize, &bitsPerPix);
        if (AK_NULL != pImgDecAttrib->pOutImgBuf)
        {
            delay = GIFEnum_GetCurDelay(pImgDecAttrib->GifHandle);
            pImgDecAttrib->GifFrameInterval = (delay < 100) ? 100 : delay;

			Fwl_Print(C3, M_IMAGE, "delay = %d",delay);
			
            ret = AK_TRUE;
        }
        else
        {
            ImgDec_GifDecClose(pImgDecAttrib);
            pImgDecAttrib->GifHandle = IMG_INVALID_HANDLE;
        }
    }

    return ret;
}

T_BOOL ImgDec_GifDecGetNextFrame(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    T_U8 bitsPerPix;
    T_S32 delay;
	T_S32 Interval;
    T_hGIFENUM gifHandle;
    T_BOOL  ret = AK_FALSE;

    gifHandle = pImgDecAttrib->GifHandle;

    if (IMG_INVALID_HANDLE != gifHandle)
    {
        if (GIFEnum_NextFrame(gifHandle) == AK_FALSE)
            GIFEnum_FirstFrame(gifHandle);
        pImgDecAttrib->pOutImgBuf = (T_pDATA)GIFEnum_GetCurBMP(gifHandle, &pImgDecAttrib->GifCurFrameSize, &bitsPerPix);
        if (AK_NULL != pImgDecAttrib->pOutImgBuf)
        {
            delay = GIFEnum_GetCurDelay(gifHandle);
			
			Interval = (delay < 100) ? 100 : delay;
            if (pImgDecAttrib->GifFrameInterval != (T_U32)Interval)
            {
				pImgDecAttrib->GifFrameInterval = Interval;
				pImgDecAttrib->bIntervalChange = AK_TRUE;
			}

            ret = AK_TRUE;
        }
    }

    return ret;
}

T_hGIFENUM ImgDec_GifDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    if (pImgDecAttrib->GifHandle != IMG_INVALID_HANDLE)
    {
        GIFEnum_Close(pImgDecAttrib->GifHandle);
        pImgDecAttrib->GifHandle = IMG_INVALID_HANDLE;
        pImgDecAttrib->pOutImgBuf = AK_NULL;
        if (AK_NULL != pImgDecAttrib->pInImgBuf)
        {
            pImgDecAttrib->pInImgBuf = Fwl_Free(pImgDecAttrib->pInImgBuf);
        }
    }

    return IMG_INVALID_HANDLE;
}

T_BOOL ImgDec_PngDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    T_U8            bitCount;
    T_U16           pngW, pngH;
    T_pCDATA        pPngBuf;

	AK_ASSERT_PTR(pImgDecAttrib, "pImgDecAttrib Is Invalid", AK_FALSE);
	
    pPngBuf = pImgDecAttrib->pInImgBuf;
	AK_ASSERT_PTR(pPngBuf, "pPngBuf Is Invalid", AK_FALSE);

    if (pPngBuf
		&& Img_PNGInfo(pPngBuf, &pngW, &pngH, &bitCount))
    {		
        pImgDecAttrib->InImgW = (T_U32)pngW;
        pImgDecAttrib->InImgH = (T_U32)pngH;
        
		if (pngW > SUPPORT_MAX_IMG_SIZE || pngH > SUPPORT_MAX_IMG_SIZE)
		{
			T_U16 dstW, dstH;
		 
			Eng_GetAspectRatio(&dstW, &dstH, pngW, pngH, SUPPORT_MAX_IMG_SIZE);
			pngW = dstW;
			pngH = dstH;
		}
		
		if (AK_NULL == (pImgDecAttrib->pOutImgBuf = Img_PNG2BMPEx(pPngBuf, &pImgDecAttrib->OutImgSize, pngW, pngH)))
		{
			Fwl_Print(C2, M_IMAGE, "DEC PNG_EX Failure");
			return AK_FALSE;
		}

		pImgDecAttrib->OutImgW = (T_U32)pngW;
       	pImgDecAttrib->OutImgH = (T_U32)pngH;
    }

    return AK_TRUE;
}

T_pDATA ImgDec_PngDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    if (AK_NULL != pImgDecAttrib->pOutImgBuf)
    {
        pImgDecAttrib->pOutImgBuf = Fwl_Free(pImgDecAttrib->pOutImgBuf);
    }

    return AK_NULL;
}

T_BOOL ImgDec_JpgDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib, T_BOOL autoShorten, T_BOOL *pLargeFlg)
{
    T_BOOL  ret = AK_FALSE;
    T_U32   jpgW = 0;
    T_U32   jpgH = 0;
    T_U32   outBufSize = 0;
    T_U32   ExtLCDHeight = 0;
    T_U32   ExtLCDWidth = 0;
    T_U32   dstW = 0;
    T_U32   dstH = 0;
    T_pCDATA pJpgBuf;
    T_U32 jpgBufSize;

    T_S32   nFactor = 0;
    T_S32   nZoom = DIGITAL_ZOOM * DIGITAL_ZOOM;
    T_S32   nMemSize = 0;
    T_DBL   dblSrc;
    T_DBL   dblResult;
    T_S32 nPowCount = 0;

    pJpgBuf = pImgDecAttrib->pInImgBuf;
    jpgBufSize = pImgDecAttrib->InImgSize;

#if (SDRAM_MODE == 8)
    if (FILE_TYPE_MAP == pImgDecAttrib->FileType)
    {
        ExtLCDHeight = EMAP_AUTOSHORT_HEIGHT;
        ExtLCDWidth =  EMAP_AUTOSHORT_WIDTH;
    }
    else
    {
        ExtLCDHeight = JPG_AUTOSHORT_HEIGHT;
        ExtLCDWidth =  JPG_AUTOSHORT_WIDTH;
    }
#else
    ExtLCDHeight = JPG_AUTOSHORT_HEIGHT;
    ExtLCDWidth =  JPG_AUTOSHORT_WIDTH;
#endif

    if (pJpgBuf)
    {
        *pLargeFlg = AK_FALSE;
        if (Img_JpegInfo(pJpgBuf, jpgBufSize, (T_U16 *)&jpgW, (T_U16 *)&jpgH, AK_NULL, AK_NULL) > 0)
        {
            pImgDecAttrib->InImgW = jpgW;
            pImgDecAttrib->InImgH = jpgH;
            outBufSize = jpgBufSize;
			
            if (jpgW <= (T_U32)Fwl_GetLcdWidth() && jpgH <= (T_U32)Fwl_GetLcdHeight())
            {
                pImgDecAttrib->OutImgW = jpgW;
                pImgDecAttrib->OutImgH = jpgH;
                pImgDecAttrib->pOutImgBuf = Img_Jpeg2BMP_Mutex(pJpgBuf, &outBufSize);
				if (AK_NULL == pImgDecAttrib->pOutImgBuf)
				{
					Fwl_Print(C3, M_IMAGE, "DEC JPEG FAILURE");
					pImgDecAttrib->pOutImgBuf = Img_Jpeg2BMPSoft(pJpgBuf, &outBufSize);
				}
                pImgDecAttrib->OutImgSize = outBufSize;

                ret = (AK_NULL != pImgDecAttrib->pOutImgBuf) ? AK_TRUE : AK_FALSE;
            }
            // New image lib support large image, not limit.
            //else if ((jpgW * jpgH) <= (IMAGE_WIDTH_MAX * IMAGE_HEIGHT_MAX))
            else
            {
                if (autoShorten && (jpgW > ExtLCDWidth || jpgH > ExtLCDHeight))
                {
                    ImgDec_GetDstWH(jpgW, jpgH, ExtLCDWidth, ExtLCDHeight, &dstW, &dstH);
                }
                else
                {
                    dstW = jpgW;
                    dstH = jpgH;
                }

                if (0 != dstW || 0 != dstH)
                {
                    //maybe the memory is not enough for the large image file ,so ,adjust the destination scale of the image converted                    

                    //Fwl_Print(C3, M_IMAGE, "debug token: 15# , dstw=%d , dstH = %d, remSize = %d, usedSize =%d" ,dstW , dstH, Fwl_GetRemainRamSize(), Fwl_GetUsedRamSize());

                    nMemSize = Fwl_GetLargestSize_Allocable();

                    if((nMemSize > 0) && (nMemSize < (T_S32)(dstW * dstH * 3)) && (dstW != 0 || dstH != 0))
                    {
                        T_DBL  dblData1 = Fwl_DblInit(nMemSize,0);
                        T_DBL  dblData2 = Fwl_DblInit(nZoom,0);
                        T_DBL  dblData3=  Fwl_DblInit(dstW, 0);
                        T_DBL  dblData4 = Fwl_DblInit(dstH, 0);

                        dblData1 = Fwl_DblMulti(dblData1,dblData2);

                        dblData3 = Fwl_DblMulti(dblData3,dblData4);
                        dblData3 = Fwl_DblMulti(dblData3, Fwl_DblInit(3,0));

                        dblSrc = Fwl_DblDivide(dblData1, dblData3);                        

                        dblResult = Fwl_DblSqrt(dblSrc);

                        Fwl_Print(C3, M_IMAGE, "dblResult.Number =%d, dblResult.Power=%d" , dblResult.Number , dblResult.Power);

                        nPowCount = dblResult.Power;
                        nFactor = dblResult.Number;

                        if(nPowCount > 0)
                        {
                            while(nPowCount > 0)
                            {
                                nFactor = nFactor * 10;
                                nPowCount = nPowCount - 1; 
                            }
                        }
                        else if(nPowCount < 0)
                        {
                            while(nPowCount < 0)
                            {
                                nFactor = nFactor / 10;
                                nPowCount  = nPowCount + 1;
                            }
                        }

                        Fwl_Print(C3, M_IMAGE, "token2 dstW = %d , dstH = %d, nFactor = %d" , dstW, dstH, nFactor);

                        if(DIGITAL_ZOOM != 0)
                        {
                            dstW = dstW * nFactor / DIGITAL_ZOOM;
                            dstH = dstH * nFactor / DIGITAL_ZOOM;
                        }
                    }
                                       
                    pImgDecAttrib->OutImgW = dstW;
                    pImgDecAttrib->OutImgH = dstH;
                    Fwl_Print(C3, M_IMAGE, "debug token: 13.1# , dstw=%d , dstH = %d , nFactor = %d" ,dstW , dstH, nFactor);

					pImgDecAttrib->pOutImgBuf = Img_Jpeg2BMPEx_Mutex(pJpgBuf, (T_U16)dstW, (T_U16)dstH, &outBufSize);
					if (AK_NULL == pImgDecAttrib->pOutImgBuf)
					{
						Fwl_Print(C3, M_IMAGE, "DEC JPEG EX FAilure\n");
						pImgDecAttrib->pOutImgBuf = Img_Jpeg2BMPExSoft(pJpgBuf, (T_U16)dstW, (T_U16)dstH, &outBufSize);
					}

                    Fwl_Print(C3, M_IMAGE, "debug token: 13.2# , outBufSize = %d", outBufSize);
                    pImgDecAttrib->OutImgSize = outBufSize;

                    ret = (AK_NULL != pImgDecAttrib->pOutImgBuf) ? AK_TRUE : AK_FALSE;
                }
            }
            /* New image lib support large image, not limit.
            else
            {
                *pLargeFlg = AK_TRUE;
            }*/
        }
        else
        {
            Fwl_Print(C3, M_IMAGE, "ImgDec_JpgDecOpen ,failed to get the info of the map");
        }
    }
    return ret;
}

T_pDATA ImgDec_JpgDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    if (AK_NULL != pImgDecAttrib->pOutImgBuf)
    {
        pImgDecAttrib->pOutImgBuf = Fwl_Free(pImgDecAttrib->pOutImgBuf);
    }

    return AK_NULL;
}

T_VOID ImgDec_LargeJpgDecClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    T_IMGDEC_ATTRIB *pImgDecAttrib;

    pImgDecAttrib = &pLargeAttrib->ImgAttrib;

    if (AK_NULL != pImgDecAttrib->pOutImgBuf)
    {
        pImgDecAttrib->pOutImgBuf = Fwl_Free(pImgDecAttrib->pOutImgBuf);
    }
    if (AK_NULL != pLargeAttrib->pOutLineBuf)
    {
        pLargeAttrib->pOutLineBuf = Fwl_Free(pLargeAttrib->pOutLineBuf);
    }

    if (AK_NULL != pImgDecAttrib->pInImgBuf)
    {
        pImgDecAttrib->pInImgBuf = Fwl_Free(pImgDecAttrib->pInImgBuf);
    }

	JpegFree();
}

T_LARGE_IMG_STATUS ImgDec_LargeJpgDecStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    T_U32 x;
    T_U8 *lpSrc, *lpDst;
    T_U32 jpgW, jpgH, outImgW, outImgH;
    T_LARGE_IMG_STATUS LargeDecRet = DEC_ERROR;
	T_S32 CurLines = 0;
	
    if (AK_NULL == pLargeAttrib || DEC_CONTINUE != pLargeAttrib->DecStatus)
        return DEC_ERROR;

    jpgW = pLargeAttrib->ImgAttrib.InImgW;
    jpgH = pLargeAttrib->ImgAttrib.InImgH;
    outImgW = pLargeAttrib->ImgAttrib.OutImgW;
    outImgH = pLargeAttrib->ImgAttrib.OutImgH;

    while (pLargeAttrib->TotalLines <= pLargeAttrib->CurY*jpgH/outImgH)
    {
    	CurLines = JpegDecodeLine(pLargeAttrib->pOutLineBuf);
        
        if (CurLines < 0)
        {
            ImgDec_LargeJpgDecClose(pLargeAttrib);
            return LargeDecRet;
        }
		else
		{
			pLargeAttrib->CurLines = (T_U32)CurLines;
		}
        pLargeAttrib->TotalLines += pLargeAttrib->CurLines;
    }
    lpSrc = pLargeAttrib->pOutLineBuf + \
            (pLargeAttrib->CurY*jpgH/outImgH+(pLargeAttrib->CurLines)-(pLargeAttrib->TotalLines))*jpgW*3;
    lpDst = pLargeAttrib->ImgAttrib.pOutImgBuf + BMP_HEAD_SIZE + \
            (outImgH-pLargeAttrib->CurY-1)*(((outImgW*24+31)>>5)<<2);

    for (x=0; x<outImgW; x++)
    {
        memcpy((void *)(lpDst+x*3), (void *)(lpSrc+x*jpgW/outImgW*3), 3);
    }
    pLargeAttrib->CurY++;
    if (pLargeAttrib->CurY >= pLargeAttrib->ImgAttrib.OutImgH)
    {
        pLargeAttrib->DecStatus = DEC_COMPLETE;
        LargeDecRet = DEC_COMPLETE;
    }
    else
    {
        LargeDecRet = DEC_CONTINUE;
    }
    return LargeDecRet;
}

T_BOOL ImgDec_LargeJpgDecOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_pCWSTR pJpgPath)
{
    T_U16 jpgW = 0;
    T_U16 jpgH = 0;
    //T_pDATA pOutImgBuf = AK_NULL;
    T_U32 outImgW, outImgH;
    T_pFILE     fp = _FOPEN_FAIL;
    T_U32 imgSize;
    T_IMGDEC_ATTRIB *pImgAttrib;

    if (AK_NULL == pJpgPath)
    {
        return AK_FALSE;
    }

    pImgAttrib = &pLargeAttrib->ImgAttrib;

    if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf)
        pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
    if (AK_NULL != pLargeAttrib->pOutLineBuf)
        pLargeAttrib->pOutLineBuf = Fwl_Free(pLargeAttrib->pOutLineBuf);
    if (AK_NULL != pImgAttrib->pInImgBuf)
        pImgAttrib->pInImgBuf = Fwl_Free(pImgAttrib->pInImgBuf);

    fp = Fwl_FileOpen(pJpgPath, _FMODE_READ, _FMODE_READ);
    if (_FOPEN_FAIL == fp)
    {
        return AK_FALSE;
    }
    imgSize = Fwl_GetFileLen(fp);
    pImgAttrib->pInImgBuf = (T_U8 *)Fwl_Malloc(imgSize + 16);
    if (AK_NULL == pImgAttrib->pInImgBuf)
    {
        Fwl_FileClose(fp);
        return AK_FALSE;
    }
    Fwl_FileRead(fp, pImgAttrib->pInImgBuf, imgSize);
    Fwl_FileClose(fp);
    fp = _FOPEN_FAIL;

    if (JpegInit(pImgAttrib->pInImgBuf, JPEG_TO_RGBLINE, &jpgW, &jpgH) > 0)
    {
        pImgAttrib->InImgW = jpgW;
        pImgAttrib->InImgH = jpgH;
        ImgDec_GetDstWH((T_U32)jpgW, (T_U32)jpgH, LARGE_DEC_JPG_DST_WIDTH_MAX, LARGE_DEC_JPG_DST_HEIGHT_MAX, &outImgW, &outImgH);
        pImgAttrib->OutImgW = outImgW;
        pImgAttrib->OutImgH = outImgH;

        pLargeAttrib->ImgAttrib.pOutImgBuf = (T_pDATA)Fwl_Malloc(outImgH*(((outImgW*24+31)>>5)<<2)+BMP_HEAD_SIZE);
        //pLargeAttrib->pOutLineBuf = (T_pDATA)Fwl_Malloc(16*jpgW*3);
        pLargeAttrib->pOutLineBuf = (T_pDATA)Fwl_Malloc(16*(((jpgW*24+31)>>5)<<2));
        if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf && AK_NULL != pLargeAttrib->pOutLineBuf)
        {
            memset((void *)pLargeAttrib->ImgAttrib.pOutImgBuf, 0x00, outImgH*(((outImgW*24+31)>>5)<<2)+BMP_HEAD_SIZE);
            ImgDec_WriteBMPHead(pLargeAttrib->ImgAttrib.pOutImgBuf, 24, 0, outImgW, outImgH);

            pLargeAttrib->CurY = 0;
            pLargeAttrib->CurLines = 0;
            pLargeAttrib->TotalLines = 0;
            pLargeAttrib->DecStatus = DEC_CONTINUE;

            return AK_TRUE;
        }
        else
        {
            if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf)
                pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
            if (AK_NULL != pLargeAttrib->pOutLineBuf)
                pLargeAttrib->pOutLineBuf = Fwl_Free(pLargeAttrib->pOutLineBuf);
            if (AK_NULL != pImgAttrib->pInImgBuf)
                pImgAttrib->pInImgBuf = Fwl_Free(pImgAttrib->pInImgBuf);
        }
    }

    return AK_FALSE;
}

T_pDATA ImgDec_BmpDecGetRgbBufPtr(T_pCDATA pBmpBuf)
{
    T_pDATA pRGBData = AK_NULL;
    T_U16 deep = 0;
    T_U32 compression = 0;
    T_U32 width = 0;
    T_U32 height = 0;

    if (AK_NULL != pBmpBuf)
    {
        if (ImgDec_DecBMPHead(pBmpBuf, &deep, &compression, &width, &height) == AK_FALSE)
            return pRGBData;
        if (deep >= 24)
        {
            pRGBData = (T_pDATA)pBmpBuf + BMP_HEAD_SIZE;
        }
        else
        {
            if (deep == 16)
            {
                if (compression == 3)
                {
                    pRGBData = (T_pDATA)pBmpBuf + BMP_HEAD_SIZE + 3*4;
                }
                else if (compression != 0)
                {
                    return pRGBData;
                }
            }
            else
            {
                pRGBData = (T_pDATA)pBmpBuf + BMP_HEAD_SIZE + PaletteNum(deep)*4;
            }
        }
    }

    return pRGBData;
}

T_VOID ImgDec_LargeBmpDecClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    if (_FOPEN_FAIL != pLargeAttrib->pLargeBmpFp)
    {
        Fwl_FileClose(pLargeAttrib->pLargeBmpFp);
        pLargeAttrib->pLargeBmpFp = _FOPEN_FAIL;
    }

    if (AK_NULL != pLargeAttrib->pOutLineBuf)
        pLargeAttrib->pOutLineBuf = Fwl_Free(pLargeAttrib->pOutLineBuf);

    if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf)
        pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
}

T_LARGE_IMG_STATUS ImgDec_LargeBmpDecStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    T_U8            *lpSrc, *lpDst;
    T_pDATA         pRGBData = AK_NULL;
    T_U32           x;
    T_U8            src, dst;
    T_U32           bmpW, bmpH, outImgW, outImgH;
    T_U16           bmpDeep;
    T_U16           OutImgDeep;
    T_LARGE_IMG_STATUS LargeDecRet = DEC_ERROR;
    T_U32           i;
    T_U32           TmpCalc;
    T_U32           compression;
    T_U32           ReadByte = 0;
    T_U32           ErrReadCount = 0; // just and special for file system can not read data and return 512 status

    if (AK_NULL == pLargeAttrib && DEC_CONTINUE != pLargeAttrib->DecStatus)
        return DEC_ERROR;

    bmpW = pLargeAttrib->ImgAttrib.InImgW;
    bmpH = pLargeAttrib->ImgAttrib.InImgH;
    outImgW = pLargeAttrib->ImgAttrib.OutImgW;
    outImgH = pLargeAttrib->ImgAttrib.OutImgH;
    bmpDeep = pLargeAttrib->ImgAttrib.BmpDeep;
    OutImgDeep = pLargeAttrib->ImgAttrib.OutImgDeep;
    compression = pLargeAttrib->ImgAttrib.BmpCompression;

    TmpCalc = pLargeAttrib->CurY * bmpH / outImgH;

    while (pLargeAttrib->TotalLines <= TmpCalc)
    {
        ReadByte = Fwl_FileRead(pLargeAttrib->pLargeBmpFp, pLargeAttrib->pOutLineBuf, BMP_OUT_LINE_NUM*(((bmpW * bmpDeep + 31) >> 5) << 2));
        // now if file system read file error, it will not return less than 0, but return 0
        ErrReadCount = (ReadByte != 0) ? 0 : (ErrReadCount + 1);
        if (ReadByte == 0)
        {
            Fwl_Print(C3, M_IMAGE, "ReadByte == 0, read may be some error");
        }
        // now if the file system can not read data, it will return 0
        //if (ReadByte < 0 || (ReadByte == 0 && ErrReadCount >= 2))
		if ((0 == ReadByte) && (ErrReadCount >= 2))
        {
            ImgDec_LargeBmpDecClose(pLargeAttrib);
            return LargeDecRet;
        }

        pLargeAttrib->CurLines = ReadByte / (((bmpW * bmpDeep + 31) >> 5) << 2);
        pLargeAttrib->TotalLines += pLargeAttrib->CurLines;
    }

    for (i=0; i<BMP_OUT_LINE_NUM && pLargeAttrib->TotalLines > TmpCalc; i++)
    {
        if (pLargeAttrib->CurY < outImgH)
        {
            lpSrc = pLargeAttrib->pOutLineBuf + (pLargeAttrib->CurY * bmpH / outImgH + pLargeAttrib->CurLines - pLargeAttrib->TotalLines)*(((bmpW * bmpDeep + 31) >> 5) << 2);
            pRGBData = (compression == 0) ? pLargeAttrib->ImgAttrib.pOutImgBuf + BMP_HEAD_SIZE \
                                          : pLargeAttrib->ImgAttrib.pOutImgBuf + BMP_HEAD_SIZE + 12;
            lpDst = pRGBData + pLargeAttrib->CurY * (((outImgW * OutImgDeep + 31) >> 5) << 2);

            if (bmpDeep >= 8)
            {
                for (x=0; x < outImgW; x++)
                    memcpy((void *)(lpDst+((x*OutImgDeep) >> 3)), (void *)(lpSrc+((x*bmpW/outImgW*bmpDeep)>>3)), OutImgDeep >> 3);
            }
            else if (bmpDeep == 4)
            {
                for (x=0; x<outImgW; x++)
                {
                    src = *(lpSrc+x*bmpW*bmpDeep/(outImgW<<3));
                    dst = *(lpDst+((x*OutImgDeep)>>3));
                    *(lpDst+((x*OutImgDeep) >> 3)) = (x%2==0) ? ((dst&0x0f)|(src&0xf0)) : ((dst&0xf0)|(src&0x0f));
                }
            }
            else if (bmpDeep == 1)
            {
                T_U8 mask, rmask;

                for (x=0; x<outImgW; x++)
                {
                    src = *(lpSrc+x*bmpW*bmpDeep/(outImgW << 3));
                    dst = *(lpDst+((x*OutImgDeep)>>3));
                    mask = 1<<(x%8);
                    rmask = ~mask;
                    *(lpDst+((x*OutImgDeep)>>3)) = (dst&rmask)|(src&mask);
                }
            }
            else
            {
                ImgDec_LargeBmpDecClose(pLargeAttrib);
                return LargeDecRet;
            }

            pLargeAttrib->CurY++;
            TmpCalc = pLargeAttrib->CurY * bmpH / outImgH;
       }
    }

    if (pLargeAttrib->CurY >= pLargeAttrib->ImgAttrib.OutImgH)
    {
        // The bmp decode complete release the bmp file handle
        // so we can go to menu and delete the bmp file
        if (_FOPEN_FAIL != pLargeAttrib->pLargeBmpFp)
        {
            Fwl_FileClose(pLargeAttrib->pLargeBmpFp);
            pLargeAttrib->pLargeBmpFp = _FOPEN_FAIL;
        }
        pLargeAttrib->DecStatus = DEC_COMPLETE;
        LargeDecRet = DEC_COMPLETE;
    }
    else
    {
        LargeDecRet = DEC_CONTINUE;
    }

    return LargeDecRet;
}
T_BOOL ImgDec_LargeBmpDecOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_pCWSTR pBmpPath, T_U32 maxW, T_U32 maxH)
{
    T_U8        headBuf[BMP_HEAD_SIZE+1];
    T_U32       bmpW, bmpH, outImgW, outImgH;
    T_U16       bmpDeep, outImgDeep;
    T_U32       compression;
    T_U8        *pMaskRGB = AK_NULL;

    if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf)
        pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
    if (AK_NULL != pLargeAttrib->pOutLineBuf)
        pLargeAttrib->pOutLineBuf = Fwl_Free(pLargeAttrib->pOutLineBuf);
    if (AK_NULL != pLargeAttrib->ImgAttrib.pInImgBuf)
        pLargeAttrib->ImgAttrib.pInImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pInImgBuf);

    pLargeAttrib->ImgAttrib.pOutImgBuf = (T_pDATA)Fwl_Malloc(BMP_HEAD_SIZE + maxW * maxH * 3 + 15);
    if (AK_NULL != pLargeAttrib->ImgAttrib.pOutImgBuf)
    {
        memset((void *)(pLargeAttrib->ImgAttrib.pOutImgBuf), 0x00, BMP_HEAD_SIZE + maxW * maxH * 3 + 15);
        pLargeAttrib->pLargeBmpFp = Fwl_FileOpen(pBmpPath, _FMODE_READ, _FMODE_READ);
        if (_FOPEN_FAIL != pLargeAttrib->pLargeBmpFp)
        {
            Fwl_FileRead(pLargeAttrib->pLargeBmpFp, headBuf, BMP_HEAD_SIZE);

            if (ImgDec_DecBMPHead(headBuf, &bmpDeep, &compression, &bmpW, &bmpH) == AK_TRUE)
            {
                outImgDeep = (bmpDeep > 24) ? 24 : bmpDeep;

                if (bmpW <= (T_U32)maxW 
                && bmpH <= (T_U32)maxH)
				{
					outImgW = bmpW;
					outImgH = bmpH;
				}
				else
				{
					if (bmpW * maxH >= bmpH * maxW)
					{
		                outImgW = maxW;
		                outImgH = maxW * bmpH / bmpW;
					}
					else
					{
						outImgH = maxH;
		                outImgW = maxH * bmpW / bmpH;
					}
				}

                pLargeAttrib->pOutLineBuf = (T_U8 *)Fwl_Malloc(BMP_OUT_LINE_NUM*(((bmpW * bmpDeep + 31) >> 5) << 2)+1);
                if (AK_NULL != pLargeAttrib->pOutLineBuf)
                {
                    pLargeAttrib->CurY = 0;
                    pLargeAttrib->CurLines = 0;
                    pLargeAttrib->TotalLines = 0;
                    pLargeAttrib->ImgAttrib.InImgW = bmpW;
                    pLargeAttrib->ImgAttrib.InImgH = bmpH;
                    pLargeAttrib->ImgAttrib.OutImgW = outImgW;
                    pLargeAttrib->ImgAttrib.OutImgH = outImgH;
                    pLargeAttrib->ImgAttrib.BmpDeep = bmpDeep;
                    pLargeAttrib->ImgAttrib.OutImgDeep = outImgDeep;
                    pLargeAttrib->ImgAttrib.BmpCompression = compression;
                    
					memcpy(pLargeAttrib->ImgAttrib.pOutImgBuf, headBuf, BMP_HEAD_SIZE);
                    ImgDec_WriteBMPHead(pLargeAttrib->ImgAttrib.pOutImgBuf, outImgDeep, compression, outImgW, outImgH);
                    pLargeAttrib->DecStatus = DEC_CONTINUE;
                    if (compression == 0)
                        Fwl_FileSeek(pLargeAttrib->pLargeBmpFp, BMP_HEAD_SIZE, _FSEEK_SET);
                    else if (compression == 3)
                    {
                        pMaskRGB = pLargeAttrib->ImgAttrib.pOutImgBuf + BMP_HEAD_SIZE;
                        Fwl_FileRead(pLargeAttrib->pLargeBmpFp, pMaskRGB, 12);
                        Fwl_FileSeek(pLargeAttrib->pLargeBmpFp, BMP_HEAD_SIZE + 12, _FSEEK_SET);
                    }

                    return AK_TRUE;
                }
                else
                {
                    Fwl_FileClose(pLargeAttrib->pLargeBmpFp);
                    pLargeAttrib->pLargeBmpFp = _FOPEN_FAIL;
                    pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
                }
            }
            else
            {
                Fwl_FileClose(pLargeAttrib->pLargeBmpFp);
                pLargeAttrib->pLargeBmpFp = _FOPEN_FAIL;
                pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
            }
        }
        else
        {
            pLargeAttrib->ImgAttrib.pOutImgBuf = Fwl_Free(pLargeAttrib->ImgAttrib.pOutImgBuf);
        }
    }

    return AK_FALSE;
}

T_VOID ImgDec_WriteBMPHead(T_U8 *headBuf, T_U16 deep, T_U32 compression, T_U32 width, T_U32 height)
{
    if (headBuf == AK_NULL)
        return;

    memcpy((void *)(headBuf+0x12), (void *)&width, 4);
    memcpy((void *)(headBuf+0x16), (void *)&height, 4);
    memcpy((void *)(headBuf+0x1c), (void *)&deep, 2);
    memcpy((void *)(headBuf+0x1e), (void *)&compression, 4);
}

T_BOOL ImgDec_DecBMPHead(T_pCDATA pHeadBuf, T_U16 *pDeep, T_U32 *pCompression, T_U32 *pBmpW, T_U32 *pBmpH)
{
    T_BOOL  ret;
    T_U32   headsize = 0;

    AK_ASSERT_PTR(pHeadBuf, "ImgDec_DecBMPHead error!",AK_FALSE);

    memcpy((void *)(&headsize), (void *)&pHeadBuf[0x0E], 4);

    if ((pHeadBuf == AK_NULL) || (*pHeadBuf != 'B') || (*(pHeadBuf+1) != 'M') || headsize != 40)
    {
        ret = AK_FALSE;
    }
    else
    {
        if (AK_NULL != pBmpW)
        {
            memcpy((void *)pBmpW, (void *)&pHeadBuf[0x12], 4);
        }

        if (AK_NULL != pBmpH)
        {
            memcpy((void *)pBmpH, (void *)&pHeadBuf[0x16], 4);
        }

        if (AK_NULL != pDeep)
        {
            memcpy((void *)pDeep, (void *)&pHeadBuf[0x1c], 2);
        }

        if (AK_NULL != pCompression)
        {
            memcpy((void *)pCompression, (void *)&pHeadBuf[0x1e], 4);
        }

        ret = AK_TRUE;
    }
    return ret;
}

T_VOID ImgDec_GetBmpSize(T_U8 *bmp_buf, T_U32 *width, T_U32 *height)
{
    memcpy((void *)width, (void *)&bmp_buf[0x12], 4);
    memcpy((void *)height, (void *)&bmp_buf[0x16], 4);
}

T_U32 ImgDec_GetBMPDataLen(T_U32 width, T_U32 height, T_U16 deep, T_U32 compression)
{
    T_U32 len = 0;

    if ((deep == 1) || (deep == 4) || (deep == 8))
    {
        len += (BMP_HEAD_SIZE+PaletteNum(deep)*4);
        len += (height*(((width*deep+31)>>5)<<2));
    }
    else if (deep == 16)
    {
        if (compression == 0)
            len += (BMP_HEAD_SIZE);
        else if (compression == 3)
            len += (BMP_HEAD_SIZE+3*4);
    }
    else if ((deep == 24) || (deep == 32)||(deep == 48) || (deep == 64))
    {
        len += (BMP_HEAD_SIZE);
        len += (height*(((width*deep+31)>>5)<<2));
    }

    return len;
}

T_pDATA ImgDec_GetImageData(T_USTR_FILE file_path)
{
    T_pFILE fp;
    T_U32 fsize;
    T_U8 *bmp_buf = AK_NULL, *tmp_buf;
    T_U8 file_type;

    if ((fsize = Fwl_FileGetSize(file_path)) > 0)
    {
        file_type = Utl_GetFileType(file_path);
        if (file_type == FILE_TYPE_BMP)
        {
            fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ);
            if (fp != _FOPEN_FAIL)
            {
                if (fsize > 0)
                {
                    bmp_buf = (T_U8 *)Fwl_Malloc(fsize);
                    if (bmp_buf != AK_NULL)
                        Fwl_FileRead(fp, bmp_buf, fsize);
                }

                Fwl_FileClose(fp);
            }
        }
        else if (file_type == FILE_TYPE_JPG)
        {
            fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ);
            if (fp != _FOPEN_FAIL)
            {
                if (fsize > 0)
                {
                    tmp_buf = (T_U8 *)Fwl_Malloc(fsize);
                    if (tmp_buf != AK_NULL)
                    {
                        T_U16 width = 0, height = 0;
                        T_U32 outLen;

                        Fwl_FileRead(fp, tmp_buf, fsize);
                        outLen = fsize;
                        if (Img_JpegInfo(tmp_buf,fsize, &width, &height, AK_NULL, AK_NULL) > 0)
                        {
                            if (width <= (T_U16)Fwl_GetLcdWidth() &&
                                height <= (T_U16)Fwl_GetLcdHeight())
                            {
                                bmp_buf = Img_Jpeg2BMP_Mutex(tmp_buf, &outLen);
								if (AK_NULL == bmp_buf)
								{
									 bmp_buf = Img_Jpeg2BMPSoft(tmp_buf, &outLen);
								}
                            }
                            else
                            {
                                T_U32 srcW, srcH, dstW, dstH, tmpW, tmpH;

                                srcW = (T_U32)width;
                                srcH = (T_U32)height;
                                dstW = (T_U32)Fwl_GetLcdWidth();
                                dstH = (T_U32)Fwl_GetLcdHeight();

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
                                bmp_buf = Img_Jpeg2BMPEx_Mutex(tmp_buf, (T_U16)tmpW, (T_U16)tmpH, &outLen);
								if (AK_NULL == bmp_buf)
								{
									bmp_buf = Img_Jpeg2BMPExSoft(tmp_buf, (T_U16)tmpW, (T_U16)tmpH, &outLen);
								}
                            }

                        }
                        tmp_buf = Fwl_Free(tmp_buf);
                    }
                }
                Fwl_FileClose(fp);
            }
        }
        else if (file_type == FILE_TYPE_PNG)
        {
            fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ);
            if (fp != _FOPEN_FAIL)
            {
                if (fsize > 0)
                {
                    tmp_buf = (T_U8 *)Fwl_Malloc(fsize);
                    if (tmp_buf != AK_NULL)
                    {
                        T_U32 outLen;

                        Fwl_FileRead(fp, tmp_buf, fsize);
                        bmp_buf = Img_PNG2BMP(tmp_buf, &outLen);

                        tmp_buf = Fwl_Free(tmp_buf);
                    }
                }

                Fwl_FileClose(fp);
            }
        }
    }

    return bmp_buf;
}


T_pDATA ImgDec_GetImageDataFromBuf(const T_U8 *img_buf, T_U32 img_Size)
{
	T_hGIFENUM  hGif = IMG_INVALID_HANDLE;
    T_U32 		fsize = img_Size;
    T_U8 		*bmp_buf = AK_NULL;
    T_U8 		*tmp_buf;
    T_FILE_TYPE file_type;

    tmp_buf = (T_U8*)img_buf;

    file_type = ImgDec_GetImgType(tmp_buf);

  	switch(file_type)
  	{
    case FILE_TYPE_BMP:
		return tmp_buf;
    	break;
    case FILE_TYPE_JPG:
	    {
	      T_U16 width = 0, height = 0;
	      T_U32 outLen;


	      outLen = fsize;
	      if (Img_JpegInfo(tmp_buf, fsize, &width, &height, AK_NULL, AK_NULL) > 0)
	      {
	          if (width <= (T_U16)Fwl_GetLcdWidth() &&
	              height <= (T_U16)Fwl_GetLcdHeight())
	          {
	              bmp_buf = Img_Jpeg2BMP_Mutex(tmp_buf, &outLen);
				  if (AK_NULL == bmp_buf)
				  {
					 bmp_buf = Img_Jpeg2BMPSoft(tmp_buf, &outLen);
				  }
	          }
	          else
	          {
	              T_U32 srcW, srcH, dstW, dstH, tmpW, tmpH;

	              srcW = (T_U32)width;
	              srcH = (T_U32)height;
	              dstW = (T_U32)Fwl_GetLcdWidth();
	              dstH = (T_U32)Fwl_GetLcdHeight();

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
	              bmp_buf = Img_Jpeg2BMPEx_Mutex(tmp_buf, (T_U16)tmpW, (T_U16)tmpH, &outLen);
				if (AK_NULL == bmp_buf)
				{
					bmp_buf = Img_Jpeg2BMPExSoft(tmp_buf, (T_U16)tmpW, (T_U16)tmpH, &outLen);
				}
	          }
	       }
	    }
	    break;
    case FILE_TYPE_PNG:
	    {
	        T_U32 outLen;

	        bmp_buf = Img_PNG2BMP(tmp_buf, &outLen);
	    }
	    break;
    case FILE_TYPE_GIF:
	    {
	       T_U8    bitsPerPix;

	       hGif = GIFEnum_New(tmp_buf, fsize);
	       if (IMG_INVALID_HANDLE == hGif)
	       {
	           return AK_NULL;
	       }
	       GIFEnum_FirstFrame(hGif);
	       bmp_buf = (T_U8*)GIFEnum_GetCurBMP(hGif, &fsize, &bitsPerPix);
	    }
	    break;
	default:
	    break;
    }

    return bmp_buf;
}


T_VOID *ImgDec_FreeImageData(T_U8 *bmp_buf)
{
    if (bmp_buf != AK_NULL)
        bmp_buf = Fwl_Free(bmp_buf);
    return AK_NULL;
}


T_FILE_TYPE ImgDec_GetImgType(const T_U8 *img_buf)
{
    T_IMAGE_TYPE img_type;
    T_FILE_TYPE  ret_type;

    img_type = Img_ImageType(img_buf);
    switch(img_type)
    {
        case IMG_IMAGE_BMP:
        case IMG_IMAGE_WBMP:
            ret_type = FILE_TYPE_BMP;
            break;

        case IMG_IMAGE_JPG:
            ret_type = FILE_TYPE_JPG;
            break;

        case IMG_IMAGE_GIF:
            ret_type = FILE_TYPE_GIF;
            break;

        case IMG_IMAGE_PNG:
            ret_type = FILE_TYPE_PNG;
            break;

        default:
            ret_type = FILE_TYPE_NONE;
    }

    return ret_type;
}


T_BOOL ImgDec_ImgOpen(T_IMGDEC_ATTRIB *pImgAttrib, T_USTR_FILE pFilePath, T_BOOL autoShorten, T_BOOL *pLargeFlag)
{
    T_pFILE     fp = _FOPEN_FAIL;
    T_U16       BmpDeep = 0;
    T_U32       BmpCompression = 0;
    T_U32       ImgW = 0;
	T_U32		ImgH = 0;
    T_BOOL      ret = AK_FALSE;
    T_U8        pBmpHeadBuf[BMP_HEAD_SIZE+1] = {0};
    T_U8        typeInfo[IMG_TYPE_INFO_SIZE] = {0};

    if (AK_NULL == pFilePath 
		|| AK_NULL == pImgAttrib 
		|| AK_NULL == pLargeFlag)
        return AK_FALSE;

    memset(pImgAttrib, 0x0, sizeof(T_IMGDEC_ATTRIB));
    pImgAttrib->GifFrameInterval = 0;
	pImgAttrib->bIntervalChange = AK_FALSE;
    *pLargeFlag = 0;
    pImgAttrib->pInImgBuf = AK_NULL;
    pImgAttrib->pOutImgBuf = AK_NULL;
    pImgAttrib->GifHandle = 0;
    pImgAttrib->FileType = 0;

	// Open File
    fp = Fwl_FileOpen(pFilePath, _FMODE_READ, _FMODE_READ);
    if (_FOPEN_FAIL == fp)
        return AK_FALSE;

	// Get Image Type
    if (FILE_TYPE_MAP == Utl_GetFileType(pFilePath))
    {
        pImgAttrib->FileType = FILE_TYPE_MAP;
    }
    else
    {
        Fwl_FileRead(fp, typeInfo, IMG_TYPE_INFO_SIZE);
        Fwl_FileSeek(fp, 0, _FSEEK_SET);
        pImgAttrib->FileType = ImgDec_GetImgType(typeInfo);

        if (pImgAttrib->FileType == FILE_TYPE_NONE)
        {
            Fwl_FileClose(fp);
            return AK_FALSE;
        }
    }

    pImgAttrib->InImgSize = Fwl_GetFileLen(fp);

    if (FILE_TYPE_BMP == pImgAttrib->FileType)
    {
        T_U32 MaxBlckSize = 0;
        T_U32 MaxUseSize = 0;

        Fwl_FileRead(fp, pBmpHeadBuf, BMP_HEAD_SIZE);
        ret = ImgDec_DecBMPHead(pBmpHeadBuf, &BmpDeep, &BmpCompression, &ImgW, &ImgH);
        MaxBlckSize = Fwl_GetRemainRamSize();
        // after open the image, keep 2M memory for other module
        MaxUseSize = (MaxBlckSize >= LEAST_FREE_SIZE) ? MaxBlckSize - LEAST_FREE_SIZE : 0;

        if (ret && (BmpCompression == 0 || BmpCompression == 3))
        {
            if (pImgAttrib->InImgSize <= MaxUseSize)
            {
                Fwl_FileSeek(fp, 0, _FSEEK_SET);
            }
            else
            {
                Fwl_FileClose(fp);
                *pLargeFlag = AK_TRUE;
                return AK_FALSE;
            }
        }
        else
        {
            Fwl_FileClose(fp);
            return AK_FALSE;
        }
    }

    pImgAttrib->pInImgBuf = (T_U8 *)Fwl_Malloc(pImgAttrib->InImgSize + 16);
    if (AK_NULL == pImgAttrib->pInImgBuf)
    {
        Fwl_FileClose(fp);
        return AK_FALSE;
    }

    Fwl_FileRead(fp, pImgAttrib->pInImgBuf, pImgAttrib->InImgSize);
    Fwl_FileClose(fp);
    fp = _FOPEN_FAIL;

	switch (pImgAttrib->FileType)
	{
    case FILE_TYPE_BMP:
        pImgAttrib->pOutImgBuf = pImgAttrib->pInImgBuf;
        pImgAttrib->pInImgBuf = AK_NULL;
        ret = ImgDec_DecBMPHead(pImgAttrib->pOutImgBuf, &BmpDeep, &BmpCompression, &ImgW, &ImgH);
        pImgAttrib->InImgW = ImgW;
        pImgAttrib->InImgH = ImgH;        
        pImgAttrib->BmpCompression = BmpCompression;
        pImgAttrib->OutImgSize = pImgAttrib->InImgSize;
		
		if (ImgW > SUPPORT_MAX_IMG_SIZE 
			|| ImgH > SUPPORT_MAX_IMG_SIZE)
		{
			T_U8* bmpBuf;
			T_U16 dstW, dstH;
	
			Eng_GetAspectRatio(&dstW, &dstH, (T_U16)ImgW, (T_U16)ImgH, SUPPORT_MAX_IMG_SIZE);

			bmpBuf = Eng_BMPSubSample(pImgAttrib->pOutImgBuf, &pImgAttrib->OutImgSize, dstW, dstH);
			if (bmpBuf)
			{			
				Fwl_Free(pImgAttrib->pOutImgBuf);
				pImgAttrib->pOutImgBuf = bmpBuf;
				
				pImgAttrib->OutImgW = dstW;
	        	pImgAttrib->OutImgH = dstH;

				return AK_TRUE;
			}
		}
		else
		{
			pImgAttrib->OutImgW = ImgW;
        	pImgAttrib->OutImgH = ImgH;
		}
		
		break;
		
    case FILE_TYPE_JPG: 
	case FILE_TYPE_MAP: 
        ret = ImgDec_JpgDecOpen(pImgAttrib, autoShorten, pLargeFlag);
		break;
		
    case FILE_TYPE_PNG:
        ret = ImgDec_PngDecOpen(pImgAttrib);
 		break;
		
    case FILE_TYPE_GIF:
        ret = ImgDec_GifDecOpen(pImgAttrib);
 		break;

	default:
		break;
	}
	
    if (ret)
    {
        if (FILE_TYPE_BMP != pImgAttrib->FileType 
			&& FILE_TYPE_GIF != pImgAttrib->FileType
			&& AK_NULL != pImgAttrib->pInImgBuf)
            pImgAttrib->pInImgBuf = Fwl_Free(pImgAttrib->pInImgBuf);
    }
    else if(AK_NULL != pImgAttrib->pInImgBuf)
    {
        pImgAttrib->pInImgBuf = Fwl_Free(pImgAttrib->pInImgBuf);
    }
    return ret;
}

T_VOID ImgDec_ImgClose(T_IMGDEC_ATTRIB *pImgAttrib)
{
    if (FILE_TYPE_GIF == pImgAttrib->FileType && IMG_INVALID_HANDLE != pImgAttrib->GifHandle)
    {
        ImgDec_GifDecClose(pImgAttrib);
    }
    else if (FILE_TYPE_JPG == pImgAttrib->FileType || FILE_TYPE_MAP == pImgAttrib->FileType)
    {
        ImgDec_JpgDecClose(pImgAttrib);
    }
    else if (FILE_TYPE_PNG == pImgAttrib->FileType)
    {
        ImgDec_PngDecClose(pImgAttrib);
    }
    else if (FILE_TYPE_BMP == pImgAttrib->FileType)
    {
        if (AK_NULL != pImgAttrib->pOutImgBuf)
        {
            pImgAttrib->pOutImgBuf = Fwl_Free(pImgAttrib->pOutImgBuf);
        }
    }
    pImgAttrib->FileType = 0;
}

T_BOOL ImgDec_LargeImgOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_USTR_FILE pFilePath)
{
    T_pFILE fp = FS_INVALID_HANDLE;
    T_U8 typeInfo[IMG_TYPE_INFO_SIZE];

    if (AK_NULL == pLargeAttrib)
        return AK_FALSE;

    pLargeAttrib->ImgAttrib.pInImgBuf = AK_NULL;
    pLargeAttrib->ImgAttrib.pOutImgBuf = AK_NULL;
    pLargeAttrib->pOutLineBuf = AK_NULL;
    pLargeAttrib->pLargeBmpFp = _FOPEN_FAIL;
    pLargeAttrib->ImgAttrib.FileType = 0;
    pLargeAttrib->CurLines = 0;
    pLargeAttrib->CurY = 0;
    pLargeAttrib->TotalLines = 0;
    pLargeAttrib->DecStatus = DEC_ERROR;

    if (FILE_TYPE_MAP == Utl_GetFileType(pFilePath))
    {
        pLargeAttrib->ImgAttrib.FileType = FILE_TYPE_MAP;
    }
    else
    {
        fp = Fwl_FileOpen(pFilePath, _FMODE_READ, _FMODE_READ);
        Fwl_FileRead(fp, typeInfo, IMG_TYPE_INFO_SIZE);
        Fwl_FileClose(fp);
        pLargeAttrib->ImgAttrib.FileType = ImgDec_GetImgType(typeInfo);
        if (pLargeAttrib->ImgAttrib.FileType == FILE_TYPE_NONE)
            return AK_FALSE;
    }

    if (FILE_TYPE_JPG == pLargeAttrib->ImgAttrib.FileType)
    {
        return ImgDec_LargeJpgDecOpen(pLargeAttrib, pFilePath);
    }
    else if (FILE_TYPE_BMP == pLargeAttrib->ImgAttrib.FileType)
    {
        return ImgDec_LargeBmpDecOpen(pLargeAttrib, pFilePath, LARGE_DEC_BMP_DST_WIDTH_MAX, LARGE_DEC_BMP_DST_HEIGHT_MAX);
    }

    return AK_FALSE;
}

T_VOID ImgDec_LargeImgClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    if (FILE_TYPE_BMP == pLargeAttrib->ImgAttrib.FileType)
    {
        ImgDec_LargeBmpDecClose(pLargeAttrib);
    }
    else if (FILE_TYPE_JPG == pLargeAttrib->ImgAttrib.FileType)
    {
        ImgDec_LargeJpgDecClose(pLargeAttrib);
    }
    pLargeAttrib->ImgAttrib.FileType = 0;
    pLargeAttrib->DecStatus = DEC_ERROR;
}

T_LARGE_IMG_STATUS ImgDec_LargeImgStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    if (AK_NULL == pLargeAttrib)
        return DEC_ERROR;

    if (FILE_TYPE_JPG == pLargeAttrib->ImgAttrib.FileType)
    {
        return ImgDec_LargeJpgDecStep(pLargeAttrib);
    }
    else if (FILE_TYPE_BMP == pLargeAttrib->ImgAttrib.FileType)
    {
        return ImgDec_LargeBmpDecStep(pLargeAttrib);
    }

    return DEC_ERROR;
}

T_LARGE_IMG_STATUS ImgDec_GetLargeDecStatus(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib)
{
    if (AK_NULL != pLargeAttrib)
    {
        return pLargeAttrib->DecStatus;
    }

    return DEC_ERROR;
}


T_FILE_TYPE ImgDec_GetFileType(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    T_FILE_TYPE FileType = 0;

    if (AK_NULL != pImgDecAttrib)
    {
        FileType = pImgDecAttrib->FileType;
    }

    return FileType;

}

T_pDATA ImgDec_GetOutBuf(T_IMGDEC_ATTRIB *pImgDecAttrib)
{
    T_pDATA pBuf = AK_NULL;

    if (AK_NULL != pImgDecAttrib && AK_NULL != pImgDecAttrib->pOutImgBuf)
    {
        pBuf = pImgDecAttrib->pOutImgBuf;
    }

    return pBuf;
}

static T_pVOID ImgDec_MemMalloc(T_U32 size, char *filename, T_U32 line)
{
    return (T_pVOID)Fwl_Malloc(size);
}

static T_pVOID ImgDec_MemFree(T_pVOID ptr)
{
    return Fwl_Free(ptr);
}

#ifdef OS_ANYKA
T_VOID MMU_InvalidateIDCache(T_VOID);
#endif



static T_VOID Cache_Flush(T_VOID)
{
    MMU_Clean_Invalidate_Dcache();
}

static T_VOID ImgDec_Wait(T_VOID)
{
#ifdef OS_ANYKA
    AK_Sleep(1);  
#endif
}

static T_S32 ImgDec_Print(T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    va_start(args, s);
    len = Fwl_VPrint(C4, M_IMG_LIB, s, args);
    va_end(args); 
    
    return len;
}


T_VOID ImgDec_SetImgLibCallBack(T_VOID)
{
    CB_FUNS Cb_Funs;


#ifdef CI37XX_PLATFORM
	SetImageChipID(AK37xC_JPEG_VER);
#endif
    // set image library call back function
    memset(&Cb_Funs, 0, sizeof(CB_FUNS));
    Cb_Funs.malloc = ImgDec_MemMalloc;
    Cb_Funs.free = (CALLBACK_FUN_FREE)ImgDec_MemFree;
    Cb_Funs.printf = (CALLBACK_FUN_PRT_S32F)ImgDec_Print;
    Cb_Funs.regModify = (CALLBACK_FUN_REGMODIFY)RegBitsWriteCB;
    Img_SetCallbackFuns(&Cb_Funs);


#ifdef OS_ANYKA
    Img_SetFlushCacheFunc((FlushCacheFunc)Cache_Flush);
	Img_SetJPEGTaskFunc(ImgDec_Wait);
#endif
}


T_VOID Img_SetJpgTask_CB(JPEGTaskFunc func)
{
    if (AK_NULL == func)
    {
        Img_SetJPEGTaskFunc(ImgDec_Wait);
    }

    else
    {
        Img_SetJPEGTaskFunc(func);
    }
}

//end of file

