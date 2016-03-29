/***********************************************************************
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * All rights reserced.
 *
 * File name: AKFrameStream.c
 * Function:this is a simple stream component,define normal stream operation 
 * Author:  wangxi
 * Date:  
 * Version: 1.0
 *
 ***********************************************************************/
#ifndef __AKFRAMESTRM_H__
#define __AKFRAMESTRM_H__

//#include "AKComponent.h"
#include "AKInterface.h"
#include "anyka_types.h"
#include "eng_debug.h"


///////////////////////////////////////////////////////////////////////////
//----Frame(Node) ---------------------------
/**
frame buffer info
 */
typedef struct tagFrameInfo
{
	T_U8   type;   // frame type
	T_RECT rect;   // frame valid  rect
    T_U32  size;   // frame buffer size
    T_U32  width;  // frame buffer width
    T_U32  height; //frmae buffer size
} T_FRM_INFO;

/**
 * frame Status; read/write/dirty
 */
typedef enum {
	eFRM_STATUS_NULL        = 0,
	eFRM_STATUS_WRITE_ING   ,// this frame is writing
	eFRM_STATUS_WRITE_DIRTY ,// this frame is write error  (will drop this fream)
	eFRM_STATUS_WRITE_OK	,// this frame is  write finished
	eFRM_STATUS_READ_ING	,// this frame is reading
	eFRM_STATUS_READ_OK	,// this frame is read finished

} T_FRM_STATUS;

/**
 * frame   data
 */
typedef struct tagFrameData
{
	T_U8		*pBuffer; // frame data pointer
	T_FRM_INFO	 info;    // frame date info
} T_FRM_DATA;

/**
 * frame   data
 */
typedef struct tagFrame
{
    T_FRM_DATA  *pData;// frame data pointer
	T_U8        status;// frame process status
} T_FRM;
//-------------------------------------
//-------stream proc----------------
typedef T_S32 (*T_STEM_GET_PROC)(T_FRM_DATA *pFrmData,     T_pVOID param);
typedef T_S32 (*T_STEM_PUT_PROC)(T_FRM_DATA *pDestFrmData, T_FRM_DATA *pSrcFrmData,  T_pVOID param);
//--------------------------------
/**
 * Stream trans mode
 */
typedef enum {
	eSTRM_TYPE_NULL = 0x10,
	eSTRM_TYPE_DISP, // frame  data  is internal buffer transfering, use tvout buffer
	eSTRM_TYPE_DATA, // frame  data  is internal buffer transfering, not use tvout buffer
	eSTRM_TYPE_ADDR, // frame  data  is extern  buffer transfering
} T_STRM_TYPE;

/**
 * Stream proc mode
 */
typedef enum {
	eSTRM_BLK_NULL = 0x20,
	eSTRM_BLK_TYPE_BLK,   // frame process  block
	eSTRM_BLK_TYPE_NOBLK, // frame process  not block
} T_STRM_BLK_TYPE;

/**
 * stream   Init Param
 */
typedef struct tagStreamInitParam
{
	T_U32            len; // stream node count
	T_FRM_INFO       frameInfo; // single node info
	T_STRM_TYPE      transMode; // stream node transfer mode
	T_STRM_BLK_TYPE  getProcMode;// stream node process(read) mode
	T_STRM_BLK_TYPE  putProcMode;// stream node process(write) mode
	T_BOOL			 enableReadFromOld;//if cannot new data, will tans Old data For User 
	T_BOOL			 enableWriteToOld;//if cannot new data, will tans Old data For User 
    T_BOOL           enableDebugMode; // enble debug infomation  print
} T_STRM_INIT_PARAM;
///////////////////////////////////////////////////////////////////////////
typedef struct IFrmStrm IFrmStrm;

/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_ITHREAD(name) \ 
        AK_INHERIT_IUNKNOWN(name); \
        //Add your function pointer list
        //of virtual table here...
If your extention doest inherit from IUnknown, you can use the Inherit 
macro AK_INHERIT_IYyy instead of AK_INHERIT_IUNKNOWN. For ezample:

#define AK_INHERIT_ICAT(name) \
        AK_INHERIT_IANIMAL(name); \
        ...    ...

 */
/*************************************************************************/
#define AK_INHERIT_IFRMSTRM(name) \
      AK_INHERIT_IUNKNOWN(name); \
      T_S32   (*Init)(name* p##name, T_STRM_INIT_PARAM InitParm); \
      T_S32   (*PutFrameByProc)(name* p##name,  T_STEM_PUT_PROC proc, T_FRM_DATA *srcFrmData, T_pVOID param); \
      T_S32   (*GetFrameByProc)(name* p##name,  T_STEM_GET_PROC proc, T_pVOID param); \
      T_U16   (*Size)(name* p##name);\
      T_U16   (*GetCapcity)(name* p##name);\
	  T_S32	  (*Dump)(name* p##name);\
	  T_U16	  (*GetTransType)(name* p##name);\
	  T_S32	  (*TouchFrmByAddr)(name* p##name, T_U8 *pFrmBufferAddr);\
	  T_S32   (*SetFrmInfo)(name* p##name, T_FRM_INFO *pInfo);\
      T_S32   (*Start)(name* p##name);\
      T_S32   (*Stop)(name* p##name)




AK_INTERFACE(IFrmStrm)
{
    AK_INHERIT_IFRMSTRM(IFrmStrm);
};

struct IFrmStrm
{
    AK_DEFINEVT(IFrmStrm);
    T_VOID *pData;
};

////////////////////////////////////////////////////////////////////////////////////////////

/*
 * @brief   create one  stream instance by init parameter
 * @author WangXi
 * @date	2011-10-25
 * @param[out] ppi: stream handle
 * @param[in] initParam:  init parameter 
 * @return	resulst AK_SUCCESS--create Ok, else  fail
 */
T_S32 CFrmStrm_New(IFrmStrm **ppi, T_STRM_INIT_PARAM initParam);


//Every ext must realize these two function...
#define IFrmStrm_AddRef(p)                    AK_GETVT(IFrmStrm, p)->AddRef(p)
#define IFrmStrm_Release(p)                   AK_GETVT(IFrmStrm, p)->Release(p)

#define IFrmStrm_Init(p,a)                    AK_GETVT(IFrmStrm, p)->Init(p,a)
#define IFrmStrm_PutFrameByProc(p,f,s,a)      AK_GETVT(IFrmStrm, p)->PutFrameByProc(p,f,s,a)
#define IFrmStrm_GetFrameByProc(p,f,a)        AK_GETVT(IFrmStrm, p)->GetFrameByProc(p,f,a)
#define IFrmStrm_Size(p)                      AK_GETVT(IFrmStrm, p)->Size(p)
#define IFrmStrm_GetCapcity(p)                AK_GETVT(IFrmStrm, p)->GetCapcity(p)
#define IFrmStrm_Dump(p)                      AK_GETVT(IFrmStrm, p)->Dump(p)
#define IFrmStrm_GetTransType(p)              AK_GETVT(IFrmStrm, p)->GetTransType(p)
#define IFrmStrm_TouchFrmByAddr(p,a)          AK_GETVT(IFrmStrm, p)->TouchFrmByAddr(p,a)
#define IFrmStrm_SetFrmInfo(p,a)              AK_GETVT(IFrmStrm, p)->SetFrmInfo(p,a)
#define IFrmStrm_Start(p)                     AK_GETVT(IFrmStrm, p)->Start(p)
#define IFrmStrm_Stop(p)                      AK_GETVT(IFrmStrm, p)->Stop(p)


#endif //__AKFRAMESTRM_H__

