
#ifndef __ENG_IMGDEC_H__
#define __ENG_IMGDEC_H__


#include "anyka_types.h"
#include "ctl_global.h"
#include "Fwl_osFS.h"
#include "lib_image_api.h"
#include "Eng_ImgConvert.h"
#include "Eng_String.h"

#if (SDRAM_MODE >= 16)
#define LEAST_FREE_SIZE (2 << 20)
#else
#define LEAST_FREE_SIZE (1500 << 10)
#endif

typedef enum {
        DEC_ERROR = 0,
        DEC_CONTINUE,
        DEC_COMPLETE
} T_LARGE_IMG_STATUS;

typedef struct {
    T_U32                       InImgW;             // input image width
    T_U32                       InImgH;             // input image height
    T_U32                       OutImgW;
    T_U32                       OutImgH;
    T_FILE_TYPE                 FileType;

    T_U16                       BmpDeep;
    T_U16                       OutImgDeep;
    T_U32                       BmpCompression;

    T_U32                       GifFrameInterval;
	T_BOOL						bIntervalChange;
    //T_pDATA                     pGifCurFrame;
    T_U32                       GifCurFrameSize;
    T_S32                       GifHandle;

    T_pDATA                     pInImgBuf;
    T_U32                       InImgSize;
    T_pDATA                     pOutImgBuf;
    T_U32                       OutImgSize;
} T_IMGDEC_ATTRIB;

typedef struct {
    T_pDATA                     pOutLineBuf;        // line buf for decode large jpg and bmp
    T_U32                       CurY;
    T_U32                       CurLines;
    T_U32                       TotalLines;
    T_LARGE_IMG_STATUS          DecStatus;           // image decode status

    T_pFILE                     pLargeBmpFp;

    T_IMGDEC_ATTRIB             ImgAttrib;
} T_IMGDEC_LARGE_ATTRIB;


T_VOID ImgDec_GetDstWH(T_U32 ImgW, T_U32 ImgH, T_U32 MaxDisW, T_U32 MaxDisH, T_U32 *pDstW, T_U32 *pDstH);
T_BOOL ImgDec_GifDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_BOOL ImgDec_GifDecGetNextFrame(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_hGIFENUM ImgDec_GifDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_BOOL ImgDec_PngDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_pDATA ImgDec_PngDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_BOOL ImgDec_JpgDecOpen(T_IMGDEC_ATTRIB *pImgDecAttrib, T_BOOL autoShorten, T_BOOL *pLargeFlg);
T_pDATA ImgDec_JpgDecClose(T_IMGDEC_ATTRIB *pImgDecAttrib);

T_VOID ImgDec_LargeJpgDecClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_LARGE_IMG_STATUS ImgDec_LargeJpgDecStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_BOOL ImgDec_LargeJpgDecOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_pCWSTR pJpgPath);

T_pDATA ImgDec_BmpDecGetRgbBufPtr(T_pCDATA pBmpBuf);
T_VOID ImgDec_LargeBmpDecClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_LARGE_IMG_STATUS ImgDec_LargeBmpDecStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_BOOL ImgDec_LargeBmpDecOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_pCWSTR pBmpPath, T_U32 maxW, T_U32 maxH);

T_BOOL ImgDec_LargeImgOpen(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib, T_USTR_FILE pFilePath);
T_VOID ImgDec_LargeImgClose(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_LARGE_IMG_STATUS ImgDec_LargeImgStep(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);
T_LARGE_IMG_STATUS ImgDec_GetLargeDecStatus(T_IMGDEC_LARGE_ATTRIB *pLargeAttrib);



T_VOID ImgDec_WriteBMPHead(T_U8 *headBuf, T_U16 deep, T_U32 compression, T_U32 width, T_U32 height);
T_BOOL ImgDec_DecBMPHead(T_pCDATA pHeadBuf, T_U16 *pDeep, T_U32 *pCompression, T_U32 *pBmpW, T_U32 *pBmpH);
T_VOID ImgDec_GetBmpSize(T_U8 *bmp_buf, T_U32 *width, T_U32 *height);
T_U32 ImgDec_GetBMPDataLen(T_U32 width, T_U32 height, T_U16 deep, T_U32 compression);
T_pDATA ImgDec_GetImageData(T_USTR_FILE file_path);
T_VOID *ImgDec_FreeImageData(T_U8 *bmp_buf);

T_pDATA ImgDec_GetImageDataFromBuf(const T_U8 *img_buf, T_U32 img_Size);
T_FILE_TYPE ImgDec_GetFileType(T_IMGDEC_ATTRIB *pImgDecAttrib);
T_pDATA ImgDec_GetOutBuf(T_IMGDEC_ATTRIB *pImgDecAttrib);

T_FILE_TYPE ImgDec_GetImgType(const T_U8 *img_buf);
T_BOOL ImgDec_ImgOpen(T_IMGDEC_ATTRIB *pImgAttrib, T_USTR_FILE pFilePath, T_BOOL autoShorten, T_BOOL *pLargeFlag);
T_VOID ImgDec_ImgClose(T_IMGDEC_ATTRIB *pImgAttrib);

T_VOID ImgDec_SetImgLibCallBack(T_VOID);
T_VOID Img_SetJpgTask_CB(JPEGTaskFunc func);

#endif



