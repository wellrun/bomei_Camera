#include "mount_pub_api.h"

#include 	"mem_api.h"
#include 	"raminit.h"

extern T_U32	Res_DynamicRelease(T_U32 size);
//***********************************************************************************
T_pVOID Ram_MallocAndTrace(T_U32 size, T_pSTR filename, T_U32 line)
{
    T_pVOID ptr;
    ptr = Ram_Alloc(Ram_GetGlobalMem(), size, filename, line);

    if(AK_NULL == ptr)
    {
        Res_DynamicRelease(Ram_GetRamBufferSize());
        ptr = Ram_Alloc(Ram_GetGlobalMem(), size, filename, line);
    }

    return ptr;
}

//***********************************************************************************
T_pVOID Ram_FreeAndTrace(T_pVOID var, T_pSTR filename, T_U32 line) 
{
    return Ram_Free(Ram_GetGlobalMem(), var, filename, line); //一些野指针释放时需要调试信息
}  


T_VOID mnt_partInfo_Int(T_PARTITION_INFO partInfo)
{
	if (AK_NULL != partInfo.handle)
	{
		Ram_Release(partInfo.handle);
		partInfo.handle = AK_NULL;
	}
	partInfo.driverCnt = 0;
	partInfo.firstDrvNo = 0;
}

//*****************nandflash mount********************************//

