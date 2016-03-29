/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKInterface.h
* Function: 
* Author: Eric
* Date: 2007-05-31
* Version: 1.0
*
***************************************************************************/
#ifndef __AKINTERFACE_H__
#define __AKINTERFACE_H__

#include "anyka_types.h"
//#include "AKClassID.h"

#define AK_INTERFACE(i) \
    typedef struct _##i##_vt _##i##_vt; \
    struct _##i##_vt

#define AK_VTABLE(i)  _##i##_vt
#define AK_DEFINEVT(i) const _##i##_vt *pvt
#define AK_GETVT(i, p) ((i *)p)->pvt
#define AK_SETVT(p, v) if ((p) && (v)) (p)->pvt = (v)


typedef struct IUnknown IUnknown;

#define AK_INHERIT_IUNKNOWN(name) \
    T_U32 (*AddRef)(name* p);  \
    T_U32 (*Release)(name* p)


AK_INTERFACE(IUnknown)
{
    AK_INHERIT_IUNKNOWN(IUnknown);
};

struct IUnknown
{
    AK_DEFINEVT(IUnknown);
    T_VOID *pData;
};

#define IUnknown_AddRef(p)                  AK_GETVT(IUnknown, p)->AddRef(p)
#define IUnknown_Release(p)                 AK_GETVT(IUnknown, p)->Release(p)


//////////////////////////////////////////////////////////////////////////////////////
/*
#define AK_ADDREFIF(p) \
    if (p) \
    { \
       IUnknown_AddRef((IUnknown *)(p)); \
    }


    
#define AK_RELEASEIF(p) \
    if (p) \
    { \
       IUnknown_Release((IUnknown *)(p)); \
       p = AK_NULL; \
    }
 */   

#define AK_ADDREFIF(pi)   AK_ADDREFIFEX((IUnknown *)pi)
#define AK_ADDREFIFEX(pi) \
    while (AK_NULL != pi)  \
    { \
       if ((AK_NULL != (((IUnknown *)pi)->pvt)) &&  \
           (AK_NULL != (((IUnknown *)pi)->pvt->AddRef)))\
       { \
          IUnknown_AddRef((IUnknown *)(pi)); \
          break; \
       } \
       pi = (IUnknown *)*((IUnknown **)pi->pData);\
    }
  
  
#define AK_RELEASEIF(pi)   AK_RELEASEIFEX((IUnknown *)pi)    
#define AK_RELEASEIFEX(pi) \
    while (AK_NULL != pi)  \
    { \
       if ((AK_NULL != (((IUnknown *)pi)->pvt)) &&  \
           (AK_NULL != (((IUnknown *)pi)->pvt->Release)))\
       { \
          IUnknown_Release((IUnknown *)(pi)); \
          pi = AK_NULL; \
          break; \
       } \
       pi = (IUnknown *)*((IUnknown **)pi->pData);\
    }

#endif //__AKINTERFACE_H__

