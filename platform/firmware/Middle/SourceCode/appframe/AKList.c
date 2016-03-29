/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKList.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKList.h"
#include "Fwl_osMalloc.h"
//#################################################################################

typedef struct Link
{
	T_VOID *node;
	struct Link *next;
} T_Link,*T_pLink;

typedef struct tagCList
{
   T_U32     m_Ref;
   IList     m_myIList;
   //Add your data here...
   T_pLink   m_head;
   T_pLink   m_tail;
   T_pLink   m_pNextNode;
   T_BOOL    m_bIsAutoFreeItem;
}CList;

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================
static T_U32 CList_AddRef(IList *pi);
static T_U32 CList_Release(IList *pi);

//Add your function declaration here...
static T_S32 CList_Add(IList* pIList, T_VOID *pItem);
static T_S32 CList_Delete(IList* pIList, T_VOID *pItem); 
static T_VOID* CList_GetHead(IList* pIList); 
static T_VOID* CList_GetTail(IList* pIList);
static T_VOID*  CList_Find(IList* pIList, T_VOID *pKey, T_pfnContainCB pfnCtnCB);
static T_VOID* CList_FindFirst(IList* pIList); 
static T_VOID* CList_FindNext(IList* pIList);

//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IList) g_IListFuncs =
{
    CList_AddRef,
    CList_Release,
    //Add your function here...
    CList_Add,
	CList_Delete,
	CList_GetHead,
	CList_GetTail,
	CList_Find,
	CList_FindFirst,
	CList_FindNext
    //... ...
};


static T_S32 CList_Constructor(CList *pMe)
{
    //Add your code here...
    pMe->m_head = AK_NULL;
	pMe->m_tail = AK_NULL;
	pMe->m_pNextNode       = AK_NULL;
    pMe->m_bIsAutoFreeItem = AK_FALSE;

    return AK_SUCCESS;
}

static T_S32 CList_Destructor(CList *pMe)
{
    //Add your code here...
	T_pLink pNext = AK_NULL;
	T_pLink pCur = pMe->m_head;

	while (AK_NULL != pCur)
	{
	    pNext = pCur->next;
        if (pMe->m_bIsAutoFreeItem)
        {
            Fwl_Free(pCur->node); 
        }
		Fwl_Free(pCur); 
		pCur = pNext;
	};

    return AK_SUCCESS;
}

static T_U32 CList_AddRef(IList *pi)
{
    CList *pMe = (CList *)pi->pData;

    return ++pMe->m_Ref;
}

static T_U32 CList_Release(IList *pi)
{
    CList *pMe = (CList *)pi->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CList_Destructor(pMe);
        pMe = Fwl_Free(pMe);
		return 0;
    }

    return pMe->m_Ref;
}

T_S32 CList_New(IList **pp, T_BOOL bIsAutoFreeItem)
{
    T_S32 nErr = AK_SUCCESS;
    CList *pNew = AK_NULL;

    if (AK_NULL == pp)
    {
        return AK_EBADPARAM;
    }
    
    *pp = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CList);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);

        pNew->m_Ref     = 1;
        AK_SETVT(&(pNew->m_myIList), &g_IListFuncs);
        pNew->m_myIList.pData = (T_VOID*)pNew;

        nErr = CList_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CList_Release(&pNew->m_myIList);
            pNew = AK_NULL;
            break;
        }
        pNew->m_bIsAutoFreeItem = bIsAutoFreeItem;

        *pp = (IList *)&pNew->m_myIList;

    } while(AK_FALSE);

    return nErr;
}

//It's your function...
static T_S32 CList_Add(IList *pIList, T_VOID *pItem)
{   
    CList *pMe = (CList *)pIList->pData;

	//Add your code here...
	//T_pLink ptr = pMe->m_head;
	T_pLink newNode;

	//construct the new node
	newNode = AK_MALLOCRECORD(T_Link);
	if (newNode == AK_NULL)
		return AK_ENOMEMORY;

	newNode->node = pItem;
	newNode->next = AK_NULL;

	//add to list
	if (pMe->m_tail == AK_NULL)
	{
		pMe->m_head = newNode;
		pMe->m_tail = pMe->m_head;
	}
	else
	{
		pMe->m_tail->next = newNode;
		pMe->m_tail = newNode;
	}

    return AK_SUCCESS;
}


static T_S32 CList_Delete(IList *pIList, T_VOID *pItem)
{   
    CList *pMe = (CList *)pIList->pData;

	//add your code here...
	T_pLink pCur = pMe->m_head;
	T_pLink pPrev = AK_NULL;

	while (pCur && pCur->node!=pItem)
	{
		pPrev = pCur;
		pCur = pCur->next;
	}

	if (!pCur)
		return AK_ENOTFOUND;

	if (pPrev == AK_NULL)
		pMe->m_head = pCur->next;
	else
		pPrev->next = pCur->next;
    
    if (pMe->m_pNextNode == pCur)
    {
       pMe->m_pNextNode = pCur->next;
    }

    /*如果删除最后一个，则尾指针向前指一个。*/
    if (pMe->m_tail == pCur)
    {
       pMe->m_tail = pPrev;
    }

    if (pMe->m_bIsAutoFreeItem)
    {
        Fwl_Free(pCur->node); 
    }
	pCur = Fwl_Free(pCur);
    
	return AK_SUCCESS;
}

static T_VOID *CList_GetHead(IList *pIList)
{
    CList *pMe = (CList *)pIList->pData;

    if (AK_NULL == pMe->m_head)
    {
        return AK_NULL;
    }
    
	return pMe->m_head->node;
}

static T_VOID *CList_GetTail(IList *pIList)
{
    CList *pMe = (CList *)pIList->pData;
    
    if (AK_NULL == pMe->m_tail)
    {
        return AK_NULL;
    } 
    
	return pMe->m_tail->node;
}

static T_VOID*  CList_Find(IList* pIList, T_VOID *pKey, T_pfnContainCB pfnCtnCB)
{
    CList *pMe = (CList *)pIList->pData;
	T_pLink pCur = pMe->m_head;
	T_pLink pPrev = AK_NULL;

    if (AK_NULL == pKey)
    {
        return AK_NULL;
    }

    if (AK_NULL != pfnCtnCB)
    {
        while ((AK_NULL != pCur) && !pfnCtnCB(pCur->node, pKey))
    	{
    		pPrev = pCur;
    		pCur = pCur->next;
    	}
    }
    else
    {
        while ((AK_NULL != pCur) && (pCur->node != pKey))
    	{
    		pPrev = pCur;
    		pCur = pCur->next;
    	}
    }

	if (AK_NULL != pCur)
		return pCur->node;

     return AK_NULL;
}

static T_VOID *CList_FindFirst(IList *pIList)
{
    CList *pMe = (CList *)pIList->pData;

    if (AK_NULL == pMe->m_head)
    {
        return AK_NULL;
    }
        
    pMe->m_pNextNode = pMe->m_head->next;
    
    return pMe->m_head->node;
}

static T_VOID *CList_FindNext(IList *pIList)
{
    CList *pMe = (CList *)pIList->pData;
    T_VOID *pItem = AK_NULL;

	if (AK_NULL != pMe->m_pNextNode)
    {
        pItem = pMe->m_pNextNode->node;
    	pMe->m_pNextNode = pMe->m_pNextNode->next;
    }

	return pItem;
}


