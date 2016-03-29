
#ifndef __FWL_IMAGE_H__
#define __FWL_IMAGE_H__

#include "Fwl_public.h"

#if (SDRAM_MODE == 8)
#define IMAGE_WIDTH_MAX                 2560
#define IMAGE_HEIGHT_MAX                2048
#define PICTURE_AUTOSHORT_WIDTH         (Fwl_GetLcdWidth())
#define PICTURE_AUTOSHORT_HEIGHT        (Fwl_GetLcdHeight())
#define BMP_OUT_LINE_NUM                32//64
#else
#define IMAGE_WIDTH_MAX                 4160
#define IMAGE_HEIGHT_MAX                3120
#define PICTURE_AUTOSHORT_WIDTH         (Fwl_GetLcdWidth() << 1)
#define PICTURE_AUTOSHORT_HEIGHT        (Fwl_GetLcdHeight() << 1)
#define BMP_OUT_LINE_NUM                128
#endif

#define GE_BITMAP_WIDTH                 Fwl_GetLcdWidth()
#define GE_BITMAP_HIGHT                 Fwl_GetLcdHeight()

#if (SDRAM_MODE >= 16)
#define LARGE_DEC_BMP_DST_WIDTH_MAX     (Fwl_GetLcdWidth() << 1)
#define LARGE_DEC_BMP_DST_HEIGHT_MAX    (Fwl_GetLcdHeight() << 1)
#else
#define LARGE_DEC_BMP_DST_WIDTH_MAX     320
#define LARGE_DEC_BMP_DST_HEIGHT_MAX    240
#endif
#define LARGE_DEC_JPG_DST_WIDTH_MAX     Fwl_GetLcdWidth()
#define LARGE_DEC_JPG_DST_HEIGHT_MAX    Fwl_GetLcdHeight()

//normal show, jpg picture is autoshorten to the size
#if (SDRAM_MODE >= 16)
#define JPG_AUTOSHORT_WIDTH             (Fwl_GetLcdWidth() << 1)
#define JPG_AUTOSHORT_HEIGHT            (Fwl_GetLcdHeight() << 1)
#else
#define JPG_AUTOSHORT_WIDTH             320
#define JPG_AUTOSHORT_HEIGHT            240
#endif

#if (SDRAM_MODE == 8)
#define EMAP_AUTOSHORT_WIDTH            800
#define EMAP_AUTOSHORT_HEIGHT           600
#endif

#ifdef CAMERA_SUPPORT
#define PREVIEW_WIDTH               MAIN_LCD_WIDTH
#define PREVIEW_HEIGHT              MAIN_LCD_HEIGHT

#define CAM_IMG_INTERVAL    4
#define CAM_MULTI_IMG_WIDHT       ((MAIN_LCD_WIDTH - CAM_IMG_INTERVAL * 3) >> 1)
#define CAM_MULTI_IMG_HEIGHT      ((MAIN_LCD_HEIGHT - CAM_IMG_INTERVAL * 3) >> 1)
#define CAM_ONE_IMG_WIDHT         (MAIN_LCD_WIDTH - (CAM_IMG_INTERVAL << 1))
#define CAM_ONE_IMG_HEIGHT        (MAIN_LCD_HEIGHT - (CAM_IMG_INTERVAL << 1))

#endif


#if (SDRAM_MODE == 8)
#define JPG_BUF_SIZE    450*1024  //600*1024
#else
#define JPG_BUF_SIZE    800*600*3
#endif

#define BMP_HEAD_SIZE           54
#define BMP_FILEHEADER_SIZE     14
#define BMP_INFOHEADER_SIZE     40
#define LARGE_DEC_BMP_DST_SIZE_MAX          (BMP_HEAD_SIZE + LARGE_DEC_BMP_DST_WIDTH_MAX * LARGE_DEC_BMP_DST_HEIGHT_MAX * 3 + 15)

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
#define FULL_BMP_SIZE           (MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 2 + 8)
#else
#define FULL_BMP_SIZE           (MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * 3 + 8)
#endif

//define size of imgbuf to get image type.
#define IMG_TYPE_INFO_SIZE  10

#endif
