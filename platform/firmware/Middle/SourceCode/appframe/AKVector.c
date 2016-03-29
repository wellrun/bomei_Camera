/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKVector.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKVector.h"
#include "eng_debug.h"
#ifdef OS_WIN32
#include <string.h>
#endif
#include "akerror.h"
#include "fwl_osmalloc.h"
#include "akcomponent.h"


//#################################################################################

typedef struct tagCVector
{
   T_U32     m_Ref;
   IVector   m_myIVector;
   //Add your data here...
   T_U16     m_wCapacity;
   T_U16     m_wIncrement;
   T_U16     m_wItemNum;
   
   T_VOID  **m_ppData;
 
}CVector;

/*=================================================================================*/

#define VECTOR_DEFAULT_CAPACITY     50
#define VECTOR_DEFAULT_INCREMENT    5



//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================

static T_U32 CVector_AddRef(IVector *pIVector);
static T_U32 CVector_Release(IVector *pIVector);


//Add your function declaration here...
static T_S32   CVector_Init(IVector *pIVector,T_U16 wCapacity, T_U16 wIncrement);
static T_S32   CVector_AddElement(IVector *pIVector,T_VOID *pItem);
static T_VOID* CVector_SetElementAt(IVector *pIVector,T_U16 wIndex, T_VOID *pItem);
static T_VOID* CVector_ElementAt(IVector *pIVector, T_U16 wIndex);
static T_S32   CVector_InsertAt(IVector *pIVector,T_U16 wIndex, T_VOID *pItem);
static T_VOID* CVector_RemoveAt(IVector *pIVector, T_U16 wIndex);
static T_S32   CVector_RemoveAll(IVector *pIVector);
static T_S32   CVector_Sort(IVector *pIVector, T_PFNITEMCMP pfnItemCmp);
static T_U16   CVector_Size(IVector *pIVector);
static T_U16   CVector_GetCapcity(IVector *pIVector);

//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IVector) g_IVectorFuncs =
{
    CVector_AddRef,
    CVector_Release,
    CVector_Init,
    CVector_AddElement,
    CVector_SetElementAt,
    CVector_ElementAt,
    CVector_InsertAt,
    CVector_RemoveAt,
    CVector_RemoveAll,
    CVector_Sort,
    CVector_Size,
    CVector_GetCapcity
};

static T_S32 CVector_Constructor(CVector *pMe)
{
    //Simple Constructor, Add your code here...
    pMe->m_wCapacity  = VECTOR_DEFAULT_CAPACITY;
    pMe->m_wIncrement = VECTOR_DEFAULT_INCREMENT;
    pMe->m_wItemNum   = 0;
    
    pMe->m_ppData = (T_VOID **)Fwl_Malloc(pMe->m_wCapacity * sizeof(T_VOID *));
    if (AK_NULL == pMe->m_ppData)
    {
       return AK_ENOMEMORY;
    }
	
    return AK_SUCCESS;
}

static T_S32 CVector_Destructor(CVector *pMe)
{
    //Add your code here...
    if (pMe->m_ppData)
    {
        pMe->m_ppData = Fwl_Free(pMe->m_ppData);
    }
    
    return AK_SUCCESS;
}

T_S32 CVector_New(IVector **ppi)
{
    T_S32 nErr = AK_SUCCESS;
    CVector *pNew = AK_NULL;

    if (AK_NULL == ppi)
    {
        return AK_EBADPARAM;
    }
    
    *ppi = AK_NULL;
  
    do 
    {
        pNew = AK_MALLOCRECORD(CVector);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        
        pNew->m_Ref = 1;
        AK_SETVT(&(pNew->m_myIVector), &g_IVectorFuncs);
        pNew->m_myIVector.pData = (T_VOID*)pNew;

        nErr = CVector_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CVector_Release(&pNew->m_myIVector);
            pNew = AK_NULL;
            break;
        }

       *ppi = (IVector *)&pNew->m_myIVector;  
    } while(AK_FALSE);

    return nErr;
}

static T_U32 CVector_AddRef(IVector *pIVector)
{
    CVector *pMe = (CVector *)pIVector->pData;

    return ++pMe->m_Ref;
}

static T_U32 CVector_Release(IVector *pIVector)
{
    CVector *pMe = (CVector *)pIVector->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CVector_Destructor(pMe);
        Fwl_Free(pMe);        
		return 0;
    }

    return pMe->m_Ref;
}

static T_S32 CVector_UpdateCapacity(CVector *pMe)
{
    T_VOID **ppData = AK_NULL;
    T_U16  wCap = pMe->m_wCapacity + pMe->m_wIncrement;
    
    ppData = (T_VOID **)Fwl_Malloc(wCap * sizeof(T_VOID *));
    if (AK_NULL == ppData)
    {
       return AK_ENOMEMORY;
    }
    memcpy(ppData, pMe->m_ppData, pMe->m_wCapacity * sizeof(T_VOID *));
    
    Fwl_Free(pMe->m_ppData);
    pMe->m_ppData = ppData;
    
    pMe->m_wCapacity  = wCap;
    pMe->m_wIncrement = pMe->m_wIncrement * 2;
    
    return AK_SUCCESS;
}

//=============================================================================================
static T_S32   CVector_Init(IVector *pIVector,T_U16 wCapacity, T_U16 wIncrement)
{
    CVector *pMe = (CVector *)pIVector->pData;
    
    pMe->m_wIncrement = wIncrement;
    
    if (wCapacity > pMe->m_wCapacity)
    {
        pMe->m_wCapacity  = wCapacity;
        Fwl_Free(pMe->m_ppData);
        pMe->m_ppData = (T_VOID **)Fwl_Malloc(pMe->m_wCapacity * sizeof(T_VOID *));
        if (AK_NULL == pMe->m_ppData)
        {
           return AK_ENOMEMORY;
        }
    }
    return AK_SUCCESS;   
}


static T_S32   CVector_AddElement(IVector *pIVector,T_VOID *pItem)
{
    CVector *pMe = (CVector *)pIVector->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if (pMe->m_wItemNum >= pMe->m_wCapacity)
    {
        lRet = CVector_UpdateCapacity(pMe);
    }
    
    if (AK_IS_SUCCESS(lRet))
    {
        pMe->m_ppData[pMe->m_wItemNum++] = pItem;
    }
    
    return lRet;   
}


static T_VOID* CVector_SetElementAt(IVector *pIVector,T_U16 wIndex, T_VOID *pItem)
{
    CVector *pMe = (CVector *)pIVector->pData;
    T_VOID *pOldItem = AK_NULL;
    
    if (wIndex < pMe->m_wItemNum)
    {
        pOldItem = pMe->m_ppData[wIndex];
        pMe->m_ppData[wIndex] = pItem;
    }
        
    return pOldItem;
}


static T_VOID* CVector_ElementAt(IVector *pIVector, T_U16 wIndex)
{
    CVector *pMe = (CVector *)pIVector->pData;
    
    if (wIndex >= pMe->m_wItemNum)
    {
        return AK_NULL;   
    }
    
    return pMe->m_ppData[wIndex];   
}


static T_S32   CVector_InsertAt(IVector *pIVector,T_U16 wIndex, T_VOID *pItem)
{
    CVector *pMe = (CVector *)pIVector->pData;
    T_S32 lRet = AK_SUCCESS;
    
    if (wIndex > pMe->m_wItemNum)
    {
        return AK_EBADPARAM;
    }
    
    if (pMe->m_wItemNum >= pMe->m_wCapacity)
    {
        lRet = CVector_UpdateCapacity(pMe);
    }
    
    if (AK_IS_SUCCESS(lRet))
    {
        if (pMe->m_wItemNum > 0)
        {
            T_S16 i = (T_S16)(pMe->m_wItemNum - 1);
        
            while(i >= wIndex)
            {
                pMe->m_ppData[i+1] = pMe->m_ppData[i];
                i--;
            } 
        }        
        pMe->m_ppData[wIndex] = pItem;
        pMe->m_wItemNum++;
    }
    
    return lRet;   
}


static T_VOID* CVector_RemoveAt(IVector *pIVector, T_U16 wIndex)
{
    CVector *pMe = (CVector *)pIVector->pData;
    T_VOID *pOldItem = AK_NULL;
    
    if (wIndex < pMe->m_wItemNum)
    {
        T_U16 i = wIndex + 1;
        
        pOldItem = pMe->m_ppData[wIndex];
        while(i < pMe->m_wItemNum)
        {
            pMe->m_ppData[i-1] = pMe->m_ppData[i];
            i++;
        }
        
        pMe->m_wItemNum--;
    }
    
    return pOldItem;
}


static T_S32   CVector_RemoveAll(IVector *pIVector)
{
    CVector *pMe = (CVector *)pIVector->pData;
    
    pMe->m_wItemNum = 0;
    
    return AK_SUCCESS;   
}


static T_S32   CVector_Sort(IVector *pIVector, T_PFNITEMCMP pfnItemCmp)
{
    //CVector *pMe = (CVector *)pIVector->pData;
    //Add latter....
    
    return AK_SUCCESS;   
}


static T_U16   CVector_Size(IVector *pIVector)
{
    CVector *pMe = (CVector *)pIVector->pData;
    
    return pMe->m_wItemNum;   
}


static T_U16   CVector_GetCapcity(IVector *pIVector)
{
    CVector *pMe = (CVector *)pIVector->pData;
    
    return pMe->m_wCapacity;
}



