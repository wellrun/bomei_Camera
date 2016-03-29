#ifndef __Eng_KeyTranslate_h_
#define __Eng_KeyTranslate_h_

#include "anyka_types.h"
#include "fwl_keyhandler.h"

 
#define Eng_TRANSLATE_SUPPORT 1  //define if support translate.  1=support , 0= not support
//translate function prototype for setting 
typedef  T_S32  (* Eng_T_fKEY_TRANSLATE )(T_MMI_KEYPAD * pKeyParam) ;


/**
 * @brief Query :key translate function 
 *
 * @author 	Liuguodong
 * @param	pKeyParam[in/out], key for translating
 * @return 	0=success, other=failed
 */ 
T_S32 Eng_KeyTranslate(T_MMI_KEYPAD * pKeyParam);

/**
 * @brief Query :set key translate function 
 *
 * @author 	Liuguodong
 * @param	pKeyTranslate[in], key translating function for setting.
 * @return 	no
 */ 
T_VOID Eng_SetKeyTranslate(Eng_T_fKEY_TRANSLATE  pKeyTranslate );

/**
 * @brief Query :set default key translating function as current translating function
 *
 * @author 	Liuguodong
 * @param	no
 * @return 	no
 */ 
T_VOID Eng_SetDefKeyTranslate(T_VOID);

/**
 * @brief Query :translating function  for standby state machine
 *
 * @author 	Liuguodong
 * @param	pKeyParam[in/out], key for translating
 * @return 	0=success, other=failed
 */ 
T_S32 Eng_StandbyTranslate(T_MMI_KEYPAD * pKeyParam);

/**
 * @brief Query :translating function  for image list  state machine
 *
 * @author 	Liuguodong
 * @param	pKeyParam[in/out], key for translating
 * @return 	0=success, other=failed
 */ 
T_S32 Eng_ImgListTranslate(T_MMI_KEYPAD * pKeyParam);

/**
 * @brief Query :translating function  for img ThumbnailView  state machine
 *
 * @author 	Liuguodong
 * @param	pKeyParam[in/out], key for translating
 * @return 	0=success, other=failed
 */ 
T_S32 Eng_ImgThumbnailViewTranslate(T_MMI_KEYPAD * pKeyParam);


#endif

