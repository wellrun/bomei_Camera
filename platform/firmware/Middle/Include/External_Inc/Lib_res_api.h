/************************************************************************************
* Copyright(c) 2006 Anyka.com
* All rights reserved.
*
* File	:	dyn_res.h
* Brief :	MMI resource dynamic loading
*           
* 
* Version : 1.0
* Author  : ZhangMuJun
* Modify  : 
* Data    : 2006-06-24
*************************************************************************************/
#ifndef __DYN_RES_H__
#define __DYN_RES_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "Fwl_osfs.h"

extern void *          GetHdrResourceEleInfo(const T_WCHR* pthFile);
extern unsigned char   GetAnyResourceEleInfo(void* hdrInfo, unsigned long resID, unsigned long *startPos, unsigned long *szData);
extern unsigned long   GetAnyResourceEleData(void* hdrInfo, unsigned char* mem, unsigned long startPos, unsigned long szData);



#define DYNAMICLOAD_VERSION	("0.0.3")


typedef T_S32 	(*Res_PrintfTraceFun)(T_pCSTR format, ...); 

typedef struct 
{
    T_U16		ResourceID;			
    T_U8		*Buff;         //The PoT_S32er to save the resource. Application should alloc the memory for it
    T_U32		Resource_len;  //if Buff is NULL, and Resource_len is 0, a failure have occurred   
} T_RES_LOADRESOURCE_CB_PARA;


//function pointer type define  

typedef T_S32 	(*T_RES_CALLBACK_FUN_FREAD)(T_hFILE hFile, T_pVOID pBuffer, T_U32 count); 
typedef T_S32 	(*T_RES_CALLBACK_FUN_FWRITE)(T_hFILE hFile, T_VOID *pBuffer, T_U32 count); 
typedef T_S32 	(*T_RES_CALLBACK_FUN_FSEEK)(T_hFILE hFile, T_S32 offset, T_U16 origin); 
typedef T_S32 	(*T_RES_CALLBACK_FUN_FGETLEN)(T_hFILE hFile); 
typedef T_S32 	(*T_RES_CALLBACK_FUN_FTELL)(T_hFILE hFile); 
typedef T_VOID  (*T_RES_CALLBACK_FUN_LOADRESOURCE)(T_RES_LOADRESOURCE_CB_PARA *pPara); 
typedef T_VOID  (*T_RES_CALLBACK_FUN_RELEASERESOURCE)(T_U8 *Buff); 
typedef T_VOID* (*T_RES_CALLBACK_FUN_MALLOC)(T_U32 size, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*T_RES_CALLBACK_FUN_FREE)(T_VOID* mem, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*T_RES_CALLBACK_FUN_REMALLOC)(T_VOID* mem, T_U32 size, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*T_RES_CALLBACK_FUN_DMAMEMCOPY)(T_VOID* dst, T_VOID* src, T_U32 count); 
typedef T_VOID 	(*T_RES_CALLBACK_FUN_SHOWFRAME)(T_VOID* srcImg, T_U32 src_width, T_U32 src_height); 
typedef T_VOID 	(*T_RES_CALLBACK_FUN_CAMERASHOWFRAME)(T_VOID* srcImg, T_U32 src_width, T_U32 src_height); 
typedef T_VOID 	(*T_RES_CALLBACK_FUN_CAPSTART)(T_VOID); 
typedef T_BOOL 	(*T_RES_CALLBACK_FUN_CAPCOMPLETE)(T_VOID); 
typedef T_VOID* (*T_RES_CALLBACK_FUN_CAPGETDATA)(T_VOID); 
typedef T_U32 	(*T_RES_CALLBACK_FUN_GETTICKCOUNT)(T_VOID); 

typedef T_S32 	(*T_RES_CALLBACK_FUN_PRT_S32F)(const char * format, ...); 



//call back functions struct keep call back function's pointers
typedef struct
{
	T_RES_CALLBACK_FUN_FREAD   				fread;
	T_RES_CALLBACK_FUN_FWRITE  				fwrite;
	T_RES_CALLBACK_FUN_FSEEK    				fseek;
	T_RES_CALLBACK_FUN_FGETLEN  				fgetlen;
	T_RES_CALLBACK_FUN_FTELL 					ftell;
	T_RES_CALLBACK_FUN_LOADRESOURCE  			LoadResource;
	T_RES_CALLBACK_FUN_RELEASERESOURCE 		ReleaseResource;
	T_RES_CALLBACK_FUN_MALLOC 				malloc;
	T_RES_CALLBACK_FUN_FREE    				free;
	T_RES_CALLBACK_FUN_REMALLOC 				remalloc;
	T_RES_CALLBACK_FUN_DMAMEMCOPY 			DMAMemcpy;
	T_RES_CALLBACK_FUN_SHOWFRAME 				ShowFrame;
	T_RES_CALLBACK_FUN_CAMERASHOWFRAME 	    CameraShowFrame;
	T_RES_CALLBACK_FUN_CAPSTART			    CapStart;
	T_RES_CALLBACK_FUN_CAPCOMPLETE			CapComplete;
	T_RES_CALLBACK_FUN_CAPGETDATA				CapGetData;
	T_RES_CALLBACK_FUN_GETTICKCOUNT			GetTickCount;
	T_RES_CALLBACK_FUN_PRT_S32F               printf;
} T_RES_CB_FUNS;


/* ********************************** Common Resource *********************************************** */

/**
 * @brief  Initialize mct resource dynamic loading handle
 *         must call Res_SetFileCallback first
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_S8 *resfilename : rsource file name
 * @param   T_U32 defStack : stack size
 *
 * @return  T_VOID
 *
 */
T_BOOL	Res_Initial(T_WCHR *resfilename, T_U32 defStack, T_RES_CB_FUNS *resCB);


/**
 * @brief  Dynamic load resource from file to memory 
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_pDATA *pAddr : return variable address
 * @param	T_BOOL bCanKill : 状态机压栈时该资源是否可以暂时销毁，当退栈时再重创建, YES for AK_TRUE, else AK_FALSE
 * @param   T_U32 id : resource id
 * @param   T_U32 *length : length variable address
 *
 * @return  T_pDATA : success for resource data else return AK_NULL
 *
 */
T_pDATA	 Res_DynamicLoad(T_pDATA *pAddr, T_BOOL bCanKill, T_U32 id, T_U32 *length);

/**
 * @brief  Static load resource from file to memory 
 *		   Ram_Free to release this resource
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_pDATA *pAddr : return variable address
 * @param   T_U32 id : resource id
 * @param   T_U32 *length : length variable address
 *
 * @return  T_pDATA : success for resource data else return AK_NULL
 *
 */
T_pDATA	 Res_StaticLoad(T_pDATA *pAddr, T_U32 id, T_U32 *length);


/**
 * @brief  Dynamic release resource from heap 
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_U32 size : needed to realease memory size
 *
 * @return  T_BOOL : AK_TRUE for release enough memory size else return AK_FALSE
 *
 */
T_U32	Res_DynamicRelease(T_U32 size);

/**
 * @brief  Dynamic release resource from heap 
 * @author  
 * @date	
 *
 * @param   
 *
 * @return  T_BOOL : AK_TRUE for current malloc is invoked from Res_DynamicLoad
 *
 */
T_BOOL Res_Nested(T_VOID);


/**
 * @brief  Release assigned resource from heap 
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_U32 id : needed to realease resource ID
 *
 * @return  T_BOOL : AK_TRUE for release else return AK_FALSE
 *
 */
T_BOOL	Res_ReleaseByID(T_U32 id);


/**
 * @brief  Debug resource relation info
 * @
 * @memory block map
 */
typedef struct tagT_RESOURCE_TRACE
{
	T_U32		id;			//resource id , 
	T_pVOID		ptr;		//resource memory ptr
	T_pVOID		varAddr;	//resource pointer var
	T_pVOID		redirect;	//redirect location
	T_U32		size;		//ptr memory size
	T_U8		stack;		//stack location
}T_RESOURCE_TRACE, *T_lpRESOURCE_TRACE;

/**
 * @brief  Debug resource relation info
 * @
 * @resource map callback func define 
 */
typedef T_VOID	(*Res_EnumMemTraceFun)(const T_RESOURCE_TRACE *map, T_pVOID attatch);


/**
 * @brief  Enumerate resource statck status info
 * @只能用于查询资源内存状态
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	Res_EnumMemTraceFun enumFun : user define resource map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_VOID Res_EnumMemTraceInfo(Res_EnumMemTraceFun enumFun, T_pVOID attach);


/**
 * @brief  Resource Information
 * @
 * @memory 
 */
typedef struct tagGLOBALRESInfo
{
	T_U32   nTotal;			//动态资源总个数
	T_U32	nUsed;			//加载资源总个数
	T_U32	szTotal;		//动态资源占用总内存大小
	T_U32	szOften;		//常驻资源占用总内存大小
}GLOBALRESInfo;	

/**
 * @brief  get global resource info
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	GLOBALRESInfo *info : res info
 *
 * @return  AK_FALSE for not initialize resource else return AK_TRUE
 *
 */
T_BOOL Res_GetResInfo(GLOBALRESInfo *info);


/**
 * @brief  过程中自动化资源加载器
 * @
 * @memory auto callback func define 
 */
typedef T_VOID	(*Res_EnterStateMachine)(T_VOID);
typedef T_VOID	(*Res_LeaveStateMachine)(T_VOID);

extern Res_EnterStateMachine	gb_resEnterStateMachine;
extern Res_LeaveStateMachine	gb_resLeaveStateMachine;


/* ********************************** Common Font Resource *********************************************** */

/**
 * @brief  Global FONT loading handler
 */
typedef T_pVOID		T_FONTHandle;


typedef enum tagANYKAFONT
{
	ANYKFONT_UNKNOWN = 0x00000000,

	GBK				 = 0x10000000,		//对齐存储的GBK字库
	GBK_TIGHT		 = 0x11000000,		//紧凑存储的GBK字库(unsupported now)
	GB2312			 = 0x20000000,		//对齐存储的GB2312字库
	GB2312_TIGHT	 = 0x21000000,		//紧凑存储的GB2312字库(unsupported now)
	ASCII			 = 0x30000000,		//对齐存储的ASCII字库
	ASCII_TIGHT		 = 0x31000000,		//紧凑存储的ASCII字库(unsupported now)

	ANYKFONT_EXTEND = 0x7FFFFFFF 
}ANYKAFONT;


/**
 * @brief  Initialize(load) FONT hashtable and preloading VIP word
 *         must call Res_init first
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	ANYKAFONT fontID : FONT字库ID : see above
 * @param   T_U32 idFtlib : BK字库的文件ID
 * @param   T_U32 idFtdeftxt: 默认预加载的部分FONT字库TXT文件ID
 * @PARAM   T_U32 idFtdeflib: 默认预加载的部分FONT字库BIN文件ID
 * @param	itemCount : 预加载字体的个数(>=默认预加载的部分FONT字数)
 *
 * @return  T_FONTHandle : success for FONT handle else return AK_NULL
 *
 */
T_FONTHandle	FONT_Load(ANYKAFONT fontID, T_U16 fontSZ, T_U32 idFtlib, T_U32 idFtdeftxt, T_U32 idFtdeflib, T_U32 itemCount);



/**
 * @brief  Initialize FONT hashtable and preloading VIP word
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_FONTHandle hFONT : FONT字库loading handle
 * @param   T_S8 *word : require word
 * @param   T_U8 *data : data for store FONT word
 *
 * @return  T_FONTHandle : success for FONT handle else return AK_NULL
 *
 */
T_BOOL		FONT_GetAnyWord(T_FONTHandle hFONT, T_S8 *word, T_U8 *data);

T_pDATA		FONT_GetAnyWordEx(T_FONTHandle hFONT, T_S8 *word);

/**
 * @brief  trace dynamic count
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_FONTHandle hFONT : FONT字库loading handle
 *
 * @return  T_U32 : return dynamic load count
 *
 */
T_U32		FONT_Stat(T_FONTHandle hFONT);





#ifdef __cplusplus
}
#endif


#endif

