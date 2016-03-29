/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKWnd.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/

#include "AKWnd.h"
#include "fwl_osmalloc.h"
#include "fwl_pfdisplay.h"

//#################################################################################

typedef struct tagCWnd
{
   T_U32       m_Ref;
   IWnd        m_myIWnd;
   //Add your data here...
   T_LEN	   width;
   T_LEN	   height;
   T_U8*	   dispbuf;
   T_DISP_TYPE type;
   T_U32       buflen;
}CWnd;



////////////////////////////////////////////////////////////////////////////////////////////////////////////


//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================

static T_U32 CWnd_AddRef(IWnd *p);
static T_U32 CWnd_Release(IWnd *p);

//Add your function declaration here...

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(IWnd) g_IWndFuncs =
{
    CWnd_AddRef,
    CWnd_Release,
    //Add your function here...
    //... ...
};


static T_S32 CWnd_Constructor(CWnd *pMe, T_WND_INITPARAM *param)
{
    //Add your code here...
	pMe->width   = param->width;
	pMe->height  = param->height;
	pMe->dispbuf = param->dispbuf;
	pMe->type    = param->type;

	if (AK_NULL == pMe->dispbuf)
    {   
	    pMe->dispbuf = Fwl_GetDispMemory();
    }
	
	switch (pMe->type)
	{
		case AK_DISP_RGB:
			pMe->buflen = pMe->width * pMe->height * 3;
			break;
		case AK_DISP_YUV:
			pMe->buflen = pMe->width * pMe->height * 2;
			break;
		default:
			return AK_EBADPARAM;
			break;
	}
    
    return AK_SUCCESS;
}

static T_S32 CWnd_Destructor(CWnd *pMe)
{
    //Add your code here...
    
    return AK_SUCCESS;
}

T_S32 CWnd_New(IWnd **pp, T_WND_INITPARAM *param)
{
    T_S32 nErr = AK_SUCCESS;
    CWnd *pNew = AK_NULL;

    if (AK_NULL == pp)
    {
        return AK_EBADPARAM;
    }
    
    *pp = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CWnd);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);

        pNew->m_Ref     = 1;
        AK_SETVT(&(pNew->m_myIWnd), &g_IWndFuncs);
        pNew->m_myIWnd.pData = (T_VOID*)pNew;

        nErr = CWnd_Constructor(pNew, param);
        if (AK_IS_FAILURE(nErr))
        {
            //Release resource...
            CWnd_Release(&pNew->m_myIWnd);
            pNew = AK_NULL;
            break;
        }

        *pp = (T_VOID *)&pNew->m_myIWnd;

    } while(AK_FALSE);

    return nErr;
}

static T_U32 CWnd_AddRef(IWnd *p)
{
    CWnd *pMe = (CWnd *)p->pData;

    return ++pMe->m_Ref;
}

static T_U32 CWnd_Release(IWnd *p)
{
    CWnd *pMe = (CWnd *)p->pData;

    if (--pMe->m_Ref == 0)
    {
        (T_VOID)CWnd_Destructor(pMe);
        pMe = Fwl_Free(pMe);
		return 0;
    }

    return pMe->m_Ref;
}

//It's your function...

