/********************************************************************
File:
Date:

Author:

Descrip:

*********************************************************************/

#ifndef __DISPLAY_FUNC_COMMON_H__
#define __DISPLAY_FUNC_COMMON_H__  1

#include <Akdefine.h>
#include <Fwl_osMalloc.h>
#include <Eng_Debug.h>
#include <Akos_api.h>


#define		MALLOC		Fwl_Malloc
#define		FREE		Fwl_Free
#define		PRINTF		AkDebugOutput
#define		CREATESEM	AK_Create_Semaphore
#define		DELETESEM	AK_Delete_Semaphore
#define		OBTAINSEM	AK_Obtain_Semaphore
#define		RELEASESEM	AK_Release_Semaphore

#endif /*__DISPLAY_FUNC_COMMON_H__*/