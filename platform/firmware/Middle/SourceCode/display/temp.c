#include "akdefine.h"
#include "gpio_config.h"
#include "arch_uart.h"
#include "hal_usb_h_disk.h"
#include "hal_usb_s_UVC.h"
#include "arch_gui.h"
#include "fwl_display.h"
#include "Eng_Debug.h"



T_S32 Img_BitBlt(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat)
{
	T_U8 ret=0;
	E_ImageFormat format_in;	

	switch(srcFormat)
	{
		case RGB565:
			format_in = FORMAT_RGB565;
			break;		
		default:
			Fwl_Print(C3, M_DISPLAY,"Img_BitBlt srcFormat error\n");
			return AK_FALSE;			
	}

#if 0
	ret = Scale_Convert(&srcBuf, 1, srcWidth, 0, 0 , srcWidth,
                     srcHeight, format_in, 
                     &dstBuf, 1, dstWidth, dstPosX, dstPosY, scaleWidth, 
                     scaleHeight, format_in);
#endif
	if(0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY,"Img_BitBlt Scale_Convert return error code %d\n", ret);		
		return AK_FALSE;
	}

	return AK_TRUE;
	
}



T_S32 Img_BitBltYUV(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcY, T_VOID *srcU, T_VOID *srcV, T_U16 srcWidth, T_U16 srcHeight)
{
	T_U8 ret=0;
	E_ImageFormat format_in;
	T_VOID *AddrYUV[3] ;

	AddrYUV[0] = srcY;
	AddrYUV[1] = srcU;
	AddrYUV[2] = srcV;
	

	format_in = FORMAT_YUV420;

#if 0
	ret = Scale_Convert(&AddrYUV, 3, srcWidth, 0, 0 , srcWidth,
					 srcHeight, format_in, 
					 &dstBuf, 1, dstWidth, dstPosX, dstPosY, scaleWidth, 
					 scaleHeight, FORMAT_RGB565);
#endif
	if(0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY,"Img_BitBlt Scale_Convert return error code %d\n", ret);		
		return AK_FALSE;
	}

	return AK_TRUE;
	
}














