/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKXxx.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKXXX_H__
#define __AKXXX_H__

#include "AKComponent.h"
#include "AKInterface.h"

typedef struct IXxx IXxx;

/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_IXXX(name) \
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
#define AK_INHERIT_IXXX(name) \
      AK_INHERIT_IUNKNOWN(name); \
      T_S32 (*Example)(name* p##name, T_pVOID param)

AK_INTERFACE(IXxx)
{
    AK_INHERIT_IXXX(IXxx);
};

struct IXxx
{
    AK_DEFINEVT(IXxx);
    T_VOID *pData;
};

T_S32 CXxx_New(IXxx **pp);
#define IXxx_AddRef(p)                  AK_GETVT(IXxx, p)->AddRef(p)
#define IXxx_Release(p)                 AK_GETVT(IXxx, p)->Release(p)

#define IXxx_Example(p, param)          AK_GETVT(IXxx, p)->Example(p,param)

#endif //__AKXXX_H__

