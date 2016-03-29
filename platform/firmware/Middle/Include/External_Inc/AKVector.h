/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKVector.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKVECTOR_H__
#define __AKVECTOR_H__

//#include "AKComponent.h"
#include "AKInterface.h"
#include "anyka_types.h"
#include "fwl_vme.h"
#include "Fwl_sysevent.h"
#include "eng_debug.h"
///////////////////////////////////////////////////////////////////////////

typedef T_S32 (*T_PFNITEMCMP)(T_VOID* pItem1, T_VOID* pItem2);


///////////////////////////////////////////////////////////////////////////
typedef struct IVector IVector;

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
#define AK_INHERIT_IVECTOR(name) \
      AK_INHERIT_IUNKNOWN(name); \
      T_S32   (*Init)(name* p##name, T_U16 wCapacity, T_U16 wIncrement); \
      T_S32   (*AddElement)(name* p##name, T_VOID *pItem); \
      T_VOID* (*SetElementAt)(name* p##name, T_U16 wIndex, T_VOID *pItem); \
      T_VOID* (*ElementAt)(name* p##name, T_U16 wIndex); \
      T_S32   (*InsertAt)(name* p##name, T_U16 wIndex, T_VOID *pItem); \
      T_VOID* (*RemoveAt)(name* p##name, T_U16 wIndex); \
      T_S32   (*RemoveAll)(name* p##name); \
      T_S32   (*Sort)(name* p##name, T_PFNITEMCMP pfnItemCmp); \
      T_U16   (*Size)(name* p##name);\
      T_U16   (*GetCapcity)(name* p##name)




AK_INTERFACE(IVector)
{
    AK_INHERIT_IVECTOR(IVector);
};

struct IVector
{
    AK_DEFINEVT(IVector);
    T_VOID *pData;
};

////////////////////////////////////////////////////////////////////////////////////////////
T_S32 CVector_New(IVector **ppi);


//Everry ext must realize these two function...
#define IVector_AddRef(p)              AK_GETVT(IVector, p)->AddRef(p)
#define IVector_Release(p)             AK_GETVT(IVector, p)->Release(p)

#define IVector_Init(p,c,i)            AK_GETVT(IVector, p)->Init(p,c,i)
#define IVector_AddElement(p,d)        AK_GETVT(IVector, p)->AddElement(p,d)
#define IVector_SetElementAt(p,i,d)    AK_GETVT(IVector, p)->SetElementAt(p,i,d)
#define IVector_ElementAt(p,i)         AK_GETVT(IVector, p)->ElementAt(p,i)
#define IVector_InsertAt(p,i,d)        AK_GETVT(IVector, p)->InsertAt(p,i,d)
#define IVector_RemoveAt(p,i)          AK_GETVT(IVector, p)->RemoveAt(p,i)
#define IVector_RemoveAll(p)           AK_GETVT(IVector, p)->RemoveAll(p)
#define IVector_Sort(p,c)              AK_GETVT(IVector, p)->Sort(p,c)
#define IVector_Size(p)                AK_GETVT(IVector, p)->Size(p)
#define IVector_GetCapcity(p)          AK_GETVT(IVector, p)->GetCapcity(p)

#endif //__AKVECTOR_H__

