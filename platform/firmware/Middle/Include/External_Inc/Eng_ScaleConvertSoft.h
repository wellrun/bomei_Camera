#ifndef __GUI_SOFT_H__
#define __GUI_SOFT_H__

#include "Anyka_types.h"
#include "Arch_gui.h"

T_U8 Scale_ConvertSoft(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8 luminance_enabled, T_U8* luminance_table);

T_U8 Scale_Convert2Soft(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color);

#endif

