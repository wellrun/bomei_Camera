/**
 * @file akdefine.h
 * @brief Type definition and some global definition
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-16
 * @version 1.0
 * @ref
 */



#ifndef __AK_DEFINE_H__
#define __AK_DEFINE_H__

/* Define OS */
#ifdef WIN32	/* OS_WIN32 OS_ANYKA */
	#ifndef OS_WIN32
		#define OS_WIN32
	#endif
#else
	#ifndef OS_ANYKA
		#define OS_ANYKA
	#endif
#endif

#include "anyka_types.h"
#include "anyka_bsp.h"
#include "drv_api.h"
//#include "eng_debug.h"
#include "mmi_config.h"
#include "fwl_osmalloc.h"
#include "gbl_global.h"

#endif

