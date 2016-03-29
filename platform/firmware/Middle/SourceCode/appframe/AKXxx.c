/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKXxx.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKXxx.h"
#include "string.h"

//#################################################################################

typedef struct tagCXxx
{
   T_U32     m_Ref;
   IXxx      m_myIXxx;
   //Add your data member here...
}CXxx;

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================

static T_U32 CXxx_AddRef(IXxx *p);
static T_U32 CXxx_Release(IXxx *p);

//Add your function declaration here...
static T_S32 CXxx_Example(IXxx *pIXxx, T_pVOID param);

//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IXxx) g_IXxxFuncs =
{
    CXxx_AddRef,
    CXxx_Release,
    //Add your function here...
    CXxx_Example,
	//... ...
};


static T_S32 CXxx_Constructor(CXxx *pMe)
{
    //Add your code here...
    
    return AK_SUCCESS;
}

static T_S32 CXxx_Destructor(CXxx *pMe)
{
    //Add your code here...
    
    return AK_SUCCESS;
}

T_S32 CXxx_New(IXxx **pp)
{
    T_S32 nErr = AK_SUCCESS;
    CXxx *pNew = AK_NULL;

    if (AK_NULL == pp)
    {
        return AK_EBADPARAM;
    }
    
    *pp = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CXxx);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);

        pNew->m_Ref     = 1;
        AK_SETVT(&(pNew->m_myIXxx), &g_IXxxFuncs);
        pNew->m_myIXxx.pData = (T_VOID*)pNew;

        nErr = CXxx_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CXxx_Release(&pNew->m_myIXxx);
            pNew = AK_NULL;
            break;
        }

        *pp = (T_VOID *)&pNew->m_myIXxx;

    } while(AK_FALSE);

    return nErr;
}

static T_U32 CXxx_AddRef(IXxx *p)
{
    CXxx *pMe = (CXxx *)p->pData;

    return ++pMe->m_Ref;
}

static T_U32 CXxx_Release(IXxx *p)
{
    CXxx *pMe = (CXxx *)p->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CXxx_Destructor(pMe);
        pMe = Fwl_Free(pMe);
		return 0;
    }

    return pMe->m_Ref;
}

//It's your function...
static T_S32 CXxx_Example(IXxx *pIXxx, T_pVOID param)
{
    //CXxx *pMe = (CXxx *)pIXxx->pData;

	//add your code

	return AK_SUCCESS;
}

