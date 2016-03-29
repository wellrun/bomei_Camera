#ifndef __MOUNT_PUB_API_H__
#define __MOUNT_PUB_API_H__
#include "anyka_types.h"


typedef struct
{
	T_U8 driverCnt;
	T_U8 firstDrvNo;
	T_VOID *handle;
} T_PARTITION_INFO, *TP_PARTITION_INFO;


#define Ram_Malloc(size)	Ram_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)(__LINE__)))
#define Ram_Release(var)	Ram_FreeAndTrace(((T_pVOID)(var)), ((T_S8*)(__FILE__)), ((T_U32)(__LINE__)))

#define PTR_ASSIGN_VAL(ptr,val) {\
	if (AK_NULL != (ptr))\
	{\
		(*ptr) = (val);\
	}\
}

#if 1
#define   TRD_MUTEX_INIT(sem) {\
	(sem) = AK_Create_Semaphore(1, AK_PRIORITY);\
}

#define   TRD_MUTEX_DEINIT(sem) {\
	if (AK_INVALID_SEMAPHORE != (sem))\
	{\
		AK_Delete_Semaphore((sem));\
		sem = AK_INVALID_SEMAPHORE;\
	}\
}
#define   TRD_LOCK(sem)  {\
	if (AK_INVALID_SEMAPHORE != (sem))\
	{\
		AK_Obtain_Semaphore((sem), AK_SUSPEND);\
	}\
}

#define   TRD_UNLOCK(sem) {\
	if (AK_INVALID_SEMAPHORE != (sem))\
	{\
		AK_Release_Semaphore((sem));\
	}\
}

#else
#define   TRD_MUTEX_INIT(sem) 
#define   TRD_MUTEX_DEINIT(sem) 
#define   TRD_LOCK(sem)  
#define   TRD_UNLOCK(sem) 
#endif

#define TRD_WAIT(m)   AK_Sleep(m)
#define TRD_DELAY(m)  mini_delay(m)

T_pVOID Ram_MallocAndTrace(T_U32 size, T_pSTR filename, T_U32 line);
T_pVOID Ram_FreeAndTrace(T_pVOID var, T_pSTR filename, T_U32 line) ;
T_VOID mnt_partInfo_Int(T_PARTITION_INFO partInfo);

#endif
