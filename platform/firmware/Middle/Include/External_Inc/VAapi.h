
/**
* @FILENAME VAapi.h
* @BRIEF visual audio lib api  
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology Co., LTD
* @AUTHOR xiang_maiyang
* @DATE 2011-1-21
* @VERSION 1.0
* @REF 
*/


/****************************************************************************
The following is an example to use visual audio lib APIs
    T_BOOL ret = 0;
    VA_CallBackFunc *cb;
    VA_Handle *vaHandle;
    VA_ColBufSize bufSize;
    VA_DrawRect   drawRect;
    cb->malloc = malloc; 
    cb->printf = printf; 
    cb->free   = free;   
    cb->GetAudioData = GetAudioData;

    vaHandle = vaVisualAudioInit(cb, VA_RGB888, MAX_WIDTH, MAX_HEIGHT, colbufAddr);
    
    if(AK_NULL == vaHandle)
    {
        return error;
    }
    
    vaSetDrawType(vaHandle, VA_DRAW_FLAME);
    
    {   // optional step
        vaSetProPara(vaHandle, VA_COLBUF_SIZE, &bufSize);
        
        vaSetProPara(vaHandle, VA_DRAW_RECT, &drawRect);

        vaSetColBufAddr(vaHandle, colBufAddr);
    }

    do 
    {
        if(vaDraw(vaHandle) != AK_TRUE)
        {
            return error;
        }

        бнбн(show ColorBuff)   //the user implement this
        
    } while (more frame);

    vaVisualAudioRelease(vaHandle);
****************************************************************************/

#ifndef _VAAPI_H_
#define _VAAPI_H_

#include "vatypes.h"

// the pattern type, if don't want some pattern, just delete the special one
#define     BAR  
#define     FOGONSEA
#define     FLAME
#define     WING
    
#define     VA_RGB565_BPP     2
#define     VA_RGB888_BPP     3

#define     AUDIO_DATA_LEN    1024   // wave audio data length, prefer to be 2^N
#define     FREQ_DATA_LEN     (AUDIO_DATA_LEN >> 1) //AUDIO_DATA_LEN/2 

typedef enum
{
    VA_RGB565,
    VA_RGB888
}VA_E_COLFORMAT;   

// draw type
typedef enum
{
    VA_DRAW_BAR,
    VA_DRAW_FOGSEA,
    VA_DRAW_FLAME,
    VA_DRAW_WING
}VA_E_DRAWTYPE;

// audio data type
typedef enum
{
    VA_DATA_TIME,
    VA_DATA_FREQ
}VA_E_DATA_TYPE;

// struct of Color buffer size 
typedef struct  
{
    T_U16 bwidth;
    T_U16 bheight;
}VA_ColBufSize;

// struct of draw rect position and size
typedef struct
{
    T_U16 pointX;
    T_U16 pointY;
    T_U16 dwidth;
    T_U16 dheight;
}VA_DrawRect;

// property para type, color buffer size and draw area
typedef enum
{
    VA_COLBUF_SIZE,
    VA_DRAW_RECT
}VA_E_PARATYPE;

// call back functions
typedef  T_VOID *(*VA_CB_MALLOC)(T_U32 size);
typedef  T_VOID  (*VA_CB_FREE)(T_VOID *buff);
typedef  T_S32   (*VA_CB_PRINTF)(const T_CHAR *string, ...);
typedef  T_BOOL  (*VA_CB_GETAUDATA)(T_VOID * AudioBuf, T_U8 dataType, T_U16 dataLen);

typedef struct
{
    VA_CB_MALLOC    malloc;
    VA_CB_FREE      free;
    VA_CB_PRINTF    printf;
    VA_CB_GETAUDATA GetAudioData;
} VA_CallBackFunc;

// the lib handle
typedef T_VOID* VA_Handle; 

/**
* @brief init the visual audio lib
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-3-11
* @Param[in] VA_CallBackFunc *cb: Call back functions
* @Param[in] T_U8 format: color format
* @Param[in] T_U16 width: color buffer width
* @Param[in] T_U16 height: color buffer height
* @Param[in] T_U32 colbufAddr: color buffer address
* @Return if success return the visual audio handle, else return NULL
*/
VA_Handle vaVisualAudioInit(VA_CallBackFunc *cb, VA_E_COLFORMAT format,
                             T_U16 width, T_U16 height, T_U32 colbufAddr);

/**
* @brief release the visual audio lib resource
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Date 2011-3-11
* @Return AK_TRUE
*/
T_BOOL vaVisualAudioRelease(VA_Handle vaHandle);

/**
* @brief set the property parameter
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-3-11
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Param[in] VA_E_PARATYPE paraType: the which para to be set
* @Param[in] T_VOID *value: the value to be set
* @Return AK_TRUE
*/
T_BOOL vaSetProPara(VA_Handle vaHandle, VA_E_PARATYPE paraType, T_VOID *value);

/**
* @brief set the draw area parameter
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-3-11
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Param[in] T_U8 drawType: the draw type to be set
* @Return AK_TRUE
*/
T_BOOL vaSetDrawType(VA_Handle vaHandle, VA_E_DRAWTYPE drawType);

/**
* @brief set the draw area parameter
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-3-11
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Param[in] T_U32 colbufAddr: the color buffer address to be set
* @Return AK_TRUE
*/
T_BOOL vaSetColBufAddr(VA_Handle vaHandle, T_U32 colbufAddr);

/**
* @brief draw the special pattern
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-3-11
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Return AK_TRUE
*/
T_BOOL vaDraw(VA_Handle vaHandle);


/**
* @brief get the audio buffer address
* Copyright (C) 2011 ANYKA (Guangzhou) Microelectronics Technology CO.,LTD
* @Author XiangMaiyang
* @Date 2011-7-11
* @Param[in] VA_Handle vaHandle: the handle of the visual audio
* @Return the buffer address or AK_NULL
*/
T_S32 *vaGetAudioBuf(VA_Handle vaHandle);

#endif
