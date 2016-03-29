/************************************************************************************
* Copyright(c) 2006 Anyka.com
* All rights reserved.
*
* File	:	mem_api.h
* Brief :	heap memory allocator
*           
* 
* Version : 1.0
* Author  : ZhangMuJun
* Modify  : 
* Data    : 2006-06-24
*************************************************************************************/
#ifndef __MEM_API_H__
#define __MEM_API_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "anyka_types.h"




/* ********************************* Common ******************************************* */

#define MEMLIB_VERSION	("2.0.1")


typedef T_S32 	(*Ram_PrintfTraceFun)(T_pCSTR format, ...); 
	

/* ********************************* 全局内存堆分配器 ********************************* */

/**
 * @brief  Global heap memory allocator handler
 */
typedef T_pVOID		T_GLOBALMEMHandle;


/**
 * @brief  Initialize Global heap memory allocator
 * @brief  该内存库可以实例化
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_U8  *ptr :	global heap memory start adress
 * @param	T_U32 size :	global heap memory length
 * @param	T_U16 llenpad : 前边界设置长度 (用于检测内存前越界) (默认为0)
 * @param	T_U16 rlenpad : 后边界设置长度 (用于检测内存后越界) (默认为0)
 * @param	T_U8  lvar :	前边界预填充值 (用于检测内存前越界)	(默认为0x27)
 * @param	T_U8  rvar :	后边界预填充值 (用于检测内存后越界)	(默认为0x75)
 * @param	T_U8 align :	内存对齐基数   (8X ==块单位大小)	(默认为16)
 * @param	T_U32 lenfree : 释放阵列的长度 (32X)				(默认为6400) [SIZE=6400*16/1024=100k : SIZE范围内从左到右分配, SIZE范围外从右到左分配]
 * @param	T_U8  sos :		最大大内存组合的可能所占总内存比例  (默认为50) [用于自适应调节搜索速度和碎片利用度 : 非强制因素]
 * @param   T_U8 split :    泡泡产生分裂的最小块间隔			(默认为4) [用于减少小碎片的数目]
 * @param   Ram_PrintfTraceFun lpPrintf : 是否打印信息			(默认为AK_NULL, 不打印)
 *
 * @return  ram handle for success;else for AK_NULL
 *
 */
T_GLOBALMEMHandle	Ram_Initial(T_U8 *ptr, T_U32 size);

T_GLOBALMEMHandle	Ram_InitialEx(T_U8 *ptr, T_U32 size, T_U16 llenpad, T_U16 rlenpad, T_U8 lvar, T_U8 rvar, T_U8 align, T_U32 lenfree, T_U8 sos, T_U8 split, Ram_PrintfTraceFun lpPrintf);


/**
 * @brief  Destroy Global heap memory allocator
 * @brief  该内存库可以实例化
 * @
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 *
 * @return  AK_NULL
 *
 */
T_GLOBALMEMHandle	Ram_Exit(T_GLOBALMEMHandle hMemory);


/**
 * @brief  Malloc one memory block from global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_U32 size : want memory size 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : memory address for success, AK_NULL for failure 
 *
 */
T_pVOID	Ram_Alloc(T_GLOBALMEMHandle hMemory, T_U32 size, T_S8 *filename, T_U32 fileline);

/**
 * @brief  Remalloc one memory block from global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param   T_pVOID var : old memory address
 * @param	T_U32 size : want memory size 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : new memory address for success, AK_NULL for failure 
 *
 */
T_pVOID	Ram_Realloc(T_GLOBALMEMHandle hMemory, T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline); 

/**
 * @brief  Free one memory block to global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_pVOID var : memory start address alloced before 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_pVOID Ram_Free(T_GLOBALMEMHandle hMemory, T_pVOID var, T_S8 *filename, T_U32 fileline);


/**
 * @brief  Debug memory relation info
 * @
 * @memory block map
 */
typedef struct tagT_MEMORY_TRACE
{
	T_pVOID		addr;		//memory ponter
	T_S32		size;		//memory size
	T_U32		line;		//memory alloc location line
	T_S8		*filename;	//memory alloc location file
	T_U8		old;		//used or not
}T_MEMORY_TRACE, *T_lpMEMORY_TRACE;

/**
 * @brief  Debug memory relation info
 * @
 * @memory map callback func define 
 */
typedef T_VOID	(*Ram_EnumMemTraceFun)(T_GLOBALMEMHandle hMemory, const T_MEMORY_TRACE *map, T_pVOID attatch);



/**
 * @brief  Enumerate memory statck status info
 * @只能用于查询内存状态和检测显式内存泄漏情况
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	Ram_EnumMemTraceFun enumFun : user define memory map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_VOID Ram_EnumMemTraceInfo(T_GLOBALMEMHandle hMemory, Ram_EnumMemTraceFun enumFun, T_pVOID attach);


/**
 * @brief  Debug memory heat info
 * @
 * @memory block map
 */
typedef struct tagT_MEMORY_STATUS
{
	T_U32		line;		//memory alloc location line
	T_S8		*filename;	//memory alloc location file
	T_U32		hits;		//location handler alloc times : free will --
	T_U32		ratio;		//location handler ratio of all (本Handler所分配的总内存/所有已经分配的总内存)*100
}T_MEMORY_STATUS, *T_lpMEMORY_STATUS;

/**
 * @brief  Debug memory heat info
 * @
 * @memory map callback func define 
 */
typedef T_VOID	(*Ram_EnumMemStatusFun)(T_GLOBALMEMHandle hMemory, const T_MEMORY_STATUS *map, T_pVOID attatch);

/**
 * @brief  Enumerate memory statck status info
 * @只能用于查询内存热点状态和检测隐式内存泄漏情况
 * @可以为进一步优化提供依据
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	Ram_EnumMemStatusFun enumFun : user define memory map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_VOID Ram_EnumMemStatusInfo(T_GLOBALMEMHandle hMemory, Ram_EnumMemStatusFun enumFun, T_pVOID attach);


/**
 * @brief  Debug memory beyond info
 * @
 * @memory block map
 */
typedef enum tagT_BEYOND_TYPE
{
	BEYOND_OK		= 0x000,	//ptr无越界
	BEYOND_PREV		= 0x101,	//ptr本身发生前向越界
	BEYOND_BACK		= 0x110,	//ptr本身发生后向越界
	BEYOND_BOTH		= 0x111,	//ptr本身发生全越界
	BEYOND_SUFFER	= 0x201,	//ptr被前驱节点越界 
	BEYOND_INVADE	= 0x202,	//ptr越界到后驱节点
	BEYOND_ERR					//UnKnwon error
}T_BEYOND_TYPE;

typedef struct tagT_MEMORY_BEYOND_TRACE
{
	T_pVOID		addr;		//memory ponter
	T_S32		size;		//memory size
	T_U32		line;		//memory alloc location line
	T_S8		*filename;	//memory alloc location file
	T_U8		old;		//used or not
	T_BEYOND_TYPE type;		//beyond type
}T_MEMORY_BEYOND_TRACE, *T_lpMEMORY_BEYOND_TRACE;

/**
 * @brief  Enumerate memory statck status info
 * @只能用于检测内存越界情况
 * @从堆内存头开始检测,检测到第一个越界即退出
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_MEMORY_BEYOND_TRACE *beyondInfo : beyond info loader
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
T_BOOL Ram_CheckBeyond(T_GLOBALMEMHandle hMemory, T_MEMORY_BEYOND_TRACE *beyondInfo);   

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存越界情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_BEYOND_TYPE *beyondType : beyond type
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
T_BOOL Ram_CheckBeyondPtr(T_GLOBALMEMHandle hMemory, T_VOID *ptr, T_BEYOND_TYPE *beyondType);

/**
 * @brief  Debug memory wild info
 * @
 * @memory block map
 */
typedef enum tagT_WILD_TYPE
{
	WILD_OK			= 0,	//ptr是有效指针
	WILD_WILD		= 1,	//ptr是野指针
	WILD_OUTRANGE	= 2,	//ptr超出了堆内存地址范围
	WILD_NOTSTART	= 3,	//ptr不是内存头指针
	WILD_ERR				//UnKnwon error
}T_WILD_TYPE;

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存合法情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_WILD_TYPE *wildInfo : wild info loader
 *
 * @return  AK_TRUE for no wild ;else for AK_FALSE
 *
 */
T_BOOL Ram_CheckPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_WILD_TYPE *wildType); 


/**
 * @brief  过程中自动化内存泄漏检测器
 * @
 * @memory auto callback func define 
 */
typedef T_VOID	(*Ram_EnterStateMachine)(T_GLOBALMEMHandle hMemory);
typedef T_VOID	(*Ram_LeaveStateMachine)(T_GLOBALMEMHandle hMemory);

/**
 * @brief  要检测代码段的内存泄漏状况 : 开始位置
 * @请保持配对使用
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 */
T_VOID  RAM_ENTER_LEAKCHK_SECTION(T_GLOBALMEMHandle hMemory);


/**
 * @brief  要检测代码段的内存泄漏状况 : 结束位置
 * @请保持配对使用
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 */
T_VOID	RAM_LEAVE_LEAKCHK_SECTION(T_GLOBALMEMHandle hMemory);



/**
 * @brief  enable memory leak auto trace mechnics
 * @只能用于状态机间内存泄漏自动检测 
 * @有2000内存块的限制,有20个状态机栈层的限制
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	Ram_EnumMemTraceFun enumFun : callback printf func
 * @param   Ram_EnterStateMachine *enterSection : GET HOOK for enter critical leakchk callback function
 * @param   Ram_LeaveStateMachine *leaveSection : GET HOOK for leave critical leakchk callback function
 *
 * @return  T_VOID
 *
 * @return  AK_TRUE for auto ok else return false
 *
 */
T_BOOL Ram_EnableAutoLeakTrace(T_GLOBALMEMHandle hMemory, Ram_EnumMemTraceFun enumFun, Ram_EnterStateMachine *enterSection, Ram_LeaveStateMachine *leaveSection);

/**
 * @brief  disable memory leak auto trace mechnics
 * @只能用于状态机间内存泄漏自动检测
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 *
 * @return  T_VOID
 *
 */
T_VOID Ram_DisableAutoLeakTrace(T_GLOBALMEMHandle hMemory);

/**
 * @brief  Memory Information
 * @
 * @memory 
 */
typedef struct tagGLOBALMEMInfo
{
	T_U16	align;			//内存对齐基数
	T_U32	szTotal;		//全局堆内存的总大小
	T_U32	szBlocks;		//全局堆内存的总块个数
	T_U32   szUsed;			//当前使用的内存的总大小
	T_U32	blkUsed;		//当前使用的内存的总块数
}GLOBALMEMInfo;	

/**
 * @brief  get global memory info
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	GLOBALMEMInfo *info : mem info
 *
 * @return  AK_FALSE for not initialize memory else return AK_TRUE
 *
 */
T_BOOL Ram_GetRamInfo(T_GLOBALMEMHandle hMemory, GLOBALMEMInfo *info);

/**
 * @brief  get memory info
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	GLOBALMEMInfo *info : mem info
 *
 * @return  AK_FALSE for not initialize memory else return AK_TRUE
 *
 */
T_BOOL Ram_GetPtrInfo(T_GLOBALMEMHandle hMemory, T_pVOID var, T_MEMORY_TRACE *map);



/* ********************************* 局部内存堆分配器 ********************************* */

/**
 * @brief  Local heap memory allocator handler
 */
typedef T_pVOID		T_LOCALMEMHandle;

/**
 * @brief  Initialize Local heap memory allocator
 * @brief  该内存库可以实例化
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_pVOID membody : external largebuf start address 
 * @param	T_U32 maxmemsize: external largebuf size (if membody is AK_NULL will drive malloc largebuf from global heap memory)
 * @param	T_U16 llenpad : 前边界设置长度 (用于检测内存前越界) (默认为0)
 * @param	T_U16 rlenpad : 后边界设置长度 (用于检测内存后越界) (默认为0)
 * @param	T_U8  lvar :	前边界预填充值 (用于检测内存前越界) (默认为0x27)
 * @param	T_U8  rvar :	后边界预填充值 (用于检测内存后越界) (默认为0x75)
 * @param	T_U8 align :	内存对齐基数   (16X)				(默认为16)
 * @param	T_U32 lenfree : 释放阵列的长度 						(默认为512) [SIZE=512*16/1024=8k : SIZE范围内从左到右分配, SIZE范围外从右到左分配]
 *
 * @return  T_LOCALMEMHandle for success else for AK_NULL
 *
 */
T_LOCALMEMHandle CreateBlockMem(T_pVOID membody, T_U32 maxmemsize);

T_LOCALMEMHandle CreateBlockMemEx(T_pVOID membody, T_U32 maxmemsize, T_U16 llenpad, T_U16 rlenpad, T_U8 lvar, T_U8 rvar, T_U8 align, T_U32 lenfree);


/**
 * @brief  Destroy local heap memory allocator
 * @
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_LOCALMEMHandle hBlockMem : assigned local memory allocator handler
 *
 * @return  AK_NULL
 *
 */
T_LOCALMEMHandle ReleaseBlockMem(T_LOCALMEMHandle hBlockMem);


/**
 * @brief  Malloc one memory block from local heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	T_U32 size : want memory size 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : memory address for success, AK_NULL for failure 
 *
 */
T_pVOID  Blk_Malloc(T_LOCALMEMHandle hBlockMem, T_U32 size, T_S8 *filename, T_U32 fileline);


/**
 * @brief  Free one memory block from local heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	T_pVOID ptr : memory start address alloced before
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : memory address for success, AK_NULL for failure 
 *
 */
T_pVOID  Blk_Free(T_LOCALMEMHandle hBlockMem, T_pVOID ptr, T_pSTR filename, T_U16 line);

/**
 * @brief  Enumerate memory statck status info
 * @只能用于查询内存状态和检测显式内存泄漏情况
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	Ram_EnumMemTraceFun enumFun : user define memory map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_VOID Blk_EnumMemTraceInfo(T_LOCALMEMHandle hBlockMem, Ram_EnumMemTraceFun enumFun, T_pVOID attach);

/**
 * @brief  Enumerate memory statck status info
 * @只能用于查询内存热点状态和检测隐式内存泄漏情况
 * @可以为进一步优化提供依据
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	Ram_EnumMemStatusFun enumFun : user define memory map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_VOID
 *
 */
T_VOID Blk_EnumMemStatusInfo(T_LOCALMEMHandle hBlockMem, Ram_EnumMemStatusFun enumFun, T_pVOID attach);

/**
 * @brief  Enumerate memory statck status info
 * @只能用于检测内存越界情况
 * @从堆内存头开始检测,检测到第一个越界即退出
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	T_MEMORY_BEYOND_TRACE *beyondInfo : beyond info loader
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
T_BOOL Blk_CheckBeyond(T_LOCALMEMHandle hBlockMem, T_MEMORY_BEYOND_TRACE *beyondInfo);   

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存越界情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	T_MEMORY_BEYOND_TRACE *beyondInfo : beyond info loader
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
T_BOOL Blk_CheckBeyondPtr(T_LOCALMEMHandle hBlockMem, T_VOID *ptr, T_BEYOND_TYPE *beyondType);

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存合法情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_LOCALMEMHandle hBlockMem : assigned local allocator handler
 * @param	T_WILD_TYPE *wildInfo : wild info loader
 *
 * @return  AK_TRUE for no wild ;else for AK_FALSE
 *
 */
T_BOOL Blk_CheckPtr(T_LOCALMEMHandle hBlockMem, T_VOID *ptr, T_WILD_TYPE *wildType); 









#ifdef __cplusplus
}
#endif


#endif






