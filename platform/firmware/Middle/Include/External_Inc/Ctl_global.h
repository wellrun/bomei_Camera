/**
* @file ctl_global.h
* @brief This file is for include/define some common info for 
* all the CTRL header file
* @author ZouMai
* @date None
* @version 1.0
*/
/** @defgroup Utl_Global Utl_Global
 *	@ingroup control
 */
/*@{*/

#ifndef __UTL_GLOBAL_H__
/**
 * @def __UTL_GLOBAL_H__
 *
 */
#define __UTL_GLOBAL_H__

/**
 * @include "Gbl_Global.h"
 *
 */
#include "fwl_vme.h"
#include "Lib_event.h"

/**line feed postfix string of the text string in single line items(menu item, title etc.)*/
#define LINE_FEED_POSTFIX	_T("...")

/** @{@name Global Refresh Define
 */
 
/**refresh nothing*/
#define CTL_REFRESH_NONE	-1

/**only refresh the current item*/
#define CTL_REFRESH_FOCUS	-2

/**need entirely refresh all the content*/
#define CTL_REFRESH_CONTENT	-3

/**need entirely refresh the control*/
#define CTL_REFRESH_ALL		-4
/** @} */

/** @{@name SCROLL DELAY Define
 */
#define TEXT_SCROLL_DELAY	2
/** @} */

typedef enum{
	COUNT_STR_FROM_HEAD_TO_TRAIL = 0,
	COUNT_STR_FROM_TRAIL_TO_HEAD,
	COUNT_MAX
}T_COUNT_STRING_DIRECT;	

typedef enum{
	COMPLEMENT_STR_HEAD = 0,
	COMPLEMENT_STR_TRAIL,
	COMPLEMENT_MAX
}T_COMPLEMENT_STR;


#endif
/*@}*/

