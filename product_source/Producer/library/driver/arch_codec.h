/**@file Arch_codec.h
 * @brief list driver library jpeg codec function
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2011-10-17
 * @version 1.0
 * @note refer to ANYKA chip technical manual.
 */
#ifndef __ARCH_CODEC_H__
#define __ARCH_CODEC_H__

#include "anyka_types.h"

/** @defgroup Codec Codec group
 *  @ingroup Drv_Lib
 */
/*@{*/

/**
* @brief wait jpeg codec finish
* @author liao_zhijun
* @date 2010-10-17
* @return T_VOID
*/
T_BOOL codec_wait_finish(T_VOID);


/**
* @brief enable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return T_BOOL
*/
T_BOOL codec_intr_enable(T_VOID);

/**
* @brief disable jpeg codec interrupt
* @author liao_zhijun
* @date 2010-10-17
* @return T_VOID
*/
T_VOID codec_intr_disable(T_VOID);


/*@}*/
#endif //__ARCH_INIT_H__

