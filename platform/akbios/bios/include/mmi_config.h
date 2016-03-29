/**
 * @file
 * @brief ANYKA software
 * 
 */

#ifndef __MMI_CONFIG_H__
/**
 * @def __MMI_CONFIG_H__
 *
 */
#define __MMI_CONFIG_H__

//#include "mmi_ui.h"

#ifndef WIN32
#include "platform_hd_config.h"
//#include "platform_sw_config.h"
#else
#include "win32_config.h"
#endif

#endif
