/**
 * @file arch_interrupt.h
 * @brief This file describe how to control the AK3223M interrupt issues.
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
 */
#ifndef __ARCH_INTERRUPT_H__
#define __ARCH_INTERRUPT_H__


/** @defgroup Interrupt Interrupt group
 *	@ingroup Drv_Lib
 */
/*@{*/

#include "anyka_types.h"

typedef T_U16 (*T_AUDIO_INTR_HANDLER)(T_VOID);

/**
 * @brief register audio interrupt handler
 *
 * @author 
 * @date 2006-01-16
 * @param[in] handler audio interrupt handler
 * @return T_BOOL
 * @retval AK_FALSE register failed
 * @retval AK_TURE register successful
 */
T_BOOL audio_register_interrupt(T_AUDIO_INTR_HANDLER handler);

/**
 * @brief Store the current interrupt mask register value, then mask all interrupt.
 *
 * @author 
 * @date 2006-01-16
 * @return T_VOID
*/
T_VOID store_all_int(T_VOID);

/**
 * @brief Recover the previous interrupt mask register value.
 *
 * @author 
 * @date 2006-01-16
 * @return T_VOID
 */
T_VOID restore_all_int(T_VOID);

/**
 * @brief mask all interrupt except specified ones.
 *
 * @author 
 * @date 2006-01-16
 * @param[in] umask_bits interrupt bits unexpected to be masked
 * @return VOID
 */
T_VOID store_all_int_ex(T_U32 umask_bits);

/**
 * @brief mask specified interrupt bit
 *
 * @author 
 * @date 2006-01-16
 * @param[in] mask_bits interrupt bits expected to be masked
 * @return VOID
 */
T_VOID store_one_int(T_U32 mask_bits);

/*@}*/

#endif

