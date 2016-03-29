#ifndef _FWL_GUI_H_
#define _FWL_GUI_H_

#include "Anyka_types.h"
#include "arch_gui.h"


T_S32 Gui_DeleteLock(T_VOID);

T_U8 Fwl_ScaleConvert(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab);

T_U8 Fwl_ScaleConvertNoBlock(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab);

T_U8 Fwl_ScaleConvertEx(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_BOOL alphaEn, T_U8 alpha, T_BOOL colorTransEn, T_U32 color);

T_U8 Fwl_ScaleConvertNoBlockEx(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_BOOL alphaEn, T_U8 alpha, T_BOOL colorTransEn, T_U32 color);

T_U8 Fwl_ScaleConvert_Sw(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab);

T_BOOL Fwl_ScaleParamCheck(T_RECT *src, T_RECT *dest, T_U32 srcW, T_U32 srcH);

#endif	// _FWL_GUI_H_
