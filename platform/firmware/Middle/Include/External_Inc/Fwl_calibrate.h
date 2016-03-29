/**
 * @FILENAME fwl_calibrate.h
 * @BRIEF    The file include touch screen calibrate point data
 * Copyright @ 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR   Li Chenjie
 * VERSION   1.0
 * @REF 
 */ 

#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#include "akdefine.h"

typedef struct
{
#if 0
    T_S32 An;                /* A = An/Divider */
    T_S32 Bn;                /* B = Bn/Divider */
    T_S32 Cn;                /* C = Cn/Divider */
    T_S32 Dn;                /* D = Dn/Divider */
    T_S32 En;                /* E = En/Divider */
    T_S32 Fn;                /* F = Fn/Divider */
    T_S32 Divider;
#else
    /* x[0] = Divider,
           X[1] = An, y[1] = Dn,
           X[2] = Bn, y[2] = En,
           X[3] = Cn, y[3] = Fn,           
    */
    T_S32 X[4];
    T_S32 Y[4];
#endif    
}T_MATRIX;

#if 0
extern  T_U16 ADC_To_LCDX[];
extern  T_U16 ADC_To_LCDY[];
extern  T_U8 ADC_To_WTX[];
extern  T_U8 ADC_To_WTY[];
#endif

typedef struct
{
    T_U32 div;
    T_U32 res;
    T_U32 base;
    T_U32 max;
}T_CALIBRATE_PARAMETER;

T_U8 setCalibrationMatrix(const T_pPOINT displayPtr, const T_pPOINT screenPtr, 
                          T_MATRIX *matrixPtr);

T_U8 getDisplayPoint_point(T_pPOINT displayPtr, const T_pPOINT screenPtr, 
                     const T_MATRIX *matrixPtr);


#endif  /* end of __CALIBRATE_H__ */

