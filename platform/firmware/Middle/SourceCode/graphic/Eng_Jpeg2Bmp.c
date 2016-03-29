/**
 * @file jpeg2bmp.c
 * @brief Convert jpeg image to BMP image or RGB data.
 * reorganized by ZouMai in 2004
 * Copyright (C) 2001 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author WangYanFei
 * @date 2001-08-01
 * @version 1.0
 * @ref
 */

#include "Fwl_public.h"
#include "Eng_Jpeg2Bmp.h"

/* image type */
#define IMAGE_TYPE_UNKNOWN  0
#define IMAGE_TYPE_BMP      1
#define IMAGE_TYPE_WBMP     2
#define IMAGE_TYPE_JPG      3
#define IMAGE_TYPE_GIF      4
#define IMAGE_TYPE_PNG      5

#define JPEG_M_DRI          0xdd
#define JPEG_M_APP0         0xe0
#define JPEG_M_DQT          0xdb
#define JPEG_M_SOF0         0xc0
#define JPEG_M_DHT          0xc4
#define JPEG_M_SOS          0xda
#define JPEG_M_EOI          0xd9

#define JPEG_W1             2841     // = 2048*sqrt(2)*cos(1*pi/16)
#define JPEG_W2             2676     // = 2048*sqrt(2)*cos(2*pi/16)
#define JPEG_W3             2408     // = 2048*sqrt(2)*cos(3*pi/16)
#define JPEG_W5             1609     // = 2048*sqrt(2)*cos(5*pi/16)
#define JPEG_W6             1108     // = 2048*sqrt(2)*cos(6*pi/16)
#define JPEG_W7             565      // = 2048*sqrt(2)*cos(7*pi/16)

#define JPEG_WIDTHBYTES(i)  ((i+31)/32*4)
#define JPEG_MAKEWORD(a, b) ((T_U16)(((T_U8)(a)) | ((T_U16)((T_U8)(b))) << 8))


typedef struct {
    T_U32	LineBytes;
	T_U32	ImgSize;
	T_U16	ImgWidth;
	T_U16	ImgHeight;
	T_U16	DstWidth;
	T_U16	DstHeight;
	T_S16	SampRate_Y_H;
	T_S16	SampRate_Y_V;
	T_S16	SampRate_U_H;
	T_S16	SampRate_U_V;
	T_S16	SampRate_V_H;
	T_S16	SampRate_V_V;
	T_S16	H_YtoU;
	T_S16	V_YtoU;
	T_S16	H_YtoV;
	T_S16	V_YtoV;
	T_S16	Y_in_MCU;
	T_S16	U_in_MCU;
	T_S16	V_in_MCU;
	T_S16	qt_table[3][64];
	T_S16	comp_num;
	T_U8	comp_index[3];
	T_U8	YDcIndex;
	T_U8	YAcIndex;
	T_U8	UVDcIndex;
	T_U8	UVAcIndex;
	T_U8	HufTabIndex;
	T_S16	*YQtTable;
	T_S16	*UQtTable;
	T_S16	*VQtTable;
	T_S16	code_pos_table[4][16];
	T_S16	code_len_table[4][16];
	T_U16	code_value_table[4][256];
	T_U16	huf_max_value[4][16];
	T_U16	huf_min_value[4][16];
	T_S16	BitPos;
	T_S16	CurByte;
	T_S16	rrun;
	T_S16	vvalue;
	T_S16	MCUBuffer[10*64];
	T_S32	QtZzMCUBuffer[10*64];
	T_S16	BlockBuffer[64];
	T_S16	ycoef;
	T_S16	ucoef;
	T_S16	vcoef;
	T_S32	IntervalFlag;
	T_U32	interval;
	T_S32	Y[4*64];
	T_S32	U[4*64];
	T_S32	V[4*64];
	T_U32	sizei;
	T_U32	sizej;
	T_S16	restart;
	T_S32	iclip[1024];
	T_S32	*iclp;
	T_U8	*lpJpegBuf;
	T_U8	*lpCur;
	T_S32	qq_flag;
	T_U8	dest_type;
}T_JPEG2BMP;

static T_JPEG2BMP *pJpeg2Bmp = AK_NULL;


const static T_U8   And[9] =
{
    0,1,3,7,0xf,0x1f,0x3f,0x7f,0xff
};

const static T_S16  Zig_Zag_tbl[8][8] =
{
    {0,1,5,6,14,15,27,28},
    {2,4,7,13,16,26,29,42},
    {3,8,12,17,25,30,41,43},
    {9,11,18,24,37,40,44,53},
    {10,19,23,32,39,45,52,54},
    {20,22,33,38,46,51,55,60},
    {21,34,37,47,50,56,59,61},
    {35,36,48,49,57,58,62,63}
};

// static function global variable declaration
static T_U8     GetImgType(T_U8 *imgbuf);
static T_S32    InitTag(T_VOID);
static T_VOID   InitTable(T_VOID);
static T_S32    Jpeg2RGB(T_U8 *RgbData, T_U8 dstType);
static T_VOID   GetYUV(T_S16 flag);
static T_VOID   StoreBuffer(T_U8 *RgbData, T_U8 dstType);
static T_S32    DecodeMCUBlock(T_VOID);
static T_S32    HufBlock(T_U8 dchufindex,T_U8 achufindex);
static T_S32    DecodeAElement(T_VOID);
static T_VOID   IQtIZzMCUComponent(T_S16 flag);
static T_VOID   IQtIZzBlock(T_S16  *s ,T_S32 * d,T_S16 flag);
static T_VOID   Fast_IDCT(T_S32 * block);
static T_U8     ReadByte(T_VOID);
static T_VOID   Initialize_Fast_IDCT(T_VOID);
static T_VOID   idctrow(T_S32 * blk);
static T_VOID   idctcol(T_S32 * blk);
static T_VOID   JpgMemCpy(T_U8 *strDest, T_U8 *strSour, T_U16 count);
static T_VOID   JpgMemSet(T_U8 *strDest, T_U8 value, T_U16 count);

/**
 * @brief initialize jpeg for decoding
 * User must call this function before call function JpegSetDstSize() or JpegDecode()
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *JpgData: the jpeg data
 * @param T_U8 dstType: destination type, must be JPEG_TO_BMP, JPEG_TO_RGB or JPEG_TO_BGR
 * @param T_U16 *Width: image width for return
 * @param T_U16 *Height: image height for return
 * @return T_S32
 * @retval > 0: means success, and the returned value is the buffer size user should allocate
 * @retval < 0: error code
 */
T_S32 JpegInit(T_U8 *JpgData, T_U8 dstType, T_U16 *Width, T_U16 *Height)
{
    T_S32       funcret;

    if (JpgData == AK_NULL)
        return JPEG_PARM_ERROR;

    if (GetImgType(JpgData) != IMAGE_TYPE_JPG)
    {
        return JPEG_DATA_ERROR;
    }

	pJpeg2Bmp = (T_JPEG2BMP *)Fwl_Malloc(sizeof(T_JPEG2BMP));
    AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);
	memset(pJpeg2Bmp, 0, sizeof(T_JPEG2BMP));

    pJpeg2Bmp->lpJpegBuf = JpgData;
    InitTable();

	funcret = InitTag();
	
    if (funcret != JPEG_OK)
    {
        pJpeg2Bmp->lpJpegBuf = AK_NULL;
        return JPEG_INIT_ERROR;
    }
    if (pJpeg2Bmp->SampRate_Y_H == 0 || pJpeg2Bmp->SampRate_Y_V == 0)
    {
        pJpeg2Bmp->lpJpegBuf = AK_NULL;
        return JPEG_SAMPLE_ERROR;
    }

    if (Width != AK_NULL)
    {
        *Width = pJpeg2Bmp->ImgWidth;
    }
    if (Height != AK_NULL)
    {
        *Height = pJpeg2Bmp->ImgHeight;
    }

    pJpeg2Bmp->DstWidth = pJpeg2Bmp->ImgWidth;
    pJpeg2Bmp->DstHeight = pJpeg2Bmp->ImgHeight;

    pJpeg2Bmp->LineBytes=(T_U32)JPEG_WIDTHBYTES(pJpeg2Bmp->ImgWidth*24);
    pJpeg2Bmp->ImgSize=(T_U32)pJpeg2Bmp->LineBytes*pJpeg2Bmp->ImgHeight;
    pJpeg2Bmp->dest_type = dstType;
    pJpeg2Bmp->qq_flag = 0;

    if (pJpeg2Bmp->dest_type == JPEG_TO_BMP)
    {
#ifdef OS_ANYKA
        return sizeof(BITMAPFILEHEADER)-2+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
#endif
#ifdef OS_WIN32
        return sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
#endif
    }
    else if (pJpeg2Bmp->dest_type == JPEG_TO_RGB || pJpeg2Bmp->dest_type == JPEG_TO_BGR || pJpeg2Bmp->dest_type == JPEG_TO_RGBLINE)
    {
        return pJpeg2Bmp->ImgSize;
    }
    else
    {
        return JPEG_DST_TYPE_ERROR;
    }
}

/**
 * @brief free the handle
 * @author songmengxing
 * @date 2011-10-25
 * @param void
 * @return void
 */
T_VOID JpegFree(T_VOID)
{
	if (AK_NULL != pJpeg2Bmp)
	{
		pJpeg2Bmp = Fwl_Free(pJpeg2Bmp);
	}
}


/**
 * @brief reset destination BMP or RGB image size
 * If user want not to chage image size, user needn't to call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U16 dstWidth: new destination image width
 * @param T_U16 dstHeight: new destination image height
 * @return T_S32
 * @retval > 0: means success, and the returned value is the buffer size user should allocate
 * @retval < 0: error code
 */
T_S32 JpegSetDstSize(T_U16 dstWidth, T_U16 dstHeight)
{
	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);
	
    if (pJpeg2Bmp->lpJpegBuf == AK_NULL)
        return JPEG_NOT_INIT;

    if (dstWidth != pJpeg2Bmp->ImgWidth || dstHeight != pJpeg2Bmp->ImgHeight)
    {
        if (pJpeg2Bmp->ImgWidth<dstWidth)
            dstWidth = pJpeg2Bmp->ImgWidth;

        if (pJpeg2Bmp->ImgHeight<dstHeight)
            dstHeight = pJpeg2Bmp->ImgHeight;

        pJpeg2Bmp->DstWidth = dstWidth;
        pJpeg2Bmp->DstHeight = dstHeight;

        pJpeg2Bmp->LineBytes=(T_U32)JPEG_WIDTHBYTES(dstWidth*24);
        pJpeg2Bmp->ImgSize=(T_U32)pJpeg2Bmp->LineBytes*dstHeight;
        pJpeg2Bmp->qq_flag = 2;
    }

    if (pJpeg2Bmp->dest_type == JPEG_TO_BMP)
    {
#ifdef OS_ANYKA
        return sizeof(BITMAPFILEHEADER)-2+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
#endif
#ifdef OS_WIN32
        return sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
#endif
    }
    else if (pJpeg2Bmp->dest_type == JPEG_TO_RGB || pJpeg2Bmp->dest_type == JPEG_TO_BGR || pJpeg2Bmp->dest_type == JPEG_TO_RGBLINE)
    {
        return pJpeg2Bmp->ImgSize;
    }
    else
    {
        return JPEG_DST_TYPE_ERROR;
    }
}

/**
 * @brief convert JPEG data to BMP or RGB data
 * User must allocate memory for dstBuf before call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *dstBuf: buffer for storage destination image(BMP or RGB)
 * @return T_S32
 * @retval > 0: means success, and the returned value is the image size
 * @retval < 0: error code
 */
T_S32 JpegDecode(T_U8 *dstBuf)
{
    T_S32               funcret;
    BITMAPFILEHEADER    bf;
    BITMAPINFOHEADER    bi;
    T_U16               y;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    if (dstBuf == AK_NULL)
        return JPEG_PARM_ERROR;
    if (pJpeg2Bmp->lpJpegBuf == AK_NULL)
        return JPEG_NOT_INIT;

    if (pJpeg2Bmp->dest_type != JPEG_TO_BMP)
    {
        funcret = Jpeg2RGB(dstBuf, pJpeg2Bmp->dest_type);
        
        for (y=0; y<pJpeg2Bmp->ImgHeight; y++)
        {
            JpgMemCpy((T_U8 *)&dstBuf[y*pJpeg2Bmp->ImgWidth*3], \
                    (T_U8 *)&dstBuf[y*(((pJpeg2Bmp->ImgWidth*24+31)>>5)<<2)], \
                    (T_U16)(3*pJpeg2Bmp->ImgWidth));
        }

        return funcret;
    }

    //create new bitmapfileheader and bitmapinfoheader
    JpgMemSet((T_S8 *)&bf,0,sizeof(BITMAPFILEHEADER));  
    JpgMemSet((T_S8 *)&bi,0,sizeof(BITMAPINFOHEADER));

    bi.biSize=(T_U32)sizeof(BITMAPINFOHEADER);
    bi.biWidth=(T_S32)(pJpeg2Bmp->DstWidth);
    bi.biHeight=(T_S32)(pJpeg2Bmp->DstHeight);
    bi.biPlanes=1;
    bi.biBitCount=24;
    bi.biClrUsed=0;
    bi.biClrImportant=0;
    bi.biCompression=0;

    bf.bfType=0x4d42;

#ifdef OS_ANYKA
    bf.bfSize=sizeof(BITMAPFILEHEADER)-2+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
    bf.bfOffBits=(T_U32)(sizeof(BITMAPFILEHEADER)-2+sizeof(BITMAPINFOHEADER));
    JpgMemCpy(dstBuf, (T_U8 *)&bf.bfType, sizeof(bf.bfType));
    JpgMemCpy(dstBuf+2, (T_U8 *)&bf.bfSize, sizeof(BITMAPFILEHEADER)-4);
    JpgMemCpy(dstBuf+sizeof(BITMAPFILEHEADER)-2, (T_U8 *)&bi, sizeof(BITMAPINFOHEADER));
    funcret = Jpeg2RGB(dstBuf+sizeof(BITMAPFILEHEADER)-2+sizeof(BITMAPINFOHEADER), JPEG_TO_RGB);
#endif
#ifdef OS_WIN32
    bf.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+pJpeg2Bmp->ImgSize;
    bf.bfOffBits=(T_U32)(sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));
    JpgMemCpy(dstBuf, (T_U8 *)&bf, sizeof(BITMAPFILEHEADER));
    JpgMemCpy(dstBuf+sizeof(BITMAPFILEHEADER), (T_U8 *)&bi, sizeof(BITMAPINFOHEADER));
    funcret = Jpeg2RGB(dstBuf+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER), JPEG_TO_RGB);
#endif

    if (funcret <= 0)
    {
        return funcret;
    }
    else
    {
        return bf.bfSize;
    }
}

/**
 * @brief decode a block line from JPEG data
 * User must allocate memory for dstBuf before call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *dstBuf: buffer for storage destination image(BMP or RGB)
 * @return T_S32
 * @retval > 0: means success, and the returned number of decoded lines 
 * @retval < 0: error code
 */
T_S32 JpegDecodeLine(T_U8 *dstBuf)
{
    T_S32               funcret;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    if (dstBuf == AK_NULL)
        return JPEG_PARM_ERROR;
    if (pJpeg2Bmp->lpJpegBuf == AK_NULL)
        return JPEG_NOT_INIT;

    if (pJpeg2Bmp->dest_type != JPEG_TO_RGBLINE)
        return JPEG_DST_TYPE_ERROR;

//    FreqMgr_StateCheckIn(FREQ_MOST_NEEDED, FREQ_PRIOR_NULL);
    pJpeg2Bmp->Y_in_MCU=pJpeg2Bmp->SampRate_Y_H*pJpeg2Bmp->SampRate_Y_V;
    pJpeg2Bmp->U_in_MCU=pJpeg2Bmp->SampRate_U_H*pJpeg2Bmp->SampRate_U_V;
    pJpeg2Bmp->V_in_MCU=pJpeg2Bmp->SampRate_V_H*pJpeg2Bmp->SampRate_V_V;
	pJpeg2Bmp->H_YtoU=pJpeg2Bmp->SampRate_Y_H/pJpeg2Bmp->SampRate_U_H;
    pJpeg2Bmp->V_YtoU=pJpeg2Bmp->SampRate_Y_V/pJpeg2Bmp->SampRate_U_V;
    pJpeg2Bmp->H_YtoV=pJpeg2Bmp->SampRate_Y_H/pJpeg2Bmp->SampRate_V_H;
    pJpeg2Bmp->V_YtoV=pJpeg2Bmp->SampRate_Y_V/pJpeg2Bmp->SampRate_V_V;
    Initialize_Fast_IDCT();
    while((funcret=DecodeMCUBlock())==JPEG_OK)
    {
        pJpeg2Bmp->interval++;
        if((pJpeg2Bmp->restart)&&(pJpeg2Bmp->interval % pJpeg2Bmp->restart==0))
             pJpeg2Bmp->IntervalFlag=1;
        else
            pJpeg2Bmp->IntervalFlag=0;
        IQtIZzMCUComponent(0);
        IQtIZzMCUComponent(1);
        IQtIZzMCUComponent(2);
        GetYUV(0);
        GetYUV(1);
        GetYUV(2);
        StoreBuffer(dstBuf, pJpeg2Bmp->dest_type);
        pJpeg2Bmp->sizej+=pJpeg2Bmp->SampRate_Y_H*8;
        if(pJpeg2Bmp->sizej>=pJpeg2Bmp->ImgWidth)
        {
            pJpeg2Bmp->sizej=0;
            pJpeg2Bmp->sizei+=pJpeg2Bmp->SampRate_Y_V*8;
            break;
        }
        if ((pJpeg2Bmp->sizej==0)&&(pJpeg2Bmp->sizei>=pJpeg2Bmp->ImgHeight))
            break;
    }

//    FreqMgr_StateCheckOut(FREQ_MOST_NEEDED);
    if (funcret==JPEG_OK)
    {
        if(pJpeg2Bmp->sizei>pJpeg2Bmp->ImgHeight)
            return pJpeg2Bmp->SampRate_Y_V*8 - pJpeg2Bmp->sizei + pJpeg2Bmp->ImgHeight;
        else
            return pJpeg2Bmp->SampRate_Y_V*8;
    }
    else
    {
        return funcret;
    }
}

/**
 * @brief judge the image type
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *imgbuf: iamge binary code
 * @return T_U8: image type
 * @retval
 */
static T_U8 GetImgType(T_U8 *imgbuf)
{
    T_U8 type = IMAGE_TYPE_UNKNOWN;
    T_U8 *mms = AK_NULL;

    mms = imgbuf;
    if (((mms[0] & 0xFF) == 0x42) && ((mms[1] & 0xFF) == 0x4D))
    {
        type = IMAGE_TYPE_BMP;
    }
    else if (((mms[0] & 0xFF) == 0x00) && ((mms[1] & 0xFF) == 0x00))
    {
        type = IMAGE_TYPE_WBMP;
    }
    else if (((mms[0] & 0xFF) == 0xFF) && ((mms[1] & 0xFF) == 0xD8))
    {
        type = IMAGE_TYPE_JPG;
    }
    else if (((mms[0] & 0xFF) == 0x47) && ((mms[1] & 0xFF) == 0x49))
    {
        type = IMAGE_TYPE_GIF;
    }
    else if (((mms[0] & 0xFF) == 0x89) && ((mms[1] & 0xFF) == 0x50)
        && ((mms[2] & 0xFF) == 0x4E) && ((mms[3] & 0xFF) == 0x47))
    {
        type = IMAGE_TYPE_PNG;
    }

    return type;
}

////////////////////////////////////////////////////////////////////////////////
static T_S32 InitTag(T_VOID)
{
    T_S32   finish=0;
    T_U8    id;
    T_S32   llength;
    T_S16   i,j,k;
    T_S16   huftab1,huftab2;
    T_S16   huftabindex;
    T_U8    qt_table_index;
    T_U8    comnum;
    T_U8    *lptemp;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    pJpeg2Bmp->lpCur = pJpeg2Bmp->lpJpegBuf + 2;

    while (!finish)
    {
        if(*pJpeg2Bmp->lpCur!=0x0ff)
            return JPEG_FORMAT_ERROR;
        id=*(pJpeg2Bmp->lpCur+1);
        pJpeg2Bmp->lpCur+=2;
        switch (id)
        {
        case JPEG_M_APP0:
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            pJpeg2Bmp->lpCur+=llength;
            break;
        case JPEG_M_DQT:
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            qt_table_index=(*(pJpeg2Bmp->lpCur+2))&0x0f;
            lptemp=pJpeg2Bmp->lpCur+3;
            if(llength<80)
            {
                for(i=0;i<64;i++)
                    pJpeg2Bmp->qt_table[qt_table_index][i]=(T_S16)*(lptemp++);
            }
            else
            {
                for(i=0;i<64;i++)
                    pJpeg2Bmp->qt_table[qt_table_index][i]=(T_S16)*(lptemp++);
                qt_table_index=(*(lptemp++))&0x0f;
                for(i=0;i<64;i++)
                    pJpeg2Bmp->qt_table[qt_table_index][i]=(T_S16)*(lptemp++);
            }
            pJpeg2Bmp->lpCur+=llength;     
            break;
        case JPEG_M_SOF0:
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            pJpeg2Bmp->ImgHeight=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+4),*(pJpeg2Bmp->lpCur+3));
            pJpeg2Bmp->ImgWidth=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+6),*(pJpeg2Bmp->lpCur+5));
            pJpeg2Bmp->comp_num=*(pJpeg2Bmp->lpCur+7);
            if((pJpeg2Bmp->comp_num!=1)&&(pJpeg2Bmp->comp_num!=3))
                return JPEG_FORMAT_ERROR;
            if(pJpeg2Bmp->comp_num==3)
            {
                pJpeg2Bmp->comp_index[0]=*(pJpeg2Bmp->lpCur+8);
                pJpeg2Bmp->SampRate_Y_H=(*(pJpeg2Bmp->lpCur+9))>>4;
                pJpeg2Bmp->SampRate_Y_V=(*(pJpeg2Bmp->lpCur+9))&0x0f;
                pJpeg2Bmp->YQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+10)];

                pJpeg2Bmp->comp_index[1]=*(pJpeg2Bmp->lpCur+11);
                pJpeg2Bmp->SampRate_U_H=(*(pJpeg2Bmp->lpCur+12))>>4;
                pJpeg2Bmp->SampRate_U_V=(*(pJpeg2Bmp->lpCur+12))&0x0f;
                pJpeg2Bmp->UQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+13)];

                pJpeg2Bmp->comp_index[2]=*(pJpeg2Bmp->lpCur+14);
                pJpeg2Bmp->SampRate_V_H=(*(pJpeg2Bmp->lpCur+15))>>4;
                pJpeg2Bmp->SampRate_V_V=(*(pJpeg2Bmp->lpCur+15))&0x0f;
                pJpeg2Bmp->VQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+16)];
            }
            else
            {
                pJpeg2Bmp->comp_index[0]=*(pJpeg2Bmp->lpCur+8);
                pJpeg2Bmp->SampRate_Y_H=(*(pJpeg2Bmp->lpCur+9))>>4;
                pJpeg2Bmp->SampRate_Y_V=(*(pJpeg2Bmp->lpCur+9))&0x0f;
                pJpeg2Bmp->YQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+10)];

                pJpeg2Bmp->comp_index[1]=*(pJpeg2Bmp->lpCur+8);
                pJpeg2Bmp->SampRate_U_H=1;
                pJpeg2Bmp->SampRate_U_V=1;
                pJpeg2Bmp->UQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+10)];

                pJpeg2Bmp->comp_index[2]=*(pJpeg2Bmp->lpCur+8);
                pJpeg2Bmp->SampRate_V_H=1;
                pJpeg2Bmp->SampRate_V_V=1;
                pJpeg2Bmp->VQtTable=(T_S16 *)pJpeg2Bmp->qt_table[*(pJpeg2Bmp->lpCur+10)];
            }
            pJpeg2Bmp->lpCur+=llength;                         
            break;
        case JPEG_M_DHT:             
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            lptemp = pJpeg2Bmp->lpCur+2;
            while((lptemp-pJpeg2Bmp->lpCur)<llength)
            {
                huftab1=(T_S16)(*(lptemp))>>4;     //huftab1=0,1
                huftab2=(T_S16)(*(lptemp))&0x0f;   //huftab2=0,1
                huftabindex=huftab1*2+huftab2;
                lptemp++;
                for (i=0; i<16; i++)
                    pJpeg2Bmp->code_len_table[huftabindex][i]=(T_S16)(*(lptemp++));
                j=0;
                for (i=0; i<16; i++)
                {
                    if(pJpeg2Bmp->code_len_table[huftabindex][i]!=0)
                    {
                        k=0;
                        while(k<pJpeg2Bmp->code_len_table[huftabindex][i])
                        {
                            pJpeg2Bmp->code_value_table[huftabindex][k+j]=(T_S16)(*(lptemp++));
                            k++;
                        }
                        j+=k;   
                    }
                }
                i=0;
                while (pJpeg2Bmp->code_len_table[huftabindex][i]==0)
                    i++;
                for (j=0;j<i;j++)
                {
                    pJpeg2Bmp->huf_min_value[huftabindex][j]=0;
                    pJpeg2Bmp->huf_max_value[huftabindex][j]=0;
                }
                pJpeg2Bmp->huf_min_value[huftabindex][i]=0;
                pJpeg2Bmp->huf_max_value[huftabindex][i]=pJpeg2Bmp->code_len_table[huftabindex][i]-1;
                for (j=i+1;j<16;j++)
                {
                    pJpeg2Bmp->huf_min_value[huftabindex][j]=(pJpeg2Bmp->huf_max_value[huftabindex][j-1]+1)<<1;
                    pJpeg2Bmp->huf_max_value[huftabindex][j]=pJpeg2Bmp->huf_min_value[huftabindex][j]+pJpeg2Bmp->code_len_table[huftabindex][j]-1;
                }
                pJpeg2Bmp->code_pos_table[huftabindex][0]=0;
                for (j=1;j<16;j++)
                    pJpeg2Bmp->code_pos_table[huftabindex][j]=pJpeg2Bmp->code_len_table[huftabindex][j-1]+pJpeg2Bmp->code_pos_table[huftabindex][j-1];
            }  //if
            pJpeg2Bmp->lpCur+=llength;
            break;
        case JPEG_M_DRI:
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            pJpeg2Bmp->restart=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+3),*(pJpeg2Bmp->lpCur+2));
            pJpeg2Bmp->lpCur+=llength;
            break;
        case JPEG_M_SOS:
            llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
            comnum=*(pJpeg2Bmp->lpCur+2);
            if(comnum!=pJpeg2Bmp->comp_num)
                return JPEG_FORMAT_ERROR;
            lptemp=pJpeg2Bmp->lpCur+3;
            for (i=0;i<pJpeg2Bmp->comp_num;i++)
            {
                if(*lptemp==pJpeg2Bmp->comp_index[0])
                {
                    pJpeg2Bmp->YDcIndex=(*(lptemp+1))>>4;   //Y
                    pJpeg2Bmp->YAcIndex=((*(lptemp+1))&0x0f)+2;
                }
                else
                {
                    pJpeg2Bmp->UVDcIndex=(*(lptemp+1))>>4;   //U,V
                    pJpeg2Bmp->UVAcIndex=((*(lptemp+1))&0x0f)+2;
                }
                lptemp+=2;
            }
            pJpeg2Bmp->lpCur+=llength;
            finish=1;
            break;
        case JPEG_M_EOI:    
            return JPEG_FORMAT_ERROR;
            //break;
        default:
            if ((id&0xf0)!=0xd0)
            {
                llength=JPEG_MAKEWORD(*(pJpeg2Bmp->lpCur+1),*pJpeg2Bmp->lpCur);
                pJpeg2Bmp->lpCur+=llength;
            }
            else pJpeg2Bmp->lpCur+=2;
            break;
        }  //switch
    } //while
    return JPEG_OK;
}

/////////////////////////////////////////////////////////////////
static T_VOID InitTable(T_VOID)
{
    T_S16 i,j;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");

    pJpeg2Bmp->sizei=pJpeg2Bmp->sizej=0;
    pJpeg2Bmp->ImgWidth=pJpeg2Bmp->ImgHeight=0;
    pJpeg2Bmp->rrun=pJpeg2Bmp->vvalue=0;
    pJpeg2Bmp->BitPos=0;
    pJpeg2Bmp->CurByte=0;
    pJpeg2Bmp->IntervalFlag=0;
	pJpeg2Bmp->interval = 0;
    pJpeg2Bmp->restart=0;
    for(i=0;i<3;i++)
        for(j=0;j<64;j++)
            pJpeg2Bmp->qt_table[i][j]=0;
    pJpeg2Bmp->comp_num=0;
    pJpeg2Bmp->HufTabIndex=0;
    for(i=0;i<3;i++)
        pJpeg2Bmp->comp_index[i]=0;
    for(i=0;i<4;i++)
        for(j=0;j<16;j++)
        {
            pJpeg2Bmp->code_len_table[i][j]=0;
            pJpeg2Bmp->code_pos_table[i][j]=0;
            pJpeg2Bmp->huf_max_value[i][j]=0;
            pJpeg2Bmp->huf_min_value[i][j]=0;
        }
    for(i=0;i<4;i++)
        for(j=0;j<256;j++)
            pJpeg2Bmp->code_value_table[i][j]=0;
    
    for(i=0;i<10*64;i++)
    {
        pJpeg2Bmp->MCUBuffer[i]=0;
        pJpeg2Bmp->QtZzMCUBuffer[i]=0;
    }
    for(i=0;i<64;i++)
    {
        pJpeg2Bmp->Y[i]=0;
        pJpeg2Bmp->U[i]=0;
        pJpeg2Bmp->V[i]=0;
        pJpeg2Bmp->BlockBuffer[i]=0;
    }
    pJpeg2Bmp->ycoef=pJpeg2Bmp->ucoef=pJpeg2Bmp->vcoef=0;
}

/////////////////////// Decode ///////////////////////////////////
static T_S32 Jpeg2RGB(T_U8 *RgbData, T_U8 dstType)
{
    T_S32 funcret;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    if (dstType != JPEG_TO_RGB && dstType != JPEG_TO_BGR)
        return JPEG_DST_TYPE_ERROR;

//    FreqMgr_StateCheckIn(FREQ_MOST_NEEDED, FREQ_PRIOR_NULL);
    pJpeg2Bmp->Y_in_MCU=pJpeg2Bmp->SampRate_Y_H*pJpeg2Bmp->SampRate_Y_V;
    pJpeg2Bmp->U_in_MCU=pJpeg2Bmp->SampRate_U_H*pJpeg2Bmp->SampRate_U_V;
    pJpeg2Bmp->V_in_MCU=pJpeg2Bmp->SampRate_V_H*pJpeg2Bmp->SampRate_V_V;
    pJpeg2Bmp->H_YtoU=pJpeg2Bmp->SampRate_Y_H/pJpeg2Bmp->SampRate_U_H;
    pJpeg2Bmp->V_YtoU=pJpeg2Bmp->SampRate_Y_V/pJpeg2Bmp->SampRate_U_V;
    pJpeg2Bmp->H_YtoV=pJpeg2Bmp->SampRate_Y_H/pJpeg2Bmp->SampRate_V_H;
    pJpeg2Bmp->V_YtoV=pJpeg2Bmp->SampRate_Y_V/pJpeg2Bmp->SampRate_V_V;
    Initialize_Fast_IDCT();
    while((funcret=DecodeMCUBlock())==JPEG_OK)
    {
        pJpeg2Bmp->interval++;
        if((pJpeg2Bmp->restart)&&(pJpeg2Bmp->interval % pJpeg2Bmp->restart==0))
             pJpeg2Bmp->IntervalFlag=1;
        else
            pJpeg2Bmp->IntervalFlag=0;
        IQtIZzMCUComponent(0);
        IQtIZzMCUComponent(1);
        IQtIZzMCUComponent(2);
        GetYUV(0);
        GetYUV(1);
        GetYUV(2);
        StoreBuffer(RgbData, dstType);
        pJpeg2Bmp->sizej+=pJpeg2Bmp->SampRate_Y_H*8;
        if(pJpeg2Bmp->sizej>=pJpeg2Bmp->ImgWidth)
        {
            pJpeg2Bmp->sizej=0;
            pJpeg2Bmp->sizei+=pJpeg2Bmp->SampRate_Y_V*8;
        }
        if ((pJpeg2Bmp->sizej==0)&&(pJpeg2Bmp->sizei>=pJpeg2Bmp->ImgHeight))
            break;
    }

//    FreqMgr_StateCheckOut(FREQ_MOST_NEEDED);
    if (funcret==JPEG_OK)
    {
        return pJpeg2Bmp->ImgSize;
    }
    else
    {
        return funcret;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
static T_VOID  GetYUV(T_S16 flag)
{
    T_S16   H=0,VV=0;
    T_S16   i,j,k,h;
    T_S32       *buf=0;
    T_S32       *pQtZzMCU=0;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");
	
    switch(flag)
    {
    case 0:
        H=pJpeg2Bmp->SampRate_Y_H;
        VV=pJpeg2Bmp->SampRate_Y_V;
        buf=pJpeg2Bmp->Y;
        pQtZzMCU=pJpeg2Bmp->QtZzMCUBuffer;
        break;
    case 1:
        H=pJpeg2Bmp->SampRate_U_H;
        VV=pJpeg2Bmp->SampRate_U_V;
        buf=pJpeg2Bmp->U;
        pQtZzMCU=pJpeg2Bmp->QtZzMCUBuffer+pJpeg2Bmp->Y_in_MCU*64;
        break;
    case 2:
        H=pJpeg2Bmp->SampRate_V_H;
        VV=pJpeg2Bmp->SampRate_V_V;
        buf=pJpeg2Bmp->V;
        pQtZzMCU=pJpeg2Bmp->QtZzMCUBuffer+(pJpeg2Bmp->Y_in_MCU+pJpeg2Bmp->U_in_MCU)*64;
        break;
    }
    for (i=0;i<VV;i++)
        for(j=0;j<H;j++)
            for(k=0;k<8;k++)
                for(h=0;h<8;h++)
                    buf[(i*8+k)*pJpeg2Bmp->SampRate_Y_H*8+j*8+h]=*pQtZzMCU++;
}

///////////////////////////////////////////////////////////////////////////////
static T_VOID StoreBuffer(T_U8 *RgbData, T_U8 dstType)
{
    T_S16 i,j;
    T_U8  *lpbmp;
    T_U8 R,G,B;
    T_S32 y,u,v,rr,gg,bb;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");

    if(pJpeg2Bmp->qq_flag==0)
    {
        for(i=0;i<pJpeg2Bmp->SampRate_Y_V*8;i++)
        {
            if((pJpeg2Bmp->sizei+i)<pJpeg2Bmp->ImgHeight)
            {
                if(dstType == JPEG_TO_RGBLINE)
                    lpbmp=(T_U8 *)RgbData+(T_U32)i*pJpeg2Bmp->ImgWidth*3+pJpeg2Bmp->sizej*3;
                else
                    lpbmp=((T_U8 *)RgbData+(T_U32)(pJpeg2Bmp->ImgHeight-pJpeg2Bmp->sizei-i-1)*pJpeg2Bmp->LineBytes+pJpeg2Bmp->sizej*3);
                for(j=0;j<pJpeg2Bmp->SampRate_Y_H*8;j++)
                {
                    if((pJpeg2Bmp->sizej+j)<pJpeg2Bmp->ImgWidth)
                    {
                        y=pJpeg2Bmp->Y[i*8*pJpeg2Bmp->SampRate_Y_H+j];
                        u=pJpeg2Bmp->U[(i/pJpeg2Bmp->V_YtoU)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoU];
                        v=pJpeg2Bmp->V[(i/pJpeg2Bmp->V_YtoV)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoV];
                        if (dstType == JPEG_TO_RGB || dstType == JPEG_TO_RGBLINE)
                        {
                            rr=((y<<8)+18*u+367*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+411*u-29*v)>>8;
                            /*rr=((y<<8)+359*(u-128))>>8;
                            gg=((y<<8)-183*(u-128)-88*(v-128))>>8;
                            bb=((y<<8)+453*(v-128))>>8;*/
                        }
                        else
                        {
                            rr=((y<<8)+411*u-29*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+18*u+367*v)>>8;
                        }
                        if (bb>255) 
                            *lpbmp++=255; 
                        else if (bb<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)bb;
                        if (gg>255) 
                            *lpbmp++=255; 
                        else if (gg<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)gg;
                        if (rr>255) 
                            *lpbmp++=255; 
                        else if (rr<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)rr;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    else if(pJpeg2Bmp->qq_flag==1)//quarter of quarter
    {
        for(i=0;i<pJpeg2Bmp->SampRate_Y_V*8;i+=4)
        {
            if((pJpeg2Bmp->sizei+i)<pJpeg2Bmp->ImgHeight)
            {
                if(dstType == JPEG_TO_RGBLINE)
                    lpbmp=(T_U8 *)RgbData+(T_U32)i*pJpeg2Bmp->ImgWidth*3/4+pJpeg2Bmp->sizej/4*3;
                else
                    lpbmp = (T_U8*)(RgbData+(T_U32)(pJpeg2Bmp->ImgHeight/4-(pJpeg2Bmp->sizei+i)/4-1)*pJpeg2Bmp->LineBytes+pJpeg2Bmp->sizej/4*3);
                //lpbmp=((T_U8 *)RgbData+(T_U32)(ImgHeight/4-(sizei+i)/4-1)*LineBytes+sizej/4*3);
                for(j=0;j<pJpeg2Bmp->SampRate_Y_H*8;j+=4)
                {
                    if((pJpeg2Bmp->sizej+j)<pJpeg2Bmp->ImgWidth)
                    {
                        y=pJpeg2Bmp->Y[i*8*pJpeg2Bmp->SampRate_Y_H+j];
                        u=pJpeg2Bmp->U[(i/pJpeg2Bmp->V_YtoU)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoU];
                        v=pJpeg2Bmp->V[(i/pJpeg2Bmp->V_YtoV)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoV];
                        if (dstType == JPEG_TO_RGB || dstType == JPEG_TO_RGBLINE)
                        {
                            rr=((y<<8)+18*u+367*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+411*u-29*v)>>8;
                            /*rr=((y<<8)+359*(u-128))>>8;
                            gg=((y<<8)-183*(u-128)-88*(v-128))>>8;
                            bb=((y<<8)+453*(v-128))>>8;*/
                        }
                        else
                        {
                            rr=((y<<8)+411*u-29*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+18*u+367*v)>>8;
                        }
                        R=(T_U8)rr;
                        G=(T_U8)gg;
                        B=(T_U8)bb;
                        if (rr&0xffffff00) 
						{
							if (rr>255) 
								R=255; 
							else if (rr<0) 
								R=0;
						}
                        if (gg&0xffffff00) 
                        {
							if (gg>255) 
								G=255; 
							else if (gg<0) 
								G=0;
                        }
                        if (bb&0xffffff00) 
                        {
							if (bb>255) 
								B=255; 
							else if (bb<0) 
								B=0;
                        }
                        *lpbmp++=B;
                        *lpbmp++=G;
                        *lpbmp++=R;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
    else if(pJpeg2Bmp->qq_flag==2)//scale down
    {
        T_U8 *lpLineBase;
        for(i=0;i<pJpeg2Bmp->SampRate_Y_V*8;i++)
        {
            if((pJpeg2Bmp->sizei+i)<pJpeg2Bmp->ImgHeight)
            {
                //lpbmp=((T_U8 *)RgbData+(T_U32)(ImgHeight-sizei-i-1)*LineBytes+sizej*3);
                lpLineBase = ((T_U8 *)RgbData+(T_U32)(pJpeg2Bmp->DstHeight-(pJpeg2Bmp->sizei+i)*pJpeg2Bmp->DstHeight/pJpeg2Bmp->ImgHeight-1)*pJpeg2Bmp->LineBytes);
                for(j=0;j<pJpeg2Bmp->SampRate_Y_H*8;j++)
                {
                    if((pJpeg2Bmp->sizej+j)<pJpeg2Bmp->ImgWidth)
                    {
                        lpbmp=(lpLineBase+(pJpeg2Bmp->sizej+j)*pJpeg2Bmp->DstWidth/pJpeg2Bmp->ImgWidth*3);

                        y=pJpeg2Bmp->Y[i*8*pJpeg2Bmp->SampRate_Y_H+j];
                        u=pJpeg2Bmp->U[(i/pJpeg2Bmp->V_YtoU)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoU];
                        v=pJpeg2Bmp->V[(i/pJpeg2Bmp->V_YtoV)*8*pJpeg2Bmp->SampRate_Y_H+j/pJpeg2Bmp->H_YtoV];
                        if (dstType == JPEG_TO_RGB)
                        {
                            rr=((y<<8)+18*u+367*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+411*u-29*v)>>8;
                            /*rr=((y<<8)+359*(u-128))>>8;
                            gg=((y<<8)-183*(u-128)-88*(v-128))>>8;
                            bb=((y<<8)+453*(v-128))>>8;*/
                        }
                        else
                        {
                            rr=((y<<8)+411*u-29*v)>>8;
                            gg=((y<<8)-159*u-220*v)>>8;
                            bb=((y<<8)+18*u+367*v)>>8;
                        }
                        if (bb>255) 
                            *lpbmp++=255; 
                        else if (bb<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)bb;
                        if (gg>255) 
                            *lpbmp++=255; 
                        else if (gg<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)gg;
                        if (rr>255) 
                            *lpbmp++=255; 
                        else if (rr<0) 
                            *lpbmp++=0;
                        else
                            *lpbmp++=(T_U8)rr;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
static T_S32 DecodeMCUBlock(T_VOID)
{
    T_S16 *lpMCUBuffer;
    T_S16 i,j;
    T_S32 funcret;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    if (pJpeg2Bmp->IntervalFlag)
    {
        pJpeg2Bmp->lpCur+=2;
        pJpeg2Bmp->ycoef=pJpeg2Bmp->ucoef=pJpeg2Bmp->vcoef=0;
        pJpeg2Bmp->BitPos=0;
        pJpeg2Bmp->CurByte=0;
    }

    switch(pJpeg2Bmp->comp_num)
    {
    case 3:
        lpMCUBuffer=pJpeg2Bmp->MCUBuffer;
        for (i=0;i<pJpeg2Bmp->SampRate_Y_H*pJpeg2Bmp->SampRate_Y_V;i++)  //Y
        {
            funcret=HufBlock(pJpeg2Bmp->YDcIndex,pJpeg2Bmp->YAcIndex);
            if (funcret!=JPEG_OK)
                return funcret;
            pJpeg2Bmp->BlockBuffer[0]=pJpeg2Bmp->BlockBuffer[0]+pJpeg2Bmp->ycoef;
            pJpeg2Bmp->ycoef=pJpeg2Bmp->BlockBuffer[0];
            for (j=0;j<64;j++)
                *lpMCUBuffer++=pJpeg2Bmp->BlockBuffer[j];
        }
        for (i=0;i<pJpeg2Bmp->SampRate_U_H*pJpeg2Bmp->SampRate_U_V;i++)  //U
        {
            funcret=HufBlock(pJpeg2Bmp->UVDcIndex,pJpeg2Bmp->UVAcIndex);
            if (funcret!=JPEG_OK)
                return funcret;
            pJpeg2Bmp->BlockBuffer[0]=pJpeg2Bmp->BlockBuffer[0]+pJpeg2Bmp->ucoef;
            pJpeg2Bmp->ucoef=pJpeg2Bmp->BlockBuffer[0];
            for (j=0;j<64;j++)
                *lpMCUBuffer++=pJpeg2Bmp->BlockBuffer[j];
        }
        for (i=0;i<pJpeg2Bmp->SampRate_V_H*pJpeg2Bmp->SampRate_V_V;i++)  //V
        {
            funcret=HufBlock(pJpeg2Bmp->UVDcIndex,pJpeg2Bmp->UVAcIndex);
            if (funcret!=JPEG_OK)
                return funcret;
            pJpeg2Bmp->BlockBuffer[0]=pJpeg2Bmp->BlockBuffer[0]+pJpeg2Bmp->vcoef;
            pJpeg2Bmp->vcoef=pJpeg2Bmp->BlockBuffer[0];
            for (j=0;j<64;j++)
                *lpMCUBuffer++=pJpeg2Bmp->BlockBuffer[j];
        }
        break;
    case 1:
        lpMCUBuffer=pJpeg2Bmp->MCUBuffer;
        funcret=HufBlock(pJpeg2Bmp->YDcIndex,pJpeg2Bmp->YAcIndex);
        if (funcret!=JPEG_OK)
            return funcret;
        pJpeg2Bmp->BlockBuffer[0]=pJpeg2Bmp->BlockBuffer[0]+pJpeg2Bmp->ycoef;
        pJpeg2Bmp->ycoef=pJpeg2Bmp->BlockBuffer[0];
        for (j=0;j<64;j++)
            *lpMCUBuffer++=pJpeg2Bmp->BlockBuffer[j];
        for (i=0;i<128;i++)
            *lpMCUBuffer++=0;
        break;
    default:
        return JPEG_FORMAT_ERROR;
    }
    return JPEG_OK;
}

//////////////////////////////////////////////////////////////////
static T_S32 HufBlock(T_U8 dchufindex,T_U8 achufindex)
{
    T_S16 count=0;
    T_S16 i;
    T_S32 funcret;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    //dc
    pJpeg2Bmp->HufTabIndex=dchufindex;
    funcret=DecodeAElement();
    if(funcret!=JPEG_OK)
        return funcret;

    pJpeg2Bmp->BlockBuffer[count++]=pJpeg2Bmp->vvalue;
    //ac
    pJpeg2Bmp->HufTabIndex=achufindex;
    while (count<64)
    {
        funcret=DecodeAElement();
        if(funcret!=JPEG_OK)
            return funcret;
        if ((pJpeg2Bmp->rrun==0)&&(pJpeg2Bmp->vvalue==0))
        {
            for (i=count;i<64;i++)
                pJpeg2Bmp->BlockBuffer[i]=0;
            count=64;
        }
        else
        {
            for (i=0;i<pJpeg2Bmp->rrun;i++)
                pJpeg2Bmp->BlockBuffer[count++]=0;
            pJpeg2Bmp->BlockBuffer[count++]=pJpeg2Bmp->vvalue;
        }
    }
    return JPEG_OK;
}

//////////////////////////////////////////////////////////////////////////////
static T_S32 DecodeAElement(T_VOID)
{
    T_S32 thiscode,tempcode;
    T_U16 temp,valueex;
    T_S16 codelen;
    T_U8 hufexbyte,runsize,tempsize,sign;
    T_U8 newbyte,lastbyte;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", JPEG_MALLOC_ERROR);

    if(pJpeg2Bmp->BitPos>=1)
    {
        pJpeg2Bmp->BitPos--;
        thiscode=(T_U8)pJpeg2Bmp->CurByte>>pJpeg2Bmp->BitPos;
        pJpeg2Bmp->CurByte=pJpeg2Bmp->CurByte&And[pJpeg2Bmp->BitPos];
    }
    else
    {
        lastbyte=ReadByte();
        pJpeg2Bmp->BitPos--;
        newbyte=pJpeg2Bmp->CurByte&And[pJpeg2Bmp->BitPos];
        thiscode=lastbyte>>7;
        pJpeg2Bmp->CurByte=newbyte;
    }
    codelen=1;

    while ((thiscode<pJpeg2Bmp->huf_min_value[pJpeg2Bmp->HufTabIndex][codelen-1])||
          (pJpeg2Bmp->code_len_table[pJpeg2Bmp->HufTabIndex][codelen-1]==0)||
          (thiscode>pJpeg2Bmp->huf_max_value[pJpeg2Bmp->HufTabIndex][codelen-1]))
    {
        if(pJpeg2Bmp->BitPos>=1)
        {
            pJpeg2Bmp->BitPos--;
            tempcode=(T_U8)pJpeg2Bmp->CurByte>>pJpeg2Bmp->BitPos;
            pJpeg2Bmp->CurByte=pJpeg2Bmp->CurByte&And[pJpeg2Bmp->BitPos];
        }
        else
        {
            lastbyte=ReadByte();
            pJpeg2Bmp->BitPos--;
            newbyte=pJpeg2Bmp->CurByte&And[pJpeg2Bmp->BitPos];
            tempcode=(T_U8)lastbyte>>7;
            pJpeg2Bmp->CurByte=newbyte;
        }
        thiscode=(thiscode<<1)+tempcode;
        codelen++;
        if(codelen>16)
            return JPEG_FORMAT_ERROR;
    }  //while
    temp=thiscode-pJpeg2Bmp->huf_min_value[pJpeg2Bmp->HufTabIndex][codelen-1]+pJpeg2Bmp->code_pos_table[pJpeg2Bmp->HufTabIndex][codelen-1];
    hufexbyte=(T_U8)pJpeg2Bmp->code_value_table[pJpeg2Bmp->HufTabIndex][temp];
    pJpeg2Bmp->rrun=(T_S16)(hufexbyte>>4);
    runsize=hufexbyte&0x0f;
    if(runsize==0)
    {
        pJpeg2Bmp->vvalue=0;
        return JPEG_OK;
    }
    tempsize=runsize;
    if(pJpeg2Bmp->BitPos>=runsize)
    {
        pJpeg2Bmp->BitPos-=runsize;
        valueex=(T_U8)pJpeg2Bmp->CurByte>>pJpeg2Bmp->BitPos;
        pJpeg2Bmp->CurByte=pJpeg2Bmp->CurByte&And[pJpeg2Bmp->BitPos];
    }
    else
    {
        valueex=pJpeg2Bmp->CurByte;
        tempsize-=pJpeg2Bmp->BitPos;
        while(tempsize>8)
        {
            lastbyte=ReadByte();
            valueex=(valueex<<8)+(T_U8)lastbyte;
            tempsize-=8;
        }  //while
        lastbyte=ReadByte();
        pJpeg2Bmp->BitPos-=tempsize;
        valueex=(valueex<<tempsize)+(lastbyte>>pJpeg2Bmp->BitPos);
        pJpeg2Bmp->CurByte=lastbyte&And[pJpeg2Bmp->BitPos];
    }  //else
    sign=valueex>>(runsize-1);
    if(sign)
    {
        pJpeg2Bmp->vvalue=valueex;
    }
    else
    {
        valueex=valueex^0xffff;
        temp=0xffff<<runsize;
        pJpeg2Bmp->vvalue=-(T_S16)(valueex^temp);
    }
    return JPEG_OK;
}

/////////////////////////////////////////////////////////////////////////////////////
static T_VOID IQtIZzMCUComponent(T_S16 flag)
{
    T_S16 H=0,VV=0;
    T_S16 i,j;
    T_S32 *pQtZzMCUBuffer=0;
    T_S16  *pMCUBuffer=0;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");

    switch(flag)
    {
    case 0:
        H=pJpeg2Bmp->SampRate_Y_H;
        VV=pJpeg2Bmp->SampRate_Y_V;
        pMCUBuffer=pJpeg2Bmp->MCUBuffer;
        pQtZzMCUBuffer=pJpeg2Bmp->QtZzMCUBuffer;
        break;
    case 1:
        H=pJpeg2Bmp->SampRate_U_H;
        VV=pJpeg2Bmp->SampRate_U_V;
        pMCUBuffer=pJpeg2Bmp->MCUBuffer+pJpeg2Bmp->Y_in_MCU*64;
        pQtZzMCUBuffer=pJpeg2Bmp->QtZzMCUBuffer+pJpeg2Bmp->Y_in_MCU*64;
        break;
    case 2:
        H=pJpeg2Bmp->SampRate_V_H;
        VV=pJpeg2Bmp->SampRate_V_V;
        pMCUBuffer=pJpeg2Bmp->MCUBuffer+(pJpeg2Bmp->Y_in_MCU+pJpeg2Bmp->U_in_MCU)*64;
        pQtZzMCUBuffer=pJpeg2Bmp->QtZzMCUBuffer+(pJpeg2Bmp->Y_in_MCU+pJpeg2Bmp->U_in_MCU)*64;
        break;
    }
    for(i=0;i<VV;i++)
        for (j=0;j<H;j++)
            IQtIZzBlock(pMCUBuffer+(i*H+j)*64,pQtZzMCUBuffer+(i*H+j)*64,flag);
}

//////////////////////////////////////////////////////////////////////////////////////////
static T_VOID IQtIZzBlock(T_S16  *s ,T_S32 * d,T_S16 flag)
{
    T_S16 i,j;
    T_S16 tag;
    T_S16 *pQt=0;
    T_S32 buffer[8][8];
    T_S16 offset=0;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");

    switch(flag)
    {
    case 0:
        pQt=pJpeg2Bmp->YQtTable;
        offset=128;
        break;
    case 1:
        pQt=pJpeg2Bmp->UQtTable;
        offset=0;
        break;
    case 2:
        pQt=pJpeg2Bmp->VQtTable;
        offset=0;
        break;
    }

    for(i=0;i<8;i++)
        for(j=0;j<8;j++)
        {
            tag=Zig_Zag_tbl[i][j];
            buffer[i][j]=(T_S32)s[tag]*(T_S32)pQt[tag];
        }
    Fast_IDCT((T_S32 *)buffer);
    for(i=0;i<8;i++)
        for(j=0;j<8;j++)
            d[i*8+j]=buffer[i][j]+offset;
}

///////////////////////////////////////////////////////////////////////////////
static T_VOID Fast_IDCT(T_S32 *block)
{
    T_S16 i;

    for (i=0; i<8; i++)
        idctrow(block+8*i);

    for (i=0; i<8; i++)
        idctcol(block+i);
}

///////////////////////////////////////////////////////////////////////////////
static T_U8  ReadByte(T_VOID)
{
    T_U8  i;

	AK_ASSERT_PTR(pJpeg2Bmp, "pJpeg2Bmp: malloc error", 0);

    i=*(pJpeg2Bmp->lpCur++);
    if(i==0xff)
        pJpeg2Bmp->lpCur++;

    pJpeg2Bmp->BitPos=8;
    pJpeg2Bmp->CurByte=i;
    return i;
}

///////////////////////////////////////////////////////////////////////
static T_VOID Initialize_Fast_IDCT(T_VOID)
{
    T_S16 i;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");

    pJpeg2Bmp->iclp = pJpeg2Bmp->iclip+512;
    for (i= -512; i<512; i++)
        pJpeg2Bmp->iclp[i] = (i<-256) ? -256 : ((i>255) ? 255 : i);
}

////////////////////////////////////////////////////////////////////////
static T_VOID idctrow(T_S32 * blk)
{
    T_S32 x0, x1, x2, x3, x4, x5, x6, x7, x8;
    //intcut
    if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
        (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
    {
        blk[0]=blk[1]=blk[2]=blk[3]=blk[4]=blk[5]=blk[6]=blk[7]=blk[0]<<3;
        return;
    }
    x0 = (blk[0]<<11) + 128; // for proper rounding in the fourth stage 
    //first stage
    x8 = JPEG_W7*(x4+x5);
    x4 = x8 + (JPEG_W1-JPEG_W7)*x4;
    x5 = x8 - (JPEG_W1+JPEG_W7)*x5;
    x8 = JPEG_W3*(x6+x7);
    x6 = x8 - (JPEG_W3-JPEG_W5)*x6;
    x7 = x8 - (JPEG_W3+JPEG_W5)*x7;
    //second stage
    x8 = x0 + x1;
    x0 -= x1;
    x1 = JPEG_W6*(x3+x2);
    x2 = x1 - (JPEG_W2+JPEG_W6)*x2;
    x3 = x1 + (JPEG_W2-JPEG_W6)*x3;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;
    //third stage
    x7 = x8 + x3;
    x8 -= x3;
    x3 = x0 + x2;
    x0 -= x2;
    x2 = (181*(x4+x5)+128)>>8;
    x4 = (181*(x4-x5)+128)>>8;
    //fourth stage
    blk[0] = (x7+x1)>>8;
    blk[1] = (x3+x2)>>8;
    blk[2] = (x0+x4)>>8;
    blk[3] = (x8+x6)>>8;
    blk[4] = (x8-x6)>>8;
    blk[5] = (x0-x4)>>8;
    blk[6] = (x3-x2)>>8;
    blk[7] = (x7-x1)>>8;
}

//////////////////////////////////////////////////////////////////////////////
static T_VOID idctcol(T_S32 * blk)
{
    T_S32 x0, x1, x2, x3, x4, x5, x6, x7, x8;

	AK_ASSERT_PTR_VOID(pJpeg2Bmp, "pJpeg2Bmp: malloc error");
    //intcut
    if (!((x1 = (blk[8*4]<<8)) | (x2 = blk[8*6]) | (x3 = blk[8*2]) |
        (x4 = blk[8*1]) | (x5 = blk[8*7]) | (x6 = blk[8*5]) | (x7 = blk[8*3])))
    {
        blk[8*0]=blk[8*1]=blk[8*2]=blk[8*3]=blk[8*4]=blk[8*5]
            =blk[8*6]=blk[8*7]=pJpeg2Bmp->iclp[(blk[8*0]+32)>>6];
        return;
    }
    x0 = (blk[8*0]<<8) + 8192;
    //first stage
    x8 = JPEG_W7*(x4+x5) + 4;
    x4 = (x8+(JPEG_W1-JPEG_W7)*x4)>>3;
    x5 = (x8-(JPEG_W1+JPEG_W7)*x5)>>3;
    x8 = JPEG_W3*(x6+x7) + 4;
    x6 = (x8-(JPEG_W3-JPEG_W5)*x6)>>3;
    x7 = (x8-(JPEG_W3+JPEG_W5)*x7)>>3;
    //second stage
    x8 = x0 + x1;
    x0 -= x1;
    x1 = JPEG_W6*(x3+x2) + 4;
    x2 = (x1-(JPEG_W2+JPEG_W6)*x2)>>3;
    x3 = (x1+(JPEG_W2-JPEG_W6)*x3)>>3;
    x1 = x4 + x6;
    x4 -= x6;
    x6 = x5 + x7;
    x5 -= x7;
    //third stage
    x7 = x8 + x3;
    x8 -= x3;
    x3 = x0 + x2;
    x0 -= x2;
    x2 = (181*(x4+x5)+128)>>8;
    x4 = (181*(x4-x5)+128)>>8;
    //fourth stage
    blk[8*0] = pJpeg2Bmp->iclp[(x7+x1)>>14];
    blk[8*1] = pJpeg2Bmp->iclp[(x3+x2)>>14];
    blk[8*2] = pJpeg2Bmp->iclp[(x0+x4)>>14];
    blk[8*3] = pJpeg2Bmp->iclp[(x8+x6)>>14];
    blk[8*4] = pJpeg2Bmp->iclp[(x8-x6)>>14];
    blk[8*5] = pJpeg2Bmp->iclp[(x0-x4)>>14];
    blk[8*6] = pJpeg2Bmp->iclp[(x3-x2)>>14];
    blk[8*7] = pJpeg2Bmp->iclp[(x7-x1)>>14];
}

/**
 * @brief memory copy
 * @author ZouMai
 * @date 2001-08-01 
 * @param T_U8 *strDest: the dest address
 * @param T_U8 *strSour: the source address 
 * @param T_U16 count: the length to be copied from source to dest
 * @return T_VOID
 * @retval
 */
static T_VOID JpgMemCpy(T_U8 *strDest, T_U8 *strSour, T_U16 count)
{
    T_U16   i;

    for (i = 0; i < count; i++)
    {
        strDest[i] = strSour[i];
    }
    return;
}

/**
 * @brief memory set
 * @author ZouMai
 * @date 2001-08-01 
 * @param T_U8 *strDest: the dest address
 * @param T_U8 value: value to be set
 * @param T_U16 count: the length to be set to dest
 * @return T_VOID
 * @retval
 */
static T_VOID JpgMemSet(T_U8 *strDest, T_U8 value, T_U16 count)
{
    T_U16   i;

    for (i = 0; i < count; i++)
    {
        strDest[i] = value;
    }
    return;
}
