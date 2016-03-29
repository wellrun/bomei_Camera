/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKCommon.h
* Function: 
* Author: Eric
* Date: 2007-05-31
* Version: 1.0
*
***************************************************************************/
#ifndef __AKCOMPONENT_H__
#define __AKCOMPONENT_H__

#include "anyka_types.h"
#include "AKError.h"
#include "AKInterface.h"
//#include "AKClassID.h"
#include "Fwl_osMalloc.h"

#define AK_BREAKIF(b, r, e) \
        if (b) \
        {\
            r = e; \
            break; \
        }


#endif //__AKCOMMON_H__

