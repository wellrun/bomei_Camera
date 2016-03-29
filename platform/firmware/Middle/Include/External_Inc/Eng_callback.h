#ifndef __ENG_CALLBACK_H__
#define __ENG_CALLBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gbl_macrodef.h"
#include "Fwl_osfs.h"

typedef struct 
{
    T_U16        ResourceID;            
    T_U8        *Buff;         //The PoT_S32er to save the resource. Application should alloc the memory for it
    T_U32        Resource_len;  //if Buff is NULL, and Resource_len is 0, a failure have occurred   
} T_LOADRESOURCE_CB_PARA;

typedef T_S32   (*CALLBACK_FUN_FREAD)(T_hFILE hFile, T_pVOID pBuffer, T_U32 count); 
typedef T_S32   (*CALLBACK_FUN_FWRITE)(T_hFILE hFile, T_VOID *pBuffer, T_U32 count); 
typedef T_S32   (*CALLBACK_FUN_FSEEK)(T_hFILE hFile, T_S32 offset, T_U16 origin); 
typedef T_S32   (*CALLBACK_FUN_FGETLEN)(T_hFILE hFile); 
typedef T_S32   (*CALLBACK_FUN_FTELL)(T_hFILE hFile); 
typedef T_VOID  (*CALLBACK_FUN_LOADRESOURCE)(T_LOADRESOURCE_CB_PARA *pPara); 
typedef T_VOID  (*CALLBACK_FUN_RELEASERESOURCE)(T_U8 *Buff); 
typedef T_VOID* (*CALLBACK_FUN_MALLOC)(T_U32 size, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*CALLBACK_FUN_FREE)(T_VOID* mem, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*CALLBACK_FUN_REMALLOC)(T_VOID* mem, T_U32 size, T_pSTR filename, T_U32 line); 
typedef T_VOID* (*CALLBACK_FUN_DMAMEMCOPY)(T_VOID* dst, T_VOID* src, T_U32 count); 
typedef T_VOID  (*CALLBACK_FUN_SHOWFRAME)(T_VOID* srcImg, T_U32 src_width, T_U32 src_height); 
typedef T_VOID  (*CALLBACK_FUN_CAMERASHOWFRAME)(T_VOID* srcImg, T_U32 src_width, T_U32 src_height); 
typedef T_VOID  (*CALLBACK_FUN_CAPSTART)(T_VOID); 
typedef T_BOOL  (*CALLBACK_FUN_CAPCOMPLETE)(T_VOID); 
typedef T_VOID* (*CALLBACK_FUN_CAPGETDATA)(T_VOID); 
typedef T_U32   (*CALLBACK_FUN_GETTICKCOUNT)(T_VOID); 
typedef T_S32   (*CALLBACK_FUN_PRT_S32F)(T_pCSTR format, ...);
typedef T_BOOL 	(*CALLBACK_FUN_REGMODIFY)(T_pVOID addr, T_U32 value, T_U32 mask); 

typedef T_VOID (*REG_BITS_WRITE_CB)(T_pVOID addr, T_U32 val, T_U32 mask);

typedef struct
{
    CALLBACK_FUN_FREAD                      fread;
    CALLBACK_FUN_FWRITE                     fwrite;
    CALLBACK_FUN_FSEEK                      fseek;
    CALLBACK_FUN_FGETLEN                    fgetlen;
    CALLBACK_FUN_FTELL                      ftell;
    CALLBACK_FUN_LOADRESOURCE               LoadResource;
    CALLBACK_FUN_RELEASERESOURCE            ReleaseResource;
    CALLBACK_FUN_MALLOC                     malloc;
    CALLBACK_FUN_FREE                       free;
    CALLBACK_FUN_REMALLOC                   remalloc;
    CALLBACK_FUN_DMAMEMCOPY                 DMAMemcpy;
    CALLBACK_FUN_SHOWFRAME                  ShowFrame;
    CALLBACK_FUN_CAMERASHOWFRAME            CameraShowFrame;
    CALLBACK_FUN_CAPSTART                   CapStart;
    CALLBACK_FUN_CAPCOMPLETE                CapComplete;
    CALLBACK_FUN_CAPGETDATA                 CapGetData;
    CALLBACK_FUN_GETTICKCOUNT               GetTickCount;
    CALLBACK_FUN_PRT_S32F                   printf;
    CALLBACK_FUN_REGMODIFY					regModify;
} CB_FUNS;


//对phyAddr寄存器中mask中为1的位进行赋值
T_BOOL RegBitsWriteCB(T_pVOID phyAddr, T_U32 val, T_U32 mask);


/**
*@brief init the file system and image,if define OS_ANYKA also init sound and flush
*@param none
*@return none
*/
extern T_VOID Gbl_SetCallbackFuncs(T_VOID);

#endif
