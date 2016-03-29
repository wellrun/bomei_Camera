/**
* @FILENAME   Lib_geshade.h
* @BRIEF        Graphic Effects
* Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR 
* @DATE         2008-11-20
* @VERSION    1.0
* @REF            Transplanting from Jupiter, which is a cell phone platform
*/

#ifndef  __GESHADE_H__
#define  __GESHADE_H__

#include "lib_geapi.h"

typedef enum
{
	NORMAL_SHADE = 0,
	EFFECT_SHADE
}GE_SHADE_SELECT;

typedef struct STR_T_MENUSHOW
{
	// use for alpha show 
    GE_PBITMAP              pBitmapOut;
    GE_PBITMAP              pBitmapPre;
} T_MENUSHOW;

typedef struct STR_T_SHADEMODE
{
	GE_EFFECT_MODE mode;
	GE_EFFECT_MODE_EXTRA extra;
	T_U32 direction;
}T_SHADEMODE;

/**
 * @brief   Initialize GE_SHADE
 * 
 * @author 
 * @date    2008-11-20
 * @param T_VOID
 * @return T_U32
 * @retval  GE_SUCCESS: Initialize OK
 * @retval  1: GE_SetCallback failed
 * @retval  2: GE_Initialize failed
 * @retval  3: get pre pic failed
 */
GE_RESULT GE_ShadeInit(T_VOID);

T_VOID GE_ShadeCancel(T_VOID);

T_VOID GE_StartShade(T_VOID);

T_BOOL GE_SetNormalShade(T_U32 mode);

T_BOOL GE_SetEffectShade(T_SHADEMODE mode);

T_VOID GE_ShadeFree(T_VOID);

//@brief: added by zj, for closing geshade in CMMB
//@date: 2009/7/7
T_BOOL GE_SetAniSwitchLevel(T_eAniMenuLevelType level);

T_eAniMenuLevelType GE_GetAniSwitchLevel(T_VOID);
#endif // end #ifndef  __GESHADE_H__ 
