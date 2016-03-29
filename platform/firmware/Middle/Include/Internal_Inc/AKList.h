/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKList.h
* Function: 
* Author: 
* Date:  
* Version: 1.0
*
***************************************************************************/
#ifndef __AKLIST_H__
#define __AKLIST_H__

#include "AKComponent.h"
#include "AKInterface.h"

/*===============================================================*/
typedef T_BOOL (*T_pfnContainCB)(T_VOID* pItem, T_VOID* pKey);

/*===============================================================*/
typedef struct IList IList;

/*************************************************************************/
/**
                       <---- DESCRIPTION ---->
#define AK_INHERIT_IList(name) \
        AK_INHERIT_IUNKNOWN(name); \
        //Add your function pointer list
        //of virtual table here...
If your extention doest inherit from T_VOID, you can use the Inherit 
macro AK_INHERIT_IYyy instead of AK_INHERIT_IUNKNOWN. For ezample:

#define AK_INHERIT_ICAT(name) \
        AK_INHERIT_IANIMAL(name); \
        ...    ...

*/
/*************************************************************************/
#define AK_INHERIT_IList(name) \
      AK_INHERIT_IUNKNOWN(name); \
      T_S32 (*Add)(name* p##name, T_VOID *pItem); \
      T_S32 (*Delete)(name* p##name, T_VOID *pItem); \
      T_VOID* (*GetHead)(name* p##name); \
      T_VOID* (*GetTail)(name* p##name); \
      T_VOID* (*Find)(name* p##name, T_VOID *pKey, T_pfnContainCB pfnCtnCB); \
      T_VOID* (*FindFirst)(name* p##name); \
      T_VOID* (*FindNext)(name* p##name)


AK_INTERFACE(IList)
{
    AK_INHERIT_IList(IList);
};

struct IList
{
    AK_DEFINEVT(IList);
    T_VOID *pData;
};

T_S32 CList_New(IList **pp, T_BOOL bIsAutoFreeItem);
#define IList_AddRef(p)                 AK_GETVT(IList, p)->AddRef(p)
#define IList_Release(p)                AK_GETVT(IList, p)->Release(p)

#define IList_Add(p, pi)				AK_GETVT(IList, p)->Add((p), (pi))
#define IList_Delete(p, pi)				AK_GETVT(IList, p)->Delete((p), (pi))
#define IList_GetHead(p)			    AK_GETVT(IList, p)->GetHead((p))
#define IList_GetTail(p)			    AK_GETVT(IList, p)->GetTail((p))
#define IList_Find(p, k, c)			    AK_GETVT(IList, p)->Find((p), (k), (c))
#define IList_FindFirst(p)			    AK_GETVT(IList, p)->FindFirst((p))
#define IList_FindNext(p)			    AK_GETVT(IList, p)->FindNext((p))

#endif //__AKList_H__

