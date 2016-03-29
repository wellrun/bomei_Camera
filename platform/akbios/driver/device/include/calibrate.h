/**
 * @FILENAME calibrate.h
 * @BRIEF    The file include touch screen calibrate point data
 * Copyright @ 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @AUTHOR   Li Chenjie
 * VERSION   1.0
 * @REF 
 */ 

#ifndef __CALIBRATE_H__
#define __CALIBRATE_H__

#include "akdefine.h"
//


typedef struct
{
#if 0
	T_S32 An;				/* A = An/Divider */
	T_S32 Bn;				/* B = Bn/Divider */
	T_S32 Cn;				/* C = Cn/Divider */
	T_S32 Dn;				/* D = Dn/Divider */
	T_S32 En;				/* E = En/Divider */
	T_S32 Fn;				/* F = Fn/Divider */
	T_S32 Divider;
#else
	T_S32 X[4];
	T_S32 Y[4];
#endif	
}T_MATRIX;

extern  T_U16 ADC_To_LCDX[];
extern  T_U16 ADC_To_LCDY[];
extern  T_U8 ADC_To_WTX[];
extern  T_U8 ADC_To_WTY[];

typedef struct
{
	T_U32 div;
	T_U32 res;
	T_U32 base;
	T_U32 max;
}T_CALIBRATE_PARAMETER;

T_U8 setCalibrationMatrix(const T_pTSPOINT displayPtr, const T_pTSPOINT screenPtr, 
                          T_MATRIX *matrixPtr);

T_U8 getDisplayPoint(T_pTSPOINT displayPtr, const T_pTSPOINT screenPtr, 
                     const T_MATRIX *matrixPtr);


#endif  /* end of __CALIBRATE_H__ */

