 /**************************************************************************
 *
 * Copyrights (C) 2002, ANYKA software Inc
 * All rights reserced.
 *
 * File name: Fwl_pfCamera.c
 * Function: This file will constraint the access to the bottom layer file
 system, avoid resource competition. Also, this file os for
 porting to different OS
 *
 * Author: Zou Mai
 * Date: 2001-06-14
 * Version: 1.0
 *
 * Revision: 
 * Author: 
 * Date: 
***************************************************************************/
#include "anyka_types.h"

#ifdef CAMERA_SUPPORT

#include <stdio.h>

#include "Fwl_pfCamera.h"
#include "eng_debug.h"
#include "fwl_power.h"
#include "ctl_msgbox.h"
#include "ctl_preview.h"
#include "akos_api.h"
#include "ctl_videorecord.h"

#include "log_media_recorder.h"
#include "lib_image_api.h"
#include "hal_camera.h"
#include "drv_gpio.h"

#ifdef OS_ANYKA
#include "gpio_config.h"
#endif

#ifdef OS_WIN32
#include <string.h>
#endif

extern T_U8 *lcd_get_disp_buf(T_VOID);


T_SHOT_PARAM    gb_camera_shotObj;

T_CAMERA_BUFFER m_CameraBuffer1, m_CameraBuffer2, m_CameraBuffer3;
static T_BOOL  gCameramRecIsOpen = AK_FALSE; 

/*
 * @brief  the function get camera's max output size 
 * @author WangXi
 * @param	data	2010-06-27
 * @param[out] pCamMaxW/pCamMaxH:max size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL	Fwl_CameraGetMaxSize(T_U32 *pCamMaxW, T_U32 *pCamMaxH)
{
	T_U32 maxW = 0,maxH = 0;
	T_CAMERA_TYPE camType = 0;
	
	if ((AK_NULL == pCamMaxW) || (AK_NULL == pCamMaxH))
	{
		return AK_FALSE;
	}
    
#ifdef OS_ANYKA
	camType = Fwl_CameraGetType();
	if (0 == camType)
	{
		return AK_FALSE;
	}
#endif

	if (CAMERA_1P3M <= camType && (SDRAM_MODE >= 16))//130万像素
	{
		maxW = 1280;
		maxH = 720;
	}
	else
	{
		//30万像素
		maxW = 640;
		maxH = 480;
	}

	(*pCamMaxW) = maxW;
	(*pCamMaxH) = maxH;
	
	return AK_TRUE;
}


/**
 * Copyright(c) Anyka.
 * @Author      : Bennis Zhang
 * @Date        : 2009-05-15
 * @Description :
 */
T_BOOL Fwl_GetRecFrameSize(T_CAMERA_MODE mode,T_U32 *wi,T_U32 *hi)
{
	T_U32 width = 0,height = 0;

	if ((AK_NULL == wi) || (AK_NULL == hi))
	{
		return AK_FALSE;
	}
	
	switch(mode)
	{
		case CAMERA_MODE_QXGA:
			width  = 2048;
			height = 1536;
			break;
		case CAMERA_MODE_UXGA:
			width  = 1600;
			height = 1200;
			break;
		case CAMERA_MODE_SXGA:
			width  = 1280;
			height = 1024;
			break;
		case CAMERA_MODE_XGA:
			width  = 1024;
			height = 768;
			break;
		case CAMERA_MODE_SVGA:
			width  = 800;
			height = 600;
			break;
		case CAMERA_MODE_QSVGA:
			width  = 400;
			height = 300;
			break;
		case CAMERA_MODE_REC:
		case CAMERA_MODE_CIF:
			width  = 352;
			height = 288;
			break;
		case CAMERA_MODE_QVGA:
			width  = 320;
			height = 240;
			break;
		case CAMERA_MODE_QCIF:
			width  = 176;
			height = 144;
			break;
		case CAMERA_MODE_QQVGA:
			width  = 160;
			height = 120;
			break;
		case CAMERA_MODE_D1:
			width  = 720;
			height = 480;
			break;
		case CAMERA_MODE_720P:
			width  = 1280;
			height = 720;
			break;
		case CAMERA_MODE_960:
			width  = 1280;
			height = 960;
			break;
		
		case CAMERA_MODE_PREV:
		case CAMERA_MODE_VGA:
		default:
			width  = 640;
			height = 480;
			break;
	}
	(*wi) = width;
	(*hi) = height;
	
	return AK_TRUE;
}


T_BOOL  Fwl_CameraCheckSize(T_U32 *pWidth, T_U32 *pHeight)
{
	T_U32 adjustW = 0,adjustH = 0;
    T_U32 tmpSize;

	if ((AK_NULL == pWidth) || (AK_NULL == pHeight))
	{
		return AK_FALSE;
	}

    adjustW = (*pWidth);
    adjustH = (*pHeight);
    
	if ((0 == adjustW) || (0 == adjustH))
	{
		return AK_FALSE;
	}
	
    tmpSize = adjustW * adjustH;

    if (0 != tmpSize%128)
    {
        //calculate the size which value is nearly the  size in parameter seted
        tmpSize += tmpSize%128;
        
        adjustH = tmpSize/adjustW;
    }
    

    adjustW -= adjustW%4;
    adjustH -= adjustH%2;

	*pWidth  = adjustW;
	*pHeight = adjustH;
	
	return AK_TRUE;
}


T_BOOL  Fwl_CameraGetRecSize(T_U32 *pWidth, T_U32 *pHeight)
{
	T_U32 adjustW = 0,adjustH = 0;
	T_U32 vgaW = 0,vgaH = 0;
    
	if ((AK_NULL == pWidth) || (AK_NULL == pHeight))
	{
		return AK_FALSE;
	}
	

    adjustW = (*pWidth);
    adjustH = (*pHeight);
    
	if ((0 == adjustW) || (0 == adjustH))
	{
		return AK_FALSE;
	}
	
    // if  not greater than  VGA,  adjust to VGA mode
    Fwl_GetRecFrameSize(CAMERA_MODE_VGA, &vgaW, &vgaH);
	if ((adjustW <= vgaW) && (adjustH <= vgaH))
	{
        adjustW  = vgaW;
        adjustH  = vgaH;
	}
    else
    {
        Fwl_CameraGetMaxSize(&adjustW , &adjustH);
    }

	*pWidth  = adjustW;
	*pHeight = adjustH;
	
	return AK_TRUE;
}


/*
 * @brief  the function get scale by output size
 * @author WangXi
 * @param	data	2010-06-27
 * @param[out] pCamWinW/pCamWinH:adjust size	
 * @param[in] destW/destH:output size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL  Fwl_CameraGetScale(T_U32 *pScaleW, T_U32 *pScaleH, T_U32 destW, T_U32 destH)
{
	T_U32 scaleMinW = 0,scaleMinH = 0;
	T_U32 cnt = 0;
	T_U32 i = 0;
    T_U8 camSupportScaleTable[][2] = //可支持的视频比例
	{
		{4, 3},{16, 9},{3, 2},{11, 9},{5, 4},
	};

	if ((AK_NULL == pScaleW) || (AK_NULL == pScaleH))
	{
		return AK_FALSE;
	}

    // 查找输出尺寸的合适比例
	cnt = sizeof(camSupportScaleTable)/sizeof(camSupportScaleTable[0]);
    for (i=0; i<cnt; i++)
	{
		scaleMinW = camSupportScaleTable[i][0];
		scaleMinH = camSupportScaleTable[i][1];

		if ((destW*scaleMinH) == (destH*scaleMinW))
		{
			break;
		}

		scaleMinW = camSupportScaleTable[i][1];
		scaleMinH = camSupportScaleTable[i][0];

		if ((destW*scaleMinH) == (destH*scaleMinW))
		{
			break;
		}
	}

	// 如果是不支持的比例，按照默认的比例处理
	if (i >= cnt)
	{
		scaleMinW = camSupportScaleTable[0][0];
		scaleMinH = camSupportScaleTable[0][1];
	}

	*pScaleW = scaleMinW;
	*pScaleH = scaleMinH;
	
	return AK_TRUE;
}

/*
 * @brief  the function get min size by output size
 * @author WangXi
 * @param	data	2010-06-27
 * @param[out] pCamWinW/pCamWinH:adjust size	
 * @param[in] destW/destH:output size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL  Fwl_CameraGetMinSize(T_U32 *pCamWinW, T_U32 *pCamWinH, T_U32 destW, T_U32 destH)
{
	//T_U32 scaleMinW = 0,scaleMinH = 0;
	T_U32 tmpW = 0, tmpH = 0;
	//T_U32 i = 0;

	if ((AK_NULL == pCamWinW) || (AK_NULL == pCamWinH))
	{
		return AK_FALSE;
	}
#if 0
	Fwl_CameraGetScale(&scaleMinW, &scaleMinH, destW,destH);
    tmpW  = scaleMinW * 16;
	tmpH  = scaleMinH * 16;
#else
    tmpW = destW >> 1; // 需求要求(最小尺寸为2分之1)
    tmpH = destH >> 1;
#endif
	*pCamWinW = tmpW;
	*pCamWinH = tmpH;
	return AK_TRUE;
}

/*
 * @brief  the function get adjust size by output size
 * @author WangXi
 * @param	data	2010-06-27
 * @param[out] pCamWinW/pCamWinH:adjust size	
 * @param[in] destW/destH:output size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL  Fwl_CameraGetWinSize(T_U32 *pCamWinW, T_U32 *pCamWinH, T_U32 destW, T_U32 destH)
{
	T_U32 srcMaxW = 0,srcMaxH = 0;
	T_U32 adjustW = 0,adjustH = 0;
	T_U32 tmpW = 0, tmpH = 0;
	T_U32 scaleMinW = 0,scaleMinH = 0;
	T_U32 maxSize = 0,tmpSize = 0;
	T_S32 i = 0;

	if ((AK_NULL == pCamWinW) || (AK_NULL == pCamWinH))
	{
		return AK_FALSE;
	}

	if (!Fwl_CameraGetMaxSize(&srcMaxW, &srcMaxH))
	{
		return AK_FALSE;
	}

	// 根据视频大小按比例调整摄像头输出大小(需要在Open之后调用)
#if 1 //特殊尺寸处理	
    if ((320 == destW) && (240 == destH))
	{
	    //setup windows VGA, camera can auto zoom out QVGA
		adjustW  = 640;
		adjustH  = 480;
	}
	// 如果视频尺寸在sensor支持的尺寸内,则调整sensor输出尺寸为视频真实大小
	else if ((srcMaxW >= destW) && (srcMaxH >= destH))
	{
		adjustW  = destW;
		adjustH  = destH;
	}
	else
#endif
	{
		maxSize = srcMaxW * srcMaxH;
		
		// 查找输出尺寸的合适比例
		Fwl_CameraGetScale(&scaleMinW, &scaleMinH, destW, destH);
		
		i=1;
		adjustW = srcMaxW;
		adjustH = srcMaxH;
		do {
			tmpW = scaleMinW * 2 * i;
			tmpH = scaleMinH * 2 * i;
			tmpSize = tmpW * tmpH;
			
			if ((tmpW <= srcMaxW) && (tmpH <= srcMaxH) && (tmpSize <= maxSize))
			{
				// 芯片限制
				if (0 == (tmpSize % 128))
				{
					adjustW = tmpW;
					adjustH = tmpH;
				}
			}
			else
			{
				break;
			}
		} while(i++);
	}
	
	*pCamWinW = adjustW;
	*pCamWinH = adjustH;
	
	return AK_TRUE;
}



/*
 * @brief  the function get focus win size by orign size  and focus lvl
 * @author WangXi
 * @param	data	2010-06-27
 * @param[out] pCamWinW/pCamWinH:adjust size	
 * @param[in] destW/destH:output size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL  Fwl_CameraGetFocusWin(T_RECT *pFocusWin, T_U32 focusLvl, T_U32 maxLvl,T_U32 orignW, T_U32 orignH)
{
	T_U32 minWidth = 0,minHeight = 0;

	if (AK_NULL == pFocusWin)
	{
		return AK_FALSE;
	}

	if (focusLvl > maxLvl)
	{
       focusLvl = 0;
	}


	if (0 == focusLvl)
	{
		pFocusWin->width  = (T_LEN)orignW;
		pFocusWin->height = (T_LEN)orignH;
	}

	// focus win , resize the rect size
	else
	{
		Fwl_CameraGetMinSize(&minWidth,&minHeight, orignW, orignH);
		pFocusWin->width  = (T_LEN)(minWidth + (((maxLvl - focusLvl) *(orignW - minWidth)) / maxLvl));
		pFocusWin->height = (T_LEN)(minHeight + (((maxLvl - focusLvl) *(orignH - minHeight)) / maxLvl));
	}

	pFocusWin->left    = (T_POS)((orignW  - pFocusWin->width)>>1);
	pFocusWin->top     = (T_POS)((orignH  - pFocusWin->height)>>1);

	pFocusWin->width  -= pFocusWin->width  % 2;
	pFocusWin->height -= pFocusWin->height % 2;
	pFocusWin->left   -= pFocusWin->left   % 2;
	pFocusWin->top    -= pFocusWin->top    % 2;

	return AK_TRUE;
}



T_VOID Fwl_CameraGetFocusTips(T_U32 focusLvl, T_U8 *tips)
{
	T_U8	strProgress[16] = {0};
	T_U32	i = 0;

	if (AK_NULL != tips)
	{
	    if (focusLvl > 0)
	    {
            strProgress[0] = 0;
            strProgress[1] = 0;
            for (i=0; i<focusLvl;i++)
            {
                strcat(strProgress, ">");
            }
            sprintf(tips, "%s %lu", strProgress, focusLvl);
	    }
	    else
	    {
            tips[0] = ' ';
            tips[1] = 0;
	    }
	}
}


/*
 * @brief  the function get clip win  by  reference win(refrence this win's scale)
 * @author WangXi
 * @param	data	2011-10-17
 * @param[out] pClipWin:adjust win	
 * @param[in] srcW/srcH:src size	
 * @param[in] refW/refH:reference size	
 * @return	resulst AK_TRUE--success AK_FALSE--fail
 */
T_BOOL  Fwl_CameraGetClipWin(T_RECT *pClipWin, T_U32 srcW, T_U32 srcH,
                            T_U32 destW, T_U32 destH, T_CAM_CLIP_MODE clipMode)
{
    T_U32 adjustW = 0,  adjustH = 0;
    T_U32 mutiProduct_FixedH = 0, mutiProduct_FixedW = 0;

    if (AK_NULL == pClipWin)
    {
        return AK_FALSE;
    }

    // check parameter is valid
    if ((0 == srcW) || (0 == srcH) || (0 == destW) || (0 == destH))
    {
        return AK_FALSE;
    }
    
    // check scale if need clip
    mutiProduct_FixedW = destW * srcH;// mutiplying product refrence as source height
    mutiProduct_FixedH = destH * srcW;// mutiplying product refrence as source width

    // if the source scale is equal dest scale, need not clip OR source scale is equal dest scale
    if ((eCAMCLIP_FULL == clipMode) || (mutiProduct_FixedW == mutiProduct_FixedH))
    {
        pClipWin->width   = (T_LEN)destW; 
        pClipWin->height  = (T_LEN)destH;
    }
    else // if scale is not equal dest scale, use below logic branch to calculate width or height 
    {
        // calulate  width or height  use source scale
        adjustH      = mutiProduct_FixedW / srcW;
        adjustW      = mutiProduct_FixedH / srcH;
        
        if (adjustW > destW) // if adjust width is more than source width 
        {
            // use adjust  height directly
            pClipWin->width  = (T_LEN)destW;

            // calculate the width by src scale factor
            pClipWin->height = (T_LEN)adjustH;
        }
        else // if adjust height is more than source height
        {
            // use dest width directly
            pClipWin->height  = (T_LEN)destH;

            // calculate the height by src scale factor
            pClipWin->width   = (T_LEN)adjustW;
        }
    }


    pClipWin->left    = (T_POS)((destW  - pClipWin->width)  >> 1);
    pClipWin->top     = (T_POS)((destH  - pClipWin->height) >> 1);

    pClipWin->width  -= pClipWin->width  % 2;
    pClipWin->height -= pClipWin->height % 2;
    pClipWin->left   -= pClipWin->left   % 2;
    pClipWin->top    -= pClipWin->top    % 2;

    return AK_TRUE;
}





/**
 * @brief Initialize camera in record mode
 * @param[in] srcwidth   window width
 * @param[in] srcheight  window height
 * @param[in] deswidth   width of capture size
 * @param[in] desheight  height of capture size
 * @param[in] pYUV1     camera buffer 1
 * @param[in] pYUV2     camera buffer 2
 * @param[in] pYUV3     camera buffer 3
 * @return success or not
 */
T_BOOL Fwl_CameraRecInit(T_U32 srcWidth, T_U32 srcHeight,
                    T_U8 *pYUV1, T_U8 *pYUV2, T_U8 *pYUV3)
{

    T_CAMERA_BUFFER *YUV1, *YUV2, *YUV3; 
    T_BOOL ret = AK_FALSE;
    
    Fwl_Print(C3,M_CAMERA,"Recordinit\n");

#if (defined(CHIP_AK3631) || defined(CHIP_AK322L) || defined(CHIP_AK3224))
    /**YUV fomat: 4:2:2*/
    if (pYUV1 != AK_NULL)
    {
        m_CameraBuffer1.dY = pYUV1;
        m_CameraBuffer1.dU = pYUV1 + srcWidth * srcHeight;
        m_CameraBuffer1.dV = pYUV1 + srcWidth * srcHeight * 3 / 2;
        
        YUV1 = &m_CameraBuffer1;
    }
    else
    {
        YUV1 = AK_NULL;
    }

    if (pYUV2 != AK_NULL)
    {
        m_CameraBuffer2.dY = pYUV2;
        m_CameraBuffer2.dU = pYUV2 + srcWidth * srcHeight;
        m_CameraBuffer2.dV = pYUV2 + srcWidth * srcHeight * 3 / 2;
        
        YUV2 = &m_CameraBuffer2;
    }
    else
    {
        YUV2 = AK_NULL;
    }

    if(pYUV3 != AK_NULL)
    {
        m_CameraBuffer3.dY = pYUV3;
        m_CameraBuffer3.dU = pYUV3 + srcWidth * srcHeight;
        m_CameraBuffer3.dV = pYUV3 + srcWidth * srcHeight * 3 / 2;
        
        YUV3 = &m_CameraBuffer3;
    }
    else
    {
        YUV3 = AK_NULL;
    }
    

#else
    /**YUV format: 4:2:0*/
    if (pYUV1 != AK_NULL)
    {
        m_CameraBuffer1.dY = pYUV1;
        m_CameraBuffer1.dU = pYUV1 + srcWidth * srcHeight;
        m_CameraBuffer1.dV = pYUV1 + srcWidth * srcHeight * 5 / 4;
        
        YUV1 = &m_CameraBuffer1;
    }
    else
    {
        YUV1 = AK_NULL;
    }

    if (pYUV2 != AK_NULL)
    {
        m_CameraBuffer2.dY = pYUV2;
        m_CameraBuffer2.dU = pYUV2 + srcWidth * srcHeight;
        m_CameraBuffer2.dV = pYUV2 + srcWidth * srcHeight * 5 / 4;
        
        YUV2 = &m_CameraBuffer2;
    }
    else
    {
        YUV2 = AK_NULL;
    }

    if (pYUV3 != AK_NULL)
    {
        m_CameraBuffer3.dY = pYUV3;
        m_CameraBuffer3.dU = pYUV3 + srcWidth * srcHeight;
        m_CameraBuffer3.dV = pYUV3 + srcWidth * srcHeight * 5 / 4;
        
        YUV3 = &m_CameraBuffer3;
    }
    else
    {
        YUV3 = AK_NULL;
    }
#endif
    
#ifdef OS_ANYKA
    ret = camstream_init(srcWidth, srcHeight, YUV1, YUV2, YUV3);
    camstream_set_callback(AK_NULL);//camstream_set_callback
#endif // OS_ANYKA
	gCameramRecIsOpen = ret;
    return ret;
}

/*
 * @brief  the function init camera
 * 
 * @author Zhengwenbo
 * @param   data    2006-8-1
 * @param[in] mode  1--synchronous mode(capture mode) 0--asynchronous mode(record mode)
 * @return  resulst 1--success 0--fail
 */
T_BOOL Fwl_CameraInit(T_VOID)
{
#ifdef OS_ANYKA
    AK_Feed_Watchdog(0);

    /* init camera */
    if (!cam_open())
    {
        return AK_FALSE;
    }

    /* set mirror */
	Fwl_CameraSetMirror(CAMERA_MIRROR_FLIP);

    return AK_TRUE;
#else
    return AK_FALSE;
#endif // OS_ANYKA
}

T_VOID Fwl_CameraFree(T_VOID)
{
#ifdef OS_ANYKA
    cam_close();

//GPIO_I2C_SDA and GPIO_KEYAPD_COLUMN1 use the same gpio
	gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_HIGH); 
#endif
}

T_VOID Fwl_CameraPowerDown(T_VOID)
{
    Fwl_CameraInit();
    Fwl_CameraFree();
}

T_VOID Fwl_CamStreamStop(T_VOID)
{
#ifdef OS_ANYKA
		if (gCameramRecIsOpen)
		{
			gCameramRecIsOpen = AK_FALSE;
#ifdef OS_ANYKA
			camstream_stop();//重复调用可能会造成死机
#endif
		}
#endif
}

T_BOOL Fwl_CamerIsNightMode(T_U16 FlashMode, T_U16 ColorEffect)
{
	if((CAMERA_NIGHT_MODE != FlashMode) && (CAMERA_EFFECT_NORMAL == ColorEffect))
	{
		return AK_FALSE;
	}

	return AK_TRUE;
}

T_VOID Fwl_CameraSetNightMode(T_U8 mode)
{
#ifdef OS_ANYKA
    cam_set_feature(CAM_FEATURE_NIGHT_MODE, mode);
#endif
}

T_VOID Fwl_CamerChangeNightMode(T_U16 *pVal)
{

	if (*pVal == CAMERA_DAY_MODE)
    {
        *pVal = CAMERA_NIGHT_MODE;
    }
    else
    {
        *pVal = CAMERA_DAY_MODE;
    }
}




T_VOID Fwl_CameraSetEffect(T_U8 effect)
{
#ifdef OS_ANYKA
    cam_set_feature(CAM_FEATURE_EFFECT, effect);
#endif
}

/*
T_BOOL Fwl_CameraChangeSize(T_CAMERA_RESOLUTION size)
{
#ifdef OS_AKRTOS
    Set_CameraResolution( size );
    return AK_TRUE;
#endif

#ifdef OS_WIN32
    return AK_TRUE;
#endif
}
*/
T_BOOL Fwl_CameraSaturationCanDec(T_U8 saturation)
{
	if(saturation > CAMERA_SATURATION_1)
		return AK_TRUE;

	return AK_FALSE;	
}


T_BOOL Fwl_CameraSaturationCanInc(T_U8 saturation)
{
	if(saturation < CAMERA_SATURATION_NUM - 1)
		return AK_TRUE;

	return AK_FALSE;
}


T_BOOL Fwl_CameraChangeSaturation(T_U8 saturation)
{
#ifdef OS_ANYKA

    if (saturation > CAMERA_SATURATION_NUM-1)
        saturation = CAMERA_SATURATION_NUM-1;

    cam_set_feature(CAM_FEATURE_SATURATION, saturation);
    return AK_TRUE;
#else
    return AK_TRUE;
#endif
}


T_BOOL Fwl_CameraContrastCanDec(T_U8 contrast)
{
	if(contrast > CAMERA_CONTRAST_1)
		return AK_TRUE;

	return AK_FALSE;	
}


T_BOOL Fwl_CameraContrastCanInc(T_U8 contrast)
{
	if(contrast < CAMERA_CONTRAST_NUM - 1)
		return AK_TRUE;

	return AK_FALSE;
}



T_BOOL Fwl_CameraChangeContrast(T_U8 contrast)
{
#ifdef OS_ANYKA

    if (contrast > CAMERA_CONTRAST_7)
        contrast = CAMERA_CONTRAST_7;

    cam_set_feature(CAM_FEATURE_CONTRAST, contrast);
    return AK_TRUE;
#else
    return AK_TRUE;
#endif
}


T_BOOL Fwl_CameraBrightnessCanDec(T_U8 brightness)
{
	if(brightness > CAMERA_BRIGHTNESS_0)
		return AK_TRUE;

	return AK_FALSE;	
}


T_BOOL Fwl_CameraBrightnessCanInc(T_U8 brightness)
{
	if(brightness < CAMERA_BRIGHTNESS_NUM - 1)
		return AK_TRUE;

	return AK_FALSE;
}


T_BOOL Fwl_CameraChangeBrightness(T_U8 brightness)
{
#ifdef OS_ANYKA
    if (brightness >= CAMERA_BRIGHTNESS_NUM)
        brightness = CAMERA_BRIGHTNESS_NUM - 1;
    else if (brightness < CAMERA_BRIGHTNESS_1)
        brightness = CAMERA_BRIGHTNESS_1;

    cam_set_feature(CAM_FEATURE_BRIGHTNESS, brightness);

    return AK_TRUE;
#else
    return AK_TRUE;
#endif
}

int Fwl_CameraSetWindows(int width, int height)
{
#ifdef OS_ANYKA
    if(width <= 0 || height <= 0 )
        return -1;
	return cam_set_window(width, height);
#else
    return 1;
#endif
}

T_VOID Fwl_CameraSetMirror(T_U8 mirror)
{
#ifdef OS_ANYKA
    if(mirror >= CAMERA_MIRROR_NUM)
        return;

    cam_set_feature(CAM_FEATURE_MIRROR, mirror);
#endif
}

T_U8*  Fwl_GetCamPreviewBuf(T_S32 yPos)
{
#ifdef OS_ANYKA
    T_U8* buf;

    buf = lcd_get_disp_buf();
    if(yPos >= 320)
    {
        Fwl_Print(C3,M_CAMERA,"------warning, camera buffer overflow------------\n");
        return buf;
    }
    return     buf+MAIN_LCD_WIDTH*3*yPos;
#else
    return AK_NULL;
#endif
    
}
/*
T_CAMERA_TYPE Fwl_GetCameraType(T_VOID)
{
#ifdef OS_AKRTOS
    return CAMERA_TYPE;
#endif
#ifdef OS_WIN32
    return CAMERA_TYPE_OV9640;
#endif
}
*/
T_BOOL FWl_CameraCaptureRGB(T_U8 *RGB, T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH, T_U32 timeout)
{
#ifdef OS_ANYKA
    if (cam_capture_RGB(RGB, dstW, dstH, timeout) != AK_FALSE)
        return AK_TRUE;
    else
        return AK_FALSE;
#else
    return AK_TRUE;
#endif
}

T_BOOL Fwl_CameraCaptureYUVNoWaitDMA(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, int dstWidth, int dstHeight, T_U32 timeout)
{
#ifdef OS_ANYKA
    //if (cam_capture_YUV_NoWaitDMA(iY, iU, iV, width, height, dstWidth, dstHeight, timeout) != AK_FALSE)
    if (cam_capture_YUV(iY, iU, iV, dstWidth, dstHeight, timeout))
        return AK_TRUE;
    else
        return AK_FALSE;
#else
    memset(iY, 0x55, dstWidth * dstHeight);
    memset(iU, 0x55, dstWidth * dstHeight / 2);
    memset(iV, 0x55, dstWidth * dstHeight / 2);
    return AK_TRUE;
#endif
}

T_BOOL Fwl_CameraCaptureYUV(T_U8 *iY, T_U8 *iU, T_U8 *iV, int dstWidth, int dstHeight, T_U32 timeout)
{
#ifdef OS_ANYKA
    if (cam_capture_YUV(iY, iU, iV, dstWidth, dstHeight, timeout))
        return AK_TRUE;
    else
        return AK_FALSE;
#else
    memset(iY, 0x55, dstWidth * dstHeight);
    memset(iU, 0x55, dstWidth * dstHeight / 2);
    memset(iV, 0x55, dstWidth * dstHeight / 2);
    return AK_TRUE;
#endif
}

T_BOOL Fwl_CameraYUV2RGB(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstRGB, \
                         T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH, T_U32 timeout)
{
#ifdef OS_ANYKA
    if (Img_YUV2RGB(srcY, srcU, srcV, dstRGB, srcW, srcH, dstW, dstH, timeout) == 0)
        return AK_TRUE;
    else
        return AK_FALSE;
#else
    return AK_TRUE;
#endif
}
/*
T_VOID Fwl_CameraCatchStillPrepare( T_CAMERA_RESOLUTION resolution )
{
#ifdef OS_AKRTOS
    camera_still_prepare( resolution );
#endif
#ifdef OS_WIN32
#endif
}

T_VOID Fwl_CameraCatchStill( unsigned char *dstY,unsigned char *dstU,unsigned char *dstV,
                    int srcWidth, int srcHeight,
                    int dstWidth, int dstHeight,
                    int timeout )
{
#ifdef OS_AKRTOS
    camera_still_catch( dstY,dstU,dstV,
                    srcWidth, srcHeight,
                    dstWidth, dstHeight,
                    timeout );
#endif
#ifdef OS_WIN32
#endif
}
T_VOID Fwl_CameraPreviewResume( T_CAMERA_RESOLUTION resolution )
{
#ifdef OS_AKRTOS
    camera_preview_resume( resolution );
#endif
#ifdef OS_WIN32
#endif
}
*/

T_U8* Fwl_CameraRecordGetData(T_VOID)
{
#ifdef OS_ANYKA
    T_CAMERA_BUFFER *YUV;

    YUV = camstream_get();

    if(YUV == AK_NULL)
        return AK_NULL;

    return YUV->dY; 
    
#else
    return AK_NULL;
#endif
}

T_BOOL Fwl_CameraCapComplete(T_VOID)
{
#ifdef OS_ANYKA
    return camstream_ready(); 
#else
    return AK_TRUE;
#endif
}

T_BOOL Fwl_CameraFlashInit(T_VOID)
{
#ifdef OS_ANYKA
    gpio_set_pin_dir( GPIO_FLASH_LIGHT1, 1 );
    gpio_set_pin_dir( GPIO_FLASH_LIGHT2, 1 );
    gpio_set_pin_level( GPIO_FLASH_LIGHT1, 0 );    
    gpio_set_pin_level( GPIO_FLASH_LIGHT2, 0 );
    return AK_TRUE;
#else
    return AK_TRUE;
#endif
}

T_VOID Fwl_CameraFlashOn(T_VOID)
{
#ifdef OS_ANYKA
    gpio_set_pin_level( GPIO_FLASH_LIGHT2, 1 );
#else

#endif
}

T_VOID Fwl_CameraFlashOff(T_VOID)
{
#ifdef OS_ANYKA
    gpio_set_pin_level( GPIO_FLASH_LIGHT2, 0 );
#else

#endif
}

T_VOID Fwl_CameraFlashClose(T_VOID)
{
#ifdef OS_ANYKA
    gpio_set_pin_level( GPIO_FLASH_LIGHT1, 0 );
    gpio_set_pin_level( GPIO_FLASH_LIGHT2, 0 );
#else

#endif
}

T_BOOL    Fwl_CameraSetToCap(T_U32 width, T_U32 height)
{
#ifdef OS_ANYKA
    return cam_set_to_cap(width, height);
#else
    return AK_TRUE;
#endif
}

T_BOOL    Fwl_CameraSetToPrev(T_U32 width, T_U32 height)
{
#ifdef OS_ANYKA
    return cam_set_to_prev(width, height);
#else
    return AK_TRUE;
#endif
}

T_BOOL  Fwl_CameraSetToRec(T_U32 width, T_U32 height)
{
#ifdef OS_ANYKA
	return cam_set_to_record(width, height);
#else
	return AK_TRUE;
#endif
}

T_U8 Fwl_CameraGetType(T_VOID)
{
#if (defined(OS_ANYKA))
	return cam_get_type();
#else
	return CAMERA_P3M;
#endif
}

T_BOOL Fwl_CameraIsSupportHd(T_VOID)
{
#ifdef OS_ANYKA
    return (T_BOOL)(Fwl_CameraGetType() >= CAMERA_2M);
#else
    return AK_TRUE;
#endif
}

#endif
