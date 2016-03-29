#include "eng_callback.h"
#include "Fwl_osMalloc.h"
#include "eng_debug.h"
#include "lib_image_api.h"
#include "mem_api.h"
#include "Eng_dataconvert.h"
#include "akos_api.h"
#include "eng_time.h"
#include "Lib_thumbsdb.h"
#include "l2_cache.h"
#include "arch_mmu.h"
#include "arch_interrupt.h"

static T_S32 ftell_cb(T_hFILE fileID)
{
    return Fwl_FileSeek(fileID, 0, _FSEEK_CUR);
}

T_S32 Fwl_PrintfNull(T_pCTSTR s, ...)
{
    return 0;
}

#if 0
static T_S32 SD_Print(T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    va_start(args, s);
    len = Fwl_VPrint(C3, "SOUNDLIB:", s, args);
    va_end(args); 
    
    return len;
}
#endif

static T_S32 IMG_Print(T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    va_start(args, s);
    len = Fwl_VPrint(C3, M_IMG_LIB, s, args);
    va_end(args); 
    
    return len;
}

//对phyAddr寄存器中mask中为1的位进行赋值
T_BOOL RegBitsWriteCB(T_pVOID phyAddr, T_U32 val, T_U32 mask)
{
#ifdef OS_ANYKA
	store_all_int();
#endif

	*(volatile T_U32 *)(phyAddr) &= ~mask;
	*(volatile T_U32 *)(phyAddr) |= mask&val;	

#ifdef OS_ANYKA
	restore_all_int();
#endif

	return AK_TRUE;
}

/*const _SD_CB_FUNS gb_lpsndCBFuncs = {
       (_SD_CALLBACK_FUN_FREAD)Fwl_FileRead,
       (_SD_CALLBACK_FUN_FWRITE)Fwl_FileWrite,
       (_SD_CALLBACK_FUN_FSEEK)Fwl_FileSeek,
       (_SD_CALLBACK_FUN_FGETLEN)Fwl_GetFileLen,
       (_SD_CALLBACK_FUN_FTELL)ftell_cb,
       (_SD_CALLBACK_FUN_LOADRESOURCE)SdLoadResource,
       (_SD_CALLBACK_FUN_RELEASERESOURCE)SdReleaseResource,
       (_SD_CALLBACK_FUN_MALLOC)Fwl_MallocAndTrace,
       (_SD_CALLBACK_FUN_FREE)Fwl_FreeAndTrace,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       (_SD_CALLBACK_FUN_PRINTF)SD_Print, //gb_PrintfNull,
       (_SD_CALLBACK_FUN_MBSTOWCS)Eng_WcsToMbs,
       (_SD_CALLBACK_FUN_WCSTOMBS)Eng_MbsToWcs,
}; */

const CB_FUNS gb_lpimgCBFuncs = 
{
       Fwl_FileRead,
       Fwl_FileWrite,
       Fwl_FileSeek,
       Fwl_GetFileLen,
       ftell_cb,
       AK_NULL,
       AK_NULL,
       Fwl_MallocAndTrace,
       (CALLBACK_FUN_FREE)Fwl_FreeAndTrace,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       AK_NULL,
       IMG_Print,//gb_PrintfNull,
       RegBitsWriteCB,
};

/*
static T_VOID Cache_FlushFunc(T_VOID)
{
    MMU_Clean_Invalidate_Dcache();	
}
*/

#ifdef SUPPORT_IMG_BROWSE

const ThumbsDB_CBFuns gb_thumbsCBFuncs =
{
	Fwl_MallocAndTrace,
    Fwl_FreeAndTrace,
    Fwl_FileOpen,
    Fwl_FileClose,
    Fwl_FileRead,
    Fwl_FileWrite,
    Fwl_FileSeek,
    Fwl_GetFileLen,
    IMG_Print,
};
#endif

T_VOID Gbl_SetCallbackFuncs(T_VOID)
{
    //sound
#ifdef OS_ANYKA
//    _AKSD_SetCallbackFuns(&gb_lpsndCBFuncs);
#endif
/*
    //image
    Img_SetCallbackFuns(&gb_lpimgCBFuncs);
#ifdef OS_ANYKA
    Img_SetFlushCacheFunc(Cache_FlushFunc);
#endif
*/
#ifdef SUPPORT_IMG_BROWSE

    ThumbsDB_SetCallbackFuns(&gb_thumbsCBFuncs);
#endif
}

